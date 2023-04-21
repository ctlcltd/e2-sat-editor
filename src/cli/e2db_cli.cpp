/*!
 * e2-sat-editor/src/cli/e2db_cli.cpp
 *
 * @link https://github.com/ctlcltd/e2-sat-editor
 * @copyright e2 SAT Editor Team
 * @author Leonardo Laureti
 * @version 0.6
 * @license MIT License
 * @license GNU GPLv3 License
 */

#include <cstdio>
#include <clocale>
#include <cstring>
#include <cctype>
#include <stdexcept>
#include <iostream>
#include <sstream>
#include <iomanip>

#include "e2db_termctl.h"

#include "e2db_cli.h"

using std::pair, std::cout, std::cerr, std::cin, std::endl, std::left;

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
	cout << "0.6" << endl;
}

void e2db_cli::cmd_shell()
{
	shell_header();

	this->dbih = new e2db;

	e2db_termctl* term = new e2db_termctl;

	while (true)
	{
		term->input();
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
		else if (cmd == "set" || cmd == "s")
			shell_command_set(is);
		else if (cmd == "unset" || cmd == "u")
			shell_command_unset(is);
		else if (cmd == "print" || cmd == "p")
			shell_command_print(is);
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

	cerr << "e2se-cli" << ':' << ' ' << "Illegal option" << ' ' << option << endl;
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
	cerr << "Error" << ':' << ' ' << "Syntax error near" << ' ' << '"' << cmd << '"' << endl;
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

		shell_usage(hint);
	}
	else if (command == COMMAND::read)
	{
		string path;
		*is >> std::skipws >> path;

		shell_file_read(path);
	}
	else if (command == COMMAND::write)
	{
		string path;
		*is >> std::skipws >> path;

		shell_file_write(path);
	}
	else if (command == COMMAND::list)
	{
		string type;
		*is >> std::skipws >> type;

		if (type == "services")
			shell_entry_list(ENTRY::service);
		else if (type == "transponders")
			shell_entry_list(ENTRY::transponder);
		else if (type == "bouquets")
			shell_entry_list(ENTRY::bouquet);
		else if (type == "userbouquets")
			shell_entry_list(ENTRY::userbouquet);
		else if (type == "tunersets_transponders")
			shell_entry_list(ENTRY::tunersets_transponder);
		else if (type == "tunersets_tables")
			shell_entry_list(ENTRY::tunersets_table);
		else if (type == "tunersets")
			shell_entry_list(ENTRY::tunersets);
		else
			cerr << "Type Error" << ':' << ' ' << "Unknown entry type." << endl;
	}
	else if (command == COMMAND::add)
	{
		string type;
		*is >> std::skipws >> type;

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
		else
			cerr << "Type Error" << ':' << ' ' << "Unknown entry type." << endl;
	}
	else if (command == COMMAND::edit)
	{
		string type, id;
		*is >> std::skipws >> type >> id;

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
		else
			cerr << "Type Error" << ':' << ' ' << "Unknown entry type." << endl;
	}
	else if (command == COMMAND::remove)
	{
		string type, id;
		*is >> std::skipws >> type >> id;

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
		else
			cerr << "Type Error" << ':' << ' ' << "Unknown entry type." << endl;
	}
	else if (command == COMMAND::set)
	{
		string type, id;
		*is >> std::skipws >> type >> id;

		if (type == "service")
			shell_entry_parentallock(ENTRY::service, id, true);
		else if (type == "userbouquet")
			shell_entry_parentallock(ENTRY::userbouquet, id, true);
		else
			cerr << "Type Error" << ':' << ' ' << "Unknown entry type." << endl;
	}
	else if (command == COMMAND::unset)
	{
		string type, id;
		*is >> std::skipws >> type >> id;

		if (type == "service")
			shell_entry_parentallock(ENTRY::service, id, false);
		else if (type == "userbouquet")
			shell_entry_parentallock(ENTRY::userbouquet, id, false);
		else
			cerr << "Type Error" << ':' << ' ' << "Unknown entry type." << endl;
	}
	else if (command == COMMAND::print)
	{
		string type;
		*is >> std::skipws >> type;

		if (type == "debug")
			shell_debug(0);
		else if (type == "index")
			shell_debug(1);
		else
			cerr << "Error" << ':' << ' ' << "Unknown command." << endl;
	}
	else
	{
		cerr << "Error" << ':' << ' ' << "Unknown command." << endl;
	}
}

