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

#include <unordered_set>
#include <algorithm>
#include <filesystem>
#include <cstdio>
#include <cstring>
#include "e2db.h"

using namespace std;

namespace e2db
{

void e2db_abstract::debug(string ns, string cmsg, string optk, string optv, string indt)
{
	if (! DEBUG) return;
	cout << '[' << "e2db." << ns << ']';
	if (! cmsg.empty()) cout << indt << cmsg;
	if (! optk.empty()) cout << indt << optk << ':';
	if (! optv.empty()) cout << ' ' << optv;
	cout << endl;
}

void e2db_abstract::error(string ns, string cmsg, string optk, string optv, string indt)
{
	cout << '[' << "e2se." << ns << ']';
	if (! cmsg.empty()) cout << indt << cmsg;
	if (! optk.empty()) cout << indt << optk << ':';
	if (! optv.empty()) cout << ' ' << optv;
	cout << endl;
}

string e2db_abstract::lowCase(string str)
{
	transform(str.begin(), str.end(), str.begin(), [](unsigned char c) { return tolower(c); });
	return str;
}

string e2db_abstract::upCase(string str)
{
	transform(str.begin(), str.end(), str.begin(), [](unsigned char c) { return toupper(c); });
	return str;
}

void e2db_abstract::add_transponder(int idx, transponder& tx)
{
	tx.index = idx;
	db.transponders.emplace(tx.txid, tx);
	index["txs"].emplace_back(pair (idx, tx.txid)); //C++ 17
}

void e2db_abstract::add_service(int idx, service& ch)
{
	if (db.services.count(ch.chid))
	{
		int m;
		string kchid = 's' + ch.chid;
		if (ch.snum) m = ch.snum;
		else m = collisions[kchid].size();
		ch.chid += ':' + to_string(m);
		collisions[kchid].emplace_back(pair (ch.chid, m)); //C++ 17
	}

	string iname = "chs:" + (STYPES.count(ch.stype) ? to_string(STYPES.at(ch.stype).first) : "0");
	ch.index = idx;
	db.services.emplace(ch.chid, ch);
	index["chs"].emplace_back(pair (idx, ch.chid)); //C++ 17
	index[iname].emplace_back(pair (idx, ch.chid)); //C++ 17
}



e2db_parser::e2db_parser()
{
	debug("e2db_parser");

	//TODO
	dbfilename = PARSER_LAMEDB5_PRIOR ? "lamedb5" : "lamedb";
}

void e2db_parser::parse_e2db()
{
	debug("e2db_parser", "parse_e2db()");
	clock_t start = clock();

	ifstream flamedb (e2db[dbfilename]);
	parse_e2db_lamedb(flamedb);
	flamedb.close();

	if (PARSER_TUNERSETS && e2db.count("satellites.xml"))
	{
		ifstream ftunxml (e2db["satellites.xml"]);
		parse_tunersets_xml(0, ftunxml);
		ftunxml.close();
	}

	for (auto & x: e2db)
	{
		if (x.second.find("bouquets.") != string::npos)
		{
			ifstream fbouquet (e2db[x.first]);
			parse_e2db_bouquet(fbouquet, x.first);
			fbouquet.close();
		}
	}
	for (auto & x: bouquets)
	{
		for (auto & w: x.second.userbouquets)
		{
			ifstream fuserbouquet (e2db[w]);
			parse_e2db_userbouquet(fuserbouquet, w, x.first);
			fuserbouquet.close();
		}
	}

	clock_t end = clock();

	// seeds./enigma_db
	// commit: 05db5bb	elapsed time: 86547
	// commit: 6615a23	elapsed time: 83523
	// commit: 67b6442	elapsed time: 65520
	// commit: HEAD		elapsed time: 65313

	// workflow/Vhannibal Motor 08 mar 2022
	// commit: 05db5bb	elapsed time: 756600
	// commit: 6615a23	elapsed time: 601638
	// commit: 67b6442	elapsed time: 421056
	// commit: HEAD		elapsed time: 423214

	debug("e2db_parser", "parse_e2db()", "elapsed time", to_string(end - start));
}

void e2db_parser::parse_e2db_lamedb(ifstream& flamedb)
{
	debug("e2db_parser", "parse_e2db_lamedb()");

	string hlamedb;
	getline(flamedb, hlamedb);
	char vlamedb = (hlamedb.substr(hlamedb.length() - 2, hlamedb.length() - 1))[0];
	int dbver = isdigit(vlamedb) ? int (vlamedb) - 48 : 0;

	debug("e2db_parser", "lamedb", "File header", hlamedb, "\t");

	switch (dbver)
	{
		case 4: parse_e2db_lamedb4(flamedb); break;
		case 5: parse_e2db_lamedb5(flamedb); break;
		default: error("e2db_parser", "lamedb", "Error", "Unknown database format.", "\t");
	}
}

void e2db_parser::parse_e2db_lamedb4(ifstream& flamedb)
{
	debug("e2db_parser", "parse_e2db_lamedb4()");
	
	LAMEDB_VER = 4;
	int step = 0;
	int s = 0;
	int tidx = 0;
	int sidx = 0;
	string line;
	transponder tx;
	service ch;

	while (getline(flamedb, line))
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
				parse_lamedb_service_params(line, ch, true);
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

void e2db_parser::parse_e2db_lamedb5(ifstream& flamedb)
{
	debug("e2db_parser", "parse_e2db_lamedb5()");

	LAMEDB_VER = 5;
	bool step;
	int tidx = 0;
	int sidx = 0;
	string line;
	transponder tx;
	service ch;

	while (getline(flamedb, line))
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
			parse_lamedb_service_params(chname, ch, true);
			parse_lamedb_service_data(chdata, ch);
			add_service(sidx, ch);
		}
	}
}

