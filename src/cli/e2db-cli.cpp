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

	if (entry_type == ENTRY::transponder)
	{
		e2db::transponder tx;

		tx.ytype = any_cast<int>(field(TYPE::yname, true));
		tx.pos = any_cast<int>(field(TYPE::pos, true));
		tx.sys = any_cast<int>(field(TYPE::sys));
		tx.tsid = any_cast<int>(field(TYPE::tsid, true));
		tx.onid = any_cast<int>(field(TYPE::onid, true));
		tx.dvbns = dbih->value_transponder_dvbns(tx);
		tx.freq = any_cast<int>(field(TYPE::freq, true));

		if (tx.ytype == e2db::YTYPE::satellite)
		{
			tx.pol = any_cast<int>(field(TYPE::pol, true));
			tx.sr = any_cast<int>(field(TYPE::sr));
			tx.fec = any_cast<int>(field(TYPE::fec));
			tx.mod = any_cast<int>(field(TYPE::mod));
			// tx.inv = any_cast<int>(field(TYPE::inv));
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
			// tx.inv = any_cast<int>(field(TYPE::inv));
			tx.guard = any_cast<int>(field(TYPE::guard));
			tx.hier = any_cast<int>(field(TYPE::hier));
		}
		else if (tx.ytype == e2db::YTYPE::cable)
		{
			tx.cmod = any_cast<int>(field(TYPE::cmod));
			tx.sr = any_cast<int>(field(TYPE::sr));
			tx.cfec = any_cast<int>(field(TYPE::cfec));
			// tx.inv = any_cast<int>(field(TYPE::inv));
		}
		else if (tx.ytype == e2db::YTYPE::atsc)
		{
			tx.amod = any_cast<int>(field(TYPE::amod));
		}

		dbih->add_transponder(tx);
	}
	else if (entry_type == ENTRY::service)
	{
		e2db::service ch;

		ch.txid = any_cast<string>(field(TYPE::txid, true));
		ch.stype = any_cast<int>(field(TYPE::stype, true));
		ch.ssid = any_cast<int>(field(TYPE::ssid, true));
		ch.chname = any_cast<string>(field(TYPE::chname, true));
		// ch.data;
		ch.snum = any_cast<int>(field(TYPE::snum));
		ch.srcid = any_cast<int>(field(TYPE::srcid));

		if (dbih->db.transponders.count(ch.txid))
		{
			e2db::transponder tx = dbih->db.transponders[ch.txid];

			ch.tsid = tx.tsid;
			ch.onid = tx.onid;
			ch.dvbns = tx.dvbns;
		}
		else
		{
			cerr << "Error" << ':' << ' ' << "Transponder \"%s\" not exists." << endl;
		}
	}
}

