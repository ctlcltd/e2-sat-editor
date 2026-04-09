/*!
 * e2-sat-editor/src/gui/termctl_gui.h
 * 
 * @link https://github.com/ctlcltd/e2-sat-editor
 * @copyright e2 SAT Editor Team
 * @author Leonardo Laureti
 * @version 1.9.0
 * @license MIT License
 * @license GNU GPLv3 License
 */

#include <functional>
#include <iostream>
#include <string>
#include <vector>

#ifndef termctl_gui_h
#define termctl_gui_h
#include <QApplication>
#include <QPlainTextEdit>
#include <QTextStream>

#include "toolkit/ConsoleWidget.h"
#include "../e2db/e2db_console.h"

namespace e2se_gui
{
class termctl_gui : public ::e2se_e2db::termiface
{
	public:

		enum EVENT {
			HistoryBack = Qt::Key_Down,
			HistoryForward = Qt::Key_Down,
			PagePrev = Qt::Key_Up,
			PageNext = Qt::Key_Down,
			CursorForward = Qt::Key_Right,
			CursorBackward = Qt::Key_Left,
			DeleteChar = Qt::Key_Backspace,
			InputReturn = Qt::Key_Return,
			InputEnter = Qt::Key_Enter
		};

		termctl_gui(ConsoleWidget* widget);
		virtual ~termctl_gui() = default;
		void handler(bool command = false);

		void clear();
		std::istream* ptr();
		const std::string str();
		void reset();
		int paged(int pos, int offset);
		std::pair<int, int> screensize() { return {}; }
		void dump_log() {}
		void load_history() {}
		void save_history() {}

		void input(std::function<void()> func)
		{
			this->inputCallback = func;
		}

	protected:
		bool eventFilter(QObject* object, QEvent* event);
		bool eventDrop(QObject* object, QEvent* event);
		void callInputCallback(EVENT key, const QString str);

	private:
		ConsoleWidget* widget = nullptr;
		std::iostream* is;
		std::streampos last;
		bool command = false;
		std::function<void()> inputCallback;
		bool connected = false;
};
}
#endif /* termctl_gui_h */
