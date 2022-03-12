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
#include <cstdio>

#include <curl/curl.h>

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

	uri  = "//";
	uri += params.user + ':' + params.pass;
	uri += '@' + params.host + ':' + to_string(params.port);

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

void ftpcom::listDir(int path)
{
	debug("list_dir()");

	curl_global_init(CURL_GLOBAL_DEFAULT);

	CURL* curl = curl_easy_init();
	curl_easy_setopt(curl, CURLOPT_PROTOCOLS, CURLPROTO_FTP);

	if (! curl)
		return error("listDir()", trs("ftpcom error."));

	stringstream data;
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
			return error("listDir()", trw("Unknown \"%s\" parameter.", "path"));
	}

	curl_easy_setopt(curl, CURLOPT_URL, ("ftp:" + uri + base).c_str());
	if (actv)
		curl_easy_setopt(curl, CURLOPT_FTPPORT, "-");
	curl_easy_setopt(curl, CURLOPT_FTPLISTONLY, true);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeData);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &data);
	// curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
	CURLcode response = curl_easy_perform(curl);
	curl_easy_cleanup(curl);

	if (response != CURLE_OK)
		return error("listDir()", trs(curl_easy_strerror(response))); // var error string

	string line;

	while (getline(data, line))
	{
		if (line.empty()) continue;
		cout << line << endl;
	}

	curl_global_cleanup();
}

size_t ftpcom::writeData(void* ptr, size_t size, size_t nmemb, void* ss)
{
	size_t relsize = size * nmemb;
	string data ((const char*) ptr, relsize);
	*((stringstream*) ss) << data << endl;
	return relsize;
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
