/*!
 * e2-sat-editor/src/e2db/e2db_parser.cpp
 * 
 * @link https://github.com/ctlcltd/e2-sat-editor
 * @copyright e2 SAT Editor Team
 * @author Leonardo Laureti
 * @version 0.4
 * @license MIT License
 * @license GNU GPLv3 License
 */

#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <cstring>
#include <clocale>
#include <algorithm>
#include <sstream>
#include <fstream>
#include <filesystem>

#include "e2db_parser.h"

using std::ifstream, std::stringstream, std::hex, std::dec, std::to_string;

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

	std::clock_t start = std::clock();

	if (! find_services_file())
		return error("parse_e2db", "Error", "Lamedb services file not found.");

	ifstream ilamedb (this->e2db[this->services_filename]);
	parse_e2db_lamedb(ilamedb);
	ilamedb.close();

	if (PARSER_TUNERSETS)
	{
		if (this->e2db.count("satellites.xml"))
		{
			ifstream itunxml (this->e2db["satellites.xml"]);
			parse_tunersets_xml(YTYPE::satellite, itunxml);
			itunxml.close();
		}
		if (this->e2db.count("terrestrial.xml"))
		{
			ifstream itunxml (this->e2db["terrestrial.xml"]);
			parse_tunersets_xml(YTYPE::terrestrial, itunxml);
			itunxml.close();
		}
		if (this->e2db.count("cables.xml"))
		{
			ifstream itunxml (this->e2db["cables.xml"]);
			parse_tunersets_xml(YTYPE::cable, itunxml);
			itunxml.close();
		}
		if (this->e2db.count("atsc.xml"))
		{
			ifstream itunxml (this->e2db["atsc.xml"]);
			parse_tunersets_xml(YTYPE::atsc, itunxml);
			itunxml.close();
		}
	}

	for (auto & x : this->e2db)
	{
		if (x.first.find("bouquets.") != string::npos)
		{
			ifstream ibouquet (this->e2db[x.first]);
			parse_e2db_bouquet(ibouquet, x.first, (x.first.rfind(".epl") != string::npos));
			ibouquet.close();
		}
	}
	for (auto & x : bouquets)
	{
		for (string & fname : x.second.userbouquets)
		{
			ifstream iuserbouquet (this->e2db[fname]);
			parse_e2db_userbouquet(iuserbouquet, fname);
			iuserbouquet.close();
		}
	}

	if (PARSER_PARENTALLOCK_LIST)
	{
		if (LAMEDB_VER < 4)
		{
			if (this->e2db.count("services.locked"))
			{
				ifstream ilocked (this->e2db["services.locked"]);
				parse_e2db_parentallock_list(PARENTALLOCK::locked, ilocked);
				ilocked.close();
			}
		}
		else
		{
			if (this->e2db.count("blacklist"))
			{
				ifstream ilocked (this->e2db["blacklist"]);
				parse_e2db_parentallock_list(PARENTALLOCK::blacklist, ilocked);
				ilocked.close();
			}
			if (this->e2db.count("whitelist"))
			{
				ifstream ilocked (this->e2db["whitelist"]);
				parse_e2db_parentallock_list(PARENTALLOCK::whitelist, ilocked);
				ilocked.close();
			}
		}
	}

	std::clock_t end = std::clock();

	// seeds./enigma_db
	// commit: 67b6442	elapsed time: 65520
	// commit: d47fec7	elapsed time: 498870
	// commit: d1f53fe	elapsed time: 56112
	// commit: HEAD		elapsed time: 67343

	info("parse_e2db", "elapsed time", to_string(int (end - start)) + " ms.");
}


