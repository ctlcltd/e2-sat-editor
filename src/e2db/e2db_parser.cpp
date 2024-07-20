/*!
 * e2-sat-editor/src/e2db/e2db_parser.cpp
 * 
 * @link https://github.com/ctlcltd/e2-sat-editor
 * @copyright e2 SAT Editor Team
 * @author Leonardo Laureti
 * @version 1.5.0
 * @license MIT License
 * @license GNU GPLv3 License
 */

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <clocale>
#include <algorithm>
#include <chrono>
#include <sstream>
#include <fstream>
#include <filesystem>
#include <stdexcept>

#include "e2db_parser.h"

using std::ifstream, std::stringstream, std::hex, std::dec, std::setfill, std::setw, std::to_string, std::sort;

namespace e2se_e2db
{

e2db_parser::e2db_parser()
{
	std::setlocale(LC_NUMERIC, "C");

	this->log = new e2se::logger("e2db", "e2db_parser");
}

void e2db_parser::parse_e2db()
{
	debug("parse_e2db");

	auto t_start = std::chrono::high_resolution_clock::now();

	if (! find_services_file())
		return error("parse_e2db", "Error", "Services file not found.");

	ifstream iservices (this->e2db[this->services_filename].path);
	try
	{
		if (this->e2db.count("services.xml"))
		{
			db.type = 1;
			parse_zapit_services_xml(iservices, "services.xml");
		}
		else
		{
			parse_e2db_lamedb(iservices);
		}
	}
	catch (...)
	{
		iservices.close();
		throw;
	}
	iservices.close();

	if (PARSER_TUNERSETS)
	{
		if (this->e2db.count("satellites.xml"))
		{
			ifstream itunxml (this->e2db["satellites.xml"].path);
			try
			{
				parse_tunersets_xml(YTYPE::satellite, itunxml);
			}
			catch (...)
			{
				itunxml.close();
				throw;
			}
			itunxml.close();
		}
		if (this->e2db.count("terrestrial.xml"))
		{
			ifstream itunxml (this->e2db["terrestrial.xml"].path);
			try
			{
				parse_tunersets_xml(YTYPE::terrestrial, itunxml);
			}
			catch (...)
			{
				itunxml.close();
				throw;
			}
			itunxml.close();

		}
		if (this->e2db.count("cables.xml"))
		{
			ifstream itunxml (this->e2db["cables.xml"].path);
			try
			{
				parse_tunersets_xml(YTYPE::cable, itunxml);
			}
			catch (...)
			{
				itunxml.close();
				throw;
			}
			itunxml.close();
		}
		if (this->e2db.count("atsc.xml"))
		{
			ifstream itunxml (this->e2db["atsc.xml"].path);
			try
			{
				parse_tunersets_xml(YTYPE::atsc, itunxml);
			}
			catch (...)
			{
				itunxml.close();
				throw;
			}
			itunxml.close();
		}
	}

	if (db.type == 1)
	{
		if (this->e2db.count("ubouquets.xml"))
		{
			ifstream ibouquetsxml (this->e2db["ubouquets.xml"].path);
			try
			{
				parse_zapit_bouquets_apix_xml(ibouquetsxml, "ubouquets.xml", ZAPIT_VER);
			}
			catch (...)
			{
				ibouquetsxml.close();
				throw;
			}
			ibouquetsxml.close();
		}
		if (this->e2db.count("bouquets.xml"))
		{
			ifstream ibouquetsxml (this->e2db["bouquets.xml"].path);
			try
			{
				parse_zapit_bouquets_apix_xml(ibouquetsxml, "bouquets.xml", ZAPIT_VER);
			}
			catch (...)
			{
				ibouquetsxml.close();
				throw;
			}
			ibouquetsxml.close();
		}
	}
	else
	{
		vector<pair<string, bool>> i_bouquets;

		for (auto & x : this->e2db)
		{
			if (x.first.find("bouquets.") != string::npos)
			{
				string fext = x.first.substr(x.first.rfind('.') + 1);

				if (fext != "tv" && fext != "radio" && fext != "epl")
					continue;

				i_bouquets.emplace_back(pair (x.first, fext == "epl"));
			}
		}

		// note: reverse sort tv, radio
		sort(i_bouquets.begin(), i_bouquets.end(), [](const auto a, const auto b) {
			return a.first > b.first;
		});

		for (auto & x : i_bouquets)
		{
			string fname = x.first;
			bool epl = x.second;

			ifstream ibouquet (this->e2db[fname].path);
			try
			{
				parse_e2db_bouquet(ibouquet, fname, epl);
			}
			catch (...)
			{
				ibouquet.close();
				throw;
			}
			ibouquet.close();

			bouquet& bs = bouquets[fname];

			for (string & fname : bs.userbouquets)
			{
				ifstream iuserbouquet (this->e2db[fname].path);
				try
				{
					parse_e2db_userbouquet(iuserbouquet, fname);
				}
				catch (...)
				{
					iuserbouquet.close();
					throw;
				}
				iuserbouquet.close();
			}
		}

		if (PARSER_PARENTALLOCK_LIST)
		{
			if (LAMEDB_VER < 4)
			{
				if (this->e2db.count("services.locked"))
				{
					ifstream ilocked (this->e2db["services.locked"].path);
					try
					{
						parse_e2db_parentallock_list(PARENTALLOCK::locked, ilocked);
					}
					catch (...)
					{
						ilocked.close();
						throw;
					}
					ilocked.close();
				}
			}
			else
			{
				if (this->e2db.count("blacklist"))
				{
					ifstream ilocked (this->e2db["blacklist"].path);
					try
					{
						parse_e2db_parentallock_list(PARENTALLOCK::blacklist, ilocked);
					}
					catch (...)
					{
						ilocked.close();
						throw;
					}
					ilocked.close();
				}
				if (this->e2db.count("whitelist"))
				{
					ifstream ilocked (this->e2db["whitelist"].path);
					try
					{
						parse_e2db_parentallock_list(PARENTALLOCK::whitelist, ilocked);
					}
					catch (...)
					{
						ilocked.close();
						throw;
					}
					ilocked.close();
				}
			}
		}
	}

	// ctlcltd/e2se-seeds/enigma_db
	// commit: 67b6442	elapsed time: 65520
	// commit: 6559e93	elapsed time: 19939
	// commit: a7022d8	elapsed time: 18506
	// commit: 6cbd126	elapsed time: 27465
	// commit: eea25b0	elapsed time: 20839
	// commit: 180b033	elapsed time: 20163
	// commit: 3e5808a	elapsed time: 19238
	// commit: HEAD     elapsed time: 19549

	auto t_end = std::chrono::high_resolution_clock::now();
	int elapsed = std::chrono::duration<double, std::micro>(t_end - t_start).count();

	info("parse_e2db", "elapsed time", to_string(elapsed) + " μs");
}

//TODO improve stringstream memory
void e2db_parser::parse_e2db(unordered_map<string, e2db_file> files)
{
	debug("parse_e2db");

	auto t_start = std::chrono::high_resolution_clock::now();

	for (auto & x : files)
	{
		string fpath = x.first;
		string filename = std::filesystem::path(fpath).filename().u8string();

		this->e2db[filename] = x.second;
	}
	if (! find_services_file())
		return error("parse_e2db", "Error", "Services file not found.");

	stringstream iservices;
	iservices.write(&files[this->e2db[this->services_filename].filename].data[0], files[this->e2db[this->services_filename].filename].size);
	if (this->e2db.count("services.xml"))
	{
		db.type = 1;
		parse_zapit_services_xml(iservices, "services.xml");
	}
	else
	{
		parse_e2db_lamedb(iservices);
	}

	if (PARSER_TUNERSETS)
	{
		if (this->e2db.count("satellites.xml"))
		{
			stringstream itunxml;
			itunxml.write(&files[this->e2db["satellites.xml"].filename].data[0], files[this->e2db["satellites.xml"].filename].size);
			parse_tunersets_xml(YTYPE::satellite, itunxml);
		}
		if (this->e2db.count("terrestrial.xml"))
		{
			stringstream itunxml;
			itunxml.write(&files[this->e2db["terrestrial.xml"].filename].data[0], files[this->e2db["terrestrial.xml"].filename].size);
			parse_tunersets_xml(YTYPE::terrestrial, itunxml);
		}
		if (this->e2db.count("cables.xml"))
		{
			stringstream itunxml;
			itunxml.write(&files[this->e2db["cables.xml"].filename].data[0], files[this->e2db["cables.xml"].filename].size);
			parse_tunersets_xml(YTYPE::cable, itunxml);
		}
		if (this->e2db.count("atsc.xml"))
		{
			stringstream itunxml;
			itunxml.write(&files[this->e2db["atsc.xml"].filename].data[0], files[this->e2db["atsc.xml"].filename].size);
			parse_tunersets_xml(YTYPE::atsc, itunxml);
		}
	}

	if (db.type == 1)
	{
		if (this->e2db.count("ubouquets.xml"))
		{
			stringstream ibouquetsxml;
			ibouquetsxml.write(&files[this->e2db["ubouquets.xml"].filename].data[0], files[this->e2db["ubouquets.xml"].filename].size);
			parse_zapit_bouquets_apix_xml(ibouquetsxml, "ubouquets.xml", ZAPIT_VER);
		}
		if (this->e2db.count("bouquets.xml"))
		{
			stringstream ibouquetsxml;
			ibouquetsxml.write(&files[this->e2db["bouquets.xml"].filename].data[0], files[this->e2db["bouquets.xml"].filename].size);
			parse_zapit_bouquets_apix_xml(ibouquetsxml, "bouquets.xml", ZAPIT_VER);
		}
	}
	else
	{
		vector<pair<string, bool>> i_bouquets;

		for (auto & x : this->e2db)
		{
			if (x.first.find("bouquets.") != string::npos)
			{
				string fext = x.first.substr(x.first.rfind('.') + 1);

				if (fext != "tv" && fext != "radio" && fext != "epl")
					continue;

				i_bouquets.emplace_back(pair (x.first, fext == "epl"));
			}
		}

		// note: reverse sort tv, radio
		sort(i_bouquets.begin(), i_bouquets.end(), [](const auto a, const auto b) {
			return a.first > b.first;
		});

		for (auto & x : i_bouquets)
		{
			string fname = x.first;
			bool epl = x.second;

			stringstream ibouquet;
			ibouquet.write(&files[this->e2db[fname].filename].data[0], files[this->e2db[fname].filename].size);
			parse_e2db_bouquet(ibouquet, fname, epl);

			bouquet& bs = bouquets[fname];

			for (string & fname : bs.userbouquets)
			{
				stringstream iuserbouquet;
				iuserbouquet.write(&files[this->e2db[fname].filename].data[0], files[this->e2db[fname].filename].size);
				parse_e2db_userbouquet(iuserbouquet, fname);
			}
		}

		if (PARSER_PARENTALLOCK_LIST)
		{
			if (LAMEDB_VER < 4)
			{
				if (this->e2db.count("services.locked"))
				{
					stringstream ilocked;
					ilocked.write(&files[this->e2db["services.locked"].filename].data[0], files[this->e2db["services.locked"].filename].size);
					parse_e2db_parentallock_list(PARENTALLOCK::locked, ilocked);
				}
			}
			else
			{
				if (this->e2db.count("blacklist"))
				{
					stringstream ilocked;
					ilocked.write(&files[this->e2db["blacklist"].filename].data[0], files[this->e2db["blacklist"].filename].size);
					parse_e2db_parentallock_list(PARENTALLOCK::blacklist, ilocked);
				}
				if (this->e2db.count("whitelist"))
				{
					stringstream ilocked;
					ilocked.write(&files[this->e2db["whitelist"].filename].data[0], files[this->e2db["whitelist"].filename].size);
					parse_e2db_parentallock_list(PARENTALLOCK::whitelist, ilocked);
				}
			}
		}
	}

	auto t_end = std::chrono::high_resolution_clock::now();
	int elapsed = std::chrono::duration<double, std::micro>(t_end - t_start).count();

	info("parse_e2db", "elapsed time", to_string(elapsed) + " μs");
}

void e2db_parser::parse_e2db_lamedb(istream& ilamedb)
{
	debug("parse_e2db_lamedb");

	bool ctx = false;

	int ver = -1;

	try
	{
		string hlamedb;
		std::getline(ilamedb, hlamedb);

		if (PARSER_FIX_CRLF && check_crlf(ctx, hlamedb)) fix_crlf(hlamedb);

		char vlamedb = (hlamedb.substr(hlamedb.length() - 2, hlamedb.length() - 1))[0];
		ver = isdigit(vlamedb) ? int (vlamedb) - 48 : 0;
	}
	catch (...)
	{
	}

	debug("parse_e2db_lamedb", "version", ver);

	switch (ver)
	{
		case 2:
		case 3:
		case 4:
			parse_e2db_lamedbx(ilamedb, ver);
		break;
		case 5:
			parse_e2db_lamedb5(ilamedb);
		break;
		default:
			return error("parse_e2db_lamedb", "Parser Error", "Unknown Lamedb services file format.");
	}
}

void e2db_parser::parse_e2db_lamedb5(istream& ilamedb)
{
	debug("parse_e2db_lamedb5", "version", 5);

	bool ctx = false;

	if (! db.dstat)
		db.version = 0x1225;

	ZAPIT_VER = -1;
	LAMEDB_VER = 5;

	bool step = 0;
	int tidx = 0;
	int sidx = 0;
	string line;
	int ln = 1;
	transponder tx;
	service ch;

	while (std::getline(ilamedb, line))
	{
		ln++;

		if (PARSER_FIX_CRLF && check_crlf(ctx, line)) fix_crlf(line);

		if (line.empty())
			continue;

		char type = line[0];

		if (type == 't')
			step = 1;
		else if (type == 's')
			step = 0;
		else
			continue;

		string str, params;
		size_t len = line.find(',');

		if (line.size() >= 13 && len != string::npos)
		{
			str = line.substr(2, len - 2);
			params = line.substr(len + 1);
		}
		else
		{
			continue;
		}

		// transponder
		if (step)
		{
			tx = transponder ();
			tidx += 1;

			parse_lamedb_transponder_params(str, tx);

			if (params.size() >= 2)
				parse_lamedb_transponder_feparams(params.substr(2), params[0], tx);

			if (tx.tsid != 0 || tx.onid != 0 || tx.dvbns != 0)
				add_transponder(tidx, tx);
			else
				error("parse_e2db_lamedb5", "Parser Error", msg("transponder (%s)", string ("lamedb5") + ':' + to_string(ln)));
		}
		// service
		else
		{
			ch = service ();
			sidx += 1;

			string chname, chdata;
			size_t len = params.rfind('"');

			if (params.size() >= 3 && len != string::npos)
			{
				chname = params.substr(1, len - 1);

				size_t pos = params.rfind(',');

				if (pos != string::npos)
				{
					chdata = params.substr(len + 2);
				}
			}
			else
			{
				continue;
			}

			parse_lamedb_service_params(str, ch);
			append_lamedb_service_name(chname, ch);
			parse_lamedb_service_data(chdata, ch);

			if (ch.ssid != 0 || ch.stype != 0)
				add_service(sidx, ch);
			else
				error("parse_e2db_lamedb5", "Parser Error", msg("service (%s)", string ("lamedb5") + ':' + to_string(ln)));
		}
	}
}

void e2db_parser::parse_e2db_lamedbx(istream& ilamedb, int ver)
{
	debug("parse_e2db_lamedbx", "version", ver);

	bool ctx = false;

	if (ver > 4)
		return error("parse_e2db_lamedbx", "Parser Error", "Unknown Lamedb services file format.");

	if (! db.dstat)
		db.version = 0x1220 + ver;

	ZAPIT_VER = -1;
	LAMEDB_VER = ver;

	int step = 0;
	int s = 0;
	int tidx = 0;
	int sidx = 0;
	string line;
	int ln = 1;
	transponder tx;
	service ch;

	while (std::getline(ilamedb, line))
	{
		ln++;

		if (PARSER_FIX_CRLF && check_crlf(ctx, line)) fix_crlf(line);

		if (! step && line == "transponders")
		{
			step = 1;
			s = 0;
			continue;
		}
		else if (! step && line == "services")
		{
			step = 2;
			s = 0;
			continue;
		}
		else if (step && line == "end")
		{
			step = 0;
			s = 0;
			continue;
		}

		// transponder
		if (step == 1)
		{
			if (s == 0)
			{
				tx = transponder ();

				parse_lamedb_transponder_params(line, tx);

				tidx += 1;
				s++;
			}
			else if (s == 1)
			{
				if (line.size() >= 3)
					parse_lamedb_transponder_feparams(line.substr(3), line.substr(1, 2)[0], tx);

				s++;
			}
			else if (s == 2)
			{
				if (tx.tsid != 0 || tx.onid != 0 || tx.dvbns != 0)
					add_transponder(tidx, tx);
				else
					error("parse_e2db_lamedbx", "Parser Error", msg("transponder (%s)", string ("lamedb") + ':' + to_string(ln)));

				s = 0;
			}
			if (line.size() == 1)
			{
				s = 0;
			}
		}
		// service
		else if (step == 2)
		{
			if (s == 0)
			{
				ch = service ();

				parse_lamedb_service_params(line, ch);

				sidx += 1;
				s++;
			}
			else if (s == 1)
			{
				append_lamedb_service_name(line, ch);

				s++;
			}
			else if (s == 2)
			{
				parse_lamedb_service_data(line, ch);

				if (ch.ssid != 0 || ch.stype != 0)
					add_service(sidx, ch);
				else
					error("parse_e2db_lamedbx", "Parser Error", msg("service (%s)", string ("lamedb") + ':' + to_string(ln)));

				s = 0;
			}
		}
	}
}

void e2db_parser::parse_lamedb_transponder_params(string str, transponder& tx)
{
	int dvbns, tsid, onid;
	dvbns = 0, tsid = 0, onid = 0;

	std::sscanf(str.c_str(), "%08x:%04x:%04x", &dvbns, &tsid, &onid);

	tx.dvbns = dvbns;
	tx.tsid = tsid;
	tx.onid = onid;
}

void e2db_parser::parse_lamedb_transponder_feparams(string str, char ty, transponder& tx)
{
	int freq, sr, pol, fec, pos, inv, flags, sys, mod, rol, pil;
	int band, hpfec, lpfec, tmod, tmx, guard, hier, plpid;
	int cmod, cfec;
	int amod;
	char feopts[37] = "";
	flags = -1, sys = -1, mod = -1, rol = -1, pil = -1, plpid = -1;

	switch (ty)
	{
		case 's': // DVB-S / DVB-S2
			std::sscanf(str.c_str(), "%8d:%8d:%1d:%1d:%5d:%1d:%1d:%1d:%1d:%1d:%1d%36s", &freq, &sr, &pol, &fec, &pos, &inv, &flags, &sys, &mod, &rol, &pil, feopts);

			tx.ytype = YTYPE::satellite;
			tx.freq = int (freq / 1e3);
			tx.sr = int (sr / 1e3);
			tx.pol = pol;
			tx.fec = fec;
			tx.pos = pos;
			tx.inv = inv;
			tx.flags = flags;
			tx.sys = sys;
			tx.mod = mod;
			tx.rol = rol;
			tx.pil = pil;

			if (LAMEDB_VER == 5)
			{
				if (std::strlen(feopts) != 0)
				{
					int plsn, isid, plscode, plsmode;
					plsn = -1, isid = -1, plscode = -1, plsmode = -1;

					std::sscanf(feopts, ",%d:%d:%d:%d", &plsn, &isid, &plscode, &plsmode);

					tx.plsn = plsn;
					tx.isid = isid;
					tx.plscode = plscode;
					tx.plsmode = plsmode;
				}
			}
		break;
		case 't': // DVB-T / DVB-T2
			std::sscanf(str.c_str(), "%9d:%1d:%1d:%1d:%1d:%1d:%1d:%1d:%1d:%1d:%1d:%1d", &freq, &band, &hpfec, &lpfec, &tmod, &tmx, &guard, &hier, &inv, &flags, &sys, &plpid);

			tx.ytype = YTYPE::terrestrial;
			tx.freq = int (freq / 1e3);
			tx.band = band;
			tx.hpfec = hpfec;
			tx.lpfec = lpfec;
			tx.tmod = tmod;
			tx.tmx = tmx;
			tx.guard = guard;
			tx.hier = hier;
			tx.inv = inv;
			tx.flags = flags;
			tx.sys = sys;
			tx.plpid = plpid;
		break;
		case 'c': // DVB-C
			std::sscanf(str.c_str(), "%8d:%8d:%1d:%1d:%1d:%1d:%1d", &freq, &sr, &inv, &cmod, &cfec, &flags, &sys);

			tx.ytype = YTYPE::cable;
			tx.freq = int (freq / 1e3);
			tx.sr = int (sr / 1e3);
			tx.inv = inv;
			tx.cmod = cmod;
			tx.cfec = cfec;
			tx.flags = flags;
			tx.sys = sys;
		break;
		case 'a': // ATSC / DVB-C ANNEX B
			std::sscanf(str.c_str(), "%8d:%1d:%1d:%1d:%1d", &freq, &inv, &amod, &flags, &sys);

			tx.ytype = YTYPE::atsc;
			tx.freq = int (freq / 1e3);
			tx.inv = inv;
			tx.amod = amod;
			tx.flags = flags;
			tx.sys = sys;
		break;
		default:
			return error("parse_lamedb_transponder_feparams", "Parser Error", "Unknown transponder type.");
	}
}

void e2db_parser::parse_lamedb_service_params(string str, service& ch)
{
	int ssid, dvbns, tsid, onid, stype, snum;
	ssid = 0, dvbns = 0, tsid = 0, onid = 0, stype = 0, snum = 0;

	if (LAMEDB_VER == 5)
	{
		int srcid = 0;
		std::sscanf(str.c_str(), "%04x:%08x:%04x:%04x:%3d:%4d:%d", &ssid, &dvbns, &tsid, &onid, &stype, &snum, &srcid);
		ch.srcid = srcid;
	}
	else
	{
		std::sscanf(str.c_str(), "%04x:%08x:%04x:%04x:%3d:%4d", &ssid, &dvbns, &tsid, &onid, &stype, &snum);
	}

	ch.ssid = ssid;
	ch.dvbns = dvbns;
	ch.tsid = tsid;
	ch.onid = onid;
	ch.stype = stype;
	ch.snum = snum;
}

void e2db_parser::parse_lamedb_service_data(string str, service& ch)
{
	if (str.empty())
		return;

	map<char, vector<string>> cstr;

	char* token = std::strtok(str.data(), ",");
	while (token != 0)
	{
		char key = token[0];
		switch (key)
		{
			case 'p': key = e2db_parser::SDATA::p; break;
			case 'c': key = e2db_parser::SDATA::c; break;
			case 'C': key = e2db_parser::SDATA::C; break;
			case 'f': key = e2db_parser::SDATA::f; break;
		}
		string val = string (token).substr(2);
		cstr[key].push_back(val);

		token = std::strtok(NULL, ",");
	}

	ch.data = cstr;
}

void e2db_parser::append_lamedb_service_name(string str, service& ch)
{
	ch.chname = str;
}

void e2db_parser::parse_e2db_bouquet(istream& ibouquet, string filename, bool epl)
{
	debug("parse_e2db_bouquet", "filename", filename);

	bool ctx = false;

	bool add = ! bouquets.count(filename);
	string line;
	int ln = 0;

	bouquet& bs = bouquets[filename];
	userbouquet ub;

	while (std::getline(ibouquet, line))
	{
		ln++;

		if (PARSER_FIX_CRLF && check_crlf(ctx, line)) fix_crlf(line);

		if (line.size() >= 9 && line.find("#SERVICE") != string::npos)
		{
			ub = userbouquet ();

			if (epl)
				parse_userbouquet_epl_reference(line.substr(9), ub);
			else
				parse_userbouquet_reference(line.substr(9), ub);

			ub.pname = filename;

			if (! ub.bname.empty())
				add_userbouquet(int (index["ubs"].size()), ub);
			else
				error("parse_e2db_bouquet", "Parser Error", msg("userbouquet (%s)", filename + ':' + to_string(ln)));
		}
		else if (line.size() >= 6 && line.find("#NAME") != string::npos)
		{
			if (add)
				bs = bouquet ();

			bs.bname = filename;
			bs.name = line.substr(6);
			if (filename.rfind(".tv") != string::npos)
			{
				bs.btype = 1;
				bs.nname = "TV";
			}
			else if (filename.rfind(".radio") != string::npos)
			{
				bs.btype = 2;
				bs.nname = "Radio";
			}
			else
			{
				continue;
			}

			if (add)
			{
				if (! bs.bname.empty())
					add_bouquet(bs.btype, bs);
				else
					error("parse_e2db_bouquet", "Parser Error", msg("bouquet (%s)", filename + ':' + to_string(ln)));
			}
		}
		ln++;
	}
}

void e2db_parser::parse_e2db_userbouquet(istream& iuserbouquet, string filename)
{
	debug("parse_e2db_userbouquet", "filename", filename);

	bool ctx = false;

	bool add = ! userbouquets.count(filename);
	bool step = 0;
	int idx = 0;
	int y = 0;
	string line;
	int ln = 0;

	if (add)
	{
		userbouquet ub;
		ub.bname = filename;

		if (filename.rfind(".tv") != string::npos)
			ub.pname = "bouquets.tv";
		else if (filename.rfind(".radio") != string::npos)
			ub.pname = "bouquets.radio";

		if (! ub.bname.empty())
			add_userbouquet(int (index["ubs"].size()), ub);
		else
			error("parse_e2db_userbouquet", "Parser Error", msg("userbouquet (%s)", filename + ':' + to_string(ln)));
	}

	userbouquet& ub = userbouquets[filename];
	channel_reference chref;
	service_reference ref;

	while (std::getline(iuserbouquet, line))
	{
		ln++;

		if (PARSER_FIX_CRLF && check_crlf(ctx, line)) fix_crlf(line);

		if (! step && line.size() >= 6 && line.find("#NAME") != string::npos)
		{
			ub.name = line.substr(6);
			step = 1;
			continue;
		}
		else if (step && line.size() >= 13 && line.find("#DESCRIPTION") != string::npos)
		{
			if (chref.etype != 0)
				set_channel_reference_description(ub, chref, line.substr(13));

			continue;
		}
		else if (step && line.size() >= 6 && line.find("#SORT") != string::npos)
		{
			continue;
		}
		else if (step && line.empty())
		{
			continue;
		}

		if (step && line.size() >= 9)
		{
			chref = channel_reference ();
			ref = service_reference ();

			parse_channel_reference(line.substr(9), chref, ref);

			if (chref.marker && chref.atype != ATYPE::marker_numbered)
			{
				idx = 0;
			}
			else
			{
				y++;
				idx = y;
			}

			if (chref.etype != 0)
				add_channel_reference(idx, ub, chref, ref);
			else
				error("parse_e2db_userbouquet", "Parser Error", msg("reference (%s)", filename + ':' + to_string(ln)));
		}
	}
}

//TODO improve parental by channel reference, by userbouquet and compatibility
void e2db_parser::parse_e2db_parentallock_list(PARENTALLOCK ltype, istream& ilocked)
{
	debug("parse_e2db_parentallock_list", "ltype", ltype);

	bool ctx = false;

	string line;

	channel_reference chref;
	service_reference ref;

	if (ltype == PARENTALLOCK::locked)
		std::getline(ilocked, line);

	int count = 0;

	while (std::getline(ilocked, line))
	{
		if (PARSER_FIX_CRLF && check_crlf(ctx, line)) fix_crlf(line);

		ref = service_reference ();

		parse_channel_reference(line, chref, ref);

		char chid[25];
		// %4x:%4x:%8x
		std::snprintf(chid, 25, "%x:%x:%x", ref.ssid, ref.tsid, ref.dvbns);

		if (ref.ssid == 0 && ref.tsid == 0 && ref.dvbns == 0)
		{
			size_t pos = line.find(" BOUQUET ");

			if (pos != string::npos)
			{
				string qs = line.substr(pos);
				size_t len = qs.size() > 10 && qs.size() <= 301 ? qs.size() - 10 : 1;

				char fname[(len + 1)];

				string format = " BOUQUET %" + to_string(len) + "s ";

				std::sscanf(qs.c_str(), format.c_str(), fname);

				if (std::strlen(fname) >= 5)
				{
					string bname = string (fname);
					bname = bname.substr(1, bname.size() - 2);

					if (! bname.empty() && index.count(bname))
						set_parentallock("", bname);
				}
			}
		}

		if (db.services.count(chid))
		{
			set_parentallock(chid);
			count++;
		}
		else if (ltype == PARENTALLOCK::locked)
		{
			userbouquet ub;

			parse_userbouquet_epl_reference(line, ub);

			if (! ub.bname.empty() && index.count(ub.bname))
				set_parentallock("", ub.bname);
		}
	}

	if (ltype == PARENTALLOCK::whitelist && count)
		db.parental = PARENTALLOCK::whitelist;
	else
		db.parental = PARENTALLOCK::blacklist;
}

//TODO userbouquet_reference and compatibility
void e2db_parser::parse_userbouquet_reference(string str, userbouquet& ub)
{
	size_t pos = str.find(" BOUQUET ");

	if (pos == string::npos)
	{
		error("parse_userbouquet_reference", "Parser Error", "Not supported yet.");

		return;
	}

	string qs = str.substr(pos);
	size_t len = qs.size() > 10 && qs.size() <= 301 ? qs.size() - 10 : 1;

	char fname[(len + 1)];
	char order[22];

	string format = " BOUQUET %" + to_string(len) + "s ORDER BY %21s";

	std::sscanf(qs.c_str(), format.c_str(), fname, order);

	if (std::strlen(fname) >= 5)
	{
		ub.bname = string (fname);
		ub.bname = ub.bname.substr(1, ub.bname.size() - 2);
	}
	ub.order = order;
	ub.sref = str.substr(0, pos - 4);

	int utype = 0;
	int btype = 0;

	std::sscanf(ub.sref.c_str(), "1:%d:%d", &utype, &btype);

	ub.utype = utype;
}

void e2db_parser::parse_userbouquet_epl_reference(string str, userbouquet& ub)
{
	string path;
	size_t pos = str.rfind("0:");

	if (pos != string::npos)
		path = str.substr(pos, str.length());

	string filename = std::filesystem::path(path).filename().u8string();

	ub.bname = filename;
}

void e2db_parser::parse_channel_reference(string str, channel_reference& chref, service_reference& ref)
{
	int etype, atype, anum, ssid, tsid, onid, dvbns, x7, x8, x9;
	char xdata;
	etype = 0, atype = 0, anum = 0, ssid = 0, tsid = 0, onid = 0, dvbns = 0, x7 = 0, x8 = 0, x9 = 0;

	std::sscanf(str.c_str(), "%d:%d:%X:%X:%X:%X:%X:%d:%d:%d:%c", &etype, &atype, &anum, &ssid, &tsid, &onid, &dvbns, &x7, &x8, &x9, &xdata);

	ref.ssid = ssid;
	ref.tsid = tsid;
	ref.onid = onid;
	ref.dvbns = dvbns;

	/*switch (etype)
	{
		// service or stream
		case ETYPE::ecast:
		// stream
		case ETYPE::evod:
		case ETYPE::eraw:
		case ETYPE::egstplayer:
		case ETYPE::exteplayer3:
		case ETYPE::eservice:
		case ETYPE::eyoutube:
		case ETYPE::eservice2:
		break;
		default:
			error("parse_channel_reference", "Parser Error", msg("Not supported yet (%s)", filename + ':' + to_string(ln)));
	}*/

	switch (atype)
	{
		// marker
		case ATYPE::marker_regular:
		case ATYPE::marker_numbered:
		case ATYPE::marker_hidden_512:
		case ATYPE::marker_hidden_832:
			chref.marker = true;
		break;
		// group
		case ATYPE::group:
			error("parse_channel_reference", "Parser Error", "Not supported yet.");
		break;
		// service
		default:
			chref.marker = false;
	}

	if (std::strlen(&xdata))
	{
		size_t pos = str.rfind(':');

		if (pos != string::npos)
		{
			string s10, s11;

			s11 = str.substr(pos + 1);
			pos = str.rfind(':', pos - 1);

			if (pos != string::npos)
			{
				s10 = str.substr(pos + 1);
				pos = s10.rfind(':');

				if (pos != string::npos)
					s10 = s10.substr(0, pos);
				if (s10.size() == 1)
					s10 = "";
			}

			if (! s11.empty() && ! s10.empty())
			{
				conv_uri_value(s10);
				conv_uri_value(s11);

				chref.uri = s10;
				chref.value = s11;
				chref.inlineval = true;
			}
			else if (! s11.empty())
			{
				conv_uri_value(s11);

				if (chref.marker)
				{
					chref.value = s11;
					chref.inlineval = true;
				}
				else
				{
					chref.uri = s11;
				}
			}

			chref.stream = ! chref.uri.empty();
		}
	}

	chref.etype = etype;
	chref.atype = atype;
	chref.anum = anum;
}

void e2db_parser::parse_tunersets_xml(int ytype, istream& itunxml)
{
	debug("parse_tunersets_xml", "ytype", ytype);

	string charset = "UTF-8";
	bool valid = parse_xml_head(itunxml, charset);

	if (! valid)
		return error("parse_tunersets_xml", "Parser Error", "Unknown file format.");

	debug("parse_tunersets_xml", "charset", charset);

	switch (ytype)
	{
		case YTYPE::satellite:
		case YTYPE::terrestrial:
		case YTYPE::cable:
		case YTYPE::atsc:
		break;
		default:
			return error("parse_tunersets_xml", "Parser Error", "These settings are not supported.");
	}

	tunersets tv;
	tunersets_table tn;
	tunersets_transponder tntxp;

	tv.ytype = ytype;
	tv.charset = charset;

	string iname = "tns:";
	char yname;
	unordered_map<string, int> depth;
	switch (ytype)
	{
		case YTYPE::satellite:
			yname = 's';
			depth["satellites"] = 0;
			depth["sat"] = 1;
		break;
		case YTYPE::terrestrial:
			yname = 't';
			depth["locations"] = 0;
			depth["terrestrial"] = 1;
		break;
		case YTYPE::cable:
			yname = 'c';
			depth["cables"] = 0;
			depth["cable"] = 1;
		break;
		case YTYPE::atsc:
			yname = 'a';
			depth["locations"] = 0;
			depth["atsc"] = 1;
		break;
	}
	depth["transponder"] = 2;
	iname += yname;

	int step = 0;
	int ln = 1;
	int bidx = 0;
	int cidx = 0;
	string line;

	while (std::getline(itunxml, line, '>'))
	{
		if (line.find("<!") != string::npos)
		{
			comment s;
			parse_xml_comment(line, s, ln);
			comments[iname].emplace_back(s);
			continue;
		}

		string tag;
		bool add = false;
		parse_xml_tag(line, tag, add);

		if (tag.empty())
		{
			continue;
		}
		else if (depth.count(tag))
		{
			ln++;
			step = depth[tag];

			if (step == 1)
			{
				tn = tunersets_table ();
				tn.ytype = ytype;
			}
			else if (step == 2)
			{
				tntxp = tunersets_transponder ();
				tntxp.freq = -1;
			}
		}
		else
		{
			return error("parse_tunersets_xml", "Parser Error", "Malformed or unknown XML file format.");
		}

		char* token = std::strtok(line.data(), " ");
		while (token != 0)
		{
			string key, val;
			parse_xml_attribute(line, token, key, val);
			token = std::strtok(NULL, " ");

			if (key.empty() || val.empty())
				continue;

			switch (ytype)
			{
				case YTYPE::satellite:
					if (key == "name")
					{
						conv_xml_value(val);
						tn.name = val;
					}
					else if (key == "flags")
						tn.flags = std::atoi(val.data());
					else if (key == "position")
						tn.pos = std::atoi(val.data());
					else if (key == "frequency")
						tntxp.freq = int (std::atoi(val.data()) / 1e3);
					else if (key == "symbol_rate")
						tntxp.sr = int (std::atoi(val.data()) / 1e3);
					else if (key == "polarization")
						tntxp.pol = std::atoi(val.data());
					else if (key == "fec_inner")
						tntxp.fec = std::atoi(val.data());
					else if (key == "inversion")
						tntxp.inv = std::atoi(val.data());
					else if (key == "system")
						tntxp.sys = std::atoi(val.data());
					else if (key == "modulation")
						tntxp.mod = std::atoi(val.data());
					else if (key == "rolloff")
						tntxp.rol = std::atoi(val.data());
					else if (key == "pilot")
						tntxp.pil = std::atoi(val.data());
					else if (key == "is_id")
						tntxp.isid = std::atoi(val.data());
					else if (key == "mts")
						tntxp.mts = std::atoi(val.data());
					else if (key == "pls_mode")
						tntxp.plsmode = std::atoi(val.data());
					else if (key == "pls_code")
						tntxp.plscode = std::atoi(val.data());
					else if (key == "plsn")
						tntxp.plsn = std::atoi(val.data());
					else if (key == "t2mi_plp_id")
						tntxp.t2mi_plpid = std::atoi(val.data());
					else if (key == "t2mi_pid")
						tntxp.t2mi_pid = std::atoi(val.data());
				break;
				case YTYPE::terrestrial:
					if (key == "name")
					{
						conv_xml_value(val);
						tn.name = val;
					}
					else if (key == "flags")
						tn.flags = std::atoi(val.data());
					else if (key == "countrycode")
						tn.country = val;
					else if (key == "centre_frequency")
						tntxp.freq = int (std::atoi(val.data()) / 1e3);
					else if (key == "bandwidth")
						tntxp.band = std::atoi(val.data());
					else if (key == "code_rate_hp")
						tntxp.hpfec = std::atoi(val.data());
					else if (key == "code_rate_lp")
						tntxp.lpfec = std::atoi(val.data());
					else if (key == "inversion")
						tntxp.inv = std::atoi(val.data());
					else if (key == "system")
						tntxp.sys = std::atoi(val.data());
					else if (key == "constellation")
						tntxp.tmod = std::atoi(val.data());
					else if (key == "transmission_mode")
						tntxp.tmx = std::atoi(val.data());
					else if (key == "guard_interval")
						tntxp.guard = std::atoi(val.data());
					else if (key == "hierarchy_information")
						tntxp.hier = std::atoi(val.data());
					else if (key == "plp_id")
						tntxp.plpid = std::atoi(val.data());
				break;
				case YTYPE::cable:
					if (key == "name")
					{
						conv_xml_value(val);
						tn.name = val;
					}
					else if (key == "flags")
						tn.flags = std::atoi(val.data());
					else if (key == "countrycode")
						tn.country = val;
					else if (key == "satfeed")
						tn.feed = (val == "true" ? 1 : 0);
					else if (key == "frequency")
						tntxp.freq = int (std::atoi(val.data()) / 1e3);
					else if (key == "symbol_rate")
						tntxp.sr = int (std::atoi(val.data()) / 1e3);
					else if (key == "inversion")
						tntxp.inv = std::atoi(val.data());
					else if (key == "modulation")
						tntxp.cmod = std::atoi(val.data());
					else if (key == "fec_inner")
						tntxp.cfec = std::atoi(val.data());
				break;
				case YTYPE::atsc:
					if (key == "name")
					{
						conv_xml_value(val);
						tn.name = val;
					}
					else if (key == "flags")
						tn.flags = std::atoi(val.data());
					else if (key == "frequency")
						tntxp.freq = int (std::atoi(val.data()) / 1e3);
					else if (key == "inversion")
						tntxp.inv = std::atoi(val.data());
					else if (key == "modulation")
						tntxp.amod = std::atoi(val.data());
					else if (key == "system")
						tntxp.sys = std::atoi(val.data());
				break;
			}
		}

		if (! add && step == 1)
		{
			bidx++;

			add_tunersets_table(bidx, tn, tv);
		}
		else if (add && step == 2)
		{
			char tnid[25];
			std::snprintf(tnid, 25, "%c:%04x", yname, bidx);

			tunersets_table& tn = tv.tables[tnid];
			cidx++;
			add_tunersets_transponder(cidx, tntxp, tn);
		}
	}

	add_tunersets(tv);
}

void e2db_parser::parse_zapit_services_xml(istream& iservicesxml, string filename)
{
	debug("parse_zapit_services_xml", "filename", filename);

	string head;
	std::getline(iservicesxml, head, '>');

	if (head.find("<?xml") == string::npos)
		return error("parse_zapit_services_xml", "Parser Error", "Unknown file format.");

	bool valid = false;
	int ver = -1;
	string line;

	while (std::getline(iservicesxml, line, '>'))
	{
		if (line.find("<zapit") != string::npos)
		{
			string pver;
			size_t pos = line.find("api", line.find_first_of(' '));
			size_t n;

			if (pos != string::npos)
			{
				n = line.find('"', pos);
				if (n == string::npos)
					n = line.rfind('\'', pos);
				pver = line.substr(n + 1);
				n = pver.rfind('"');
				if (n == string::npos)
					n = pver.rfind('\'');
				pver = pver.substr(0, n);
				ver = std::atoi(pver.data());
			}
			valid = true;

			break;
		}
	}

	iservicesxml.seekg(0);

	if (valid)
		ver = ver > 1 ? ver : 1;
	else
		return error("parse_zapit_services_xml", "Parser Error", "These settings are not supported.");

	if (ver < 1 || ver > 4)
		return error("parse_zapit_services_xml", "Parser Error", "Unknown Zapit services file format.");

	parse_zapit_services_apix_xml(iservicesxml, filename, ver);
}

void e2db_parser::parse_zapit_services_apix_xml(istream& iservicesxml, string filename, int ver)
{
	debug("parse_zapit_services_apix_xml", "filename", filename);
	debug("parse_zapit_services_apix_xml", "version", ver);

	if (ver < 1 || ver > 4)
		return error("parse_zapit_services_apix_xml", "Parser Error", "Unknown Zapit services file format.");

	if (! db.dstat)
		db.version = 0x1010 + ver;

	LAMEDB_VER = -1;
	ZAPIT_VER = ver;

	string charset = "UTF-8";
	bool valid = parse_xml_head(iservicesxml, charset);

	if (! valid)
		return error("parse_zapit_services_apix_xml", "Parser Error", "Unknown file format.");

	debug("parse_zapit_services_apix_xml", "charset", charset);

	string dname = "services.xml";

	datasets dat;
	dat.dname = dname;
	dat.itype = 0;
	dat.charset = charset;

	table tr;
	transponder tx;
	service ch;

	string iname = dname;
	unordered_map<string, int> depth;
	depth["zapit"] = 0;
	depth["sat"] = 1;
	if (ver > 1)
	{
		depth["TS"] = 2;
		depth["S"] = 3;
	}
	else
	{
		depth["transponder"] = 2;
		depth["channel"] = 3;
	}

	int step = 0;
	int ln = 1;
	int aidx = 0;
	int bidx = 0;
	int cidx = 0;
	string line;

	while (std::getline(iservicesxml, line, '>'))
	{
		if (line.find("<!") != string::npos)
		{
			comment s;
			parse_xml_comment(line, s, ln);
			comments[iname].emplace_back(s);
			continue;
		}

		string tag;
		bool add = false;
		parse_xml_tag(line, tag, add);

		if (tag.empty())
		{
			continue;
		}
		else if (depth.count(tag))
		{
			ln++;
			step = depth[tag];

			if (! add && step == 1)
			{
				tr = table ();
			}
			else if (step == 2)
			{
				tx = transponder ();
			}
			else if (step == 3)
			{
				ch = service ();
			}
		}
		else
		{
			return error("parse_zapit_services_apix_xml", "Parser Error", "Malformed or unknown XML file format.");
		}

		char* token = std::strtok(line.data(), " ");
		while (token != 0)
		{
			string key, val;
			parse_xml_attribute(line, token, key, val);
			token = std::strtok(NULL, " ");

			if (key.empty() || val.empty())
				continue;

			if (ver > 1)
			{
				if (key == "name")
				{
					conv_xml_value(val);
					tr.name = val;
				}
				else if (key == "position")
					tr.pos = std::atoi(val.data());
				else if (key == "diseqc")
					tr.diseqc = std::atoi(val.data());
				else if (key == "uncommitted")
					tr.uncomtd = std::atoi(val.data());
				else if (key == "id")
					tx.tsid = int (std::strtol(val.data(), NULL, 16));
				else if (key == "on")
					tx.onid = int (std::strtol(val.data(), NULL, 16));
				else if (key == "frq")
					tx.freq = int (std::atoi(val.data()) / 1e3);
				else if (key == "inv")
				{
					int i = std::atoi(val.data());
					tx.inv = (ver < 3 && i != 2 ? i : 0);
				}
				else if (key == "sr")
					tx.sr = int (std::atoi(val.data()) / 1e3);
				else if (key == "fec")
				{
					int i = std::atoi(val.data());
					if (ver == 4)
					{
						if (i != 0 && i < 4)
							tx.fec = i;
						else if (i == 4)
							tx.fec = 8;
						else if (i == 5)
							tx.fec = 4;
						else if (i == 6)
							tx.fec = 10;
						else if (i == 7)
							tx.fec = 5;
						else if (i == 8)
							tx.fec = 6;
						else if (i == 10)
							tx.fec = 7;
						else if (i == 11)
							tx.fec = 9;
					}
					else if (ver == 3)
					{
						if (i < 4)
							tx.fec = i;
						else if (i == 5)
							tx.fec = 4;
						else if (i == 7)
							tx.fec = 5;
					}
					else if (ver == 2)
					{
						if (i < 6)
							tx.fec = i;
					}
				}
				else if (key == "pol")
					tx.pol = std::atoi(val.data());
				else if (key == "mod")
					tx.mod = std::atoi(val.data());
				else if (key == "sys")
					tx.sys = std::atoi(val.data());
				else if (key == "i")
					ch.ssid = int (std::strtol(val.data(), NULL, 16));
				else if (key == "n")
				{
					conv_xml_value(val);
					ch.chname = val;
				}
				else if (key == "v")
				{
					int cval = int (std::strtol(val.data(), NULL, 16));
					if (cval != 0)
					{
						char pid[7];
						std::snprintf(pid, 7, "%02d%04x", SDATA_PIDS::vpid, cval);
						ch.data[SDATA::c].emplace_back(pid);
					}
				}
				else if (key == "a")
				{
					int cval = int (std::strtol(val.data(), NULL, 16));
					if (cval != 0)
					{
						char pid[7];
						std::snprintf(pid, 7, "%02d%04x", SDATA_PIDS::mpegapid, cval);
						ch.data[SDATA::c].emplace_back(pid);
					}
				}
				else if (key == "p")
				{
					int cval = int (std::strtol(val.data(), NULL, 16));
					if (cval != 0)
					{
						char pid[7];
						std::snprintf(pid, 7, "%02d%04x", SDATA_PIDS::pcrpid, cval);
						ch.data[SDATA::c].emplace_back(pid);
					}
				}
				else if (key == "pmt")
				{
					int cval = int (std::strtol(val.data(), NULL, 16));
					if (cval != 0)
					{
						char pid[7];
						std::snprintf(pid, 7, "%02d%04x", SDATA_PIDS::pmt, cval);
						ch.data[SDATA::c].emplace_back(pid);
					}
				}
				else if (key == "tx")
				{
					int cval = int (std::strtol(val.data(), NULL, 16));
					if (cval != 0)
					{
						char pid[7];
						std::snprintf(pid, 7, "%02d%04x", SDATA_PIDS::tpid, cval);
						ch.data[SDATA::c].emplace_back(pid);
					}
				}
				else if (key == "t")
					ch.stype = int (std::strtol(val.data(), NULL, 16));
				else if (key == "vt")
				{
					int cval = int (std::strtol(val.data(), NULL, 16));
					if (cval != 0)
					{
						char pid[7];
						std::snprintf(pid, 7, "%02d%04x", SDATA_PIDS::vtype, cval);
						ch.data[SDATA::c].emplace_back(pid);
					}
				}
				else if (key == "s")
				{
					int cval = std::atoi(val.data());
					if (cval != 0)
					{
						ch.data[SDATA::C];
					}
				}
				else if (key == "num")
					ch.snum = std::atoi(val.data());
				//TODO zapit flags
				// else if (key == "f")
			}
			else
			{
				if (step == 0 && key == "name")
				{
					conv_xml_value(val);
					tr.name = val;
				}
				else if (key == "diseqc")
					tr.diseqc = std::atoi(val.data());
				else if (key == "position")
					tr.pos = std::atoi(val.data());
				else if (key == "id")
					tx.tsid = int (std::strtol(val.data(), NULL, 16));
				else if (key == "onid")
					tx.onid = int (std::strtol(val.data(), NULL, 16));
				else if (key == "frequency")
					tx.freq = int (std::atoi(val.data()) / 1e3);
				else if (key == "inversion")
				{
					int i = std::atoi(val.data());
					tx.inv = (i != 2 ? i : 0);
				}
				else if (key == "symbol_rate")
					tx.sr = int (std::atoi(val.data()) / 1e3);
				else if (key == "fec_inner")
				{
					int i = std::atoi(val.data());
					if (i < 4)
						tx.fec = i;
					else if (i == 4)
						tx.fec = 8;
					else if (i == 5)
						tx.fec = 4;
					else if (i == 6)
						tx.fec = 10;
					else if (i == 7)
						tx.fec = 5;
					else if (i == 8)
						tx.fec = 6;
				}
				else if (key == "polarization")
					tx.pol = std::atoi(val.data());
				else if (key == "service_id")
					ch.ssid = int (std::strtol(val.data(), NULL, 16));
				else if (key == "name")
				{
					conv_xml_value(val);
					ch.chname = val;
				}
				else if (key == "service_type")
					ch.stype = int (std::strtol(val.data(), NULL, 16));
			}
		}

		if (add && step == 1)
		{
			aidx++;
			tr.index = aidx;
			db.tables.emplace(tr.pos, tr);
			index["trs"].emplace_back(pair (tr.index, to_string(tr.pos)));
		}
		else if (! add && step == 2)
		{
			bidx++;
			tx.pos = tr.pos;
			tx.dvbns = value_transponder_dvbns(tx);
			add_transponder(bidx, tx);

			tr.transponders.emplace_back(tx.txid);
		}
		else if (add && step == 3)
		{
			cidx++;
			ch.tsid = tx.tsid;
			ch.onid = tx.onid;
			ch.dvbns = tx.dvbns;
			add_service(cidx, ch);
		}
	}

	datas.emplace(dat.dname, dat);
}

void e2db_parser::parse_zapit_bouquets_apix_xml(istream& ibouquetsxml, string filename, int ver)
{
	debug("parse_zapit_bouquets_apix_xml", "filename", filename);
	debug("parse_zapit_bouquets_apix_xml", "version", ver);

	if (ver < 1 || ver > 4)
		return error("parse_zapit_bouquets_apix_xml", "Parser Error", "Unknown Zapit bouquets file format.");

	ZAPIT_VER = ver;

	string charset = "UTF-8";
	bool valid = parse_xml_head(ibouquetsxml, charset);

	if (! valid)
		return error("parse_zapit_bouquets_apix_xml", "Parser Error", "Unknown file format.");

	debug("parse_zapit_bouquets_apix_xml", "charset", charset);

	string dname = ver > 1 ? "ubouquets.xml" : "bouquets.xml";
	string ub_fname_suffix = USERBOUQUET_FILENAME_SUFFIX;

	datasets dat;
	dat.dname = dname;
	dat.itype = 1;
	dat.charset = charset;

	table tr;
	userbouquet ub;
	service_reference ref;
	channel_reference chref;
	int pos = -1;
	int freq = -1;
	bool locked = false;

	vector<pair<userbouquet, vector<string>>> ubouquets;
	vector<string> chindex;

	string iname = dname;
	unordered_map<string, int> depth;
	depth["zapit"] = 0;
	depth["Bouquet"] = 1;
	if (ver > 1)
	{
		depth["S"] = 2;
	}
	else
	{
		depth["channel"] = 2;
	}

	int step = 0;
	int ln = 1;
	int bidx = 0;
	int cidx = 0;
	string line;

	while (std::getline(ibouquetsxml, line, '>'))
	{
		if (line.find("<!") != string::npos)
		{
			comment s;
			parse_xml_comment(line, s, ln);
			comments[iname].emplace_back(s);
			continue;
		}

		string tag;
		bool add = false;
		parse_xml_tag(line, tag, add);

		if (tag.empty())
		{
			continue;
		}
		else if (depth.count(tag))
		{
			ln++;
			step = depth[tag];

			if (! add && step == 1)
			{
				ub = userbouquet ();
				chindex.clear();
				cidx = 0;
			}
			else if (step == 2)
			{
				ref = service_reference ();
				chref = channel_reference ();
				pos = -1;
				freq = -1;
				locked = false;
			}
		}
		else
		{
			return error("parse_zapit_bouquets_apix_xml", "Parser Error", "Malformed or unknown XML file format.");
		}

		char* token = std::strtok(line.data(), " ");
		while (token != 0)
		{
			string key, val;
			parse_xml_attribute(line, token, key, val);
			token = std::strtok(NULL, " ");

			if (key.empty() || val.empty())
				continue;

			if (ver > 1)
			{
				if (key == "name")
				{
					conv_xml_value(val);
					ub.name = val;
				}
				else if (key == "hidden")
					ub.hidden = std::atoi(val.data());
				else if (key == "locked")
					ub.locked = std::atoi(val.data());
				else if (key == "i")
					ref.ssid = int (std::strtol(val.data(), NULL, 16));
				// else if (key == "n")
				// {
				// 	conv_xml_value(val);
				// 	chref.value = val;
				// }
				else if (key == "t")
					ref.tsid = int (std::strtol(val.data(), NULL, 16));
				else if (key == "on")
					ref.onid = int (std::strtol(val.data(), NULL, 16));
				else if (key == "s")
					pos = std::atoi(val.data());
				else if (key == "frq")
					freq = std::atoi(val.data());
				else if (key == "l")
					locked = std::atoi(val.data());
			}
			else
			{
				if (step == 1 && key == "name")
				{
					conv_xml_value(val);
					ub.name = val;
				}
				else if (key == "hidden")
					ub.hidden = std::atoi(val.data());
				else if (key == "locked")
					ub.locked = std::atoi(val.data());
				else if (key == "serviceID")
					ref.ssid = int (std::strtol(val.data(), NULL, 16));
				// else if (key == "name")
				// {
				// 	conv_xml_value(val);
				// 	chref.value = val;
				// }
				else if (key == "tsid")
					ref.tsid = int (std::strtol(val.data(), NULL, 16));
				else if (key == "onid")
					ref.onid = int (std::strtol(val.data(), NULL, 16));
				else if (key == "sat_position")
					pos = std::atoi(val.data());
			}
		}

		if (add && step == 1)
		{
			bidx++;
			ub.index = bidx;
			ubouquets.emplace_back(pair (ub, chindex));
		}
		else if (add && step == 2)
		{
			if (pos != -1 && pos != 0 && ref.onid != 0 && freq != 0)
			{
				ref.dvbns = value_transponder_dvbns(YTYPE::satellite, ref.tsid, ref.onid, pos, freq);
			}
			else
			{
				for (auto & x : db.transponders)
				{
					transponder tx = x.second;

					if (pos == tx.pos && ref.onid == tx.onid && ref.tsid == tx.tsid)
					{
						ref.dvbns = value_transponder_dvbns(tx);
						break;
					}
				}
			}

			char chid[25];
			std::snprintf(chid, 25, "%x:%x:%x", ref.ssid, ref.tsid, ref.dvbns);

			cidx++;
			chref.chid = chid;
			chref.index = cidx;

			ub.channels.emplace(chref.chid, chref);
			chindex.emplace_back(chref.chid);

			if (locked && db.services.count(chref.chid))
			{
				service& ch = db.services[chref.chid];
				ch.locked = true;
				db.services[chref.chid] = ch;
			}
		}
	}

	unordered_map<int, string> count;

	for (auto & q : ubouquets)
	{
		bouquet bs;
		userbouquet ub = q.first;

		for (auto & x : ub.channels)
		{
			channel_reference chref = x.second;

			if (! chref.marker && db.services.count(chref.chid))
			{
				service ch = db.services[chref.chid];
				count.emplace(STYPE_EXT_TYPE.count(ch.stype) ? STYPE_EXT_TYPE.at(ch.stype) : 0, ch.chid);
			}
		}

		// btype autodetect
		if (count[STYPE::radio] > count[STYPE::tv])
			bs = bouquets["bouquets.radio"];
		else
			bs = bouquets["bouquets.tv"];

		string ktype;
		if (bs.btype == STYPE::tv)
			ktype = "tv";
		else if (bs.btype == STYPE::radio)
			ktype = "radio";

		stringstream ub_bname;
		ub_bname << "userbouquet." << ub_fname_suffix << setfill('0') << setw(2) << ub.index << '.' << ktype;

		ub.bname = ub_bname.str();
		ub.pname = bs.bname;

		add_userbouquet(ub.index, ub);

		int idx = int (index[ub.pname].size());
		for (auto & w : q.second)
		{
			channel_reference chref = ub.channels[w];

			if (! chref.marker && db.services.count(chref.chid))
			{
				index[ub.bname].emplace_back(pair (chref.index, chref.chid));

				if (bs.services.count(chref.chid) == 0)
				{
					idx += 1;
					bs.services.emplace(chref.chid);
					index[ub.pname].emplace_back(pair (idx, chref.chid));
				}
			}
		}

		count.clear();
	}

	datas.emplace(dat.dname, dat);
}

bool e2db_parser::parse_xml_head(istream& ixml, string& charset)
{
	string head;
	std::getline(ixml, head, '>');

	if (head.find("<?xml") == string::npos)
		return false;

	size_t pos = head.find("encoding");
	size_t n;

	if (pos != string::npos)
	{
		n = head.find('"', pos);
		if (n == string::npos)
			n = head.rfind('\'', pos);
		charset = head.substr(n + 1);
		n = charset.rfind('"');
		if (n == string::npos)
			n = charset.rfind('\'');
		charset = charset.substr(0, n);
	}
	return true;
}

void e2db_parser::parse_xml_comment(string line, comment& s, int ln)
{
	s.type = line.find('\n') != string::npos;
	s.ln = ln;
	s.text = line.substr(line.find("<!--") + 4);
	s.text = s.text.substr(0, s.text.length() - 2);
}

void e2db_parser::parse_xml_tag(string line, string& tag, bool& closed)
{
	size_t pos = line.find('<');
	size_t n;

	if (pos != string::npos)
	{
		tag = line.substr(pos + 1);
		pos = tag[0] == '\n';
		if (pos)
		{
			tag = "";
			return;
		}
		pos = tag[0] == '/';
		n = tag.size() - 1;

		closed = pos || tag[n] == '/';

		n = tag.find(' ');
		if (! pos && n == string::npos)
			n = tag.rfind('/');
		tag = tag.substr(pos, n);
	}
}

void e2db_parser::parse_xml_attribute(string line, string token, string& key, string& val)
{
	size_t pos;
	size_t n = token.find('=');

	if (n != string::npos)
	{
		key = token.substr(0, n);
		val = token.substr(n + 1);
	}
	else
	{
		key = token;
	}

	pos = val.find('"');
	n = string::npos;

	if (pos != string::npos)
	{
		n = val.rfind('"');
	}
	else
	{
		pos = val.find('\'');
		if (pos != string::npos)
			n = val.rfind('\'');
	}

	if (n != string::npos && pos != n)
	{
		val = val.substr(0, n);
		if (pos != string::npos)
			val = val.substr(pos + 1);
	}
	else
	{
		val = line.substr(line.find(key) + key.length());
		pos = val.find('"');
		if (pos == string::npos)
			pos = val.find('\'');
		if (pos != string::npos)
			val = val.substr(pos + 1);

		n = val.find('"');
		if (n == string::npos)
			n = val.find('\'');
		if (n != string::npos)
			val = val.substr(0, n);

		std::transform(val.begin(), val.end(), val.begin(), [](unsigned char c) { return c ? c : ' '; });
	}
}

void e2db_parser::conv_uri_value(string& val)
{
	if (val.find("%3a") != string::npos)
	{
		size_t pos = val.find("%3a");

		while (pos != string::npos)
		{
			val = val.substr(0, pos) + ':' + val.substr(pos + 3);

			if (pos != val.size())
				pos = val.find("%3a");
			else
				pos = string::npos;
		}
	}
}

void e2db_parser::conv_xml_value(string& val)
{
	unordered_map<string, char> xmlents = {
		{"&amp;", '&'},
		{"&quot;", '"'},
		{"&apos;", '\''},
		{"&lt;", '<'},
		{"&gt;", '>'}
	};

	size_t pos = val.find('&');
	size_t n;

	while (pos != string::npos)
	{
		n = val.find(';', pos);
		if (n != string::npos)
		{
			string w = val.substr(pos, n - pos + 1);
			if (xmlents.count(w))
				val = val.substr(0, pos) + xmlents.at(w) + val.substr(n + 1);
		}
		if (pos != val.size())
			pos = val.find('&', pos + 1);
		else
			pos = string::npos;
	}
}

bool e2db_parser::find_services_file()
{
	debug("find_services_file");

	if (PARSER_PRIOR_LAMEDB5 && this->e2db.count("lamedb5"))
		this->services_filename = "lamedb5";
	else if (this->e2db.count("lamedb"))
		this->services_filename = "lamedb";
	else if (this->e2db.count("lamedb5"))
		this->services_filename = "lamedb5";
	else if (this->e2db.count("services"))
		this->services_filename = "services";
	else if (this->e2db.count("services.xml"))
		this->services_filename = "services.xml";

	return ! this->services_filename.empty();
}

bool e2db_parser::list_file(string path)
{
	debug("list_file", "path", path);

	if (! std::filesystem::exists(path))
	{
		error("list_file", "File Error", msg("File \"%s\" not exists.", path));
		return false;
	}
	if
	(
		(std::filesystem::status(path).permissions() & std::filesystem::perms::owner_read) == std::filesystem::perms::none &&
		(std::filesystem::status(path).permissions() & std::filesystem::perms::group_read) == std::filesystem::perms::none
	)
	{
		error("list_file", "File Error", msg("File \"%s\" is not readable.", path));
		return false;
	}

	std::filesystem::directory_iterator filelist (path);

	for (const auto & entry : filelist)
	{
		if (! std::filesystem::is_regular_file(entry))
		{
			continue;
		}
		if
		(
			(std::filesystem::status(entry).permissions() & std::filesystem::perms::owner_read) == std::filesystem::perms::none &&
			(std::filesystem::status(entry).permissions() & std::filesystem::perms::group_read) == std::filesystem::perms::none
		)
		{
			error("list_file", "File Error", msg("File \"%s\" is not readable.", path));
			return false;
		}

		string fpath = entry.path().u8string();
		string filename = std::filesystem::path(fpath).filename().u8string();

		if (filename[0] == '.') // hidden file
		{
			continue;
		}

		FPORTS fpi = file_type_detect(fpath);
		string mime = file_mime_value(fpi, fpath);
		std::uintmax_t fsize = std::filesystem::file_size(fpath);

		e2db_file file;
		file.origin = FORG::filesys;
		file.path = fpath;
		file.filename = filename;
		file.mime = mime;
		file.size = int (fsize);

		this->e2db[filename] = file;
	}
	if (! find_services_file())
	{
		error("list_file", "File Error", "Services file not found.");
		return false;
	}
	this->filepath = path;

	return true;
}

bool e2db_parser::read(string path)
{
	debug("read", "path", path);

	try
	{
		if (list_file(path))
			parse_e2db();
		else
			return false;

		db.dstat = DSTAT::d_read;

		return true;
	}
	catch (const std::invalid_argument& err)
	{
		exception("read", "Parser Error", msg(MSG::except_invalid_argument, err.what()));
	}
	catch (const std::out_of_range& err)
	{
		exception("read", "Parser Error", msg(MSG::except_out_of_range, err.what()));
	}
	catch (const std::filesystem::filesystem_error& err)
	{
		exception("read", "Parser Error", msg(MSG::except_filesystem, err.what()));
	}
	catch (...)
	{
		exception("read", "Parser Error", msg(MSG::except_uncaught));
	}

	return false;
}

unordered_map<string, e2db_abstract::e2db_file> e2db_parser::get_input()
{
	debug("get_input");

	return this->e2db;
}

}