void e2db_parser::parse_lamedb_transponder_params(string data, transponder& tx)
{
	char txid[25];
	int dvbns, tsid, onid;
	dvbns = 0, tsid = 0, onid = 0;

	sscanf(data.c_str(), "%08x:%04x:%04x", &dvbns, &tsid, &onid);

	tx.dvbns = dvbns;
	tx.tsid = tsid;
	tx.onid = onid;
	sprintf(txid, "%x:%x:%x", tsid, onid, dvbns);
	tx.txid = txid;
}

void e2db_parser::parse_lamedb_transponder_feparms(string data, char ttype, transponder& tx)
{
	int freq, sr, pol, fec, pos, inv, flgs, sys, mod, rol, pil;
	int band, hpfec, lpfec, termod, trxmod, guard, hier;
	int cabmod, ifec;
	char oflgs[33];
	sys = -1, mod = -1, rol = -1, pil = -1;
	tx.pol = -1, tx.fec = -1, tx.inv = -1, tx.sys = -1, tx.mod = -1, tx.rol = -1, tx.pil = -1;
	tx.band = -1, tx.hpfec = -1, tx.lpfec = -1, tx.termod = -1, tx.trxmod = -1, tx.guard = -1, tx.hier = -1;
	tx.cabmod = -1, tx.ifec = -1;

	switch (ttype)
	{
		case 's':
			sscanf(data.c_str(), "%8d:%8d:%1d:%1d:%3d:%1d:%1d:%1d:%1d:%1d:%1d%s", &freq, &sr, &pol, &fec, &pos, &inv, &flgs, &sys, &mod, &rol, &pil, oflgs);

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
		case 't': // DVB-T
			sscanf(data.c_str(), "%9d:%1d:%1d:%1d:%1d:%1d:%1d:%1d:%1d%s", &freq, &band, &hpfec, &lpfec, &termod, &trxmod, &guard, &hier, &inv, oflgs);

			tx.freq = int (freq / 1e3);
			tx.band = band;
			tx.hpfec = hpfec;
			tx.lpfec = lpfec;
			tx.termod = termod;
			tx.trxmod = trxmod;
			tx.guard = guard;
			tx.hier = hier;
			tx.inv = inv;
			tx.oflgs = string (oflgs);
		break;
		case 'c': // DVB-C
			//TODO
			sscanf(data.c_str(), "%8d:%8d:%1d:%1d:%1d%s", &freq, &sr, &inv, &cabmod, &ifec, oflgs);

			tx.freq = int (freq / 1e3);
			tx.sr = int (sr / 1e3);
			tx.inv = inv;
			tx.cabmod = cabmod;
			tx.ifec = ifec;
			tx.oflgs = string (oflgs);
		break;
		//TODO ATSC
		case 'a': // ATSC:
			error("e2db_parser", "lamedb", "Error", "ATSC not supported yet.", "\t");
		break;
		default:
			error("e2db_parser", "lamedb", "Error", "Transponder type is unknown.", "\t");
			return;
	}
	tx.ttype = ttype;
}

