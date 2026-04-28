/*!
 * e2-sat-editor/src/e2db/e2db_console.cpp
 *
 * @link https://github.com/ctlcltd/e2-sat-editor
 * @copyright e2 SAT Editor Team
 * @author Leonardo Laureti
 * @version 2.0.0
 * @license MIT License
 * @license GNU GPLv3 License
 */

#include <cstdio>
#include <clocale>
#include <cstring>
#include <cctype>
#include <algorithm>
#include <fstream>
#include <iomanip>
#include <filesystem>

// note: std::runtime_error behaviour
#include <stdexcept>

#include "e2db_console.h"

using std::pair, std::ifstream, std::ofstream;

namespace e2se_e2db
{

string e2db_console::editor_version()
{
	return "2.0.0";
}

void e2db_console::console_exit()
{
	termctl->reset();
}

void e2db_console::console_header()
{
	console_version(true);
	*pout << pout->endl();
	*pout << "Enter \"help\" to display usage informations." << pout->endl();
	*pout << pout->endl();
}

void e2db_console::console_error(const string& cmd)
{
	*perr << "Error: " << msg("Syntax error near: %s", cmd) << pout->endl();
}

void e2db_console::console_version(bool extended)
{
	*pout << "e2-sat-editor" << ' ' << editor_version() << pout->endl();
}

void e2db_console::console_resolver(COMMAND command, istream* is)
{
	string src;
	*is >> std::skipws >> src;

	if (command == COMMAND::usage)
	{
		string hint;
		*is >> std::skipws >> hint;

		if (hint.empty())
			console_usage(COMMAND::usage);
		else if (hint == "read")
			console_usage(COMMAND::fread);
		else if (hint == "write")
			console_usage(COMMAND::fwrite);
		else if (hint == "list")
			console_usage(COMMAND::list);
		else if (hint == "add")
			console_usage(COMMAND::add);
		else if (hint == "edit")
			console_usage(COMMAND::edit);
		else if (hint == "remove")
			console_usage(COMMAND::remove);
		// else if (hint == "copy")
		// 	console_usage(COMMAND::copy);
		// else if (hint == "move")
		// 	console_usage(COMMAND::move);
		else if (hint == "set")
			console_usage(COMMAND::set);
		else if (hint == "unset")
			console_usage(COMMAND::unset);
		else if (hint == "import")
			console_usage(COMMAND::fimport);
		else if (hint == "export")
			console_usage(COMMAND::fexport);
		else if (hint == "merge")
			console_usage(COMMAND::merge);
		else if (hint == "print")
			console_usage(COMMAND::print);
		else if (hint == "parse")
			console_usage(COMMAND::parse);
		else if (hint == "make")
			console_usage(COMMAND::make);
		else if (hint == "convert")
			console_usage(COMMAND::convert);
		else if (hint == "tool")
			console_usage(COMMAND::tool);
		else if (hint == "macro")
			console_usage(COMMAND::macro);
		else if (hint == "inspect")
			console_usage(COMMAND::inspect);
		else if (hint == "preferences")
			console_usage(COMMAND::preferences);
		else
		{
			*perr << "Type Error: " << msg("Unknown usage type: %s", hint) << pout->endl();
			*pout << pout->endl();

			console_usage(COMMAND::usage);
		}
	}
	else if (command == COMMAND::fread)
	{
		string path;
		*is >> std::skipws >> path;

		if (path.empty())
			console_usage(command);
		else
			console_file_read(path);
	}
	else if (command == COMMAND::fwrite)
	{
		string path;
		*is >> std::skipws >> path;

		console_file_write(path);
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
			entry_list(ENTRY::service, offset0, offset1);
		else if (type == "transponders")
			entry_list(ENTRY::transponder, offset0, offset1);
		else if (type == "bouquets")
			entry_list(ENTRY::bouquet, offset0, offset1);
		else if (type == "userbouquets")
			entry_list(ENTRY::userbouquet, offset0, offset1);
		else if (type == "tunersets_transponders")
			entry_list(ENTRY::tunersets_transponder, offset0, offset1);
		else if (type == "tunersets_tables")
			entry_list(ENTRY::tunersets_table, offset0, offset1);
		else if (type == "tunersets")
			entry_list(ENTRY::tunersets, offset0, offset1);
		else if (type == "channels")
			entry_list(ENTRY::channel_reference, bname, offset0, offset1);
		else if (type.empty())
			console_usage(command);
		else
			*perr << "Type Error: " << msg("Unknown entry type: %s", type) << pout->endl();
	}
	else if (command == COMMAND::add)
	{
		string type, bname;
		*is >> std::skipws >> type >> bname;

		if (type == "service")
			entry_add(ENTRY::service);
		else if (type == "transponder")
			entry_add(ENTRY::transponder);
		else if (type == "bouquet")
			entry_add(ENTRY::bouquet);
		else if (type == "userbouquet")
			entry_add(ENTRY::userbouquet);
		else if (type == "tunersets_transponder")
			entry_add(ENTRY::tunersets_transponder);
		else if (type == "tunersets_table")
			entry_add(ENTRY::tunersets_table);
		else if (type == "tunersets")
			entry_add(ENTRY::tunersets);
		else if (type == "channel")
			entry_add(ENTRY::channel_reference, 1, bname);
		else if (type == "marker")
			entry_add(ENTRY::channel_reference, 0, bname);
		else if (type == "stream")
			entry_add(ENTRY::channel_reference, 2, bname);
		else if (type.empty())
			console_usage(command);
		else
			*perr << "Type Error: " << msg("Unknown entry type: %s", type) << pout->endl();
	}
	else if (command == COMMAND::edit)
	{
		string type, id, bname;
		*is >> std::skipws >> type >> id >> bname;

		if (type == "service")
			entry_edit(ENTRY::service, id);
		else if (type == "transponder")
			entry_edit(ENTRY::transponder, id);
		else if (type == "bouquet")
			entry_edit(ENTRY::bouquet, id);
		else if (type == "userbouquet")
			entry_edit(ENTRY::userbouquet, id);
		else if (type == "tunersets_transponder")
			entry_edit(ENTRY::tunersets_transponder, id);
		else if (type == "tunersets_table")
			entry_edit(ENTRY::tunersets_table, id);
		else if (type == "tunersets")
			entry_edit(ENTRY::tunersets, id);
		else if (type == "channel")
			entry_edit(ENTRY::channel_reference, 1, bname, id);
		else if (type == "marker")
			entry_edit(ENTRY::channel_reference, 0, bname, id);
		else if (type == "stream")
			entry_edit(ENTRY::channel_reference, 2, bname, id);
		else if (type.empty())
			console_usage(command);
		else
			*perr << "Type Error: " << msg("Unknown entry type: %s", type) << pout->endl();
	}
	else if (command == COMMAND::remove)
	{
		string type, id, bname;
		*is >> std::skipws >> type >> id >> bname;

		if (type == "service")
			entry_remove(ENTRY::service, id);
		else if (type == "transponder")
			entry_remove(ENTRY::transponder, id);
		else if (type == "bouquet")
			entry_remove(ENTRY::bouquet, id);
		else if (type == "userbouquet")
			entry_remove(ENTRY::tunersets_transponder, id);
		else if (type == "tunersets_transponder")
			entry_remove(ENTRY::tunersets_transponder, id);
		else if (type == "tunersets_table")
			entry_remove(ENTRY::tunersets_table, id);
		else if (type == "tunersets")
			entry_remove(ENTRY::tunersets, id);
		else if (type == "channel")
			entry_remove(ENTRY::channel_reference, 1, bname, id);
		else if (type == "marker")
			entry_remove(ENTRY::channel_reference, 0, bname, id);
		else if (type.empty())
			console_usage(command);
		else
			*perr << "Type Error: " << msg("Unknown entry type: %s", type) << pout->endl();
	}
	else if (command == COMMAND::set)
	{
		string type, id;
		*is >> std::skipws >> type >> id;

		if (type == "service")
			entry_parentallock(ENTRY::service, id, true);
		else if (type == "userbouquet")
			entry_parentallock(ENTRY::userbouquet, id, true);
		else if (type.empty())
			console_usage(command);
		else
			*perr << "Type Error: " << msg("Unknown entry type: %s", type) << pout->endl();
	}
	else if (command == COMMAND::unset)
	{
		string type, id;
		*is >> std::skipws >> type >> id;

		if (type == "service")
			entry_parentallock(ENTRY::service, id, false);
		else if (type == "userbouquet")
			entry_parentallock(ENTRY::userbouquet, id, false);
		else if (type.empty())
			console_usage(command);
		else
			*perr << "Type Error: " << msg("Unknown entry type: %s", type) << pout->endl();
	}
	else if (command == COMMAND::print)
	{
		string type;
		*is >> std::skipws >> type;

		if (type == "debug")
			console_print(0);
		else if (type == "index")
			console_print(1);
		else if (type.empty())
			console_usage(command);
		else
			*perr << "Type Error: " << msg("Unknown print type: %s", type) << pout->endl();
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
		try { ver = std::stoi(optx); paths.pop_back(); }
		catch (...) {}

		if (type == "enigma")
			console_e2db_import(ENTRY::lamedb, paths);
		else if (type == "neutrino")
			console_e2db_import(ENTRY::zapit, paths);
		else if (type == "lamedb")
			console_e2db_import(ENTRY::lamedb_services, paths, ver);
		else if (type == "bouquets")
			console_e2db_import(ENTRY::bouquet, paths);
		else if (type == "userbouquets")
			console_e2db_import(ENTRY::userbouquet, paths);
		else if (type == "zapit services")
			console_e2db_import(ENTRY::zapit_services, paths, ver);
		else if (type == "zapit bouquets")
			console_e2db_import(ENTRY::zapit_bouquets, paths, ver);
		else if (type == "tunersets")
			console_e2db_import(ENTRY::tunersets, paths);
		else if (type == "parentallock blacklist")
			console_e2db_import(ENTRY::parentallock_blacklist, paths);
		else if (type == "parentallock whitelist")
			console_e2db_import(ENTRY::parentallock_whitelist, paths);
		else if (type == "parentallock locked")
			console_e2db_import(ENTRY::parentallock_locked, paths);
		else if (type.empty())
			console_usage(command);
		else
			*perr << "Type Error: " << msg("Unknown entry type: %s", type) << pout->endl();
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
		try { ver = std::stoi(optx); paths.pop_back(); }
		catch (...) {}

		if (type == "enigma")
			console_e2db_export(ENTRY::lamedb, paths, -1, true);
		else if (type == "neutrino")
			console_e2db_export(ENTRY::zapit, paths, -1, true);
		else if (type == "lamedb")
			console_e2db_export(ENTRY::lamedb_services, paths, ver);
		else if (type == "bouquet")
			console_e2db_export(ENTRY::bouquet, paths, ver, false, bname);
		else if (type == "bouquets")
			console_e2db_export(ENTRY::bouquet, paths, ver, true);
		else if (type == "userbouquet")
			console_e2db_export(ENTRY::userbouquet, paths, -1, false, bname);
		else if (type == "userbouquets")
			console_e2db_export(ENTRY::userbouquet, paths, -1, true);
		else if (type == "zapit services")
			console_e2db_export(ENTRY::zapit_services, paths, ver);
		else if (type == "zapit bouquets")
			console_e2db_export(ENTRY::zapit_bouquets, paths, ver);
		else if (type == "tunersets")
			console_e2db_export(ENTRY::tunersets, paths);
		else if (type == "parentallock blacklist")
			console_e2db_export(ENTRY::parentallock_blacklist, paths);
		else if (type == "parentallock whitelist")
			console_e2db_export(ENTRY::parentallock_whitelist, paths);
		else if (type == "parentallock locked")
			console_e2db_export(ENTRY::parentallock_locked, paths);
		else if (type == "parentallock")
			console_e2db_export(ENTRY::parentallock, paths);
		else if (type.empty())
			console_usage(command);
		else
			*perr << "Type Error: " << msg("Unknown entry type: %s", type) << pout->endl();
	}
	else if (command == COMMAND::merge)
	{
		string opt0, opt1, opt2, opt3;
		string type, path, bname0, bname1;
		int ver = -1;
		*is >> std::skipws >> opt0 >> opt1 >> opt2 >> opt3;

		if ((opt0 == "zapit" || opt0 == "parentallock") && ! opt1.empty())
		{
			type = opt0 + ' ' + opt1, path = opt2;
			try { ver = std::stoi(opt3); } catch (...) {}
		}
		else if ((opt0 == "bouquets" || opt0 == "userbouquets") && ! opt2.empty())
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
			console_e2db_merge(ENTRY::lamedb, path, -1, true);
		else if (type == "neutrino")
			console_e2db_merge(ENTRY::zapit, path, -1, true);
		else if (type == "lamedb")
			console_e2db_merge(ENTRY::lamedb_services, path, ver);
		else if (type == "bouquet")
			console_e2db_merge(ENTRY::bouquet, path, ver);
		else if (type == "bouquets" && path.empty())
			console_e2db_merge(ENTRY::bouquet, path, ver, true);
		else if (type == "bouquets")
			console_e2db_merge(ENTRY::bouquet, ver, bname0, bname1);
		else if (type == "userbouquet")
			console_e2db_merge(ENTRY::userbouquet, path);
		else if (type == "userbouquets" && path.empty())
			console_e2db_merge(ENTRY::userbouquet, -1, bname0, bname1);
		else if (type == "userbouquets")
			console_e2db_merge(ENTRY::userbouquet, path, -1, true);
		else if (type == "zapit services")
			console_e2db_merge(ENTRY::zapit_services, path, ver);
		else if (type == "zapit bouquets")
			console_e2db_merge(ENTRY::zapit_bouquets, path, ver);
		else if (type == "tunersets")
			console_e2db_merge(ENTRY::tunersets, path);
		else if (type == "parentallock blacklist")
			console_e2db_merge(ENTRY::parentallock_blacklist, path);
		else if (type == "parentallock whitelist")
			console_e2db_merge(ENTRY::parentallock_whitelist, path);
		else if (type == "parentallock locked")
			console_e2db_merge(ENTRY::parentallock_locked, path);
		else if (type.empty())
			console_usage(command);
		else
			*perr << "Type Error: " << msg("Unknown entry type: %s", type) << pout->endl();
	}
	else if (command == COMMAND::parse)
	{
		string opt0, opt1, opt2, opt3;
		string type, path;
		int ver = -1;
		*is >> std::skipws >> opt0 >> opt1 >> opt2 >> opt3;

		if ((opt0 == "zapit" || opt0 == "parentallock") && ! opt1.empty())
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
			console_e2db_parse(ENTRY::lamedb, path, -1, true);
		else if (type == "neutrino")
			console_e2db_parse(ENTRY::zapit, path, -1, true);
		else if (type == "lamedb")
			console_e2db_parse(ENTRY::lamedb_services, path, ver);
		else if (type == "bouquet")
			console_e2db_parse(ENTRY::bouquet, path);
		else if (type == "userbouquet")
			console_e2db_parse(ENTRY::userbouquet, path);
		else if (type == "zapit services")
			console_e2db_parse(ENTRY::zapit_services, path, ver);
		else if (type == "zapit bouquets")
			console_e2db_parse(ENTRY::zapit_bouquets, path, ver);
		else if (type == "tunersets")
			console_e2db_parse(ENTRY::tunersets, path);
		else if (type == "parentallock blacklist")
			console_e2db_parse(ENTRY::parentallock_blacklist, path);
		else if (type == "parentallock whitelist")
			console_e2db_parse(ENTRY::parentallock_whitelist, path);
		else if (type == "parentallock locked")
			console_e2db_parse(ENTRY::parentallock_locked, path);
		else if (type.empty())
			console_usage(command);
		else
			*perr << "Type Error: " << msg("Unknown entry type: %s", type) << pout->endl();
	}
	else if (command == COMMAND::make)
	{
		string opt0, opt1, opt2, opt3;
		string type, path, bname;
		int ver = -1;
		*is >> std::skipws >> opt0 >> opt1 >> opt2 >> opt3;

		if ((opt0 == "zapit" || opt0 == "parentallock") && ! opt1.empty())
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
			console_e2db_make(ENTRY::lamedb, path, ver, true);
		else if (type == "neutrino")
			console_e2db_make(ENTRY::zapit, path, ver, true);
		else if (type == "lamedb")
			console_e2db_make(ENTRY::lamedb_services, path, ver);
		else if (type == "bouquet")
			console_e2db_make(ENTRY::bouquet, path, ver, false, bname);
		else if (type == "bouquets")
			console_e2db_make(ENTRY::bouquet, path, ver, true);
		else if (type == "userbouquet")
			console_e2db_make(ENTRY::userbouquet, path, ver, false, bname);
		else if (type == "userbouquets")
			console_e2db_make(ENTRY::userbouquet, path, ver, true);
		else if (type == "zapit services")
			console_e2db_make(ENTRY::zapit_services, path, ver);
		else if (type == "zapit bouquets")
			console_e2db_make(ENTRY::zapit_bouquets, path, ver);
		else if (type == "tunersets")
			console_e2db_make(ENTRY::tunersets, path);
		else if (type == "parentallock blacklist")
			console_e2db_make(ENTRY::parentallock_blacklist, path);
		else if (type == "parentallock whitelist")
			console_e2db_make(ENTRY::parentallock_whitelist, path);
		else if (type == "parentallock locked")
			console_e2db_make(ENTRY::parentallock_locked, path);
		else if (type == "parentallock")
			console_e2db_make(ENTRY::parentallock, path);
		else if (type.empty())
			console_usage(command);
		else
			*perr << "Type Error: " << msg("Unknown entry type: %s", type) << pout->endl();
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
			console_e2db_convert(ENTRY::index, fopt, ftype, path);
		else if (type == "all")
			console_e2db_convert(ENTRY::all, fopt, ftype, path);
		else if (type == "services")
			console_e2db_convert(ENTRY::service, fopt, ftype, path, "", stype);
		else if (type == "bouquets")
			console_e2db_convert(ENTRY::bouquet, fopt, ftype, path);
		else if (type == "bouquet")
			console_e2db_convert(ENTRY::bouquet, fopt, ftype, path, bname);
		else if (type == "userbouquets")
			console_e2db_convert(ENTRY::userbouquet, fopt, ftype, path);
		else if (type == "userbouquet")
			console_e2db_convert(ENTRY::userbouquet, fopt, ftype, path, bname);
		else if (type == "tunersets")
			console_e2db_convert(ENTRY::tunersets, fopt, ftype, path, "", -1, ytype);
		else if (type.empty())
			console_e2db_convert(ENTRY::all, fopt, ftype, path);
		else
			*perr << "Type Error: " << msg("Unknown entry type: %s", type) << pout->endl();
	}
	else if (command == COMMAND::tool)
	{
		string fn, opt1, opt2, opt3;
		*is >> std::skipws >> fn >> opt1 >> opt2 >> opt3;

		if (fn == "sort_references")
			console_e2db_tool(fn, opt1, opt2, opt3 == "desc");
		else if (fn.find("sort_") != string::npos)
			console_e2db_tool(fn, "", opt1, opt2 == "desc");
		else if (! fn.empty())
			console_e2db_tool(fn);
		else
			console_usage(command);
	}
	else if (command == COMMAND::macro)
	{
		istream* cp_is = new istream (is->rdbuf());
		string opts;
		*cp_is >> opts;

		if (opts.find(',') != string::npos)
		{
			vector<string> pattern;
			string fn;

			//TODO FIX *is maybe empty
			while (std::getline(*is, fn, ','))
				pattern.emplace_back(fn);

			console_e2db_macro(pattern);
		}
		else if (! opts.empty())
		{
			string id = opts;

			console_e2db_macro(id);
		}
		else
		{
			console_usage(command);
		}
	}
	else if (command == COMMAND::inspect)
	{
		console_inspect();
	}
	else if (command == COMMAND::preferences)
	{
		string type, opt1;
		*is >> std::skipws >> type >> opt1;

		if (type.empty() || type == "output" || type == "history")
			console_preferences(type, opt1);
		else
			*perr << "Type Error: " << msg("Unknown entry type: %s", type) << pout->endl();
	}
	else
	{
		*perr << "Error: " << msg("Unknown command") << pout->endl();
	}
}

void e2db_console::console_usage(COMMAND hint, int level)
{
	if (hint == COMMAND::usage)
	{
		console_usage(COMMAND::add, 0);
		console_usage(COMMAND::edit, 0);
		console_usage(COMMAND::remove, 0);
		console_usage(COMMAND::list, 0);
		console_usage(COMMAND::set, 0);
		console_usage(COMMAND::unset, 0);
		console_usage(COMMAND::fread, 0);
		console_usage(COMMAND::fwrite, 0);
		console_usage(COMMAND::print, 0);
		console_usage(COMMAND::inspect, 0);
		console_usage(COMMAND::preferences, 0);
		console_usage(COMMAND::version, 0);
		console_usage(COMMAND::quit, 0);

		*pout << "  ", pout->width(36), *pout << pout->left() << "help", *pout << ' ' << "Display usage hints." << pout->endl();
		pout->width(10), *pout << ' ', *pout << "list" << pout->endl();
		pout->width(10), *pout << ' ', *pout << "import" << pout->endl();
		pout->width(10), *pout << ' ', *pout << "export" << pout->endl();
		pout->width(10), *pout << ' ', *pout << "merge" << pout->endl();
		pout->width(10), *pout << ' ', *pout << "parse" << pout->endl();
		pout->width(10), *pout << ' ', *pout << "make" << pout->endl();
		pout->width(10), *pout << ' ', *pout << "convert" << pout->endl();
		pout->width(10), *pout << ' ', *pout << "tool" << pout->endl();
		pout->width(10), *pout << ' ', *pout << "macro" << pout->endl();
		pout->width(10), *pout << ' ', *pout << "preferences" << pout->endl();
	}
	else if (hint == COMMAND::version)
	{
		*pout << "  ", pout->width(36), *pout << pout->left() << "version", *pout << ' ' << "Display version." << pout->endl();
		*pout << pout->endl();
	}
	else if (hint == COMMAND::quit)
	{
		*pout << "  ", pout->width(36), *pout << pout->left() << "quit", *pout << ' ' << "Exit." << pout->endl();
		*pout << pout->endl();
	}
	else if (hint == COMMAND::add)
	{
		*pout << "  ", pout->width(7), *pout << pout->left() << "add", *pout << ' ';
		pout->width(28), *pout << pout->left() << "transponder", *pout << ' ' << "Add new entry." << pout->endl();
		pout->width(10), *pout << ' ', *pout << "service" << pout->endl();
		pout->width(10), *pout << ' ', *pout << "bouquet" << pout->endl();
		pout->width(10), *pout << ' ', *pout << "userbouquet" << pout->endl();
		pout->width(10), *pout << ' ', *pout << "tunersets" << pout->endl();
		pout->width(10), *pout << ' ', *pout << "tunersets-table" << pout->endl();
		pout->width(10), *pout << ' ', *pout << "tunersets-transponder" << pout->endl();
		pout->width(10), *pout << ' ', pout->width(28), *pout << pout->left() << "channel  [chid] [bname]", *pout << ' ' << "Add service reference to userbouquet." << pout->endl();
		pout->width(10), *pout << ' ', pout->width(28), *pout << pout->left() << "marker  [bname]", *pout << ' ' << "Add marker to userbouquet." << pout->endl();
		pout->width(10), *pout << ' ', pout->width(28), *pout << pout->left() << "stream  [bname]", *pout << ' ' << "Add stream to userbouquet." << pout->endl();
		*pout << pout->endl();
	}
	else if (hint == COMMAND::edit)
	{
		*pout << "  ", pout->width(7), *pout << pout->left() << "edit", *pout << ' ';
		pout->width(28), *pout << pout->left() << "transponder  [txid]", *pout << ' ' << "Edit an entry." << pout->endl();
		pout->width(10), *pout << ' ', *pout << "service  [txid]" << pout->endl();
		pout->width(10), *pout << ' ', *pout << "bouquet  [bname]" << pout->endl();
		pout->width(10), *pout << ' ', *pout << "userbouquet  [bname]" << pout->endl();
		pout->width(10), *pout << ' ', *pout << "tunersets  [ytype]" << pout->endl();
		pout->width(10), *pout << ' ', *pout << "tunersets-table  [tnid]" << pout->endl();
		pout->width(10), *pout << ' ', *pout << "tunersets-transponder  [trid]" << pout->endl();
		pout->width(10), *pout << ' ', pout->width(28), *pout << pout->left() << "marker  [chid] [bname]", *pout << ' ' << "Edit marker from userbouquet." << pout->endl();
		*pout << pout->endl();
	}
	else if (hint == COMMAND::remove)
	{
		*pout << "  ", pout->width(7), *pout << pout->left() << "remove", *pout << ' ';
		pout->width(28), *pout << pout->left() << "transponder  [txid]", *pout << ' ' << "Remove an entry." << pout->endl();
		pout->width(10), *pout << ' ', *pout << "service  [txid]" << pout->endl();
		pout->width(10), *pout << ' ', *pout << "bouquet  [bname]" << pout->endl();
		pout->width(10), *pout << ' ', *pout << "userbouquet  [bname]" << pout->endl();
		pout->width(10), *pout << ' ', *pout << "tunersets  [ytype]" << pout->endl();
		pout->width(10), *pout << ' ', *pout << "tunersets-table  [tnid]" << pout->endl();
		pout->width(10), *pout << ' ', *pout << "tunersets-transponder  [trid]" << pout->endl();
		pout->width(10), *pout << ' ', pout->width(28), *pout << pout->left() << "channel  [chid] [bname]", *pout << ' ' << "Remove service reference from userbouquet." << pout->endl();
		pout->width(10), *pout << ' ', pout->width(28), *pout << pout->left() << "marker  [chid] [bname]", *pout << ' ' << "Remove marker from userbouquet." << pout->endl();
		pout->width(10), *pout << ' ', pout->width(28), *pout << pout->left() << "stream  [chid] [bname]", *pout << ' ' << "Remove stream from userbouquet." << pout->endl();
		*pout << pout->endl();
	}
	else if (hint == COMMAND::list)
	{
		*pout << "  ", pout->width(7), *pout << pout->left() << "list", *pout << ' ';
		pout->width(28), *pout << pout->left() << "transponders", *pout << ' ' << "List entries." << pout->endl();
		pout->width(10), *pout << ' ', *pout << "services" << pout->endl();
		pout->width(10), *pout << ' ', *pout << "bouquets" << pout->endl();
		pout->width(10), *pout << ' ', *pout << "userbouquets" << pout->endl();
		pout->width(10), *pout << ' ', *pout << "tunersets" << pout->endl();
		pout->width(10), *pout << ' ', *pout << "tunersets-tables" << pout->endl();
		pout->width(10), *pout << ' ', *pout << "tunersets-transponders" << pout->endl();
		pout->width(10), *pout << ' ', pout->width(28), *pout << pout->left() << "channels  [bname]", *pout << ' ' << "List channels from userbouquet." << pout->endl();
		*pout << pout->endl();

		if (level & 1)
		{
			*pout << "  ", *pout << "USAGE" << pout->endl() << pout->endl();
			*pout << "  ", pout->width(7), *pout << pout->left() << "list", *pout << ' ';
			pout->width(28), *pout << pout->left() << "[ENTRY] [limit]", *pout << ' ' << "Set the number of items to display per page." << pout->endl();
			pout->width(10), *pout << ' ', pout->width(24), *pout << pout->left() << "[ENTRY] [start] [limit]";
			*pout << pout->endl();
		}
		if (level & 2)
		{
			*pout << pout->endl();
			*pout << "  ", *pout << "EXAMPLE" << pout->endl() << pout->endl();
			*pout << "  ", pout->width(24), *pout << pout->left() << "list services 15", *pout << ' ' << "Displays 15 items per page." << pout->endl();
			*pout << "  ", pout->width(24), *pout << pout->left() << "list services 20 15", *pout << ' ' << "Displays 15 items per page starting from 20." << pout->endl();
			*pout << "  ", pout->width(24), *pout << pout->left() << "list services 0", *pout << ' ' << "Turn off pagination." << pout->endl();
			*pout << pout->endl();
		}
	}
	else if (hint == COMMAND::set)
	{
		*pout << "  ", pout->width(7), *pout << pout->left() << "set", *pout << ' ';
		pout->width(32), *pout << pout->left() << "parentallock service  [chid]", *pout << ' ' << "Set parental lock." << pout->endl();
		pout->width(10), *pout << ' ', *pout << "parentallock userbouquet  [bname]" << pout->endl();
		*pout << pout->endl();
	}
	else if (hint == COMMAND::unset)
	{
		*pout << "  ", pout->width(7), *pout << pout->left() << "unset", *pout << ' ';
		pout->width(32), *pout << pout->left() << "parentallock service  [chid]", *pout << ' ' << "Unset parental lock." << pout->endl();
		pout->width(10), *pout << ' ', *pout << "parentallock userbouquet  [bname]" << pout->endl();
		*pout << pout->endl();
	}
	else if (hint == COMMAND::fread)
	{
		*pout << "  ", pout->width(7), *pout << pout->left() << "read", *pout << ' ';
		pout->width(28), *pout << pout->left() << "[directory]", *pout << ' ' << "Read from directory file." << pout->endl();
		*pout << pout->endl();
	}
	else if (hint == COMMAND::fwrite)
	{
		*pout << "  ", pout->width(7), *pout << pout->left() << "write", *pout << ' ';
		pout->width(28), *pout << pout->left() << "[directory]", *pout << ' ' << "Write to directory file." << pout->endl();
		*pout << pout->endl();
	}
	else if (hint == COMMAND::print)
	{
		*pout << "  ", pout->width(36), *pout << pout->left() << "print", *pout << ' ' << "Print debug informations." << pout->endl();
		*pout << pout->endl();

		if (level & 1)
		{
			*pout << "  ", *pout << "USAGE" << pout->endl() << pout->endl();
			*pout << "  ", pout->width(7), *pout << pout->left() << "print", *pout << ' ';
			pout->width(28), *pout << pout->left() << "debug", *pout << ' ' << "Print debug info." << pout->endl();
			// pout->width(10), *pout << ' ', pout->width(28), *pout << pout->left() << "index", *pout << ' ' << "Print index." << pout->endl();
			*pout << pout->endl();
		}
	}
	else if (hint == COMMAND::fimport)
	{
		*pout << "  ", pout->width(7), *pout << pout->left() << "import", *pout << ' ';
		pout->width(36), *pout << pout->left() << "enigma  [directory]", *pout << ' ' << "Import Enigma directory." << pout->endl();
		pout->width(10), *pout << ' ', pout->width(36), *pout << pout->left() << "neutrino  [directory]", *pout << ' ' << "Import Neutrino directory." << pout->endl();
		pout->width(10), *pout << ' ', pout->width(36), *pout << pout->left() << "lamedb  [files]", *pout << ' ' << "Import Lamedb services files." << pout->endl();
		pout->width(10), *pout << ' ', pout->width(36), *pout << pout->left() << "bouquets  [files]", *pout << ' ' << "Import Enigma bouquet files." << pout->endl();
		pout->width(10), *pout << ' ', pout->width(36), *pout << pout->left() << "userbouquets  [files]", *pout << ' ' << "Import Enigma userbouquet files." << pout->endl();
		pout->width(10), *pout << ' ', pout->width(36), *pout << pout->left() << "zapit services  [files]", *pout << ' ' << "Import Neutrino services files." << pout->endl();
		pout->width(10), *pout << ' ', pout->width(36), *pout << pout->left() << "zapit bouquets  [files]", *pout << ' ' << "Import Neutrino bouquets files." << pout->endl();
		pout->width(10), *pout << ' ', pout->width(36), *pout << pout->left() << "tunersets  [files]", *pout << ' ' << "Import tuner settings xml files." << pout->endl();
		pout->width(10), *pout << ' ', pout->width(36), *pout << pout->left() << "parentallock blacklist  [file]", *pout << ' ' << "Import Enigma blacklist parental lock file." << pout->endl();
		pout->width(10), *pout << ' ', pout->width(36), *pout << pout->left() << "parentallock whitelist  [file]", *pout << ' ' << "Import Enigma whitelist parental lock file." << pout->endl();
		pout->width(10), *pout << ' ', pout->width(36), *pout << pout->left() << "parentallock locked  [file]", *pout << ' ' << "Import Enigma .locked parental lock file." << pout->endl();
		*pout << pout->endl();

		if (level & 1)
		{
			*pout << "  ", *pout << "USAGE" << pout->endl() << pout->endl();
			*pout << "  ", pout->width(7), *pout << pout->left() << "import", *pout << ' ';
			pout->width(28), *pout << pout->left() << "[ENTRY] [files] [version]", *pout << ' ' << "Specific version (eg. Lamedb, Neutrino api)." << pout->endl();
			*pout << pout->endl();
		}
	}
	else if (hint == COMMAND::fexport)
	{
		*pout << "  ", pout->width(7), *pout << pout->left() << "export", *pout << ' ';
		pout->width(36), *pout << pout->left() << "enigma  [directory]", *pout << ' ' << "Export Enigma files to directory." << pout->endl();
		pout->width(10), *pout << ' ', pout->width(36), *pout << pout->left() << "neutrino  [directory]", *pout << ' ' << "Export Neutrino files to directory." << pout->endl();
		pout->width(10), *pout << ' ', pout->width(36), *pout << pout->left() << "lamedb  [file]", *pout << ' ' << "Export Lamedb services files." << pout->endl();
		pout->width(10), *pout << ' ', pout->width(36), *pout << pout->left() << "bouquet  [bname] [file]", *pout << ' ' << "Export Enigma bouquet file." << pout->endl();
		pout->width(10), *pout << ' ', pout->width(36), *pout << pout->left() << "bouquets  [directory]", *pout << ' ' << "Export Enigma bouquet files to directory." << pout->endl();
		pout->width(10), *pout << ' ', pout->width(36), *pout << pout->left() << "userbouquet  [bname] [file]", *pout << ' ' << "Export Enigma userbouquet files." << pout->endl();
		pout->width(10), *pout << ' ', pout->width(36), *pout << pout->left() << "userbouquets  [directory]", *pout << ' ' << "Export Enigma userbouquet files to directory." << pout->endl();
		pout->width(10), *pout << ' ', pout->width(36), *pout << pout->left() << "zapit services  [files]", *pout << ' ' << "Export Neutrino services files." << pout->endl();
		pout->width(10), *pout << ' ', pout->width(36), *pout << pout->left() << "zapit bouquets  [files]", *pout << ' ' << "Export Neutrino bouquets files." << pout->endl();
		pout->width(10), *pout << ' ', pout->width(36), *pout << pout->left() << "tunersets  [file]", *pout << ' ' << "Export tuner settings xml file." << pout->endl();
		pout->width(10), *pout << ' ', pout->width(36), *pout << pout->left() << "tunersets  [directory]", *pout << ' ' << "Export tuner settings xml files to directory." << pout->endl();
		pout->width(10), *pout << ' ', pout->width(36), *pout << pout->left() << "parentallock blacklist  [file]", *pout << ' ' << "Export Enigma blacklist parental lock file." << pout->endl();
		pout->width(10), *pout << ' ', pout->width(36), *pout << pout->left() << "parentallock whitelist  [file]", *pout << ' ' << "Export Enigma whitelist parental lock file." << pout->endl();
		pout->width(10), *pout << ' ', pout->width(36), *pout << pout->left() << "parentallock locked  [file]", *pout << ' ' << "Export Enigma .locked parental lock file." << pout->endl();
		pout->width(10), *pout << ' ', pout->width(36), *pout << pout->left() << "parentallock  [directory]", *pout << ' ' << "Export Enigma parental lock files to directory." << pout->endl();
		*pout << pout->endl();

		if (level & 1)
		{
			*pout << "  ", *pout << "USAGE" << pout->endl() << pout->endl();
			*pout << "  ", pout->width(7), *pout << pout->left() << "export", *pout << ' ';
			pout->width(28), *pout << pout->left() << "[ENTRY] [...] [version]", *pout << ' ' << "Specific version (eg. Lamedb, Neutrino api)." << pout->endl();
			*pout << pout->endl();
		}
	}
	else if (hint == COMMAND::merge)
	{
		*pout << "  ", pout->width(7), *pout << pout->left() << "merge", *pout << ' ';
		pout->width(36), *pout << pout->left() << "enigma  [directory]", *pout << ' ' << "Merge with Enigma directory." << pout->endl();
		pout->width(10), *pout << ' ', pout->width(36), *pout << pout->left() << "neutrino  [directory]", *pout << ' ' << "Merge with Neutrino directory." << pout->endl();
		pout->width(10), *pout << ' ', pout->width(36), *pout << pout->left() << "lamedb  [file]", *pout << ' ' << "Merge with Lamedb services file." << pout->endl();
		pout->width(10), *pout << ' ', pout->width(36), *pout << pout->left() << "bouquet  [file]", *pout << ' ' << "Merge with Enigma bouquet file." << pout->endl();
		// pout->width(10), *pout << ' ', pout->width(36), *pout << pout->left() << "bouquets  [bname] [bname]", *pout << ' ' << "Merge two Enigma bouquet." << pout->endl();
		pout->width(10), *pout << ' ', pout->width(36), *pout << pout->left() << "bouquets  [directory]", *pout << ' ' << "Merge Enigma bouquet files from directory." << pout->endl();
		pout->width(10), *pout << ' ', pout->width(36), *pout << pout->left() << "userbouquet  [file]", *pout << ' ' << "Merge with Enigma userbouquet file." << pout->endl();
		// pout->width(10), *pout << ' ', pout->width(36), *pout << pout->left() << "userbouquets  [bname] [bname]", *pout << ' ' << "Merge two Enigma userbouquet." << pout->endl();
		pout->width(10), *pout << ' ', pout->width(36), *pout << pout->left() << "userbouquets  [directory]", *pout << ' ' << "Merge Enigma userbouquet files from directory." << pout->endl();
		pout->width(10), *pout << ' ', pout->width(36), *pout << pout->left() << "zapit services  [file]", *pout << ' ' << "Merge with Neutrino services file." << pout->endl();
		pout->width(10), *pout << ' ', pout->width(36), *pout << pout->left() << "zapit bouquets  [file]", *pout << ' ' << "Merge with Neutrino bouquets file." << pout->endl();
		pout->width(10), *pout << ' ', pout->width(36), *pout << pout->left() << "tunersets  [file]", *pout << ' ' << "Merge with tuner settings xml file." << pout->endl();
		pout->width(10), *pout << ' ', pout->width(36), *pout << pout->left() << "tunersets  [directory]", *pout << ' ' << "Merge tuner settings xml files from directory." << pout->endl();
		pout->width(10), *pout << ' ', pout->width(36), *pout << pout->left() << "parentallock blacklist  [file]", *pout << ' ' << "Merge with Enigma blacklist parental lock file." << pout->endl();
		pout->width(10), *pout << ' ', pout->width(36), *pout << pout->left() << "parentallock whitelist  [file]", *pout << ' ' << "Merge with Enigma whitelist parental lock file." << pout->endl();
		pout->width(10), *pout << ' ', pout->width(36), *pout << pout->left() << "parentallock locked  [file]", *pout << ' ' << "Merge with Enigma .locked parental lock file." << pout->endl();
		*pout << pout->endl();

		if (level & 1)
		{
			*pout << "  ", *pout << "USAGE" << pout->endl() << pout->endl();
			*pout << "  ", pout->width(7), *pout << pout->left() << "merge", *pout << ' ';
			pout->width(28), *pout << pout->left() << "[ENTRY] [file] [version]", *pout << ' ' << "Specific version (eg. Lamedb, Neutrino api)." << pout->endl();
			*pout << pout->endl();
		}
	}
	else if (hint == COMMAND::parse)
	{
		*pout << "  ", pout->width(7), *pout << pout->left() << "parse", *pout << ' ';
		pout->width(36), *pout << pout->left() << "enigma  [directory]", *pout << ' ' << "Parse Enigma directory." << pout->endl();
		pout->width(10), *pout << ' ', pout->width(36), *pout << pout->left() << "neutrino  [directory]", *pout << ' ' << "Parse Neutrino directory." << pout->endl();
		pout->width(10), *pout << ' ', pout->width(36), *pout << pout->left() << "lamedb  [file]", *pout << ' ' << "Parse Lamedb services file." << pout->endl();
		pout->width(10), *pout << ' ', pout->width(36), *pout << pout->left() << "bouquet  [file]", *pout << ' ' << "Parse Enigma bouquet file." << pout->endl();
		pout->width(10), *pout << ' ', pout->width(36), *pout << pout->left() << "userbouquet  [file]", *pout << ' ' << "Parse Enigma userbouquet file." << pout->endl();
		pout->width(10), *pout << ' ', pout->width(36), *pout << pout->left() << "zapit services  [file]", *pout << ' ' << "Parse Neutrino services file." << pout->endl();
		pout->width(10), *pout << ' ', pout->width(36), *pout << pout->left() << "zapit bouquets  [file]", *pout << ' ' << "Parse Neutrino bouquets file." << pout->endl();
		pout->width(10), *pout << ' ', pout->width(36), *pout << pout->left() << "tunersets  [file]", *pout << ' ' << "Parse tuner settings xml file." << pout->endl();
		pout->width(10), *pout << ' ', pout->width(36), *pout << pout->left() << "parentallock blacklist  [file]", *pout << ' ' << "Parse Enigma blacklist parental lock file." << pout->endl();
		pout->width(10), *pout << ' ', pout->width(36), *pout << pout->left() << "parentallock whitelist  [file]", *pout << ' ' << "Parse Enigma whitelist parental lock file." << pout->endl();
		pout->width(10), *pout << ' ', pout->width(36), *pout << pout->left() << "parentallock locked  [file]", *pout << ' ' << "Parse Enigma .locked parental lock file." << pout->endl();
		*pout << pout->endl();

		if (level & 1)
		{
			*pout << "  ", *pout << "USAGE" << pout->endl() << pout->endl();
			*pout << "  ", pout->width(7), *pout << pout->left() << "parse", *pout << ' ';
			pout->width(28), *pout << pout->left() << "[ENTRY] [file] [version]", *pout << ' ' << "Specific version (eg. Lamedb, Neutrino api)." << pout->endl();
			*pout << pout->endl();
		}
	}
	else if (hint == COMMAND::make)
	{
		*pout << "  ", pout->width(7), *pout << pout->left() << "make", *pout << ' ';
		pout->width(36), *pout << pout->left() << "enigma  [directory]", *pout << ' ' << "Make Enigma files to directory." << pout->endl();
		pout->width(10), *pout << ' ', pout->width(36), *pout << pout->left() << "neutrino  [directory]", *pout << ' ' << "Make Neutrino files to directory." << pout->endl();
		pout->width(10), *pout << ' ', pout->width(36), *pout << pout->left() << "lamedb  [file]", *pout << ' ' << "Make Lamedb services file." << pout->endl();
		pout->width(10), *pout << ' ', pout->width(36), *pout << pout->left() << "bouquet  [bname] [file]", *pout << ' ' << "Make Enigma bouquet file." << pout->endl();
		pout->width(10), *pout << ' ', pout->width(36), *pout << pout->left() << "bouquets  [directory]", *pout << ' ' << "Make Enigma bouquet files to directory." << pout->endl();
		pout->width(10), *pout << ' ', pout->width(36), *pout << pout->left() << "userbouquet  [bname] [file]", *pout << ' ' << "Make Enigma userbouquet file." << pout->endl();
		pout->width(10), *pout << ' ', pout->width(36), *pout << pout->left() << "userbouquets  [directory]", *pout << ' ' << "Make Enigma userbouquet files to directory." << pout->endl();
		pout->width(10), *pout << ' ', pout->width(36), *pout << pout->left() << "zapit services  [file]", *pout << ' ' << "Make Neutrino services file." << pout->endl();
		pout->width(10), *pout << ' ', pout->width(36), *pout << pout->left() << "zapit bouquets  [file]", *pout << ' ' << "Make Neutrino bouquets file." << pout->endl();
		pout->width(10), *pout << ' ', pout->width(36), *pout << pout->left() << "tunersets  [file]", *pout << ' ' << "Make tuner settings xml file." << pout->endl();
		pout->width(10), *pout << ' ', pout->width(36), *pout << pout->left() << "tunersets  [directory]", *pout << ' ' << "Make tuner settings xml files." << pout->endl();
		pout->width(10), *pout << ' ', pout->width(36), *pout << pout->left() << "parentallock blacklist  [file]", *pout << ' ' << "Make Enigma blacklist parental lock file." << pout->endl();
		pout->width(10), *pout << ' ', pout->width(36), *pout << pout->left() << "parentallock whitelist  [file]", *pout << ' ' << "Make Enigma whitelist parental lock file." << pout->endl();
		pout->width(10), *pout << ' ', pout->width(36), *pout << pout->left() << "parentallock locked  [file]", *pout << ' ' << "Make Enigma .locked parental lock file." << pout->endl();
		pout->width(10), *pout << ' ', pout->width(36), *pout << pout->left() << "parentallock  [directory]", *pout << ' ' << "Make Enigma parental lock files to directory." << pout->endl();
		*pout << pout->endl();

		if (level & 1)
		{
			*pout << "  ", *pout << "USAGE" << pout->endl() << pout->endl();
			*pout << "  ", pout->width(7), *pout << pout->left() << "make", *pout << ' ';
			pout->width(28), *pout << pout->left() << "[ENTRY] [...] [version]", *pout << ' ' << "Specific version (eg. Lamedb, Neutrino api)." << pout->endl();
			*pout << pout->endl();
		}
	}
	else if (hint == COMMAND::convert)
	{
		*pout << "  ", pout->width(7), *pout << pout->left() << "convert", *pout << ' ';
		pout->width(36), *pout << pout->left() << "from csv services  [file]", *pout << ' ' << "Convert from CSV services file." << pout->endl();
		pout->width(10), *pout << ' ', pout->width(36), *pout << pout->left() << "from csv bouquets  [file]", *pout << ' ' << "Convert from CSV bouquets file." << pout->endl();
		pout->width(10), *pout << ' ', pout->width(36), *pout << pout->left() << "from csv userbouquets  [file]", *pout << ' ' << "Convert from CSV userbouquets file." << pout->endl();
		pout->width(10), *pout << ' ', pout->width(36), *pout << pout->left() << "from csv tunersets  [file]", *pout << ' ' << "Convert from CSV tuner settings file." << pout->endl();
		*pout << pout->endl();
		*pout << "  ", pout->width(7), *pout << pout->left() << "convert", *pout << ' ';
		pout->width(36), *pout << pout->left() << "to csv  [file]", *pout << ' ' << "Convert all the entries to CSV file." << pout->endl();
		pout->width(10), *pout << ' ', pout->width(36), *pout << pout->left() << "to csv services  [file]", *pout << ' ' << "Convert to CSV services file." << pout->endl();
		pout->width(10), *pout << ' ', pout->width(36), *pout << pout->left() << "to csv services  [stype] [file]", *pout << ' ' << "Convert services of type to CSV services file." << pout->endl();
		pout->width(10), *pout << ' ', pout->width(36), *pout << pout->left() << "to csv bouquets  [file]", *pout << ' ' << "Convert to CSV bouquets file." << pout->endl();
		pout->width(10), *pout << ' ', pout->width(36), *pout << pout->left() << "to csv bouquet  [bname] [file]", *pout << ' ' << "Convert a bouquet to CSV bouquet file." << pout->endl();
		pout->width(10), *pout << ' ', pout->width(36), *pout << pout->left() << "to csv userbouquets  [file]", *pout << ' ' << "Convert to CSV userbouquets file." << pout->endl();
		pout->width(10), *pout << ' ', pout->width(36), *pout << pout->left() << "to csv userbouquet  [bname] [file]", *pout << ' ' << "Convert an userbouquet to CSV userbouquet file." << pout->endl();
		pout->width(10), *pout << ' ', pout->width(36), *pout << pout->left() << "to csv tunersets  [file]", *pout << ' ' << "Convert to CSV tuner settings file." << pout->endl();
		pout->width(10), *pout << ' ', pout->width(36), *pout << pout->left() << "to csv tunersets  [ytype] [file]", *pout << ' ' << "Convert a tuner settings to CSV tuner settings file." << pout->endl();
		*pout << pout->endl();
		*pout << "  ", pout->width(7), *pout << pout->left() << "convert", *pout << ' ';
		pout->width(36), *pout << pout->left() << "from m3u  [file]", *pout << ' ' << "Convert from M3U file." << pout->endl();
		*pout << pout->endl();
		*pout << "  ", pout->width(7), *pout << pout->left() << "convert", *pout << ' ';
		pout->width(36), *pout << pout->left() << "to m3u  [file]", *pout << ' ' << "Convert entries to M3U file." << pout->endl();
		pout->width(10), *pout << ' ', pout->width(36), *pout << pout->left() << "to m3u userbouquets  [file]", *pout << ' ' << "Convert userbouquets to M3U file." << pout->endl();
		pout->width(10), *pout << ' ', pout->width(36), *pout << pout->left() << "to m3u userbouquet  [bname] [file]", *pout << ' ' << "Convert an userbouquet to M3U file." << pout->endl();
		*pout << pout->endl();
		*pout << "  ", pout->width(7), *pout << pout->left() << "convert", *pout << ' ';
		pout->width(36), *pout << pout->left() << "to html  [file]", *pout << ' ' << "Convert all the entries to HTML files." << pout->endl();
		pout->width(10), *pout << ' ', pout->width(36), *pout << pout->left() << "to html index  [file]", *pout << ' ' << "Convert to HTML index of contents files." << pout->endl();
		pout->width(10), *pout << ' ', pout->width(36), *pout << pout->left() << "to html services  [file]", *pout << ' ' << "Convert to HTML services files." << pout->endl();
		pout->width(10), *pout << ' ', pout->width(36), *pout << pout->left() << "to html services  [stype] [file]", *pout << ' ' << "Convert services of type to HTML services files." << pout->endl();
		pout->width(10), *pout << ' ', pout->width(36), *pout << pout->left() << "to html bouquets  [file]", *pout << ' ' << "Convert to HTML bouquets files." << pout->endl();
		pout->width(10), *pout << ' ', pout->width(36), *pout << pout->left() << "to html bouquet  [bname] [file]", *pout << ' ' << "Convert a bouquet to HTML bouquets files." << pout->endl();
		pout->width(10), *pout << ' ', pout->width(36), *pout << pout->left() << "to html userbouquets  [file]", *pout << ' ' << "Convert to HTML userbouquets files." << pout->endl();
		pout->width(10), *pout << ' ', pout->width(36), *pout << pout->left() << "to html userbouquet  [bname] [file]", *pout << ' ' << "Convert an userbouquet to HTML userbouquets files." << pout->endl();
		pout->width(10), *pout << ' ', pout->width(36), *pout << pout->left() << "to html tunersets  [file]", *pout << ' ' << "Convert to HTML tuner settings files." << pout->endl();
		pout->width(10), *pout << ' ', pout->width(36), *pout << pout->left() << "to html tunersets  [ytype] [file]", *pout << ' ' << "Convert a tuner settings to HTML tuner settings files." << pout->endl();
		*pout << pout->endl();
	}
	else if (hint == COMMAND::tool)
	{
		*pout << "  ", pout->width(36), *pout << pout->left() << "tool", *pout << ' ' << "Utils for channel lists." << pout->endl();
		*pout << pout->endl();
		*pout << "  ", *pout << "CLEAN" << pout->endl() << pout->endl();
		*pout << "  ", pout->width(7), *pout << pout->left() << "tool", *pout << ' ';
		pout->width(36), *pout << pout->left() << "remove_orphaned_services", *pout << ' ' << "Remove orphaned services." << pout->endl();
		pout->width(10), *pout << ' ', pout->width(36), *pout << pout->left() << "remove_orphaned_references", *pout << ' ' << "Remove orphaned references." << pout->endl();
		pout->width(10), *pout << ' ', pout->width(36), *pout << pout->left() << "fix_bouquets", *pout << ' ' << "Fix bouquets." << pout->endl();
		pout->width(10), *pout << ' ', pout->width(36), *pout << pout->left() << "fix_bouquets_uniq", *pout << ' ' << "Fix bouquets unique userbouquets." << pout->endl();
		pout->width(10), *pout << ' ', pout->width(36), *pout << pout->left() << "fix_remove_references", *pout << ' ' << "Fix (remove) reference with errors." << pout->endl();
		*pout << pout->endl();
		*pout << "  ", *pout << "PARAMS" << pout->endl() << pout->endl();
		*pout << "  ", pout->width(7), *pout << pout->left() << "tool", *pout << ' ';
		pout->width(36), *pout << pout->left() << "clear_services_cached", *pout << ' ' << "Remove service cached." << pout->endl();
		pout->width(10), *pout << ' ', pout->width(36), *pout << pout->left() << "clear_services_caid", *pout << ' ' << "Remove service CAID." << pout->endl();
		pout->width(10), *pout << ' ', pout->width(36), *pout << pout->left() << "clear_services_flags", *pout << ' ' << "Remove service flags." << pout->endl();
		pout->width(10), *pout << ' ', pout->width(36), *pout << pout->left() << "clear_services_data", *pout << ' ' << "Remove all service data." << pout->endl();
		pout->width(10), *pout << ' ', pout->width(36), *pout << pout->left() << "fix_dvbns", *pout << ' ' << "Recalculate DVBNS for services." << pout->endl();
		*pout << pout->endl();
		*pout << "  ", *pout << "REMOVE" << pout->endl() << pout->endl();
		*pout << "  ", pout->width(7), *pout << pout->left() << "tool", *pout << ' ';
		pout->width(36), *pout << pout->left() << "clear_favourites", *pout << ' ' << "Remove unreferenced entries (favourites)." << pout->endl();
		pout->width(10), *pout << ' ', pout->width(36), *pout << pout->left() << "clear_bouquets_unused_services", *pout << ' ' << "Remove from bouquets (unused services)." << pout->endl();
		pout->width(10), *pout << ' ', pout->width(36), *pout << pout->left() << "remove_parentallock", *pout << ' ' << "Remove parental lock." << pout->endl();
		pout->width(10), *pout << ' ', pout->width(36), *pout << pout->left() << "remove_bouquets", *pout << ' ' << "Remove all bouquets." << pout->endl();
		pout->width(10), *pout << ' ', pout->width(36), *pout << pout->left() << "remove_userbouquets", *pout << ' ' << "Remove all userbouquets." << pout->endl();
		*pout << pout->endl();
		*pout << "  ", *pout << "DUPLICATES" << pout->endl() << pout->endl();
		*pout << "  ", pout->width(7), *pout << pout->left() << "tool", *pout << ' ';
		pout->width(36), *pout << pout->left() << "remove_duplicates", *pout << ' ' << "Remove all duplicates." << pout->endl();
		pout->width(10), *pout << ' ', pout->width(36), *pout << pout->left() << "remove_duplicates_transponders", *pout << ' ' << "Remove duplicate transponders." << pout->endl();
		pout->width(10), *pout << ' ', pout->width(36), *pout << pout->left() << "remove_duplicates_services", *pout << ' ' << "Remove duplicate services." << pout->endl();
		pout->width(10), *pout << ' ', pout->width(36), *pout << pout->left() << "remove_duplicates_references", *pout << ' ' << "Remove duplicate references." << pout->endl();
		pout->width(10), *pout << ' ', pout->width(36), *pout << pout->left() << "remove_duplicates_markers", *pout << ' ' << "Remove duplicate markers (names)." << pout->endl();
		*pout << pout->endl();
		*pout << "  ", *pout << "TRANSFORM" << pout->endl() << pout->endl();
		*pout << "  ", pout->width(7), *pout << pout->left() << "tool", *pout << ' ';
		pout->width(36), *pout << pout->left() << "transform_tunersets_to_transponders", *pout << ' ' << "Transform transponders to XML settings." << pout->endl();
		pout->width(10), *pout << ' ', pout->width(36), *pout << pout->left() << "transform_transponders_to_tunersets", *pout << ' ' << "Transform XML settings to transponders." << pout->endl();
		*pout << pout->endl();
		*pout << "  ", *pout << "SORT" << pout->endl() << pout->endl();
		*pout << "  ", pout->width(7), *pout << pout->left() << "tool", *pout << ' ';
		pout->width(42), *pout << pout->left() << "sort_transponders  [prop] [order]", *pout << ' ' << "Sort transponders." << pout->endl();
		pout->width(10), *pout << ' ', pout->width(42), *pout << pout->left() << "sort_services  [prop] [order]", *pout << ' ' << "Sort services." << pout->endl();
		pout->width(10), *pout << ' ', pout->width(42), *pout << pout->left() << "sort_userbouquets  [prop] [order]", *pout << ' ' << "Sort userbouquets." << pout->endl();
		pout->width(10), *pout << ' ', pout->width(42), *pout << pout->left() << "sort_references  [bname] [prop] [order]", *pout << ' ' << "Sort references." << pout->endl();
		*pout << pout->endl();

		if (level & 1)
		{
			*pout << "  ", *pout << "USAGE" << pout->endl() << pout->endl();
			*pout << "  ", pout->width(7), *pout << pout->left() << "sort_transponders", *pout << ' ';
			*pout << "[tsid,onid,dvbns,ytype,pos,freq,sr,pol,sys,index] [asc,desc]" << pout->endl();
			*pout << "  ", pout->width(7), *pout << pout->left() << "sort_services", *pout << ' ';
			*pout << "[chname,sdata_p,ssid,tsid,onid,dvbns,stype,snum,srcid,parental,txr,index] [asc,desc]" << pout->endl();
			*pout << "  ", pout->width(7), *pout << pout->left() << "sort_userbouquets", *pout << ' ';
			*pout << "[ubname,name,utype,parental,index] [asc,desc]" << pout->endl();
			*pout << "  ", pout->width(7), *pout << pout->left() << "sort_transponders", *pout << ' ';
			*pout << "[bname] [chname,ssid,tsid,onid,dvbns,url,value,inum,txr,index] [asc,desc]" << pout->endl();
			*pout << pout->endl();
		}
	}
	else if (hint == COMMAND::macro)
	{
		*pout << "  ", pout->width(36), *pout << pout->left() << "macro", *pout << ' ' << "Execute tools macro." << pout->endl();
		*pout << pout->endl();
		*pout << "  ", pout->width(7), *pout << pout->left() << "macro", *pout << ' ';
		pout->width(28), *pout << pout->left() << "autofix", *pout << ' ' << "Autofix macro." << pout->endl();
		pout->width(10), *pout << ' ', pout->width(28), *pout << pout->left() << "[util],[util],[...]", *pout << ' ' << "Apply utils sequentially." << pout->endl();
		*pout << pout->endl();

		if (level & 1)
		{
			*pout << "  ", *pout << "USAGE" << pout->endl() << pout->endl();
			*pout << "  ", *pout << "Type \"help tool\" for full util lists." << pout->endl();
			*pout << pout->endl();
		}
	}
	else if (hint == COMMAND::inspect)
	{
		*pout << "  ", pout->width(36), *pout << pout->left() << "inspect", *pout << ' ' << "Display log." << pout->endl();
		*pout << pout->endl();
	}
	else if (hint == COMMAND::preferences)
	{
		*pout << "  ", pout->width(36), *pout << pout->left() << "preferences", *pout << ' ' << "CLI preferences." << pout->endl();
		*pout << pout->endl();

		if (level & 1)
		{
			*pout << "  ", *pout << "USAGE" << pout->endl() << pout->endl();
			*pout << "  ", pout->width(7), *pout << pout->left() << "preferences", *pout << ' ';
			pout->width(24), *pout << pout->left() << "output  [format]", *pout << ' ' << "Output format (tabular, byline, json)" << pout->endl();
			pout->width(14), *pout << ' ', pout->width(24), *pout << pout->left() << "history  [type]", *pout << ' ' << "History type (file, memory)" << pout->endl();
			*pout << pout->endl();
		}
	}
}


void e2db_console::console_print(int opt)
{
	try
	{
		if (opt)
			*pout << "TODO" << pout->endl();
		else
			dbih->dump();
	}
	catch (const std::invalid_argument& err)
	{
		*perr << "Error: " << msg(MSG::except_invalid_argument, err.what()) << pout->endl();
	}
	catch (const std::out_of_range& err)
	{
		*perr << "Error: " << msg(MSG::except_out_of_range, err.what()) << pout->endl();
	}
	catch (...)
	{
		*perr << "Error: " << msg(MSG::except_uncaught) << pout->endl();
	}
}

void e2db_console::console_inspect()
{
	*pout << plog->str();
}

void e2db_console::console_preferences(string type, string val)
{
	if (type == "output")
	{
		OBJIO format = OBJIO::tabular;

		if (! val.empty())
		{
			if (val == "tabular")
				format = OBJIO::tabular;
			else if (val == "byline")
				format = OBJIO::byline;
			else if (val == "json")
				format = OBJIO::json;
		}

		console_preferences(format);
	}
	else if (type == "history")
	{
		HISTORY type = HISTORY::file;

		if (! val.empty())
		{
			if (val == "file")
				type = HISTORY::file;
			else if (val == "memory")
				type = HISTORY::memory;
		}

		console_preferences(type);
	}
	else if (type.empty())
	{
		string format;
		string type;
		switch (__objio.out)
		{
			case OBJIO::byline: format = "byline"; break;
			case OBJIO::json: format = "json"; break;
			default: format = "tabular";
		}
		switch (history)
		{
			case HISTORY::file: type = "file"; break;
			case HISTORY::memory: type = "memory"; break;
		}
		*pout << "Output format: " << format << pout->endl();
		*pout << "History type: " << type << pout->endl();
	}
	else
	{
		*perr << "Error: " << msg("Wrong parameter type.") << pout->endl();
	}
}

void e2db_console::console_preferences(OBJIO format)
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
		*perr << "Error: " << msg("Wrong parameter format.") << pout->endl();
	}
}

