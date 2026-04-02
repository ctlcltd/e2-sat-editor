/*!
 * e2-sat-editor/src/gui/e2db_console_gui.h
 * 
 * @link https://github.com/ctlcltd/e2-sat-editor
 * @copyright e2 SAT Editor Team
 * @author Leonardo Laureti
 * @version 1.9.0
 * @license MIT License
 * @license GNU GPLv3 License
 */

#include <string>
#include <vector>

#ifndef e2db_console_gui_h
#define e2db_console_gui_h
#include <QApplication>

#include "../e2db/e2db_console.h"

namespace e2se_gui
{
class termctl : public ::e2se_e2db::termiface
{
	public:
		termctl() {}
		virtual ~termctl() = default;

		static void reset() {}
		void input(bool shell = false, bool ins = false) {}
		void clear() {}
		const std::string str() { return ""; }
		std::istream* stream() { return nullptr; }
		static int paged(int pos, int offset) { return 0; }
		static std::pair<int, int> screensize() { return {}; }
		void dump_log() {}
		void load_history() {}
		void save_history() {}
};

class e2db_console_gui : public ::e2se_e2db::e2db_console
{
	Q_DECLARE_TR_FUNCTIONS(e2db_console_gui)

	public:
		e2db_console_gui();
		~e2db_console_gui(); // final destructor

	protected:
		::e2se_e2db::termiface* term() { return new termctl; }
		static void term_reset() { return termctl::reset(); }
		static int term_paged(int pos, int offset) { return termctl::paged(pos, offset); }
		static std::pair<int, int> term_screensize() { return termctl::screensize(); }
};
}
#endif /* e2db_console_gui_h */
