/*!
 * e2-sat-editor/src/ftpcom.cpp
 *
 * @link https://github.com/ctlcltd/e2-sat-editor
 * @copyright e2 SAT Editor Team
 * @author Leonardo Laureti
 * @version 0.1
 * @license MIT License
 * @license GNU GPLv3 License
 */

#include <sstream>
#include <string>

#include "commons.h"
#include "ftpcom.h"

using namespace std;

namespace e2se_ftpcom
{
ftpcom::ftpcom(ftp_params params)
{
	debug("ftpcom()");

	if (params.user.empty())
		error("ftpcom()", trw("Missing \"%s\" parameter.", "username"));
	if (params.pass.empty())
		error("ftpcom()", trw("Missing \"%s\" parameter.", "password"));
	if (params.host.empty())
		error("ftpcom()", trw("Missing \"%s\" parameter.", "IP address"));
	if (! params.port)
		error("ftpcom()", trw("Missing \"%s\" parameter.", "port"));
	if (params.actv)
		actv = true;

	host = params.host;
	port = params.port;
	user = params.user;
	pass = params.pass;

	if (params.tpath.empty())
		error("ftpcom()", trw("Missing \"%s\" path parameter.", "Transponders"));
	if (params.bpath.empty())
		error("ftpcom()", trw("Missing \"%s\" path parameter.", "Bouquets"));
	if (params.spath.empty())
		error("ftpcom()", trw("Missing \"%s\" path parameter.", "Services"));

	baset = params.tpath;
	baseb = params.bpath;
	bases = params.spath;
}

bool ftpcom::handle()
{
	curl_global_init(CURL_GLOBAL_DEFAULT);

	this->curl = curl_easy_init();
	curl_easy_setopt(curl, CURLOPT_PROTOCOLS, CURLPROTO_FTP);

	if (! curl)
		return false;

	this->urlp = curl_url();
	curl_url_set(urlp, CURLUPART_SCHEME, "ftp", 0);
	curl_url_set(urlp, CURLUPART_HOST, host.c_str(), 0);
	curl_easy_setopt(curl, CURLOPT_CURLU, urlp);
	curl_easy_setopt(curl, CURLOPT_USERNAME, user.c_str());
	curl_easy_setopt(curl, CURLOPT_PASSWORD, pass.c_str());
	curl_easy_setopt(curl, CURLOPT_PORT, port);
	if (actv)
		curl_easy_setopt(curl, CURLOPT_FTPPORT, "-");
	// curl_easy_setopt(curl, CURLOPT_FTP_RESPONSE_TIMEOUT, 0); // 0 = default no timeout
	// curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);

	return true;
}

CURLcode ftpcom::perform(bool cleanup)
{
	CURLcode res = curl_easy_perform(curl);
	if (cleanup) this->cleanup();
	return res;
}

void ftpcom::cleanup()
{
	curl_url_cleanup(urlp);
	curl_easy_cleanup(curl);
	curl_global_cleanup();
}

bool ftpcom::connect()
{
	debug("connect()");

	if (! handle())
	{
		error("connect()", trs("ftpcom error."));
		return false;
	}
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, discardData);
	CURLcode res = perform();
	return (res == CURLE_OK) ? true : false;
}

void ftpcom::listDir(int path)
{
	debug("list_dir()");

	if (! handle())
		return error("listDir()", trs("ftpcom error."));

	stringstream data;
	string base = getBasePath(path);

	curl_url_set(urlp, CURLUPART_PATH, base.c_str(), 0);
	curl_easy_setopt(curl, CURLOPT_FTPLISTONLY, true);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeData);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &data);
	CURLcode res = perform();

	if (res != CURLE_OK)
		return error("listDir()", trs(curl_easy_strerror(res))); // var error string

	string line;

	while (getline(data, line))
	{
		if (line.empty()) continue;
		cout << line << endl;
	}
}

void ftpcom::upload(int path, string filename, string os)
{
	debug("upload()");

	if (! handle())
		return error("upload()", trs("ftpcom error."));

	string remotefile;
	string base = getBasePath(path);
	int attempts = 3;
	long uplen = 0;
	CURLcode res = CURLE_GOT_NOTHING;
	remotefile = base + '/' + filename;

	curl_url_set(urlp, CURLUPART_PATH, remotefile.c_str(), 0);
	curl_easy_setopt(curl, CURLOPT_UPLOAD, true);
	curl_easy_setopt(curl, CURLOPT_FTP_CREATE_MISSING_DIRS, false);
	curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, getContentLength);
	curl_easy_setopt(curl, CURLOPT_HEADERDATA, &uplen);
	// curl_easy_setopt(curl, CURLOPT_READFUNCTION, readData);
	curl_easy_setopt(curl, CURLOPT_READDATA, &os);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, discardData);

	for (int a = 0; (res != CURLE_OK) && (a < attempts); a++) {
		debug("attempt: " + to_string(a));
		if (a)
		{
			curl_easy_setopt(curl, CURLOPT_NOBODY, true);
			curl_easy_setopt(curl, CURLOPT_HEADER, true);
			res = perform(false);
			if (res != CURLE_OK)
			  continue;
			curl_easy_setopt(curl, CURLOPT_NOBODY, false);
			curl_easy_setopt(curl, CURLOPT_HEADER, false);
			os = os.substr(0, uplen);
			curl_easy_setopt(curl, CURLOPT_APPEND, true);
		}
		else
		{
			curl_easy_setopt(curl, CURLOPT_APPEND, false);
		}
		res = perform(false);
	}

	if (res != CURLE_OK)
		return error("upload()", trs(curl_easy_strerror(res))); // var error string

	cleanup();
}

size_t ftpcom::readData(void* ptr, size_t size, size_t nmemb, void* stream)
{
}

size_t ftpcom::writeData(void* ptr, size_t size, size_t nmemb, void* stream)
{
	string data ((const char*) ptr, size * nmemb);
	*((stringstream*) stream) << data << endl;
	return size * nmemb;
}

size_t ftpcom::discardData(void* ptr, size_t size, size_t nmemb, void* stream)
{
  (void) ptr;
  (void) stream;
  return size * nmemb;
}

size_t ftpcom::getContentLength(void* ptr, size_t size, size_t nmemb, void* stream)
{
	string data ((const char*) ptr, size * nmemb);
	size_t pos = 0;
	if ((pos = data.find("Content-Length:")) != string::npos)
		*((long*) stream) = stoi(data.substr(pos, data.length() - 1));
	return size * nmemb;
}

string ftpcom::getBasePath(int path)
{
	string base;

	switch (path)
	{
		case ftpcom::path_param::transponders:
			base = baset;
		break;
		case ftpcom::path_param::services:
			base = bases;
		break;
		case ftpcom::path_param::bouquets:
			base = baseb;
		break;
		default:
			error("getBasePath()", trw("Unknown \"%s\" parameter.", "path"));
	}

	return base;
}

void ftpcom::debug(string cmsg)
{
	e2se::debug("ftpcom", cmsg);
}

void ftpcom::error(string cmsg, string rmsg)
{
	curl_global_cleanup();
	e2se::error("ftpcom", cmsg, "Error", rmsg);
}

string ftpcom::trs(string str)
{
	return str;
}

string ftpcom::trw(string str, string param)
{
	size_t tsize = str.length() + param.length();
	char tstr[tsize];
	sprintf(tstr, str.c_str(), param.c_str());
	return string (tstr);
}

}