void e2db_parser::parse_lamedb_service_params(string data, service& ch)
{
	char chid[25];
	char txid[25];
	int ssid, dvbns, tsid, stype, snum;
	char onid[5]; //TODO to int
	ssid = 0, dvbns = 0, tsid = 0, stype = -1, snum = -1;

	if (LAMEDB_VER == 5)
	{
		int srcid = -1;
		sscanf(data.c_str(), "%04x:%08x:%04x:%4s:%3d:%4d:%d", &ssid, &dvbns, &tsid, onid, &stype, &snum, &srcid);
		ch.srcid = srcid;
	}
	else
	{
		sscanf(data.c_str(), "%04x:%08x:%04x:%4s:%3d:%4d", &ssid, &dvbns, &tsid, onid, &stype, &snum);
	}

	ch.ssid = ssid;
	ch.dvbns = dvbns;
	ch.tsid = tsid;
	ch.onid = onid;
	ch.onid.erase(0, ch.onid.find_first_not_of('0'));
	ch.stype = stype;
	ch.snum = snum;
	sprintf(txid, "%x:%s:%x", tsid, ch.onid.c_str(), dvbns);
	sprintf(chid, "%x:%x:%s:%x", ssid, tsid, ch.onid.c_str(), dvbns);
	ch.txid = txid;
	ch.chid = chid;
}

void e2db_parser::parse_lamedb_service_params(string data, service& ch, bool add)
{
	ch.chname = data;
}

void e2db_parser::parse_lamedb_service_data(string data, service& ch)
{
	if (data.empty()) return;

	//TODO 256
	// !p: provider
	//  c: cache
	//  C: ciad
	//  f: flags
	stringstream ss (data);
	string line;
	map<char, vector<string>> cdata;

	while (getline(ss, line, ','))
	{
		char k = line[0];
		char key = e2db_parser::PVDR_DATA.count(k) ? e2db_parser::PVDR_DATA.at(k) : k;
		string val = line.substr(2);
		cdata[key].push_back(val);
	}
	ch.data = cdata; //TODO lassign infinite loop
}

void e2db_parser::parse_e2db_bouquet(ifstream& fbouquet, string bname)
{
	debug("e2db_parser", "parse_e2db_bouquet()", "bname", bname);

	int idx = 0;
	string line;
	bouquet bs;

	while (getline(fbouquet, line))
	{
		if (line.find("#SERVICE") != string::npos)
		{
			char c_refid[33];
			char c_fname[33];
			char c_oby[13];

			sscanf(line.substr(9).c_str(), "%32s BOUQUET %32s ORDER BY %12s", c_refid, c_fname, c_oby);

			string fname = string (c_fname);
			fname = fname.substr(1, fname.length() - 2);

			index["ubs"].emplace_back(pair (idx++, fname)); //C++ 17
			bs.userbouquets.emplace_back(fname);
		}
		else if (line.find("#NAME") != string::npos)
		{
			bs = bouquet ();

			bs.name = line.substr(6); //TODO
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
			bs.count = 1;
			index["bss"].emplace_back(pair (bs.btype, bname)); //C++ 17
		}
	}

	bouquets.emplace(bname, bs);
}