void e2db_console::console_preferences(HISTORY type)
{
	if (type == HISTORY::file)
		history = HISTORY::file;
	else if (type == HISTORY::memory)
		history = HISTORY::memory;
	else
		*perr << "Error: " << msg("Wrong parameter type.") << pout->endl();
}

void e2db_console::console_file_read(string path)
{
	try
	{
		if (dbih->read(path))
			*pout << "Info: " << msg("File read: %s", path) << pout->endl();
	}
	catch (const std::invalid_argument& err)
	{
		*perr << "Error: " << msg(MSG::except_invalid_argument, err.what()) << pout->endl();
	}
	catch (const std::out_of_range& err)
	{
		*perr << "Error: " << msg(MSG::except_out_of_range, err.what()) << pout->endl();
	}
	catch (const std::filesystem::filesystem_error& err)
	{
		*perr << "Error: " << msg(MSG::except_filesystem, err.what()) << pout->endl();
	}
	catch (const std::ifstream::failure& err)
	{
		*perr << "Error: " << msg("File \"%s\" is not readable.", path) << pout->endl();
	}
	catch (const std::runtime_error& err)
	{
		*perr << "Error: " << err.what() << pout->endl();
	}
	catch (...)
	{
		*perr << "Error: " << msg(MSG::except_uncaught) << pout->endl();
	}
}

