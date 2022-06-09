/*!
 * e2-sat-editor/src/e2db.cpp
 * 
 * @link https://github.com/ctlcltd/e2-sat-editor
 * @copyright e2 SAT Editor Team
 * @author Leonardo Laureti
 * @version 0.1
 * @license MIT License
 * @license GNU GPLv3 License
 */

#include <algorithm>
#include <unordered_set>
#include <sstream>
#include <filesystem>
#include <cstdio>
#include <cstring>

#include "e2db.h"

using std::string, std::pair, std::vector, std::map, std::unordered_map, std::unordered_set, std::set_difference, std::inserter, std::cout, std::endl, std::ifstream, std::ofstream, std::stringstream, std::to_string, std::hex, std::dec, std::setfill, std::setw, std::uppercase;

namespace e2se_e2db
{

void e2db_abstract::debug(string cmsg)
{
	this->log->debug(cmsg);
}

void e2db_abstract::debug(string cmsg, string optk, string optv)
{
	this->log->debug(cmsg, optk, optv);
}

void e2db_abstract::error(string cmsg)
{
	this->log->error(cmsg);
}

void e2db_abstract::error(string cmsg, string optk, string optv)
{
	this->log->error(cmsg, optk, optv);
}

void e2db_abstract::add_transponder(int idx, transponder& tx)
{
	char txid[25];
	std::sprintf(txid, "%x:%x", tx.tsid, tx.dvbns);
	tx.txid = txid;

	tx.index = idx;
	db.transponders.emplace(tx.txid, tx);
	index["txs"].emplace_back(pair (idx, tx.txid)); //C++17
}

void e2db_abstract::add_service(int idx, service& ch)
{
	char chid[25];
	char txid[25];
	std::sprintf(txid, "%x:%x", ch.tsid, ch.dvbns);
	std::sprintf(chid, "%x:%x:%x", ch.ssid, ch.tsid, ch.dvbns);
	ch.txid = txid;
	ch.chid = chid;

	if (db.services.count(ch.chid))
	{
		int m;
		string kchid = 's' + ch.chid;
		if (ch.snum) m = ch.snum;
		else m = collisions[kchid].size();
		ch.chid += ':' + to_string(m);
		collisions[kchid].emplace_back(pair (ch.chid, m)); //C++17
	}

	string iname = "chs:" + (STYPES.count(ch.stype) ? to_string(STYPES.at(ch.stype).first) : "0");
	ch.index = idx;
	db.services.emplace(ch.chid, ch);
	index["chs"].emplace_back(pair (idx, ch.chid)); //C++17
	index[iname].emplace_back(pair (idx, ch.chid)); //C++17
}

void e2db_abstract::add_bouquet(int idx, bouquet& bs)
{
	bs.index = idx;
	index["bss"].emplace_back(pair (idx, bs.bname)); //C++17
	bouquets.emplace(bs.bname, bs);
}

void e2db_abstract::add_userbouquet(int idx, userbouquet& ub)
{
	ub.index = idx;
	index["ubs"].emplace_back(pair (idx, ub.bname)); //C++17
	bouquets[ub.pname].userbouquets.emplace_back(ub.bname);
	userbouquets.emplace(ub.bname, ub);
}

void e2db_abstract::add_channel_reference(int idx, userbouquet& ub, channel_reference& chref, service_reference& ref)
{
	char chid[25];

	if (chref.marker)
		std::sprintf(chid, "%d:%x:%d", chref.type, chref.anum, ub.index);
	else
		std::sprintf(chid, "%x:%x:%x", ref.ssid, ref.tsid, ref.dvbns);

	chref.chid = chid;
	chref.index = idx;

	ub.channels.emplace(chref.chid, chref);
	index[ub.bname].emplace_back(pair (idx, chref.chid)); //C++17

	if (chref.marker)
		index["mks"].emplace_back(pair (ub.index, chref.chid)); //C++17
	else
		index[ub.pname].emplace_back(pair ((index[ub.pname].size() + 1), chref.chid)); //C++17
}

void e2db_abstract::set_channel_reference_marker_value(userbouquet& ub, string chid, string value)
{
	ub.channels[chid].value = value;
}

void e2db_abstract::add_tunerset(int idx, tuner_sets& tn)
{
	string iname = "tns:";
	char type;
	switch (tn.ytype)
	{
		case YTYPE::sat: type = 's'; break;
		case YTYPE::terrestrial: type = 't'; break;
		case YTYPE::cable: type = 'c'; break;
		case YTYPE::atsc: type = 'a'; break;
		default: return; //TODO
	}
	iname += type;
	char tnid[12];
	std::sprintf(tnid, "%c:%04x", type, idx);
	tn.tnid = tnid;
	tn.index = idx;
	tunersets.emplace(tn.tnid, tn);
	index[iname].emplace_back(pair (idx, tn.tnid)); //C++17
	if (tn.ytype == YTYPE::sat)
		tunersets_pos.emplace(tn.pos, tn.tnid);
}

void e2db_abstract::add_tunerset_transponder(int idx, tuner_sets& tn, tuner_transponder& tntxp)
{
	char type;
	switch (tn.ytype)
	{
		case YTYPE::sat: type = 's'; break;
		case YTYPE::terrestrial: type = 't'; break;
		case YTYPE::cable: type = 'c'; break;
		case YTYPE::atsc: type = 'a'; break;
		default: return; //TODO
	}
	char trid[12];
	std::sprintf(trid, "%c:%04x", type, tntxp.freq);
	tntxp.trid = trid;
	tntxp.index = idx;
	tn.transponders.emplace(tntxp.trid, tntxp);
	index[tn.tnid].emplace_back(pair (idx, tntxp.trid)); //C++17
}



e2db_parser::e2db_parser()
{
	this->log = new e2se::logger("e2db");

	debug("e2db_parser()");

	//TEST
	dbfilename = PARSER_LAMEDB5_PRIOR ? "lamedb5" : "lamedb";
	//TEST
}

void e2db_parser::parse_e2db()
{
	debug("parse_e2db()");
	std::clock_t start = std::clock();

	ifstream ilamedb (e2db[dbfilename]);
	parse_e2db_lamedb(ilamedb);
	ilamedb.close();

	if (PARSER_TUNERSETS)
	{
		if (e2db.count("satellites.xml"))
		{
			ifstream itunxml (e2db["satellites.xml"]);
			parse_tunersets_xml(YTYPE::sat, itunxml);
			itunxml.close();
		}
		if (e2db.count("terrestrial.xml"))
		{
			ifstream itunxml (e2db["terrestrial.xml"]);
			parse_tunersets_xml(YTYPE::terrestrial, itunxml);
			itunxml.close();
		}
		if (e2db.count("cables.xml"))
		{
			ifstream itunxml (e2db["cables.xml"]);
			parse_tunersets_xml(YTYPE::cable, itunxml);
			itunxml.close();
		}
		if (e2db.count("atsc.xml"))
		{
			ifstream itunxml (e2db["atsc.xml"]);
			parse_tunersets_xml(YTYPE::atsc, itunxml);
			itunxml.close();
		}
	}

	for (auto & x: e2db)
	{
		if (x.first.find("bouquets.") != string::npos)
		{
			ifstream ibouquet (e2db[x.first]);
			parse_e2db_bouquet(ibouquet, x.first);
			ibouquet.close();
		}
	}
	for (auto & x: bouquets)
	{
		for (auto & w: x.second.userbouquets)
		{
			ifstream iuserbouquet (e2db[w]);
			parse_e2db_userbouquet(iuserbouquet, w);
			iuserbouquet.close();
		}
	}

	std::clock_t end = std::clock();

	// seeds./enigma_db
	// commit: 67b6442	elapsed time: 65520
	// commit: d47fec7	elapsed time: 498870
	// commit: d1f53fe	elapsed time: 56112
	// commit: HEAD		elapsed time: 67343

	debug("parse_e2db()", "elapsed time", to_string(end - start));
}


void e2db_parser::parse_e2db(unordered_map<string, e2db_file> files)
{
	debug("parse_e2db()");
	std::clock_t start = std::clock();

	for (auto & x: files)
	{
		string filename = std::filesystem::path(x.first).filename().u8string(); //C++17
		e2db[filename] = x.first;
	}
	
	stringstream ilamedb;
	ilamedb.write(&files[e2db[dbfilename]][0], files[e2db[dbfilename]].size());
	parse_e2db_lamedb(ilamedb);

	if (PARSER_TUNERSETS)
	{
		if (e2db.count("satellites.xml"))
		{
			stringstream itunxml;
			itunxml.write(&files[e2db["satellites.xml"]][0], files[e2db["satellites.xml"]].size());
			parse_tunersets_xml(YTYPE::sat, itunxml);
		}
		if (e2db.count("terrestrial.xml"))
		{
			stringstream itunxml;
			itunxml.write(&files[e2db["terrestrial.xml"]][0], files[e2db["terrestrial.xml"]].size());
			parse_tunersets_xml(YTYPE::terrestrial, itunxml);
		}
		if (e2db.count("cables.xml"))
		{
			stringstream itunxml;
			itunxml.write(&files[e2db["cables.xml"]][0], files[e2db["cables.xml"]].size());
			parse_tunersets_xml(YTYPE::cable, itunxml);
		}
		if (e2db.count("atsc.xml"))
		{
			stringstream itunxml;
			itunxml.write(&files[e2db["atsc.xml"]][0], files[e2db["atsc.xml"]].size());
			parse_tunersets_xml(YTYPE::atsc, itunxml);
		}
	}

	for (auto & x: e2db)
	{
		if (x.first.find("bouquets.") != string::npos)
		{
			stringstream ibouquet;
			ibouquet.write(&files[x.second][0], files[x.second].size());
			string filename = std::filesystem::path(x.first).filename().u8string();
			parse_e2db_bouquet(ibouquet, filename);
		}
	}
	for (auto & x: bouquets)
	{
		for (auto & w: x.second.userbouquets)
		{
			stringstream iuserbouquet;
			iuserbouquet.write(&files[e2db[w]][0], files[e2db[w]].size());
			string filename = std::filesystem::path(w).filename().u8string();
			parse_e2db_userbouquet(iuserbouquet, filename);
		}
	}

	std::clock_t end = std::clock();

	debug("parse_e2db()", "elapsed time", to_string(end - start));
}

void e2db_parser::parse_e2db_lamedb(istream& ilamedb)
{
	debug("parse_e2db_lamedb()");

	string hlamedb;
	std::getline(ilamedb, hlamedb);
	char vlamedb = (hlamedb.substr(hlamedb.length() - 2, hlamedb.length() - 1))[0];
	int dbver = isdigit(vlamedb) ? int (vlamedb) - 48 : 0;

	debug("parse_e2db_lamedb()", "File header", hlamedb);

	switch (dbver)
	{
		case 4: parse_e2db_lamedb4(ilamedb); break;
		case 5: parse_e2db_lamedb5(ilamedb); break;
		default: error("parse_e2db_lamedb()", "Error", "Unknown database format.");
	}
}

void e2db_parser::parse_e2db_lamedb4(istream& ilamedb)
{
	debug("parse_e2db_lamedb4()");
	
	LAMEDB_VER = 4;
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

void e2db_parser::parse_e2db_lamedb5(istream& ilamedb)
{
	debug("parse_e2db_lamedb5()");

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
		string data = line.substr(2, delimit - 2);
		string params = line.substr(delimit + 1);

		// transponder
		if (step)
		{
			tx = transponder ();
			tidx += 1;
			parse_lamedb_transponder_params(data, tx);
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
			parse_lamedb_service_params(data, ch);
			append_lamedb_service_name(chname, ch);
			parse_lamedb_service_data(chdata, ch);
			add_service(sidx, ch);
		}
	}
}

void e2db_parser::parse_lamedb_transponder_params(string data, transponder& tx)
{
	int dvbns, tsid, onid;
	dvbns = 0, tsid = 0, onid = 0;

	std::sscanf(data.c_str(), "%08x:%04x:%04x", &dvbns, &tsid, &onid);

	tx.dvbns = dvbns;
	tx.tsid = tsid;
	tx.onid = onid;
}

void e2db_parser::parse_lamedb_transponder_feparms(string data, char ttype, transponder& tx)
{
	int freq, sr, pol, fec, pos, inv, flgs, sys, mod, rol, pil;
	int band, hpfec, lpfec, tmod, tmx, guard, hier;
	int cmod, cfec;
	int amod;
	char oflgs[33] = "";
	sys = -1, mod = -1, rol = -1, pil = -1;
	tx.pol = -1, tx.fec = -1, tx.inv = -1, tx.sys = -1, tx.mod = -1, tx.rol = -1, tx.pil = -1;
	tx.band = -1, tx.hpfec = -1, tx.lpfec = -1, tx.tmod = -1, tx.tmx = -1, tx.guard = -1, tx.hier = -1;
	tx.cmod = -1, tx.cfec = -1;
	tx.amod = -1;

	switch (ttype)
	{
		case 's':
			std::sscanf(data.c_str(), "%8d:%8d:%1d:%1d:%4d:%1d:%1d:%1d:%1d:%1d:%1d%s", &freq, &sr, &pol, &fec, &pos, &inv, &flgs, &sys, &mod, &rol, &pil, oflgs);

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
		//TODO test
		case 't': // DVB-T
			std::sscanf(data.c_str(), "%9d:%1d:%1d:%1d:%1d:%1d:%1d:%1d:%1d%s", &freq, &band, &hpfec, &lpfec, &tmod, &tmx, &guard, &hier, &inv, oflgs);

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
		//TODO test
		case 'c': // DVB-C
			std::sscanf(data.c_str(), "%8d:%8d:%1d:%1d:%1d%s", &freq, &sr, &inv, &cmod, &cfec, oflgs);

			tx.freq = int (freq / 1e3);
			tx.sr = int (sr / 1e3);
			tx.inv = inv;
			tx.cmod = cmod;
			tx.cfec = cfec;
			tx.oflgs = string (oflgs);
		break;
		//TODO test
		case 'a': // ATSC
			std::sscanf(data.c_str(), "%8d:%1d:%1d:%1d:%1d%s", &freq, &inv, &amod, &flgs, &sys, oflgs);

			tx.freq = int (freq / 1e3);
			tx.inv = inv;
			tx.amod = amod;
			tx.flgs = flgs;
			tx.sys = sys;
			tx.oflgs = string (oflgs);
		break;
		default:
			error("lamedb", "Error", "Unknown transponder type.");
			return;
	}
	tx.ttype = ttype;
}

void e2db_parser::parse_lamedb_service_params(string data, service& ch)
{
	int ssid, dvbns, tsid, stype, snum;
	char onid[5]; //TODO to int
	ssid = 0, dvbns = 0, tsid = 0, stype = -1, snum = -1;

	if (LAMEDB_VER == 5)
	{
		int srcid = -1;
		std::sscanf(data.c_str(), "%04x:%08x:%04x:%4s:%3d:%4d:%d", &ssid, &dvbns, &tsid, onid, &stype, &snum, &srcid);
		ch.srcid = srcid;
	}
	else
	{
		std::sscanf(data.c_str(), "%04x:%08x:%04x:%4s:%3d:%4d", &ssid, &dvbns, &tsid, onid, &stype, &snum);
	}

	ch.ssid = ssid;
	ch.dvbns = dvbns;
	ch.tsid = tsid;
	ch.onid = onid;
	ch.onid.erase(0, ch.onid.find_first_not_of('0'));
	ch.stype = stype;
	ch.snum = snum;
}

//TODO FIX could cause SEGFAULT with empty data (infinite loop)
void e2db_parser::parse_lamedb_service_data(string data, service& ch)
{
	if (data.empty())
		return;

	//TODO 256 EOL
	stringstream ss (data);
	string line;
	map<char, vector<string>> cdata;

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
		cdata[key].push_back(val);
	}
	ch.data = cdata;
}

