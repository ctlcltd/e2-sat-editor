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

#include <algorithm>
#include <string>
#include <filesystem>
#include <sstream>
#include <cstring>

#include "ftpcom.h"

using std::string, std::stringstream, std::getline, std::min, std::endl, std::to_string, std::stoi;

namespace e2se_ftpcom
{

ftpcom::ftpcom()
{
	this->log = new e2se::logger("ftpcom");
	debug("ftpcom()");
}

void ftpcom::setup(ftp_params params)
{
	debug("setup()");

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

	//TODO remove trailing slash
	baset = params.tpath;
	baseb = params.bpath;
	bases = params.spath;

	std::cout << params.user << ':' << params.pass << '@' << params.host << ':' << params.port << '/' << params.spath << std::endl;
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
	if (cleanup)
		this->cleanup();
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
		return false;
	}
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, dataDiscard_func);
	CURLcode res = perform();
	return (res == CURLE_OK) ? true : false;
}

bool ftpcom::disconnect()
{
	debug("disconnect()");

	if (! curl)
		return false;

	cleanup();
	return true;
}

vector<string> ftpcom::listDir(string base)
{
	debug("listDir()");

	vector<string> list;

	if (! handle())
	{
		error("listDir()", trs("ftpcom error."));
		return list;
	}

	stringstream data;
	string remotedir = base + '/';

	curl_url_set(urlp, CURLUPART_PATH, remotedir.c_str(), 0);
	curl_easy_setopt(curl, CURLOPT_FTPLISTONLY, true);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, dataRead_func);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &data);
	CURLcode res = perform();

	if (res != CURLE_OK)
	{
		error("listDir()", trs(curl_easy_strerror(res))); // var error string
		return list;
	}

	string line;

	while (getline(data, line))
	{
		if (line.empty())
			continue;
		list.emplace_back(remotedir + line);
	}

	cleanup();

	return list;
}

//TODO resuming
string ftpcom::downloadData(string base, string filename)
{
	debug("downloadData()");

	stringstream data;

	if (! handle())
	{
		error("downloadData()", trs("ftpcom error."));
		return data.str();
	}

	CURLcode res = CURLE_GOT_NOTHING;
	string remotefile = base + '/' + filename;

	debug("downloadData() file: " + remotefile);

	curl_url_set(urlp, CURLUPART_PATH, remotefile.c_str(), 0);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, dataDownload_func);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &data);
	res = perform();

	if (res != CURLE_OK)
	{
		error("downloadData()", trs(curl_easy_strerror(res))); // var error string
		return data.str();
	}

	cleanup();

	return data.str();
}

void ftpcom::uploadData(string base, string filename, string os)
{
	debug("uploadData()");

	if (! handle())
		return error("uploadData()", trs("ftpcom error."));

	soi data;
	data.data = os.data();
	data.sizel = os.length();
	size_t uplen = 0;
	CURLcode res = CURLE_GOT_NOTHING;
	string remotefile = base + '/' + filename;
	
	debug("uploadData() file: " + remotefile);

	curl_url_set(urlp, CURLUPART_PATH, remotefile.c_str(), 0);
	curl_easy_setopt(curl, CURLOPT_UPLOAD, true);
	curl_easy_setopt(curl, CURLOPT_FTP_CREATE_MISSING_DIRS, false);
	curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, getContentLength_func);
	curl_easy_setopt(curl, CURLOPT_HEADERDATA, &uplen);
	curl_easy_setopt(curl, CURLOPT_READFUNCTION, dataUpload_func);
	curl_easy_setopt(curl, CURLOPT_READDATA, &data);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, dataDiscard_func);

	for (int a = 0; (res != CURLE_OK) && (a < MAX_RESUME_ATTEMPTS); a++) {
		debug("uploadData() attempt: " + to_string(a + 1));
		if (a)
		{
			curl_easy_setopt(curl, CURLOPT_NOBODY, true);
			curl_easy_setopt(curl, CURLOPT_HEADER, true);
			res = perform();
			if (res != CURLE_OK)
			  continue;
			curl_easy_setopt(curl, CURLOPT_NOBODY, false);
			curl_easy_setopt(curl, CURLOPT_HEADER, false);
			data.data += uplen;
			data.sizel -= uplen;
			curl_easy_setopt(curl, CURLOPT_APPEND, true);
		}
		else
		{
			curl_easy_setopt(curl, CURLOPT_APPEND, false);
		}
		res = perform();
	}

	if (res != CURLE_OK)
		return error("uploadData()", trs(curl_easy_strerror(res))); // var error string

	cleanup();
}

