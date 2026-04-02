/*!
 * e2-sat-editor/src/cli/cli.h
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

using std::string, std::vector;

#ifndef e2se_cli_h
#define e2se_cli_h
#include "../logger/logger.h"
#include "../e2db/e2db.h"
#include "../e2db/e2db_console.h"
#include "termctl.h"

using namespace e2se_e2db;

namespace e2se_cli
{
class cli : public e2db_console
{
	public:
		cli(int argc, char* argv[]);
		virtual ~cli() = default;
		int exited();

	protected:
		HISTORY history = HISTORY::file;

		void options(int argc, char* argv[]);
		void version(bool verbose);
		void cmd_shell();
		void cmd_version();
		void cmd_error(string option);
		void cmd_usage(bool descriptive = false);

		::e2se_e2db::termiface* term() { return new termctl; }
		static void term_reset() { return termctl::reset(); }
		static int term_paged(int pos, int offset) { return termctl::paged(pos, offset); }
		static std::pair<int, int> term_screensize() { return termctl::screensize(); }
};
}
#endif /* e2se_cli_h */