void e2db_parser::append_lamedb_service_name(string data, service& ch)
{
	ch.chname = data;
}

void e2db_parser::parse_e2db_bouquet(istream& ibouquet, string bname)
{
	debug("parse_e2db_bouquet()", "bname", bname);

	bool add = true;
	string line;
	bouquet& bs = bouquets[bname];
	userbouquet ub;

	while (std::getline(ibouquet, line))
	{
		if (line.find("#SERVICE") != string::npos)
		{
			ub = userbouquet ();
			parse_userbouquet_reference(line.substr(9), ub);
			ub.pname = bname;
			add_userbouquet(index["ubs"].size(), ub);
		}
		else if (line.find("#NAME") != string::npos)
		{
			if (! bouquets.count(bname))
				bs = bouquet ();
			else
				add = false;

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
	debug("parse_e2db_userbouquet()", "bname", bname);

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

void e2db_parser::parse_userbouquet_reference(string data, userbouquet& ub)
{
	char refid[33];
	char fname[33];
	char oby[13];

	std::sscanf(data.c_str(), "%32s BOUQUET %32s ORDER BY %12s", refid, fname, oby);

	ub.bname = string (fname);
	ub.bname = ub.bname.substr(1, ub.bname.length() - 2);
}

void e2db_parser::parse_channel_reference(string data, channel_reference& chref, service_reference& ref)
{
	int i, type, anum, ssid, tsid, onid, dvbns;
	i = -1, type = -1, anum = -1, ssid = 0, tsid = 0, onid = -1, dvbns = 0;

	std::sscanf(data.c_str(), "%d:%d:%4X:%4X:%4X:%4X:%8X", &i, &type, &anum, &ssid, &tsid, &onid, &dvbns);
	//TODO other flags ?

	switch (type)
	{
		case 64:  // regular marker
		case 320: // numbered marker
		case 512: // hidden marker
		case 832: // hidden marker
			chref.marker = true;
		break;
		case 128: // group //TODO
			error("parse_channel_reference()", "Error", "Not supported yet.");
		break;
		default:  // service
			chref.marker = false;
			ref.ssid = ssid;
			ref.dvbns = dvbns;
			ref.tsid = tsid;
	}

	chref.type = type;
	chref.anum = anum;
}

//TODO value xml entities
void e2db_parser::parse_tunersets_xml(int ytype, istream& ftunxml)
{
	debug("parse_tunersets_xml()", "ytype", to_string(ytype));

	string htunxml;
	string charset = "UTF-8";
	std::getline(ftunxml, htunxml, '>');

	if (htunxml.find("<?xml") == string::npos)
		return error("parse_tunersets_xml()", "Error", "Unknown file format.");

	unsigned long pos = htunxml.find("encoding=");
	unsigned long len;
	if (pos != string::npos)
	{
		charset = htunxml.substr(pos + 10);
		len = charset.rfind('"');
		if (len == string::npos)
			len = charset.rfind('\'');
		charset = charset.substr(0, len);
		std::transform(charset.begin(), charset.end(), charset.begin(), [](unsigned char c) { return toupper(c); });
	}

	debug("parse_tunersets_xml()", "charset", charset);

	tuner_sets tn;
	tuner_transponder tntxp;

	switch (ytype)
	{
		case YTYPE::sat:
		case YTYPE::terrestrial:
		case YTYPE::cable:
		case YTYPE::atsc:
		break;
		default:
			return error("parse_tunersets_xml()", "Error", "These settings are not supported.");
	}

	char type;
	unordered_map<string, int> depth;
	switch (ytype)
	{
		case YTYPE::sat:
			type = 's';
			depth["satellites"] = 0;
			depth["sat"] = 1;
		break;
		case YTYPE::terrestrial:
			type = 't';
			depth["locations"] = 0;
			depth["terrestrial"] = 1;
		break;
		case YTYPE::cable:
			type = 'c';
			depth["cables"] = 0;
			depth["cable"] = 1;
		break;
		case YTYPE::atsc:
			type = 'a';
			depth["locations"] = 0;
			depth["atsc"] = 1;
		break;
	}
	depth["transponder"] = 2;

	int step = 0;
	int bidx = 0;
	int cidx = 0;
	string line;

	while (std::getline(ftunxml, line, '>'))
	{
		//TODO comments non-destructive edit
		if (line.find("<!") != string::npos)
			continue;

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
			step = depth[tag];

			if (step == 1)
			{
				tn = tuner_sets ();
				tn.ytype = ytype;
				tn.flgs = -1, tn.pos = -1, tn.feed = 0;
			}
			else if (step == 2)
			{
				tntxp = tuner_transponder ();
				tntxp.freq = -1, tntxp.sr = -1, tntxp.pol = -1, tntxp.fec = -1, tntxp.mod = -1, tntxp.inv = -1, tntxp.sys = -1, tntxp.rol = -1, tntxp.pil = -1, tntxp.isid = -1, tntxp.plsmode = -1, tntxp.plscode = -1;
				tntxp.band = -1, tntxp.hpfec = -1, tntxp.lpfec = -1, tntxp.tmod = -1, tntxp.tmx = -1, tntxp.guard = -1, tntxp.hier = -1;
				tntxp.cfec = -1, tntxp.cmod = -1;
				tntxp.amod = -1;
			}
		}
		else if (tag.empty())
		{
			continue;
		}
		else
		{
			return error("parse_tunersets_xml()", "Error", "Malformed or unknown XML error.");
		}

		string yey;
		char* token = std::strtok(line.data(), " ");
		while (token != 0)
		{
			string str = string (token);
			string key, val;
			unsigned long pos;
			unsigned long len = str.find('=');

			if (len != string::npos)
			{
				key = str.substr(0, len);
				std::transform(key.begin(), key.end(), key.begin(), [](unsigned char c) { return std::isspace(c) ? '\0' : c; });
				key.erase(0, key.find_first_not_of('\0'));
				pos = key.find_first_of('\0');
				if (pos != string::npos)
					key.erase(pos, key.size());
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
				pos = key.find_first_of('\0');
				if (pos != string::npos)
					key.erase(pos, key.size());
				if (! key.empty())
					yey = str;
			}
			token = std::strtok(NULL, " ");

			switch (ytype)
			{
				case YTYPE::sat:
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
					else if (key == "guard")
						tntxp.guard = std::atoi(val.data());
					else if (key == "hier")
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
						tn.feed = (val == "true" ? true : false);
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
				break;
			}
		}

		if (! add && step == 1)
		{
			bidx++;
			add_tunerset(bidx, tn);
		}
		else if (add && step == 2)
		{
			char tnid[12];
			std::sprintf(tnid, "%c:%04x", type, bidx);
			tuner_sets& tn = tunersets[tnid];
			cidx++;
			add_tunerset_transponder(cidx, tn, tntxp);
		}
	}
}

unordered_map<string, string> e2db_parser::get_input() {
	debug("get_input()");

	return e2db;
}

void e2db_parser::debugger()
{
	debug("debugger()");

	cout << "transponders" << endl << endl;
	for (auto & x: db.transponders)
	{
		cout << "txid: " << x.first << endl;
		cout << hex;
		cout << "dvbns: " << x.second.dvbns << endl;
		cout << "tsid: " << x.second.tsid << endl;
		cout << "onid: " << x.second.onid << endl;
		cout << dec;
		cout << "ttype: " << x.second.ttype << endl;
		cout << "freq: " << x.second.freq << endl;
		cout << "sr: " << x.second.sr << endl;
		cout << "pol: " << x.second.pol << endl;
		cout << "fec: " << x.second.fec << endl;
		cout << "hpfec: " << x.second.hpfec << endl;
		cout << "lpfec: " << x.second.lpfec << endl;
		cout << "cfec: " << x.second.cfec << endl;
		cout << "pos: " << x.second.pos << endl;
		cout << "inv: " << x.second.inv << endl;
		cout << "flgs: " << x.second.flgs << endl;
		cout << "sys: " << x.second.sys << endl;
		cout << "mod: " << x.second.mod << endl;
		cout << "tmod: " << x.second.tmod << endl;
		cout << "cmod: " << x.second.cmod << endl;
		cout << "amod: " << x.second.amod << endl;
		cout << "rol: " << x.second.rol << endl;
		cout << "pil: " << x.second.pil << endl;
		cout << "band: " << x.second.band << endl;
		cout << "tmx: " << x.second.tmx << endl;
		cout << "guard: " << x.second.guard << endl;
		cout << "hier: " << x.second.hier << endl;
		cout << "oflgs: " << x.second.oflgs << endl;
		cout << endl;
	}
	cout << endl;
	cout << "services" << endl << endl;
	for (auto & x: db.services)
	{
		cout << "chid: " << x.first << endl;
		cout << "txid: " << x.second.txid << endl;
		cout << hex;
		cout << "ssid: " << x.second.ssid << endl;
		cout << "dvbns: " << x.second.dvbns << endl;
		cout << "tsid: " << x.second.tsid << endl;
		cout << dec;
		cout << "onid: " << x.second.onid << endl;
		cout << "stype: " << x.second.stype << endl;
		cout << "snum: " << x.second.snum << endl;
		cout << "srcid: " << x.second.srcid << endl;
		cout << "chname: " << x.second.chname << endl;
		cout << "data: [" << endl << endl;
		for (auto & q: x.second.data)
		{
			cout << q.first << ": [" << endl;
			for (string & w: q.second)
				cout << w << ", ";
			cout << endl << "]";
		}
		cout << "]" << endl << endl;
		cout << "index: " << x.second.index << endl;
		cout << endl;
	}
	cout << endl;
	cout << "bouquets" << endl << endl;
	for (auto & x: bouquets)
	{
		cout << "filename: " << x.first << endl;
		cout << "name: " << x.second.name << endl;
		cout << "nname: " << x.second.nname << endl;
		cout << "btype: " << x.second.btype << endl;
		cout << "userbouquets: [" << endl << endl;
		for (auto & w: x.second.userbouquets)
			cout << w << endl;
		cout << "]" << endl << endl;
	}
	cout << endl;
	cout << "userbouquets" << endl << endl;
	for (auto & x: userbouquets)
	{
		cout << "filename: " << x.first << endl;
		cout << "name: " << x.second.name << endl;
		cout << "channels: [" << endl << endl;
		for (auto & q: x.second.channels)
		{
			cout << "chid: " << q.first << endl;
			cout << "index: " << q.second.index << endl;
		}
		cout << "]" << endl << endl;
	}
	cout << endl;
	cout << "tunersets" << endl << endl;
	for (auto & x: tunersets)
	{
		cout << "tnid: " << x.second.tnid << endl;
		cout << "ytype: " << x.second.ytype << endl;
		cout << "name: " << x.second.name << endl;
		cout << "flags: " << x.second.flgs << endl;
		cout << "pos: " << x.second.pos << endl;
		cout << "index: " << x.second.index << endl;
		cout << "transponders: [" << endl << endl;
		for (auto & q: x.second.transponders)
		{
			cout << "trid: " << q.first << endl;
			cout << "freq: " << q.second.freq << endl;
			cout << "sr: " << q.second.sr << endl;
			cout << "pol: " << q.second.pol << endl;
			cout << "fec: " << q.second.fec << endl;
			cout << "hpfec: " << q.second.hpfec << endl;
			cout << "lpfec: " << q.second.lpfec << endl;
			cout << "cfec: " << q.second.cfec << endl;
			cout << "inv: " << q.second.inv << endl;
			cout << "sys: " << q.second.sys << endl;
			cout << "mod: " << q.second.mod << endl;
			cout << "tmod: " << q.second.tmod << endl;
			cout << "cmod: " << q.second.cmod << endl;
			cout << "amod: " << q.second.amod << endl;
			cout << "band: " << q.second.band << endl;
			cout << "tmx: " << q.second.tmx << endl;
			cout << "guard: " << q.second.guard << endl;
			cout << "hier: " << q.second.hier << endl;
			cout << "rol: " << q.second.rol << endl;
			cout << "pil: " << q.second.pil << endl;
			cout << "isid: " << q.second.isid << endl;
			cout << "plsmode: " << q.second.plsmode << endl;
			cout << "plscode: " << q.second.plscode << endl;
			cout << "index: " << q.second.index << endl;
			cout << endl;
		}
		cout << "]" << endl << endl;
	}
	cout << endl;
}

unordered_map<string, e2db_parser::transponder> e2db_parser::get_transponders()
{
	debug("get_transponders()");
	return db.transponders;
}

unordered_map<string, e2db_parser::service> e2db_parser::get_services()
{
	debug("get_services()");
	return db.services;
}

pair<unordered_map<string, e2db_parser::bouquet>, unordered_map<string, e2db_parser::userbouquet>> e2db_parser::get_bouquets()
{
	debug("get_bouquets()");
	return pair (bouquets, userbouquets); //C++17
}

bool e2db_parser::list_localdir(string localdir)
{
	debug("list_localdir()", "localdir", localdir);

	if (! std::filesystem::exists(localdir)) //C++17
	{
		error("list_localdir()", "Error", "Directory \"" + localdir + "\" not exists.");
		return false;
	}

	std::filesystem::directory_iterator dirlist(localdir); //C++17

	for (const auto & entry : dirlist)
	{
		//TODO is file & permissions check ...
		string path = entry.path().u8string(); //C++17
		string filename = std::filesystem::path(path).filename().u8string(); //C++17
		e2db[filename] = path;
	}
	if (e2db.count(dbfilename) < 1)
	{
		error("list_localdir()", "Error", "lamedb not found.");
		return false;
	}
	this->localdir = localdir;

	return true;
}

bool e2db_parser::read(string localdir)
{
	debug("read()", "localdir", localdir);

	if (list_localdir(localdir))
		parse_e2db();
	else
		return false;

	return true;
}



e2db_maker::e2db_maker()
{
	this->log = new e2se::logger("e2db");

	debug("e2db_maker()");
}

void e2db_maker::make_e2db()
{
	debug("make_e2db()");

	begin_transaction();
	make_e2db_lamedb();
	make_e2db_bouquets();
	make_e2db_userbouquets();
	end_transaction();
}

void e2db_maker::begin_transaction()
{
	debug("begin_transaction()");

	std::time_t curr_tst = std::time(0);
	std::tm* _out_tst = std::localtime(&curr_tst);
	this->_out_tst = _out_tst;
}

void e2db_maker::end_transaction()
{
	debug("end_transaction()");
}

//TODO FIX mingw32 wrong %z %Z
string e2db_maker::get_timestamp()
{
	debug("get_timestamp()");

	char datetime[80];
	// @link https://sourceforge.net/p/mingw-w64/bugs/793/
	std::strftime(datetime, 80, "%Y-%m-%d %H:%M:%S %z", _out_tst);
	return string (datetime);
}

string e2db_maker::get_editor_string()
{
	return "e2-sat-editor 0.1 <https://github.com/ctlcltd/e2-sat-editor>";
}

void e2db_maker::make_e2db_lamedb()
{
	debug("make_e2db_lamedb()");

	make_e2db_lamedb4();

	//TEST
	if (MAKER_LAMEDB5)
		make_e2db_lamedb5();
	//TEST
}

void e2db_maker::make_e2db_lamedb4()
{
	debug("make_e2db_lamedb4()");
	LAMEDB_VER = 4;
	make_lamedb("lamedb");
}

void e2db_maker::make_e2db_lamedb5()
{
	debug("make_e2db_lamedb5()");
	LAMEDB_VER = 5;
	make_lamedb("lamedb5");
}

void e2db_maker::make_lamedb(string filename)
{
	debug("make_lamedb()");

	// formatting
	//
	// [0]  comment
	// [1]  transponders start
	// [2]  services start
	// [3]  section end
	// [4]  delimiter
	// [5]  transponder flag
	// [6]  transponder params separator
	// [7]  transponder space delimiter
	// [8]  transponder endline
	// [9]  service flag
	// [10] service params separator
	// [11] service param escape
	// [12] service endline
	const string (&formats)[13] = LAMEDB_VER < 5 ? LAMEDB4_FORMATS : LAMEDB5_FORMATS;

	stringstream ss;
	ss << "eDVB services /" << LAMEDB_VER << "/" << endl;

	ss << formats[1];
	for (auto & x: index["txs"])
	{
		transponder tx = db.transponders[x.second];
		ss << formats[5] << formats[4];
		ss << hex;
		ss << setfill('0') << setw(8) << tx.dvbns;
		ss << ':' << setfill('0') << setw(4) << tx.tsid;
		ss << ':' << setfill('0') << setw(4) << tx.onid;
		ss << dec;
		ss << formats[6];
		ss << tx.ttype << formats[7];
		switch (tx.ttype)
		{
			case 's': // DVB-S
				ss << int (tx.freq * 1e3);
				ss << ':' << int (tx.sr * 1e3);
				ss << ':' << tx.pol;
				ss << ':' << tx.fec;
				ss << ':' << tx.pos;
				ss << ':' << tx.inv;
				ss << ':' << tx.flgs;
				if (tx.sys != -1)
					ss << ':' << tx.sys;
				if (tx.mod != -1)
					ss << ':' << tx.mod;
				if (tx.rol != -1)
					ss << ':' << tx.rol;
				if (tx.pil != -1)
					ss << ':' << tx.pil;
			break;
			case 't': // DVB-T
				ss << int (tx.freq * 1e3);
				ss << ':' << tx.band;
				ss << ':' << tx.hpfec;
				ss << ':' << tx.lpfec;
				ss << ':' << tx.tmod;
				ss << ':' << tx.tmx;
				ss << ':' << tx.guard;
				ss << ':' << tx.hier;
				ss << ':' << tx.inv;
				if (! tx.oflgs.empty())
					ss << tx.oflgs;
			break;
			//TODO
			case 'c': // DVB-C
				ss << int (tx.freq * 1e3);
				ss << ':' << int (tx.sr * 1e3);
				ss << ':' << tx.inv;
				ss << ':' << tx.cmod;
				ss << ':' << tx.cfec;
				if (! tx.oflgs.empty())
					ss << tx.oflgs;
			break;
			//TODO
			case 'a': // ATSC
				ss << int (tx.freq * 1e3);
				ss << ':' << tx.inv;
				ss << ':' << tx.amod;
				ss << ':' << tx.flgs;
				ss << ':' << tx.sys;
				if (! tx.oflgs.empty())
					ss << tx.oflgs;
			break;
		}
		ss << formats[8];
	}
	ss << formats[3];

	ss << formats[2];
	for (auto & x: index["chs"])
	{
		service ch = db.services[x.second];
		ss << formats[9] << formats[4];
		ss << hex;
		ss << setfill('0') << setw(4) << ch.ssid;
		ss << ':' << setfill('0') << setw(8) << ch.dvbns;
		ss << ':' << setfill('0') << setw(4) << ch.tsid;
		ss << ':' << setfill('0') << setw(4) << ch.onid;
		ss << dec;
		ss << ':' << ch.stype;
		ss << ':' << ch.snum;
		if (LAMEDB_VER == 5)
			ss << ':' << ch.srcid;
		ss << formats[10];
		ss << formats[11] << ch.chname << formats[11];
		ss << formats[10];
		//TODO 256
		auto last_key = (*prev(ch.data.cend()));
		for (auto & q: ch.data)
		{
			char d;
			switch (q.first)
			{
				case e2db::SDATA::p: d = 'p'; break;
				case e2db::SDATA::c: d = 'c'; break;
				case e2db::SDATA::C: d = 'C'; break;
				case e2db::SDATA::f: d = 'f'; break;
				default: d = q.first;
			}
			for (unsigned int i = 0; i < q.second.size(); i++)
			{
				ss << d << ':' << q.second[i];
				if (! q.second[i].empty() && (i != q.second.size() - 1 || q.first != last_key.first))
					ss << ',';
			}
		}
		ss << formats[12];
	}
	ss << formats[3];

	ss << formats[0] << "editor: " << get_editor_string() << endl;
	ss << formats[0] << "datetime: " << get_timestamp() << endl;
	e2db_out[filename] = ss.str();
}

void e2db_maker::make_e2db_bouquets()
{
	debug("make_e2db_bouquets()");

	for (auto & x: bouquets)
		make_bouquet(x.first);
}

void e2db_maker::make_e2db_userbouquets()
{
	debug("make_e2db_userbouquets()");

	for (auto & x: userbouquets)
		make_userbouquet(x.first);
}

void e2db_maker::make_bouquet(string bname)
{
	debug("make_bouquet()", "bname", bname);

	bouquet bs = bouquets[bname];
	stringstream ss;

	ss << "#NAME " << bs.name << endl;
	for (auto & w: bs.userbouquets)
	{
		ss << "#SERVICE ";
		ss << "1:7:" << bs.btype << ":0:0:0:0:0:0:0:";
		ss << "FROM BOUQUET ";
		ss << "\"" << w << "\" ";
		ss << "ORDER BY bouquet";
		ss << endl;
	}
	// ss << endl;
	e2db_out[bname] = ss.str();
}

//TODO upCase or loCase
void e2db_maker::make_userbouquet(string bname)
{
	debug("make_userbouquet()", "bname", bname);

	userbouquet ub = userbouquets[bname];
	stringstream ss;

	ss << "#NAME " << ub.name << endl;
	for (auto & x: index[bname])
	{
		channel_reference chref = userbouquets[bname].channels[x.second];
		ss << "#SERVICE ";
		ss << "1:";
		ss << chref.type << ':';
		ss << hex;
		ss << uppercase << chref.anum << ':'; //TODO ("global markers index)
		
		if (db.services.count(x.second))
		{
			service ch = db.services[x.second];
			string onid = ch.onid.empty() ? "0" : ch.onid;
			std::transform(onid.begin(), onid.end(), onid.begin(), [](unsigned char c) { return toupper(c); });

			ss << uppercase << ch.ssid << ':';
			ss << uppercase << ch.tsid << ':';
			ss << onid << ':';
			ss << uppercase << ch.dvbns << ':';
			ss << "0:0:0:";
		}
		else
		{
			if (chref.marker)
			{
				ss << "0:0:0:0:0:0:0:0:" << endl;
				ss << "#DESCRIPTION " << chref.value;
			}
			else
			{
				error("make_userbouquet()", "Error", "Missing channel_reference \"" + x.second + "\".");
			}
		}
		ss << dec;
		ss << endl;
	}
	// ss << endl;
	e2db_out[bname] = ss.str();
}

bool e2db_maker::write_to_localdir(string localdir, bool overwrite)
{
	debug("write_to_localdir()", "localdir", localdir);

	if (! std::filesystem::is_directory(localdir)) //C++17
	{
		error("write_to_localdir()", "Error", "Directory \"" + localdir + "\" not exists.");
		return false;
	}
	//TODO file exists and (force) overwrite
	else if (! overwrite)
	{
		std::filesystem::create_directory(localdir); //C++17
	}
	//TODO permission check ...
	for (auto & o: e2db_out)
	{
		string localfile = localdir + '/' + o.first;

		ofstream out (localfile);
		out << o.second;
		out.close();
	}

	return true;
}

bool e2db_maker::write(string localdir, bool overwrite)
{
	debug("write()", "localdir", localdir);

	make_e2db();

	if (write_to_localdir(localdir, overwrite))
		return true;
	else
		return false;
}

unordered_map<string, e2db_file> e2db_maker::get_output() {
	debug("get_output()");
	
	make_e2db();

	return e2db_out;
}

void e2db_maker::set_index(unordered_map<string, vector<pair<int, string>>> index)
{
	debug("set_index()");
	this->index = index;
}

void e2db_maker::set_transponders(unordered_map<string, e2db_maker::transponder> transponders)
{
	debug("set_transponders()");
	db.transponders = transponders;
}

void e2db_maker::set_channels(unordered_map<string, e2db_maker::service> services)
{
	debug("set_channels()");
	db.services = services;
}

void e2db_maker::set_bouquets(pair<unordered_map<string, e2db_maker::bouquet>, unordered_map<string, e2db_maker::userbouquet>> bouquets)
{
	debug("set_bouquets()");
	this->bouquets = bouquets.first;
	this->userbouquets = bouquets.second;
}



e2db::e2db()
{
	this->log = new e2se::logger("e2db");

	debug("e2db()");
}

//TODO bname destructive edit
//TODO bname non-destructive edit
void e2db::merge(e2db* dbih)
{
	debug("merge()");

	db.transponders.merge(dbih->db.transponders); //C++17
	db.services.merge(dbih->db.services); //C++17
	tunersets.merge(dbih->tunersets); //C++17
	bouquets.merge(dbih->bouquets); //C++17
	collisions.merge(dbih->collisions); //C++17

	unordered_map<string, vector<pair<int, string>>> index;
	unordered_map<string, vector<pair<int, string>>> cp_index_0 = this->index;
	unordered_map<string, vector<pair<int, string>>> cp_index_1 = dbih->index;
	unordered_map<string, userbouquet> cp_ubs_0;
	unordered_map<string, userbouquet> cp_ubs_1;
	cp_index_0.erase("mks");
	cp_index_1.erase("mks");

	//TODO refresh cached data
	/*for (auto & chdata : db.services)
	{
	}*/

	for (auto & i : cp_index_0["ubs"])
	{
		userbouquet& ub = userbouquets[i.second];
		bouquet& bs = bouquets[ub.pname];
		string key = "1:7:" + to_string(bs.btype) + ':' + ub.name;
		auto iub = cp_index_0[i.second];
		cp_index_0[key] = iub;
		cp_index_0.erase(i.second);
		cp_ubs_0[key] = ub;
		// cout << "cp_index_0 " << i.second << ' ' << cp_index_0[key].size() << ' ' << key << endl;
		i.second = key;
	}
	for (auto & i : cp_index_1["ubs"])
	{
		userbouquet& ub = dbih->userbouquets[i.second];
		bouquet& bs = dbih->bouquets[ub.pname];
		string key = "1:7:" + to_string(bs.btype) + ':' + ub.name;
		auto iub = cp_index_1[i.second];
		cp_index_1[key] = iub;
		cp_index_1.erase(i.second);
		cp_ubs_1[key] = ub;
		// cout << "cp_index_1 " << i.second << ' ' << cp_index_1[key].size() << ' ' << key << endl;
		i.second = key;
	}

	index = cp_index_1;
	for (auto & i : cp_index_0)
	{
		vector<pair<int, string>> i_diff;
		set_difference(i.second.begin(), i.second.end(), cp_index_1[i.first].begin(), cp_index_1[i.first].end(), inserter(i_diff, i_diff.begin()));
		index[i.first].insert(index[i.first].end(), i_diff.begin(), i_diff.end());
		// cout << "i_diff " << i.first << ' ' << i_diff.size() << endl;
	}

	cp_ubs_0.merge(cp_ubs_1);
	userbouquets.clear();

	for (auto & bsdata : bouquets)
	{
		bsdata.second.userbouquets.clear();
	}
	for (auto & i : index["ubs"])
	{
		userbouquet& ub = cp_ubs_0[i.second];
		bouquet& bs = bouquets[ub.pname];
		//TODO "userbouquet.dbe.01234.tv"
		int idx = bs.userbouquets.size();
		string key = "1:7:" + to_string(bs.btype) + ':' + ub.name;
		string ktype;
		if (bs.btype == 1)
			ktype = "tv";
		else if (bs.btype == 2)
			ktype = "radio";
		//cout << ktype << ' ' << i.second << endl;

		stringstream bname;
		bname << "userbouquet.dbe" << setfill('0') << setw(2) << idx << '.' << ktype;

		ub.bname = bname.str();
		ub.index = idx;

		if (cp_ubs_1.count(i.second))
			ub.channels.merge(cp_ubs_1[i.second].channels);

		index[ub.bname] = index[key];
		index.erase(key);

		idx = 0;
		for (auto & x : index[ub.bname])
		{
			channel_reference& chref = ub.channels[x.second];

			if (chref.marker)
			{
				char chid[25];
				std::sprintf(chid, "%d:%x:%d", chref.type, chref.anum, ub.index);
				chref.chid = chid;
				index["mks"].emplace_back(pair (ub.index, chid)); //C++17
			}
			else
			{
				idx += 1;
				chref.index = idx;
				x.first = chref.index;
			}
		}

		bs.userbouquets.emplace_back(ub.bname);
		userbouquets[ub.bname] = ub;
		i.first = ub.index;
		i.second = ub.bname;
		// cout << "index " << ub.bname << ' ' << index[key].size() << ' ' << key << endl;
	}

	index.erase("chs:0");
	index.erase("chs:1");
	index.erase("chs:2");
	for (auto & i : index)
	{
		if (i.first == "txs" || i.first == "chs" || i.first.find("bouquets.") != string::npos)
		{
			int idx = 0;
			for (auto & x : i.second)
			{
				idx += 1;
				x.first = idx;
			}
			// cout << i.first << ' ' << i.second.size() << endl;
		}
	}
	for (auto & i : index["chs"])
	{
		service& ch = db.services[i.second];
		string iname = "chs:" + (STYPES.count(ch.stype) ? to_string(STYPES.at(ch.stype).first) : "0");
		index[iname].emplace_back(pair (i.first, ch.chid)); //C++17
	}

	this->index = index;

	//TODO mem
	cp_index_0.clear();
	cp_index_1.clear();
	cp_ubs_0.clear();
	cp_ubs_1.clear();
	index.clear();
}

void e2db::import_file()
{
	debug("import_file()");
}

void e2db::export_file()
{
	debug("export_file()");
}

void e2db::add_transponder(transponder& tx)
{
	debug("add_transponder()", "txid", tx.txid);

	tx.index = index.count("txs");
	e2db_abstract::add_transponder(tx.index, tx);
}

void e2db::edit_transponder(string txid, transponder& tx)
{
	debug("edit_transponder()", "txid", txid);

	char nw_txid[25];
	std::sprintf(nw_txid, "%x:%x", tx.tsid, tx.dvbns);
	tx.txid = nw_txid;

	debug("edit_service()", "nw_txid", tx.txid);

	if (tx.txid == txid)
	{
		db.transponders[tx.txid] = tx;
	}
	else
	{
		//TODO swap
		db.transponders.erase(txid);
		db.transponders.emplace(tx.txid, tx);

		for (auto it = index["txs"].begin(); it != index["txs"].end(); it++)
		{
			if (it->second == txid)
				it->second = tx.txid;
		}
	}
}

void e2db::remove_transponder(string txid)
{
	debug("remove_transponder()", "txid", txid);

	db.transponders.erase(txid);

	for (auto it = index["txs"].begin(); it != index["txs"].end(); it++)
	{
		if (it->second == txid)
			index["txs"].erase(it);
	}
}

void e2db::add_service(service& ch)
{
	debug("add_service()", "chid", ch.chid);

	ch.index = index.count("chs");
	e2db_abstract::add_service(ch.index, ch);
}

void e2db::edit_service(string chid, service& ch)
{
	debug("edit_service()", "chid", chid);

	if (! db.services.count(chid))
		return error("edit_service()", "Error", "Service \"" + chid + "\" not exists.");

	char nw_chid[25];
	char nw_txid[25];
	std::sprintf(nw_txid, "%x:%x", ch.tsid, ch.dvbns);
	std::sprintf(nw_chid, "%x:%x:%x", ch.ssid, ch.tsid, ch.dvbns);
	ch.txid = nw_txid;
	ch.chid = nw_chid;

	debug("edit_service()", "nw_chid", ch.chid);

	if (ch.chid == chid)
	{
		db.services[ch.chid] = ch;
	}
	else
	{
		string kchid = 's' + chid;
		collisions.erase(kchid);

		if (db.services.count(ch.chid))
		{
			int m;
			string kchid = 's' + ch.chid;
			if (ch.snum) m = ch.snum;
			else m = collisions[kchid].size();
			ch.chid += ':' + to_string(m);
			collisions[kchid].emplace_back(pair (ch.chid, m)); //C++17
		}

		//TODO swap
		db.services.erase(chid);
		db.services.emplace(ch.chid, ch);

		for (auto & x : index)
		{
			for (auto it = x.second.begin(); it != x.second.end(); it++)
			{
				if (it->second == chid)
					it->second = ch.chid;
			}
		}
		for (auto & x : userbouquets)
		{
			//TODO swap
			if (x.second.channels.count(chid))
			{
				channel_reference chref = x.second.channels[chid];
				chref.chid = ch.chid;
				x.second.channels.erase(chid);
				x.second.channels.emplace(ch.chid, chref);
			}
		}
	}
}

void e2db::remove_service(string chid)
{
	debug("remove_service()", "chid", chid);

	if (! db.services.count(chid))
		return error("edit_service()", "Error", "Service \"" + chid + "\" not exists.");

	service ch = db.services[chid];
	string kchid = 's' + chid;
	db.services.erase(chid);

	for (auto & x : index)
	{
		for (auto it = x.second.begin(); it != x.second.end(); it++)
		{
			if (it->second == chid)
				x.second.erase(it);
		}
	}
	for (auto & x : userbouquets)
	{
		x.second.channels.erase(chid);
	}
	collisions.erase(kchid);
}

void e2db::add_bouquet(bouquet& bs)
{
	debug("add_bouquet()", "bname", bs.bname);

	e2db_abstract::add_bouquet(bs.index, bs);
}

void e2db::edit_bouquet(bouquet& bs)
{
	debug("edit_bouquet()", "bname", bs.bname);

	if (! bouquets.count(bs.bname))
		return error("edit_bouquet()", "Error", "Bouquet \"" + bs.bname + "\" not exists.");

	bouquets[bs.bname] = bs;
}

void e2db::remove_bouquet(string bname)
{
	debug("remove_bouquet()", "bname", bname);

	if (! bouquets.count(bname))
		return error("remove_bouquet()", "Error", "Bouquet \"" + bname + "\" not exists.");

	for (auto it = index["bss"].begin(); it != index["bss"].end(); it++)
	{
		if (it->second == bname)
			index["bss"].erase(it);
	}
	for (auto it = index["ubs"].begin(); it != index["ubs"].end(); it++)
	{
		userbouquet ub = userbouquets[it->second];
		if (ub.pname == bname)
		{
			index["ubs"].erase(it);
			index.erase(ub.bname);
			userbouquets.erase(ub.bname);
		}
	}
	index.erase(bname);
	bouquets.erase(bname);
}

//TODO bname destructive edit
void e2db::add_userbouquet(userbouquet& ub)
{
	debug("add_userbouquet()");

	bouquet bs = bouquets[ub.pname];

	if (ub.index < 0)
	{
		int idx = 0;
		string ktype;
		if (bs.btype == 1)
			ktype = "tv";
		else if (bs.btype == 2)
			ktype = "radio";

		for (auto it = index["ubs"].begin(); it != index["ubs"].end(); it++)
		{
			unsigned long pos0 = it->second.find(".dbe");
			unsigned long pos1 = it->second.find('.' + ktype);
			int len = it->second.length();
			int n = 0;
			if (pos0 != string::npos && pos1 != string::npos)
			{
				n = atoi(it->second.substr(pos0 + 4, len - pos1 - 1).data());
				idx = n > idx ? n : idx;
			}
		}

		idx = idx ? idx + 1 : 0;
		ub.index = idx;
	}
	if (ub.bname.empty())
	{
		stringstream bname;
		string ktype;
		if (bs.btype == 1)
			ktype = "tv";
		else if (bs.btype == 2)
			ktype = "radio";

		bname << "userbouquet.dbe" << setfill('0') << setw(2) << ub.index << '.' << ktype;
		ub.bname = bname.str();
	}

	e2db_abstract::add_userbouquet(ub.index, ub);
}

void e2db::edit_userbouquet(userbouquet& ub)
{
	debug("edit_userbouquet()", "bname", ub.bname);

	if (! userbouquets.count(ub.bname))
		return error("edit_userbouquet()", "Error", "Userbouquet \"" + ub.bname + "\" not exists.");

	userbouquets[ub.bname] = ub;
}

void e2db::remove_userbouquet(string bname)
{
	debug("remove_userbouquet()", "bname", bname);

	if (! userbouquets.count(bname))
		return error("remove_userbouquet()", "Error", "Userbouquet \"" + bname + "\" not exists.");

	userbouquet ub = userbouquets[bname];
	bouquet& bs = bouquets[ub.pname];

	for (auto it = index["ubs"].begin(); it != index["ubs"].end(); it++)
	{
		if (it->second == bname)
			index["ubs"].erase(it);
	}

	index.erase(ub.pname);

	vector<string>::iterator pos;
	for (auto it = bs.userbouquets.begin(); it != bs.userbouquets.end(); it++)
	{
		if (*it == bname)
		{
			pos = it;
			break;
		}
	}
	if (pos != bs.userbouquets.end())
	{
		bs.userbouquets.erase(pos);
	}
	for (auto & w : bs.userbouquets)
	{
		for (auto & x : userbouquets[w].channels)
		{
			if (! x.second.marker)
				index[ub.pname].emplace_back(pair ((index[ub.pname].size() + 1), x.first)); //C++17
		}
	}
	index.erase(bname);
	userbouquets.erase(bname);
}

void e2db::add_channel_reference(channel_reference& chref, string bname)
{
	debug("add_channel_reference()", "chid", chref.chid);

	if (! db.services.count(chref.chid))
		return error("add_channel_reference()", "Error", "Service \"" + chref.chid + "\" not exists.");
	if (! userbouquets.count(bname))
		return error("add_channel_reference()", "Error", "Userbouquet \"" + bname + "\" not exists.");

	service ch = db.services[chref.chid];
	userbouquet& ub = userbouquets[bname];
	service_reference ref;

	if (! chref.marker)
	{
		ref.ssid = ch.ssid;
		ref.dvbns = ch.dvbns;
		ref.tsid = ch.tsid;
	}

	e2db_abstract::add_channel_reference(chref.index, ub, chref, ref);
}

void e2db::edit_channel_reference(string chid, channel_reference& chref, string bname)
{
	debug("edit_channel_reference()", "chid", chid);

	if (! db.services.count(chid))
		return error("edit_channel_reference()", "Error", "Service \"" + chid + "\" not exists.");
	if (! userbouquets.count(bname))
		return error("edit_channel_reference()", "Error", "Userbouquet \"" + bname + "\" not exists.");

	service ch = db.services[chref.chid];
	userbouquet& ub = userbouquets[bname];

	char nw_chid[25];
	char nw_txid[25];
	std::sprintf(nw_txid, "%x:%x", ch.tsid, ch.dvbns);
	std::sprintf(nw_chid, "%x:%x:%x", ch.ssid, ch.tsid, ch.dvbns);
	ch.txid = nw_txid;
	ch.chid = nw_chid;

	debug("edit_channel_reference()", "nw_chid", ch.chid);

	if (ch.chid == chid)
	{
		ub.channels[ch.chid] = chref;
	}
	else
	{
		service_reference ref;

		if (! chref.marker)
		{
			ref.ssid = ch.ssid;
			ref.dvbns = ch.dvbns;
			ref.tsid = ch.tsid;
		}

		//TODO swap
		ub.channels.erase(chid);
		ub.channels.emplace(ch.chid, chref);

		for (auto it = index[bname].begin(); it != index[bname].end(); it++)
		{
			if (it->second == chid)
				it->second = ch.chid;
		}
		if (chref.marker)
		{
			for (auto it = index["mks"].begin(); it != index["mks"].end(); it++)
			{
				if (it->second == chid)
					it->second = ch.chid;
			}
		}
		else
		{
			for (auto it = index[ub.pname].begin(); it != index[ub.pname].end(); it++)
			{
				if (it->second == chid)
					it->second = ch.chid;
			}
		}
	}
}

void e2db::remove_channel_reference(string chid, string bname)
{
	debug("remove_channel_reference()", "chid", chid);

	if (! db.services.count(chid))
		return error("edit_channel_reference()", "Error", "Service \"" + chid + "\" not exists.");
	if (! userbouquets.count(bname))
		return error("edit_channel_reference()", "Error", "Userbouquet \"" + bname + "\" not exists.");
	if (! userbouquets[bname].channels.count(chid))
		return error("edit_channel_reference()", "Error", "Channel reference \"" + chid + "\" not exists.");

	channel_reference chref = userbouquets[bname].channels[chid];
	userbouquet& ub = userbouquets[bname];

	/*for (auto it = index[bname].begin(); it != index[bname].end(); it++)
	{
		if (it->second == chid)
			index[bname].erase(it);
	}
	if (chref.marker)
	{
		for (auto it = index["mks"].begin(); it != index["mks"].end(); it++)
		{
			if (it->second == chid)
				index["mks"].erase(it);
		}
	}
	else
	{
		for (auto it = index[ub.pname].begin(); it != index[ub.pname].end(); it++)
		{
			if (it->second == chid)
				index[ub.pname].erase(it);
		}
	}*/
	ub.channels.erase(chid);
}

string e2db::get_reference_id(string chid)
{
	// debug("get_reference_id()", "chid", chid);

	char refid[33];
	int stype, snum, ssid, tsid, dvbns;
	stype = 0, snum = 0, ssid = 0, tsid = 0, dvbns = 0;
	string onid = "0";

	if (db.services.count(chid))
	{
		service ch = db.services[chid];
		stype = ch.stype;
		snum = ch.snum;
		ssid = ch.ssid;
		tsid = ch.tsid;
		onid = ch.onid.empty() ? onid : ch.onid;
		std::transform(onid.begin(), onid.end(), onid.begin(), [](unsigned char c) { return toupper(c); });
		dvbns = ch.dvbns;
	}

	std::sprintf(refid, "%d:%d:%X:%X:%X:%s:%X:0:0:0", 1, stype, snum, ssid, tsid, onid.c_str(), dvbns);
	return refid;
}

string e2db::get_reference_id(channel_reference chref)
{
	// debug("get_reference_id()", "chref.chid", chref.chid);

	char refid[33];
	int ssid, tsid, dvbns;
	ssid = 0, tsid = 0, dvbns = 0;
	string onid = "0";

	if (! chref.marker && db.services.count(chref.chid))
	{
		service ch = db.services[chref.chid];
		ssid = ch.ssid;
		tsid = ch.tsid;
		onid = ch.onid.empty() ? onid : ch.onid;
		std::transform(onid.begin(), onid.end(), onid.begin(), [](unsigned char c) { return toupper(c); });
		dvbns = ch.dvbns;
	}

	std::sprintf(refid, "%d:%d:%X:%X:%X:%s:%X:0:0:0", 1, chref.type, chref.anum, ssid, tsid, onid.c_str(), dvbns);
	return refid;
}

//TODO unique (eg. terrestrial MUX)
map<string, vector<pair<int, string>>> e2db::get_channels_index()
{
	debug("get_channels_index()");

	map<string, vector<pair<int, string>>> _index;

	for (auto & x: index["chs"])
	{
		service ch = db.services[x.second];
		transponder tx = db.transponders[ch.txid];
		_index[to_string(tx.pos)].emplace_back(x);
		_index[ch.txid].emplace_back(x);
	}

	return _index;
}

map<string, vector<pair<int, string>>> e2db::get_transponders_index()
{
	debug("get_transponders_index()");

	map<string, vector<pair<int, string>>> _index;

	for (auto & x: index["txs"])
	{
		transponder tx = db.transponders[x.second];
		_index[to_string(tx.pos)].emplace_back(x);
	}

	return _index;
}

map<string, vector<pair<int, string>>> e2db::get_services_index()
{
	debug("get_services_index()");

	map<string, vector<pair<int, string>>> _index;
	_index["chs"] = index["chs"];

	return _index;
}

map<string, vector<pair<int, string>>> e2db::get_bouquets_index()
{
	debug("get_bouquets_index()");

	map<string, vector<pair<int, string>>> _index;

	for (auto & x: bouquets)
		_index[x.first] = index[x.first];

	return _index;
}

map<string, vector<pair<int, string>>> e2db::get_userbouquets_index()
{
	debug("get_userbouquets_index()");

	map<string, vector<pair<int, string>>> _index;

	for (auto & x: userbouquets)
		_index[x.first] = index[x.first];

	return _index;
}

map<string, vector<pair<int, string>>> e2db::get_packages_index()
{
	debug("get_packages_index()");

	map<string, vector<pair<int, string>>> _index;

	for (auto & x: index["chs"])
	{
		service ch = db.services[x.second];

		if (ch.data.count(SDATA::p))
		{
			string pvdrn = ch.data[SDATA::p][0];

			if (pvdrn.empty()) _index["(Unknown)"].emplace_back(x);
			else _index[pvdrn].emplace_back(x);
		}
	}

	return _index;
}

map<string, vector<pair<int, string>>> e2db::get_resolution_index()
{
	debug("get_resolution_index()");

	map<string, vector<pair<int, string>>> _index;

	for (auto & x: index["chs"])
	{
		service ch = db.services[x.second];
		_index[to_string(ch.stype)].emplace_back(x);
	}

	return _index;
}

map<string, vector<pair<int, string>>> e2db::get_encryption_index()
{
	debug("get_encryption_index()");

	map<string, vector<pair<int, string>>> _index;
	unordered_set<string> _unique;

	for (auto & x: index["chs"])
	{
		service ch = db.services[x.second];

		if (ch.data.count(SDATA::C))
		{
			for (string & w : ch.data[SDATA::C])
			{
				string caidpx = w.substr(0, 2);
				string cx = caidpx + '|' + to_string(x.first);
				if (SDATA_CAS.count(caidpx) && ! _unique.count(cx))
				{
					_index[SDATA_CAS.at(caidpx)].emplace_back(x);
					_unique.insert(cx);
				}
			}
		}
		else
		{
			_index["(FTA)"].emplace_back(x);
		}
	}

	return _index;
}

map<string, vector<pair<int, string>>> e2db::get_az_index()
{
	debug("get_az_index()");

	map<string, vector<pair<int, string>>> _index;

	for (auto & x: index["chs"])
	{
		service ch = db.services[x.second];
		string lt = ch.chname.substr(0, 1);

		if (isdigit(lt[0])) _index["0-9"].emplace_back(x);
		else _index[lt].emplace_back(x);
	}

	return _index;
}

}