void e2db_console::console_file_write(string path)
{
	try
	{
		if (dbih->write(path))
			*pout << "Info: " << msg("File written: %s", path) << pout->endl();
	}
	catch (const std::invalid_argument& err)
	{
		*perr << "Error: " << msg(MSG::except_invalid_argument, err.what()) << pout->endl();
	}
	catch (const std::out_of_range& err)
	{
		*perr << "Error: " << msg(MSG::except_out_of_range, err.what()) << pout->endl();
	}
	catch (const std::filesystem::filesystem_error& err)
	{
		*perr << "Error: " << msg(MSG::except_filesystem, err.what()) << pout->endl();
	}
	catch (const std::ofstream::failure& err)
	{
		*perr << "Error: " << msg("File \"%s\" is not writable.", path) << pout->endl();
	}
	catch (const std::runtime_error& err)
	{
		*perr << "Error: " << err.what() << pout->endl();
	}
	catch (...)
	{
		*perr << "Error: " << msg(MSG::except_uncaught) << pout->endl();
	}
}

void e2db_console::console_e2db_import(ENTRY entry_type, vector<string> paths, int ver, bool dir)
{
	if (paths.size() == 0)
	{
		*perr << "Error: " << msg("Wrong parameter paths.") << pout->endl();

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
		*perr << "Error: " << msg(MSG::except_invalid_argument, err.what()) << pout->endl();
	}
	catch (const std::out_of_range& err)
	{
		*perr << "Error: " << msg(MSG::except_out_of_range, err.what()) << pout->endl();
	}
	catch (const std::filesystem::filesystem_error& err)
	{
		*perr << "Error: " << msg(MSG::except_filesystem, err.what()) << pout->endl();
	}
	catch (const std::ifstream::failure& err)
	{
		*perr << "Error: " << msg("File Error") << pout->endl();
	}
	catch (const std::runtime_error& err)
	{
		*perr << "Error: " << err.what() << pout->endl();
	}
	catch (...)
	{
		*perr << "Error: " << msg(MSG::except_uncaught) << pout->endl();
	}
}

