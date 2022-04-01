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
#include <cstdio>

#include <curl/curl.h>

#include "logger.h"

using std::string;

#ifndef ftpcom_h
#define ftpcom_h
namespace e2se_ftpcom
{
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
		enum path_param
		{
			transponders,
			services,
			bouquets
		};
		ftpcom();
		void setup(ftp_params params);
		bool handle();
		CURLcode perform(bool cleanup = false);
		void cleanup();
		bool connect();
		bool disconnect();
		void listDir(path_param path);
		void uploadData(path_param path, string filename, string os);
	protected:
		struct soi {
			const char* data;
			size_t sizel;
		};
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
		string getBasePath(path_param path);
};
}
#endif /* ftpcom_h */
