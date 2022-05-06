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

using std::string, std::stringstream, std::min, std::endl, std::to_string;

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

	// std::cout << params.user << ':' << params.pass << '@' << params.host << ':' << params.port << '/' << params.spath << std::endl;
}

bool ftpcom::handle()
{
	if (! curl)
	{
		curl_global_init(CURL_GLOBAL_DEFAULT);
		this->curl = curl_easy_init();
	}
	if (! curl)
		return false;

	this->urlp = curl_url();
	curl_url_set(urlp, CURLUPART_SCHEME, "ftp", 0);
	curl_url_set(urlp, CURLUPART_HOST, host.c_str(), 0);
	curl_easy_setopt(curl, CURLOPT_PROTOCOLS, CURLPROTO_FTP);
	curl_easy_setopt(curl, CURLOPT_CURLU, urlp);
	curl_easy_setopt(curl, CURLOPT_USERNAME, user.c_str());
	curl_easy_setopt(curl, CURLOPT_PASSWORD, pass.c_str());
	curl_easy_setopt(curl, CURLOPT_PORT, port);
	if (actv)
		curl_easy_setopt(curl, CURLOPT_FTPPORT, "-");
	// curl_easy_setopt(curl, CURLOPT_FTP_RESPONSE_TIMEOUT, 0); // 0 = default no timeout
	// curl_easy_setopt(curl, CURLOPT_VERBOSE, true);

	return true;
}

CURLcode ftpcom::perform()
{
	return curl_easy_perform(curl);
}

void ftpcom::reset()
{
	curl_url_cleanup(urlp);
	curl_easy_reset(curl);
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
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, data_discard_func);
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

vector<string> ftpcom::list_dir(string base)
{
	debug("list_dir()");

	vector<string> list;

	if (! handle())
	{
		error("list_dir()", trs("ftpcom error."));
		return list;
	}

	stringstream data;
	string remotedir = base + '/';

	curl_url_set(urlp, CURLUPART_PATH, remotedir.c_str(), 0);
	curl_easy_setopt(curl, CURLOPT_FTPLISTONLY, true);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, data_read_func);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &data);
	CURLcode res = perform();

	if (res != CURLE_OK)
	{
		error("list_dir()", trs(curl_easy_strerror(res))); // var error string
		return list;
	}

	string line;

	while (std::getline(data, line))
	{
		if (line.empty())
			continue;
		list.emplace_back(remotedir + line);
	}

	data.clear();
	reset();

	return list;
}

//TODO resuming
string ftpcom::download_data(string base, string filename)
{
	debug("download_data()");

	if (! handle())
	{
		error("download_data()", trs("ftpcom error."));
		return "";
	}

	sio data;
	data.sizel = 0;
	CURLcode res = CURLE_GOT_NOTHING;
	string remotefile = base + '/' + filename;

	debug("download_data()", "file", remotefile);

	curl_url_set(urlp, CURLUPART_PATH, remotefile.c_str(), 0);
	curl_easy_setopt(curl, CURLOPT_CURLU, urlp);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, data_download_func);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &data);
	res = perform();

	if (res != CURLE_OK)
	{
		error("download_data()", trs(curl_easy_strerror(res))); // var error string
		return "";
	}

	reset();

	return data.data;
}

void ftpcom::upload_data(string base, string filename, string os)
{
	if (! handle())
		return error("upload_data()", trs("ftpcom error."));

	soi data;
	data.data = os.data();
	data.sizel = os.length();
	size_t uplen = 0;
	CURLcode res = CURLE_GOT_NOTHING;
	string remotefile = base + '/' + filename;
	
	debug("upload_data()", "file", remotefile);

	curl_url_set(urlp, CURLUPART_PATH, remotefile.c_str(), 0);
	curl_easy_setopt(curl, CURLOPT_UPLOAD, true);
	curl_easy_setopt(curl, CURLOPT_FTP_CREATE_MISSING_DIRS, false);
	curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, get_content_length_func);
	curl_easy_setopt(curl, CURLOPT_HEADERDATA, &uplen);
	curl_easy_setopt(curl, CURLOPT_READFUNCTION, data_upload_func);
	curl_easy_setopt(curl, CURLOPT_READDATA, &data);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, data_discard_func);

	for (int a = 0; (res != CURLE_OK) && (a < MAX_RESUME_ATTEMPTS); a++) {
		debug("upload_data()", "attempt", to_string(a + 1));
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
		return error("upload_data()", trs(curl_easy_strerror(res))); // var error string

	reset();
}