void e2db_parser::parse_e2db_userbouquet(ifstream& fuserbouquet, string bname, string pname)
{
	debug("e2db_parser", "parse_e2db_userbouquet()", "bname", bname);

	bool step = false;
	int idx = 0;
	int j = 0;
	string line;
	string chid;
	userbouquet ub;
	reference ref;

	while (getline(fuserbouquet, line))
	{
		if (! step && line.find("#NAME") != string::npos)
		{
			ub = userbouquet ();

			ub.name = line.substr(6);
			ub.pname = pname;
			step = true;
			continue;
		}
		else if (step && line.find("#DESCRIPTION") != string::npos)
		{
			ub.channels[chid].refval = line.substr(13);
			continue;
		}
		else if (step && line.empty())
		{
			continue;
		}

		if (step)
		{
			line = line.substr(9);
			bool sseq = false;
			char cchid[24];
			int i0, i1, anum, ssid, tsid, dvbns;
			char onid[5];
			i0 = -1, i1 = -1, anum = -1, ssid = 0, tsid = 0, dvbns = 0;

			//TODO performance optimization selective tolower
			transform(line.begin(), line.end(), line.begin(), [](unsigned char c){ return c == ':' ? ' ' : tolower(c); });
			sscanf(line.c_str(), "%d %d %4x %4x %4x %4s %8x", &i0, &i1, &anum, &ssid, &tsid, onid, &dvbns);
			//TODO other flags ?

			switch (i1)
			{
				case 64:  // regular marker
				case 320: // numbered marker
				case 512: // hidden marker
				case 832: // hidden marker
					sseq = false;
					sprintf(cchid, "%d:%d:%x:%d", i0, i1, anum, ssid);
				break;
				case 128: // group //TODO
				default:  // service
					sseq = true;
					sprintf(cchid, "%x:%x:%s:%x", ssid, tsid, onid, dvbns);
			}
			if (sseq)
			{
				j += 1;
				idx = j;
			}
			else
			{
				idx = 0;
			}

			chid = string (cchid);
			ref.chid = chid;
			ref.reftype = i1;
			ref.refmrker = ! sseq;
			ref.refanum = anum;
			ref.index = idx;

			ub.channels[chid] = ref;
			index[bname].emplace_back(pair (idx, chid)); //C++ 17

			if (sseq)
				index[pname].emplace_back(pair (bouquets[pname].count++, chid)); //C++ 17

			ref = reference ();
		}
	}

	userbouquets.emplace(bname, ub);
}