void e2db_console::console_e2db_export(ENTRY entry_type, vector<string> paths, int ver, bool dir, string bname)
{
	if (paths.size() == 0)
	{
		*perr << "Error: " << msg("Wrong parameter paths.") << pout->endl();

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
		*perr << "Error: " << msg(MSG::except_invalid_argument, err.what()) << pout->endl();
	}
	catch (const std::out_of_range& err)
	{
		*perr << "Error: " << msg(MSG::except_out_of_range, err.what()) << pout->endl();
	}
	catch (const std::filesystem::filesystem_error& err)
	{
		*perr << "Error: " << msg(MSG::except_filesystem, err.what()) << pout->endl();
	}
	catch (const std::ofstream::failure& err)
	{
		*perr << "Error: " << msg("File Error") << pout->endl();
	}
	catch (const std::runtime_error& err)
	{
		*perr << "Error: " << err.what() << pout->endl();
	}
	catch (...)
	{
		*perr << "Error: " << msg(MSG::except_uncaught) << pout->endl();
	}
}

void e2db_console::console_e2db_merge(ENTRY entry_type, string path, int ver, bool dir)
{
	if (path.empty())
	{
		*perr << "Error: " << msg("Wrong parameter path.") << pout->endl();

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
		*perr << "Error: " << msg(MSG::except_invalid_argument, err.what()) << pout->endl();
	}
	catch (const std::out_of_range& err)
	{
		*perr << "Error: " << msg(MSG::except_out_of_range, err.what()) << pout->endl();
	}
	catch (const std::filesystem::filesystem_error& err)
	{
		*perr << "Error: " << msg(MSG::except_filesystem, err.what()) << pout->endl();
	}
	catch (const std::ifstream::failure& err)
	{
		*perr << "Error: " << msg("File \"%s\" is not readable.", path) << pout->endl();
	}
	catch (const std::runtime_error& err)
	{
		*perr << "Error: " << err.what() << pout->endl();
	}
	catch (...)
	{
		*perr << "Error: " << msg(MSG::except_uncaught) << pout->endl();
	}
}

