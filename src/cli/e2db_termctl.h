/*!
 * e2-sat-editor/src/cli/e2db_termctl.h
 * 
 * @link https://github.com/ctlcltd/e2-sat-editor
 * @copyright e2 SAT Editor Team
 * @author Leonardo Laureti
 * @version 0.6
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
		e2db_termctl();
		~e2db_termctl();
		static void reset();
		std::istream* input();
		std::istream* clear();
		void debugger();
		void tmp_history();

	private:
		static void tty_set_raw(int tty_fd = 0);
		static void tty_set_sane(int tty_fd = 0);
		static void tty_gotoxy(int x, int y);
		static void tty_gotoright();
		static void tty_gotoleft();
		static void tty_erase();
		static void tty_bell();

		std::iostream* is;
		std::iostream* history;
		std::streampos last;
};
}
#endif /* e2db_termctl_h */
