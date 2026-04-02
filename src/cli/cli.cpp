/*!
 * e2-sat-editor/src/cli/cli.cpp
 *
 * @link https://github.com/ctlcltd/e2-sat-editor
 * @copyright e2 SAT Editor Team
 * @author Leonardo Laureti
 * @version 1.9.0
 * @license MIT License
 * @license GNU GPLv3 License
 */

#include <cstdio>
#include <clocale>
#include <cstring>
#include <cctype>
#include <algorithm>
#include <iomanip>

// note: std::runtime_error behaviour
#include <stdexcept>

#include "cli.h"

using std::pair, std::endl, std::left;
using std::cout;

namespace e2se_cli
{

cli::cli(int argc, char* argv[])
{
	std::setlocale(LC_NUMERIC, "C");

	pout.rdbuf(std::cout.rdbuf());
	perr.rdbuf(std::cerr.rdbuf());

	if (__objio.out == OBJIO::byline)
		__objio.hrn = false;
	else if (__objio.out == OBJIO::json)
		__objio.hrn = false;

	if (argc > 1)
		options(argc, argv);
	else
		cmd_shell();
}

int cli::exited()
{
	return 0;
}

void cli::options(int argc, char* argv[])
{
	for (int i = 0; i < argc; i++)
	{
		if (i == 0)
			continue;

		// pout << "argc" << ':' << ' ' << i << endl;
		// pout << "argv" << ':' << ' ' << argv[i] << endl;

		string opt = argv[i];

		if (opt == "-h" || opt == "--help")
			return cmd_usage();
#ifdef WIN32
		else if (opt == "/?")
			return cmd_usage();
#endif
		else if (opt == "-v" || opt == "--version")
			return cmd_version();
		else if (opt == "-s" || opt == "--shell")
			return cmd_shell();
		else
			return cmd_error(opt);
	}
}

void cli::version(bool verbose)
{
	if (verbose)
		pout << "e2se-cli" << ' ' << "version" << ' ';
	pout << "1.9.0" << endl;
}

void cli::cmd_shell()
{
	console_header();

	this->log = new e2se::logger("cli", "cmd_shell");
	this->dbih = new e2db;

	auto* termctl = term();

	if (history == HISTORY::file)
		termctl->load_history();

	while (true)
	{
		termctl->input(true);
		string cmd = termctl->str();
		std::istream* is = termctl->stream();
		termctl->clear();

		if (cmd == "quit" || cmd == "exit" || cmd == "q")
		{
			if (history == HISTORY::file)
				termctl->save_history();

			return console_exit();
		}
		else if (cmd == "help" || cmd == "h")
			command_help(is);
		else if (cmd == "version" || cmd == "v")
			command_version();
		else if (cmd == "read" || cmd == "i")
			command_read(is);
		else if (cmd == "write" || cmd == "o")
			command_write(is);
		else if (cmd == "list" || cmd == "l")
			command_list(is);
		else if (cmd == "add" || cmd == "a")
			command_add(is);
		else if (cmd == "edit" || cmd == "e")
			command_edit(is);
		else if (cmd == "remove" || cmd == "r")
			command_remove(is);
		// else if (cmd == "copy" || cmd == "c")
		// 	command_copy(is);
		// else if (cmd == "move" || cmd == "m")
		// 	command_move(is);
		else if (cmd == "set" || cmd == "s")
			command_set(is);
		else if (cmd == "unset" || cmd == "u")
			command_unset(is);
		else if (cmd == "print" || cmd == "p")
			command_print(is);
		else if (cmd == "import")
			command_import(is);
		else if (cmd == "export")
			command_export(is);
		else if (cmd == "merge")
			command_merge(is);
		else if (cmd == "parse")
			command_parse(is);
		else if (cmd == "make")
			command_make(is);
		else if (cmd == "convert")
			command_convert(is);
		else if (cmd == "tool")
			command_tool(is);
		else if (cmd == "macro")
			command_macro(is);
		else if (cmd == "debug")
			command_debug(is);
		else if (cmd == "preferences")
			command_preferences(is);
#if E2SE_BUILD == E2SE_TARGET_DEBUG
		else if (cmd == "dump")
			termctl->dump_log();
#endif
		else if (! cmd.empty())
			console_error(cmd);

		// pout << "input: " << cmd << endl;

		delete is;
	}
}

void cli::cmd_version()
{
	version(false);
}

void cli::cmd_error(string option)
{
	term_reset();

	perr << "e2se-cli: " << msg("Illegal option \"%s\"", option) << endl;
	perr << endl;

	exit(1);
}

void cli::cmd_usage(bool descriptive)
{
	if (descriptive)
		pout << "e2se command line" << endl << endl;

	pout << "e2se-cli [OPTIONS]" << endl;
	pout << endl;
	pout << '\t', cout.width(18), pout << left << "-s --shell", pout << ' ' << "Interactive shell" << endl;
	pout << '\t', cout.width(18), pout << left << "-v --version", pout << ' ' << "Display version" << endl;
	pout << '\t', cout.width(18), pout << left << "-h --help", pout << ' ' << "Display this help and exit" << endl;
}

}
