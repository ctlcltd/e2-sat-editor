/*!
 * e2-sat-editor/src/cli/e2db_termctl.h
 * 
 * @link https://github.com/ctlcltd/e2-sat-editor
 * @copyright e2 SAT Editor Team
 * @author Leonardo Laureti
 * @version 1.6.0
 * @license MIT License
 * @license GNU GPLv3 License
 */

#include <iostream>

#ifndef e2db_termctl_h
#define e2db_termctl_h
namespace e2se_cli
{
class e2db_termctl
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
			StdinRelease = KeyReturn
		};

		e2db_termctl();
		~e2db_termctl();
		static void reset();
		void input(bool shell = false, bool ins = false);
		void clear();
		const std::string str();
		std::istream* stream();
		static int paged(int pos, int offset);
		static std::pair<int, int> screensize();
		void debugger();
		void tmp_history();

	private:
		static void tty_set_raw(int tty_fd = 0);
		static void tty_set_sane(int tty_fd = 0);
		static std::pair<int, int> tty_get_screensize();
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
#endif /* e2db_termctl_h */