void e2db_parser::parse_e2db(unordered_map<string, e2db_file> files)
{
	debug("parse_e2db");

	std::clock_t start = std::clock();

	for (auto & x : files)
	{
		string filename = std::filesystem::path(x.first).filename().u8string(); //C++17
		this->e2db[filename] = x.first;
	}
	if (! find_services_file())
		return error("parse_e2db", "Error", "Lamedb services file not found.");

	stringstream ilamedb;
	ilamedb.write(&files[this->e2db[this->services_filename]].data[0], files[this->e2db[this->services_filename]].size);
	parse_e2db_lamedb(ilamedb);

	if (PARSER_TUNERSETS)
	{
		if (this->e2db.count("satellites.xml"))
		{
			stringstream itunxml;
			itunxml.write(&files[this->e2db["satellites.xml"]].data[0], files[this->e2db["satellites.xml"]].size);
			parse_tunersets_xml(YTYPE::satellite, itunxml);
		}
		if (this->e2db.count("terrestrial.xml"))
		{
			stringstream itunxml;
			itunxml.write(&files[this->e2db["terrestrial.xml"]].data[0], files[this->e2db["terrestrial.xml"]].size);
			parse_tunersets_xml(YTYPE::terrestrial, itunxml);
		}
		if (this->e2db.count("cables.xml"))
		{
			stringstream itunxml;
			itunxml.write(&files[this->e2db["cables.xml"]].data[0], files[this->e2db["cables.xml"]].size);
			parse_tunersets_xml(YTYPE::cable, itunxml);
		}
		if (this->e2db.count("atsc.xml"))
		{
			stringstream itunxml;
			itunxml.write(&files[this->e2db["atsc.xml"]].data[0], files[this->e2db["atsc.xml"]].size);
			parse_tunersets_xml(YTYPE::atsc, itunxml);
		}
	}

	for (auto & x : this->e2db)
	{
		if (x.first.find("bouquets.") != string::npos)
		{
			stringstream ibouquet;
			ibouquet.write(&files[x.second].data[0], files[x.second].size);
			parse_e2db_bouquet(ibouquet, x.second, (x.first.rfind(".epl") != string::npos));
		}
	}
	for (auto & x : bouquets)
	{
		for (string & filename : x.second.userbouquets)
		{
			stringstream iuserbouquet;
			iuserbouquet.write(&files[this->e2db[filename]].data[0], files[this->e2db[filename]].size);
			parse_e2db_userbouquet(iuserbouquet, filename);
		}
	}

	if (PARSER_PARENTALLOCK_LIST)
	{
		if (LAMEDB_VER < 4)
		{
			if (this->e2db.count("services.locked"))
			{
				stringstream ilocked;
				ilocked.write(&files[this->e2db["services.locked"]].data[0], files[this->e2db["services.locked"]].size);
				parse_e2db_parentallock_list(PARENTALLOCK::locked, ilocked);
			}
		}
		else
		{
			if (this->e2db.count("blacklist"))
			{
				stringstream ilocked;
				ilocked.write(&files[this->e2db["blacklist"]].data[0], files[this->e2db["blacklist"]].size);
				parse_e2db_parentallock_list(PARENTALLOCK::blacklist, ilocked);
			}
			if (this->e2db.count("whitelist"))
			{
				stringstream ilocked;
				ilocked.write(&files[this->e2db["whitelist"]].data[0], files[this->e2db["whitelist"]].size);
				parse_e2db_parentallock_list(PARENTALLOCK::whitelist, ilocked);
			}
		}
	}

	std::clock_t end = std::clock();

	info("parse_e2db", "elapsed time", to_string(int (end - start)) + " ms.");
}

