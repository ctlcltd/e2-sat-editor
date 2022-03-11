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
		void listDir();
	protected:
		size_t ftpcom::writeData(void* ptr, size_t size, size_t nmemb, void* ss);
		void error(string cmsg);
		void debug(string cmsg, string rmsg);
};
}
#endif /* ftpcom_h */