void e2db_cli::shell_usage(string hint)
{
	if (hint.empty())
	{
		cout << "  ", cout.width(7), cout << left << "add", cout << ' ';
		cout.width(24), cout << left << "service  [chid]", cout << ' ' << "Add new entry." << endl;
		cout.width(10), cout << ' ', cout << "transponder  [txid]" << endl;
		cout.width(10), cout << ' ', cout << "userbouquet  [bname]" << endl;
		cout.width(10), cout << ' ', cout << "bouquet  [bname]" << endl;
		cout.width(10), cout << ' ', cout << "tunersets-transponder  [trid]" << endl;
		cout.width(10), cout << ' ', cout << "tunersets-table  [tnid]" << endl;
		cout.width(10), cout << ' ', cout << "tunersets  [ytype]" << endl;
		cout << endl;

		cout << "  ", cout.width(7), cout << left << "edit", cout << ' ';
		cout.width(24), cout << left << "service  [chid]", cout << ' ' << "Edit an entry." << endl;
		cout.width(10), cout << ' ', cout << "transponder  [txid]" << endl;
		cout.width(10), cout << ' ', cout << "userbouquet  [bname]" << endl;
		cout.width(10), cout << ' ', cout << "bouquet  [bname]" << endl;
		cout.width(10), cout << ' ', cout << "tunersets-transponder  [trid]" << endl;
		cout.width(10), cout << ' ', cout << "tunersets-table  [tnid]" << endl;
		cout.width(10), cout << ' ', cout << "tunersets  [ytype]" << endl;
		cout << endl;

		cout << "  ", cout.width(7), cout << left << "remove", cout << ' ';
		cout.width(24), cout << left << "service", cout << ' ' << "Remove an entry." << endl;
		cout.width(10), cout << ' ', cout << "transponder  [txid]" << endl;
		cout.width(10), cout << ' ', cout << "userbouquet  [bname]" << endl;
		cout.width(10), cout << ' ', cout << "bouquet  [bname]" << endl;
		cout.width(10), cout << ' ', cout << "tunersets-transponder  [trid]" << endl;
		cout.width(10), cout << ' ', cout << "tunersets-table  [tnid]" << endl;
		cout.width(10), cout << ' ', cout << "tunersets  [ytype]" << endl;
		cout << endl;

		cout << "  ", cout.width(7), cout << left << "list", cout << ' ';
		cout.width(24), cout << left << "services", cout << ' ' << "List entries." << endl;
		cout.width(10), cout << ' ', cout << "transponders" << endl;
		cout.width(10), cout << ' ', cout << "userbouquets" << endl;
		cout.width(10), cout << ' ', cout << "bouquets" << endl;
		cout.width(10), cout << ' ', cout << "tunersets-transponders" << endl;
		cout.width(10), cout << ' ', cout << "tunersets-tables" << endl;
		cout.width(10), cout << ' ', cout << "tunersets" << endl;
		cout << endl;

		cout << "  ", cout.width(7), cout << left << "set", cout << ' ';
		cout.width(39), cout << left << "parentallock  service  [chid]", cout << ' ' << "Set parental lock." << endl;
		cout.width(10), cout << ' ', cout << "parentallock  userbouquet  [bname]" << endl;
		cout << endl;

		cout << "  ", cout.width(7), cout << left << "unset", cout << ' ';
		cout.width(39), cout << left << "parentallock  service  [chid]", cout << ' ' << "Unset parental lock." << endl;
		cout.width(10), cout << ' ', cout << "parentallock  userbouquet  [bname]" << endl;
		cout << endl;

		cout << "  ", cout.width(32), cout << left << "read", cout << ' ' << "Read from file." << endl;
		cout << endl;

		cout << "  ", cout.width(32), cout << left << "write", cout << ' ' << "Write to file." << endl;
		cout << endl;

		cout << "  ", cout.width(7), cout << left << "print", cout << ' ';
		cout.width(24), cout << left << "debug", cout << ' ' << "Print debug informations." << endl;
		cout.width(10), cout << ' ', cout << "index" << endl;
		cout << endl;

		cout << "  ", cout.width(32), cout << left << "debug", cout << ' ' << "Debugger." << endl;
		cout << endl;

		// cout << "  ", cout.width(7), cout << left << "history", cout << ' ';
		// cout.width(24), cout << left << "file", cout << ' ' << "Save history to file, instead of memory." << endl;
		// cout << endl;

		cout << "  ", cout.width(32), cout << left << "version", cout << ' ' << "Display version." << endl;
		cout << endl;

		cout << "  ", cout.width(32), cout << left << "help", cout << ' ' << "Display this help and exit." << endl;
	}
}

