/*!
 * e2-sat-editor/src/gui/ftpcom_gui_wasm.h
 * 
 * @link https://github.com/ctlcltd/e2-sat-editor
 * @copyright e2 SAT Editor Team
 * @author Leonardo Laureti
 * @version 1.8.0
 * @license MIT License
 * @license GNU GPLv3 License
 */

#include <functional>
#include <string>
#include <unordered_map>

using std::string, std::unordered_map;

#ifndef ftpcom_gui_h
#define ftpcom_gui_h

namespace e2se_ftpcom
{
struct ftpcom
{
	public:
		struct ftpcom_file
		{
			string filename;
			string data;
			string mime;
			size_t size = 0;
		};
};
}

namespace e2se_gui
{
using ftpcom_file = e2se_ftpcom::ftpcom::ftpcom_file;

class ftpcom
{
	public:
		ftpcom() {}
		virtual ~ftpcom() = default;
		bool connect() { return false; }
		bool disconnect() { return false; }
		void didChange() {}
		string get_server_hostname() { return ""; }
		unordered_map<string, ftpcom_file> get_files(std::function<void(const string filename)> func) { unordered_map<string, ftpcom_file> files; return files; }
		void put_files(unordered_map<string, ftpcom_file> files, std::function<void(const string filename)> func) {}
		bool cmd_ifreload() { return false; }
		bool cmd_tnreload() { return false; }
};
}
#endif /* ftpcom_gui_h */
