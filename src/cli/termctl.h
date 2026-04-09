/*!
 * e2-sat-editor/src/cli/termctl.h
 * 
 * @link https://github.com/ctlcltd/e2-sat-editor
 * @copyright e2 SAT Editor Team
 * @author Leonardo Laureti
 * @version 1.9.0
 * @license MIT License
 * @license GNU GPLv3 License
 */

#include <iostream>
#include <string>
#include <vector>

#ifndef e2se_termctl_h
#define e2se_termctl_h
#include "../e2db/e2db_console.h"

namespace e2se_cli
{
class termctl : public ::e2se_e2db::termiface
{
	public:

		enum KEY_MAP {
			EscapeSequence = 27,
			KeyUp = 65,
			KeyDown = 66,
			KeyRight = 67,
			KeyLeft = 68,
			KeyDelete = 127,
			KeyReturn = 10
		};

		enum EVENT {
			HistoryBack = KeyUp,
			HistoryForward = KeyDown,
			PagePrev = KeyUp,
			PageNext = KeyDown,
			CursorForward = KeyRight,
			CursorBackward = KeyLeft,
			DeleteChar = KeyDelete,
			InputReturn = KeyReturn
		};

		termctl();
		~termctl();
		void handler(bool command);
		void clear();
		std::istream* ptr();
		const std::string str();
		void reset();
		int paged(int pos, int offset);
		std::pair<int, int> screensize();
		void dump_log();
		void load_history();
		void save_history();

		std::string log_file;
		std::string history_file;

	private:
		static void tty_setraw(int tty_fd = 0);
		static void tty_setsane(int tty_fd = 0);
		static std::pair<int, int> tty_screensize();
		static void tty_gotoxy(int x, int y);
		static void tty_goforward();
		static void tty_gobackward();
		static void tty_delchar();
		static void tty_eraseline(int cols = 0);
		static void tty_bell();

		std::iostream* is;
		std::iostream* history;
		std::streampos last;
};
}
#endif /* e2se_termctl_h */