void e2db_cli::shell_file_read(string path)
{
	if (! dbih->read(path))
		cerr << "File Error" << ':' << ' ' << "Error reading file." << endl;
}

void e2db_cli::shell_file_write(string path)
{
	if (! dbih->write(path))
		cerr << "File Error" << ':' << ' ' << "Error writing file." << endl;
}

void e2db_cli::shell_entry_list(ENTRY entry_type, bool paged, int limit)
{
	int pos = 0;
	int offset = 0;
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

		shell_entry_list(entry_type, pos, offset, end);

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

			shell_entry_list(entry_type, pos, offset, end);
		}
	}
	else
	{
		shell_entry_list(entry_type, pos, offset, end);
	}
}

void e2db_cli::shell_entry_list(ENTRY entry_type, int pos, int offset, int& end)
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
			print_obj_pair(TYPE::flgs, tx.flgs), print_obj_sep();
			print_obj_pair(TYPE::oflgs, tx.oflgs), print_obj_sep();
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

			//TODO
			print_obj_pair(TYPE::chdata, NULL);
			print_obj_begin(1), print_obj_sep(-1);
			print_obj_pair(TYPE::sdata_p, ch.data.count(e2db::SDATA::p) ? ch.data[e2db::SDATA::p] : vxnul), print_obj_sep();
			print_obj_pair(TYPE::sdata_c, ch.data.count(e2db::SDATA::c) ? ch.data[e2db::SDATA::c] : vxnul), print_obj_sep();
			print_obj_pair(TYPE::sdata_C, ch.data.count(e2db::SDATA::C) ? ch.data[e2db::SDATA::C] : vxnul), print_obj_sep();
			print_obj_pair(TYPE::sdata_f, ch.data.count(e2db::SDATA::f) ? ch.data[e2db::SDATA::f] : vxnul), print_obj_sep(1);
			print_obj_end(1), print_obj_sep();

			print_obj_pair(TYPE::locked, ch.locked), print_obj_sep();
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
			print_obj_pair(TYPE::locked, ub.locked), print_obj_sep();
			print_obj_pair(TYPE::hidden, ub.hidden), print_obj_sep();
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
			print_obj_pair(TYPE::flgs, tn.flgs), print_obj_sep();
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
			print_obj_pair(TYPE::isid, tntxp.isid), print_obj_sep();
			print_obj_pair(TYPE::plsmode, tntxp.plsmode), print_obj_sep();
			print_obj_pair(TYPE::plscode, tntxp.plscode), print_obj_sep();
			print_obj_pair(TYPE::idx, tntxp.index), print_obj_sep(1);
			print_obj_end(), print_obj_dlm();
		}
	}
}

void e2db_cli::shell_entry_add(ENTRY entry_type)
{
	shell_entry_edit(entry_type, false);
}

void e2db_cli::shell_entry_edit(ENTRY entry_type, string id)
{
	shell_entry_edit(entry_type, true, id);
}

