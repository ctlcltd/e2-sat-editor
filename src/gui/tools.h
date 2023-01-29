/*!
 * e2-sat-editor/src/gui/tools.h
 *
 * @link https://github.com/ctlcltd/e2-sat-editor
 * @copyright e2 SAT Editor Team
 * @author Leonardo Laureti
 * @version 0.3
 * @license MIT License
 * @license GNU GPLv3 License
 */

#ifndef tools_h
#define tools_h
#include <QWidget>

#include "../logger/logger.h"
#include "dataHandler.h"

namespace e2se_gui
{
class gui;
class tab;

class tools : public e2se::log_factory
{
	public:
		tools(tab* tid, gui* gid, QWidget* cwid, dataHandler* data);
		~tools() {};
		void inspector();
		void importFileCSV(e2db::FCONVS fci, e2db::fcopts opts);
		void exportFileCSV(e2db::FCONVS fco, e2db::fcopts opts);
		void exportFileHTML(e2db::FCONVS fco, e2db::fcopts opts);
		void destroy();

	private:
		gui* gid;
		tab* tid;
		QWidget* cwid;
		dataHandler* data = nullptr;
};
}
#endif /* tools_h */