//TODO
void e2db_console::console_e2db_merge(ENTRY entry_type, int ver, string bname0, string bname1)
{
	*pout << "TODO" << pout->endl();
}

void e2db_console::console_e2db_parse(ENTRY entry_type, string path, int ver, bool dir)
{
	if (path.empty())
	{
		*perr << "Error: " << msg("Wrong parameter path.") << pout->endl();

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
			iservices.exceptions(ifstream::badbit);
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
			ibouquet.exceptions(ifstream::badbit);
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
			iuserbouquet.exceptions(ifstream::badbit);
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
			iservicesxml.exceptions(ifstream::badbit);
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
			ibouquetsxml.exceptions(ifstream::badbit);
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
			itunxml.exceptions(ifstream::badbit);
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
			iparental.exceptions(ifstream::badbit);
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
			iparental.exceptions(ifstream::badbit);
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
			iparental.exceptions(ifstream::badbit);
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
		*perr << "Error: " << msg(MSG::except_invalid_argument, err.what()) << pout->endl();
	}
	catch (const std::out_of_range& err)
	{
		*perr << "Error: " << msg(MSG::except_out_of_range, err.what()) << pout->endl();
	}
	catch (const std::filesystem::filesystem_error& err)
	{
		*perr << "Error: " << msg(MSG::except_filesystem, err.what()) << pout->endl();
	}
	catch (const std::ifstream::failure& err)
	{
		*perr << "Error: " << msg("File \"%s\" is not readable.", path) << pout->endl();
	}
	catch (const std::runtime_error& err)
	{
		*perr << "Error: " << err.what() << pout->endl();
	}
	catch (...)
	{
		*perr << "Error: " << msg(MSG::except_uncaught) << pout->endl();
	}
}

