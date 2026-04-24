/*!
 * e2-sat-editor/src/ftpcom/ftpcom.h
 *
 * @link https://github.com/ctlcltd/e2-sat-editor
 * @copyright e2 SAT Editor Team
 * @author Leonardo Laureti
 * @version 2.0.0
 * @license MIT License
 * @license GNU GPLv3 License
 */

#include <functional>
#include <string>
#include <vector>
#include <atomic>
#include <cstdio>

#include <curl/curl.h>

#include "../logger/logger.h"

using std::string, std::vector, std::istream;

#ifndef ftpcom_h
#define ftpcom_h
namespace e2se_ftpcom
{
class ftpcom : protected e2se::log_factory
{
	public:

		inline static bool VERBOSE = false;
		inline static int CONNECT_TIMEOUT = 10;
		inline static int FTP_RESPONSE_TIMEOUT = 10;
		inline static int FTP_TIMEOUT = 0;
		inline static int HTTP_TIMEOUT = 15;
		inline static int TELNET_TIMEOUT = 20;
		inline static int MAX_RESUME_ATTEMPTS = 5;
		inline static bool FILENAME_CHECK = true;

		inline static bool FIX_CRLF = false;

		struct ftp_params
		{
			string host;
			int ftport;
			int htport;
			int tnport;
			bool actv = false;
			string user;
			string pwrd;
			string tpath;
			string spath;
			string bpath;
			string ifreload;
			string tnreload;
		};

		struct ftpcom_file
		{
			string path;
			string filename;
			string data;
			string mime;
			size_t size = 0;
		};

		ftpcom();
		virtual ~ftpcom();
		void setParameters(ftp_params params);
		bool handle();
		bool connect();
		bool disconnect();
		bool reconnect();
		void abort();
		string get_server_hostname();
		vector<string> list_dir(string basedir);
		bool file_valid_check(string path);
		string file_mime_value(string path);
		void download_data(string basedir, string filename, ftpcom_file& file);
		void upload_data(string basedir, string filename, ftpcom_file file);
		void fetch_paths();
		bool cmd_ifreload();
		bool cmd_tnreload();

		vector<string> ftdb;

	protected:
		struct sio
		{
			string data;
			size_t size = 0;
		};

		struct soi
		{
			const char* data;
			size_t size = 0;
		};

		struct tnvars
		{
			soi* ps;
			int step = 0;
			string user;
			string pwrd;
			bool send = false;
			string cmd;
		};

		CURLcode perform(CURL* ch);
		void reset(CURL* ch, CURLU* rh);
		void cleanup(CURL* ch);
		vector<string> list_dir_nlst(string basedir);
		vector<string> list_dir_mlsd(string basedir);
		static int data_abort_func(void* ptr, curl_off_t, curl_off_t, curl_off_t, curl_off_t);
		static size_t data_download_func(void* csi, size_t size, size_t nmemb, void* pso);
		static size_t data_upload_func(char* cso, size_t size, size_t nmemb, void* psi);
		static size_t data_write_func(void* csi, size_t size, size_t nmemb, void* pso);
		static size_t data_discard_func(void* csi, size_t size, size_t nmemb, void* pso);
		static size_t data_tn_shell_func(char* cso, size_t size, size_t nmemb, void* psi);
		static size_t get_content_length_func(void* csi, size_t size, size_t nmemb, void* pso);
		static void fix_crlf(string& line);

		virtual string msg(string str, string param) { return e2se::logger::msg(str, param); }
		virtual string msg(string str) { return e2se::logger::msg(str); }

	private:
		bool actv = false;
		string host;
		int ftport;
		int htport;
		int tnport;
		string user;
		string pwrd;
		string baset;
		string bases;
		string baseb;
		string ifreload;
		string tnreload;
		bool mlsd = true;
		CURL* cph = nullptr;
		CURL* csh = nullptr;
		CURL* cth = nullptr;
		CURLU* rph = nullptr;
		CURLU* rsh = nullptr;
		CURLU* rth = nullptr;
		std::atomic<bool> aborting = false;
};
}
#endif /* ftpcom_h */
