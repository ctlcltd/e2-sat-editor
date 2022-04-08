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
		static const int MAX_RESUME_ATTEMPTS = 5;

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
		CURLcode perform();
		void reset();
		void cleanup();
		bool connect();
		bool disconnect();
		vector<string> list_dir(string base);
		string download_data(string base, string filename);
		void upload_data(string base, string filename, ftpcom_file os);
		void fetch_paths();
		unordered_map<string, ftpcom_file> get_files();
		void put_files(unordered_map<string, ftpcom_file> files);
	protected:
		struct sio {
			string data;
			size_t sizel;
		};
		struct soi {
			const char* data;
			size_t sizel;
		};
		vector<string> ftdb;
		static size_t data_download_func(void* csi, size_t size, size_t nmemb, void* pso);
		static size_t data_upload_func(char* cso, size_t size, size_t nmemb, void* psi);
		static size_t data_read_func(void* csi, size_t size, size_t nmemb, void* pso);
		static size_t data_discard_func(void* csi, size_t size, size_t nmemb, void* pso);
		static size_t get_content_length_func(void* csi, size_t size, size_t nmemb, void* pso);
		virtual string trs(string str);
		virtual string trw(string str, string param);
		virtual void debug(string cmsg);
		virtual void debug(string cmsg, string optk, string optv);
		virtual void error(string cmsg, string rmsg);
		virtual void error(string cmsg, string optk, string optv);
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