std::any e2db_cli::field(TYPE type, bool required)
{
	string label, description;

	switch (type)
	{
		case TYPE::type: label = "type"; break;
		case TYPE::parental: label = "parental"; break;
		case TYPE::index: label = "index"; break;
		case TYPE::chid: label = "CHID"; description = "Channel ID [ssid]:[tsid]:[dvbns] eg. 4d2:3e8:eeee0000"; break;
		case TYPE::txid: label = "TXID"; description = "Transponder ID [tsid]:[dvbns] eg. 3e8:eeee0000"; break;
		case TYPE::refid: label = "REFID"; description = "Reference ID, colon separated values"; break;
		case TYPE::tnid: label = "TNID"; description = "Tunersets Table ID [yname]:[idx] eg. s:0001"; break;
		case TYPE::trid: label = "TRID"; description = "Tunersets Transponder ID [yname]:[freq]:[sr] eg. s:2710:55f0"; break;
		case TYPE::yname: label = "YNAME"; description = "Tuner Type: s = satellite, t = terrestrial, c = cable, a = atsc"; break;
		case TYPE::ytype: label = "YTYPE"; description = "Tuner Type: 0 = satellite, 1 = terrestrial, 2 = cable, 3 = atsc"; break;
		case TYPE::ssid: label = "SSID"; description = "Service ID, in digits"; break;
		case TYPE::dvbns: label = "DVBNS"; break;
		case TYPE::tsid: label = "TSID"; description = "Transport ID, in digits"; break;
		case TYPE::onid: label = "ONID"; break;
		case TYPE::stype: label = "Service Type"; description = "Data, TV, Radio, HD, H.264, H.265, UHD"; break;
		case TYPE::snum: label = "snum"; description = "Service Number"; break;
		case TYPE::srcid: label = "srcid"; description = "Source ID"; break;
		case TYPE::locked: label = "locked"; break;
		case TYPE::chname: label = "Service Name"; break;
		case TYPE::mname: label = "Marker Name"; break;
		case TYPE::marker: label = "Marker"; break;
		case TYPE::freq: label = "Frequency"; description = "in Hertz, 6 digits"; break;
		case TYPE::sr: label = "Symbol Rate"; break;
		case TYPE::pol: label = "Polarization"; description = "H = horizontal, V = vertical, L = Left Circular, R = Right Circular"; break;
		case TYPE::fec: label = "FEC"; description = "eg. 3/4, <empty> for Auto"; break;
		case TYPE::hpfec: label = "HP FEC"; description = "eg. 3/4, <empty> for Auto"; break;
		case TYPE::lpfec: label = "LP FEC"; description = "eg. 3/4, <empty> for Auto"; break;
		case TYPE::cfec: label = "Inner FEC"; description = "eg. 3/4, <empty> for Auto"; break;
		case TYPE::inv: label = "Inversion"; description = "<empty>, Off, On"; break;
		case TYPE::sys: label = "System"; description = "eg. DVB-S or ATSC"; break;
		case TYPE::mod: case TYPE::cmod: case TYPE::amod: label = "Modulation"; break;
		case TYPE::tmod: label = "Constellation"; break;
		case TYPE::rol: label = "Roll Offset"; description = "<empty>, 0.35, 0.25, 0.20"; break;
		case TYPE::pil: label = "Pilot"; description = "<empty>, Off, On, Auto"; break;
		case TYPE::band: label = "Bandwidth"; description = "in MHz or <empty>, eg. 8"; break;
		case TYPE::tmx: label = "Transmission Mode"; description = "<empty>, 2k, 8k, 4k, 1k, 16k, 32k"; break;
		case TYPE::guard: label = "Guard Interval"; description = "<empty>, 1/32, 1/16, 1/8, 1/4, 1/128, 19/128, 19/256"; break;
		case TYPE::hier: label = "Hierarchy"; description = "<empty>, 0, 1, 2, 4"; break;
		case TYPE::isid: label = "isid"; break;
		case TYPE::mts: label = "mts"; break;
		case TYPE::plsmode: label = "plsmode"; break;
		case TYPE::plscode: label = "plscode"; break;
		case TYPE::plsn: label = "plsn"; break;
		case TYPE::pos: label = "Position"; description = "in degree, eg. 0.0E, 0.0W"; break;
		case TYPE::diseqc: label = "diseqc"; break;
		case TYPE::uncomtd: label = "uncomtd"; break;
		case TYPE::charset: label = "Charset"; description = "characters encoding: UTF-8, ISO-8859-1"; break;
		case TYPE::tname: label = "Position Name"; description = "eg. Sputnik 0.0E"; break;
		case TYPE::country: label = "Country"; description = "3 letters ISO-3166 Country Code, eg. XYZ"; break;
		case TYPE::feed: label = "Feed"; break;
		case TYPE::bname: label = "Bouquet Filename [bname]"; description = "eg. userbouquet.dbe01.tv, bouquet.radio"; break;
		case TYPE::pname: label = "Parent Bouquet [bname]"; description = "eg. bouquets.tv, bouquets.radio"; break;
		case TYPE::rname: label = "New Bouquet Name [rname]"; description = "eg. userbouquet.dbe01.tv, bouquet.radio"; break;
		case TYPE::qname: label = "Bouquet Name"; description = "eg. User - bouquet (TV)"; break;
		case TYPE::nname: label = "Bouquet Nice Name"; description = "eg. TV, Radio"; break;
		case TYPE::btype: label = "Bouquet Type"; description = "TV, Radio"; break;
		case TYPE::hidden: label = "hidden"; break;
		case TYPE::dname: label = "dname"; break;
		case TYPE::itype: label = "itype"; break;
		case TYPE::flgs: label = "Flags"; break;
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

		if (! is.empty())
		{
			switch (type)
			{
				case TYPE::index:
				case TYPE::ytype:
				case TYPE::ssid:
				case TYPE::dvbns:
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
					return std::atoi(is.data());
				break;
				case TYPE::type:
				case TYPE::locked:
				case TYPE::marker:
					{
						int d = std::atoi(is.data());
						if (d < 0 || d > 1)
							continue;
						else
							return d;
					}
				break;
				case TYPE::yname:
					if (is.size() == 1)
						return dbih->value_transponder_type(is[0]);
					else
						continue;
				break;
				case TYPE::sys:
					return dbih->value_transponder_system(is);
				break;
				case TYPE::pos:
					return dbih->value_transponder_position(is);
				break;
				case TYPE::pol:
					return dbih->value_transponder_polarization(is);
				break;
				case TYPE::fec:
					return dbih->value_transponder_fec(is, e2db::YTYPE::satellite);
				break;
				case TYPE::hpfec:
				case TYPE::lpfec:
					return dbih->value_transponder_fec(is, e2db::YTYPE::terrestrial);
				break;
				case TYPE::cfec:
					return dbih->value_transponder_fec(is, e2db::YTYPE::atsc);
				break;
				case TYPE::mod:
					return dbih->value_transponder_modulation(is, e2db::YTYPE::satellite);
				break;
				case TYPE::tmod:
					return dbih->value_transponder_modulation(is, e2db::YTYPE::terrestrial);
				break;
				case TYPE::cmod:
					return dbih->value_transponder_modulation(is, e2db::YTYPE::cable);
				break;
				case TYPE::amod:
					return dbih->value_transponder_modulation(is, e2db::YTYPE::atsc);
				break;
				case TYPE::rol:
					return dbih->value_transponder_rollof(is);
				break;
				case TYPE::pil:
					return dbih->value_transponder_pilot(is);
				break;
				case TYPE::band:
					return dbih->value_transponder_bandwidth(is);
				break;
				case TYPE::tmx:
					return dbih->value_transponder_tmx_mode(is);
				break;
				case TYPE::guard:
					return dbih->value_transponder_guard(is);
				break;
				case TYPE::hier:
					return dbih->value_transponder_hier(is);
				break;
				case TYPE::btype:
					return dbih->value_bouquet_type(is);
				break;
				default:
					return is;
			}
		}
		else if (! required)
		{
			break;
		}
	}

	return -1;
}

}
