/*!
 * e2-sat-editor/src/cli/cli.h
 * 
 * @link https://github.com/ctlcltd/e2-sat-editor
 * @copyright e2 SAT Editor Team
 * @author Leonardo Laureti
 * @version 2.0.0
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

class stream : public streamiface
{
	public:
		stream(std::ostream& p) : os(p) {}
		virtual ~stream() = default;

		streamiface &operator<<(int i) override { os << i; return *this; }
		streamiface &operator<<(char c) override { os << c; return *this; }
		streamiface &operator<<(const char* s) override { os << s; return *this; }
		streamiface &operator<<(const std::string& s) override { os << s; return *this; }
		streamiface &operator<<(const streamiface&) override { return *this; }
		// streamiface &operator<<(std::ostream&(*p)(std::ostream&)) override { os << p; return *this; }
		// streamiface &operator<<(std::ios_base&(*p)(std::ios_base&)) override { os << p; return *this; }

		streamiface &endl() override { os << std::endl; return *this; }
		streamiface &flush() override { os << std::flush; return *this; }
		streamiface &left() override { os << std::left; return *this; }
		streamiface &right() override { os << std::right; return *this; }
		int width() const override { return os.width(); }
		int width(int width) override { return os.width(width); }

	private:
		std::ostream& os;
};


class cli : public e2db_console
{
	public:
		cli(int argc, char* argv[]);
		virtual ~cli() = default;
		int exited();

	protected:
		void options(int argc, char* argv[]);

		void cmd_shell();
		void cmd_version();
		void cmd_error(string opt);
		void cmd_usage();

		void console_version(bool extended = false);
		void console_exit();
};

}
#endif /* e2se_cli_h */
