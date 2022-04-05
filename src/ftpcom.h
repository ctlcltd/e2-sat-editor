/*!
 * e2-sat-editor/src/ftpcom.h
 *
 * @link https://github.com/ctlcltd/e2-sat-editor
 * @copyright e2 SAT Editor Team
 * @author Leonardo Laureti
 * @version 0.1
 * @license MIT License
 * @license GNU GPLv3 License
 */

#include <string>
#include <vector>
#include <unordered_set>
#include <unordered_map>
#include <iostream>
#include <cstdio>

#include <curl/curl.h>

#include "logger.h"

using std::string, std::vector, std::unordered_set, std::unordered_map, std::istream;

#ifndef ftpcom_h
#define ftpcom_h
namespace e2se_ftpcom
{
using ftpcom_file = std::string;

class ftpcom
{
	public:
		static const int MAX_RESUME_ATTEMPTS = 3;

		struct ftp_params
		{
			string host;
			int port;
			bool actv;
			string user;
			string pass;
			string tpath;
			string spath;
			string bpath;
		};
		ftpcom();
		virtual ~ftpcom() = default;
		void setup(ftp_params params);
		bool handle();
		CURLcode perform(bool cleanup = false);
		void cleanup();
		bool connect();
		bool disconnect();
		vector<string> listDir(string base);
		string downloadData(string base, string filename);
		void uploadData(string base, string filename, ftpcom_file os);
		void fetchPaths();
		unordered_map<string, ftpcom_file> getFiles();
		void putFiles(unordered_map<string, ftpcom_file> files);
	protected:
		struct soi {
			const char* data;
			size_t sizel;
		};
		vector<string> ftdb;
		static size_t dataDownload_func(void* csi, size_t size, size_t nmemb, void* pso);
		static size_t dataUpload_func(char* cso, size_t size, size_t nmemb, void* psi);
		static size_t dataRead_func(void* csi, size_t size, size_t nmemb, void* pso);
		static size_t dataDiscard_func(void* csi, size_t size, size_t nmemb, void* pso);
		static size_t getContentLength_func(void* csi, size_t size, size_t nmemb, void* pso);
		virtual string trs(string str);
		virtual string trw(string str, string param);
		virtual void debug(string cmsg);
		virtual void error(string cmsg, string rmsg);
		e2se::logger* log;
	private:
		bool actv;
		string host;
		int port;
		string user;
		string pass;
		string baset;
		string bases;
		string baseb;
		CURL* curl = nullptr;
		CURLU* urlp = nullptr;
};
}
#endif /* ftpcom_h */
