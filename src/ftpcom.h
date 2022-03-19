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

#include <cstdio>
#include <curl/curl.h>

using namespace std;

#ifndef ftpcom_h
#define ftpcom_h
namespace e2se_ftpcom
{
class ftpcom
{
	public:
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
        ftpcom(ftp_params params);
		bool handle();
		CURLcode perform(bool cleanup = true);
		void cleanup();
		bool connect();
		void listDir(int path);
		void upload(int path, string filename, string os);
	protected:
		static size_t writeData(void* ptr, size_t size, size_t nmemb, void* stream);
		static size_t readData(void* ptr, size_t size, size_t nmemb, void* stream);
		static size_t discardData(void* ptr, size_t size, size_t nmemb, void* stream);
		static size_t getContentLength(void* ptr, size_t size, size_t nmemb, void* stream);
		void debug(string cmsg);
		void error(string cmsg, string rmsg);
		string trs(string str);
		string trw(string str, string param);
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
		string getBasePath(int path);
};
}
#endif /* ftpcom_h */