void e2db_parser::parse_e2db_lamedb(istream& ilamedb)
{
	debug("parse_e2db_lamedb");

	string hlamedb;
	std::getline(ilamedb, hlamedb);
	char vlamedb = (hlamedb.substr(hlamedb.length() - 2, hlamedb.length() - 1))[0];
	int ver = isdigit(vlamedb) ? int (vlamedb) - 48 : 0;

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

	LAMEDB_VER = 5;
	bool step;
	int tidx = 0;
	int sidx = 0;
	string line;
	transponder tx;
	service ch;

	while (std::getline(ilamedb, line))
	{
		char type = line[0];
		if (type == 't')
			step = 1;
		else if (type == 's')
			step = 0;
		else
			continue;

		size_t delimit = line.find(',');
		string str = line.substr(2, delimit - 2);
		string params = line.substr(delimit + 1);

		// transponder
		if (step)
		{
			tx = transponder ();
			tidx += 1;
			parse_lamedb_transponder_params(str, tx);
			parse_lamedb_transponder_feparms(params.substr(2), params[0], tx);
			add_transponder(tidx, tx);
		}
		// service
		else
		{
			ch = service ();
			sidx += 1;
			size_t delimit = params.rfind('"');
			string chname = params.substr(1, delimit - 1);
			string chdata = params.rfind(',') != string::npos ? params.substr(delimit + 2) : "";
			parse_lamedb_service_params(str, ch);
			append_lamedb_service_name(chname, ch);
			parse_lamedb_service_data(chdata, ch);
			add_service(sidx, ch);
		}
	}
}

