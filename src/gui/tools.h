/*!
 * e2-sat-editor/src/gui/tools.h
 *
 * @link https://github.com/ctlcltd/e2-sat-editor
 * @copyright e2 SAT Editor Team
 * @author Leonardo Laureti
 * @version 1.1
 * @license MIT License
 * @license GNU GPLv3 License
 */

#ifndef tools_h
#define tools_h
#include <QApplication>
#include <QWidget>
#include <QTextEdit>

#include "../logger/logger.h"
#include "theme.h"
#include "dataHandler.h"

namespace e2se_gui
{
class gui;
class tab;

class tools : protected e2se::log_factory
{
	Q_DECLARE_TR_FUNCTIONS(tools)

	public:

		enum INSPECT_FILTER {
			AllLog,
			Debug,
			Info,
			Error
		};

		tools(tab* tid, gui* gid, QWidget* cwid, dataHandler* data);
		virtual ~tools();
		void inspector();
		void importFileCSV(e2db::FCONVS fci, e2db::fcopts opts);
		void exportFileCSV(e2db::FCONVS fco, e2db::fcopts opts);
		void exportFileHTML(e2db::FCONVS fco, e2db::fcopts opts);
		void destroy();

	protected:
		QString inspectContent(string str, int filter = 0);
		void inspectUpdate(QTextEdit* view, int filter = 0);
		void inspectReset();

	private:
		e2se_gui::theme* theme;
		gui* gid;
		tab* tid;
		QWidget* cwid;
		dataHandler* data = nullptr;
		size_t inspect_pos = 0;
		INSPECT_FILTER inspect_curr;
};
}
#endif /* tools_h */