void e2db_cli::shell_entry_edit(ENTRY entry_type, bool edit, string id)
{
	using std::any_cast;

	if (edit && id.empty())
	{
		cerr << "Error" << ':' << ' ' << "Wrong parameter identifier." << endl;
		return;
	}

	this->last_is = id;

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
					throw std::runtime_error ("Transponder \"%s\" not exists.");
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
				tx.flgs = any_cast<int>(field(TYPE::flgs));
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
			}
			else if (tx.ytype == e2db::YTYPE::cable)
			{
				tx.cmod = any_cast<int>(field(TYPE::cmod));
				tx.sr = any_cast<int>(field(TYPE::sr));
				tx.cfec = any_cast<int>(field(TYPE::cfec));
				tx.inv = any_cast<int>(field(TYPE::cinv));
			}
			else if (tx.ytype == e2db::YTYPE::atsc)
			{
				tx.amod = any_cast<int>(field(TYPE::amod));
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
					throw std::runtime_error ("Service \"%s\" not exists.");
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
				throw std::runtime_error ("Transponder \"%s\" not exists.");
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
					throw std::runtime_error ("Bouquet \"%s\" not exists.");
			}

			bs.btype = any_cast<int>(field(TYPE::btype, true));
			if (edit && 1)
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
					throw std::runtime_error ("Userbouquet \"%s\" not exists.");
			}

			if (edit && 1)
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
					throw std::runtime_error ("Tuner settings \"%s\" not exists.");
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
					throw std::runtime_error ("Tuner settings table \"%s\" not exists.");
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

			tn.flgs = any_cast<int>(field(TYPE::flgs));

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
					throw std::runtime_error ("Tuner settings transponder \"%s\" not exists.");
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
					tntxp.isid = any_cast<int>(field(TYPE::isid));
					tntxp.mts = any_cast<int>(field(TYPE::mts));
					tntxp.plsmode = any_cast<int>(field(TYPE::plsmode));
					tntxp.plscode = any_cast<int>(field(TYPE::plscode));
					tntxp.plsn = any_cast<int>(field(TYPE::plsn));
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
	}
	catch (const std::runtime_error& err)
	{
		size_t csize = std::strlen(err.what()) + this->last_is.size();
		char cstr[csize];
		std::snprintf(cstr, csize, err.what(), this->last_is.c_str());

		cerr << "Error" << ':' << ' ' << cstr << endl;
	}
	// catch (const std::bad_any_cast& err)
	// catch (const std::invalid_argument& err)
	catch (...)
	{
		string msg = "Wrong input data in \"%s\" field.";

		size_t csize = msg.size() + this->last_label.size();
		char cstr[csize];
		std::snprintf(cstr, csize, msg.c_str(), this->last_label.c_str());

		cerr << "Error" << ':' << ' ' << cstr << endl;
	}
}

void e2db_cli::shell_entry_remove(ENTRY entry_type, string id)
{
	if (id.empty())
	{
		cerr << "Error" << ':' << ' ' << "Wrong parameter identifier." << endl;
		return;
	}

	this->last_is = id;

	try
	{
		if (entry_type == ENTRY::transponder)
		{
			if (! dbih->db.transponders.count(id))
				throw std::runtime_error ("Transponder \"%s\" not exists.");

			dbih->remove_transponder(id);
		}
		else if (entry_type == ENTRY::service)
		{
			if (! dbih->db.services.count(id))
				throw std::runtime_error ("Service \"%s\" not exists.");

			dbih->remove_service(id);
		}
		else if (entry_type == ENTRY::bouquet)
		{
			if (! dbih->bouquets.count(id))
				throw std::runtime_error ("Bouquet \"%s\" not exists.");

			dbih->remove_bouquet(id);
		}
		else if (entry_type == ENTRY::userbouquet)
		{
			if (! dbih->userbouquets.count(id))
				throw std::runtime_error ("Userbouquet \"%s\" not exists.");

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
				throw std::runtime_error ("Tuner settings \"%s\" not exists.");
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
				throw std::runtime_error ("Tuner settings table \"%s\" not exists.");

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
				throw std::runtime_error ("Tuner settings transponder \"%s\" not exists.");

			dbih->remove_tunersets_transponder(id, tn);
		}
	}
	catch (const std::runtime_error& err)
	{
		size_t csize = std::strlen(err.what()) + this->last_is.size();
		char cstr[csize];
		std::snprintf(cstr, csize, err.what(), this->last_is.c_str());

		cerr << "Error" << ':' << ' ' << cstr << endl;
	}
}

