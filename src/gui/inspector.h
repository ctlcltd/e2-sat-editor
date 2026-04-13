/*!
 * e2-sat-editor/src/gui/inspector.h
 *
 * @link https://github.com/ctlcltd/e2-sat-editor
 * @copyright e2 SAT Editor Team
 * @author Leonardo Laureti
 * @version 2.0.0
 * @license MIT License
 * @license GNU GPLv3 License
 */

#include <string>

using std::string, std::vector, std::map;

#ifndef inspector_h
#define inspector_h
#include <QApplication>
#include <QWidget>
#include <QTextEdit>

#include "../logger/logger.h"
#include "toolkit/DialogDockWidget.h"
#include "theme.h"

namespace e2se_gui
{
class gui;

class inspector : protected e2se::log_factory
{
	Q_DECLARE_TR_FUNCTIONS(inspector)

	public:

		enum INSPECT_FILTER {
			AllLog,
			Debug,
			Info,
			Error
		};

		inspector();
		virtual ~inspector();
		DialogDockWidget* logInspector();
		void close();
		void destroy();

	protected:
		void layout();
		QString inspectContent(string str, int filter = 0);
		void inspectUpdate(QTextEdit* view, int filter = 0);
		void inspectReset();

		//TODO rename
		DialogDockWidget* dial = nullptr;

	private:
		e2se_gui::theme* theme;
		size_t inspect_pos = 0;
		INSPECT_FILTER inspect_curr;
};
}
#endif /* inspector_h */