//TODO TEST ofstream path
void e2db_console::console_e2db_make(ENTRY entry_type, string path, int ver, bool dir, string bname)
{
	if (path.empty())
	{
		*perr << "Error: " << msg("Wrong parameter path.") << pout->endl();

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
			out.exceptions(ofstream::failbit | ofstream::badbit);
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
				out.exceptions(ofstream::failbit | ofstream::badbit);
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
				out.exceptions(ofstream::failbit | ofstream::badbit);
				out << file.data;
				out.close();
			}
		}
		else if (entry_type == ENTRY::zapit_services)
		{
			e2db::e2db_file file;

			dbih->make_bouquets_xml(filename, file, ver);

			ofstream out (path);
			out.exceptions(ofstream::failbit | ofstream::badbit);
			out << file.data;
			out.close();
		}
		else if (entry_type == ENTRY::zapit_bouquets)
		{
			e2db::e2db_file file;

			dbih->make_bouquets_xml(filename, file, ver);

			ofstream out (path);
			out.exceptions(ofstream::failbit | ofstream::badbit);
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
				out.exceptions(ofstream::failbit | ofstream::badbit);
				out << file.data;
				out.close();
			}
		}
		else if (entry_type == ENTRY::parentallock_locked)
		{
			e2db::e2db_file file;

			dbih->make_parentallock_list(filename, e2db::PARENTALLOCK::locked, file);

			ofstream out (path);
			out.exceptions(ofstream::failbit | ofstream::badbit);
			out << file.data;
			out.close();
		}
		else if (entry_type == ENTRY::parentallock_blacklist)
		{
			e2db::e2db_file file;

			dbih->make_parentallock_list(filename, e2db::PARENTALLOCK::blacklist, file);

			ofstream out (path);
			out.exceptions(ofstream::failbit | ofstream::badbit);
			out << file.data;
			out.close();
		}
		else if (entry_type == ENTRY::parentallock_whitelist)
		{
			e2db::e2db_file file;

			dbih->make_parentallock_list(filename, e2db::PARENTALLOCK::whitelist, file);

			ofstream out (path);
			out.exceptions(ofstream::failbit | ofstream::badbit);
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
				out.exceptions(ofstream::failbit | ofstream::badbit);
				out << file.data;
				out.close();
			}
			else
			{
				filename = dbih->db.parental ? "whitelist" : "blacklist";

				dbih->make_parentallock_list(filename, dbih->db.parental, file);

				ofstream out (path);
				out.exceptions(ofstream::failbit | ofstream::badbit);
				out << file.data;
				out.close();

				{
					filename = dbih->db.parental ? "blacklist" : "whitelist";

					e2db::e2db_file empty;
					empty.filename = filename;
					empty.mime = "text/plain";
					empty.size = 0;

					ofstream out (path);
					out.exceptions(ofstream::failbit | ofstream::badbit);
					out << empty.data;
					out.close();
				}
			}
		}
	}
	catch (const std::invalid_argument& err)
	{
		*perr << "Error: " << msg(MSG::except_invalid_argument, err.what()) << pout->endl();
	}
	catch (const std::out_of_range& err)
	{
		*perr << "Error: " << msg(MSG::except_out_of_range, err.what()) << pout->endl();
	}
	catch (const std::filesystem::filesystem_error& err)
	{
		*perr << "Error: " << msg(MSG::except_filesystem, err.what()) << pout->endl();
	}
	catch (const std::ofstream::failure& err)
	{
		*perr << "Error: " << msg("File \"%s\" is not writable.", path) << pout->endl();
	}
	catch (const std::runtime_error& err)
	{
		*perr << "Error: " << err.what() << pout->endl();
	}
	catch (...)
	{
		*perr << "Error: " << msg(MSG::except_uncaught) << pout->endl();
	}
}

void e2db_console::console_e2db_convert(ENTRY entry_type, int fopt, int ftype, string path, string bname, int stype, int ytype)
{
	if (path.empty())
	{
		*perr << "Error: " << msg("Wrong parameter path.") << pout->endl();

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
		*perr << "Error: " << msg(MSG::except_invalid_argument, err.what()) << pout->endl();
	}
	catch (const std::out_of_range& err)
	{
		*perr << "Error: " << msg(MSG::except_out_of_range, err.what()) << pout->endl();
	}
	catch (const std::filesystem::filesystem_error& err)
	{
		*perr << "Error: " << msg(MSG::except_filesystem, err.what()) << pout->endl();
	}
	catch (const std::ios_base::failure& err)
	{
		if (fopt == 0)
			*perr << "Error: " << msg("File \"%s\" is not readable.", path) << pout->endl();
		else if (fopt == 1)
			*perr << "Error: " << msg("File \"%s\" is not writable.", path) << pout->endl();
	}
	catch (const std::runtime_error& err)
	{
		*perr << "Error: " << err.what() << pout->endl();
	}
	catch (...)
	{
		*perr << "Error: " << msg(MSG::except_uncaught) << pout->endl();
	}
}

void e2db_console::console_e2db_tool(string fn, string bname, string prop, int order)
{
	e2db::uoopts opts;

	if (fn.find("sort_") != string::npos)
	{
		e2db::SORT_ITEM model = e2db::SORT_ITEM::item_reference;

		if (fn == "sort_transponders")
			model = e2db::SORT_ITEM::item_transponder;
		else if (fn == "sort_services")
			model = e2db::SORT_ITEM::item_service;
		else if (fn == "sort_userbouquets")
			model = e2db::SORT_ITEM::item_userbouquet;
		else if (fn == "sort_references")
			model = e2db::SORT_ITEM::item_reference;
		else
		{
			*perr << "Error: " << msg("Wrong util name \"%s\".", fn);

			return;
		}

		if (model == e2db::SORT_ITEM::item_reference && ! bname.empty())
			opts.iname = bname;

		opts.order = (order ? e2db::SORT_ORDER::sort_desc : e2db::SORT_ORDER::sort_asc);

		if (! prop.empty())
		{
			if (model == e2db::SORT_ITEM::item_userbouquet)
			{
				if (prop == "ubname") opts.prop = prop;
				else if (prop == "name") opts.prop = prop;
				else if (prop == "utype") opts.prop = prop;
				else if (prop == "parental") opts.prop = prop;
				else if (prop == "index") opts.prop = prop;
				else
					*perr << "Error: " << msg("Wrong property name \"%s\".", prop);
			}
			else if (model == e2db::SORT_ITEM::item_reference)
			{
				if (prop == "chname") opts.prop = prop;
				else if (prop == "ssid") opts.prop = prop;
				else if (prop == "tsid") opts.prop = prop;
				else if (prop == "onid") opts.prop = prop;
				else if (prop == "dvbns") opts.prop = prop;
				else if (prop == "url") opts.prop = prop;
				else if (prop == "value") opts.prop = prop;
				else if (prop == "inum") opts.prop = prop;
				else if (prop == "txr") opts.prop = prop;
				else if (prop == "index") opts.prop = prop;
				else
					*perr << "Error: " << msg("Wrong property name \"%s\".", prop);
			}
			else if (model == e2db::SORT_ITEM::item_service)
			{
				if (prop == "chname") opts.prop = prop;
				else if (prop == "sdata_p") opts.prop = prop;
				else if (prop == "ssid") opts.prop = prop;
				else if (prop == "tsid") opts.prop = prop;
				else if (prop == "onid") opts.prop = prop;
				else if (prop == "dvbns") opts.prop = prop;
				else if (prop == "stype") opts.prop = prop;
				else if (prop == "snum") opts.prop = prop;
				else if (prop == "srcid") opts.prop = prop;
				else if (prop == "parental") opts.prop = prop;
				else if (prop == "txr") opts.prop = prop;
				else if (prop == "index") opts.prop = prop;
				else
					*perr << "Error: " << msg("Wrong property name \"%s\".", prop);
			}
			else if (model == e2db::SORT_ITEM::item_transponder)
			{
				if (prop == "tsid") opts.prop = prop;
				else if (prop == "onid") opts.prop = prop;
				else if (prop == "dvbns") opts.prop = prop;
				else if (prop == "ytype") opts.prop = prop;
				else if (prop == "pos") opts.prop = prop;
				else if (prop == "freq") opts.prop = prop;
				else if (prop == "sr") opts.prop = prop;
				else if (prop == "pol") opts.prop = prop;
				else if (prop == "sys") opts.prop = prop;
				else if (prop == "index") opts.prop = prop;
				else
					*perr << "Error: " << msg("Wrong property name \"%s\".", prop);
			}
		}
	}

	try
	{
		if (fn.empty())
			throw std::runtime_error (msg("Wrong util name."));
		else if (fn == "remove_orphaned_services")
			dbih->remove_orphaned_services();
		else if (fn == "remove_orphaned_references")
			dbih->remove_orphaned_references();
		else if (fn == "fix_remove_references")
			dbih->fix_remove_references();
		else if (fn == "fix_bouquets")
			dbih->fix_bouquets(false);
		else if (fn == "fix_bouquets_uniq")
			dbih->fix_bouquets(true);
		else if (fn == "fix_dvbns")
			dbih->fix_dvbns();
		else if (fn == "clear_services_cached")
			dbih->clear_services_cached();
		else if (fn == "clear_services_caid")
			dbih->clear_services_caid();
		else if (fn == "clear_services_flags")
			dbih->clear_services_flags();
		else if (fn == "clear_services_data")
			dbih->clear_services_data();
		else if (fn == "clear_favourites")
			dbih->clear_favourites();
		else if (fn == "clear_bouquets_unused_services")
			dbih->clear_bouquets_unused_services();
		else if (fn == "remove_parentallock")
			dbih->remove_parentallock();
		else if (fn == "remove_bouquets")
			dbih->remove_bouquets();
		else if (fn == "remove_userbouquets")
			dbih->remove_userbouquets();
		else if (fn == "remove_duplicates")
			dbih->remove_duplicates();
		else if (fn == "remove_duplicates_transponders")
			dbih->remove_duplicates_transponders();
		else if (fn == "remove_duplicates_services")
			dbih->remove_duplicates_services();
		else if (fn == "remove_duplicates_references")
			dbih->remove_duplicates_references();
		else if (fn == "remove_duplicates_markers")
			dbih->remove_duplicates_markers();
		else if (fn == "transform_tunersets_to_transponders")
			dbih->transform_tunersets_to_transponders();
		else if (fn == "transform_transponders_to_tunersets")
			dbih->transform_transponders_to_tunersets();
		else if (fn == "sort_transponders")
			dbih->sort_transponders(opts);
		else if (fn == "sort_services")
			dbih->sort_services(opts);
		else if (fn == "sort_userbouquets")
			dbih->sort_userbouquets(opts);
		else if (fn == "sort_references")
			dbih->sort_references(opts);
		else
			throw std::runtime_error (msg("Wrong util name \"%s\".", fn));
		}
	catch (const std::invalid_argument& err)
	{
		*perr << "Error: " << msg(MSG::except_invalid_argument, err.what()) << pout->endl();
	}
	catch (const std::out_of_range& err)
	{
		*perr << "Error: " << msg(MSG::except_out_of_range, err.what()) << pout->endl();
	}
	catch (const std::runtime_error& err)
	{
		*perr << "Error: " << err.what() << pout->endl();
	}
	catch (...)
	{
		*perr << "Error: " << msg(MSG::except_uncaught) << pout->endl();
	}
}

