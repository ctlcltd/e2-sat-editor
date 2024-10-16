/*!
 * e2-sat-editor/src/cli/e2db_cli.cpp
 *
 * @link https://github.com/ctlcltd/e2-sat-editor
 * @copyright e2 SAT Editor Team
 * @author Leonardo Laureti
 * @version 1.7.0
 * @license MIT License
 * @license GNU GPLv3 License
 */

#include <cstdio>
#include <clocale>
#include <cstring>
#include <cctype>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <filesystem>

// note: std::runtime_error behaviour
#include <stdexcept>

#include "e2db_termctl.h"

#include "e2db_cli.h"

using std::pair, std::cout, std::cerr, std::cin, std::endl, std::left, std::ifstream, std::ofstream;

namespace e2se_cli
{

e2db_cli::e2db_cli(int argc, char* argv[])
{
	std::setlocale(LC_NUMERIC, "C");

	if (__objio.out == OBJIO::byline)
		__objio.hrn = false;
	else if (__objio.out == OBJIO::json)
		__objio.hrn = false;

	if (argc > 1)
		options(argc, argv);
	else
		cmd_shell();
}

int e2db_cli::exited()
{
	return 0;
}

void e2db_cli::options(int argc, char* argv[])
{
	for (int i = 0; i < argc; i++)
	{
		if (i == 0)
			continue;

		// cout << "argc" << ':' << ' ' << i << endl;
		// cout << "argv" << ':' << ' ' << argv[i] << endl;

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

void e2db_cli::version(bool verbose)
{
	if (verbose)
		cout << "e2se-cli" << ' ' << "version" << ' ';
	cout << "1.7.0" << endl;
}

void e2db_cli::cmd_shell()
{
	shell_header();

	this->log = new e2se::logger("cli", "cmd_shell");
	this->dbih = new e2db;

	e2db_termctl* term = new e2db_termctl;

	while (true)
	{
		term->input(true);
		string cmd = term->str();
		std::istream* is = term->stream();
		term->clear();

		if (cmd == "quit" || cmd == "exit" || cmd == "q")
		{
			// term->tmp_history();
			return shell_exit();
		}
		else if (cmd == "help" || cmd == "h")
			shell_command_help(is);
		else if (cmd == "version" || cmd == "v")
			shell_command_version();
		else if (cmd == "read" || cmd == "i")
			shell_command_read(is);
		else if (cmd == "list" || cmd == "l")
			shell_command_list(is);
		else if (cmd == "add" || cmd == "a")
			shell_command_add(is);
		else if (cmd == "edit" || cmd == "e")
			shell_command_edit(is);
		else if (cmd == "remove" || cmd == "r")
			shell_command_remove(is);
		else if (cmd == "copy" || cmd == "c")
			shell_command_copy(is);
		else if (cmd == "move" || cmd == "m")
			shell_command_move(is);
		else if (cmd == "set" || cmd == "s")
			shell_command_set(is);
		else if (cmd == "unset" || cmd == "u")
			shell_command_unset(is);
		else if (cmd == "merge")
			shell_command_merge(is);
		else if (cmd == "print" || cmd == "p")
			shell_command_print(is);
		else if (cmd == "parse")
			shell_command_parse(is);
		else if (cmd == "make")
			shell_command_make(is);
		else if (cmd == "convert")
			shell_command_convert(is);
		else if (cmd == "debug")
			shell_command_debug(is);
		else if (cmd == "preferences")
			shell_command_preferences(is);
		else if (! cmd.empty())
			shell_error(cmd);

		// cout << "input: " << cmd << endl;

		delete is;
	}
}

void e2db_cli::cmd_version()
{
	version(false);
}

void e2db_cli::cmd_error(string option)
{
	e2db_termctl::reset();

	cerr << "e2se-cli: " << msg("Illegal option \"%s\"", option) << endl;
	cerr << endl;

	exit(1);
}

void e2db_cli::cmd_usage(bool descriptive)
{
	if (descriptive)
		cout << "e2se command line" << endl << endl;

	cout << "e2se-cli [OPTIONS]" << endl;
	cout << endl;
	cout << '\t', cout.width(18), cout << left << "-s --shell", cout << ' ' << "Interactive shell" << endl;
	cout << '\t', cout.width(18), cout << left << "-v --version", cout << ' ' << "Display version" << endl;
	cout << '\t', cout.width(18), cout << left << "-h --help", cout << ' ' << "Display this help and exit" << endl;
}

void e2db_cli::shell_exit()
{
	e2db_termctl::reset();
	exit(0);
}

void e2db_cli::shell_header()
{
	version(true);
	cout << endl;
	cout << "Enter \"help\" to display usage informations." << endl;
	cout << endl;
}

void e2db_cli::shell_error(const string& cmd)
{
	cerr << "Error: " << msg("Syntax error near: %s", cmd) << endl;
}

void e2db_cli::shell_command_version()
{
	version(true);
}

void e2db_cli::shell_resolver(COMMAND command, istream* is)
{
	string src;
	*is >> std::skipws >> src;

	if (command == COMMAND::usage)
	{
		string hint;
		*is >> std::skipws >> hint;

		if (hint.empty())
			shell_usage(COMMAND::usage);
		else if (hint == "read")
			shell_usage(COMMAND::fread);
		else if (hint == "write")
			shell_usage(COMMAND::fwrite);
		else if (hint == "list")
			shell_usage(COMMAND::list);
		else if (hint == "add")
			shell_usage(COMMAND::add);
		else if (hint == "edit")
			shell_usage(COMMAND::edit);
		else if (hint == "remove")
			shell_usage(COMMAND::remove);
		else if (hint == "copy")
			shell_usage(COMMAND::copy);
		else if (hint == "move")
			shell_usage(COMMAND::move);
		else if (hint == "set")
			shell_usage(COMMAND::set);
		else if (hint == "unset")
			shell_usage(COMMAND::unset);
		else if (hint == "import")
			shell_usage(COMMAND::fimport);
		else if (hint == "export")
			shell_usage(COMMAND::fexport);
		else if (hint == "merge")
			shell_usage(COMMAND::merge);
		else if (hint == "print")
			shell_usage(COMMAND::print);
		else if (hint == "parse")
			shell_usage(COMMAND::parse);
		else if (hint == "make")
			shell_usage(COMMAND::make);
		else if (hint == "convert")
			shell_usage(COMMAND::convert);
		else if (hint == "debug")
			shell_usage(COMMAND::debug);
		else if (hint == "preferences")
			shell_usage(COMMAND::preferences);
		else
		{
			cerr << "Type Error: " << msg("Unknown usage type: %s", hint) << endl;
			cout << endl;

			shell_usage(COMMAND::usage);
		}
	}
	else if (command == COMMAND::fread)
	{
		string path;
		*is >> std::skipws >> path;

		if (path.empty())
			shell_usage(command);
		else
			shell_file_read(path);
	}
	else if (command == COMMAND::fwrite)
	{
		string path;
		*is >> std::skipws >> path;

		shell_file_write(path);
	}
	else if (command == COMMAND::list)
	{
		string type, bname;
		int offset0 = -1;
		int offset1 = -1;

		*is >> std::skipws >> type;

		if (type == "channels")
			*is >> std::skipws >> bname >> offset0 >> offset1;
		else
			*is >> std::skipws >> offset0 >> offset1;

		if (type == "services")
			shell_entry_list(ENTRY::service, offset0, offset1);
		else if (type == "transponders")
			shell_entry_list(ENTRY::transponder, offset0, offset1);
		else if (type == "bouquets")
			shell_entry_list(ENTRY::bouquet, offset0, offset1);
		else if (type == "userbouquets")
			shell_entry_list(ENTRY::userbouquet, offset0, offset1);
		else if (type == "tunersets_transponders")
			shell_entry_list(ENTRY::tunersets_transponder, offset0, offset1);
		else if (type == "tunersets_tables")
			shell_entry_list(ENTRY::tunersets_table, offset0, offset1);
		else if (type == "tunersets")
			shell_entry_list(ENTRY::tunersets, offset0, offset1);
		else if (type == "channels")
			shell_entry_list(ENTRY::channel_reference, bname, offset0, offset1);
		else if (type.empty())
			shell_usage(command);
		else
			cerr << "Type Error: " << msg("Unknown entry type: %s", type) << endl;
	}
	else if (command == COMMAND::add)
	{
		string type, bname;
		*is >> std::skipws >> type >> bname;

		if (type == "service")
			shell_entry_add(ENTRY::service);
		else if (type == "transponder")
			shell_entry_add(ENTRY::transponder);
		else if (type == "bouquet")
			shell_entry_add(ENTRY::bouquet);
		else if (type == "userbouquet")
			shell_entry_add(ENTRY::userbouquet);
		else if (type == "tunersets_transponder")
			shell_entry_add(ENTRY::tunersets_transponder);
		else if (type == "tunersets_table")
			shell_entry_add(ENTRY::tunersets_table);
		else if (type == "tunersets")
			shell_entry_add(ENTRY::tunersets);
		else if (type == "channel")
			shell_entry_add(ENTRY::channel_reference, 1, bname);
		else if (type == "marker")
			shell_entry_add(ENTRY::channel_reference, 0, bname);
		else if (type == "stream")
			shell_entry_add(ENTRY::channel_reference, 2, bname);
		else if (type.empty())
			shell_usage(command);
		else
			cerr << "Type Error" << msg("Unknown entry type: %s", type) << endl;
	}
	else if (command == COMMAND::edit)
	{
		string type, id, bname;
		*is >> std::skipws >> type >> id >> bname;

		if (type == "service")
			shell_entry_edit(ENTRY::service, id);
		else if (type == "transponder")
			shell_entry_edit(ENTRY::transponder, id);
		else if (type == "bouquet")
			shell_entry_edit(ENTRY::bouquet, id);
		else if (type == "userbouquet")
			shell_entry_edit(ENTRY::userbouquet, id);
		else if (type == "tunersets_transponder")
			shell_entry_edit(ENTRY::tunersets_transponder, id);
		else if (type == "tunersets_table")
			shell_entry_edit(ENTRY::tunersets_table, id);
		else if (type == "tunersets")
			shell_entry_edit(ENTRY::tunersets, id);
		else if (type == "channel")
			shell_entry_edit(ENTRY::channel_reference, 1, bname, id);
		else if (type == "marker")
			shell_entry_edit(ENTRY::channel_reference, 0, bname, id);
		else if (type == "stream")
			shell_entry_edit(ENTRY::channel_reference, 2, bname, id);
		else if (type.empty())
			shell_usage(command);
		else
			cerr << "Type Error: " << msg("Unknown entry type: %s", type) << endl;
	}
	else if (command == COMMAND::remove)
	{
		string type, id, bname;
		*is >> std::skipws >> type >> id >> bname;

		if (type == "service")
			shell_entry_remove(ENTRY::service, id);
		else if (type == "transponder")
			shell_entry_remove(ENTRY::transponder, id);
		else if (type == "bouquet")
			shell_entry_remove(ENTRY::bouquet, id);
		else if (type == "userbouquet")
			shell_entry_remove(ENTRY::tunersets_transponder, id);
		else if (type == "tunersets_transponder")
			shell_entry_remove(ENTRY::tunersets_transponder, id);
		else if (type == "tunersets_table")
			shell_entry_remove(ENTRY::tunersets_table, id);
		else if (type == "tunersets")
			shell_entry_remove(ENTRY::tunersets, id);
		else if (type == "channel")
			shell_entry_remove(ENTRY::channel_reference, 1, bname, id);
		else if (type == "marker")
			shell_entry_remove(ENTRY::channel_reference, 0, bname, id);
		else if (type.empty())
			shell_usage(command);
		else
			cerr << "Type Error: " << msg("Unknown entry type: %s", type) << endl;
	}
	else if (command == COMMAND::set)
	{
		string type, id;
		*is >> std::skipws >> type >> id;

		if (type == "service")
			shell_entry_parentallock(ENTRY::service, id, true);
		else if (type == "userbouquet")
			shell_entry_parentallock(ENTRY::userbouquet, id, true);
		else if (type.empty())
			shell_usage(command);
		else
			cerr << "Type Error: " << msg("Unknown entry type: %s", type) << endl;
	}
	else if (command == COMMAND::unset)
	{
		string type, id;
		*is >> std::skipws >> type >> id;

		if (type == "service")
			shell_entry_parentallock(ENTRY::service, id, false);
		else if (type == "userbouquet")
			shell_entry_parentallock(ENTRY::userbouquet, id, false);
		else if (type.empty())
			shell_usage(command);
		else
			cerr << "Type Error: " << msg("Unknown entry type: %s", type) << endl;
	}
	else if (command == COMMAND::fimport)
	{
		string opt0, opt1, optx;
		string type;
		vector<string> paths;
		int ver = -1;
		*is >> std::skipws >> opt0 >> opt1;

		if (opt0 == "zapit" || opt0 == "parentallock")
			type = opt0 + ' ' + opt1;
		else
			type = opt0, paths.emplace_back(opt1);

		while (std::getline(*is, optx, ' '))
			paths.emplace_back(optx);
		try { int ver = std::stoi(optx); paths.pop_back(); }
		catch (...) {}

		if (type == "enigma")
			shell_e2db_import(ENTRY::lamedb, paths);
		else if (type == "neutrino")
			shell_e2db_import(ENTRY::zapit, paths);
		else if (type == "lamedb")
			shell_e2db_import(ENTRY::lamedb_services, paths, ver);
		else if (type == "bouquets")
			shell_e2db_import(ENTRY::bouquet, paths);
		else if (type == "userbouquets")
			shell_e2db_import(ENTRY::userbouquet, paths);
		else if (type == "zapit services")
			shell_e2db_import(ENTRY::zapit_services, paths, ver);
		else if (type == "zapit bouquets")
			shell_e2db_import(ENTRY::zapit_bouquets, paths, ver);
		else if (type == "tunersets")
			shell_e2db_import(ENTRY::tunersets, paths);
		else if (type == "parentallock blacklist")
			shell_e2db_import(ENTRY::parentallock_blacklist, paths);
		else if (type == "parentallock whitelist")
			shell_e2db_import(ENTRY::parentallock_whitelist, paths);
		else if (type == "parentallock locked")
			shell_e2db_import(ENTRY::parentallock_locked, paths);
		else if (type.empty())
			shell_usage(command);
		else
			cerr << "Type Error: " << msg("Unknown entry type: %s", type) << endl;
	}
	else if (command == COMMAND::fexport)
	{
		string opt0, opt1, optx;
		string type, bname;
		vector<string> paths;
		int ver = -1;
		*is >> std::skipws >> opt0 >> opt1;

		if (opt0 == "zapit" || (opt0 == "parentallock" && (opt1 == "locked" || opt1 == "blacklist" || opt1 == "whitelist")))
			type = opt0 + ' ' + opt1;
		else if (opt0 == "bouquet" || opt0 == "userbouquet")
			type = opt0, bname = opt1;
		else
			type = opt0, paths.emplace_back(opt1);

		while (std::getline(*is, optx, ' '))
			paths.emplace_back(optx);
		try { int ver = std::stoi(optx); paths.pop_back(); }
		catch (...) {}

		if (type == "enigma")
			shell_e2db_export(ENTRY::lamedb, paths, -1, true);
		else if (type == "neutrino")
			shell_e2db_export(ENTRY::zapit, paths, -1, true);
		else if (type == "lamedb")
			shell_e2db_export(ENTRY::lamedb_services, paths, ver);
		else if (type == "bouquet")
			shell_e2db_export(ENTRY::bouquet, paths, ver, false, bname);
		else if (type == "bouquets")
			shell_e2db_export(ENTRY::bouquet, paths, ver, true);
		else if (type == "userbouquet")
			shell_e2db_export(ENTRY::userbouquet, paths, -1, false, bname);
		else if (type == "userbouquets")
			shell_e2db_export(ENTRY::userbouquet, paths, -1, true);
		else if (type == "zapit services")
			shell_e2db_export(ENTRY::zapit_services, paths, ver);
		else if (type == "zapit bouquets")
			shell_e2db_export(ENTRY::zapit_bouquets, paths, ver);
		else if (type == "tunersets")
			shell_e2db_export(ENTRY::tunersets, paths);
		else if (type == "parentallock blacklist")
			shell_e2db_export(ENTRY::parentallock_blacklist, paths);
		else if (type == "parentallock whitelist")
			shell_e2db_export(ENTRY::parentallock_whitelist, paths);
		else if (type == "parentallock locked")
			shell_e2db_export(ENTRY::parentallock_locked, paths);
		else if (type == "parentallock")
			shell_e2db_export(ENTRY::parentallock, paths);
		else if (type.empty())
			shell_usage(command);
		else
			cerr << "Type Error: " << msg("Unknown entry type: %s", type) << endl;
	}
	else if (command == COMMAND::merge)
	{
		string opt0, opt1, opt2, opt3;
		string type, path, bname0, bname1;
		int ver = -1;
		*is >> std::skipws >> opt0 >> opt1 >> opt2 >> opt3;

		if (opt0 == "zapit" || opt0 == "parentallock" && ! opt1.empty())
		{
			type = opt0 + ' ' + opt1, path = opt2;
			try { ver = std::stoi(opt3); } catch (...) {}
		}
		else if (opt0 == "bouquets" || opt0 == "userbouquets" && ! opt2.empty())
		{
			type = opt0, bname0 = opt1, bname1 = opt2;
			try { ver = std::stoi(opt3); } catch (...) {}
		}
		else
		{
			type = opt0, path = opt1;
			try { ver = std::stoi(opt2); } catch (...) {}
		}

		if (type == "enigma")
			shell_e2db_merge(ENTRY::lamedb, path, -1, true);
		else if (type == "neutrino")
			shell_e2db_merge(ENTRY::zapit, path, -1, true);
		else if (type == "lamedb")
			shell_e2db_merge(ENTRY::lamedb_services, path, ver);
		else if (type == "bouquet")
			shell_e2db_merge(ENTRY::bouquet, path, ver);
		else if (type == "bouquets" && path.empty())
			shell_e2db_merge(ENTRY::bouquet, path, ver, true);
		else if (type == "bouquets")
			shell_e2db_merge(ENTRY::bouquet, ver, bname0, bname1);
		else if (type == "userbouquet")
			shell_e2db_merge(ENTRY::userbouquet, path);
		else if (type == "userbouquets" && path.empty())
			shell_e2db_merge(ENTRY::userbouquet, -1, bname0, bname1);
		else if (type == "userbouquets")
			shell_e2db_merge(ENTRY::userbouquet, path, -1, true);
		else if (type == "zapit services")
			shell_e2db_merge(ENTRY::zapit_services, path, ver);
		else if (type == "zapit bouquets")
			shell_e2db_merge(ENTRY::zapit_bouquets, path, ver);
		else if (type == "tunersets")
			shell_e2db_merge(ENTRY::tunersets, path);
		else if (type == "parentallock blacklist")
			shell_e2db_merge(ENTRY::parentallock_blacklist, path);
		else if (type == "parentallock whitelist")
			shell_e2db_merge(ENTRY::parentallock_whitelist, path);
		else if (type == "parentallock locked")
			shell_e2db_merge(ENTRY::parentallock_locked, path);
		else if (type.empty())
			shell_usage(command);
		else
			cerr << "Type Error: " << msg("Unknown entry type: %s", type) << endl;
	}
	else if (command == COMMAND::print)
	{
		string type;
		*is >> std::skipws >> type;

		if (type == "debug")
			shell_print(0);
		else if (type == "index")
			shell_print(1);
		else if (type.empty())
			shell_usage(command);
		else
			cerr << "Type Error" << msg("Unknown print type: %s", type) << endl;
	}
	else if (command == COMMAND::parse)
	{
		string opt0, opt1, opt2, opt3;
		string type, path;
		int ver = -1;
		*is >> std::skipws >> opt0 >> opt1 >> opt2 >> opt3;

		if (opt0 == "zapit" || opt0 == "parentallock" && ! opt1.empty())
		{
			type = opt0 + ' ' + opt1, path = opt2;
			try { ver = std::stoi(opt3); } catch (...) {}
		}
		else
		{
			type = opt0, path = opt1;
			try { ver = std::stoi(opt2); } catch (...) {}
		}

		if (type == "enigma")
			shell_e2db_parse(ENTRY::lamedb, path, -1, true);
		else if (type == "neutrino")
			shell_e2db_parse(ENTRY::zapit, path, -1, true);
		else if (type == "lamedb")
			shell_e2db_parse(ENTRY::lamedb_services, path, ver);
		else if (type == "bouquet")
			shell_e2db_parse(ENTRY::bouquet, path);
		else if (type == "userbouquet")
			shell_e2db_parse(ENTRY::userbouquet, path);
		else if (type == "zapit services")
			shell_e2db_parse(ENTRY::zapit_services, path, ver);
		else if (type == "zapit bouquets")
			shell_e2db_parse(ENTRY::zapit_bouquets, path, ver);
		else if (type == "tunersets")
			shell_e2db_parse(ENTRY::tunersets, path);
		else if (type == "parentallock blacklist")
			shell_e2db_parse(ENTRY::parentallock_blacklist, path);
		else if (type == "parentallock whitelist")
			shell_e2db_parse(ENTRY::parentallock_whitelist, path);
		else if (type == "parentallock locked")
			shell_e2db_parse(ENTRY::parentallock_locked, path);
		else if (type.empty())
			shell_usage(command);
		else
			cerr << "Type Error: " << msg("Unknown entry type: %s", type) << endl;
	}
	else if (command == COMMAND::make)
	{
		string opt0, opt1, opt2, opt3;
		string type, path, bname;
		int ver = -1;
		*is >> std::skipws >> opt0 >> opt1 >> opt2 >> opt3;

		if (opt0 == "zapit" || opt0 == "parentallock" && ! opt1.empty())
		{
			type = opt0 + ' ' + opt1, path = opt2;
			try { ver = std::stoi(opt3); } catch (...) {}
		}
		else if (opt0 == "bouquet" || opt0 == "userbouquet")
		{
			type = opt0, path = opt1, bname = opt2;
			try { ver = std::stoi(opt3); } catch (...) {}
		}
		else
		{
			type = opt0, path = opt1;
			try { ver = std::stoi(opt2); } catch (...) {}
		}

		if (type == "enigma")
			shell_e2db_make(ENTRY::lamedb, path, ver, true);
		else if (type == "neutrino")
			shell_e2db_make(ENTRY::zapit, path, ver, true);
		else if (type == "lamedb")
			shell_e2db_make(ENTRY::lamedb_services, path, ver);
		else if (type == "bouquet")
			shell_e2db_make(ENTRY::bouquet, path, ver, false, bname);
		else if (type == "bouquets")
			shell_e2db_make(ENTRY::bouquet, path, ver, true);
		else if (type == "userbouquet")
			shell_e2db_make(ENTRY::userbouquet, path, ver, false, bname);
		else if (type == "userbouquets")
			shell_e2db_make(ENTRY::userbouquet, path, ver, true);
		else if (type == "zapit services")
			shell_e2db_make(ENTRY::zapit_services, path, ver);
		else if (type == "zapit bouquets")
			shell_e2db_make(ENTRY::zapit_bouquets, path, ver);
		else if (type == "tunersets")
			shell_e2db_make(ENTRY::tunersets, path);
		else if (type == "parentallock blacklist")
			shell_e2db_make(ENTRY::parentallock_blacklist, path);
		else if (type == "parentallock whitelist")
			shell_e2db_make(ENTRY::parentallock_whitelist, path);
		else if (type == "parentallock locked")
			shell_e2db_make(ENTRY::parentallock_locked, path);
		else if (type == "parentallock")
			shell_e2db_make(ENTRY::parentallock, path);
		else if (type.empty())
			shell_usage(command);
		else
			cerr << "Type Error: " << msg("Unknown entry type: %s", type) << endl;
	}
	else if (command == COMMAND::convert)
	{
		string opt0, opt1, opt2, opt3, opt4;
		string type, path, bname;
		int stype = -1, ytype = -1;
		int fopt = -1, ftype = 0;
		*is >> std::skipws >> opt0 >> opt1 >> opt2 >> opt3 >> opt4;

		type = opt2;

		if (opt0 == "from")
			fopt = 0;
		else if (opt1 == "to")
			fopt = 1;
		if (opt1 == "csv")
			ftype = 1;
		else if (opt1 == "m3u")
			ftype = 2;
		else if (opt1 == "html")
			ftype = 3;

		if (opt2 == "services" && ! opt3.empty())
		{
			path = opt4;
			try { stype = std::stoi(opt3); } catch (...) {}
		}
		else if (opt2 == "bouquet" || opt2 == "userbouquet")
		{
			path = opt4, bname = opt3;
		}
		else if (opt2 == "tunersets" && ! opt3.empty())
		{
			path = opt4;
			try { ytype = std::stoi(opt3); } catch (...) {}
		}
		else
		{
			path = opt3;
		}

		if (type == "index")
			shell_e2db_convert(ENTRY::index, fopt, ftype, path);
		else if (type == "all")
			shell_e2db_convert(ENTRY::all, fopt, ftype, path);
		else if (type == "services")
			shell_e2db_convert(ENTRY::service, fopt, ftype, path, "", stype);
		else if (type == "bouquets")
			shell_e2db_convert(ENTRY::bouquet, fopt, ftype, path);
		else if (type == "bouquet")
			shell_e2db_convert(ENTRY::bouquet, fopt, ftype, path, bname);
		else if (type == "userbouquets")
			shell_e2db_convert(ENTRY::userbouquet, fopt, ftype, path);
		else if (type == "userbouquet")
			shell_e2db_convert(ENTRY::userbouquet, fopt, ftype, path, bname);
		else if (type == "tunersets")
			shell_e2db_convert(ENTRY::tunersets, fopt, ftype, path, "", -1, ytype);
		else if (type.empty())
			shell_e2db_convert(ENTRY::all, fopt, ftype, path);
		else
			cerr << "Type Error: " << msg("Unknown entry type: %s", type) << endl;
	}
	else if (command == COMMAND::debug)
	{
		shell_debug();
	}
	else if (command == COMMAND::preferences)
	{
		string type, opt0;
		*is >> std::skipws >> type >> opt0;
		OBJIO format = OBJIO::tabular;

		if (! opt0.empty())
		{
			if (opt0 == "tabular")
				format = OBJIO::tabular;
			else if (opt0 == "byline")
				format = OBJIO::byline;
			else if (opt0 == "json")
				format = OBJIO::json;
		}

		if (type == "output")
			shell_preference_output(format);
		else
			cerr << "Type Error: " << msg("Unknown entry type: %s", type) << endl;
	}
	else
	{
		cerr << "Error: " << msg("Unknown command") << endl;
	}
}

void e2db_cli::shell_usage(COMMAND hint, bool specs)
{
	if (hint == COMMAND::usage)
	{
		shell_usage(COMMAND::add, false);
		shell_usage(COMMAND::edit, false);
		shell_usage(COMMAND::remove, false);
		shell_usage(COMMAND::list, false);
		shell_usage(COMMAND::set, false);
		shell_usage(COMMAND::unset, false);
		shell_usage(COMMAND::fread, false);
		shell_usage(COMMAND::fwrite, false);
		shell_usage(COMMAND::print, false);
		shell_usage(COMMAND::debug, false);
		shell_usage(COMMAND::preferences, false);

		// cout << "  ", cout.width(7), cout << left << "history", cout << ' ';
		// cout.width(24), cout << left << "file", cout << ' ' << "Save history to file, instead of memory." << endl;
		// cout << endl;

		cout << "  ", cout.width(32), cout << left << "version", cout << ' ' << "Display version." << endl;
		cout << endl;

		cout << "  ", cout.width(32), cout << left << "help", cout << ' ' << "Display usage hints." << endl;
		cout.width(10), cout << ' ', cout << "list" << endl;
		cout.width(10), cout << ' ', cout << "import" << endl;
		cout.width(10), cout << ' ', cout << "export" << endl;
		cout.width(10), cout << ' ', cout << "merge" << endl;
		cout.width(10), cout << ' ', cout << "parse" << endl;
		cout.width(10), cout << ' ', cout << "make" << endl;
		cout.width(10), cout << ' ', cout << "convert" << endl;
		cout.width(10), cout << ' ', cout << "preferences" << endl;
	}
	else if (hint == COMMAND::add)
	{
		cout << "  ", cout.width(7), cout << left << "add", cout << ' ';
		cout.width(24), cout << left << "transponder", cout << ' ' << "Add new entry." << endl;
		cout.width(10), cout << ' ', cout << "service" << endl;
		cout.width(10), cout << ' ', cout << "bouquet" << endl;
		cout.width(10), cout << ' ', cout << "userbouquet" << endl;
		cout.width(10), cout << ' ', cout << "tunersets" << endl;
		cout.width(10), cout << ' ', cout << "tunersets-table" << endl;
		cout.width(10), cout << ' ', cout << "tunersets-transponder" << endl;
		cout.width(10), cout << ' ', cout.width(24), cout << left << "channel  [chid] [bname]", cout << ' ' << "Add service reference to userbouquet." << endl;
		cout.width(10), cout << ' ', cout.width(24), cout << left << "marker  [bname]", cout << ' ' << "Add marker to userbouquet." << endl;
		cout.width(10), cout << ' ', cout.width(24), cout << left << "stream  [bname]", cout << ' ' << "Add stream to userbouquet." << endl;
		cout << endl;
	}
	else if (hint == COMMAND::edit)
	{
		cout << "  ", cout.width(7), cout << left << "edit", cout << ' ';
		cout.width(24), cout << left << "transponder  [txid]", cout << ' ' << "Edit an entry." << endl;
		cout.width(10), cout << ' ', cout << "service  [txid]" << endl;
		cout.width(10), cout << ' ', cout << "bouquet  [bname]" << endl;
		cout.width(10), cout << ' ', cout << "userbouquet  [bname]" << endl;
		cout.width(10), cout << ' ', cout << "tunersets  [ytype]" << endl;
		cout.width(10), cout << ' ', cout << "tunersets-table  [tnid]" << endl;
		cout.width(10), cout << ' ', cout << "tunersets-transponder  [trid]" << endl;
		cout.width(10), cout << ' ', cout.width(24), cout << left << "marker  [chid] [bname]", cout << ' ' << "Edit marker from userbouquet." << endl;
		cout << endl;
	}
	else if (hint == COMMAND::remove)
	{
		cout << "  ", cout.width(7), cout << left << "remove", cout << ' ';
		cout.width(24), cout << left << "transponder  [txid]", cout << ' ' << "Remove an entry." << endl;
		cout.width(10), cout << ' ', cout << "service  [txid]" << endl;
		cout.width(10), cout << ' ', cout << "bouquet  [bname]" << endl;
		cout.width(10), cout << ' ', cout << "userbouquet  [bname]" << endl;
		cout.width(10), cout << ' ', cout << "tunersets  [ytype]" << endl;
		cout.width(10), cout << ' ', cout << "tunersets-table  [tnid]" << endl;
		cout.width(10), cout << ' ', cout << "tunersets-transponder  [trid]" << endl;
		cout.width(10), cout << ' ', cout.width(24), cout << left << "channel  [chid] [bname]", cout << ' ' << "Remove service reference from userbouquet." << endl;
		cout.width(10), cout << ' ', cout.width(24), cout << left << "marker  [chid] [bname]", cout << ' ' << "Remove marker from userbouquet." << endl;
		cout.width(10), cout << ' ', cout.width(24), cout << left << "stream  [chid] [bname]", cout << ' ' << "Remove stream from userbouquet." << endl;
		cout << endl;
	}
	else if (hint == COMMAND::list)
	{
		cout << "  ", cout.width(7), cout << left << "list", cout << ' ';
		cout.width(24), cout << left << "transponders", cout << ' ' << "List entries." << endl;
		cout.width(10), cout << ' ', cout << "services" << endl;
		cout.width(10), cout << ' ', cout << "bouquets" << endl;
		cout.width(10), cout << ' ', cout << "userbouquets" << endl;
		cout.width(10), cout << ' ', cout << "tunersets" << endl;
		cout.width(10), cout << ' ', cout << "tunersets-tables" << endl;
		cout.width(10), cout << ' ', cout << "tunersets-transponders" << endl;
		cout.width(10), cout << ' ', cout.width(24), cout << left << "channels  [bname]", cout << ' ' << "List channels from userbouquet." << endl;
		cout << endl;

		if (specs)
		{
			cout << "  ", cout.width(7), cout << left << "list", cout << ' ';
			cout.width(24), cout << left << "[ENTRY] [limit]", cout << ' ' << "Set the number of items to display per page." << endl;
			cout.width(10), cout << ' ', cout.width(24), cout << left << "[ENTRY] [start] [limit]";
			cout << endl << endl;
			cout << "  ", cout << "EXAMPLE" << endl << endl;
			cout << "  ", cout.width(24), cout << left << "list services 15", cout << ' ' << "Displays 15 items per page." << endl;
			cout << "  ", cout.width(24), cout << left << "list services 20 15", cout << ' ' << "Displays 15 items per page starting from 20." << endl;
			cout << "  ", cout.width(24), cout << left << "list services 0", cout << ' ' << "Turn off pagination." << endl;
			cout << endl;
		}
	}
	else if (hint == COMMAND::set)
	{
		cout << "  ", cout.width(7), cout << left << "set", cout << ' ';
		cout.width(39), cout << left << "parentallock service  [chid]", cout << ' ' << "Set parental lock." << endl;
		cout.width(10), cout << ' ', cout << "parentallock userbouquet  [bname]" << endl;
		cout << endl;
	}
	else if (hint == COMMAND::unset)
	{
		cout << "  ", cout.width(7), cout << left << "unset", cout << ' ';
		cout.width(39), cout << left << "parentallock service  [chid]", cout << ' ' << "Unset parental lock." << endl;
		cout.width(10), cout << ' ', cout << "parentallock userbouquet  [bname]" << endl;
		cout << endl;
	}
	else if (hint == COMMAND::fread)
	{
		cout << "  ", cout.width(7), cout << left << "read", cout << ' ';
		cout.width(24), cout << left << "[directory]", cout << ' ' << "Read from directory file." << endl;
		cout << endl;
	}
	else if (hint == COMMAND::fwrite)
	{
		cout << "  ", cout.width(7), cout << left << "write", cout << ' ';
		cout.width(24), cout << left << "[directory]", cout << ' ' << "Write to directory file." << endl;
		cout << endl;
	}
	else if (hint == COMMAND::print)
	{
		cout << "  ", cout.width(7), cout << left << "print", cout << ' ';
		cout.width(24), cout << left << "debug", cout << ' ' << "Print debug informations." << endl;
		cout.width(10), cout << ' ', cout << "index" << endl;
		cout << endl;
	}
	else if (hint == COMMAND::fimport)
	{
		cout << "  ", cout.width(7), cout << left << "import", cout << ' ';
		cout.width(24), cout << left << "enigma  [directory]", cout << ' ' << "Import Enigma directory." << endl;
		cout.width(10), cout << ' ', cout.width(24), cout << left << "neutrino  [directory]", cout << ' ' << "Import Neutrino directory." << endl;
		cout.width(10), cout << ' ', cout.width(24), cout << left << "lamedb  [files]", cout << ' ' << "Import Lamedb services files." << endl;
		cout.width(10), cout << ' ', cout.width(24), cout << left << "bouquets  [files]", cout << ' ' << "Import Enigma bouquet files." << endl;
		cout.width(10), cout << ' ', cout.width(24), cout << left << "userbouquets  [files]", cout << ' ' << "Import Enigma userbouquet files." << endl;
		cout.width(10), cout << ' ', cout.width(24), cout << left << "zapit services  [files]", cout << ' ' << "Import Neutrino services files." << endl;
		cout.width(10), cout << ' ', cout.width(24), cout << left << "zapit bouquets  [files]", cout << ' ' << "Import Neutrino bouquets files." << endl;
		cout.width(10), cout << ' ', cout.width(24), cout << left << "tunersets  [files]", cout << ' ' << "Import tuner settings xml files." << endl;
		cout.width(10), cout << ' ', cout.width(39), cout << left << "parentallock blacklist  [file]", cout << ' ' << "Import Enigma blacklist parental lock file." << endl;
		cout.width(10), cout << ' ', cout.width(39), cout << left << "parentallock whitelist  [file]", cout << ' ' << "Import Enigma whitelist parental lock file." << endl;
		cout.width(10), cout << ' ', cout.width(39), cout << left << "parentallock locked  [file]", cout << ' ' << "Import Enigma .locked parental lock file." << endl;
		cout << endl;

		if (specs)
		{
			cout << "  ", cout.width(7), cout << left << "import", cout << ' ';
			cout.width(24), cout << left << "[ENTRY] [files] [version]", cout << ' ' << "Specific version (eg. Lamedb, Neutrino api)." << endl;
			cout << endl;
		}
	}
	else if (hint == COMMAND::fexport)
	{
		cout << "  ", cout.width(7), cout << left << "export", cout << ' ';
		cout.width(24), cout << left << "enigma  [directory]", cout << ' ' << "Export Enigma files to directory." << endl;
		cout.width(10), cout << ' ', cout.width(24), cout << left << "neutrino  [directory]", cout << ' ' << "Export Neutrino files to directory." << endl;
		cout.width(10), cout << ' ', cout.width(24), cout << left << "lamedb  [file]", cout << ' ' << "Export Lamedb services files." << endl;
		cout.width(10), cout << ' ', cout.width(24), cout << left << "bouquet  [bname] [file]", cout << ' ' << "Export Enigma bouquet file." << endl;
		cout.width(10), cout << ' ', cout.width(24), cout << left << "bouquets  [directory]", cout << ' ' << "Export Enigma bouquet files to directory." << endl;
		cout.width(10), cout << ' ', cout.width(39), cout << left << "userbouquet  [bname] [file]", cout << ' ' << "Export Enigma userbouquet files." << endl;
		cout.width(10), cout << ' ', cout.width(39), cout << left << "userbouquets  [directory]", cout << ' ' << "Export Enigma userbouquet files to directory." << endl;
		cout.width(10), cout << ' ', cout.width(24), cout << left << "zapit services  [files]", cout << ' ' << "Export Neutrino services files." << endl;
		cout.width(10), cout << ' ', cout.width(24), cout << left << "zapit bouquets  [files]", cout << ' ' << "Export Neutrino bouquets files." << endl;
		cout.width(10), cout << ' ', cout.width(24), cout << left << "tunersets  [file]", cout << ' ' << "Export tuner settings xml file." << endl;
		cout.width(10), cout << ' ', cout.width(24), cout << left << "tunersets  [directory]", cout << ' ' << "Export tuner settings xml files to directory." << endl;
		cout.width(10), cout << ' ', cout.width(39), cout << left << "parentallock blacklist  [file]", cout << ' ' << "Export Enigma blacklist parental lock file." << endl;
		cout.width(10), cout << ' ', cout.width(39), cout << left << "parentallock whitelist  [file]", cout << ' ' << "Export Enigma whitelist parental lock file." << endl;
		cout.width(10), cout << ' ', cout.width(39), cout << left << "parentallock locked  [file]", cout << ' ' << "Export Enigma .locked parental lock file." << endl;
		cout.width(10), cout << ' ', cout.width(24), cout << left << "parentallock  [directory]", cout << ' ' << "Export Enigma parental lock files to directory." << endl;
		cout << endl;

		if (specs)
		{
			cout << "  ", cout.width(7), cout << left << "export", cout << ' ';
			cout.width(24), cout << left << "[ENTRY] [...] [version]", cout << ' ' << "Specific version (eg. Lamedb, Neutrino api)." << endl;
			cout << endl;
		}
	}
	else if (hint == COMMAND::merge)
	{
		cout << "  ", cout.width(7), cout << left << "merge", cout << ' ';
		cout.width(24), cout << left << "enigma  [directory]", cout << ' ' << "Merge with Enigma directory." << endl;
		cout.width(10), cout << ' ', cout.width(24), cout << left << "neutrino  [directory]", cout << ' ' << "Merge with Neutrino directory." << endl;
		cout.width(10), cout << ' ', cout.width(24), cout << left << "lamedb  [file]", cout << ' ' << "Merge with Lamedb services file." << endl;
		cout.width(10), cout << ' ', cout.width(24), cout << left << "bouquet  [file]", cout << ' ' << "Merge with Enigma bouquet file." << endl;
		cout.width(10), cout << ' ', cout.width(24), cout << left << "bouquets  [bname] [bname]", cout << ' ' << "Merge two Enigma bouquet." << endl;
		cout.width(10), cout << ' ', cout.width(24), cout << left << "bouquets  [directory]", cout << ' ' << "Merge Enigma bouquet files from directory." << endl;
		cout.width(10), cout << ' ', cout.width(24), cout << left << "userbouquet  [file]", cout << ' ' << "Merge with Enigma userbouquet file." << endl;
		cout.width(10), cout << ' ', cout.width(24), cout << left << "userbouquets  [bname] [bname]", cout << ' ' << "Merge two Enigma userbouquet." << endl;
		cout.width(10), cout << ' ', cout.width(24), cout << left << "userbouquets  [directory]", cout << ' ' << "Merge Enigma userbouquet files from directory." << endl;
		cout.width(10), cout << ' ', cout.width(24), cout << left << "zapit services  [file]", cout << ' ' << "Merge with Neutrino services file." << endl;
		cout.width(10), cout << ' ', cout.width(24), cout << left << "zapit bouquets  [file]", cout << ' ' << "Merge with Neutrino bouquets file." << endl;
		cout.width(10), cout << ' ', cout.width(24), cout << left << "tunersets  [file]", cout << ' ' << "Merge with tuner settings xml file." << endl;
		cout.width(10), cout << ' ', cout.width(24), cout << left << "tunersets  [directory]", cout << ' ' << "Merge tuner settings xml files from directory." << endl;
		cout.width(10), cout << ' ', cout.width(39), cout << left << "parentallock blacklist  [file]", cout << ' ' << "Merge with Enigma blacklist parental lock file." << endl;
		cout.width(10), cout << ' ', cout.width(39), cout << left << "parentallock whitelist  [file]", cout << ' ' << "Merge with Enigma whitelist parental lock file." << endl;
		cout.width(10), cout << ' ', cout.width(39), cout << left << "parentallock locked  [file]", cout << ' ' << "Merge with Enigma .locked parental lock file." << endl;
		cout << endl;

		if (specs)
		{
			cout << "  ", cout.width(7), cout << left << "merge", cout << ' ';
			cout.width(24), cout << left << "[ENTRY] [file] [version]", cout << ' ' << "Specific version (eg. Lamedb, Neutrino api)." << endl;
			cout << endl;
		}
	}
	else if (hint == COMMAND::parse)
	{
		cout << "  ", cout.width(7), cout << left << "parse", cout << ' ';
		cout.width(24), cout << left << "enigma  [directory]", cout << ' ' << "Parse Enigma directory." << endl;
		cout.width(10), cout << ' ', cout.width(24), cout << left << "neutrino  [directory]", cout << ' ' << "Parse Neutrino directory." << endl;
		cout.width(10), cout << ' ', cout.width(24), cout << left << "lamedb  [file]", cout << ' ' << "Parse Lamedb services file." << endl;
		cout.width(10), cout << ' ', cout.width(24), cout << left << "bouquet  [file]", cout << ' ' << "Parse Enigma bouquet file." << endl;
		cout.width(10), cout << ' ', cout.width(24), cout << left << "userbouquet  [file]", cout << ' ' << "Parse Enigma userbouquet file." << endl;
		cout.width(10), cout << ' ', cout.width(24), cout << left << "zapit services  [file]", cout << ' ' << "Parse Neutrino services file." << endl;
		cout.width(10), cout << ' ', cout.width(24), cout << left << "zapit bouquets  [file]", cout << ' ' << "Parse Neutrino bouquets file." << endl;
		cout.width(10), cout << ' ', cout.width(24), cout << left << "tunersets  [file]", cout << ' ' << "Parse tuner settings xml file." << endl;
		cout.width(10), cout << ' ', cout.width(39), cout << left << "parentallock blacklist  [file]", cout << ' ' << "Parse Enigma blacklist parental lock file." << endl;
		cout.width(10), cout << ' ', cout.width(39), cout << left << "parentallock whitelist  [file]", cout << ' ' << "Parse Enigma whitelist parental lock file." << endl;
		cout.width(10), cout << ' ', cout.width(39), cout << left << "parentallock locked  [file]", cout << ' ' << "Parse Enigma .locked parental lock file." << endl;
		cout << endl;

		if (specs)
		{
			cout << "  ", cout.width(7), cout << left << "parse", cout << ' ';
			cout.width(24), cout << left << "[ENTRY] [file] [version]", cout << ' ' << "Specific version (eg. Lamedb, Neutrino api)." << endl;
			cout << endl;
		}
	}
	else if (hint == COMMAND::make)
	{
		cout << "  ", cout.width(7), cout << left << "make", cout << ' ';
		cout.width(24), cout << left << "enigma  [directory]", cout << ' ' << "Make Enigma files to directory." << endl;
		cout.width(10), cout << ' ', cout.width(24), cout << left << "neutrino  [directory]", cout << ' ' << "Make Neutrino files to directory." << endl;
		cout.width(10), cout << ' ', cout.width(24), cout << left << "lamedb  [file]", cout << ' ' << "Make Lamedb services file." << endl;
		cout.width(10), cout << ' ', cout.width(24), cout << left << "bouquet  [bname] [file]", cout << ' ' << "Make Enigma bouquet file." << endl;
		cout.width(10), cout << ' ', cout.width(24), cout << left << "bouquets  [directory]", cout << ' ' << "Make Enigma bouquet files to directory." << endl;
		cout.width(10), cout << ' ', cout.width(39), cout << left << "userbouquet  [bname] [file]", cout << ' ' << "Make Enigma userbouquet file." << endl;
		cout.width(10), cout << ' ', cout.width(39), cout << left << "userbouquets  [directory]", cout << ' ' << "Make Enigma userbouquet files to directory." << endl;
		cout.width(10), cout << ' ', cout.width(24), cout << left << "zapit services  [file]", cout << ' ' << "Make Neutrino services file." << endl;
		cout.width(10), cout << ' ', cout.width(24), cout << left << "zapit bouquets  [file]", cout << ' ' << "Make Neutrino bouquets file." << endl;
		cout.width(10), cout << ' ', cout.width(24), cout << left << "tunersets  [file]", cout << ' ' << "Make tuner settings xml file." << endl;
		cout.width(10), cout << ' ', cout.width(24), cout << left << "tunersets  [directory]", cout << ' ' << "Make tuner settings xml files." << endl;
		cout.width(10), cout << ' ', cout.width(39), cout << left << "parentallock blacklist  [file]", cout << ' ' << "Make Enigma blacklist parental lock file." << endl;
		cout.width(10), cout << ' ', cout.width(39), cout << left << "parentallock whitelist  [file]", cout << ' ' << "Make Enigma whitelist parental lock file." << endl;
		cout.width(10), cout << ' ', cout.width(39), cout << left << "parentallock locked  [file]", cout << ' ' << "Make Enigma .locked parental lock file." << endl;
		cout.width(10), cout << ' ', cout.width(24), cout << left << "parentallock  [directory]", cout << ' ' << "Make Enigma parental lock files to directory." << endl;
		cout << endl;

		if (specs)
		{
			cout << "  ", cout.width(7), cout << left << "make", cout << ' ';
			cout.width(24), cout << left << "[ENTRY] [...] [version]", cout << ' ' << "Specific version (eg. Lamedb, Neutrino api)." << endl;
			cout << endl;
		}
	}
	else if (hint == COMMAND::convert)
	{
		cout << "  ", cout.width(7), cout << left << "convert", cout << ' ';
		cout.width(39), cout << left << "from csv services  [file]", cout << ' ' << "Convert from CSV services file." << endl;
		cout.width(10), cout << ' ', cout.width(39), cout << left << "from csv bouquets  [file]", cout << ' ' << "Convert from CSV bouquets file." << endl;
		cout.width(10), cout << ' ', cout.width(39), cout << left << "from csv userbouquets  [file]", cout << ' ' << "Convert from CSV userbouquets file." << endl;
		cout.width(10), cout << ' ', cout.width(39), cout << left << "from csv tunersets  [file]", cout << ' ' << "Convert from CSV tuner settings file." << endl;
		cout << endl;
		cout << "  ", cout.width(7), cout << left << "convert", cout << ' ';
		cout.width(39), cout << left << "to csv  [file]", cout << ' ' << "Convert all the entries to CSV file." << endl;
		cout.width(10), cout << ' ', cout.width(39), cout << left << "to csv services  [file]", cout << ' ' << "Convert to CSV services file." << endl;
		cout.width(10), cout << ' ', cout.width(39), cout << left << "to csv services  [stype] [file]", cout << ' ' << "Convert services of type to CSV services file." << endl;
		cout.width(10), cout << ' ', cout.width(39), cout << left << "to csv bouquets  [file]", cout << ' ' << "Convert to CSV bouquets file." << endl;
		cout.width(10), cout << ' ', cout.width(39), cout << left << "to csv bouquet  [bname] [file]", cout << ' ' << "Convert a bouquet to CSV bouquet file." << endl;
		cout.width(10), cout << ' ', cout.width(39), cout << left << "to csv userbouquets  [file]", cout << ' ' << "Convert to CSV userbouquets file." << endl;
		cout.width(10), cout << ' ', cout.width(39), cout << left << "to csv userbouquet  [bname] [file]", cout << ' ' << "Convert an userbouquet to CSV userbouquet file." << endl;
		cout.width(10), cout << ' ', cout.width(39), cout << left << "to csv tunersets  [file]", cout << ' ' << "Convert to CSV tuner settings file." << endl;
		cout.width(10), cout << ' ', cout.width(39), cout << left << "to csv tunersets  [ytype] [file]", cout << ' ' << "Convert a tuner settings to CSV tuner settings file." << endl;
		cout << endl;
		cout << "  ", cout.width(7), cout << left << "convert", cout << ' ';
		cout.width(39), cout << left << "from m3u  [file]", cout << ' ' << "Convert from M3U file." << endl;
		cout << endl;
		cout << "  ", cout.width(7), cout << left << "convert", cout << ' ';
		cout.width(39), cout << left << "to m3u  [file]", cout << ' ' << "Convert entries to M3U file." << endl;
		cout.width(10), cout << ' ', cout.width(39), cout << left << "to m3u userbouquets  [file]", cout << ' ' << "Convert userbouquets to M3U file." << endl;
		cout.width(10), cout << ' ', cout.width(39), cout << left << "to m3u userbouquet  [bname] [file]", cout << ' ' << "Convert an userbouquet to M3U file." << endl;
		cout << endl;
		cout << "  ", cout.width(7), cout << left << "convert", cout << ' ';
		cout.width(39), cout << left << "to html  [file]", cout << ' ' << "Convert all the entries to HTML files." << endl;
		cout.width(10), cout << ' ', cout.width(39), cout << left << "to html index  [file]", cout << ' ' << "Convert to HTML index of contents files." << endl;
		cout.width(10), cout << ' ', cout.width(39), cout << left << "to html services  [file]", cout << ' ' << "Convert to HTML services files." << endl;
		cout.width(10), cout << ' ', cout.width(39), cout << left << "to html services  [stype] [file]", cout << ' ' << "Convert services of type to HTML services files." << endl;
		cout.width(10), cout << ' ', cout.width(39), cout << left << "to html bouquets  [file]", cout << ' ' << "Convert to HTML bouquets files." << endl;
		cout.width(10), cout << ' ', cout.width(39), cout << left << "to html bouquet  [bname] [file]", cout << ' ' << "Convert a bouquet to HTML bouquets files." << endl;
		cout.width(10), cout << ' ', cout.width(39), cout << left << "to html userbouquets  [file]", cout << ' ' << "Convert to HTML userbouquets files." << endl;
		cout.width(10), cout << ' ', cout.width(39), cout << left << "to html userbouquet  [bname] [file]", cout << ' ' << "Convert an userbouquet to HTML userbouquets files." << endl;
		cout.width(10), cout << ' ', cout.width(39), cout << left << "to html tunersets  [file]", cout << ' ' << "Convert to HTML tuner settings files." << endl;
		cout.width(10), cout << ' ', cout.width(39), cout << left << "to html tunersets  [ytype] [file]", cout << ' ' << "Convert a tuner settings to HTML tuner settings files." << endl;
		cout << endl;
	}
	else if (hint == COMMAND::debug)
	{
		cout << "  ", cout.width(32), cout << left << "debug", cout << ' ' << "Display debug info." << endl;
		cout << endl;
	}
	else if (hint == COMMAND::preferences)
	{
		cout << "  ", cout.width(32), cout << left << "preferences", cout << ' ' << "CLI preferences." << endl;
		cout << endl;

		if (specs)
		{
			cout << "  ", cout.width(7), cout << left << "preferences", cout << ' ';
			cout.width(39), cout << left << "output  [format]", cout << ' ' << "Output format (tabular, byline, json)" << endl;
			cout << endl;
		}
	}
}

void e2db_cli::shell_file_read(string path)
{
	try
	{
		if (dbih->read(path))
			cout << "Info: " << msg("File read: %s", path) << endl;
	}
	catch (const std::invalid_argument& err)
	{
		cerr << "Error: " << msg(MSG::except_invalid_argument, err.what()) << endl;
	}
	catch (const std::out_of_range& err)
	{
		cerr << "Error: " << msg(MSG::except_out_of_range, err.what()) << endl;
	}
	catch (const std::filesystem::filesystem_error& err)
	{
		cerr << "Error: " << msg(MSG::except_filesystem, err.what()) << endl;
	}
	catch (const std::runtime_error& err)
	{
		cerr << "Error: " << err.what() << endl;
	}
	catch (...)
	{
		cerr << "Error: " << msg(MSG::except_uncaught) << endl;
	}
}

void e2db_cli::shell_file_write(string path)
{
	try
	{
		if (dbih->write(path))
			cout << "Info: " << msg("File written: %s", path) << endl;
	}
	catch (const std::invalid_argument& err)
	{
		cerr << "Error: " << msg(MSG::except_invalid_argument, err.what()) << endl;
	}
	catch (const std::out_of_range& err)
	{
		cerr << "Error: " << msg(MSG::except_out_of_range, err.what()) << endl;
	}
	catch (const std::filesystem::filesystem_error& err)
	{
		cerr << "Error: " << msg(MSG::except_filesystem, err.what()) << endl;
	}
	catch (const std::runtime_error& err)
	{
		cerr << "Error: " << err.what() << endl;
	}
	catch (...)
	{
		cerr << "Error: " << msg(MSG::except_uncaught) << endl;
	}
}

void e2db_cli::shell_e2db_parse(ENTRY entry_type, string path, int ver, bool dir)
{
	if (path.empty())
	{
		cerr << "Error: " << msg("Wrong parameter path.") << endl;

		return;
	}

	try
	{
		if (! std::filesystem::exists(path))
			throw std::runtime_error (msg("File \"%s\" not exists.", path));
		if (! std::filesystem::is_regular_file(path))
			throw std::runtime_error (msg("File \"%s\" is not a valid file.", path));
		if (dir && ! std::filesystem::is_directory(path))
			throw std::runtime_error (msg("File \"%s\" is not a valid directory.", path));
		if
		(
			(std::filesystem::status(path).permissions() & std::filesystem::perms::owner_read) == std::filesystem::perms::none &&
			(std::filesystem::status(path).permissions() & std::filesystem::perms::group_read) == std::filesystem::perms::none
		)
			throw std::runtime_error (msg("File \"%s\" is not readable.", path));

		string filename = std::filesystem::path(path).filename().u8string();

		if (entry_type == ENTRY::lamedb || entry_type == ENTRY::zapit)
		{
			bool merge = dbih->get_input().size() != 0 ? true : false;
			auto* dst = merge ? new e2db : dbih;

			try
			{
				bool read = dst->read(path);

				if (! read)
				{
					if (merge) delete dst;
					throw std::runtime_error (msg("Error reading file."));
				}
				if (merge)
				{
					dbih->merge(dst);
					delete dst;
				}
			}
			catch (...)
			{
				if (merge) delete dst;
				throw;
			}
		}
		else if (entry_type == ENTRY::lamedb_services)
		{
			ifstream iservices (path);
			try
			{
				if (ver == 5)
					dbih->parse_e2db_lamedb5(iservices);
				else if (ver > 0 && ver < 5)
					dbih->parse_e2db_lamedbx(iservices, ver);
				else
					dbih->parse_e2db_lamedb(iservices);
			}
			catch (...)
			{
				iservices.close();
				throw;
			}
			iservices.close();
		}
		else if (entry_type == ENTRY::bouquet)
		{
			string fext = filename.substr(filename.rfind('.') + 1);

			if (fext != "tv" && fext != "radio" && fext != "epl")
				throw std::runtime_error (msg("Unknown Bouquet file."));

			ifstream ibouquet (path);
			try
			{
				dbih->parse_e2db_bouquet(ibouquet, filename, fext == "epl");
			}
			catch (...)
			{
				ibouquet.close();
				throw;
			}
			ibouquet.close();
		}
		else if (entry_type == ENTRY::userbouquet)
		{
			ifstream iuserbouquet (path);
			try
			{
				dbih->parse_e2db_userbouquet(iuserbouquet, filename);
			}
			catch (...)
			{
				iuserbouquet.close();
				throw;
			}
			iuserbouquet.close();
		}
		else if (entry_type == ENTRY::zapit_services)
		{
			ifstream iservicesxml (path);
			try
			{
				if (ver != -1)
					dbih->parse_zapit_services_apix_xml(iservicesxml, filename, ver);
				else
					dbih->parse_zapit_services_xml(iservicesxml, filename);
			}
			catch (...)
			{
				iservicesxml.close();
				throw;
			}
			iservicesxml.close();
		}
		else if (entry_type == ENTRY::zapit_bouquets)
		{
			ver = ver != -1 ? ver : dbih->get_zapit_version();

			ifstream ibouquetsxml (path);
			try
			{
				dbih->parse_zapit_bouquets_apix_xml(ibouquetsxml, filename, ver);
			}
			catch (...)
			{
				ibouquetsxml.close();
				throw;
			}
			ibouquetsxml.close();
		}
		else if (entry_type == ENTRY::tunersets)
		{
			e2db::YTYPE ytype = e2db::YTYPE::satellite;

			if (filename == "satellites.xml")
				ytype = e2db::YTYPE::satellite;
			else if (filename == "terrestrial.xml")
				ytype = e2db::YTYPE::terrestrial;
			else if (filename == "cables.xml")
				ytype = e2db::YTYPE::cable;
			else if (filename == "atsc.xml")
				ytype = e2db::YTYPE::atsc;
			else
				throw std::runtime_error (msg("Unknown Tuner settings type."));

			ifstream itunxml (path);
			try
			{
				dbih->parse_tunersets_xml(ytype, itunxml);
			}
			catch (...)
			{
				itunxml.close();
				throw;
			}
			itunxml.close();
		}
		else if (entry_type == ENTRY::parentallock_locked)
		{
			ifstream iparental (path);
			try
			{
				dbih->parse_e2db_parentallock_list(e2db::PARENTALLOCK::locked, iparental);
			}
			catch (...)
			{
				iparental.close();
				throw;
			}
			iparental.close();
		}
		else if (entry_type == ENTRY::parentallock_blacklist)
		{
			ifstream iparental (path);
			try
			{
				dbih->parse_e2db_parentallock_list(e2db::PARENTALLOCK::blacklist, iparental);
			}
			catch (...)
			{
				iparental.close();
				throw;
			}
			iparental.close();
		}
		else if (entry_type == ENTRY::parentallock_whitelist)
		{
			ifstream iparental (path);
			try
			{
				dbih->parse_e2db_parentallock_list(e2db::PARENTALLOCK::whitelist, iparental);
			}
			catch (...)
			{
				iparental.close();
				throw;
			}
			iparental.close();
		}
	}
	catch (const std::invalid_argument& err)
	{
		cerr << "Error: " << msg(MSG::except_invalid_argument, err.what()) << endl;
	}
	catch (const std::out_of_range& err)
	{
		cerr << "Error: " << msg(MSG::except_out_of_range, err.what()) << endl;
	}
	catch (const std::filesystem::filesystem_error& err)
	{
		cerr << "Error: " << msg(MSG::except_filesystem, err.what()) << endl;
	}
	catch (const std::runtime_error& err)
	{
		cerr << "Error: " << err.what() << endl;
	}
	catch (...)
	{
		cerr << "Error: " << msg(MSG::except_uncaught) << endl;
	}
}

void e2db_cli::shell_e2db_make(ENTRY entry_type, string path, int ver, bool dir, string bname)
{
	if (path.empty())
	{
		cerr << "Error: " << msg("Wrong parameter path.") << endl;

		return;
	}

	try
	{
		ver = ver != -1 ? ver : 4;

		//TODO local overwrite

		bool overwrite = e2db::OVERWRITE_FILE;

		if (! overwrite && std::filesystem::exists(path))
			throw std::runtime_error (msg("File \"%s\" already exists.", path));
		if (! std::filesystem::is_regular_file(path))
			throw std::runtime_error (msg("File \"%s\" is not a valid file.", path));
		if (dir && ! std::filesystem::is_directory(path))
			throw std::runtime_error (msg("File \"%s\" is not a valid directory.", path));
		if (
			(std::filesystem::status(path).permissions() & std::filesystem::perms::owner_write) == std::filesystem::perms::none &&
			(std::filesystem::status(path).permissions() & std::filesystem::perms::group_write) == std::filesystem::perms::none
		)
			throw std::runtime_error (msg("File \"%s\" is not writable.", path));

		string filename = std::filesystem::path(path).filename().u8string();

		if (entry_type == ENTRY::lamedb || entry_type == ENTRY::zapit)
		{
			int srctype = dbih->get_e2db_services_type();
			int lamedb_ver = dbih->get_lamedb_version();
			int zapit_ver = dbih->get_zapit_version();

			if (entry_type == ENTRY::lamedb)
			{
				dbih->set_e2db_services_type(0);
				dbih->set_lamedb_version(ver);
			}
			else if (entry_type == ENTRY::zapit)
			{
				dbih->set_e2db_services_type(1);
				dbih->set_zapit_version(ver);
			}

			bool write = dbih->write(path);

			dbih->set_e2db_services_type(srctype);
			dbih->set_lamedb_version(lamedb_ver);
			dbih->set_zapit_version(zapit_ver);

			if (! write)
				throw std::runtime_error (msg("Error writing file."));
		}
		else if (entry_type == ENTRY::lamedb_services)
		{
			e2db::e2db_file file;

			dbih->make_lamedb(filename, file, ver);

			ofstream out (path);
			out << file.data;
			out.close();
		}
		else if (entry_type == ENTRY::bouquet)
		{
			vector<string> bouquets;

			if (dir)
			{
				for (auto & x : dbih->bouquets)
					bouquets.emplace_back(x.first);
			}
			else if (dbih->bouquets.count(bname))
			{
				bouquets.emplace_back(bname);
			}
			else
			{
				throw std::runtime_error (msg("Bouquet \"%s\" not exists.", bname));
			}

			for (string & bname : bouquets)
			{
				e2db::e2db_file file;

				string fext = filename.substr(filename.rfind('.') + 1);

				if (fext == "epl" || ver < 3)
					dbih->make_bouquet_epl(filename, file, ver);
				else
					dbih->make_bouquet(filename, file, ver);

				ofstream out (path);
				out << file.data;
				out.close();
			}
		}
		else if (entry_type == ENTRY::userbouquet)
		{
			vector<string> userbouquets;

			if (dir)
			{
				for (auto & x : dbih->userbouquets)
					userbouquets.emplace_back(x.first);
			}
			else if (dbih->userbouquets.count(bname))
			{
				userbouquets.emplace_back(bname);
			}
			else
			{
				throw std::runtime_error (msg("Userbouquet \"%s\" not exists.", bname));
			}

			for (string & bname : userbouquets)
			{
				e2db::e2db_file file;

				dbih->make_userbouquet(bname, file, ver);

				ofstream out (path);
				out << file.data;
				out.close();
			}
		}
		else if (entry_type == ENTRY::zapit_services)
		{
			e2db::e2db_file file;

			dbih->make_bouquets_xml(filename, file, ver);

			ofstream out (path);
			out << file.data;
			out.close();
		}
		else if (entry_type == ENTRY::zapit_bouquets)
		{
			e2db::e2db_file file;

			dbih->make_bouquets_xml(filename, file, ver);

			ofstream out (path);
			out << file.data;
			out.close();
		}
		else if (entry_type == ENTRY::tunersets)
		{
			vector<string> files;

			if (dir)
			{
				for (auto & x : dbih->tuners)
				{
					if (dbih->get_zapit_version() != -1 && x.first != e2db::YTYPE::satellite)
						continue;

					string filename;

					switch (x.first)
					{
						case e2db::YTYPE::satellite:
							filename = "satellites.xml";
						break;
						case e2db::YTYPE::terrestrial:
							filename = "terrestrial.xml";
						break;
						case e2db::YTYPE::cable:
							filename = "cables.xml";
						break;
						case e2db::YTYPE::atsc:
							filename = "atsc.xml";
						break;
					}

					files.emplace_back(filename);
				}
			}
			else
			{
				files.emplace_back(filename);
			}

			for (string & filename : files)
			{
				e2db::e2db_file file;

				e2db::YTYPE ytype = e2db::YTYPE::satellite;

				if (filename == "satellites.xml")
					ytype = e2db::YTYPE::satellite;
				else if (filename == "terrestrial.xml")
					ytype = e2db::YTYPE::terrestrial;
				else if (filename == "cables.xml")
					ytype = e2db::YTYPE::cable;
				else if (filename == "atsc.xml")
					ytype = e2db::YTYPE::atsc;
				else
					throw std::runtime_error (msg("Unknown Tuner settings type."));

				dbih->make_tunersets_xml(filename, ytype, file);

				ofstream out (path);
				out << file.data;
				out.close();
			}
		}
		else if (entry_type == ENTRY::parentallock_locked)
		{
			e2db::e2db_file file;

			dbih->make_parentallock_list(filename, e2db::PARENTALLOCK::locked, file);

			ofstream out (path);
			out << file.data;
			out.close();
		}
		else if (entry_type == ENTRY::parentallock_blacklist)
		{
			e2db::e2db_file file;

			dbih->make_parentallock_list(filename, e2db::PARENTALLOCK::blacklist, file);

			ofstream out (path);
			out << file.data;
			out.close();
		}
		else if (entry_type == ENTRY::parentallock_whitelist)
		{
			e2db::e2db_file file;

			dbih->make_parentallock_list(filename, e2db::PARENTALLOCK::whitelist, file);

			ofstream out (path);
			out << file.data;
			out.close();
		}
		else if (entry_type == ENTRY::parentallock)
		{
			e2db::e2db_file file;

			if (ver < 3)
			{
				filename = "services.locked";

				dbih->make_parentallock_list(filename, e2db::PARENTALLOCK::locked, file);

				ofstream out (path);
				out << file.data;
				out.close();
			}
			else
			{
				filename = dbih->db.parental ? "whitelist" : "blacklist";

				dbih->make_parentallock_list(filename, dbih->db.parental, file);

				ofstream out (path);
				out << file.data;
				out.close();

				{
					filename = dbih->db.parental ? "blacklist" : "whitelist";

					e2db::e2db_file empty;
					empty.filename = filename;
					empty.mime = "text/plain";
					empty.size = 0;

					ofstream out (path);
					out << empty.data;
					out.close();
				}
			}
		}
	}
	catch (const std::invalid_argument& err)
	{
		cerr << "Error: " << msg(MSG::except_invalid_argument, err.what()) << endl;
	}
	catch (const std::out_of_range& err)
	{
		cerr << "Error: " << msg(MSG::except_out_of_range, err.what()) << endl;
	}
	catch (const std::filesystem::filesystem_error& err)
	{
		cerr << "Error: " << msg(MSG::except_filesystem, err.what()) << endl;
	}
	catch (const std::runtime_error& err)
	{
		cerr << "Error: " << err.what() << endl;
	}
	catch (...)
	{
		cerr << "Error: " << msg(MSG::except_uncaught) << endl;
	}
}

void e2db_cli::shell_e2db_convert(ENTRY entry_type, int fopt, int ftype, string path, string bname, int stype, int ytype)
{
	if (path.empty())
	{
		cerr << "Error: " << msg("Wrong parameter path.") << endl;

		return;
	}

	try
	{
		// from html raise
		if (fopt == 0 && ftype == 3)
			throw 1;

		//TODO local overwrite

		string filename = std::filesystem::path(path).filename().u8string();

		e2db::FCONVS fcx = e2db::FCONVS::convert_current;

		e2db::fcopts opts;
		opts.filename = filename;

		switch (entry_type)
		{
			case ENTRY::index:
				fcx = e2db::FCONVS::convert_index;
			break;
			case ENTRY::all:
				fcx = e2db::FCONVS::convert_all;
			break;
			case ENTRY::service:
				if (stype != -1)
					opts.stype = stype;
				else
					fcx = e2db::FCONVS::convert_services;
			break;
			case ENTRY::bouquet:
				if (! bname.empty())
					opts.bname = bname;
				else
					fcx = e2db::FCONVS::convert_bouquets;
			break;
			case ENTRY::userbouquet:
				if (! bname.empty())
					opts.bname = bname;
				else
					fcx = e2db::FCONVS::convert_userbouquets;
			break;
			case ENTRY::tunersets:
				if (ytype != -1)
					opts.ytype = ytype;
				else
					fcx = e2db::FCONVS::convert_tunersets;
			break; 
			default:
				throw 1;
		}

		if (ftype == 1)
		{
			if (fopt == 0)
				dbih->import_csv_file(fcx, opts, path);
			else if (fopt == 1)
				dbih->export_csv_file(fcx, opts, path);
		}
		else if (ftype == 2)
		{
			//TODO pass m3u options
			opts.flags = opts.bname.empty() ? 0xf: 0x1f;

			if (fopt == 0)
				dbih->import_m3u_file(fcx, opts, path);
			else if (fopt == 1)
				dbih->export_m3u_file(fcx, opts, path);
		}
		else if (ftype == 3)
		{
			dbih->export_html_file(fcx, opts, path);
		}
	}
	catch (const std::invalid_argument& err)
	{
		cerr << "Error: " << msg(MSG::except_invalid_argument, err.what()) << endl;
	}
	catch (const std::out_of_range& err)
	{
		cerr << "Error: " << msg(MSG::except_out_of_range, err.what()) << endl;
	}
	catch (const std::filesystem::filesystem_error& err)
	{
		cerr << "Error: " << msg(MSG::except_filesystem, err.what()) << endl;
	}
	catch (const std::runtime_error& err)
	{
		cerr << "Error: " << err.what() << endl;
	}
	catch (...)
	{
		cerr << "Error: " << msg(MSG::except_uncaught) << endl;
	}
}

void e2db_cli::shell_e2db_merge(ENTRY entry_type, string path, int ver, bool dir)
{
	if (path.empty())
	{
		cerr << "Error: " << msg("Wrong parameter path.") << endl;

		return;
	}

	try
	{
		if (! std::filesystem::is_regular_file(path))
			throw std::runtime_error (msg("File \"%s\" is not a valid file.", path));
		if (dir && ! std::filesystem::is_directory(path))
			throw std::runtime_error (msg("File \"%s\" is not a valid directory.", path));

		e2db::FPORTS fpi = e2db::FPORTS::unknown;

		switch (entry_type)
		{
			case ENTRY::lamedb:
			case ENTRY::zapit:
				fpi = e2db::FPORTS::directory;
			break;
			case ENTRY::lamedb_services:
				if (ver == 5)
					fpi = e2db::FPORTS::all_services__2_5;
				else if (ver == 4)
					fpi = e2db::FPORTS::all_services__2_4;
				else if (ver == 3)
					fpi = e2db::FPORTS::all_services__2_3;
				else if (ver == 2)
					fpi = e2db::FPORTS::all_services__2_2;
				else
					fpi = e2db::FPORTS::all_services;
			break;
			case ENTRY::bouquet:
				if (dir && ver < 3)
					fpi = e2db::FPORTS::all_bouquets_epl;
				else if (dir)
					fpi = e2db::FPORTS::all_bouquets;
				else if (ver < 3)
					fpi = e2db::FPORTS::single_bouquet_epl;
				else
					fpi = e2db::FPORTS::single_bouquet;
			break;
			case ENTRY::userbouquet:
				if (dir)
					fpi = e2db::FPORTS::all_userbouquets;
				else
					fpi = e2db::FPORTS::single_userbouquet;
			break;
			case ENTRY::zapit_services:
				if (ver == 4)
					fpi = e2db::FPORTS::all_services_xml__4;
				else if (ver == 3)
					fpi = e2db::FPORTS::all_services_xml__3;
				else if (ver == 2)
					fpi = e2db::FPORTS::all_services_xml__2;
				else if (ver == 1)
					fpi = e2db::FPORTS::all_services_xml__1;
				else
					fpi = e2db::FPORTS::all_services_xml;
			break;
			case ENTRY::zapit_bouquets:
				if (ver == 4)
					fpi = e2db::FPORTS::all_bouquets_xml__4;
				else if (ver == 3)
					fpi = e2db::FPORTS::all_bouquets_xml__3;
				else if (ver == 2)
					fpi = e2db::FPORTS::all_bouquets_xml__2;
				else if (ver == 1)
					fpi = e2db::FPORTS::all_bouquets_xml__1;
				else
					fpi = e2db::FPORTS::all_bouquets_xml;
			break;
			case ENTRY::tunersets:
				if (dir)
					fpi = e2db::FPORTS::all_tunersets;
				else
					fpi = e2db::FPORTS::single_tunersets;
			break;
			case ENTRY::parentallock_blacklist:
				fpi = e2db::FPORTS::single_parentallock_blacklist;
			break;
			case ENTRY::parentallock_whitelist:
				fpi = e2db::FPORTS::single_parentallock_whitelist;
			break;
			case ENTRY::parentallock_locked:
				fpi = e2db::FPORTS::single_parentallock_locked;
			break;
			default:
			break;
		}

		dbih->import_file(fpi, { path });
	}
	catch (const std::invalid_argument& err)
	{
		cerr << "Error: " << msg(MSG::except_invalid_argument, err.what()) << endl;
	}
	catch (const std::out_of_range& err)
	{
		cerr << "Error: " << msg(MSG::except_out_of_range, err.what()) << endl;
	}
	catch (const std::filesystem::filesystem_error& err)
	{
		cerr << "Error: " << msg(MSG::except_filesystem, err.what()) << endl;
	}
	catch (const std::runtime_error& err)
	{
		cerr << "Error: " << err.what() << endl;
	}
	catch (...)
	{
		cerr << "Error: " << msg(MSG::except_uncaught) << endl;
	}
}

//TODO
void e2db_cli::shell_e2db_merge(ENTRY entry_type, int ver, string bname0, string bname1)
{
	try
	{
		if (entry_type == ENTRY::bouquet)
		{
		}
		else if (entry_type == ENTRY::userbouquet)
		{
		}

		cout << "TODO" << endl;
	}
	catch (const std::invalid_argument& err)
	{
		cerr << "Error: " << msg(MSG::except_invalid_argument, err.what()) << endl;
	}
	catch (const std::out_of_range& err)
	{
		cerr << "Error: " << msg(MSG::except_out_of_range, err.what()) << endl;
	}
	catch (const std::filesystem::filesystem_error& err)
	{
		cerr << "Error: " << msg(MSG::except_filesystem, err.what()) << endl;
	}
	catch (const std::runtime_error& err)
	{
		cerr << "Error: " << err.what() << endl;
	}
	catch (...)
	{
		cerr << "Error: " << msg(MSG::except_uncaught) << endl;
	}
}

void e2db_cli::shell_e2db_import(ENTRY entry_type, vector<string> paths, int ver, bool dir)
{
	if (paths.size() == 0)
	{
		cerr << "Error: " << msg("Wrong parameter paths.") << endl;

		return;
	}

	try
	{
		for (string & path : paths)
		{
			if (! std::filesystem::is_regular_file(path))
				throw std::runtime_error (msg("File \"%s\" is not a valid file.", path));
			if (dir && ! std::filesystem::is_directory(path))
				throw std::runtime_error (msg("File \"%s\" is not a valid directory.", path));
		}

		e2db::FPORTS fpi = e2db::FPORTS::unknown;

		switch (entry_type)
		{
			case ENTRY::lamedb:
			case ENTRY::zapit:
				fpi = e2db::FPORTS::directory;
			break;
			case ENTRY::lamedb_services:
				if (ver == 5)
					fpi = e2db::FPORTS::all_services__2_5;
				else if (ver == 4)
					fpi = e2db::FPORTS::all_services__2_4;
				else if (ver == 3)
					fpi = e2db::FPORTS::all_services__2_3;
				else if (ver == 2)
					fpi = e2db::FPORTS::all_services__2_2;
				else
					fpi = e2db::FPORTS::all_services;
			break;
			case ENTRY::bouquet:
				if (ver < 3)
					fpi = e2db::FPORTS::all_bouquets_epl;
				else
					fpi = e2db::FPORTS::all_bouquets;
			break;
			case ENTRY::userbouquet:
				fpi = e2db::FPORTS::all_userbouquets;
			break;
			case ENTRY::zapit_services:
				if (ver == 4)
					fpi = e2db::FPORTS::all_services_xml__4;
				else if (ver == 3)
					fpi = e2db::FPORTS::all_services_xml__3;
				else if (ver == 2)
					fpi = e2db::FPORTS::all_services_xml__2;
				else if (ver == 1)
					fpi = e2db::FPORTS::all_services_xml__1;
				else
					fpi = e2db::FPORTS::all_services_xml;
			break;
			case ENTRY::zapit_bouquets:
				if (ver == 4)
					fpi = e2db::FPORTS::all_bouquets_xml__4;
				else if (ver == 3)
					fpi = e2db::FPORTS::all_bouquets_xml__3;
				else if (ver == 2)
					fpi = e2db::FPORTS::all_bouquets_xml__2;
				else if (ver == 1)
					fpi = e2db::FPORTS::all_bouquets_xml__1;
				else
					fpi = e2db::FPORTS::all_bouquets_xml;
			break;
			case ENTRY::tunersets:
				fpi = e2db::FPORTS::all_tunersets;
			break;
			case ENTRY::parentallock_blacklist:
				fpi = e2db::FPORTS::single_parentallock_blacklist;
			break;
			case ENTRY::parentallock_whitelist:
				fpi = e2db::FPORTS::single_parentallock_whitelist;
			break;
			case ENTRY::parentallock_locked:
				fpi = e2db::FPORTS::single_parentallock_locked;
			break;
			default:
			break;
		}

		dbih->import_file(fpi, paths);
	}
	catch (const std::invalid_argument& err)
	{
		cerr << "Error: " << msg(MSG::except_invalid_argument, err.what()) << endl;
	}
	catch (const std::out_of_range& err)
	{
		cerr << "Error: " << msg(MSG::except_out_of_range, err.what()) << endl;
	}
	catch (const std::filesystem::filesystem_error& err)
	{
		cerr << "Error: " << msg(MSG::except_filesystem, err.what()) << endl;
	}
	catch (const std::runtime_error& err)
	{
		cerr << "Error: " << err.what() << endl;
	}
	catch (...)
	{
		cerr << "Error: " << msg(MSG::except_uncaught) << endl;
	}
}

void e2db_cli::shell_e2db_export(ENTRY entry_type, vector<string> paths, int ver, bool dir, string bname)
{
	if (paths.size() == 0)
	{
		cerr << "Error: " << msg("Wrong parameter paths.") << endl;

		return;
	}

	try
	{
		for (string & path : paths)
		{
			if (! std::filesystem::is_regular_file(path))
				throw std::runtime_error (msg("File \"%s\" is not a valid file.", path));
			if (dir && ! std::filesystem::is_directory(path))
				throw std::runtime_error (msg("File \"%s\" is not a valid directory.", path));
		}

		e2db::FPORTS fpo = e2db::FPORTS::unknown;

		switch (entry_type)
		{
			case ENTRY::lamedb:
			case ENTRY::zapit:
				{
					fpo = e2db::FPORTS::directory;

					int lamedb_ver = dbih->get_lamedb_version();
					int zapit_ver = dbih->get_zapit_version();
					ver = ver != -1 ? ver : 4;

					if (entry_type == ENTRY::lamedb)
					{
						dbih->set_lamedb_version(ver);
						dbih->set_zapit_version(-1);
					}
					else if (entry_type == ENTRY::zapit)
					{
						dbih->set_lamedb_version(-1);
						dbih->set_zapit_version(ver);
					}

					dbih->export_file(fpo, paths);

					dbih->set_lamedb_version(lamedb_ver);
					dbih->set_zapit_version(zapit_ver);
				}
			return;
			case ENTRY::lamedb_services:
				if (ver == 5)
					fpo = e2db::FPORTS::all_services__2_5;
				else if (ver == 4)
					fpo = e2db::FPORTS::all_services__2_4;
				else if (ver == 3)
					fpo = e2db::FPORTS::all_services__2_3;
				else if (ver == 2)
					fpo = e2db::FPORTS::all_services__2_2;
				else
					fpo = e2db::FPORTS::all_services;
			break;
			case ENTRY::bouquet:
				if (dir && ver < 3)
					fpo = e2db::FPORTS::all_bouquets_epl;
				else if (dir)
					fpo = e2db::FPORTS::all_bouquets;
				else if (ver < 3)
					fpo = e2db::FPORTS::single_bouquet_epl;
				else
					fpo = e2db::FPORTS::single_bouquet;
			break;
			case ENTRY::userbouquet:
				if (dir)
					fpo = e2db::FPORTS::all_userbouquets;
				else
					fpo = e2db::FPORTS::single_userbouquet;
			break;
			case ENTRY::zapit_services:
				if (ver == 4)
					fpo = e2db::FPORTS::all_services_xml__4;
				else if (ver == 3)
					fpo = e2db::FPORTS::all_services_xml__3;
				else if (ver == 2)
					fpo = e2db::FPORTS::all_services_xml__2;
				else if (ver == 1)
					fpo = e2db::FPORTS::all_services_xml__1;
				else
					fpo = e2db::FPORTS::all_services_xml;
			break;
			case ENTRY::zapit_bouquets:
				if (ver == 4)
					fpo = e2db::FPORTS::all_bouquets_xml__4;
				else if (ver == 3)
					fpo = e2db::FPORTS::all_bouquets_xml__3;
				else if (ver == 2)
					fpo = e2db::FPORTS::all_bouquets_xml__2;
				else if (ver == 1)
					fpo = e2db::FPORTS::all_bouquets_xml__1;
				else
					fpo = e2db::FPORTS::all_bouquets_xml;
			break;
			case ENTRY::tunersets:
				for (string & path : paths)
				{
					if (std::filesystem::is_directory(path))
					{
						dir = true;
					}
					else
					{
						dir = false;
						break;
					}
				}
				if (dir)
					fpo = e2db::FPORTS::all_tunersets;
				else
					fpo = e2db::FPORTS::single_tunersets;
			break;
			case ENTRY::parentallock_blacklist:
				fpo = e2db::FPORTS::single_parentallock_blacklist;
			break;
			case ENTRY::parentallock_whitelist:
				fpo = e2db::FPORTS::single_parentallock_whitelist;
			break;
			case ENTRY::parentallock_locked:
				fpo = e2db::FPORTS::single_parentallock_locked;
			break;
			default:
			break;
		}

		dbih->export_file(fpo, paths);
	}
	catch (const std::invalid_argument& err)
	{
		cerr << "Error: " << msg(MSG::except_invalid_argument, err.what()) << endl;
	}
	catch (const std::out_of_range& err)
	{
		cerr << "Error: " << msg(MSG::except_out_of_range, err.what()) << endl;
	}
	catch (const std::filesystem::filesystem_error& err)
	{
		cerr << "Error: " << msg(MSG::except_filesystem, err.what()) << endl;
	}
	catch (const std::runtime_error& err)
	{
		cerr << "Error: " << err.what() << endl;
	}
	catch (...)
	{
		cerr << "Error: " << msg(MSG::except_uncaught) << endl;
	}
}

void e2db_cli::shell_entry_list(ENTRY entry_type, string bname, int offset0, int offset1)
{
	shell_entry_list(entry_type, offset0, offset1, bname);
}

void e2db_cli::shell_entry_list(ENTRY entry_type, int offset0, int offset1, string bname)
{
	if (offset0 != -1 && offset1 != -1) // [start] [limit]
		shell_entry_list(entry_type, offset1 != 0, offset1, offset0, bname);
	else if (offset0 != -1) // [limit]
		shell_entry_list(entry_type, offset0 != 0, offset0, 0, bname);
	else
		shell_entry_list(entry_type, true, PAGED_LIMIT, 0, bname);
}

void e2db_cli::shell_entry_list(ENTRY entry_type, bool paged, int limit, int pos, string bname)
{
	int offset = limit;
	int end = 0;
	int rows = 1;

	//TODO FIX
	if (__objio.out == OBJIO::byline)
	{
		switch (entry_type)
		{
			case ENTRY::transponder: rows = 32; break;
			case ENTRY::service: rows = 24; break;
			case ENTRY::bouquet: rows = 6; break;
			case ENTRY::userbouquet: rows = 1; break;
			case ENTRY::tunersets: rows = 10; break;
			case ENTRY::tunersets_table: rows = 5; break;
			case ENTRY::tunersets_transponder: rows = 29; break;
			case ENTRY::channel_reference: rows = 1; break;
			default: rows = 1;
		}
	}
	else if (__objio.out == OBJIO::tabular)
	{
		rows = 5;
	}
	else if (__objio.out == OBJIO::json)
	{
		rows = 4;
	}

	if (paged)
	{
		if (limit == 0)
		{
			auto screensize = e2db_termctl::screensize();
			offset = screensize.first ? screensize.first / rows : 1;
		}

		shell_entry_list(entry_type, pos, offset, end, bname);

		while (! end)
		{
			if (limit == 0)
			{
				auto screensize = e2db_termctl::screensize();
				offset = screensize.first ? screensize.first / rows : 1;
			}

			int curr = e2db_termctl::paged(pos, offset);

			switch (curr)
			{
				case 0: return;
				case 65: pos -= offset; break;
				default: pos += offset;
			}

			shell_entry_list(entry_type, pos, offset, end, bname);
		}
	}
	else
	{
		shell_entry_list(entry_type, pos, offset, end, bname);
	}
}

//TODO improve
void e2db_cli::shell_entry_list(ENTRY entry_type, int pos, int offset, int& end, string bname)
{
	try
	{
		if (entry_type == ENTRY::transponder)
		{
			auto it = dbih->index["txs"].begin();
			auto last = dbih->index["txs"].end();

			if (offset != 0)
			{
				if (it + pos < last)
					it += pos;
				else
					it = last;
				if (it + offset < last)
					last = it + offset;
			}

			end = (last == dbih->index["txs"].end());

			for (; it != last; it++)
			{
				e2db::transponder tx = dbih->db.transponders[it->second];

				print_obj_begin(), print_obj_sep(-1);
				print_obj_pair(TYPE::txid, tx.txid), print_obj_sep();
				print_obj_pair(TYPE::ytype, tx.ytype), print_obj_sep();
				print_obj_pair(TYPE::pos, tx.pos), print_obj_sep();
				print_obj_pair(TYPE::tsid, tx.tsid), print_obj_sep();
				print_obj_pair(TYPE::onid, tx.onid), print_obj_sep();
				print_obj_pair(TYPE::dvbns, tx.dvbns), print_obj_sep();
				TYPE sys;
				switch (tx.ytype)
				{
					case e2db::YTYPE::satellite: sys = TYPE::sys; break;
					case e2db::YTYPE::terrestrial: sys = TYPE::tsys; break;
					case e2db::YTYPE::cable: sys = TYPE::csys; break;
					case e2db::YTYPE::atsc: sys = TYPE::asys; break;
				}
				print_obj_pair(sys, tx.sys), print_obj_sep();
				print_obj_pair(TYPE::freq, tx.freq), print_obj_sep();
				print_obj_pair(TYPE::pol, tx.pol), print_obj_sep();
				print_obj_pair(TYPE::sr, tx.sr), print_obj_sep();
				print_obj_pair(TYPE::mod, tx.mod), print_obj_sep();
				print_obj_pair(TYPE::tmod, tx.tmod), print_obj_sep();
				print_obj_pair(TYPE::cmod, tx.cmod), print_obj_sep();
				print_obj_pair(TYPE::amod, tx.amod), print_obj_sep();
				print_obj_pair(TYPE::band, tx.band), print_obj_sep();
				print_obj_pair(TYPE::tmx, tx.tmx), print_obj_sep();
				print_obj_pair(TYPE::fec, tx.fec), print_obj_sep();
				print_obj_pair(TYPE::hpfec, tx.hpfec), print_obj_sep();
				print_obj_pair(TYPE::lpfec, tx.lpfec), print_obj_sep();
				print_obj_pair(TYPE::cfec, tx.cfec), print_obj_sep();
				print_obj_pair(TYPE::inv, tx.inv), print_obj_sep();
				print_obj_pair(TYPE::rol, tx.rol), print_obj_sep();
				print_obj_pair(TYPE::pil, tx.pil), print_obj_sep();
				print_obj_pair(TYPE::guard, tx.guard), print_obj_sep();
				print_obj_pair(TYPE::hier, tx.hier), print_obj_sep();
				print_obj_pair(TYPE::flags, tx.flags), print_obj_sep();
				print_obj_pair(TYPE::plpid, tx.plpid), print_obj_sep();
				print_obj_pair(TYPE::isid, tx.isid), print_obj_sep();
				print_obj_pair(TYPE::plscode, tx.plscode), print_obj_sep();
				print_obj_pair(TYPE::plsmode, tx.plsmode), print_obj_sep();
				print_obj_pair(TYPE::t2mi_plpid, tx.t2mi_plpid), print_obj_sep();
				print_obj_pair(TYPE::t2mi_pid, tx.t2mi_pid), print_obj_sep();
				print_obj_pair(TYPE::idx, it->first), print_obj_sep(1);
				print_obj_end(), print_obj_dlm();
			}
		}
		else if (entry_type == ENTRY::service)
		{
			auto it = dbih->index["chs"].begin();
			auto last = dbih->index["chs"].end();

			if (offset != 0)
			{
				if (it + pos < last)
					it += pos;
				else
					it = last;
				if (it + offset < last)
					last = it + offset;
			}

			end = (last == dbih->index["chs"].end());

			for (; it != last; it++)
			{
				e2db::service ch = dbih->db.services[it->second];

				string refid = dbih->get_reference_id(ch.chid);
				vector<string> vxnul;

				print_obj_begin(), print_obj_sep(-1);
				print_obj_pair(TYPE::chid, ch.chid), print_obj_sep();
				print_obj_pair(TYPE::txid, ch.txid), print_obj_sep();
				print_obj_pair(TYPE::chname, ch.chname), print_obj_sep();
				print_obj_pair(TYPE::ssid, ch.ssid), print_obj_sep();
				print_obj_pair(TYPE::tsid, ch.tsid), print_obj_sep();
				print_obj_pair(TYPE::onid, ch.onid), print_obj_sep();
				print_obj_pair(TYPE::dvbns, ch.dvbns), print_obj_sep();
				print_obj_pair(TYPE::stype, ch.stype), print_obj_sep();
				print_obj_pair(TYPE::snum, ch.snum), print_obj_sep();
				print_obj_pair(TYPE::srcid, ch.srcid), print_obj_sep();

				print_obj_pair(TYPE::chdata, NULL);
				print_obj_begin(1), print_obj_sep(-1);
				print_obj_pair(TYPE::sdata_p, ch.data.count(e2db::SDATA::p) ? ch.data[e2db::SDATA::p] : vxnul), print_obj_sep();
				print_obj_pair(TYPE::sdata_c, ch.data.count(e2db::SDATA::c) ? ch.data[e2db::SDATA::c] : vxnul), print_obj_sep();
				print_obj_pair(TYPE::sdata_C, ch.data.count(e2db::SDATA::C) ? ch.data[e2db::SDATA::C] : vxnul), print_obj_sep();
				print_obj_pair(TYPE::sdata_f, ch.data.count(e2db::SDATA::f) ? ch.data[e2db::SDATA::f] : vxnul), print_obj_sep(1);
				print_obj_end(1), print_obj_sep();

				print_obj_pair(TYPE::parental, ch.parental), print_obj_sep();
				print_obj_pair(TYPE::locked, ch.locked), print_obj_sep();
				print_obj_pair(TYPE::refid, refid), print_obj_sep();
				print_obj_pair(TYPE::idx, it->first), print_obj_sep(1);
				print_obj_end(), print_obj_dlm();
			}
		}
		else if (entry_type == ENTRY::bouquet)
		{
			auto it = dbih->index["bss"].begin();
			auto last = dbih->index["bss"].end();

			if (offset != 0)
			{
				if (it + pos < last)
					it += pos;
				else
					it = last;
				if (it + offset < last)
					last = it + offset;
			}

			end = (last == dbih->index["bss"].end());

			for (; it != last; it++)
			{
				e2db::bouquet bs = dbih->bouquets[it->second];

				print_obj_begin(), print_obj_sep(-1);
				print_obj_pair(TYPE::bname, bs.bname), print_obj_sep();
				print_obj_pair(TYPE::rname, bs.rname), print_obj_sep();
				print_obj_pair(TYPE::btype, bs.btype), print_obj_sep();
				print_obj_pair(TYPE::qname, bs.name), print_obj_sep();
				print_obj_pair(TYPE::nname, bs.nname), print_obj_sep();
				print_obj_pair(TYPE::bsdata, NULL);
				print_obj_begin(1), print_obj_sep(-1);
				size_t i = 0;
				for (string & w : bs.userbouquets)
				{
					i++;

					print_obj_begin(2), print_obj_sep(-1);
					print_obj_pair(TYPE::bname, w), print_obj_sep(1);
					print_obj_end(2), print_obj_dlm(2, i == bs.userbouquets.size());
				}
				print_obj_end(1), print_obj_sep();
				print_obj_pair(TYPE::idx, it->first), print_obj_sep(1);
				print_obj_end(), print_obj_dlm();
			}
		}
		else if (entry_type == ENTRY::userbouquet)
		{
			auto it = dbih->index["ubs"].begin();
			auto last = dbih->index["ubs"].end();

			if (offset != 0)
			{
				if (it + pos < last)
					it += pos;
				else
					it = last;
				if (it + offset < last)
					last = it + offset;
			}

			end = (last == dbih->index["ubs"].end());

			for (; it != last; it++)
			{
				e2db::userbouquet ub = dbih->userbouquets[it->second];

				print_obj_begin(), print_obj_sep(-1);
				print_obj_pair(TYPE::bname, ub.bname), print_obj_sep();
				print_obj_pair(TYPE::rname, ub.rname), print_obj_sep();
				print_obj_pair(TYPE::qname, ub.name), print_obj_sep();
				print_obj_pair(TYPE::pname, ub.pname), print_obj_sep();
				print_obj_pair(TYPE::ubdata, NULL);
				print_obj_begin(1), print_obj_sep(-1);
				size_t i = 0;
				for (auto & x : ub.channels)
				{
					e2db::channel_reference chref = x.second;
					i++;

					print_obj_begin(2), print_obj_sep(-1);
					print_obj_pair(TYPE::chid, chref.chid), print_obj_sep(1);
					print_obj_end(2), print_obj_dlm(2, i == ub.channels.size());
				}
				print_obj_end(1), print_obj_sep();
				print_obj_pair(TYPE::parental, ub.parental), print_obj_sep();
				print_obj_pair(TYPE::hidden, ub.hidden), print_obj_sep();
				print_obj_pair(TYPE::locked, ub.locked), print_obj_sep();
				print_obj_pair(TYPE::idx, it->first), print_obj_sep(1);
				print_obj_end(), print_obj_dlm();
			}
		}
		else if (entry_type == ENTRY::tunersets)
		{
			vector<int> _index;

			if (dbih->tuners.size() == 4)
			{
				for (auto & ytype : {0, 1, 2, 3})
				{
					_index.push_back(ytype);
				}
			}

			auto it = _index.begin();
			auto last = _index.end();

			if (offset != 0)
			{
				if (it + pos < last)
					it += pos;
				else
					it = last;
				if (it + offset < last)
					last = it + offset;
			}

			end = (last == _index.end());

			for (; it != last; it++)
			{
				int tvid = *it;
				e2db::tunersets tv = dbih->tuners[tvid];

				print_obj_begin(), print_obj_sep(-1);
				print_obj_pair(TYPE::tvid, tv.ytype), print_obj_sep();
				print_obj_pair(TYPE::ytype, tv.ytype), print_obj_sep();
				print_obj_pair(TYPE::charset, tv.charset), print_obj_sep();
				print_obj_pair(TYPE::tvdata, NULL);
				print_obj_begin(1), print_obj_sep(-1);
				size_t i = 0;
				for (auto & x : tv.tables)
				{
					e2db::tunersets_table tn = x.second;
					i++;

					print_obj_begin(2), print_obj_sep(-1);
					print_obj_pair(TYPE::tnid, tn.tnid), print_obj_sep();
					print_obj_pair(TYPE::idx, tn.index), print_obj_sep(1);
					print_obj_end(2), print_obj_dlm(2, i == tv.tables.size());
				}
				print_obj_end(1);
				print_obj_end(), print_obj_dlm();
			}
		}
		else if (entry_type == ENTRY::tunersets_table)
		{
			vector<string> _index;

			if (dbih->tuners.size() == 4)
			{
				for (auto & ytype : {0, 1, 2, 3})
				{
					string iname = "tns:";
					char yname = dbih->value_transponder_type(ytype);
					iname += yname;

					for (auto & x : dbih->index[iname])
					{
						string tnid = x.second;
						_index.push_back(tnid);
					}
				}
			}

			auto it = _index.begin();
			auto last = _index.end();

			if (offset != 0)
			{
				if (it + pos < last)
					it += pos;
				else
					it = last;
				if (it + offset < last)
					last = it + offset;
			}

			end = (last == _index.end());

			for (; it != last; it++)
			{
				string tnid = *it;
				char ty = tnid[0];
				int tvid = dbih->value_transponder_type(ty);

				e2db::tunersets_table tn = dbih->tuners[tvid].tables[tnid];

				print_obj_begin(), print_obj_sep(-1);
				print_obj_pair(TYPE::tnid, tn.tnid), print_obj_sep();
				print_obj_pair(TYPE::ytype, tn.ytype), print_obj_sep();
				print_obj_pair(TYPE::tname, tn.name), print_obj_sep();
				print_obj_pair(TYPE::pos, tn.pos), print_obj_sep();
				print_obj_pair(TYPE::country, tn.country), print_obj_sep();
				print_obj_pair(TYPE::feed, tn.feed), print_obj_sep();
				print_obj_pair(TYPE::flags, tn.flags), print_obj_sep();
				print_obj_pair(TYPE::tndata, NULL);
				print_obj_begin(1), print_obj_sep(-1);
				size_t i = 0;
				for (auto & x : tn.transponders)
				{
					e2db::tunersets_transponder tntxp = x.second;
					i++;

					print_obj_begin(2), print_obj_sep(-1);
					print_obj_pair(TYPE::trid, tntxp.trid), print_obj_sep();
					print_obj_pair(TYPE::idx, tntxp.index), print_obj_sep(1);
					print_obj_end(2), print_obj_dlm(2, i == tn.transponders.size());
				}
				print_obj_end(1), print_obj_sep();
				print_obj_pair(TYPE::idx, tn.index), print_obj_sep(1);
				print_obj_end(), print_obj_dlm();
			}
		}
		else if (entry_type == ENTRY::tunersets_transponder)
		{
			vector<pair<string, string>> _index;

			if (dbih->tuners.size() == 4)
			{
				for (auto & d : {0, 1, 2, 3})
				{
					e2db::tunersets tv = dbih->tuners[d];

					string iname = "tns:";
					char yname = dbih->value_transponder_type(tv.ytype);
					iname += yname;

					for (auto & x : dbih->index[iname])
					{
						string tnid = x.second;

						for (auto & x : dbih->index[tnid])
						{
							string trid = x.second;
							_index.push_back(pair (trid, tnid));
						}
					}
				}
			}

			auto it = _index.begin();
			auto last = _index.end();

			if (offset != 0)
			{
				if (it + pos < last)
					it += pos;
				else
					it = last;
				if (it + offset < last)
					last = it + offset;
			}

			end = (last == _index.end());

			for (; it != last; it++)
			{
				string trid = it->first;
				string tnid = it->second;
				char ty = tnid[0];
				int tvid = dbih->value_transponder_type(ty);

				e2db::tunersets_table tn = dbih->tuners[tvid].tables[tnid];
				e2db::tunersets_transponder tntxp = dbih->tuners[tvid].tables[tnid].transponders[trid];

				print_obj_begin(), print_obj_sep(-1);
				print_obj_pair(TYPE::trid, tntxp.trid), print_obj_sep();
				print_obj_pair(TYPE::tnid, tn.tnid), print_obj_sep();
				print_obj_pair(TYPE::ytype, tn.ytype), print_obj_sep();
				print_obj_pair(TYPE::pos, tn.pos), print_obj_sep();
				TYPE sys;
				switch (tn.ytype)
				{
					case e2db::YTYPE::satellite: sys = TYPE::sys; break;
					case e2db::YTYPE::terrestrial: sys = TYPE::tsys; break;
					case e2db::YTYPE::cable: sys = TYPE::csys; break;
					case e2db::YTYPE::atsc: sys = TYPE::asys; break;
				}
				print_obj_pair(sys, tntxp.sys), print_obj_sep();
				print_obj_pair(TYPE::freq, tntxp.freq), print_obj_sep();
				print_obj_pair(TYPE::pol, tntxp.pol), print_obj_sep();
				print_obj_pair(TYPE::sr, tntxp.sr), print_obj_sep();
				print_obj_pair(TYPE::mod, tntxp.mod), print_obj_sep();
				print_obj_pair(TYPE::tmod, tntxp.tmod), print_obj_sep();
				print_obj_pair(TYPE::cmod, tntxp.cmod), print_obj_sep();
				print_obj_pair(TYPE::amod, tntxp.amod), print_obj_sep();
				print_obj_pair(TYPE::band, tntxp.band), print_obj_sep();
				print_obj_pair(TYPE::tmx, tntxp.tmx), print_obj_sep();
				print_obj_pair(TYPE::fec, tntxp.fec), print_obj_sep();
				print_obj_pair(TYPE::hpfec, tntxp.hpfec), print_obj_sep();
				print_obj_pair(TYPE::lpfec, tntxp.lpfec), print_obj_sep();
				print_obj_pair(TYPE::cfec, tntxp.cfec), print_obj_sep();
				print_obj_pair(TYPE::inv, tntxp.inv), print_obj_sep();
				print_obj_pair(TYPE::rol, tntxp.rol), print_obj_sep();
				print_obj_pair(TYPE::pil, tntxp.pil), print_obj_sep();
				print_obj_pair(TYPE::guard, tntxp.guard), print_obj_sep();
				print_obj_pair(TYPE::hier, tntxp.hier), print_obj_sep();
				print_obj_pair(TYPE::plpid, tntxp.plpid), print_obj_sep();
				print_obj_pair(TYPE::plsn, tntxp.plsn), print_obj_sep();
				print_obj_pair(TYPE::plscode, tntxp.plscode), print_obj_sep();
				print_obj_pair(TYPE::plsmode, tntxp.plsmode), print_obj_sep();
				print_obj_pair(TYPE::isid, tntxp.isid), print_obj_sep();
				print_obj_pair(TYPE::idx, tntxp.index), print_obj_sep(1);
				print_obj_end(), print_obj_dlm();
			}
		}
		else if (entry_type == ENTRY::channel_reference)
		{
			if (! dbih->userbouquets.count(bname))
			{
				cerr << "Error: " << msg("Userbouquet \"%s\" not exists.", bname) << endl;

				end = 1;

				return;
			}

			auto it = dbih->index[bname].begin();
			auto last = dbih->index[bname].end();

			if (offset != 0)
			{
				if (it + pos < last)
					it += pos;
				else
					it = last;
				if (it + offset < last)
					last = it + offset;
			}

			end = (last == dbih->index[bname].end());

			for (; it != last; it++)
			{
				e2db::channel_reference chref = dbih->userbouquets[bname].channels[it->second];

				if (chref.marker)
				{
					string refid = dbih->get_reference_id(chref);

					print_obj_begin(), print_obj_sep(-1);
					print_obj_pair(TYPE::chid, chref.chid), print_obj_sep();
					print_obj_pair(TYPE::mname, chref.value), print_obj_sep();
					print_obj_pair(TYPE::etype, chref.etype), print_obj_sep();
					print_obj_pair(TYPE::atype, chref.atype), print_obj_sep();
					print_obj_pair(TYPE::mnum, chref.anum), print_obj_sep();
					print_obj_pair(TYPE::refid, refid), print_obj_sep();
					print_obj_pair(TYPE::idx, it->first), print_obj_sep(1);
					print_obj_end(), print_obj_dlm();
				}
				else if (chref.stream)
				{
					string refid = dbih->get_reference_id(chref);

					print_obj_begin(), print_obj_sep(-1);
					print_obj_pair(TYPE::chid, chref.chid), print_obj_sep();
					print_obj_pair(TYPE::chvalue, chref.value), print_obj_sep();
					print_obj_pair(TYPE::etype, chref.etype), print_obj_sep();
					print_obj_pair(TYPE::ssid, chref.ref.ssid), print_obj_sep();
					print_obj_pair(TYPE::tsid, chref.ref.tsid), print_obj_sep();
					print_obj_pair(TYPE::stype, chref.anum), print_obj_sep();
					print_obj_pair(TYPE::churi, chref.uri), print_obj_sep();
					print_obj_pair(TYPE::refid, refid), print_obj_sep();
					print_obj_pair(TYPE::idx, it->first), print_obj_sep(1);
					print_obj_end(), print_obj_dlm();
				}
				else if (dbih->db.services.count(chref.chid))
				{
					e2db::service ch = dbih->db.services[chref.chid];

					string refid = dbih->get_reference_id(ch.chid);
					vector<string> vxnul;

					print_obj_begin(), print_obj_sep(-1);
					print_obj_pair(TYPE::chid, ch.chid), print_obj_sep();
					print_obj_pair(TYPE::txid, ch.txid), print_obj_sep();
					print_obj_pair(TYPE::chname, ch.chname), print_obj_sep();
					print_obj_pair(TYPE::ssid, ch.ssid), print_obj_sep();
					print_obj_pair(TYPE::tsid, ch.tsid), print_obj_sep();
					print_obj_pair(TYPE::stype, ch.stype), print_obj_sep();

					print_obj_pair(TYPE::chdata, NULL);
					print_obj_begin(1), print_obj_sep(-1);
					print_obj_pair(TYPE::sdata_p, ch.data.count(e2db::SDATA::p) ? ch.data[e2db::SDATA::p] : vxnul), print_obj_sep();
					print_obj_pair(TYPE::sdata_c, ch.data.count(e2db::SDATA::c) ? ch.data[e2db::SDATA::c] : vxnul), print_obj_sep();
					print_obj_pair(TYPE::sdata_C, ch.data.count(e2db::SDATA::C) ? ch.data[e2db::SDATA::C] : vxnul), print_obj_sep();
					print_obj_pair(TYPE::sdata_f, ch.data.count(e2db::SDATA::f) ? ch.data[e2db::SDATA::f] : vxnul), print_obj_sep(1);
					print_obj_end(1), print_obj_sep();

					print_obj_pair(TYPE::refid, refid), print_obj_sep();
					print_obj_pair(TYPE::parental, ch.parental), print_obj_sep();
					print_obj_pair(TYPE::locked, ch.locked), print_obj_sep();
					print_obj_pair(TYPE::idx, it->first), print_obj_sep(1);
					print_obj_end(), print_obj_dlm();
				}
				else
				{
					// orphan
				}
			}
		}
	}
	catch (const std::invalid_argument& err)
	{
		cerr << "Error: " << msg(MSG::except_invalid_argument, err.what()) << endl;
	}
	catch (const std::out_of_range& err)
	{
		cerr << "Error: " << msg(MSG::except_out_of_range, err.what()) << endl;
	}
	catch (const std::bad_any_cast& err)
	{
		cerr << "Error: " << msg(MSG::except_bad_any_cast, err.what()) << endl;
	}
	catch (const std::runtime_error& err)
	{
		cerr << "Error: " << err.what() << endl;
	}
	catch (...)
	{
		cerr << "Error: " << msg(MSG::except_uncaught) << endl;
	}
}

void e2db_cli::shell_entry_add(ENTRY entry_type)
{
	shell_entry_edit(entry_type, false);
}

void e2db_cli::shell_entry_add(ENTRY entry_type, int ref, string bname)
{
	shell_entry_edit(entry_type, false, "", ref, bname);
}

void e2db_cli::shell_entry_edit(ENTRY entry_type, string id)
{
	shell_entry_edit(entry_type, true, id);
}

void e2db_cli::shell_entry_edit(ENTRY entry_type, int ref, string bname, string id)
{
	shell_entry_edit(entry_type, true, id, ref, bname);
}

//TODO improve
void e2db_cli::shell_entry_edit(ENTRY entry_type, bool edit, string id, int ref, string bname)
{
	using std::any_cast;

	if (edit && id.empty())
	{
		cerr << "Error: " << msg("Wrong parameter identifier.") << endl;

		return;
	}

	try
	{
		if (entry_type == ENTRY::transponder)
		{
			e2db::transponder tx;

			if (edit)
			{
				if (dbih->db.transponders.count(id))
					tx = dbih->db.transponders[id];
				else
					throw std::runtime_error (msg("Transponder \"%s\" not exists.", id));
			}

			tx.ytype = any_cast<int>(field(TYPE::yname, true));
			tx.pos = any_cast<int>(field(TYPE::pos, true));
			tx.sys = any_cast<int>(field(TYPE::sys));
			tx.tsid = any_cast<int>(field(TYPE::tsid, true));
			tx.onid = any_cast<int>(field(TYPE::onid, true));
			tx.freq = any_cast<int>(field(TYPE::freq, true));

			if (tx.ytype == e2db::YTYPE::satellite)
			{
				tx.pol = any_cast<int>(field(TYPE::pol, true));
				tx.sr = any_cast<int>(field(TYPE::sr));
				tx.fec = any_cast<int>(field(TYPE::fec));
				tx.mod = any_cast<int>(field(TYPE::mod));
				tx.inv = any_cast<int>(field(TYPE::inv));
				tx.rol = any_cast<int>(field(TYPE::rol));
				tx.pil = any_cast<int>(field(TYPE::pil));
				tx.flags = any_cast<int>(field(TYPE::flags));

				if (any_cast<int>(field(TYPE::txdata)))
				{
					tx.isid = any_cast<int>(field(TYPE::isid));
					tx.plscode = any_cast<int>(field(TYPE::plscode));
					tx.plsmode = any_cast<int>(field(TYPE::plsmode));
					tx.t2mi_plpid = any_cast<int>(field(TYPE::t2mi_plpid));
					tx.t2mi_pid = any_cast<int>(field(TYPE::t2mi_pid));

					tx.mispls = tx.mispls ?: tx.isid != -1 || tx.plscode != -1 || tx.plsmode != -1;
					tx.t2mi = tx.t2mi ?: tx.t2mi_plpid != -1 || tx.t2mi_pid != -1;
				}
			}
			else if (tx.ytype == e2db::YTYPE::terrestrial)
			{
				tx.tmod = any_cast<int>(field(TYPE::tmod));
				tx.band = any_cast<int>(field(TYPE::band));
				tx.tmx = any_cast<int>(field(TYPE::tmx));
				tx.hpfec = any_cast<int>(field(TYPE::hpfec));
				tx.lpfec = any_cast<int>(field(TYPE::lpfec));
				tx.inv = any_cast<int>(field(TYPE::tinv));
				tx.guard = any_cast<int>(field(TYPE::guard));
				tx.hier = any_cast<int>(field(TYPE::hier));
				tx.flags = any_cast<int>(field(TYPE::flags));

				if (any_cast<int>(field(TYPE::txdata)))
				{
					tx.plpid = any_cast<int>(field(TYPE::plpid));
				}
			}
			else if (tx.ytype == e2db::YTYPE::cable)
			{
				tx.cmod = any_cast<int>(field(TYPE::cmod));
				tx.sr = any_cast<int>(field(TYPE::sr));
				tx.cfec = any_cast<int>(field(TYPE::cfec));
				tx.inv = any_cast<int>(field(TYPE::cinv));
				tx.flags = any_cast<int>(field(TYPE::flags));
			}
			else if (tx.ytype == e2db::YTYPE::atsc)
			{
				tx.amod = any_cast<int>(field(TYPE::amod));
				tx.flags = any_cast<int>(field(TYPE::flags));
			}

			tx.dvbns = dbih->value_transponder_dvbns(tx);

			if (edit)
				dbih->edit_transponder(id, tx);
			else
				dbih->add_transponder(tx);
		}
		else if (entry_type == ENTRY::service)
		{
			e2db::service ch;

			if (edit)
			{
				if (dbih->db.services.count(id))
					ch = dbih->db.services[id];
				else
					throw std::runtime_error (msg("Service \"%s\" not exists.", id));
			}

			ch.txid = any_cast<string>(field(TYPE::txid, true));

			if (dbih->db.transponders.count(ch.txid))
			{
				e2db::transponder tx = dbih->db.transponders[ch.txid];

				ch.tsid = tx.tsid;
				ch.onid = tx.onid;
				ch.dvbns = tx.dvbns;
			}
			else
			{
				throw std::runtime_error (msg("Transponder \"%s\" not exists.", ch.txid));
			}

			ch.stype = any_cast<int>(field(TYPE::stype, true));
			ch.ssid = any_cast<int>(field(TYPE::ssid, true));
			ch.chname = any_cast<string>(field(TYPE::chname, true));
			ch.snum = any_cast<int>(field(TYPE::snum));
			ch.srcid = any_cast<int>(field(TYPE::srcid));

			if (any_cast<int>(field(TYPE::chdata)))
			{
				ch.data[e2db::SDATA::p] = any_cast<vector<string>>(field(TYPE::sdata_p));
				ch.data[e2db::SDATA::c] = any_cast<vector<string>>(field(TYPE::sdata_c));
				ch.data[e2db::SDATA::C] = any_cast<vector<string>>(field(TYPE::sdata_C));
				ch.data[e2db::SDATA::f] = any_cast<vector<string>>(field(TYPE::sdata_f));
			}

			if (edit)
				dbih->edit_service(id, ch);
			else
				dbih->add_service(ch);
		}
		else if (entry_type == ENTRY::bouquet)
		{
			e2db::bouquet bs;

			if (edit)
			{
				if (dbih->bouquets.count(id))
					bs = dbih->bouquets[id];
				else
					throw std::runtime_error (msg("Bouquet \"%s\" not exists.", id));
			}

			bs.btype = any_cast<int>(field(TYPE::btype, true));
			if (edit)
				bs.rname = any_cast<string>(field(TYPE::rname));
			else
				bs.bname = any_cast<string>(field(TYPE::bname, true));
			bs.name = any_cast<string>(field(TYPE::qname, true));
			bs.nname = any_cast<string>(field(TYPE::nname));

			if (edit)
				dbih->edit_bouquet(bs);
			else
				dbih->add_bouquet(bs);
		}
		else if (entry_type == ENTRY::userbouquet)
		{
			e2db::userbouquet ub;

			if (edit)
			{
				if (dbih->userbouquets.count(id))
					ub = dbih->userbouquets[id];
				else
					throw std::runtime_error (msg("Userbouquet \"%s\" not exists.", id));
			}

			if (edit)
				ub.rname = any_cast<string>(field(TYPE::rname));
			else
				ub.bname = any_cast<string>(field(TYPE::bname, true));
			ub.pname = any_cast<string>(field(TYPE::pname, true));
			ub.name = any_cast<string>(field(TYPE::qname, true));

			if (edit)
				dbih->edit_userbouquet(ub);
			else
				dbih->add_userbouquet(ub);
		}
		else if (entry_type == ENTRY::tunersets)
		{
			e2db::tunersets tv;

			int tvid = -1;

			if (edit)
			{
				try
				{
					tvid = std::stoi(id);

					if (dbih->tuners.count(tvid))
						tv = dbih->tuners[tvid];
				}
				catch (...)
				{
					throw std::runtime_error (msg("Tuner settings \"%s\" not exists.", id));
				}
			}

			tv.ytype = any_cast<int>(field(TYPE::ytype, true));
			tv.charset = any_cast<string>(field(TYPE::charset));

			if (tv.charset.empty())
				tv.charset = "UTF-8";

			if (edit)
				dbih->edit_tunersets(tvid, tv);
			else
				dbih->add_tunersets(tv);
		}
		else if (entry_type == ENTRY::tunersets_table)
		{
			e2db::tunersets tv;
			e2db::tunersets_table tn;

			if (edit)
			{
				bool found = false;

				for (auto & x : dbih->tuners)
				{
					tv = x.second;

					if (tv.tables.count(id))
					{
						found = true;
						tn = x.second.tables[id];
						break;
					}
				}

				if (! found)
					throw std::runtime_error (msg("Tuner settings table \"%s\" not exists.", id));
			}

			tn.ytype = any_cast<int>(field(TYPE::ytype, true));
			tn.name = any_cast<string>(field(TYPE::tname, true));

			if (tn.ytype == e2db::YTYPE::satellite)
			{
				tn.pos = any_cast<int>(field(TYPE::pos, true));
			}
			else if (tn.ytype == e2db::YTYPE::terrestrial)
			{
				tn.country = any_cast<string>(field(TYPE::country));
			}
			else if (tn.ytype == e2db::YTYPE::cable)
			{
				tn.country = any_cast<string>(field(TYPE::country));
				tn.feed = any_cast<int>(field(TYPE::feed));
			}

			tn.flags = any_cast<int>(field(TYPE::flags));

			if (edit)
				dbih->edit_tunersets_table(id, tn, tv);
			else
				dbih->add_tunersets_table(tn, tv);
		}
		else if (entry_type == ENTRY::tunersets_transponder)
		{
			e2db::tunersets tv;
			e2db::tunersets_table tn;
			e2db::tunersets_transponder tntxp;

			if (edit)
			{
				bool found = false;

				for (auto & x : dbih->tuners)
				{
					tv = x.second;

					for (auto & x : tv.tables)
					{
						tn = x.second;

						if (tn.transponders.count(id))
						{
							found = true;
							tntxp = tn.transponders[id];
							break;
							break;
						}
					}
				}

				if (! found)
					throw std::runtime_error (msg("Tuner settings transponder \"%s\" not exists.", id));
			}

			tntxp.sys = any_cast<int>(field(TYPE::sys));
			tntxp.freq = any_cast<int>(field(TYPE::freq, true));

			if (tn.ytype == e2db::YTYPE::satellite)
			{
				tntxp.pol = any_cast<int>(field(TYPE::pol, true));
				tntxp.sr = any_cast<int>(field(TYPE::sr));
				tntxp.fec = any_cast<int>(field(TYPE::fec));
				tntxp.mod = any_cast<int>(field(TYPE::mod));
				tntxp.inv = any_cast<int>(field(TYPE::inv));
				tntxp.rol = any_cast<int>(field(TYPE::rol));
				tntxp.pil = any_cast<int>(field(TYPE::pil));

				if (any_cast<int>(field(TYPE::txdata)))
				{
					tntxp.plsn = any_cast<int>(field(TYPE::plsn));
					tntxp.plscode = any_cast<int>(field(TYPE::plscode));
					tntxp.plsmode = any_cast<int>(field(TYPE::plsmode));
					tntxp.isid = any_cast<int>(field(TYPE::isid));
					tntxp.mts = any_cast<int>(field(TYPE::mts));
				}
			}
			else if (tn.ytype == e2db::YTYPE::terrestrial)
			{
				tntxp.tmod = any_cast<int>(field(TYPE::tmod));
				tntxp.band = any_cast<int>(field(TYPE::band));
				tntxp.tmx = any_cast<int>(field(TYPE::tmx));
				tntxp.hpfec = any_cast<int>(field(TYPE::hpfec));
				tntxp.lpfec = any_cast<int>(field(TYPE::lpfec));
				tntxp.inv = any_cast<int>(field(TYPE::tinv));
				tntxp.guard = any_cast<int>(field(TYPE::guard));
				tntxp.hier = any_cast<int>(field(TYPE::hier));

				if (any_cast<int>(field(TYPE::txdata)))
				{
					tntxp.plpid = any_cast<int>(field(TYPE::plpid));
				}
			}
			else if (tn.ytype == e2db::YTYPE::cable)
			{
				tntxp.cmod = any_cast<int>(field(TYPE::cmod));
				tntxp.sr = any_cast<int>(field(TYPE::sr));
				tntxp.cfec = any_cast<int>(field(TYPE::cfec));
				tntxp.inv = any_cast<int>(field(TYPE::cinv));
			}
			else if (tn.ytype == e2db::YTYPE::atsc)
			{
				tntxp.amod = any_cast<int>(field(TYPE::amod));
			}

			if (edit)
				dbih->edit_tunersets_transponder(id, tntxp, tn);
			else
				dbih->add_tunersets_transponder(tntxp, tn);
		}
		else if (entry_type == ENTRY::channel_reference)
		{
			if (! dbih->userbouquets.count(bname))
				throw std::runtime_error (msg("Userbouquet \"%s\" not exists.", bname));

			e2db::channel_reference chref;

			if (ref == 0) // marker
			{
				chref.marker = true;
				chref.etype = 1;
				chref.value = any_cast<string>(field(TYPE::mname));
				chref.atype = any_cast<int>(field(TYPE::atype));

				if (any_cast<int>(field(TYPE::ffdata)))
				{
					chref.anum = any_cast<int>(field(TYPE::mnum));
				}
			}
			else if (ref == 2) // stream
			{
				chref.stream = true;
				chref.etype = any_cast<int>(field(TYPE::etype));
				chref.value = any_cast<string>(field(TYPE::chvalue));
				chref.uri = any_cast<string>(field(TYPE::churi));

				if (chref.etype == 0)
					chref.etype = 1;
			}
			else // service
			{
				if (! dbih->db.services.count(id))
					throw std::runtime_error (msg("Service \"%s\" not exists.", id));

				chref.chid = id;
				chref.etype = 1;
			}

			if (edit)
				dbih->edit_channel_reference(id, chref, bname);
			else
				dbih->add_channel_reference(chref, bname);
		}
	}

	catch (const std::invalid_argument& err)
	{
		cerr << "Error: " << msg(MSG::except_invalid_argument, err.what()) << endl;
	}
	catch (const std::out_of_range& err)
	{
		cerr << "Error: " << msg(MSG::except_out_of_range, err.what()) << endl;
	}
	catch (const std::bad_any_cast& err)
	{
		cerr << "Error: " << msg("Wrong input data in \"%s\" field.", this->last_label) << endl;
	}
	catch (const std::runtime_error& err)
	{
		cerr << "Error: " << err.what() << endl;
	}
	catch (...)
	{
		cerr << "Error: " << msg(MSG::except_uncaught) << endl;
	}
}

void e2db_cli::shell_entry_remove(ENTRY entry_type, int ref, string bname, string id)
{
	shell_entry_remove(entry_type, id, bname);
}

void e2db_cli::shell_entry_remove(ENTRY entry_type, string id, string bname)
{
	if (id.empty())
	{
		cerr << "Error: " << msg("Wrong parameter identifier.") << endl;

		return;
	}

	try
	{
		if (entry_type == ENTRY::transponder)
		{
			if (! dbih->db.transponders.count(id))
				throw std::runtime_error (msg("Transponder \"%s\" not exists.", id));

			dbih->remove_transponder(id);
		}
		else if (entry_type == ENTRY::service)
		{
			if (! dbih->db.services.count(id))
				throw std::runtime_error (msg("Service \"%s\" not exists.", id));

			dbih->remove_service(id);
		}
		else if (entry_type == ENTRY::bouquet)
		{
			if (! dbih->bouquets.count(id))
				throw std::runtime_error (msg("Bouquet \"%s\" not exists.", id));

			dbih->remove_bouquet(id);
		}
		else if (entry_type == ENTRY::userbouquet)
		{
			if (! dbih->userbouquets.count(id))
				throw std::runtime_error (msg("Userbouquet \"%s\" not exists.", id));

			dbih->remove_userbouquet(id);
		}
		else if (entry_type == ENTRY::tunersets)
		{
			int tvid = -1;

			try
			{
				tvid = std::stoi(id);

				if (dbih->tuners.count(tvid))
					throw 1;
			}
			catch (...)
			{
				throw std::runtime_error (msg("Tuner settings \"%s\" not exists.", id));
			}

			dbih->remove_tunersets(tvid);
		}
		else if (entry_type == ENTRY::tunersets_table)
		{
			e2db::tunersets tv;
			bool found = false;

			for (auto & x : dbih->tuners)
			{
				tv = x.second;

				if (tv.tables.count(id))
				{
					found = true;
					break;
				}
			}

			if (! found)
				throw std::runtime_error (msg("Tuner settings table \"%s\" not exists.", id));

			dbih->remove_tunersets_table(id, tv);
		}
		else if (entry_type == ENTRY::tunersets_transponder)
		{
			e2db::tunersets tv;
			e2db::tunersets_table tn;
			bool found = false;

			for (auto & x : dbih->tuners)
			{
				tv = x.second;

				for (auto & x : tv.tables)
				{
					tn = x.second;

					if (tn.transponders.count(id))
					{
						found = true;
						break;
						break;
					}
				}
			}

			if (! found)
				throw std::runtime_error (msg("Tuner settings transponder \"%s\" not exists.", id));

			dbih->remove_tunersets_transponder(id, tn);
		}
		else if (entry_type == ENTRY::channel_reference)
		{
			if (! dbih->userbouquets.count(bname))
				throw std::runtime_error (msg("Userbouquet \"%s\" not exists.", bname));

			if (! dbih->userbouquets[bname].channels.count(id))
				throw std::runtime_error (msg("Channel reference \"%s\" not exists.", id));

			e2db::channel_reference chref = dbih->userbouquets[bname].channels[id];

			dbih->remove_channel_reference(chref, bname);
		}
	}
	catch (const std::runtime_error& err)
	{
		cerr << "Error: " << err.what() << endl;
	}
	catch (...)
	{
		cerr << "Error: " << msg(MSG::except_uncaught) << endl;
	}
}

void e2db_cli::shell_entry_parentallock(ENTRY entry_type, string id, bool flag)
{
	if (id.empty())
	{
		cerr << "Error: " << msg("Wrong parameter identifier.") << endl;

		return;
	}

	try
	{
		if (entry_type == ENTRY::service)
		{
			if (! dbih->db.services.count(id))
				throw std::runtime_error (msg("Service \"%s\" not exists.", id));

			if (flag)
				dbih->set_service_parentallock(id);
			else
				dbih->unset_service_parentallock(id);
		}
		else if (entry_type == ENTRY::userbouquet)
		{
			if (! dbih->userbouquets.count(id))
				throw std::runtime_error (msg("Userbouquet \"%s\" not exists.", id));

			if (flag)
				dbih->set_userbouquet_parentallock(id);
			else
				dbih->unset_userbouquet_parentallock(id);
		}
	}
	catch (const std::runtime_error& err)
	{
		cerr << "Error: " << err.what() << endl;
	}
	catch (...)
	{
		cerr << "Error: " << msg(MSG::except_uncaught) << endl;
	}
}

void e2db_cli::shell_print(int opt)
{
	try
	{
		if (opt)
			cout << "TODO" << endl;
		else
			dbih->debugger();
	}
	catch (const std::invalid_argument& err)
	{
		cerr << "Error: " << msg(MSG::except_invalid_argument, err.what()) << endl;
	}
	catch (const std::out_of_range& err)
	{
		cerr << "Error: " << msg(MSG::except_out_of_range, err.what()) << endl;
	}
	catch (...)
	{
		cerr << "Error: " << msg(MSG::except_uncaught) << endl;
	}
}

void e2db_cli::shell_debug()
{
	cout << log->str();
}

void e2db_cli::shell_preference_output(OBJIO format)
{
	if (format == OBJIO::tabular)
	{
		__objio.out = OBJIO::tabular;
		__objio.hrn = true;
	}
	else if (format == OBJIO::byline)
	{
		__objio.out = OBJIO::byline;
		__objio.hrn = false;
	}
	else if (format == OBJIO::json)
	{
		__objio.out = OBJIO::json;
		__objio.hrn = false;
	}
	else
	{
		cerr << "Error: " << msg("Wrong parameter format.") << endl;
	}
}

void e2db_cli::print_obj_begin(int depth)
{
	if (__objio.out == OBJIO::tabular)
	{
		if (depth)
			cout << '[' << ' ';
	}
	else if (__objio.out == OBJIO::byline)
	{
		if (depth)
			cout << '[' << endl;
	}
	else if (__objio.out == OBJIO::json)
	{
		if (depth == 1)
			cout << '[';
		else
			cout << '{';
	}
}

void e2db_cli::print_obj_end(int depth)
{
	if (__objio.out == OBJIO::tabular)
	{
		if (depth)
			cout << ' ' << ']';
	}
	else if (__objio.out == OBJIO::byline)
	{
		if (depth)
			cout << ']' << endl;
	}
	else if (__objio.out == OBJIO::json)
	{
		if (depth == 1)
			cout << ']';
		else
			cout << '}';
	}
}

void e2db_cli::print_obj_sep(int xpos)
{
	if (__objio.out == OBJIO::tabular)
	{
		if (xpos == 0)
			cout << '\t' << ' ';
	}
	else if (__objio.out == OBJIO::byline)
	{
		cout << endl;
	}
	else if (__objio.out == OBJIO::json)
	{
		if (xpos == 0)
			cout << ',' << ' ';
	}
}

void e2db_cli::print_obj_dlm(int depth, int xpos)
{
	if (__objio.out == OBJIO::tabular)
	{
		if (depth == 2)
		{
			if (xpos == 0)
				cout << '\t';
		}
		else
		{
			cout << endl << endl;
		}
	}
	else if (__objio.out == OBJIO::byline)
	{
		cout << endl;
	}
	else if (__objio.out == OBJIO::json)
	{
		if (xpos == 0)
			cout << ',' << ' ';
		if (depth == 0)
			cout << endl << endl;
	}
}

void e2db_cli::print_obj_pair(TYPE type, std::any val)
{
	using std::any_cast;

	bool hrn = __objio.hrn;
	bool hrv = __objio.hrv;

	string name;
	VALUE value_type;

	switch (type)
	{
		case TYPE::dbtype: name = hrn ? "Format" : "dbtype"; value_type = hrv ? VALUE::val_string : VALUE::val_int; break;
		case TYPE::dbparental: name = hrn ? "Parental lock" : "dbparental"; value_type = hrv ? VALUE::val_string : VALUE::val_int; break;
		case TYPE::idx: name = hrn ? "Index" : "idx"; value_type = VALUE::val_int; break;
		case TYPE::chid: name = hrn ? "CHID" : "chid"; value_type = VALUE::val_string; break;
		case TYPE::txid: name = hrn ? "TXID" : "txid"; value_type = VALUE::val_string; break;
		case TYPE::refid: name = hrn ? "Reference" : "refid"; value_type = VALUE::val_string; break;
		case TYPE::tvid: name = hrn ? "TVID" : "tvid"; value_type = VALUE::val_int; break;
		case TYPE::tnid: name = hrn ? "TNID" : "tnid"; value_type = VALUE::val_string; break;
		case TYPE::trid: name = hrn ? "TRID" : "trid"; value_type = VALUE::val_string; break;
		case TYPE::yname: name = hrn ? "YNAME" : "yname"; value_type = VALUE::val_char; break;
		case TYPE::ytype: name = hrn ? "YTYPE" : "ytype"; value_type = VALUE::val_int; break;
		case TYPE::ssid: name = hrn ? "SSID" : "ssid"; value_type = VALUE::val_int; break;
		case TYPE::dvbns: name = hrn ? "DVBNS" : "dvbns"; value_type = VALUE::val_string; break;
		case TYPE::tsid: name = hrn ? "TSID" : "tsid"; value_type = VALUE::val_int; break;
		case TYPE::onid: name = hrn ? "ONID" : "onid"; value_type = VALUE::val_string; break;
		case TYPE::stype: name = hrn ? "Service Type" : "stype"; value_type = hrv ? VALUE::val_string : VALUE::val_int; break;
		case TYPE::snum: name = hrn ? "snum" : "snum"; value_type = VALUE::val_int; break;
		case TYPE::srcid: name = hrn ? "srcid" : "srcid"; value_type = VALUE::val_int; break;
		case TYPE::parental: name = hrn ? "Parental locked" : "locked"; value_type = VALUE::val_bool; break;
		case TYPE::chname: name = hrn ? "Service Name" : "chname"; value_type = VALUE::val_string; break;
		case TYPE::sdata_p: name = hrn ? "Provider Name" : "sdata_p"; value_type = VALUE::val_string; break;
		case TYPE::sdata_c: name = hrn ? "Service Cache" : "sdata_c"; value_type = VALUE::val_string; break;
		case TYPE::sdata_C: name = hrn ? "Service CAS" : "sdata_C"; value_type = VALUE::val_string; break;
		case TYPE::sdata_f: name = hrn ? "Service Flags" : "sdata_f"; value_type = VALUE::val_string; break;
		case TYPE::mname: name = hrn ? "Marker Name" : "mname"; value_type = VALUE::val_string; break;
		case TYPE::mnum: name = hrn ? "Marker Number" : "mnum"; value_type = VALUE::val_int; break;
		case TYPE::chvalue: name = hrn ? "Channel Name" : "chvalue"; value_type = VALUE::val_string; break;
		case TYPE::churi: name = hrn ? "Channel URI" : "churi"; value_type = VALUE::val_string; break;
		case TYPE::etype: name = hrn ? "Favourite Type" : "etype"; value_type = VALUE::val_int; break;
		case TYPE::atype: name = hrn ? "Favourite Flag" : "atype"; value_type = VALUE::val_int; break;
		case TYPE::freq: name = hrn ? "Frequency" : "freq"; value_type = VALUE::val_int; break;
		case TYPE::sr: name = hrn ? "Symbol Rate" : "sr"; value_type = VALUE::val_int; break;
		case TYPE::pol: name = hrn ? "Polarization" : "pol"; value_type = hrv ? VALUE::val_string : VALUE::val_int; break;
		case TYPE::fec: name = hrn ? "FEC" : "fec"; value_type = hrv ? VALUE::val_string : VALUE::val_int; break;
		case TYPE::hpfec: name = hrn ? "HP FEC" : "hpfec"; value_type = hrv ? VALUE::val_string : VALUE::val_int; break;
		case TYPE::lpfec: name = hrn ? "LP FEC" : "lpfec"; value_type = hrv ? VALUE::val_string : VALUE::val_int; break;
		case TYPE::cfec: name = hrn ? "Inner FEC" : "cfec"; value_type = hrv ? VALUE::val_string : VALUE::val_int; break;
		case TYPE::inv: name = hrn ? "Sat Inversion" : "inv"; value_type = hrv ? VALUE::val_string : VALUE::val_int; break;
		case TYPE::tinv: name = hrn ? "Ter Inversion" : "tinv"; value_type = hrv ? VALUE::val_string : VALUE::val_int; break;
		case TYPE::cinv: name = hrn ? "Cab Inversion" : "cinv"; value_type = hrv ? VALUE::val_string : VALUE::val_int; break;
		case TYPE::sys: name = hrn ? "Sat System" : "sys"; value_type = hrv ? VALUE::val_string : VALUE::val_int; break;
		case TYPE::tsys: name = hrn ? "Ter System" : "sys"; value_type = hrv ? VALUE::val_string : VALUE::val_int; break;
		case TYPE::csys: name = hrn ? "Cab System" : "sys"; value_type = hrv ? VALUE::val_string : VALUE::val_int; break;
		case TYPE::asys: name = hrn ? "Atsc System" : "sys"; value_type = hrv ? VALUE::val_string : VALUE::val_int; break;
		case TYPE::mod: name = hrn ? "Sat Modulation" : "mod"; value_type = hrv ? VALUE::val_string : VALUE::val_int; break;
		case TYPE::tmod: name = hrn ? "Ter Constellation" : "tmod"; value_type = hrv ? VALUE::val_string : VALUE::val_int; break;
		case TYPE::cmod: name = hrn ? "Cab Modulation" : "cmod"; value_type = hrv ? VALUE::val_string : VALUE::val_int; break;
		case TYPE::amod: name = hrn ? "Atsc Modulation" : "amod"; value_type = hrv ? VALUE::val_string : VALUE::val_int; break;
		case TYPE::rol: name = hrn ? "Roll Offset" : "rol"; value_type = hrv ? VALUE::val_string : VALUE::val_int; break;
		case TYPE::pil: name = hrn ? "Pilot" : "pil"; value_type = hrv ? VALUE::val_string : VALUE::val_int; break;
		case TYPE::band: name = hrn ? "Bandwidth" : "band"; value_type = hrv ? VALUE::val_string : VALUE::val_int; break;
		case TYPE::tmx: name = hrn ? "Transmission Mode" : "tmx"; value_type = hrv ? VALUE::val_string : VALUE::val_int; break;
		case TYPE::guard: name = hrn ? "Guard Interval" : "guard"; value_type = hrv ? VALUE::val_string : VALUE::val_int; break;
		case TYPE::hier: name = hrn ? "Hierarchy" : "hier"; value_type = hrv ? VALUE::val_string : VALUE::val_int; break;
		case TYPE::plsn: name = hrn ? "plsn | mis id" : "plsn"; value_type = VALUE::val_int; break;
		case TYPE::plsmode: name = hrn ? "plsmode" : "plsmode"; value_type = VALUE::val_int; break;
		case TYPE::plscode: name = hrn ? "plscode" : "plscode"; value_type = VALUE::val_int; break;
		case TYPE::isid: name = hrn ? "is id" : "isid"; value_type = VALUE::val_int; break;
		case TYPE::mts: name = hrn ? "mts" : "mts"; value_type = VALUE::val_int; break;
		case TYPE::t2mi_plpid: name = hrn ? "t2mi plpid" : "t2mi_plpid"; value_type = VALUE::val_int; break;
		case TYPE::t2mi_pid: name = hrn ? "t2mi pid" : "t2mt_pid"; value_type = VALUE::val_int; break;
		case TYPE::pos: name = hrn ? "Position" : "pos"; value_type = hrv ? VALUE::val_string : VALUE::val_int; break;
		case TYPE::diseqc: name = hrn ? "diseqc" : "diseqc"; value_type = VALUE::val_int; break;
		case TYPE::uncomtd: name = hrn ? "uncomtd" : "uncomtd"; value_type = VALUE::val_int; break;
		case TYPE::charset: name = hrn ? "Charset" : "charset"; value_type = VALUE::val_string; break;
		case TYPE::tname: name = hrn ? "Position Name" : "tname"; value_type = VALUE::val_string; break;
		case TYPE::country: name = hrn ? "Country" : "country"; value_type = VALUE::val_string; break;
		case TYPE::feed: name = hrn ? "Feed" : "feed"; value_type = VALUE::val_int; break;
		case TYPE::bname: name = hrn ? "Bouquet Filename [bname]" : "bname"; value_type = VALUE::val_string; break;
		case TYPE::pname: name = hrn ? "Parent Bouquet [bname]" : "pname"; value_type = VALUE::val_string; break;
		case TYPE::rname: name = hrn ? "New Bouquet Name [rname]" : "rname"; value_type = VALUE::val_string; break;
		case TYPE::qname: name = hrn ? "Bouquet Name" : "qname"; value_type = VALUE::val_string; break;
		case TYPE::nname: name = hrn ? "Bouquet Nice Name" : "nname"; value_type = VALUE::val_string; break;
		case TYPE::btype: name = hrn ? "Bouquet Type" : "btype"; value_type = hrv ? VALUE::val_string : VALUE::val_int; break;
		case TYPE::hidden: name = hrn ? "Hidden" : "hidden"; value_type = VALUE::val_bool; break;
		case TYPE::locked: name = hrn ? "Locked" : "locked"; value_type = VALUE::val_bool; break;
		case TYPE::fname: name = hrn ? "Filename" : "fname"; value_type = VALUE::val_string; break;
		case TYPE::itype: name = hrn ? "Zapit Data Type" : "itype"; value_type = VALUE::val_int; break;
		case TYPE::flags: name = hrn ? "Flags" : "flags"; value_type = VALUE::val_int; break;
		case TYPE::chdata: name = hrn ? "Service flags" : "chdata"; value_type = VALUE::val_obj; break;
		case TYPE::txdata: name = hrn ? "Transponder flags" : "txdata"; value_type = VALUE::val_obj; break;
		case TYPE::ffdata: name = hrn ? "Favourite flags" : "txdata"; value_type = VALUE::val_obj; break;
		case TYPE::bsdata: name = hrn ? "Userbouquets" : "userbouquets"; value_type = VALUE::val_obj; break;
		case TYPE::ubdata: name = hrn ? "Channels" : "channels"; value_type = VALUE::val_obj; break;
		case TYPE::tvdata: name = hrn ? "Tables" : "tables"; value_type = VALUE::val_obj; break;
		case TYPE::tndata: name = hrn ? "Transponders" : "transponders"; value_type = VALUE::val_obj; break;
		default: return;
	}

	int d = -1;
	string str;

	switch (type)
	{
		case TYPE::idx:
		case TYPE::tvid:
		case TYPE::ytype:
		case TYPE::ssid:
		case TYPE::tsid:
		case TYPE::snum:
		case TYPE::srcid:
		case TYPE::freq:
		case TYPE::sr:
		case TYPE::plpid:
		case TYPE::isid:
		case TYPE::plscode:
		case TYPE::plsmode:
		case TYPE::t2mi_plpid:
		case TYPE::t2mi_pid:
		case TYPE::mts:
		case TYPE::plsn:
		case TYPE::diseqc:
		case TYPE::uncomtd:
		case TYPE::feed:
		case TYPE::itype:
		case TYPE::etype:
		case TYPE::atype:
		case TYPE::mnum:
		case TYPE::flags:
			d = any_cast<int>(val);
		break;
		case TYPE::parental:
		case TYPE::hidden:
		case TYPE::locked:
			d = any_cast<bool>(val);
			str = d ? "true" : "false";
		break;
		case TYPE::chid:
		case TYPE::txid:
		case TYPE::refid:
		case TYPE::tnid:
		case TYPE::trid:
		case TYPE::chname:
		case TYPE::mname:
		case TYPE::chvalue:
		case TYPE::churi:
		case TYPE::charset:
		case TYPE::tname:
		case TYPE::country:
		case TYPE::bname:
		case TYPE::pname:
		case TYPE::rname:
		case TYPE::qname:
		case TYPE::nname:
		case TYPE::fname:
			str = any_cast<string>(val);
		break;
		case TYPE::dbtype:
			d = any_cast<int>(val);
		break;
		case TYPE::dbparental:
			d = any_cast<int>(val);
		break;
		case TYPE::yname:
			d = any_cast<int>(val);
			if (value_type == VALUE::val_char)
				str = dbih->value_transponder_type(d);
		break;
		case TYPE::dvbns:
			d = any_cast<int>(val);
			if (value_type == VALUE::val_string)
				str = dbih->value_transponder_dvbns(d);
		break;
		case TYPE::onid:
			d = any_cast<int>(val);
			if (value_type == VALUE::val_string)
				str = dbih->value_transponder_onid(d);
		break;
		case TYPE::sys:
			d = any_cast<int>(val);
			if (value_type == VALUE::val_string)
				str = dbih->value_transponder_system(d, e2db::YTYPE::satellite);
		break;
		case TYPE::tsys:
			d = any_cast<int>(val);
			if (value_type == VALUE::val_string)
				str = dbih->value_transponder_system(d, e2db::YTYPE::terrestrial);
		break;
		case TYPE::csys:
			d = any_cast<int>(val);
			if (value_type == VALUE::val_string)
				str = dbih->value_transponder_system(d, e2db::YTYPE::cable);
		break;
		case TYPE::asys:
			d = any_cast<int>(val);
			if (value_type == VALUE::val_string)
				str = dbih->value_transponder_system(d, e2db::YTYPE::atsc);
		break;
		case TYPE::pos:
			d = any_cast<int>(val);
			if (value_type == VALUE::val_string)
				str = dbih->value_transponder_position(d);
		break;
		case TYPE::pol:
			d = any_cast<int>(val);
			if (value_type == VALUE::val_string)
				str = dbih->value_transponder_polarization(d);
		break;
		case TYPE::fec:
			d = any_cast<int>(val);
			if (value_type == VALUE::val_string)
				str = dbih->value_transponder_fec(d, e2db::YTYPE::satellite);
		break;
		case TYPE::hpfec:
		case TYPE::lpfec:
			d = any_cast<int>(val);
			if (value_type == VALUE::val_string)
				str = dbih->value_transponder_fec(d, e2db::YTYPE::terrestrial);
		break;
		case TYPE::cfec:
			d = any_cast<int>(val);
			if (value_type == VALUE::val_string)
				str = dbih->value_transponder_fec(d, e2db::YTYPE::atsc);
		break;
		case TYPE::mod:
			d = any_cast<int>(val);
			if (value_type == VALUE::val_string)
				str = dbih->value_transponder_modulation(d, e2db::YTYPE::satellite);
		break;
		case TYPE::tmod:
			d = any_cast<int>(val);
			if (value_type == VALUE::val_string)
				str = dbih->value_transponder_modulation(d, e2db::YTYPE::terrestrial);
		break;
		case TYPE::cmod:
			d = any_cast<int>(val);
			if (value_type == VALUE::val_string)
				str = dbih->value_transponder_modulation(d, e2db::YTYPE::cable);
		break;
		case TYPE::amod:
			d = any_cast<int>(val);
			if (value_type == VALUE::val_string)
				str = dbih->value_transponder_modulation(d, e2db::YTYPE::atsc);
		break;
		case TYPE::inv:
			d = any_cast<int>(val);
			if (value_type == VALUE::val_string)
				str = dbih->value_transponder_inversion(d, e2db::YTYPE::satellite);
		break;
		case TYPE::tinv:
			d = any_cast<int>(val);
			if (value_type == VALUE::val_string)
				str = dbih->value_transponder_inversion(d, e2db::YTYPE::terrestrial);
		break;
		case TYPE::cinv:
			d = any_cast<int>(val);
			if (value_type == VALUE::val_string)
				str = dbih->value_transponder_inversion(d, e2db::YTYPE::cable);
		break;
		case TYPE::rol:
			d = any_cast<int>(val);
			if (value_type == VALUE::val_string)
				str = dbih->value_transponder_rollof(d);
		break;
		case TYPE::pil:
			d = any_cast<int>(val);
			if (value_type == VALUE::val_string)
				str = dbih->value_transponder_pilot(d);
		break;
		case TYPE::band:
			d = any_cast<int>(val);
			if (value_type == VALUE::val_string)
				str = dbih->value_transponder_bandwidth(d);
		break;
		case TYPE::tmx:
			d = any_cast<int>(val);
			if (value_type == VALUE::val_string)
				str = dbih->value_transponder_tmx_mode(d);
		break;
		case TYPE::guard:
			d = any_cast<int>(val);
			if (value_type == VALUE::val_string)
				str = dbih->value_transponder_guard(d);
		break;
		case TYPE::hier:
			d = any_cast<int>(val);
			if (value_type == VALUE::val_string)
				str = dbih->value_transponder_hier(d);
		break;
		case TYPE::stype:
			d = any_cast<int>(val);
			if (value_type == VALUE::val_string)
				str = dbih->value_service_type(d);
		break;
		case TYPE::btype:
			d = any_cast<int>(val);
			if (value_type == VALUE::val_string)
				str = dbih->value_bouquet_type(d);
		break;

 		case TYPE::sdata_p:
 			{
 				vector<string> vx = any_cast<vector<string>>(val);

 				if (! vx.empty())
 				{
 					str = vx[0];
 				}
 			}
		break;
 		case TYPE::sdata_c:
 			{
 				vector<string> vx = any_cast<vector<string>>(val);

 				if (! vx.empty())
 				{
		 			string cached;

					for (size_t i = 0; i < vx.size(); i++)
					{
						cached.append(vx[i]);
						if (i != vx.size() - 1)
							cached.append("|");
					}

					str = cached;
				}
 			}
		break;
 		case TYPE::sdata_C:
 			{
 				vector<string> vx = any_cast<vector<string>>(val);

 				if (! vx.empty())
 				{
	 				string scas = "$";
	 				string scaid;
					vector<string> cas;

					for (string & w : vx)
					{
						string caidpx = w.substr(0, 2);
						if (e2db::SDATA_CAS.count(caidpx))
							cas.emplace_back(e2db::SDATA_CAS.at(caidpx) + ':' + w);
						else
							cas.emplace_back(w);
					}
					for (size_t i = 0; i < cas.size(); i++)
					{
						scaid.append(cas[i]);
						if (i != cas.size() - 1)
							scaid.append("|");
					}

					str.append(scas);
					str.append(" ");
					str.append(scaid);
				}
 			}
		break;
 		case TYPE::sdata_f:
 			{
 				vector<string> vx = any_cast<vector<string>>(val);

 				if (! vx.empty())
 				{
 					str = vx[0];
 				}
 			}
		break;

		default:
		break;
	}

	cout << obj_escape(ESCAPE::name_begin, value_type);
	cout << name;
	cout << obj_escape(ESCAPE::name_end, value_type);
	cout << obj_escape(ESCAPE::divider, value_type);
	cout << obj_escape(ESCAPE::value_begin, value_type);
	if (value_type == VALUE::val_int)
	{
		cout << d;
	}
	else if (value_type == VALUE::val_obj)
	{
	}
	else
	{
		cout << str;
	}
	cout << obj_escape(ESCAPE::value_end, value_type);
}

string e2db_cli::obj_escape(ESCAPE esc, VALUE value_type)
{
	if (__objio.out == OBJIO::json)
	{
		switch (esc)
		{ 
			case ESCAPE::name_begin:
			case ESCAPE::name_end:
				return "\"";
			break;
			case ESCAPE::value_begin:
			case ESCAPE::value_end:
				if (value_type == VALUE::val_char || value_type == VALUE::val_string)
					return "\"";
			break;
			case ESCAPE::divider:
				return ": ";
			break;
		}
	}
	else
	{
		switch (esc)
		{ 
			case ESCAPE::divider:
				return ": ";
			break;
			default:
			break;
		}
	}

	return "";
}

std::any e2db_cli::field(TYPE type, bool required)
{
	string label, description;

	switch (type)
	{
		case TYPE::dbtype: label = "Format"; description = "exact match: <empty>, 0 = Lamedb, 1 = Zapit"; break;
		case TYPE::dbparental: label = "Parental lock"; description = "exact match: <empty>, 0 = blacklist, 1 = whitelist, 2 = Enigma 1 format"; break;
		case TYPE::idx: label = "Index"; description = "Channel list number, in digits"; break;
		case TYPE::chid: label = "CHID"; description = "Channel ID [ssid]:[tsid]:[dvbns] eg. 4d2:3e8:eeee0000"; break;
		case TYPE::txid: label = "TXID"; description = "Transponder ID [tsid]:[dvbns] eg. 3e8:eeee0000"; break;
		case TYPE::refid: label = "REFID"; description = "Reference ID, colon separated values"; break;
		case TYPE::tnid: label = "TNID"; description = "Tunersets Table ID [yname]:[idx] eg. s:0001"; break;
		case TYPE::trid: label = "TRID"; description = "Tunersets Transponder ID [yname]:[freq]:[sr] eg. s:2710:55f0"; break;
		case TYPE::yname: label = "YNAME"; description = "Tuner Name, exact match: s = satellite, t = terrestrial, c = cable, a = atsc"; break;
		case TYPE::ytype: label = "YTYPE"; description = "Tuner Type, exact match: 0 = satellite, 1 = terrestrial, 2 = cable, 3 = atsc"; break;
		case TYPE::ssid: label = "SSID"; description = "Service ID, in digits"; break;
		case TYPE::dvbns: label = "DVBNS"; description = "DVB namespace, in hex"; break;
		case TYPE::tsid: label = "TSID"; description = "Transport ID, in digits"; break;
		case TYPE::onid: label = "ONID"; description = "Network ID, in digits"; break;
		case TYPE::stype: label = "Service Type"; description = "exact match: Data, TV, Radio, HD, H.264, H.265, UHD"; break;
		case TYPE::snum: label = "snum"; description = "Service Number, in digits"; break;
		case TYPE::srcid: label = "srcid"; description = "Source ID, in digits"; break;
		case TYPE::parental: label = "Parental locked"; description = "[Y]es or [N]one"; break;
		case TYPE::chname: label = "Service Name"; break;
		case TYPE::sdata_p: label = "Provider Name"; break;
		case TYPE::sdata_c: label = "Service Cache"; description = "comma separated values in hex or <empty>, eg. c:0101,c:0202"; break;
		case TYPE::sdata_C: label = "Service CAS"; description = "comma separated values in hex or <empty>, eg. C:0101,C:0202"; break;
		case TYPE::sdata_f: label = "Service Flags"; description = "comma separated values in hex or <empty>, eg. f:0101,f:0202"; break;
		case TYPE::mname: label = "Marker Name"; break;
		case TYPE::mnum: label = "Marker Number"; description = "in digits"; break;
		case TYPE::chvalue: label = "Channel Name"; break;
		case TYPE::churi: label = "Channel URI"; break;
		case TYPE::etype: label = "Favourite Type"; description = "exact match: 1 = broadcast, 2 = file, 3 = 4097, 8139 = youtube, 8193 = eservice"; break;
		case TYPE::atype: label = "Favourite Flag"; description = "exact match: 64 = marker, 512 = marker hidden, 832 = marker hidden, 320 = marker numbered, 128 = group"; break;
		case TYPE::freq: label = "Frequency"; description = "in Hertz, 6 digits"; break;
		case TYPE::sr: label = "Symbol Rate"; description = "in digits"; break;
		case TYPE::pol: label = "Polarization"; description = "exact match: H = horizontal, V = vertical, L = Left Circular, R = Right Circular"; break;
		case TYPE::fec: label = "FEC"; description = "exact match: <empty>, Auto, 1/2, 2/3, 3/4, 5/6, 7/8, 8/9, 3/5 4/5, 9/10, 6/7"; break;
		case TYPE::hpfec: label = "HP FEC"; description = "exact match: <empty>, Auto, 1/2, 2/3, 3/4, 5/6, 7/8, 6/7, 8/9, 3/5, 4/5"; break;
		case TYPE::lpfec: label = "LP FEC"; description = "exact match: <empty>, Auto, 1/2, 2/3, 3/4, 5/6, 7/8, 6/7, 8/9, 3/5, 4/5"; break;
		case TYPE::cfec: label = "Inner FEC"; description = "exact match: <empty>, Auto, 1/2, 2/3, 3/4, 5/6, 7/8, 8/9"; break;
		case TYPE::inv: case TYPE::tinv: case TYPE::cinv: label = "Inversion"; description = "exact match: <empty>, Off, On"; break;
		case TYPE::sys: case TYPE::tsys: case TYPE::csys: case TYPE::asys: label = "System"; description = "exact match: DVB-S, DVB-T, DVB-C, ATSC, DVB-S2, DVB-T2, DVB-S/S2, DVB-T/T2, DVB-C ANNEX B"; break;
		case TYPE::mod: label = "Modulation"; description = "exact match: <empty>, Auto, QPSK, 8PSK, QAM16, 16APSK, 32APSK"; break;
		case TYPE::tmod: label = "Constellation"; description = "exact match: <empty>, Auto, QPSK, QAM16, QAM64, QAM256"; break;
		case TYPE::cmod: label = "Modulation"; description = "exact match: <empty>, Auto, QAM16, QAM32, QAM64, QAM128, QAM256"; break;
		case TYPE::amod: label = "Modulation"; description = "exact match: <empty>, Auto, QAM16, QAM32, QAM64, QAM128, QAM256, 8VSB, 16VSB"; break;
		case TYPE::rol: label = "Roll Offset"; description = "exact match: <empty>, 0.35, 0.25, 0.20"; break;
		case TYPE::pil: label = "Pilot"; description = "exact match: <empty>, Off, On, Auto"; break;
		case TYPE::band: label = "Bandwidth"; description = "exact match: <empty>, Auto, 8MHz, 7MHz, 6MHz, 5MHz, 1.712MHz, 10MHz"; break;
		case TYPE::tmx: label = "Transmission Mode"; description = "exact match: <empty>, Auto, 2k, 8k, 4k, 1k, 16k, 32k"; break;
		case TYPE::guard: label = "Guard Interval"; description = "exact match: <empty>, Auto, 1/32, 1/16, 1/8, 1/4, 1/128, 19/128, 19/256"; break;
		case TYPE::hier: label = "Hierarchy"; description = "exact match: <empty>, 0, 1, 2, 4"; break;
		case TYPE::plpid: label = "plp id"; description = "in digits"; break;
		case TYPE::isid: label = "is id"; description = "in digits"; break;
		case TYPE::plscode: label = "plscode"; description = "in digits"; break;
		case TYPE::plsmode: label = "plsmode"; description = "in digits"; break;
		case TYPE::t2mi_plpid: label = "t2mi plpid"; description = "in digits"; break;
		case TYPE::t2mi_pid: label = "t2mi pid"; description = "in digits"; break;
		case TYPE::mts: label = "mts"; description = "in digits"; break;
		case TYPE::plsn: label = "plsn | mis id"; description = "in digits"; break;
		case TYPE::pos: label = "Position"; description = "in degree, eg. 0.0E, 0.0W"; break;
		case TYPE::diseqc: label = "diseqc"; description = "in digits"; break;
		case TYPE::uncomtd: label = "uncomtd"; description = "in digits"; break;
		case TYPE::charset: label = "Charset"; description = "characters encoding: UTF-8, ISO-8859-1"; break;
		case TYPE::tname: label = "Position Name"; description = "eg. Telstar 0.0E"; break;
		case TYPE::country: label = "Country"; description = "Country Code, 3 letters ISO-3166, eg. XYZ"; break;
		case TYPE::feed: label = "Feed"; description = "[Y]es or [N]one"; break;
		case TYPE::bname: label = "Bouquet Filename [bname]"; description = "eg. userbouquet.dbe01.tv, bouquet.radio"; break;
		case TYPE::pname: label = "Parent Bouquet [bname]"; description = "eg. bouquets.tv, bouquets.radio"; break;
		case TYPE::rname: label = "New Bouquet Name [rname]"; description = "eg. userbouquet.dbe01.tv, bouquet.radio"; break;
		case TYPE::qname: label = "Bouquet Name"; description = "eg. User - bouquet (TV)"; break;
		case TYPE::nname: label = "Bouquet Nice Name"; description = "eg. TV, Radio"; break;
		case TYPE::btype: label = "Bouquet Type"; description = "TV, Radio"; break;
		case TYPE::hidden: label = "Hidden"; description = "[Y]es or [N]one"; break;
		case TYPE::locked: label = "Locked"; description = "[Y]es or [N]one"; break;
		case TYPE::fname: label = "Filename"; break;
		case TYPE::itype: label = "Zapit Data Type"; description = "exact match: 0 = services, 1 = bouquets"; break;
		case TYPE::flags: label = "Flags"; description = "in digits"; break;
		case TYPE::chdata: label = "Add optional Service flags?"; description = "[Y]es or [N]one"; break;
		case TYPE::txdata: label = "Add optional Transponder flags?"; description = "[Y]es or [N]one"; break;
		case TYPE::ffdata: label = "Add optional Favourite flags?"; description = "[Y]es or [N]one"; break;
		default: return -1;
	}

	e2db_termctl* term = new e2db_termctl;

	while (true)
	{
		cout << label;
		if (! description.empty())
			cout << ' ' << '(' << description << ')';
		if (required)
			cout << ' ' << '*';
		cout << ':' << ' ';

		term->input();
		string str = term->str();
		term->clear();

		// failsafe string trim
		str.erase(0, str.find_first_not_of(" \n\r\t\v\b\f"));
		str.erase(str.find_last_not_of(" \n\r\t\v\b\f") + 1);

		this->last_label = label;

		if (! str.empty())
		{
			int d = -1;

			switch (type)
			{
				case TYPE::idx:
				case TYPE::ssid:
				case TYPE::tsid:
				case TYPE::onid:
				case TYPE::snum:
				case TYPE::srcid:
				case TYPE::freq:
				case TYPE::sr:
				case TYPE::plpid:
				case TYPE::isid:
				case TYPE::plscode:
				case TYPE::plsmode:
				case TYPE::t2mi_plpid:
				case TYPE::t2mi_pid:
				case TYPE::mts:
				case TYPE::plsn:
				case TYPE::diseqc:
				case TYPE::uncomtd:
				case TYPE::etype:
				case TYPE::atype:
				case TYPE::mnum:
				case TYPE::flags:
					d = std::atoi(str.data());
					if (! d && required)
						continue;
				break;
				case TYPE::dvbns:
					d = std::stol(str, nullptr, 16);
					if (d == 0 && required)
						continue;
				break;
				case TYPE::dbtype:
					d = std::atoi(str.data());
					if (d < 0 || d > 1 && required)
						continue;
				break;
				case TYPE::dbparental:
					d = std::atoi(str.data());
					if (d < 0 || d > 2 && required)
						continue;
				break;
				case TYPE::ytype:
					d = std::atoi(str.data());
					if (d < 0 || d > 3 && required)
						continue;
				break;
				case TYPE::chdata:
				case TYPE::txdata:
				case TYPE::ffdata:
				case TYPE::parental:
				case TYPE::feed:
				case TYPE::hidden:
				case TYPE::locked:
					if (str == "Y" || str == "y")
						d = 1;
					else if (str == "N" || str == "n")
						d = 0;
					else if (required)
						continue;
				break;
				case TYPE::sdata_p:
					return { str };
				break;
				case TYPE::yname:
					if (str.size() == 1)
						d = dbih->value_transponder_type(str[0]);
					else if (required)
						continue;
				break;
				case TYPE::sys:
					// failsafe string uppercase
					std::transform(str.begin(), str.end(), str.begin(), [](unsigned char c) { return std::toupper(c); });
					d = dbih->value_transponder_system(str);
				break;
				case TYPE::pos:
					// failsafe string uppercase
					std::transform(str.begin(), str.end(), str.begin(), [](unsigned char c) { return std::toupper(c); });
					d = dbih->value_transponder_position(str);
				break;
				case TYPE::pol:
					// failsafe string uppercase
					std::transform(str.begin(), str.end(), str.begin(), [](unsigned char c) { return std::toupper(c); });
					d = dbih->value_transponder_polarization(str);
				break;
				case TYPE::fec:
					d = dbih->value_transponder_fec(str, e2db::YTYPE::satellite);
				break;
				case TYPE::hpfec:
				case TYPE::lpfec:
					d = dbih->value_transponder_fec(str, e2db::YTYPE::terrestrial);
				break;
				case TYPE::cfec:
					d = dbih->value_transponder_fec(str, e2db::YTYPE::atsc);
				break;
				case TYPE::mod:
					d = dbih->value_transponder_modulation(str, e2db::YTYPE::satellite);
				break;
				case TYPE::tmod:
					d = dbih->value_transponder_modulation(str, e2db::YTYPE::terrestrial);
				break;
				case TYPE::cmod:
					d = dbih->value_transponder_modulation(str, e2db::YTYPE::cable);
				break;
				case TYPE::amod:
					d = dbih->value_transponder_modulation(str, e2db::YTYPE::atsc);
				break;
				case TYPE::inv:
					d = dbih->value_transponder_inversion(str, e2db::YTYPE::satellite);
				break;
				case TYPE::tinv:
					d = dbih->value_transponder_inversion(str, e2db::YTYPE::terrestrial);
				break;
				case TYPE::cinv:
					d = dbih->value_transponder_inversion(str, e2db::YTYPE::cable);
				break;
				case TYPE::rol:
					d = dbih->value_transponder_rollof(str);
				break;
				case TYPE::pil:
					d = dbih->value_transponder_pilot(str);
				break;
				case TYPE::band:
					d = dbih->value_transponder_bandwidth(str);
				break;
				case TYPE::tmx:
					d = dbih->value_transponder_tmx_mode(str);
				break;
				case TYPE::guard:
					d = dbih->value_transponder_guard(str);
				break;
				case TYPE::hier:
					d = dbih->value_transponder_hier(str);
				break;
				case TYPE::stype:
					d = dbih->value_service_type(str);
				break;
				case TYPE::btype:
					d = dbih->value_bouquet_type(str);
				break;
				case TYPE::country:
					if (str.size() < 3 || str.size() > 3 && required)
						continue;
					// failsafe string uppercase
					std::transform(str.begin(), str.end(), str.begin(), [](unsigned char c) { return std::toupper(c); });
					return str;
				break;
				case TYPE::charset:
					// failsafe string uppercase
					std::transform(str.begin(), str.end(), str.begin(), [](unsigned char c) { return std::toupper(c); });
					return str;
				break;
				default:
					return str;
			}

			if (d == -1 && required)
				continue;
			else
				return d;
		}
		else if (required)
		{
			continue;
		}
		else
		{
			break;
		}
	}

	return -1;
}

}
