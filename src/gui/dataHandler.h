/*!
 * e2-sat-editor/src/gui/dataHandler.h
 *
 * @link https://github.com/ctlcltd/e2-sat-editor
 * @copyright e2 SAT Editor Team
 * @author Leonardo Laureti
 * @version 0.1
 * @license MIT License
 * @license GNU GPLv3 License
 */

#include <string>

using std::string;

#ifndef dataHandler_h
#define dataHandler_h

#include "../logger/logger.h"
#include "e2db_gui.h"

namespace e2se_gui
{
class dataHandler : protected e2se::log_factory
{
	public:
		dataHandler(e2se::logger::session* log);
		~dataHandler();
		void newFile();
		bool readFile(string filename);
		bool writeFile(string path, bool overwrite);

		e2db* dbih = nullptr;
		string filename;
		bool newfile = false;
		bool overwrite = false;
};
}
#endif /* dataHandler_h */