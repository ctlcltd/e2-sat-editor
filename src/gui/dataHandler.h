/*!
 * e2-sat-editor/src/gui/dataHandler.h
 *
 * @link https://github.com/ctlcltd/e2-sat-editor
 * @copyright e2 SAT Editor Team
 * @author Leonardo Laureti
 * @version 1.8.0
 * @license MIT License
 * @license GNU GPLv3 License
 */

#include <string>
#include <vector>

using std::string, std::vector;

#ifndef dataHandler_h
#define dataHandler_h

#include "../logger/logger.h"
#include "e2db_gui.h"

namespace e2se_gui
{
class dataHandler : protected e2se::log_factory
{
	public:
		dataHandler();
		virtual ~dataHandler();
		void newFile();
		bool readFile(string path);
		bool readBlob(string path, unordered_map<string, e2db::e2db_file> files);
		bool writeFile(string path);
		bool haveErrors();
		vector<string> getErrors();
		void clearErrors();
		void setChanged(bool changed);
		bool hasChanged();
		void setNewfile(bool newfile);
		bool isNewfile();
		string getPath();
		void settingsChanged();

		e2db* dbih = nullptr;

	protected:
		string path;
		bool newfile = false;
		bool changed = false;
};
}
#endif /* dataHandler_h */