void ftpcom::fetchPaths()
{
	debug("fetchPaths()");

	unordered_set<string> base = {baset, baseb, bases};
	vector<string> list;

	for (auto & w : base)
	{
		list = listDir(w);
		ftdb.insert(ftdb.end(), list.begin(), list.end());
	}
}

//TODO resuming
size_t ftpcom::dataDownload_func(void* csi, size_t size, size_t nmemb, void* pso)
{
	size_t relsize = size * nmemb;
	string data ((const char*) csi, relsize);
	*((stringstream*) pso) << data << endl;
	return relsize;
}

size_t ftpcom::dataUpload_func(char* cso, size_t size, size_t nmemb, void* psi)
{
	size_t relsize = size * nmemb;
	soi* data = reinterpret_cast<soi*>(psi);

	if (1 > relsize || 0 >= data->sizel)
	  return 0;

	size_t nsize = min(relsize, data->sizel);
	std::memcpy(cso, data->data, nsize);

	data->data += nsize;
	data->sizel = (data->sizel > nsize ? (data->sizel - nsize) : 0);

	return nsize;
}

size_t ftpcom::dataRead_func(void* csi, size_t size, size_t nmemb, void* pso)
{
	size_t relsize = size * nmemb;
	string data ((const char*) csi, relsize);
	*((stringstream*) pso) << data << endl;
	return relsize;
}

size_t ftpcom::dataDiscard_func(void* csi, size_t size, size_t nmemb, void* pso)
{
	(void) csi;
	(void) pso;
	return size * nmemb;
}

size_t ftpcom::getContentLength_func(void* csi, size_t size, size_t nmemb, void* pso)
{
	size_t relsize = size * nmemb;
	string data ((const char*) csi, relsize);
	size_t pos = 0;
	if ((pos = data.find("Content-Length:")) != string::npos)
		*((size_t*) pso) = stoi(data.substr(pos, data.length() - 1));
	return relsize;
}

void ftpcom::debug(string cmsg)
{
	this->log->debug(cmsg);
}

void ftpcom::error(string cmsg, string rmsg)
{
	curl_global_cleanup();
	this->log->error(cmsg, "Error", rmsg);
}

string ftpcom::trs(string str)
{
	return str;
}

string ftpcom::trw(string str, string param)
{
	size_t tsize = str.length() + param.length();
	char tstr[tsize];
	std::sprintf(tstr, str.c_str(), param.c_str());
	return string (tstr);
}

//TODO FIX EOL EOF
unordered_map<string, ftpcom_file> ftpcom::getFiles()
{
	debug("getFiles()");

	fetchPaths();

	unordered_map<string, ftpcom_file> files;

	for (auto & w : ftdb)
	{
		std::filesystem::path path = std::filesystem::path(w); //C++17
		string base = path.parent_path().u8string(); //C++17
		string filename = path.filename().u8string(); //C++17
		files[w] = downloadData(base, filename);
	}

	return files;
}

void ftpcom::putFiles(unordered_map<string, ftpcom_file> files)
{
	debug("putFiles()");

	fetchPaths();

	for (auto & x : files)
	{
		std::filesystem::path path = std::filesystem::path(x.first); //C++17
		string base = path.parent_path().u8string(); //C++17
		string filename = path.filename().u8string(); //C++17
		uploadData(base, x.first, x.second);
	}
}

}
