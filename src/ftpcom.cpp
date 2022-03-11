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

#include "ftpcom.h"

using namespace std;

namespace e2se_ftpcom
{
void ftpcom::listDir()
{
	debug("list_dir()");

	curl_global_init(CURL_GLOBAL_DEFAULT);

	CURL* curl = curl_easy_init();

	if (! curl)
		return error("listDir()", "ftpcom error.");

	stringstream data;

	curl_easy_setopt(curl, CURLOPT_URL, "ftp://root:test@127.0.0.1:2121/seeds./enigma_db/");
	curl_easy_setopt(curl, CURLOPT_FTPLISTONLY, true);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeData);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &data);
	// curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
	CURLcode response = curl_easy_perform(curl);
	curl_easy_cleanup(curl);

	if (response != CURLE_OK)
		return error("listDir()", curl_easy_strerror(response));

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
	::debug("ftpcom", cmsg);
}

void ftpcom::error(string cmsg, string rmsg)
{
	curl_global_cleanup();
	::error("ftpcom", cmsg, rmsg);
}
}