void e2db_cli::shell_entry_parentallock(ENTRY entry_type, string id, bool flag)
{
	if (id.empty())
	{
		cerr << "Error" << ':' << ' ' << "Wrong parameter identifier." << endl;
		return;
	}

	this->last_is = id;

	try
	{
		if (entry_type == ENTRY::service)
		{
			if (! dbih->db.services.count(id))
				throw std::runtime_error ("Service \"%s\" not exists.");

			if (flag)
				dbih->set_service_parentallock(id);
			else
				dbih->unset_service_parentallock(id);
		}
		else if (entry_type == ENTRY::userbouquet)
		{
			if (! dbih->userbouquets.count(id))
				throw std::runtime_error ("Userbouquet \"%s\" not exists.");

			if (flag)
				dbih->set_userbouquet_parentallock(id);
			else
				dbih->unset_userbouquet_parentallock(id);
		}
	}
	catch (const std::runtime_error& err)
	{
		size_t csize = std::strlen(err.what()) + this->last_is.size();
		char cstr[csize];
		std::snprintf(cstr, csize, err.what(), this->last_is.c_str());

		cerr << "Error" << ':' << ' ' << cstr << endl;
	}
}

void e2db_cli::shell_debug(int opt)
{
	if (opt)
		cout << "TODO" << endl;
	else
		dbih->debugger();
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
		case TYPE::refid: name = hrn ? "REFID" : "refid"; value_type = VALUE::val_string; break;
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
		case TYPE::locked: name = hrn ? "Parental locked" : "locked"; value_type = VALUE::val_bool; break;
		case TYPE::chname: name = hrn ? "Service Name" : "chname"; value_type = VALUE::val_string; break;
		case TYPE::sdata_p: name = hrn ? "Provider Name" : "sdata_p"; value_type = VALUE::val_string; break;
		case TYPE::sdata_c: name = hrn ? "Service Cache" : "sdata_c"; value_type = VALUE::val_string; break;
		case TYPE::sdata_C: name = hrn ? "Service CAS" : "sdata_C"; value_type = VALUE::val_string; break;
		case TYPE::sdata_f: name = hrn ? "Service Flags" : "sdata_f"; value_type = VALUE::val_string; break;
		case TYPE::mname: name = hrn ? "Marker Name" : "mname"; value_type = VALUE::val_string; break;
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
		case TYPE::isid: name = hrn ? "isid" : "isid"; value_type = VALUE::val_int; break;
		case TYPE::mts: name = hrn ? "mts" : "mts"; value_type = VALUE::val_int; break;
		case TYPE::plsmode: name = hrn ? "plsmode" : "plsmode"; value_type = VALUE::val_int; break;
		case TYPE::plscode: name = hrn ? "plscode" : "plscode"; value_type = VALUE::val_int; break;
		case TYPE::plsn: name = hrn ? "plsn" : "plsn"; value_type = VALUE::val_int; break;
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
		case TYPE::dname: name = hrn ? "dname" : "dname"; value_type = VALUE::val_string; break;
		case TYPE::itype: name = hrn ? "itype" : "itype"; value_type = VALUE::val_int; break;
		case TYPE::flgs: name = hrn ? "Flags" : "flgs"; value_type = VALUE::val_int; break;
		case TYPE::oflgs: name = hrn ? "Other Flags" : "oflgs"; value_type = VALUE::val_string; break;
		case TYPE::chdata: name = hrn ? "Service data" : "chdata"; value_type = VALUE::val_obj; break;
		case TYPE::txdata: name = hrn ? "Transponder data" : "txdata"; value_type = VALUE::val_obj; break;
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
		case TYPE::isid:
		case TYPE::mts:
		case TYPE::plsmode:
		case TYPE::plscode:
		case TYPE::plsn:
		case TYPE::diseqc:
		case TYPE::uncomtd:
		case TYPE::feed:
		case TYPE::itype:
		case TYPE::flgs:
			d = any_cast<int>(val);
		break;
		case TYPE::locked:
		case TYPE::hidden:
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
		case TYPE::charset:
		case TYPE::tname:
		case TYPE::country:
		case TYPE::bname:
		case TYPE::pname:
		case TYPE::rname:
		case TYPE::qname:
		case TYPE::nname:
		case TYPE::dname:
		case TYPE::oflgs:
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

		//TODO
 		case TYPE::sdata_p:
 			// str = any_cast<string>(val);
		break;
 		case TYPE::sdata_c:
 			// str = any_cast<string>(val);
		break;
 		case TYPE::sdata_C:
 			// str = any_cast<string>(val);
		break;
 		case TYPE::sdata_f:
 			// str = any_cast<string>(val);
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
		case TYPE::locked: label = "Parental locked"; description = "[Y]es or [N]one"; break;
		case TYPE::chname: label = "Service Name"; break;
		case TYPE::sdata_p: label = "Provider Name"; break;
		case TYPE::sdata_c: label = "Service Cache"; description = "comma separated values in hex or <empty>, eg. c:0101,c:0202"; break;
		case TYPE::sdata_C: label = "Service CAS"; description = "comma separated values in hex or <empty>, eg. C:0101,C:0202"; break;
		case TYPE::sdata_f: label = "Service Flags"; description = "comma separated values in hex or <empty>, eg. f:0101,f:0202"; break;
		case TYPE::mname: label = "Marker Name"; break;
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
		case TYPE::isid: label = "isid"; description = "in digits"; break;
		case TYPE::mts: label = "mts"; description = "in digits"; break;
		case TYPE::plsmode: label = "plsmode"; description = "in digits"; break;
		case TYPE::plscode: label = "plscode"; description = "in digits"; break;
		case TYPE::plsn: label = "plsn"; description = "in digits"; break;
		case TYPE::pos: label = "Position"; description = "in degree, eg. 0.0E, 0.0W"; break;
		case TYPE::diseqc: label = "diseqc"; description = "in digits"; break;
		case TYPE::uncomtd: label = "uncomtd"; description = "in digits"; break;
		case TYPE::charset: label = "Charset"; description = "characters encoding: UTF-8, ISO-8859-1"; break;
		case TYPE::tname: label = "Position Name"; description = "eg. Sputnik 0.0E"; break;
		case TYPE::country: label = "Country"; description = "Country Code, 3 letters ISO-3166, eg. XYZ"; break;
		case TYPE::feed: label = "Feed"; description = "[Y]es or [N]one"; break;
		case TYPE::bname: label = "Bouquet Filename [bname]"; description = "eg. userbouquet.dbe01.tv, bouquet.radio"; break;
		case TYPE::pname: label = "Parent Bouquet [bname]"; description = "eg. bouquets.tv, bouquets.radio"; break;
		case TYPE::rname: label = "New Bouquet Name [rname]"; description = "eg. userbouquet.dbe01.tv, bouquet.radio"; break;
		case TYPE::qname: label = "Bouquet Name"; description = "eg. User - bouquet (TV)"; break;
		case TYPE::nname: label = "Bouquet Nice Name"; description = "eg. TV, Radio"; break;
		case TYPE::btype: label = "Bouquet Type"; description = "TV, Radio"; break;
		case TYPE::hidden: label = "Hidden"; description = "[Y]es or [N]one"; break;
		case TYPE::dname: label = "dname"; break;
		case TYPE::itype: label = "itype"; break;
		case TYPE::flgs: label = "Flags"; description = "in digits"; break;
		case TYPE::oflgs: label = "Other Flags"; break;
		case TYPE::chdata: label = "Add Service data?"; description = "[Y]es or [N]one"; break;
		case TYPE::txdata: label = "Add Transponder data?"; description = "[Y]es or [N]one"; break;
		default: return -1;
	}

	string is;

	while (true)
	{
		cout << label;
		if (! description.empty())
			cout << ' ' << '(' << description << ')';
		if (required)
			cout << ' ' << '*';
		cout << ':' << ' ';

		cin >> is;

		// failsafe string trim
		is.erase(0, is.find_first_not_of(" \n\r\t\v\b\f"));
		is.erase(is.find_last_not_of(" \n\r\t\v\b\f") + 1);

		this->last_label = label;
		this->last_is = is;

		if (! is.empty())
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
				case TYPE::isid:
				case TYPE::mts:
				case TYPE::plsmode:
				case TYPE::plscode:
				case TYPE::plsn:
				case TYPE::diseqc:
				case TYPE::uncomtd:
				case TYPE::flgs:
					d = std::atoi(is.data());
					if (! d && required)
						continue;
				break;
				case TYPE::dvbns:
					d = std::stol(is, nullptr, 16);
					if (d == 0 && required)
						continue;
				break;
				case TYPE::dbtype:
					d = std::atoi(is.data());
					if (d < 0 || d > 1 && required)
						continue;
				break;
				case TYPE::dbparental:
					d = std::atoi(is.data());
					if (d < 0 || d > 2 && required)
						continue;
				break;
				case TYPE::ytype:
					d = std::atoi(is.data());
					if (d < 0 || d > 3 && required)
						continue;
				break;
				case TYPE::chdata:
				case TYPE::txdata:
				case TYPE::locked:
				case TYPE::feed:
				case TYPE::hidden:
					if (is == "Y" || is == "y")
						d = 1;
					else if (is == "N" || is == "n")
						d = 0;
					else if (required)
						continue;
				break;
				case TYPE::sdata_p:
					return { is };
				break;
				case TYPE::yname:
					if (is.size() == 1)
						d = dbih->value_transponder_type(is[0]);
					else if (required)
						continue;
				break;
				case TYPE::sys:
					// failsafe string uppercase
					std::transform(is.begin(), is.end(), is.begin(), [](unsigned char c) { return std::toupper(c); });
					d = dbih->value_transponder_system(is);
				break;
				case TYPE::pos:
					// failsafe string uppercase
					std::transform(is.begin(), is.end(), is.begin(), [](unsigned char c) { return std::toupper(c); });
					d = dbih->value_transponder_position(is);
				break;
				case TYPE::pol:
					// failsafe string uppercase
					std::transform(is.begin(), is.end(), is.begin(), [](unsigned char c) { return std::toupper(c); });
					d = dbih->value_transponder_polarization(is);
				break;
				case TYPE::fec:
					d = dbih->value_transponder_fec(is, e2db::YTYPE::satellite);
				break;
				case TYPE::hpfec:
				case TYPE::lpfec:
					d = dbih->value_transponder_fec(is, e2db::YTYPE::terrestrial);
				break;
				case TYPE::cfec:
					d = dbih->value_transponder_fec(is, e2db::YTYPE::atsc);
				break;
				case TYPE::mod:
					d = dbih->value_transponder_modulation(is, e2db::YTYPE::satellite);
				break;
				case TYPE::tmod:
					d = dbih->value_transponder_modulation(is, e2db::YTYPE::terrestrial);
				break;
				case TYPE::cmod:
					d = dbih->value_transponder_modulation(is, e2db::YTYPE::cable);
				break;
				case TYPE::amod:
					d = dbih->value_transponder_modulation(is, e2db::YTYPE::atsc);
				break;
				case TYPE::inv:
					d = dbih->value_transponder_inversion(is, e2db::YTYPE::satellite);
				break;
				case TYPE::tinv:
					d = dbih->value_transponder_inversion(is, e2db::YTYPE::terrestrial);
				break;
				case TYPE::cinv:
					d = dbih->value_transponder_inversion(is, e2db::YTYPE::cable);
				break;
				case TYPE::rol:
					d = dbih->value_transponder_rollof(is);
				break;
				case TYPE::pil:
					d = dbih->value_transponder_pilot(is);
				break;
				case TYPE::band:
					d = dbih->value_transponder_bandwidth(is);
				break;
				case TYPE::tmx:
					d = dbih->value_transponder_tmx_mode(is);
				break;
				case TYPE::guard:
					d = dbih->value_transponder_guard(is);
				break;
				case TYPE::hier:
					d = dbih->value_transponder_hier(is);
				break;
				case TYPE::stype:
					d = dbih->value_service_type(is);
				break;
				case TYPE::btype:
					d = dbih->value_bouquet_type(is);
				break;
				case TYPE::country:
					if (is.size() < 3 || is.size() > 3 && required)
						continue;
					// failsafe string uppercase
					std::transform(is.begin(), is.end(), is.begin(), [](unsigned char c) { return std::toupper(c); });
					return is;
				break;
				case TYPE::charset:
					// failsafe string uppercase
					std::transform(is.begin(), is.end(), is.begin(), [](unsigned char c) { return std::toupper(c); });
					return is;
				break;
				default:
					return is;
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
