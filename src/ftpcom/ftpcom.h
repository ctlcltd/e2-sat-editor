/*!
 * e2-sat-editor/src/ftpcom/ftpcom.h
 *
 * @link https://github.com/ctlcltd/e2-sat-editor
 * @copyright e2 SAT Editor Team
 * @author Leonardo Laureti
 * @version 0.8
 * @license MIT License
 * @license GNU GPLv3 License
 */

#include <functional>
#include <string>
#include <vector>
#include <unordered_set>
#include <unordered_map>
#include <cstdio>

#include <curl/curl.h>

#include "../logger/logger.h"

using std::string, std::vector, std::unordered_set, std::unordered_map, std::istream;

#ifndef ftpcom_h
#define ftpcom_h
namespace e2se_ftpcom
{
class ftpcom : protected e2se::log_factory
{
	public:

		static const bool VERBOSE = false;
		static const int FTP_CONNECT_TIMEOUT = 10;
		static const int HTTP_TIMEOUT = 60;
		static const int MAX_RESUME_ATTEMPTS = 5;

		struct ftp_params
		{
			string host;
			int ftport;
			int htport;
			bool actv = false;
			string user;
			string pass;
			string tpath;
			string spath;
			string bpath;
			string ifreload;
			string tnreload;
		};

		struct ftpcom_file
		{
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
		string get_server_hostname();
		vector<string> list_dir(string basedir);
		string file_mime_detect(string path);
		void download_data(string basedir, string filename, ftpcom_file& file);
		void upload_data(string basedir, string filename, ftpcom_file file);
		void fetch_paths();
		unordered_map<string, ftpcom_file> get_files(std::function<void(const string filename)> func);
		void put_files(unordered_map<string, ftpcom_file> files, std::function<void(const string filename)> func);
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
			string user;
			string pass;
			bool send = false;
			string cmd;
		};

		static CURLcode perform(CURL* ch);
		static void reset(CURL* ch, CURLU* rh);
		static void cleanup(CURL* ch);
		static size_t data_download_func(void* csi, size_t size, size_t nmemb, void* pso);
		static size_t data_upload_func(char* cso, size_t size, size_t nmemb, void* psi);
		static size_t data_write_func(void* csi, size_t size, size_t nmemb, void* pso);
		static size_t data_discard_func(void* csi, size_t size, size_t nmemb, void* pso);
		static size_t data_tn_shell_func(char* cso, size_t size, size_t nmemb, void* psi);
		static size_t get_content_length_func(void* csi, size_t size, size_t nmemb, void* pso);

		virtual string msg(string str, string param) { return e2se::logger::msg(str, param); }
		virtual string msg(string str) { return e2se::logger::msg(str); }

	private:
		bool actv;
		string host;
		int ftport;
		int htport;
		string user;
		string pass;
		string baset;
		string bases;
		string baseb;
		string ifreload;
		string tnreload;
		CURL* cph = nullptr;
		CURL* csh = nullptr;
		CURLU* rph = nullptr;
		CURLU* rsh = nullptr;
};
}
#endif /* ftpcom_h */