void e2db_console::console_e2db_macro(string id)
{
	if (id == "autofix")
	{
		vector<string> pattern = {
			"remove_duplicates",
			"fix_bouquets",
			"fix_remove_references",
			"fix_dvbns"
		};

		console_e2db_macro(pattern);
	}
	else
	{
		*perr << "Error: " << msg("Wrong parameter identifier.") << pout->endl();
	}
}

void e2db_console::console_e2db_macro(vector<string> pattern)
{
	if (pattern.size() == 0)
	{
		*perr << "Error: " << msg("Wrong parameter pattern.") << pout->endl();
	}
	else
	{
		for (string & fn : pattern)
			console_e2db_tool(fn);
	}
}

void e2db_console::entry_list(ENTRY entry_type, string bname, int offset0, int offset1)
{
	entry_list(entry_type, offset0, offset1, bname);
}

void e2db_console::entry_list(ENTRY entry_type, int offset0, int offset1, string bname)
{
	if (offset0 != -1 && offset1 != -1) // [start] [limit]
		entry_list(entry_type, offset1 != 0, offset1, offset0, bname);
	else if (offset0 != -1) // [limit]
		entry_list(entry_type, offset0 != 0, offset0, 0, bname);
	else
		entry_list(entry_type, true, PAGED_LIMIT, 0, bname);
}

void e2db_console::entry_list(ENTRY entry_type, bool paged, int limit, int pos, string bname)
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
			auto screensize = termctl->screensize();
			offset = screensize.first ? screensize.first / rows : 1;
		}

		entry_list_exec(entry_type, pos, offset, end, bname);

		while (! end)
		{
			if (limit == 0)
			{
				auto screensize = termctl->screensize();
				offset = screensize.first ? screensize.first / rows : 1;
			}

			int key = termctl->paged(pos, offset);

			switch (key)
			{
				case 65: pos -= offset; break; // termctl::EVENT::PagePrev
				case 66: pos += offset; break; // termctl::EVENT::PageNext
				default: return; // any key
			}

			entry_list_exec(entry_type, pos, offset, end, bname);
		}
	}
	else
	{
		entry_list_exec(entry_type, pos, offset, end, bname);
	}
}

void e2db_console::entry_add(ENTRY entry_type)
{
	entry_edit(entry_type, false);
}

void e2db_console::entry_add(ENTRY entry_type, int ref, string bname)
{
	entry_edit(entry_type, false, "", ref, bname);
}

void e2db_console::entry_edit(ENTRY entry_type, string id)
{
	entry_edit(entry_type, true, id);
}

void e2db_console::entry_edit(ENTRY entry_type, int ref, string bname, string id)
{
	entry_edit(entry_type, true, id, ref, bname);
}

void e2db_console::entry_edit(ENTRY entry_type, bool edit, string id, int ref, string bname)
{
	entry_edit_exec(entry_type, edit, id, ref, bname);
}

void e2db_console::entry_remove(ENTRY entry_type, int ref, string bname, string id)
{
	entry_remove(entry_type, id, bname);
}

void e2db_console::entry_remove(ENTRY entry_type, string id, string bname)
{
	entry_remove_exec(entry_type, id, bname);
}

void e2db_console::entry_parentallock(ENTRY entry_type, string id, bool flag)
{
	entry_parentallock_exec(entry_type, id, flag);
}

//TODO improve
void e2db_console::entry_list_exec(ENTRY entry_type, int pos, int offset, int& end, string bname)
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
				for (string & bname : bs.userbouquets)
				{
					i++;

					print_obj_begin(2), print_obj_sep(-1);
					print_obj_pair(TYPE::bname, bname), print_obj_sep(1);
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
				*perr << "Error: " << msg("Userbouquet \"%s\" not exists.", bname) << pout->endl();

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
					print_obj_pair(TYPE::churl, chref.url), print_obj_sep();
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
		*perr << "Error: " << msg(MSG::except_invalid_argument, err.what()) << pout->endl();
	}
	catch (const std::out_of_range& err)
	{
		*perr << "Error: " << msg(MSG::except_out_of_range, err.what()) << pout->endl();
	}
	catch (const std::bad_any_cast& err)
	{
		*perr << "Error: " << msg(MSG::except_bad_any_cast, err.what()) << pout->endl();
	}
	catch (const std::runtime_error& err)
	{
		*perr << "Error: " << err.what() << pout->endl();
	}
	catch (...)
	{
		*perr << "Error: " << msg(MSG::except_uncaught) << pout->endl();
	}
}

//TODO improve
void e2db_console::entry_edit_exec(ENTRY entry_type, bool edit, string id, int ref, string bname)
{
	using std::any_cast;

	if (edit && id.empty())
	{
		*perr << "Error: " << msg("Wrong parameter identifier.") << pout->endl();

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
			tx.dvbns = any_cast<int>(field(TYPE::dvbns, false));
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

					tx.mispls = tx.mispls || (tx.isid != -1 || tx.plscode != -1 || tx.plsmode != -1);
					tx.t2mi = tx.t2mi || (tx.t2mi_plpid != -1 || tx.t2mi_pid != -1);
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

			if (tx.dvbns == 0)
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
					else
						throw 1;
				}
				catch (...)
				{
					throw std::runtime_error (msg("Tuner settings \"%s\" not exists.", id));
				}
			}

			tv.ytype = any_cast<int>(field(TYPE::ytype, true));
			tv.charset = any_cast<string>(field(TYPE::charset));

			if (tv.charset.empty())
				tv.charset = "utf-8";

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
				chref.url = any_cast<string>(field(TYPE::churl));

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
		*perr << "Error: " << msg(MSG::except_invalid_argument, err.what()) << pout->endl();
	}
	catch (const std::out_of_range& err)
	{
		*perr << "Error: " << msg(MSG::except_out_of_range, err.what()) << pout->endl();
	}
	catch (const std::bad_any_cast& err)
	{
		*perr << "Error: " << msg("Wrong input data in \"%s\" field.", this->curr_field) << pout->endl();
	}
	catch (const std::runtime_error& err)
	{
		*perr << "Error: " << err.what() << pout->endl();
	}
	catch (...)
	{
		*perr << "Error: " << msg(MSG::except_uncaught) << pout->endl();
	}
}

void e2db_console::entry_remove_exec(ENTRY entry_type, string id, string bname)
{
	if (id.empty())
	{
		*perr << "Error: " << msg("Wrong parameter identifier.") << pout->endl();

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

				if (! dbih->tuners.count(tvid))
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
		*perr << "Error: " << err.what() << pout->endl();
	}
	catch (...)
	{
		*perr << "Error: " << msg(MSG::except_uncaught) << pout->endl();
	}
}

void e2db_console::entry_parentallock_exec(ENTRY entry_type, string id, bool flag)
{
	if (id.empty())
	{
		*perr << "Error: " << msg("Wrong parameter identifier.") << pout->endl();

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
		*perr << "Error: " << err.what() << pout->endl();
	}
	catch (...)
	{
		*perr << "Error: " << msg(MSG::except_uncaught) << pout->endl();
	}
}

bool e2db_console::label_field(TYPE type, string &label, string &description)
{
	switch (type)
	{
		case TYPE::dbtype: label = "Format"; description = "exact match: <empty>, 0 = Lamedb, 1 = Zapit"; break;
		case TYPE::dbparental: label = "Parental lock"; description = "exact match: <empty>, 0 = blacklist, 1 = whitelist, 2 = Enigma 1 format"; break;
		case TYPE::idx: label = "Index"; description = "Channel list number, in digits"; break;
		case TYPE::chid: label = "CHID"; description = "Channel ID [ssid]:[tsid]:[dvbns] eg. 4d2:3e8:eeee0000"; break;
		case TYPE::txid: label = "TXID"; description = "Transponder ID [tsid]:[dvbns] eg. 3e8:eeee0000"; break;
		case TYPE::refid: label = "REFID"; description = "Reference ID, colon separated values"; break;
		case TYPE::tnid: label = "TNID"; description = "Tunersets Table ID [yname]:[index] eg. s:0001"; break;
		case TYPE::trid: label = "TRID"; description = "Tunersets Transponder ID [yname]:[freq]:[index] eg. s:2710:1000"; break;
		case TYPE::yname: label = "YNAME"; description = "Tuner Name, exact match: s = satellite, t = terrestrial, c = cable, a = atsc"; break;
		case TYPE::ytype: label = "YTYPE"; description = "Tuner Type, exact match: 0 = satellite, 1 = terrestrial, 2 = cable, 3 = atsc"; break;
		case TYPE::ssid: label = "SSID"; description = "Service ID, in digits"; break;
		case TYPE::dvbns: label = "DVBNS"; description = "DVB namespace, in hex or <empty>"; break;
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
		case TYPE::churl: label = "Channel URL"; break;
		case TYPE::etype: label = "Favourite Type"; description = "exact match: 1 = broadcast, 2 = file, 3 = 4097, 8139 = youtube, 8193 = eservice"; break;
		case TYPE::atype: label = "Favourite Flag"; description = "exact match: 64 = marker, 512 = marker hidden, 832 = marker hidden, 320 = marker numbered, 128 = group"; break;
		case TYPE::freq: label = "Frequency"; description = "in Hertz, 6 or 8 digits"; break;
		case TYPE::sr: label = "Symbol Rate"; description = "in Hertz, 6 or 8 digits"; break;
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
		default: return false;
	}

	return true;
}

void e2db_console::label_obj_pair(TYPE type, string &name, VALUE &value_type)
{
	bool hrn = __objio.hrn;
	bool hrv = __objio.hrv;

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
		case TYPE::churl: name = hrn ? "Channel URL" : "churl"; value_type = VALUE::val_string; break;
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
}

bool e2db_console::value_field(TYPE type, string str, bool required, std::any &val)
{
	if (str.empty() && required)
		return false;

	VALUE value_type = VALUE::val_int;
	int d = -1;

	switch (type)
	{
		case TYPE::idx:
		case TYPE::ssid:
		case TYPE::tsid:
		case TYPE::onid:
		case TYPE::snum:
		case TYPE::srcid:
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
				return false;
		break;
		case TYPE::freq:
			d = dbih->value_transponder_frequency(str);
			if (! d && required)
				return false;
		break;
		case TYPE::sr:
			d = dbih->value_transponder_sr(str);
			if (! d && required)
				return false;
		break;
		case TYPE::dvbns:
			d = dbih->value_transponder_dvbns(str);
			if (d == 0 && required)
				return false;
		break;
		case TYPE::dbtype:
			d = std::atoi(str.data());
			if (required && (d < 0 || d > 1))
				return false;
		break;
		case TYPE::dbparental:
			d = std::atoi(str.data());
			if (required && (d < 0 || d > 2))
				return false;
		break;
		case TYPE::ytype:
			d = std::atoi(str.data());
			if (required && (d < 0 || d > 3))
				return false;
		break;
		case TYPE::chdata:
		case TYPE::txdata:
		case TYPE::ffdata:
		case TYPE::parental:
		case TYPE::feed:
		case TYPE::hidden:
		case TYPE::locked:
			value_type = VALUE::val_int;
			if (str == "Y" || str == "y")
				d = 1;
			else if (str == "N" || str == "n")
				d = 0;
			else if (required)
				return false;
		break;
		case TYPE::sdata_p:
			value_type = VALUE::val_obj;
			val = { str };
		break;
		case TYPE::yname:
			if (str.size() == 1)
				d = dbih->value_transponder_type(str[0]);
			else if (required)
				return false;
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
			value_type = VALUE::val_string;
			if (required && (str.size() < 3 || str.size() > 3))
				return false;
			// failsafe string uppercase
			std::transform(str.begin(), str.end(), str.begin(), [](unsigned char c) { return std::toupper(c); });
			val = str;
		break;
		case TYPE::charset:
			value_type = VALUE::val_string;
			// failsafe string uppercase
			std::transform(str.begin(), str.end(), str.begin(), [](unsigned char c) { return std::toupper(c); });
			val = str;
		break;
		default:
			value_type = VALUE::val_string;
			val = str;
	}

	if (value_type == VALUE::val_int)
	{
		val = d;

		if (d == -1 && required)
			return false;
	}
	else if (str.empty() && required)
	{
		return false;
	}

	return true;
}

void e2db_console::value_obj_pair(TYPE type, VALUE value_type, std::any val, int &d, string &str)
{
	using std::any_cast;

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
		case TYPE::churl:
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
}