//TODO terrestrial.xml, cable.xml, ...
//TODO FIX wrong position
//TODO needs index
void e2db_parser::parse_tunersets_xml(int ytype, ifstream& ftunxml)
{
	debug("e2db_parser", "parse_tunersets_xml()", "ytype", to_string(ytype));

	string htunxml;
	getline(ftunxml, htunxml);

	if (htunxml.find("<?xml") == string::npos)
	{
		error("e2db_parser", "parse_tunersets_xml()", "Error", "Unknown file format.", "\t");
		return;
	}

	tuner_sets tn;
	tuner_reference tr;

	switch (ytype)
	{
		case 0:
			tn.ytype = 0;
		break;
		default:
			error("e2db_parser", "parse_tunersets_xml()", "Error", "Not supported yet.", "\t");
			return;
	}

	int step = 0;
	string line;

	while (getline(ftunxml, line))
	{
		if (line.find("<!") != string::npos) //TODO inline comments
		{
			step = 0;
			continue;
		}
		else if (step && line.find("</") != string::npos)
		{
			step--;
			char trid[17];
			sprintf(trid, "%d:%d:%d", tr.freq, tr.pol, tr.sr);
			tn.references.emplace(trid, tr);
			tuners.emplace(tn.pos, tn);
			tn = tuner_sets ();
			tr = tuner_reference ();
			continue;
		}
		else if (! step && line.find("<") != string::npos)
		{
			step++;
		}

		if (step)
		{
			string mkey;
			string sline = line;
			line.erase(0, line.find_first_not_of(' '));
			char* token = strtok(line.data(), " ");
			while (token != 0)
			{
				string pstr = string (token);
				string key, val;
				unsigned long pos = pstr.find('=');

				if (pos != string::npos)
				{
					key = pstr.substr(0, pos);
					val = pstr.substr(pos + 1);
					pos = val.rfind('"');

					if (pos)
					{
						val = val.substr(val.find('"') + 1, pos - 1);
					}
					else
					{
						val = sline.substr(sline.find(key));
						val = val.substr(val.find('"') + 1);
						val = val.substr(0, val.find('"'));
					}
				}
				else if (pstr[0] == '<')
				{
					mkey = pstr.substr(1, pstr.rfind('>') - 1);
				}

				if (step != 2 && mkey == "sat")
					step++;
				else if (key == "name")
					tn.name = val;
				else if (key == "flags")
					tn.flgs = stoi(val);
				else if (key == "position")
					tn.pos = stoi(val);
				else if (key == "frequency")
					tr.freq = int (stoi(val) / 1e3);
				else if (key == "symbol_rate")
					tr.sr = int (stoi(val) / 1e3);
				else if (key == "polarization")
					tr.pol = stoi(val);
				else if (key == "fec_inner")
					tr.fec = stoi(val);
				else if (key == "modulation")
					tr.mod = stoi(val);
				else if (key == "rolloff")
					tr.rol = stoi(val);
				else if (key == "pilot")
					tr.pil = stoi(val);
				else if (key == "inversion")
					tr.inv = stoi(val);
				else if (key == "system")
					tr.sys = stoi(val);
				else if (key == "is_id")
					tr.isid = stoi(val);
				else if (key == "pls_mode")
					tr.plsmode = stoi(val);
				else if (key == "pls_code")
					tr.plscode = stoi(val);

				// cout << mkey << ':' << key << ':' << val << ' ' << step << endl;
				token = strtok(NULL, " ");
			}
		}
	}
}

