/*!
 * e2-sat-editor/src/ftpcom/ftpcom.cpp
 *
 * @link https://github.com/ctlcltd/e2-sat-editor
 * @copyright e2 SAT Editor Team
 * @author Leonardo Laureti
 * @version 0.1
 * @license MIT License
 * @license GNU GPLv3 License
 */

#include <cstdio>
#include <cstring>
#include <algorithm>
#include <string>
#include <filesystem>
#include <sstream>

#include "ftpcom.h"

using std::string, std::stringstream, std::min, std::endl, std::to_string;

namespace e2se_ftpcom
{

/*ftpcom::ftpcom(e2se::logger::session log)
{
	this->log = new e2se::logger(log, "ftpcom");
	debug("ftpcom()");

	curl_global_init(CURL_GLOBAL_DEFAULT);
}*/

ftpcom::ftpcom()
{
	curl_global_init(CURL_GLOBAL_DEFAULT);
}

ftpcom::~ftpcom()
{
	curl_global_cleanup();
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
	if (! params.ftport)
		error("ftpcom()", trw("Missing \"%s\" parameter.", "FTP port"));
	if (! params.htport)
		error("ftpcom()", trw("Missing \"%s\" parameter.", "HTTP port"));
	if (params.actv)
		actv = true;

	host = params.host;
	ftport = params.ftport;
	htport = params.htport;
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
	ifreload = params.ifreload;
	tnreload = params.tnreload;
}

bool ftpcom::handle()
{
	if (! cph)
		this->cph = curl_easy_init();
	if (! cph)
		return false;

	this->rph = curl_url();
	curl_url_set(rph, CURLUPART_SCHEME, "ftp", 0);
	curl_url_set(rph, CURLUPART_HOST, host.c_str(), 0);
	curl_easy_setopt(cph, CURLOPT_PROTOCOLS, CURLPROTO_FTP);
	curl_easy_setopt(cph, CURLOPT_CURLU, rph);
	curl_easy_setopt(cph, CURLOPT_USERNAME, user.c_str());
	curl_easy_setopt(cph, CURLOPT_PASSWORD, pass.c_str());
	curl_easy_setopt(cph, CURLOPT_PORT, ftport);
	if (actv)
		curl_easy_setopt(cph, CURLOPT_FTPPORT, "-");
	//TODO FIX hangs the main thread
	curl_easy_setopt(cph, CURLOPT_CONNECTTIMEOUT, 10);
	// curl_easy_setopt(cph, CURLOPT_FTP_RESPONSE_TIMEOUT, 10); // 0 = default no timeout
	// curl_easy_setopt(cph, CURLOPT_VERBOSE, true);

	return true;
}

CURLcode ftpcom::perform(CURL* ch)
{
	return curl_easy_perform(ch);
}

void ftpcom::reset(CURL* ch, CURLU* rh)
{
	curl_url_cleanup(rh);
	curl_easy_reset(ch);
}

//TODO FIX SIGABRT
/*void ftpcom::cleanup(CURL* ch, CURLU* rh)
{
	// curl_url_cleanup(rh);
	curl_easy_cleanup(ch);
}*/
void ftpcom::cleanup(CURL* ch)
{
	curl_easy_cleanup(ch);
}

bool ftpcom::connect()
{
	debug("connect()");

	if (! handle())
	{
		return false;
	}
	curl_easy_setopt(cph, CURLOPT_WRITEFUNCTION, data_discard_func);
	CURLcode res = perform(cph);
	return (res == CURLE_OK) ? true : false;
}

bool ftpcom::disconnect()
{
	debug("disconnect()");

	if (! cph)
		return false;

	// cleanup(cph, rph);
	cleanup(cph);
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
	string remotedir = '/' + base + '/';

	curl_url_set(rph, CURLUPART_PATH, remotedir.c_str(), 0);
	curl_easy_setopt(cph, CURLOPT_FTPLISTONLY, true);
	curl_easy_setopt(cph, CURLOPT_WRITEFUNCTION, data_write_func);
	curl_easy_setopt(cph, CURLOPT_WRITEDATA, &data);
	CURLcode res = perform(cph);

	if (res != CURLE_OK)
	{
		error("list_dir()", trs(curl_easy_strerror(res))); // var error string
		reset(cph, rph);
		data.clear();
		return list;
	}

	string line;

	while (std::getline(data, line))
	{
		if (line.empty())
			continue;
		list.emplace_back(remotedir + line);
	}

	reset(cph, rph);
	data.clear();

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
	string remotefile = '/' + base + '/' + filename;

	debug("download_data()", "file", remotefile);

	curl_url_set(rph, CURLUPART_PATH, remotefile.c_str(), 0);
	curl_easy_setopt(cph, CURLOPT_WRITEFUNCTION, data_download_func);
	curl_easy_setopt(cph, CURLOPT_WRITEDATA, &data);
	res = perform(cph);

	if (res != CURLE_OK)
	{
		error("download_data()", trs(curl_easy_strerror(res))); // var error string
		reset(cph, rph);
		return "";
	}

	reset(cph, rph);

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
	string remotefile = '/' + base + '/' + filename;
	
	debug("upload_data()", "file", remotefile);

	curl_url_set(rph, CURLUPART_PATH, remotefile.c_str(), 0);
	curl_easy_setopt(cph, CURLOPT_UPLOAD, true);
	curl_easy_setopt(cph, CURLOPT_FTP_CREATE_MISSING_DIRS, false);
	curl_easy_setopt(cph, CURLOPT_HEADERFUNCTION, get_content_length_func);
	curl_easy_setopt(cph, CURLOPT_HEADERDATA, &uplen);
	curl_easy_setopt(cph, CURLOPT_READFUNCTION, data_upload_func);
	curl_easy_setopt(cph, CURLOPT_READDATA, &data);
	curl_easy_setopt(cph, CURLOPT_WRITEFUNCTION, data_discard_func);

	for (int a = 0; (res != CURLE_OK) && (a < MAX_RESUME_ATTEMPTS); a++) {
		debug("upload_data()", "attempt", (a + 1));
		if (a)
		{
			curl_easy_setopt(cph, CURLOPT_NOBODY, true);
			curl_easy_setopt(cph, CURLOPT_HEADER, true);
			res = perform(cph);
			if (res != CURLE_OK)
				continue;
			curl_easy_setopt(cph, CURLOPT_NOBODY, false);
			curl_easy_setopt(cph, CURLOPT_HEADER, false);
			data.data += uplen;
			data.sizel -= uplen;
			curl_easy_setopt(cph, CURLOPT_APPEND, true);
		}
		else
		{
			curl_easy_setopt(cph, CURLOPT_APPEND, false);
		}
		res = perform(cph);
	}

	if (res != CURLE_OK)
	{
		error("upload_data()", trs(curl_easy_strerror(res))); // var error string
		reset(cph, rph);
		return;
	}

	reset(cph, rph);
}

void ftpcom::fetch_paths()
{
	debug("fetch_paths()");

	unordered_set<string> base = {baset, baseb, bases};
	vector<string> list;

	for (auto & q : base)
	{
		list = list_dir(q);
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

size_t ftpcom::data_write_func(void* csi, size_t size, size_t nmemb, void* pso)
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

size_t ftpcom::data_tn_shell_func(char* cso, size_t size, size_t nmemb, void* psi)
{
	tnvars* vars = reinterpret_cast<tnvars*>(psi);
	string os ((const char*) cso, size * nmemb);
	string data;

	if (os.find("login:") != string::npos)
		data = vars->user;
	else if (os.find("Password:") != string::npos)
		data = vars->pass;
	else if (os.find("done!") != string::npos)
		vars->send = true;

	if (vars->send)
	{
		data = vars->cmd;
		vars->send = false;
	}

	vars->ps->data = data.length() ? (data + "\n").data() : data.data();
	vars->ps->sizel = data.length() ? data.length() + 1 : data.length();

	return data_upload_func(cso, size, nmemb, vars->ps);
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

void ftpcom::debug(string msg)
{
	this->log->debug(msg);
}

void ftpcom::debug(string msg, string optk, string optv)
{
	this->log->debug(msg, optk, optv);
}

void ftpcom::debug(string msg, string optk, int optv)
{
	this->log->debug(msg, optk, std::to_string(optv));
}

void ftpcom::info(string msg)
{
	this->log->info(msg);
}

void ftpcom::info(string msg, string optk, string optv)
{
	this->log->info(msg, optk, optv);
}

void ftpcom::info(string msg, string optk, int optv)
{
	this->log->info(msg, optk, std::to_string(optv));
}

void ftpcom::error(string tmsg, string rmsg)
{
	this->log->error(tmsg, "Error", rmsg);
}

void ftpcom::error(string msg, string optk, string optv)
{
	this->log->error(msg, optk, optv);
}

void ftpcom::error(string msg, string optk, int optv)
{
	this->log->error(msg, optk, std::to_string(optv));
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

	if (ftdb.empty())
		return files;
	for (string & w : ftdb)
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

	if (files.empty())
		return;
	for (auto & x : files)
	{
		std::filesystem::path path = std::filesystem::path(x.first); //C++17
		string base = path.parent_path().u8string(); //C++17
		string filename = path.filename().u8string(); //C++17
		upload_data(base, filename, x.second);
	}
}

bool ftpcom::cmd_ifreload()
{
	debug("cmd_ifreload()");

	if (! csh)
		this->csh = curl_easy_init();
	if (! csh)
		return false;

	stringstream data;
	this->rsh = curl_url();

	curl_url_set(rsh, CURLUPART_SCHEME, "http", 0);
	curl_url_set(rsh, CURLUPART_HOST, host.c_str(), 0);
	curl_url_set(rsh, CURLUPART_PORT, to_string(htport).c_str(), 0);
	if (ifreload.empty())
	{
		curl_url_set(rsh, CURLUPART_PATH, "/web/servicelistreload", 0);
		curl_url_set(rsh, CURLUPART_QUERY, "mode=0", 0);
	}
	else
	{
		curl_url_set(rsh, CURLUPART_URL, ifreload.c_str(), 0);
	}

	// char* url;
	// curl_url_get(rsh, CURLUPART_URL, &url, 0);
	// debug("cmd_ifreload()", "URL", url);
	// url = NULL;

	curl_easy_setopt(csh, CURLOPT_PROTOCOLS, CURLPROTO_HTTP);
	curl_easy_setopt(csh, CURLOPT_HTTPGET, true);
	curl_easy_setopt(csh, CURLOPT_FOLLOWLOCATION, true);
	curl_easy_setopt(csh, CURLOPT_CURLU, rsh);
	curl_easy_setopt(csh, CURLOPT_WRITEFUNCTION, data_write_func);
	curl_easy_setopt(csh, CURLOPT_WRITEDATA, &data);
	curl_easy_setopt(csh, CURLOPT_TIMEOUT, HTTP_TIMEOUT); // 0 = default no timeout
	// curl_easy_setopt(csh, CURLOPT_VERBOSE, true);
	CURLcode res = perform(csh);

	if (res != CURLE_OK)
	{
		error("cmd_ifreload()", trs(curl_easy_strerror(res))); // var error string
		reset(csh, rsh);
		data.clear();
		return false;
	}

	bool cmd = false;

	if (data.str().find("True") != string::npos)
		cmd = true;

	// debug("cmd_ifreload()", "data", data.str());

	reset(csh, rsh);
	data.clear();

	return cmd;
}

bool ftpcom::cmd_tnreload()
{
	debug("cmd_tnreload()");

	if (! csh)
		this->csh = curl_easy_init();
	if (! csh)
		return false;

	this->rsh = curl_url();
	tnvars data;
	data.ps = new soi;
	data.ps->sizel = 0;
	data.user = user;
	data.pass = pass;
	data.send = false;
	data.cmd = "init 3";

	curl_url_set(rsh, CURLUPART_SCHEME, "telnet", 0);
	curl_url_set(rsh, CURLUPART_HOST, host.c_str(), 0);

	curl_easy_setopt(csh, CURLOPT_PROTOCOLS, CURLPROTO_TELNET);
	curl_easy_setopt(csh, CURLOPT_CURLU, rsh);
	curl_easy_setopt(csh, CURLOPT_PORT, 23);
	curl_easy_setopt(csh, CURLOPT_READFUNCTION, data_tn_shell_func);
	curl_easy_setopt(csh, CURLOPT_READDATA, &data);
	curl_easy_setopt(cph, CURLOPT_WRITEFUNCTION, data_discard_func);
	curl_easy_setopt(csh, CURLOPT_FAILONERROR, true);
	// curl_easy_setopt(csh, CURLOPT_VERBOSE, true);

	debug("cmd_tnreload()", "stdout", "start");

	CURLcode res = perform(csh);

	if (res != CURLE_OK)
	{
		error("cmd_tnreload()", trs(curl_easy_strerror(res))); // var error string
		reset(csh, rsh);
		return false;
	}
	
	debug("cmd_tnreload()", "stdout", "end");

	reset(csh, rsh);

	return true;
}

}