void e2db_parser::parse_e2db_lamedbx(istream& ilamedb, int ver)
{
	debug("parse_e2db_lamedbx", "version", ver);

	LAMEDB_VER = ver;
	int step = 0;
	int s = 0;
	int tidx = 0;
	int sidx = 0;
	string line;
	transponder tx;
	service ch;

	while (std::getline(ilamedb, line))
	{
		if (! step && line == "transponders")
		{
			step = 1;
			continue;
		}
		else if (! step && line == "services")
		{
			step = 2;
			continue;
		}
		else if (step && line == "end")
		{
			step = 0;
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
				parse_lamedb_transponder_feparms(line.substr(3), line.substr(1, 2)[0], tx);
				s++;
			}
			else if (s == 2)
			{
				add_transponder(tidx, tx);
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
				add_service(sidx, ch);
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

void e2db_parser::parse_lamedb_transponder_feparms(string str, char ty, transponder& tx)
{
	int freq, sr, pol, fec, pos, inv, flgs, sys, mod, rol, pil;
	int band, hpfec, lpfec, tmod, tmx, guard, hier;
	int cmod, cfec;
	int amod;
	char oflgs[33] = "";
	sys = -1, mod = -1, rol = -1, pil = -1;
	// tx.pol = -1, tx.fec = -1, tx.inv = -1, tx.sys = -1, tx.mod = -1, tx.rol = -1, tx.pil = -1;
	// tx.band = -1, tx.hpfec = -1, tx.lpfec = -1, tx.tmod = -1, tx.tmx = -1, tx.guard = -1, tx.hier = -1;
	// tx.cmod = -1, tx.cfec = -1;
	// tx.amod = -1;

	switch (ty)
	{
		case 's': // DVB-S / DVB-S2
			std::sscanf(str.c_str(), "%8d:%8d:%1d:%1d:%4d:%1d:%1d:%1d:%1d:%1d:%1d%s", &freq, &sr, &pol, &fec, &pos, &inv, &flgs, &sys, &mod, &rol, &pil, oflgs);

			tx.ytype = YTYPE::satellite;
			tx.freq = int (freq / 1e3);
			tx.sr = int (sr / 1e3);
			tx.pol = pol;
			tx.fec = fec;
			tx.pos = pos;
			tx.inv = inv;
			tx.flgs = flgs;
			tx.sys = sys;
			tx.mod = mod;
			tx.rol = rol;
			tx.pil = pil;
			tx.oflgs = string (oflgs);
		break;
		case 't': // DVB-T / DVB-T2
			std::sscanf(str.c_str(), "%9d:%1d:%1d:%1d:%1d:%1d:%1d:%1d:%1d%s", &freq, &band, &hpfec, &lpfec, &tmod, &tmx, &guard, &hier, &inv, oflgs);

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
			tx.oflgs = string (oflgs);
		break;
		case 'c': // DVB-C
			std::sscanf(str.c_str(), "%8d:%8d:%1d:%1d:%1d%s", &freq, &sr, &inv, &cmod, &cfec, oflgs);

			tx.ytype = YTYPE::cable;
			tx.freq = int (freq / 1e3);
			tx.sr = int (sr / 1e3);
			tx.inv = inv;
			tx.cmod = cmod;
			tx.cfec = cfec;
			tx.oflgs = string (oflgs);
		break;
		case 'a': // ATSC / DVB-C ANNEX B
			std::sscanf(str.c_str(), "%8d:%1d:%1d:%1d:%1d%s", &freq, &inv, &amod, &flgs, &sys, oflgs);

			tx.ytype = YTYPE::atsc;
			tx.freq = int (freq / 1e3);
			tx.inv = inv;
			tx.amod = amod;
			tx.flgs = flgs;
			tx.sys = sys;
			tx.oflgs = string (oflgs);
		break;
		default:
		return error("parse_lamedb_transponder_feparms", "Parser Error", "Unknown transponder type.");
	}
}

void e2db_parser::parse_lamedb_service_params(string str, service& ch)
{
	int ssid, dvbns, tsid, onid, stype, snum;
	ssid = 0, dvbns = 0, tsid = 0, onid = 0;
	stype = -1, snum = -1;

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

//TODO improve
void e2db_parser::parse_lamedb_service_data(string str, service& ch)
{
	if (str.empty())
		return;

	//TODO max length 256 EOL
	stringstream ss (str);
	string line;
	map<char, vector<string>> cstr;

	while (std::getline(ss, line, ','))
	{
		char key = line[0];
		switch (key)
		{
			case 'p': key = e2db_parser::SDATA::p; break;
			case 'c': key = e2db_parser::SDATA::c; break;
			case 'C': key = e2db_parser::SDATA::C; break;
			case 'f': key = e2db_parser::SDATA::f; break;
		}
		string val = line.substr(2);
		cstr[key].push_back(val);
	}
	ch.data = cstr;
}

void e2db_parser::append_lamedb_service_name(string str, service& ch)
{
	ch.chname = str;
}

void e2db_parser::parse_e2db_bouquet(istream& ibouquet, string bname, bool epl)
{
	debug("parse_e2db_bouquet", "bname", bname);

	bool add = ! bouquets.count(bname);
	string line;

	bouquet& bs = bouquets[bname];
	userbouquet ub;

	while (std::getline(ibouquet, line))
	{
		if (line.find("#SERVICE") != string::npos)
		{
			ub = userbouquet ();
			if (epl)
				parse_userbouquet_epl_reference(line.substr(9), ub);
			else
				parse_userbouquet_reference(line.substr(9), ub);
			ub.pname = bname;
			add_userbouquet(int (index["ubs"].size()), ub);
		}
		else if (line.find("#NAME") != string::npos)
		{
			if (add)
				bs = bouquet ();

			bs.bname = bname;
			bs.name = line.substr(6);
			if (bname.find(".tv") != string::npos)
			{
				bs.btype = 1;
				bs.nname = "TV";
			}
			else if (bname.find(".radio") != string::npos)
			{
				bs.btype = 2;
				bs.nname = "Radio";
			}

			if (add)
				add_bouquet(bs.btype, bs);
		}
	}
}

void e2db_parser::parse_e2db_userbouquet(istream& iuserbouquet, string bname)
{
	debug("parse_e2db_userbouquet", "bname", bname);

	int step = 0;
	int idx = 0;
	int y = 0;
	string line;

	userbouquet& ub = userbouquets[bname];
	channel_reference chref;
	service_reference ref;

	while (std::getline(iuserbouquet, line))
	{
		if (! step && line.find("#NAME") != string::npos)
		{
			ub.name = line.substr(6);
			step = 1;
			continue;
		}
		else if (step == 2)
		{
			if (line.find("#DESCRIPTION") != string::npos)
				set_channel_reference_marker_value(ub, chref.chid, line.substr(13));
			step = 1;
			continue;
		}
		else if (step && line.find("#SORT") != string::npos)
		{
			continue;
		}
		else if (step && line.empty())
		{
			continue;
		}

		if (step)
		{
			chref = channel_reference ();
			ref = service_reference ();

			parse_channel_reference(line.substr(9), chref, ref);

			if (chref.marker)
			{
				step = 2;
				idx = 0;
			}
			else
			{
				y++;
				idx = y;
			}

			add_channel_reference(idx, ub, chref, ref);
		}
	}
}

void e2db_parser::parse_e2db_parentallock_list(PARENTALLOCK ltype, istream& ilocked)
{
	debug("parse_e2db_parentallock_list", "ltype", ltype);

	string line;

	channel_reference chref;
	service_reference ref;

	if (ltype == PARENTALLOCK::locked)
		std::getline(ilocked, line);

	int count = 0;

	while (std::getline(ilocked, line))
	{
		ref = service_reference ();

		parse_channel_reference(line, chref, ref);

		char chid[25];

		std::snprintf(chid, 25, "%x:%x:%x", ref.ssid, ref.tsid, ref.dvbns);

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

void e2db_parser::parse_userbouquet_reference(string str, userbouquet& ub)
{
	char refid[33];
	char fname[33];
	char oby[13];

	std::sscanf(str.c_str(), "%32s BOUQUET %32s ORDER BY %12s", refid, fname, oby);

	ub.bname = string (fname);
	ub.bname = ub.bname.substr(1, ub.bname.length() - 2);
}

void e2db_parser::parse_userbouquet_epl_reference(string str, userbouquet& ub)
{
	string path;
	size_t pos = str.rfind("0:");

	if (pos != string::npos)
		path = str.substr(pos, str.length());

	string filename = std::filesystem::path(path).filename().u8string(); //C++17

	ub.bname = filename;
}

void e2db_parser::parse_channel_reference(string str, channel_reference& chref, service_reference& ref)
{
	int i, atype, anum, ssid, tsid, onid, dvbns;
	i = 0, atype = 0, anum = 0, ssid = 0, tsid = 0, onid = 0, dvbns = 0;

	std::sscanf(str.c_str(), "%d:%d:%X:%X:%X:%X:%X", &i, &atype, &anum, &ssid, &tsid, &onid, &dvbns);
	//TODO other flags ? "...:%d:%d:%d:"

	switch (atype)
	{
		// marker
		case STYPE::regular_marker:
		case STYPE::numbered_marker:
		case STYPE::hidden_marker_1:
		case STYPE::hidden_marker_2:
			chref.marker = true;
		break;
		//TODO group
		// group
		case STYPE::group:
			error("parse_channel_reference", "Parser Error", "Not supported yet.");
		break;
		// service
		default:
			chref.marker = false;
			ref.ssid = ssid;
			ref.dvbns = dvbns;
			ref.tsid = tsid;
	}

	chref.atype = atype;
	chref.anum = anum;
}

//TODO FIX string::npos != 4294967295 in mingw // pos = key.find_first_of('\0');
//TODO value xml entities
void e2db_parser::parse_tunersets_xml(int ytype, istream& ftunxml)
{
	debug("parse_tunersets_xml", "ytype", ytype);

	string htunxml;
	string charset = "UTF-8";
	std::getline(ftunxml, htunxml, '>');

	if (htunxml.find("<?xml") == string::npos)
		return error("parse_tunersets_xml", "Parser Error", "Unknown file format.");

	unsigned long pos = htunxml.find("encoding=");
	unsigned long len;
	if (pos != string::npos)
	{
		charset = htunxml.substr(pos + 10);
		len = charset.rfind('"');
		if (len == string::npos)
			len = charset.rfind('\'');
		charset = charset.substr(0, len);
		// std::transform(charset.begin(), charset.end(), charset.begin(), [](unsigned char c) { return toupper(c); });
	}

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

	while (std::getline(ftunxml, line, '>'))
	{
		if (line.find("<!") != string::npos)
		{
			comment s;
			s.type = line.find('\n') != string::npos;
			s.ln = ln;
			s.text = line.substr(line.find("<!--") + 4);
			s.text = s.text.substr(0, s.text.length() - 2);
			comments[iname].emplace_back(s);
			continue;
		}

		string tag;
		bool add = false;
		unsigned long pos = line.find('<');
		unsigned long len;
		if (pos != string::npos)
		{
			tag = line.substr(pos + 1);
			pos = tag[0] == '/';
			len = tag.find(' ');
			add = pos || tag[tag.length() - 1] == '/';
			if (! pos && len == string::npos)
				len = tag.find('/');
			tag = tag.substr(pos, len);
		}
		if (depth.count(tag))
		{
			ln++;
			step = depth[tag];

			if (step == 1)
			{
				tn = tunersets_table ();
				tn.ytype = ytype;
				// tn.flgs = -1, tn.pos = -1, tn.feed = -1;
			}
			else if (step == 2)
			{
				tntxp = tunersets_transponder ();
				tntxp.freq = -1;
				// tntxp.freq = -1, tntxp.sr = -1, tntxp.pol = -1, tntxp.fec = -1, tntxp.mod = -1, tntxp.inv = -1, tntxp.sys = -1, tntxp.rol = -1, tntxp.pil = -1, tntxp.isid = -1, tntxp.mts = -1, tntxp.plsmode = -1, tntxp.plscode = -1, tntxp.plsn = -1;
				// tntxp.band = -1, tntxp.hpfec = -1, tntxp.lpfec = -1, tntxp.tmod = -1, tntxp.tmx = -1, tntxp.guard = -1, tntxp.hier = -1;
				// tntxp.cfec = -1, tntxp.cmod = -1;
				// tntxp.amod = -1;
			}
		}
#ifndef __MINGW32__
		else if (tag.empty())
#else
		else if (tag.size() < 2)
#endif
		{
			continue;
		}
		else
		{
			return error("parse_tunersets_xml", "Parser Error", "Malformed or unknown XML error.");
		}

		string yey;
		char* token = std::strtok(line.data(), " ");
		while (token != 0)
		{
			string str = string (token);
			string key, val;
			size_t pos;
			size_t len = str.find('=');

			if (len != string::npos)
			{
				key = str.substr(0, len);
				std::transform(key.begin(), key.end(), key.begin(), [](unsigned char c) { return std::isspace(c) ? '\0' : c; });
				key.erase(0, key.find_first_not_of('\0'));
#ifndef __MINGW32__
				pos = key.find_first_of('\0');
				if (pos != string::npos)
					key.erase(pos, key.size());
#endif
				val = str.substr(len + 1);
				pos = val.find('"');
				if (pos != string::npos)
				{
					len = val.rfind('"');
				}
				else
				{
					pos = val.find('\'');
					if (pos != string::npos)
						len = val.rfind('\'');
				}

				if (len)
				{
					val = val.substr(pos + 1, len - 1);
				}
				else
				{
					val = line.substr(line.find(key) + key.length() + 2);
					len = val.find('"');
					if (len == string::npos)
						len = val.find('\'');
					val = val.substr(0, len);
					std::transform(val.begin(), val.end(), val.begin(), [](unsigned char c) { return c ? c : ' '; });
				}
			}
			else if (! yey.empty() && ((str[0] == '"' && str.find('"') != string::npos) || (str[0] == '\'' && str.find('\'') != string::npos)))
			{
				key = yey;
				val = str;
				yey = "";
				pos = val.find('"');
				if (pos != string::npos)
				{
					len = val.rfind('"');
				}
				else
				{
					pos = val.find('\'');
					if (pos != string::npos)
						len = val.rfind('\'');
				}

				if (len)
				{
					val = val.substr(pos + 1, len - 1);
				}
				else
				{
					val = line.substr(line.find(key) + key.length() + 2);
					len = val.find('"');
					if (len == string::npos)
						len = val.find('\'');
					val = val.substr(0, len);
					std::transform(val.begin(), val.end(), val.begin(), [](unsigned char c) { return c ? c : ' '; });
				}
			}
			else if (str.find('/') == string::npos && str.find('<') == string::npos)
			{
				key = str;
				std::transform(key.begin(), key.end(), key.begin(), [](unsigned char c) { return std::isspace(c) ? '\0' : c; });
				key.erase(0, key.find_first_not_of('\0'));
#ifndef __MINGW32__
				pos = key.find_first_of('\0');
				if (pos != string::npos)
					key.erase(pos, key.size());
#endif
				if (! key.empty())
					yey = str;
			}
			token = std::strtok(NULL, " ");

			switch (ytype)
			{
				case YTYPE::satellite:
					if (key == "name")
						tn.name = val;
					else if (key == "flags")
						tn.flgs = std::atoi(val.data());
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
				break;
				case YTYPE::terrestrial:
					if (key == "name")
						tn.name = val;
					else if (key == "flags")
						tn.flgs = std::atoi(val.data());
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
				break;
				case YTYPE::cable:
					if (key == "name")
						tn.name = val;
					else if (key == "flags")
						tn.flgs = std::atoi(val.data());
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
						tn.name = val;
					else if (key == "flags")
						tn.flgs = std::atoi(val.data());
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
			char tnid[7];
			std::snprintf(tnid, 7, "%c:%04x", yname, bidx);
			tunersets_table& tn = tv.tables[tnid];
			cidx++;
			add_tunersets_transponder(cidx, tntxp, tn);
		}
	}

	add_tunersets(tv);
}

bool e2db_parser::find_services_file()
{
	debug("find_services_file");

	if (PARSER_LAMEDB5_PRIOR && this->e2db.count("lamedb5"))
		this->services_filename = "lamedb5";
	else if (this->e2db.count("lamedb5"))
		this->services_filename = "lamedb5";
	else if (this->e2db.count("lamedb"))
		this->services_filename = "lamedb";
	else if (this->e2db.count("services"))
		this->services_filename = "services";

	return ! this->services_filename.empty();
}

bool e2db_parser::list_file(string path)
{
	debug("list_file", "path", path);

	if (! std::filesystem::exists(path)) //C++17
	{
		error("list_file", "File Error", "File \"" + path + "\" not exists.");
		return false;
	}
	if ((std::filesystem::status(path).permissions() & std::filesystem::perms::group_read)  == std::filesystem::perms::none) //C++17
	{
		error("list_file", "File Error", "File \"" + path + "\" is not readable.");
		return false;
	}

	std::filesystem::directory_iterator filelist (path); //C++17

	for (const auto & entry : filelist)
	{
		if (! std::filesystem::is_regular_file(entry)) //C++17
		{
			continue;
		}
		if ((std::filesystem::status(entry).permissions() & std::filesystem::perms::group_read) == std::filesystem::perms::none) //C++17
		{
			error("list_file", "File Error", "File \"" + path + "\" is not readable.");
			return false;
		}

		string fpath = entry.path().u8string(); //C++17
		string filename = std::filesystem::path(fpath).filename().u8string(); //C++17
		this->e2db[filename] = fpath;
	}
	if (! find_services_file())
	{
		error("list_file", "File Error", "Lamedb services file not found.");
		return false;
	}
	this->filepath = path;

	return true;
}

bool e2db_parser::read(string path)
{
	debug("read", "path", path);

	if (list_file(path))
		parse_e2db();
	else
		return false;

	return true;
}

unordered_map<string, string> e2db_parser::get_input() {
	debug("get_input");

	return this->e2db;
}

}
