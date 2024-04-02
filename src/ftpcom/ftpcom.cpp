/*!
 * e2-sat-editor/src/ftpcom/ftpcom.cpp
 *
 * @link https://github.com/ctlcltd/e2-sat-editor
 * @copyright e2 SAT Editor Team
 * @author Leonardo Laureti
 * @version 1.3.0
 * @license MIT License
 * @license GNU GPLv3 License
 */

#include <cstdio>
#include <cstring>
#include <clocale>
#include <algorithm>
#include <string>
#include <filesystem>
#include <sstream>
#include <stdexcept>

#if defined(unix) || defined(__unix__) || defined(__unix) || defined(linux) || defined(__linux__) || defined(__APPLE__)
#define PLATFORM_UX
#endif

#if defined(WIN32) || defined(_WIN32)
#define PLATFORM_WIN
#endif

#include "ftpcom.h"

using std::string, std::stringstream, std::min, std::endl, std::to_string;

namespace e2se_ftpcom
{

ftpcom::ftpcom()
{
	std::setlocale(LC_NUMERIC, "C");

	this->log = new e2se::logger("ftpcom", "ftpcom");

	curl_global_init(CURL_GLOBAL_DEFAULT);
}

ftpcom::~ftpcom()
{
	curl_global_cleanup();
}

void ftpcom::setParameters(ftp_params params)
{
	debug("setParameters");

	if (params.user.empty())
		error("setParameters", "FTP Error", msg("Missing \"%s\" parameter.", "username"));
	if (params.pass.empty())
		error("setParameters", "FTP Error", msg("Missing \"%s\" parameter.", "password"));
	if (params.host.empty())
		error("setParameters", "FTP Error", msg("Missing \"%s\" parameter.", "IP address"));
	if (! params.ftport)
		error("setParameters", "FTP Error", msg("Missing \"%s\" parameter.", "FTP port"));
	if (! params.htport)
		error("setParameters", "FTP Error", msg("Missing \"%s\" parameter.", "HTTP port"));
	if (! params.tnport)
		error("setParameters", "FTP Error", msg("Missing \"%s\" parameter.", "Telnet port"));

	this->host = params.host;
	this->ftport = params.ftport;
	this->htport = params.htport;
	this->tnport = params.tnport;
	this->user = params.user;
	this->pass = params.pass;
	this->actv = params.actv;

	if (params.tpath.empty())
		error("setParameters", "FTP Error", msg("Missing \"%s\" path parameter.", "Transponders"));
	if (params.bpath.empty())
		error("setParameters", "FTP Error", msg("Missing \"%s\" path parameter.", "Bouquets"));
	if (params.spath.empty())
		error("setParameters", "FTP Error", msg("Missing \"%s\" path parameter.", "Services"));

	this->baset = params.tpath;
	this->baseb = params.bpath;
	this->bases = params.spath;

	this->ifreload = params.ifreload;
	this->tnreload = params.tnreload;
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

#if LIBCURL_VERSION_NUM < 0x075500
	curl_easy_setopt(cph, CURLOPT_PROTOCOLS, CURLPROTO_FTP);
#else
	curl_easy_setopt(cph, CURLOPT_PROTOCOLS_STR, "ftp");
#endif

	curl_easy_setopt(cph, CURLOPT_CURLU, rph);
	curl_easy_setopt(cph, CURLOPT_USERNAME, user.c_str());
	curl_easy_setopt(cph, CURLOPT_PASSWORD, pass.c_str());
	curl_easy_setopt(cph, CURLOPT_PORT, ftport);
	if (actv)
		curl_easy_setopt(cph, CURLOPT_FTPPORT, "-");

	curl_easy_setopt(cph, CURLOPT_CONNECTTIMEOUT, ftpcom::FTP_CONNECT_TIMEOUT);

	if (ftpcom::VERBOSE)
		curl_easy_setopt(cph, CURLOPT_VERBOSE, true);

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

void ftpcom::cleanup(CURL* ch)
{
	curl_easy_cleanup(ch);
}

bool ftpcom::connect()
{
	debug("connect");

	if (! handle())
		return false;

	curl_easy_setopt(cph, CURLOPT_WRITEFUNCTION, data_discard_func);
	CURLcode res = perform(cph);
	return (res == CURLE_OK);
}

bool ftpcom::disconnect()
{
	debug("disconnect");

	if (! cph)
		return false;

	cleanup(cph);

	return true;
}

string ftpcom::get_server_hostname()
{
	debug("get_server_hostname");

	return host.empty() ? "" : host + ':' + to_string(ftport);
}

vector<string> ftpcom::list_dir(string basedir)
{
	debug("list_dir");

	try
	{
		if (this->mlsd)
			return list_dir_mlsd(basedir);
		else
			return list_dir_nlst(basedir);
	}
	catch (std::runtime_error& err)
	{
		int code = std::stoi(err.what());

		if (code == 101)
		{
			error("list_dir", "FTP Error", "Failed to resume FTP connection.");
		}
		else if (code == 102)
		{
			error("list_dir", "FTP Error", "Error");
		}
		// fallback to NLST
		else if (this->mlsd)
		{
			this->mlsd = false;

			return this->list_dir(basedir);
		}
		else
		{
			CURLcode res = (CURLcode) code;

			error("list_dir", "FTP Error", msg(curl_easy_strerror(res))); // var error string
		}
	}
	catch (...)
	{
		error("list_dir", "FTP Error", "Error");
	}

	return {};
}

vector<string> ftpcom::list_dir_nlst(string basedir)
{
	debug("list_dir_nlst");

	if (! handle())
		throw std::runtime_error("101");

	stringstream data;

	if (basedir.size() && basedir[basedir.size() - 1] != '/')
		basedir.append("/");

	string remotedir = '/' + basedir;

	curl_url_set(rph, CURLUPART_PATH, remotedir.c_str(), 0);
	curl_easy_setopt(cph, CURLOPT_FTPLISTONLY, true);
	curl_easy_setopt(cph, CURLOPT_WRITEFUNCTION, data_write_func);
	curl_easy_setopt(cph, CURLOPT_WRITEDATA, &data);
	CURLcode res = perform(cph);

	if (res != CURLE_OK)
	{
		reset(cph, rph);
		data.clear();

		throw std::runtime_error(to_string(res));
	}

	vector<string> list;

	string line;

	while (std::getline(data, line))
	{
#ifdef PLATFORM_WIN
		fix_crlf(line);
#endif

		if (line.empty())
			continue;

		string filename = line;

		if (filename[0] == '.') // hidden file
			continue;

		list.emplace_back(basedir + filename);
	}

	reset(cph, rph);
	data.clear();

	return list;
}

vector<string> ftpcom::list_dir_mlsd(string basedir)
{
	debug("list_dir_mlsd");

	if (! handle())
		throw std::runtime_error("101");

	stringstream data;

	if (basedir.size() && basedir[basedir.size() - 1] != '/')
		basedir.append("/");

	string remotedir = '/' + basedir;

	curl_url_set(rph, CURLUPART_PATH, remotedir.c_str(), 0);
	curl_easy_setopt(cph, CURLOPT_CUSTOMREQUEST, "MLSD");
	curl_easy_setopt(cph, CURLOPT_WRITEFUNCTION, data_write_func);
	curl_easy_setopt(cph, CURLOPT_WRITEDATA, &data);
	CURLcode res = perform(cph);

	if (res != CURLE_OK)
	{
		reset(cph, rph);
		data.clear();

		throw std::runtime_error(to_string(res));
	}

	vector<string> list;

	string line;

	while (std::getline(data, line))
	{
#ifdef PLATFORM_WIN
		fix_crlf(line);
#endif

		if (line.empty())
			continue;

		if (line.rfind("type=dir;") != string::npos) // directory
			continue;

		size_t pos = line.rfind("; ");

		if (pos == string::npos) // not valid
			throw std::runtime_error("102");

		string filename = line.substr(pos + 2);

		if (filename[0] == '.') // hidden file
			continue;

		list.emplace_back(basedir + filename);
	}

	reset(cph, rph);
	data.clear();

	return list;
}

string ftpcom::file_mime_detect(string path)
{
	string filename = std::filesystem::path(path).filename().u8string();

	if (filename == "lamedb")
		return "text/plain";
	else if (filename == "lamedb5")
		return "text/plain";
	else if (filename == "services")
		return "text/plain";
	else if (filename.find("bouquets.") != string::npos)
		return "text/plain";
	else if (filename.find("userbouquet.") != string::npos)
		return "text/plain";
	else if (filename.find("userbouquets.") != string::npos)
		return "text/plain";
	else if (filename == "blacklist")
		return "text/plain";
	else if (filename == "whitelist")
		return "text/plain";
	else if (filename == "services.locked")
		return "text/plain";
	else if (path.rfind(".xml") != string::npos)
		return "text/xml";
	else if (path.rfind(".csv") != string::npos)
		return "text/csv";
	else if (path.rfind(".html") != string::npos)
		return "text/html";
	return "application/octet-stream";
}

void ftpcom::download_data(string basedir, string filename, ftpcom_file& file)
{
	debug("download_data");

	if (! handle())
		return error("download_data", "FTP Error", "Failed to resume FTP connection.");

	sio data;
	data.size = 0;
	CURLcode res = CURLE_GOT_NOTHING;

	if (basedir.size() && basedir[basedir.size() - 1] != '/')
		basedir.append("/");

	string path = basedir + filename;
	string remotefile = '/' + path;

	debug("download_data", "file", path);

	curl_url_set(rph, CURLUPART_PATH, remotefile.c_str(), 0);
	curl_easy_setopt(cph, CURLOPT_WRITEFUNCTION, data_download_func);
	curl_easy_setopt(cph, CURLOPT_WRITEDATA, &data);
	res = perform(cph);

	if (res != CURLE_OK)
	{
		error("download_data", "FTP Error", msg(curl_easy_strerror(res))); // var error string

		reset(cph, rph);

		return;
	}

	reset(cph, rph);

	string mime = file_mime_detect(path);

#ifdef PLATFORM_WIN
	if (mime.find("text/") != string::npos)
	{
		string text;
		string line;

		while (std::getline(data.data, line))
		{
			fix_crlf(line);
			text.append(line);
		}

		data.data = text;
		data.size = text.size();
	}
#endif

	file.path = path;
	file.filename = filename;
	file.mime = mime;
	file.data = data.data;
	file.size = data.size;
}

void ftpcom::upload_data(string basedir, string filename, ftpcom_file file)
{
	if (! handle())
		return error("upload_data", "FTP Error", "Failed to resume FTP connection.");

	soi data;
	data.data = file.data.data();
	data.size = file.size;
	size_t len = 0;
	CURLcode res = CURLE_GOT_NOTHING;

	if (basedir.size() && basedir[basedir.size() - 1] != '/')
		basedir.append("/");

	string path = basedir + filename;
	string remotefile = '/' + path;

	debug("upload_data", "file", path);

	curl_url_set(rph, CURLUPART_PATH, remotefile.c_str(), 0);
	curl_easy_setopt(cph, CURLOPT_UPLOAD, true);
	curl_easy_setopt(cph, CURLOPT_FTP_CREATE_MISSING_DIRS, false);
	curl_easy_setopt(cph, CURLOPT_HEADERFUNCTION, get_content_length_func);
	curl_easy_setopt(cph, CURLOPT_HEADERDATA, &len);
	curl_easy_setopt(cph, CURLOPT_READFUNCTION, data_upload_func);
	curl_easy_setopt(cph, CURLOPT_READDATA, &data);
	curl_easy_setopt(cph, CURLOPT_WRITEFUNCTION, data_discard_func);

	for (int a = 0; (res != CURLE_OK) && (a < MAX_RESUME_ATTEMPTS); a++)
	{
		debug("upload_data", "attempt", (a + 1));

		if (a)
		{
			curl_easy_setopt(cph, CURLOPT_NOBODY, true);
			curl_easy_setopt(cph, CURLOPT_HEADER, true);
			res = perform(cph);
			if (res != CURLE_OK)
				continue;
			curl_easy_setopt(cph, CURLOPT_NOBODY, false);
			curl_easy_setopt(cph, CURLOPT_HEADER, false);
			data.data += len;
			data.size -= len;
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
		error("upload_data", "FTP Error", msg(curl_easy_strerror(res))); // var error string

		reset(cph, rph);

		return;
	}

	reset(cph, rph);
}

void ftpcom::fetch_paths()
{
	debug("fetch_paths");

	ftdb.clear();

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
	os->size += relsize;

	return relsize;
}

size_t ftpcom::data_upload_func(char* cso, size_t size, size_t nmemb, void* psi)
{
	size_t relsize = size * nmemb;
	soi* is = reinterpret_cast<soi*>(psi);

	if (1 > relsize || 0 >= is->size)
		return 0;

	size_t nsize = min(relsize, is->size);
	std::memcpy(cso, is->data, nsize);

	is->data += nsize;
	is->size = (is->size > nsize ? (is->size - nsize) : 0);

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
	int step = vars->step;
	string os ((const char*) cso, size * nmemb);
	string data;

	if (step == 0 && os.find("login:") != string::npos)
	{
		data = vars->user;
		vars->step = 1;
	}
	else if (step == 1 && os.find("Password:") != string::npos)
	{
		data = vars->pass;
		vars->step = 2;
	}
	else if (step == 2 && (os.find("#") != string::npos || os.find(">") != string::npos || os.find("~") != string::npos || os.find("done!") != string::npos))
	{
		vars->send = true;
		vars->step = 3;
	}

	if (vars->send)
	{
		data = vars->cmd;
		vars->send = false;
	}
	if (data.size() != 0)
	{
		data.append("\n");
	}

	vars->ps->data = data.data();
	vars->ps->size = data.size();

	return data_upload_func(cso, size, nmemb, vars->ps);
}

//TODO TEST
size_t ftpcom::get_content_length_func(void* csi, size_t size, size_t nmemb, void* pso)
{
	size_t relsize = size * nmemb;
	string data ((const char*) csi, relsize);
	size_t pos = 0;
	if ((pos = data.find("Content-Length:")) != string::npos)
		*((size_t*) pso) = std::stoi(data.substr(pos, data.length() - 1));
	return relsize;
}

void ftpcom::fix_line(string& line)
{
	if (line.size() != 0 && line[line.size() - 1] == '\r')
		line = line.substr(0, line.size() - 1);
}

unordered_map<string, ftpcom::ftpcom_file> ftpcom::get_files(std::function<void(const string filename)> func)
{
	debug("get_files");

	fetch_paths();

	unordered_map<string, ftpcom_file> files;

	if (ftdb.empty())
		return files;

	for (string & w : ftdb)
	{
		std::filesystem::path fpath = std::filesystem::path(w);
		string basedir = fpath.parent_path().u8string();
		string filename = fpath.filename().u8string();

		if (func)
			func(filename);

		ftpcom_file file;

		download_data(basedir, filename, file);

		files[filename] = file;
	}

	return files;
}

void ftpcom::put_files(unordered_map<string, ftpcom_file> files, std::function<void(const string filename)> func)
{
	debug("put_files");

	fetch_paths();

	if (files.empty())
		return;

	for (auto & x : files)
	{
		std::filesystem::path fpath = std::filesystem::path(x.first);
		string basedir = fpath.parent_path().u8string();
		string filename = fpath.filename().u8string();

		if (func)
			func(filename);

		upload_data(basedir, filename, x.second);
	}
}

//TODO FIX rand SEGFAULT with a strange routine (connect > disconnect > upload > disconnect > upload)
bool ftpcom::cmd_ifreload()
{
	debug("cmd_ifreload");

	if (ifreload == "-")
		return true;

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
	// debug("cmd_ifreload", "URL", url);
	// url = NULL;

#if LIBCURL_VERSION_NUM < 0x075500
	curl_easy_setopt(csh, CURLOPT_PROTOCOLS, CURLPROTO_HTTP);
#else
	curl_easy_setopt(csh, CURLOPT_PROTOCOLS_STR, "http");
#endif

	curl_easy_setopt(csh, CURLOPT_HTTPGET, true);
	curl_easy_setopt(csh, CURLOPT_FOLLOWLOCATION, true);
	curl_easy_setopt(csh, CURLOPT_CURLU, rsh);

	// note: CURLU port value overridden by DNS cache port value
	curl_easy_setopt(csh, CURLOPT_PORT, htport);

	curl_easy_setopt(csh, CURLOPT_WRITEFUNCTION, data_write_func);
	curl_easy_setopt(csh, CURLOPT_WRITEDATA, &data);

	curl_easy_setopt(csh, CURLOPT_CONNECTTIMEOUT, ftpcom::FTP_CONNECT_TIMEOUT);
	curl_easy_setopt(csh, CURLOPT_TIMEOUT, ftpcom::HTTP_TIMEOUT); // 0 = default no timeout

	if (ftpcom::VERBOSE)
		curl_easy_setopt(csh, CURLOPT_VERBOSE, true);

	CURLcode res = perform(csh);

	if (res != CURLE_OK)
	{
		string message;

		if (res == CURLE_COULDNT_CONNECT)
			message = msg("Couldn't connect to STB Webif");
		else
			message = msg(curl_easy_strerror(res)); // var error string

		error("cmd_ifreload", "Webif Reload Error", message);

		reset(csh, rsh);
		data.clear();

		return false;
	}

	bool cmd = false;

	if (data.str().find("True") != string::npos)
		cmd = true;

	// debug("cmd_ifreload", "data", data.str());

	reset(csh, rsh);
	data.clear();

	return cmd;
}

bool ftpcom::cmd_tnreload()
{
	debug("cmd_tnreload");

	if (tnreload == "-")
		return true;

	if (! cth)
		this->cth = curl_easy_init();

	if (! cth)
		return false;

	this->rth = curl_url();
	tnvars data;
	data.ps = new soi;
	data.ps->size = 0;
	data.user = user;
	data.pass = pass;
	data.send = false;
	data.cmd = tnreload.empty() ? "init 3" : tnreload;

	curl_url_set(rth, CURLUPART_SCHEME, "telnet", 0);
	curl_url_set(rth, CURLUPART_HOST, host.c_str(), 0);

#if LIBCURL_VERSION_NUM < 0x075500
	curl_easy_setopt(cth, CURLOPT_PROTOCOLS, CURLPROTO_TELNET);
#else
	curl_easy_setopt(cth, CURLOPT_PROTOCOLS_STR, "telnet");
#endif

	curl_easy_setopt(cth, CURLOPT_CURLU, rth);
	curl_easy_setopt(cth, CURLOPT_PORT, tnport);
	curl_easy_setopt(cth, CURLOPT_READFUNCTION, data_tn_shell_func);
	curl_easy_setopt(cth, CURLOPT_READDATA, &data);
	curl_easy_setopt(cth, CURLOPT_WRITEFUNCTION, data_discard_func);

	curl_easy_setopt(cth, CURLOPT_FAILONERROR, true);
	curl_easy_setopt(cth, CURLOPT_CONNECTTIMEOUT, ftpcom::FTP_CONNECT_TIMEOUT);
	curl_easy_setopt(cth, CURLOPT_TIMEOUT, ftpcom::TELNET_TIMEOUT); // 0 = default no timeout

	if (ftpcom::VERBOSE)
		curl_easy_setopt(cth, CURLOPT_VERBOSE, true);

	// debug("cmd_tnreload", "stdout", "start");

	CURLcode res = perform(cth);

	if (res != CURLE_OK)
	{
		string message;

		if (res == CURLE_COULDNT_CONNECT)
			message = msg("Couldn't connect to STB Telnet");
		else
			message = msg(curl_easy_strerror(res)); // var error string

		error("cmd_tnreload", "Telnet Reload Error", message);

		reset(cth, rth);

		return false;
	}

	// debug("cmd_tnreload", "stdout", "end");

	reset(cth, rth);

	return true;
}

}
