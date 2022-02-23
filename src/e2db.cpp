/*!
 * e2-sat-editor/src/e2db.cpp
 * 
 * @author Leonardo Laureti
 * @version 0.1
 * @license MIT License
 */

#include <iostream>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <algorithm>
#include <string>
#include <vector>
#include <map>
#include <unordered_map>
#include <tuple>
#include <regex>
#include <cstdio>
#include <cstring>
#include "commons.h"
#include "e2db.h"

using namespace std;

e2db_parser::e2db_parser()
{
	lamedb db;
	map<string, bouquet> bouquets;
	map<string, userbouquet> userbouquets;
}

void e2db_parser::parse_e2db()
{
	debug("e2db_parser", "parse_e2db()");

	ifstream flamedb(e2db["lamedb"]);
	parse_e2db_lamedb(flamedb);
	flamedb.close();

	for (auto & x: e2db)
	{
		if (x.second.find("bouquets.") != string::npos)
		{
			ifstream fbouquet(e2db[x.first]);
			parse_e2db_bouquet(fbouquet, x.first);
			fbouquet.close();
		}
	}
	for (auto & x: bouquets)
	{
		for (auto & w: x.second.userbouquets)
		{
			ifstream fuserbouquet(e2db[w]);
			parse_e2db_userbouquet(fuserbouquet, w, x.first);
			fuserbouquet.close();
		}
	}
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

//TODO stype DVB-T & DVB-C
void e2db_parser::parse_e2db_lamedb4(ifstream& flamedb)
{
	debug("e2db_parser", "parse_e2db_lamedb4()");

	int step = 0;
	int count = 0;
	int idx = 0;
	string line;
	string txid = "";
	string chid = "";
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

		if (step == 1)
		{
			count += 1;

			if (count == 1)
			{
				// 00820000:005e:0055
				// 0082afc2:0065:0001

				char dvbns[9];
				char tsid[5];
				char onid[5];

				sscanf(upCase(line).c_str(), "%8s:%4s:%4s", dvbns, tsid, onid);

				tx.dvbns = dvbns;
				tx.dvbns.erase(0, tx.dvbns.find_first_not_of('0'));
				tx.tsid = string (tsid);
				tx.tsid.erase(0, tx.tsid.find_first_not_of('0'));
				tx.onid = string (onid);
				tx.onid.erase(0, tx.onid.find_first_not_of('0'));
				txid = tx.tsid + ":" + tx.onid + ":" + tx.dvbns;
			}
			else if (count == 2)
			{
				//  s 11219000:29900000:0:0:130:0:0
				//  s 10949000:29900000:1:7:130:2:0:1:2:0:2

				tx.ttype = line.substr(1, 2)[0];
				string txdata = line.substr(3);

				switch (tx.ttype) {
					case 's': // DVB-S
						int freq;
						int sr;
						int pol;
						int fec;
						int pos;
						int inv;
						int flgs;
						int sys;
						int mod;
						int rol;
						int pil;
						flgs = NULL;
						sys = NULL;
						mod = NULL;
						rol = NULL;
						pil = NULL;

						sscanf(txdata.c_str(), "%8d:%8d:%1d:%1d:%3d:%1d:%1d:%1d:%1d:%1d:%1d", &freq, &sr, &pol, &fec, &pos, &inv, &flgs, &sys, &mod, &rol, &pil);

						tx.freq = to_string(int (freq / 1e3));
						tx.sr = to_string(int (sr / 1e3));
						tx.pol = pol;
						tx.fec = fec;
						tx.pos = pos; //TODO satellites.xml
						tx.inv = inv;
						tx.flgs = to_string(flgs);
						tx.sys = sys;
						tx.mod = mod;
						tx.rol = rol; // DVB-S2 only
						tx.pil = pil; // DVB-S2 only
					break;
					case 't': // DVB-T
						debug("e2db_parser", "parse_e2db_lamedb4()", "txtyp", "'t'\tTODO");
					break;
					case 'c': // DVB-C
						debug("e2db_parser", "parse_e2db_lamedb4()", "txtyp", "'c'\tTODO");
					break;
				}
			}
			else if (count == 3)
			{
				db.transponders.emplace(txid, tx);
				count = 0;
				txid = "";
				transponder tx;
			}
		}
		else if (step == 2)
		{
			count += 1;

			if (count == 1)
			{
				// 3b69:00820000:012c:013e:25:2381
				// 32cd:00820000:0190:013e:1:2382
				//
				// 0082afc2:0065:0001 [transponder]

				char ssid[5];
				char dvbns[9];
				char tsid[5];
				char onid[5];
				int stype;
				int snum;
				stype = NULL;
				snum = NULL;

				sscanf(upCase(line).c_str(), "%4s:%8s:%4s:%4s:%3d:%4d", ssid, dvbns, tsid, onid, &stype, &snum);

				ch.ssid = string (ssid);
				ch.ssid.erase(0, ch.ssid.find_first_not_of('0'));
				ch.dvbns = string (dvbns);
				ch.dvbns.erase(0, ch.dvbns.find_first_not_of('0'));
				ch.tsid = string (tsid);
				ch.tsid.erase(0, ch.tsid.find_first_not_of('0'));
				ch.onid = string (onid);
				ch.onid.erase(0, ch.onid.find_first_not_of('0'));
				ch.stype = stype;
				ch.snum = snum;
				txid = ch.tsid + ":" + ch.onid + ":" + ch.dvbns;
				chid = ch.ssid + ":" + ch.tsid + ":" + ch.onid + ":" + ch.dvbns;
				idx += 1;
			}
			else if (count == 2)
			{
				ch.index = idx;
				ch.txid = txid;
				ch.chname = line;
			}
			else if (count == 3)
			{
				if (line.size())
				{
					stringstream datas(line);
					string l;
					unordered_map<char, vector<string>> data;

					while (getline(datas, l, ','))
					{
						char key = l.substr(0, 1)[0];
						string value = l.substr(2);
						data[key].push_back(value);
					}
					ch.data = data;
				}

				db.services.emplace(chid, ch);
				index["all"].emplace_back(pair (idx, chid));
				count = 0;
				chid = "";
				service ch;
			}
		}
	}
}

