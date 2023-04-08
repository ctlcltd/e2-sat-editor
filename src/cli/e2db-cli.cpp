/*!
 * e2-sat-editor/src/cli/e2db-cli.cpp
 *
 * @link https://github.com/ctlcltd/e2-sat-editor
 * @copyright e2 SAT Editor Team
 * @author Leonardo Laureti
 * @version 0.6
 * @license MIT License
 * @license GNU GPLv3 License
 */

#include <clocale>
#include <cstring>
#include <iostream>
#include <iomanip>

#include "e2db-cli.h"

using std::cout, std::cerr, std::cin, std::endl, std::left;

namespace e2se_cli
{

e2db_cli::e2db_cli(int argc, char* argv[])
{
	std::setlocale(LC_NUMERIC, "C");

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

		string option = argv[i];

		if (option == "-v" || option == "--version")
			return cmd_version();
		else if (option == "-h" || option == "--help")
			return cmd_usage();
		else if (option == "-s" || option == "--shell")
			return cmd_shell();
		else
			return cmd_error(option);
			
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

	string is;

	while (true) {
		cout << '>';
		cout.flush();
		cin >> is;

		if (cin.fail())
			break;

		if (is == "quit" || is == "exit" || is == "q")
			return shell_exit();
		if (is == "version" || is == "v")
			shell_command_version();
		else if (is == "help" || is == "h")
			shell_command_help();
		else if (is == "read" || is == "i")
			shell_command_read();
		else if (is == "list" || is == "l")
			shell_command_list();
		else if (is == "add" || is == "a")
			shell_command_add();
		else if (is == "edit" || is == "e")
			shell_command_edit();
		else if (is == "remove" || is == "r")
			shell_command_remove();
		else if (is == "set" || is == "s")
			shell_command_set();
		else if (is == "unset" || is == "u")
			shell_command_unset();
		else if (is == "print" || is == "p")
			shell_command_print();
		else if (is == "debug" || is == "d")
			shell_debugger();
		else
			shell_error(is);

		// cout << "input: " << is << endl;
	}
}

void e2db_cli::cmd_version()
{
	version(false);
}

void e2db_cli::cmd_error(string option)
{
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
	exit(0);
}

void e2db_cli::shell_header()
{
	version(true);
	cout << endl;
	cout << "Enter \"help\" to display usage informations." << endl;
	cout << endl;
}

void e2db_cli::shell_error(string is)
{
	cerr << "Error" << ':' << ' ' << "Syntax error near" << ' ' << '"' << is << '"' << endl;
}

void e2db_cli::shell_command_version()
{
	version(true);
}

void e2db_cli::shell_command_help()
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
	cout.width(24), cout << left << "index", cout << ' ' << "Print debug informations." << endl;
	cout << endl;

	cout << "  ", cout.width(32), cout << left << "debug", cout << ' ' << "Debugger." << endl;
	cout << endl;

	cout << "  ", cout.width(7), cout << left << "history", cout << ' ';
	cout.width(24), cout << left << "file", cout << ' ' << "Save history to file, instead of memory." << endl;
	cout << endl;

	cout << "  ", cout.width(32), cout << left << "version", cout << ' ' << "Display version." << endl;
	cout << endl;

	cout << "  ", cout.width(32), cout << left << "help", cout << ' ' << "Display this help and exit." << endl;
}

void e2db_cli::shell_command_read()
{
	cin >> std::ws;
	string path;
	std::getline(cin, path);

	dbih->read(path);
}

void e2db_cli::shell_command_add()
{
	cin >> std::ws;
	string id;
	std::getline(cin, id);

	if (0)
		shell_debugger();
	else if (id == "service")
		add_service();
	else if (id == "transponder")
		add_transponder();
}

void e2db_cli::shell_debugger()
{
	if (this->dbih == nullptr)
	{
		cerr << "Error" << ':' << ' ' << "No file opened." << endl;

		return;
	}

	dbih->debugger();
}

void e2db_cli::add_transponder()
{
	string question = ": ";
	string required = " *";

	string is;

	e2db::transponder tx;

	while (true) {
		cout << "DVBNS" << required << question, cin >> is;
		if (! is.empty()) { tx.dvbns = std::atoi(is.data()); break; }
	}
	while (true) {
		cout << "TSID" << required << question, cin >> is;
		if (! is.empty()) { tx.tsid = std::atoi(is.data()); break; }
	}
	while (true) {
		cout << "ONID" << required << question, cin >> is;
		if (! is.empty()) { tx.onid = std::atoi(is.data()); break; }
	}
	while (true) {
		cout << "Position" << required << question, cin >> is;
		if (! is.empty()) { tx.pos = std::atoi(is.data()); break; }
	}
	while (true) {
		cout << "Transponder Type" << required << question, cin >> is;
		if (! is.empty()) { tx.ytype = std::atoi(is.data()); break; }
	}
	while (true) {
		cout << "Frequency" << required << question, cin >> is;
		if (! is.empty()) { tx.freq = std::atoi(is.data()); break; }
	}
	if (tx.ytype == ::e2se_e2db::e2db::YTYPE::satellite)
	{
		{
			cout << "Symbol Rate" << question, cin >> is;
			if (! is.empty()) { tx.sr = std::atoi(is.data()); }
		}
		{
			cout << "Polarization" << question, cin >> is;
			if (! is.empty()) { tx.pol = std::atoi(is.data()); }
		}
		{
			cout << "FEC" << question, cin >> is;
			if (! is.empty()) { tx.fec = std::atoi(is.data()); }
		}
	}

	dbih->add_transponder(tx);
}

void e2db_cli::add_service()
{
	string question = ": ";
	string required = " *";

	string is;

	e2db::service ch;

	while (true) {
		cout << "SSID" << required << question, cin >> is;
		if (! is.empty()) { ch.ssid = std::atoi(is.data()); break; }
	}
	while (true) {
		cout << "DVBNS" << required << question, cin >> is;
		if (! is.empty()) { ch.dvbns = std::atoi(is.data()); break; }
	}
	while (true) {
		cout << "ONID" << required << question, cin >> is;
		if (! is.empty()) { ch.onid = std::atoi(is.data()); break; }
	}
	while (true) {
		cout << "TSID" << required << question, cin >> is;
		if (! is.empty()) { ch.tsid = std::atoi(is.data()); break; }
	}
	{
		cout << "Service Name" << question, cin >> is;
		if (! is.empty()) { ch.chname = is; }
	}
	while (true) {
		cout << "Service Type" << required << question, cin >> is;
		if (! is.empty()) { ch.stype = std::atoi(is.data()); break; }
	}
	{
		cout << "Service Number" << question, cin >> is;
		if (! is.empty()) { ch.snum = std::atoi(is.data()); }
	}
	{
		cout << "Src ID" << question, cin >> is;
		if (! is.empty()) { ch.srcid = std::atoi(is.data()); }
	}
	{
		cout << "Service Data" << question, cin >> is;
		if (! is.empty())
		{
			{
				cout << "Provider Name" << question, cin >> is;
				if (! is.empty()) { ch.data[::e2db::SDATA::p] = { is }; }
			}
		}
	}

	dbih->add_service(ch);
}

}
