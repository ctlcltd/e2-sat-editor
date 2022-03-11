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
		void listDir(int path);
	protected:
		static size_t writeData(void* ptr, size_t size, size_t nmemb, void* ss);
		void debug(string cmsg);
		void error(string cmsg, string rmsg);
		string trs(string str);
		string trw(string str, string param);
    private:
        string uri;
        string baset;
        string bases;
        string baseb;
};
}
#endif /* ftpcom_h */