//TODO lamedb ver. 5
void e2db_parser::parse_e2db_lamedb5(ifstream& flamedb)
{
	error("e2db_parser", "parse_e2db_lamedb5()", "Notice", "Not supported, ver. 5 is not currently supported.", "\t");
}

void e2db_parser::parse_e2db_bouquet(ifstream& fbouquet, string bname)
{
	debug("e2db_parser", "parse_e2db_bouquet()", "bname", bname);

	string line;
	bouquet bs;

	while (getline(fbouquet, line))
	{
		if (line.find("#SERVICE") != string::npos)
		{
			// #SERVICE 1:7:1:0:0:0:0:0:0:0:FROM BOUQUET "userbouquet.dbe01.tv" ORDER BY bouquet
			// #SERVICE 1:7:2:0:0:0:0:0:0:0:FROM BOUQUET "userbouquet.dbe01.radio" ORDER BY bouquet

			char c_refid[33];
			char c_fname[33];
			char c_oby[13];

			sscanf(line.substr(9).c_str(), "%32s BOUQUET %32s ORDER BY %12s", c_refid, c_fname, c_oby);

			string fname = string (c_fname);
			fname = fname.substr(1, fname.length() - 2);

			bs.userbouquets.emplace_back(fname);
		}
		else if (line.find("#NAME") != string::npos)
		{
			bs.name = line.substr(6);
			if (bname.find(".tv") != string::npos) bs.nname = "TV";
			else if (bname.find(".radio") != string::npos) bs.nname = "Radio";
			bs.count = 1; //TODO FIX
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
		if (step && line.find("#SORT") != string::npos) //TODO ? #SORT
		{
			step = false;
			continue;
		}
		else if (! step && line.find("#NAME") != string::npos)
		{
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

		if (step)
		{
			// 1:0:2:32:2E18:B0:820000:0:0:0:
			// 1:0:2:2CB:1B58:13E:820000:0:0:0:
			// %d:%d:btype:ssid:tsid:onid:dvbns:?:?:?:
			// %d:%d:btype:anum:0:0:0:?:?:?:

			line = line.substr(9);
			bool sseq = false;
			char cchid[24];
			int i0, i1;
			char btype[5];
			char ssid[5]; //TODO check lpad '0'
			char tsid[5];
			char onid[5];
			int dvbns;
			i0 = NULL;
			i1 = NULL;
			dvbns = NULL;

			//TODO upCase call impact
			transform(line.begin(), line.end(), line.begin(), [](unsigned char c){ return c == ':' ? ' ' : toupper(c); });
			sscanf(line.c_str(), "%d %d %4s %4s %4s %4s %8d", &i0, &i1, btype, ssid, tsid, onid, &dvbns);

			switch (i1) {
				case 64:  // regular marker
				case 320: // numbered marker
				case 512: // hidden marker
				case 832: // hidden marker ? //TODO
					sseq = false;
					sprintf(cchid, "%d:%d:%s:%s", i0, i1, btype, ssid);
				break;
				case 128: // group //TODO
				default:  // service
					sseq = true;
					sprintf(cchid, "%s:%s:%s:%d", ssid, tsid, onid, dvbns);
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
			ref.reftype = i0;
			ref.index = idx;

			ub.channels[chid] = ref;
			index[bname].emplace_back(pair (idx, chid));

			//TODO convenient way
			if (sseq)
				index[pname].emplace_back(pair (bouquets[pname].count++, chid));

			reference ref;
		}
	}

	userbouquets.emplace(bname, ub);
}

void e2db_parser::debugger()
{
	cout << "e2db_parser debugger()" << endl;
	cout << endl;
	for (auto & x: db.transponders)
	{
		cout << "txid: " << x.first << endl;
		cout << "dvbns: " << x.second.dvbns << endl;
		cout << "tsid: " << x.second.tsid << endl;
		cout << "onid: " << x.second.onid << endl;
		cout << "ttype: " << x.second.ttype << endl;
		cout << "freq: " << x.second.freq << endl;
		cout << "sr: " << x.second.sr << endl;
		cout << "pol: " << to_string(x.second.pol) << endl;
		cout << "fec: " << to_string(x.second.fec) << endl;
		cout << "pos: " << to_string(x.second.pos) << endl;
		cout << "inv: " << to_string(x.second.inv) << endl;
		cout << "flgs: " << x.second.flgs << endl;
		cout << "sys: " << to_string(x.second.sys) << endl;
		cout << "mod: " << to_string(x.second.mod) << endl;
		cout << "rol: " << to_string(x.second.rol) << endl;
		cout << "pil: " << to_string(x.second.pil) << endl;
		cout << endl;
	}
	cout << endl;
	for (auto & x: db.services)
	{
		cout << "chid: " << x.first << endl;
		cout << "ssid: " << x.second.ssid << endl;
		cout << "dvbns: " << x.second.dvbns << endl;
		cout << "tsid: " << x.second.tsid << endl;
		cout << "onid: " << x.second.onid << endl;
		cout << "stype: " << x.second.stype << endl;
		cout << "snum: " << x.second.snum << endl;
		cout << "chname: " << x.second.chname << endl;
		cout << "data: [" << endl;
 		for (auto & q: x.second.data)
		{
			cout << "\"" << q.first << "\": [";
			cout << q.second.size() << endl;
			for (string & w: q.second)
				cout << "\"" << w << "\",";
			cout << "]";
		}
		cout << endl << "]" << endl;
		cout << "index: " << x.second.index << endl;
		cout << endl;
	}
	cout << endl;
	for (auto & x: bouquets)
	{
		cout << "filename: " << x.first << endl;
		cout << "name: " << x.second.name << endl;
		cout << "nname: " << x.second.nname << endl;
		cout << "userbouquets: [";
		for (auto & w: x.second.userbouquets)
			cout << "\"" << w << "\",";
		cout << "]" << endl;
		cout << endl;
	}
	cout << endl;
	for (auto & x: userbouquets)
	{
		cout << "filename: " << x.first << endl;
		cout << "name: " << x.second.name << endl;
		cout << "channels: [" << endl;
		for (auto & q: x.second.channels)
			cout << "{\"chid\":\"" << q.first << "\",\"index\":" << q.second.index << "}," << endl;
		cout << "]" << endl;
		cout << endl;
	}
}

map<string, e2db_parser::transponder> e2db_parser::get_transponders()
{
	debug("e2db_parser", "get_transponders()");
	return db.transponders;
}

map<string, e2db_parser::service> e2db_parser::get_channels()
{
	debug("e2db_parser", "get_channels()");
	return db.services;
}

pair<map<string, e2db_parser::bouquet>, map<string, e2db_parser::userbouquet>> e2db_parser::get_bouquets()
{
	debug("e2db_parser", "get_bouquets()");
	return pair (bouquets, userbouquets);
}

// C++17
bool e2db_parser::read_localdir(string localdir)
{
	debug("e2db_parser", "read_localdir()", "localdir", localdir);

	if (! filesystem::exists(localdir))
	{
		error("e2db_parser", "read_localdir()", "Error", "File not exists: \"" + localdir + "\".", "\t");
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
	if (e2db.count("lamedb") < 1)
	{
		error("e2db_parser", "read_localdir()", "Error", "lamedb not found.", "\t");
		return false;
	}
	this->localdir = localdir;

	return true;
}

bool e2db_parser::read(string localdir)
{
	debug("e2db_parser", "read()", "localdir", localdir);

	if (read_localdir(localdir))
		parse_e2db();
	else
		return false;

	return true;
}



e2db_maker::e2db_maker()
{
	debug("e2db_maker");

	time_t curr_tst = time(0);
	tm* _out_tst = localtime(&curr_tst);
	this->_out_tst = _out_tst;

	lamedb db;
	map<string, bouquet> bouquets;
	map<string, userbouquet> userbouquets;
}

void e2db_maker::make_lamedb()
{
	debug("e2db_maker", "make_lamedb()");

	make_lamedb4();
}

void e2db_maker::make_lamedb4()
{
	debug("e2db_maker", "make_lamedb4()");

	stringstream ss;

	ss << "eDVB services /4/" << endl;

	ss << "transponders" << endl;
	for (auto & x: db.transponders)
	{
		string dvbns = x.second.dvbns;
		string tsid = lowCase(x.second.tsid);
		string onid = lowCase(x.second.onid);
		dvbns.insert(dvbns.begin(), 8 - dvbns.length(), '0');
		tsid.insert(tsid.begin(), 4 - tsid.length(), '0');
		onid.insert(onid.begin(), 4 - onid.length(), '0');

		ss << dvbns;
		ss << ':' << tsid;
		ss << ':' << onid;
		ss << endl;
		ss << '\t' << x.second.ttype;
		ss << ' ' << to_string(int (stoi(x.second.freq) * 1e3));
		ss << ':' << to_string(int (stoi(x.second.sr) * 1e3));
		ss << ':' << x.second.pol;
		ss << ':' << x.second.fec;
		ss << ':' << x.second.pos; //TODO satellites.xml
		ss << ':' << x.second.inv;
		if (x.second.flgs != "")
			ss << ':' << x.second.flgs;
		if (x.second.sys)
			ss << ':' << x.second.sys;
		if (x.second.mod)
			ss << ':' << x.second.mod;
		if (x.second.rol) // DVB-S2 only
			ss << ':' << x.second.rol;
		if (x.second.pil) // DVB-S2 only
			ss << ':' << x.second.pil;
		ss << endl << '/' << endl;
	}
	ss << "end" << endl;

	ss << "services" << endl;
	for (auto & x: db.services)
	{
		ss << lowCase(x.second.ssid);
		ss << ':' << x.second.dvbns;
		ss << ':' << lowCase(x.second.tsid);
		ss << ':' << lowCase(x.second.onid);
		ss << ':' << x.second.stype;
		ss << ':' << x.second.snum;
		ss << endl << x.second.chname << endl;
		for (auto & q: x.second.data)
		{
			ss << q.first << ":";
			for (string & w: q.second)
				ss << w << ",";
		}
		ss << endl;
	}
	ss << "end" << endl;

	ss << "editor: e2-sat-editor 0.1 <https://github.com/ctlcltd/e2-sat-editor>" << endl;
	ss << "datetime: " << asctime(_out_tst);
	e2db_out["lamedb"] = ss.str();
}

void e2db_maker::make_bouquets()
{
	debug("e2db_maker", "make_bouquets()");

	for (auto & x: bouquets)
		make_bouquet(x.first);
}

void e2db_maker::make_userbouquets()
{
	debug("e2db_maker", "make_userbouquets()");

	for (auto & x: userbouquets)
		make_userbouquet(x.first);
}

void e2db_maker::make_bouquet(string bname)
{
	debug("e2db_maker", "make_bouquet()", "bname", bname);

	bouquet bs = bouquets[bname];
	stringstream ss;

	ss << "#NAME " << bs.nname << endl;
	for (auto & w: bs.userbouquets)
	{
		ss << "#SERVICE ";
		ss << "1:7:0:0:0:0:0:0:0:0:"; //TODO 1:7:1: tv, 1:7:2: radio, ...
		ss << "FROM BOUQUET ";
		ss << "\"" << w << "\" ";
		ss << "ORDER BY bouquet";
		ss << endl;
	}
	ss << endl;
	e2db_out[bname] = ss.str();
}

void e2db_maker::make_userbouquet(string bname)
{
	debug("e2db_maker", "make_userbouquet()", "bname", bname);

	userbouquet ub = userbouquets[bname];
	stringstream ss;

	ss << "#NAME " << ub.name << endl;
	for (auto & q: ub.channels)
	{
		ss << "#SERVICE " << q.first << endl; //TODO ("global markers index)
	}
	ss << endl;
	e2db_out[bname] = ss.str();
}

//TEST
void e2db_maker::write_e2db()
{
	debug("e2db_maker", "write_e2db()");

	string basedir = "/usr/local/var/tmp/e2-sat-editor";

	filesystem::create_directory(basedir);

	if (! filesystem::is_directory(basedir))
		error("e2db_maker", "write_e2db()", "Error", "Directory: \"/usr/local/var/tmp/e2-sat-editor\" not exists.");

	for (auto & o: e2db_out)
	{
		string localfile = basedir + '/' + o.first;

		ofstream out(localfile);
		out << o.second;
		out.close();
	}
}
//TEST

void e2db_maker::set_transponders(map<string, e2db_parser::transponder> transponders)
{
	debug("e2db_maker", "set_transponders()");
	db.transponders = transponders;
}

void e2db_maker::set_channels(map<string, e2db_parser::service> services)
{
	debug("e2db_maker", "set_channels()");
	db.services = services;
}

void e2db_maker::set_bouquets(pair<map<string, e2db_parser::bouquet>, map<string, e2db_parser::userbouquet>> bouquets)
{
	debug("e2db_maker", "set_bouquets()");
	this->bouquets = bouquets.first;
	this->userbouquets = bouquets.second;
}

void e2db_maker::tester()
{
	debug("e2db_maker", "tester()");

	make_lamedb();
	make_bouquets();
	make_userbouquets();
	write_e2db();
}


e2db::e2db()
{
	debug("e2db");
}