void ftpcom::fetch_paths()
{
	debug("fetch_paths()");

	unordered_set<string> base = {baset, baseb, bases};
	vector<string> list;

	for (auto & w : base)
	{
		list = list_dir(w);
		ftdb.insert(ftdb.end(), list.begin(), list.end());
	}
}

size_t ftpcom::data_download_func(void* csi, size_t size, size_t nmemb, void* pso)
{
	size_t relsize = size * nmemb;
	sio* os = reinterpret_cast<sio*>(pso);

	if (1 > relsize)
		return 0;

	os->data.append((const char*) csi, relsize);
	os->sizel += relsize;

	return relsize;
}

size_t ftpcom::data_upload_func(char* cso, size_t size, size_t nmemb, void* psi)
{
	size_t relsize = size * nmemb;
	soi* is = reinterpret_cast<soi*>(psi);

	if (1 > relsize || 0 >= is->sizel)
		return 0;

	size_t nsize = min(relsize, is->sizel);
	std::memcpy(cso, is->data, nsize);

	is->data += nsize;
	is->sizel = (is->sizel > nsize ? (is->sizel - nsize) : 0);

	return nsize;
}

size_t ftpcom::data_read_func(void* csi, size_t size, size_t nmemb, void* pso)
{
	size_t relsize = size * nmemb;
	string data ((const char*) csi, relsize);
	*((stringstream*) pso) << data;
	return relsize;
}

size_t ftpcom::data_discard_func(void* csi, size_t size, size_t nmemb, void* pso)
{
	(void) csi;
	(void) pso;
	return size * nmemb;
}

size_t ftpcom::get_content_length_func(void* csi, size_t size, size_t nmemb, void* pso)
{
	size_t relsize = size * nmemb;
	string data ((const char*) csi, relsize);
	size_t pos = 0;
	if ((pos = data.find("Content-Length:")) != string::npos)
		*((size_t*) pso) = std::stoi(data.substr(pos, data.length() - 1));
	return relsize;
}

void ftpcom::debug(string cmsg)
{
	this->log->debug(cmsg);
}

void ftpcom::debug(string cmsg, string optk, string optv)
{
	this->log->debug(cmsg, optk, optv);
}

void ftpcom::error(string cmsg, string rmsg)
{
	curl_global_cleanup();
	this->log->error(cmsg, "Error", rmsg);
}

void ftpcom::error(string cmsg, string optk, string optv)
{
	curl_global_cleanup();
	this->log->error(cmsg, optk, optv);
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

unordered_map<string, ftpcom_file> ftpcom::get_files()
{
	debug("get_files()");

	fetch_paths();

	unordered_map<string, ftpcom_file> files;

	for (auto & w : ftdb)
	{
		std::filesystem::path path = std::filesystem::path(w); //C++17
		string base = path.parent_path().u8string(); //C++17
		string filename = path.filename().u8string(); //C++17
		files[w] = download_data(base, filename);
	}

	return files;
}

void ftpcom::put_files(unordered_map<string, ftpcom_file> files)
{
	debug("put_files()");

	fetch_paths();

	for (auto & x : files)
	{
		std::filesystem::path path = std::filesystem::path(x.first); //C++17
		string base = path.parent_path().u8string(); //C++17
		string filename = path.filename().u8string(); //C++17
		upload_data(base, x.first, x.second);
	}
}

}
