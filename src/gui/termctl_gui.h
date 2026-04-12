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

#include "toolkit/ConsoleWidget.h"
#include "../e2db/e2db_console.h"

namespace e2se_gui
{
class termctl_gui : public ::e2se_e2db::termiface
{
	public:

		enum EVENT {
			InputReturn = Qt::Key_Return,
			InputEnd = Qt::Key_Escape,
			PagePrev = Qt::Key_Up,
			PageNext = Qt::Key_Down,
			HistoryBack = Qt::Key_Down,
			HistoryForward = Qt::Key_Down
		};

		termctl_gui(ConsoleWidget* widget);
		~termctl_gui();
		void handler(HANDLE handle);
		void clear();
		std::istream* ptr();
		const std::string line();
		const std::string token();
		void reset();
		int paged(int pos, int offset);
		std::pair<int, int> screensize();
		virtual void dump_log() {}
		virtual void load_history() {}
		virtual void save_history() {}

		void input(std::function<void()> func)
		{
			this->inputCallback = func;
		}

	protected:
		void callInputCallback(const int key, const QString str);

	private:
		ConsoleWidget* widget = nullptr;
		std::iostream* is;
		std::streampos last;
		HANDLE currhr;
		int currkey;
		std::function<void()> inputCallback;
		bool connected = false;
};
}
#endif /* termctl_gui_h */
