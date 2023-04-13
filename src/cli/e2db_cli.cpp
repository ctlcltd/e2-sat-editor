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
#include <iomanip>

#include "e2db_termctl.h"

#include "e2db_cli.h"

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

		string opt = argv[i];

		if (opt == "-v" || opt == "--version")
			return cmd_version();
		else if (opt == "-h" || opt == "--help")
			return cmd_usage();
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

	string cmd;

	e2db_termctl* term = new e2db_termctl;

	while (true)
	{
		std::istream* is = term->input();
		*is >> cmd;
		term->clear();

		if (cmd == "quit" || cmd == "exit" || cmd == "q")
		{
			// term->tmp_history();
			return shell_exit();
		}
		else if (cmd == "version" || cmd == "v")
			shell_command_version();
		else if (cmd == "help" || cmd == "h")
			shell_command_help();
		else if (cmd == "read" || cmd == "i")
			shell_command_read();
		else if (cmd == "list" || cmd == "l")
			shell_command_list();
		else if (cmd == "add" || cmd == "a")
			shell_command_add();
		else if (cmd == "edit" || cmd == "e")
			shell_command_edit();
		else if (cmd == "remove" || cmd == "r")
			shell_command_remove();
		else if (cmd == "set" || cmd == "s")
			shell_command_set();
		else if (cmd == "unset" || cmd == "u")
			shell_command_unset();
		else if (cmd == "print" || cmd == "p")
			shell_command_print();
		else if (cmd == "debug" || cmd == "d")
			shell_debugger();
		else if (! cmd.empty())
			shell_error(cmd);

		cout << "input: " << cmd << endl;

		cmd.clear();
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
	cout << "add" << endl;
	cin >> std::ws;
	string id;
	std::getline(cin, id);

	if (id == "service")
		shell_entry_edit(ENTRY::service);
	else if (id == "transponder")
		shell_entry_edit(ENTRY::transponder);
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

void e2db_cli::shell_entry_edit(ENTRY entry_type, string id)
{
	using std::any_cast;

	try
	{
		if (entry_type == ENTRY::transponder)
		{
			e2db::transponder tx;

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

			dbih->add_transponder(tx);
		}
		else if (entry_type == ENTRY::service)
		{
			e2db::service ch;

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
		}
	}
	catch (const std::runtime_error& err)
	{
		//TODO FIX
		size_t csize = sizeof(err.what()) + this->last_is.size();
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
		case TYPE::chdata: label = "Service Data"; description = "[Y]es or [N]one"; break;
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
		case TYPE::sys: label = "System"; description = "exact match: DVB-S, DVB-T, DVB-C, ATSC, DVB-S2, DVB-T2, DVB-S/S2, DVB-T/T2, DVB-C ANNEX B"; break;
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
		is.erase(0, is.find_first_not_of(" \n\r\t\v\a\b\f"));
		is.erase(is.find_last_not_of(" \n\r\t\v\a\b\f") + 1);

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