//TODO hex/dec values
void e2db_parser::debugger()
{
	debug("e2db_parser", "debugger()");

	cout << hex;
	cout << "transponders" << endl << endl;
	for (auto & x: db.transponders)
	{
		cout << "txid: " << x.first << endl;
		cout << "dvbns: " << x.second.dvbns << endl;
		cout << "tsid: " << x.second.tsid << endl;
		cout << "onid: " << x.second.onid << endl;
		cout << "ttype: " << x.second.ttype << endl;
		cout << "freq: " << x.second.freq << endl;
		cout << "sr: " << x.second.sr << endl;
		cout << "pol: " << x.second.pol << endl;
		cout << "fec: " << x.second.fec << endl;
		cout << "hpfec: " << x.second.hpfec << endl;
		cout << "lpfec: " << x.second.lpfec << endl;
		cout << "ifec: " << x.second.ifec << endl;
		cout << "pos: " << x.second.pos << endl;
		cout << "inv: " << x.second.inv << endl;
		cout << "flgs: " << x.second.flgs << endl;
		cout << "sys: " << x.second.sys << endl;
		cout << "mod: " << x.second.mod << endl;
		cout << "termod: " << x.second.termod << endl;
		cout << "cabmod: " << x.second.cabmod << endl;
		cout << "rol: " << x.second.rol << endl;
		cout << "pil: " << x.second.pil << endl;
		cout << "band: " << x.second.band << endl;
		cout << "trxmod: " << x.second.trxmod << endl;
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
		cout << "ssid: " << x.second.ssid << endl;
		cout << "dvbns: " << x.second.dvbns << endl;
		cout << "tsid: " << x.second.tsid << endl;
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
	for (auto & x: tuners)
	{
		cout << "tnid: " << x.first << endl;
		cout << "ytype: " << x.second.ytype << endl;
		cout << "name: " << x.second.name << endl;
		cout << "flags: " << x.second.flgs << endl;
		cout << "pos: " << x.second.pos << endl;
		cout << "references: [" << endl << endl;
		for (auto & q: x.second.references)
		{
			cout << "trid: " << q.first << endl;
			cout << "freq: " << q.second.freq << endl;
			cout << "sr: " << q.second.sr << endl;
			cout << "pol: " << q.second.pol << endl;
			cout << "fec: " << q.second.fec << endl;
			cout << "mod: " << q.second.mod << endl;
			cout << "rol: " << q.second.rol << endl;
			cout << "pil: " << q.second.pil << endl;
			cout << "inv: " << q.second.inv << endl;
			cout << "sys: " << q.second.sys << endl;
			cout << "isid: " << q.second.isid << endl;
			cout << "plsmode: " << q.second.plsmode << endl;
			cout << "plscode: " << q.second.plscode << endl;
			cout << endl;
		}
		cout << "]" << endl << endl;
	}
	cout << endl;
	cout << dec;
}

unordered_map<string, e2db_parser::transponder> e2db_parser::get_transponders()
{
	debug("e2db_parser", "get_transponders()");
	return db.transponders;
}

unordered_map<string, e2db_parser::service> e2db_parser::get_services()
{
	debug("e2db_parser", "get_services()");
	return db.services;
}

pair<unordered_map<string, e2db_parser::bouquet>, unordered_map<string, e2db_parser::userbouquet>> e2db_parser::get_bouquets()
{
	debug("e2db_parser", "get_bouquets()");
	return pair (bouquets, userbouquets); //C++ 17
}

// C++17
bool e2db_parser::read_from_localdir(string localdir)
{
	debug("e2db_parser", "read_from_localdir()", "localdir", localdir);

	if (! filesystem::exists(localdir))
	{
		error("e2db_parser", "read_from_localdir()", "Error", "Directory \"" + localdir + "\" not exists.", "\t");
		return false;
	}

	filesystem::directory_iterator dirlist(localdir);

	for (const auto & entry : dirlist)
	{
		//TODO is file & permissions check ...
		string path = entry.path();
		string filename = filesystem::path(path).filename();
		e2db[filename] = path;
	}
	if (e2db.count(dbfilename) < 1)
	{
		error("e2db_parser", "read_from_localdir()", "Error", "lamedb not found.", "\t");
		return false;
	}
	this->localdir = localdir;

	return true;
}

bool e2db_parser::read(string localdir)
{
	debug("e2db_parser", "read()", "localdir", localdir);

	if (read_from_localdir(localdir))
		parse_e2db();
	else
		return false;

	return true;
}



e2db_maker::e2db_maker()
{
	debug("e2db_maker");
}

void e2db_maker::make_e2db()
{
	debug("e2db_maker", "make_e2db()");

	begin_transaction();
	make_e2db_lamedb();
	make_e2db_bouquets();
	make_e2db_userbouquets();
	end_transaction();
}

void e2db_maker::begin_transaction()
{
	debug("e2db_maker", "begin_transaction()");

	time_t curr_tst = time(0);
	tm* _out_tst = localtime(&curr_tst);
	this->_out_tst = _out_tst;
}

void e2db_maker::end_transaction()
{
	debug("e2db_maker", "end_transaction()");
}

string e2db_maker::get_timestamp()
{
	debug("e2db_maker", "get_timestamp()");

	char datetime[80];
	strftime(datetime, 80, "%F %T %Z", _out_tst);
	return string (datetime);
}

string e2db_maker::get_editor_string()
{
	return "e2-sat-editor 0.1 <https://github.com/ctlcltd/e2-sat-editor>";
}

void e2db_maker::make_e2db_lamedb()
{
	debug("e2db_maker", "make_e2db_lamedb()");

	make_e2db_lamedb4();

	//TEST
	if (MAKER_LAMEDB5)
		make_e2db_lamedb5();
	//TEST
}

void e2db_maker::make_e2db_lamedb4()
{
	debug("e2db_maker", "make_e2db_lamedb4()");
	LAMEDB_VER = 4;
	make_lamedb("lamedb");
}

void e2db_maker::make_e2db_lamedb5()
{
	debug("e2db_maker", "make_e2db_lamedb5()");
	LAMEDB_VER = 5;
	make_lamedb("lamedb5");
}

//TODO ATSC
void e2db_maker::make_lamedb(string filename)
{
	debug("e2db_maker", "make_lamedb()");

	//TODO
	unordered_map<char, char> PVDR_DATA_DENUM;
	for (auto & x: e2db_maker::PVDR_DATA) PVDR_DATA_DENUM[x.second] = x.first;

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
				ss << ':' << tx.termod;
				ss << ':' << tx.trxmod;
				ss << ':' << tx.guard;
				ss << ':' << tx.hier;
				ss << ':' << tx.inv;
				if (! tx.oflgs.empty())
					ss << tx.oflgs;
			break;
			case 'c': // DVB-C
				//TODO
				ss << int (tx.freq * 1e3);
				ss << ':' << int (tx.sr * 1e3);
				ss << ':' << tx.inv;
				ss << ':' << tx.cabmod;
				ss << ':' << tx.ifec;
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
			char d = PVDR_DATA_DENUM.count(q.first) ? PVDR_DATA_DENUM.at(q.first) : q.first;
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
	debug("e2db_maker", "make_e2db_bouquets()");

	for (auto & x: bouquets)
		make_bouquet(x.first);
}

void e2db_maker::make_e2db_userbouquets()
{
	debug("e2db_maker", "make_e2db_userbouquets()");

	for (auto & x: userbouquets)
		make_userbouquet(x.first);
}

//TODO index
void e2db_maker::make_bouquet(string bname)
{
	debug("e2db_maker", "make_bouquet()", "bname", bname);

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

//TODO upCase or lowCase
void e2db_maker::make_userbouquet(string bname)
{
	debug("e2db_maker", "make_userbouquet()", "bname", bname);

	userbouquet ub = userbouquets[bname];
	stringstream ss;

	ss << "#NAME " << ub.name << endl;
	for (auto & x: index[bname])
	{
		reference cref = userbouquets[bname].channels[x.second];
		ss << "#SERVICE ";
		ss << "1:";
		ss << cref.reftype << ':';
		ss << hex;
		ss << uppercase << cref.refanum << ':'; //TODO ("global markers index)
		
		if (db.services.count(x.second))
		{
			service cdata = db.services[x.second];
			ss << uppercase << cdata.ssid << ':';
			ss << uppercase << cdata.tsid << ':';
			ss << upCase(cdata.onid) << ':';
			ss << uppercase << cdata.dvbns << ':';
			ss << "0:0:0:";
		}
		else
		{
			ss << "0:0:0:0:0:0:0:0:" << endl;
			ss << "#DESCRIPTION " << cref.refval;
		}
		ss << dec;
		ss << endl;
	}
	// ss << endl;
	e2db_out[bname] = ss.str();
}

//C++17
bool e2db_maker::write_to_localdir(string localdir, bool overwrite)
{
	debug("e2db_maker", "write_to_localdir()", "localdir", localdir);

	if (! filesystem::is_directory(localdir))
	{
		error("e2db_maker", "write_to_localdir()", "Error", "Directory \"" + localdir + "\" not exists.");
		return false;
	}
	//TODO file exists and (force) overwrite
	else if (! overwrite)
	{
		filesystem::create_directory(localdir);
	}
	//TODO permission check ...
	for (auto & o: e2db_out)
	{
		string localfile = localdir + '/' + o.first;

		ofstream out(localfile);
		out << o.second;
		out.close();
	}

	return true;
}

bool e2db_maker::write(string localdir, bool overwrite)
{
	debug("e2db_maker", "write()", "localdir", localdir);

	make_e2db();

	if (write_to_localdir(localdir, overwrite))
		return true;
	else
		return false;
}

void e2db_maker::set_index(unordered_map<string, vector<pair<int, string>>> index)
{
	debug("e2db_maker", "set_index()");
	this->index = index;
}

void e2db_maker::set_transponders(unordered_map<string, e2db_maker::transponder> transponders)
{
	debug("e2db_maker", "set_transponders()");
	db.transponders = transponders;
}

void e2db_maker::set_channels(unordered_map<string, e2db_maker::service> services)
{
	debug("e2db_maker", "set_channels()");
	db.services = services;
}

void e2db_maker::set_bouquets(pair<unordered_map<string, e2db_maker::bouquet>, unordered_map<string, e2db_maker::userbouquet>> bouquets)
{
	debug("e2db_maker", "set_bouquets()");
	this->bouquets = bouquets.first;
	this->userbouquets = bouquets.second;
}



e2db::e2db()
{
	debug("e2db");
}

//TODO unique (eg. terrestrial MUX)
map<string, vector<pair<int, string>>> e2db::get_channels_index()
{
	debug("e2db", "get_channels_index()");

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
	debug("e2db", "get_transponders_index()");

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
	debug("e2db", "get_services_index()");

	map<string, vector<pair<int, string>>> _index;
	_index["chs"] = index["chs"];

	return _index;
}

map<string, vector<pair<int, string>>> e2db::get_bouquets_index()
{
	debug("e2db", "get_bouquets_index()");

	map<string, vector<pair<int, string>>> _index;

	for (auto & x: bouquets)
		_index[x.first] = index[x.first];

	return _index;
}

map<string, vector<pair<int, string>>> e2db::get_userbouquets_index()
{
	debug("e2db", "get_userbouquets_index()");

	map<string, vector<pair<int, string>>> _index;

	for (auto & x: userbouquets)
		_index[x.first] = index[x.first];

	return _index;
}

map<string, vector<pair<int, string>>> e2db::get_packages_index()
{
	debug("e2db", "get_packages_index()");

	map<string, vector<pair<int, string>>> _index;

	for (auto & x: index["chs"])
	{
		service ch = db.services[x.second];

		if (ch.data.count(e2db::PVDR_DATA.at('p')))
		{
			string pvdrn = ch.data[e2db::PVDR_DATA.at('p')][0];

			if (pvdrn.empty()) _index["(Unknown)"].emplace_back(x);
			else _index[pvdrn].emplace_back(x);
		}
	}

	return _index;
}

map<string, vector<pair<int, string>>> e2db::get_resolution_index()
{
	debug("e2db", "get_resolution_index()");

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
	debug("e2db", "get_encryption_index()");

	map<string, vector<pair<int, string>>> _index;
	unordered_set<string> _unique;

	for (auto & x: index["chs"])
	{
		service ch = db.services[x.second];

		if (ch.data.count('2'))
		{
			for (string & w : ch.data['2'])
			{
				string caidpx = w.substr(0, 2);
				string cx = caidpx + '|' + to_string(x.first);
				if (e2db::SCAS.count(caidpx) && ! _unique.count(cx))
				{
					_index[e2db::SCAS.at(caidpx)].emplace_back(x);
					_unique.insert(cx);
				}
			}
		}
		else
		{
			_index["(FTA)"].emplace_back(x);
		}
	}
	_unique.clear();

	return _index;
}

map<string, vector<pair<int, string>>> e2db::get_az_index()
{
	debug("e2db", "get_az_index()");

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