std::any e2db_console::field(TYPE type, bool required)
{
	string label;
	string description;

	if (! label_field(type, label, description))
		return -1;

	while (true)
	{
		*pout << label;
		if (! description.empty())
			*pout << ' ' << '(' << description << ')';
		if (required)
			*pout << ' ' << '*';
		*pout << ':' << ' ';

		termctl->handler(termiface::HANDLE::Input);
		string str = termctl->line();
		termctl->clear();

		// failsafe string trim
		str.erase(0, str.find_first_not_of(" \n\r\t\v\b\f"));
		str.erase(str.find_last_not_of(" \n\r\t\v\b\f") + 1);

		this->curr_field = label;

		std::any val;

		if (value_field(type, str, required, val))
			return val;
		else
			continue;
	}

	return -1;
}

//TODO improve
map<int, vector<pair<e2db_console::TYPE, bool>>> e2db_console::input_mask(ENTRY entry_type, bool edit, string id, int ref, string bname)
{
	map<int, vector<pair<TYPE, bool>>> mask;

	if (edit && id.empty())
	{
		*perr << "Error: " << msg("Wrong parameter identifier.") << pout->endl();

		return mask;
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

			auto &props = mask[0];
			props.emplace_back(pair (TYPE::yname, true));
			props.emplace_back(pair (TYPE::pos, true));
			props.emplace_back(pair (TYPE::sys, false));
			props.emplace_back(pair (TYPE::tsid, true));
			props.emplace_back(pair (TYPE::onid, true));
			props.emplace_back(pair (TYPE::dvbns, false));
			props.emplace_back(pair (TYPE::freq, true));

			{
				auto &props = mask[1];
				props.emplace_back(pair (TYPE::pol, true));
				props.emplace_back(pair (TYPE::sr, false));
				props.emplace_back(pair (TYPE::fec, false));
				props.emplace_back(pair (TYPE::mod, false));
				props.emplace_back(pair (TYPE::inv, false));
				props.emplace_back(pair (TYPE::rol, false));
				props.emplace_back(pair (TYPE::pil, false));
				props.emplace_back(pair (TYPE::flags, false));

				props.emplace_back(pair (TYPE::txdata, false));
				{
					auto &props = mask[-1];
					props.emplace_back(pair (TYPE::isid, false));
					props.emplace_back(pair (TYPE::plscode, false));
					props.emplace_back(pair (TYPE::plsmode, false));
					props.emplace_back(pair (TYPE::t2mi_plpid, false));
					props.emplace_back(pair (TYPE::t2mi_pid, false));
				}
			}
			{
				auto &props = mask[2];
				props.emplace_back(pair (TYPE::tmod, false));
				props.emplace_back(pair (TYPE::band, false));
				props.emplace_back(pair (TYPE::tmx, false));
				props.emplace_back(pair (TYPE::hpfec, false));
				props.emplace_back(pair (TYPE::lpfec, false));
				props.emplace_back(pair (TYPE::tinv, false));
				props.emplace_back(pair (TYPE::guard, false));
				props.emplace_back(pair (TYPE::hier, false));
				props.emplace_back(pair (TYPE::flags, false));

				props.emplace_back(pair (TYPE::txdata, false));
				{
					auto &props = mask[-2];
					props.emplace_back(pair (TYPE::plpid, false));
				}
			}
			{
				auto &props = mask[3];
				props.emplace_back(pair (TYPE::cmod, false));
				props.emplace_back(pair (TYPE::sr, false));
				props.emplace_back(pair (TYPE::cfec, false));
				props.emplace_back(pair (TYPE::cinv, false));
				props.emplace_back(pair (TYPE::flags, false));
			}
			{
				auto &props = mask[4];
				props.emplace_back(pair (TYPE::amod, false));
				props.emplace_back(pair (TYPE::flags, false));
			}
		}
		else if (entry_type == ENTRY::service)
		{
			if (edit)
			{
				if (! dbih->db.services.count(id))
					throw std::runtime_error (msg("Service \"%s\" not exists.", id));
			}

			auto &props = mask[0];
			props.emplace_back(pair (TYPE::txid, true));

			{
				auto &props = mask[1];
				props.emplace_back(pair (TYPE::stype, true));
				props.emplace_back(pair (TYPE::ssid, true));
				props.emplace_back(pair (TYPE::chname, true));
				props.emplace_back(pair (TYPE::snum, false));
				props.emplace_back(pair (TYPE::srcid, false));

				props.emplace_back(pair (TYPE::chdata, false));
			}
			{
				auto &props = mask[-1];
				props.emplace_back(pair (TYPE::sdata_p, false));
				props.emplace_back(pair (TYPE::sdata_c, false));
				props.emplace_back(pair (TYPE::sdata_C, false));
				props.emplace_back(pair (TYPE::sdata_f, false));
			}
		}
		else if (entry_type == ENTRY::bouquet)
		{
			e2db::bouquet bs;

			if (edit)
			{
				if (! dbih->bouquets.count(id))
					throw std::runtime_error (msg("Bouquet \"%s\" not exists.", id));
			}

			auto &props = mask[0];
			props.emplace_back(pair (TYPE::btype, true));
			if (edit)
				props.emplace_back(pair (TYPE::rname, false));
			else
				props.emplace_back(pair (TYPE::bname, true));
			props.emplace_back(pair (TYPE::qname, true));
			props.emplace_back(pair (TYPE::nname, false));
		}
		else if (entry_type == ENTRY::userbouquet)
		{
			if (edit)
			{
				if (! dbih->userbouquets.count(id))
					throw std::runtime_error (msg("Userbouquet \"%s\" not exists.", id));
			}

			auto &props = mask[0];
			if (edit)
				props.emplace_back(pair (TYPE::rname, false));
			else
				props.emplace_back(pair (TYPE::bname, true));
			props.emplace_back(pair (TYPE::pname, true));
			props.emplace_back(pair (TYPE::qname, true));
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

					if (! dbih->tuners.count(tvid))
						throw 1;
				}
				catch (...)
				{
					throw std::runtime_error (msg("Tuner settings \"%s\" not exists.", id));
				}
			}

			auto &props = mask[0];
			props.emplace_back(pair (TYPE::ytype, true));
			props.emplace_back(pair (TYPE::charset, false));
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

			auto &props = mask[0];
			props.emplace_back(pair (TYPE::ytype, true));
			props.emplace_back(pair (TYPE::tname, true));

			{
				auto &props = mask[1];
				props.emplace_back(pair (TYPE::pos, true));
				props.emplace_back(pair (TYPE::flags, false));
			}
			{
				auto &props = mask[2];
				props.emplace_back(pair (TYPE::country, false));
				props.emplace_back(pair (TYPE::flags, false));
			}
			{
				auto &props = mask[3];
				props.emplace_back(pair (TYPE::country, false));
				props.emplace_back(pair (TYPE::feed, false));
				props.emplace_back(pair (TYPE::flags, false));
			}
			{
				auto &props = mask[4];
				props.emplace_back(pair (TYPE::flags, false));
			}
		}
		else if (entry_type == ENTRY::tunersets_transponder)
		{
			e2db::tunersets tv;
			e2db::tunersets_table tn;

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
							break;
							break;
						}
					}
				}

				if (! found)
					throw std::runtime_error (msg("Tuner settings transponder \"%s\" not exists.", id));
			}

			auto &props = mask[0];
			props.emplace_back(pair (TYPE::sys, false));
			props.emplace_back(pair (TYPE::freq, true));

			if (tn.ytype == e2db::YTYPE::satellite)
			{
				auto &props = mask[1];
				props.emplace_back(pair (TYPE::pol, true));
				props.emplace_back(pair (TYPE::sr, false));
				props.emplace_back(pair (TYPE::fec, false));
				props.emplace_back(pair (TYPE::mod, false));
				props.emplace_back(pair (TYPE::inv, false));
				props.emplace_back(pair (TYPE::rol, false));
				props.emplace_back(pair (TYPE::pil, false));

				props.emplace_back(pair (TYPE::txdata, false));
				{
					auto &props = mask[-1];
					props.emplace_back(pair (TYPE::plsn, false));
					props.emplace_back(pair (TYPE::plscode, false));
					props.emplace_back(pair (TYPE::plsmode, false));
					props.emplace_back(pair (TYPE::isid, false));
					props.emplace_back(pair (TYPE::mts, false));
				}
			}
			else if (tn.ytype == e2db::YTYPE::terrestrial)
			{
				auto &props = mask[1];
				props.emplace_back(pair (TYPE::tmod, false));
				props.emplace_back(pair (TYPE::band, false));
				props.emplace_back(pair (TYPE::tmx, false));
				props.emplace_back(pair (TYPE::hpfec, false));
				props.emplace_back(pair (TYPE::lpfec, false));
				props.emplace_back(pair (TYPE::tinv, false));
				props.emplace_back(pair (TYPE::guard, false));
				props.emplace_back(pair (TYPE::hier, false));

				props.emplace_back(pair (TYPE::txdata, false));
				{
					auto &props = mask[-1];
					props.emplace_back(pair (TYPE::plpid, false));
				}
			}
			else if (tn.ytype == e2db::YTYPE::cable)
			{
				auto &props = mask[1];
				props.emplace_back(pair (TYPE::cmod, false));
				props.emplace_back(pair (TYPE::sr, false));
				props.emplace_back(pair (TYPE::cfec, false));
				props.emplace_back(pair (TYPE::cinv, false));
			}
			else if (tn.ytype == e2db::YTYPE::atsc)
			{
				auto &props = mask[1];
				props.emplace_back(pair (TYPE::amod, false));
			}
		}
		else if (entry_type == ENTRY::channel_reference)
		{
			if (! dbih->userbouquets.count(bname))
				throw std::runtime_error (msg("Userbouquet \"%s\" not exists.", bname));

			auto &props = mask[0];

			if (ref == 0) // marker
			{
				props.emplace_back(pair (TYPE::mname, false));
				props.emplace_back(pair (TYPE::atype, false));

				props.emplace_back(pair (TYPE::ffdata, false));
				{
					auto &props = mask[-1];
					props.emplace_back(pair (TYPE::mnum, false));
				}
			}
			else if (ref == 2) // stream
			{
				props.emplace_back(pair (TYPE::etype, false));
				props.emplace_back(pair (TYPE::chvalue, false));
				props.emplace_back(pair (TYPE::churl, false));
			}
			else // service
			{
				if (! dbih->db.services.count(id))
					throw std::runtime_error (msg("Service \"%s\" not exists.", id));
			}
		}
	}
	catch (const std::invalid_argument& err)
	{
		*perr << "Error: " << msg(MSG::except_invalid_argument, err.what()) << pout->endl();
	}
	catch (const std::runtime_error& err)
	{
		*perr << "Error: " << err.what() << pout->endl();
	}
	catch (...)
	{
		*perr << "Error: " << msg(MSG::except_uncaught) << pout->endl();
	}

	return mask;
}

string e2db_console::obj_escape(ESCAPE esc, VALUE value_type)
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

void e2db_console::print_obj_begin(int depth)
{
	if (__objio.out == OBJIO::tabular)
	{
		if (depth)
			*pout << '[' << ' ';
	}
	else if (__objio.out == OBJIO::byline)
	{
		if (depth)
			*pout << '[' << pout->endl();
	}
	else if (__objio.out == OBJIO::json)
	{
		if (depth == 1)
			*pout << '[';
		else
			*pout << '{';
	}
}

void e2db_console::print_obj_end(int depth)
{
	if (__objio.out == OBJIO::tabular)
	{
		if (depth)
			*pout << ' ' << ']';
	}
	else if (__objio.out == OBJIO::byline)
	{
		if (depth)
			*pout << ']' << pout->endl();
	}
	else if (__objio.out == OBJIO::json)
	{
		if (depth == 1)
			*pout << ']';
		else
			*pout << '}';
	}
}

void e2db_console::print_obj_sep(int xpos)
{
	if (__objio.out == OBJIO::tabular)
	{
		if (xpos == 0)
			*pout << '\t' << ' ';
	}
	else if (__objio.out == OBJIO::byline)
	{
		*pout << pout->endl();
	}
	else if (__objio.out == OBJIO::json)
	{
		if (xpos == 0)
			*pout << ',' << ' ';
	}
}

void e2db_console::print_obj_dlm(int depth, int xpos)
{
	if (__objio.out == OBJIO::tabular)
	{
		if (depth == 2)
		{
			if (xpos == 0)
				*pout << '\t';
		}
		else
		{
			*pout << pout->endl() << pout->endl();
		}
	}
	else if (__objio.out == OBJIO::byline)
	{
		*pout << pout->endl();
	}
	else if (__objio.out == OBJIO::json)
	{
		if (xpos == 0)
			*pout << ',' << ' ';
		if (depth == 0)
			*pout << pout->endl() << pout->endl();
	}
}

void e2db_console::print_obj_pair(TYPE type, std::any val)
{
	string name;
	VALUE value_type;

	label_obj_pair(type, name, value_type);

	int d = -1;
	string str;

	value_obj_pair(type, value_type, val, d, str);

	*pout << obj_escape(ESCAPE::name_begin, value_type);
	*pout << name;
	*pout << obj_escape(ESCAPE::name_end, value_type);
	*pout << obj_escape(ESCAPE::divider, value_type);
	*pout << obj_escape(ESCAPE::value_begin, value_type);
	if (value_type == VALUE::val_int)
	{
		*pout << d;
	}
	//TODO
	else if (value_type == VALUE::val_obj)
	{
	}
	else
	{
		*pout << str;
	}
	*pout << obj_escape(ESCAPE::value_end, value_type);
}

}
