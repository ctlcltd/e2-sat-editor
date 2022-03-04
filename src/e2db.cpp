/*!
 * e2-sat-editor/src/e2db.cpp
 * 
 * @author Leonardo Laureti
 * @version 0.1
 * @license MIT License
 */

#include <algorithm>
#include <filesystem>
#include <regex>
#include <cstdio>
#include <cstring>
#include "commons.h"
#include "e2db.h"

using namespace std;

const string dbfilename = "lamedb";

e2db_parser::e2db_parser()
{
	debug("e2db_parser");
}

void e2db_parser::parse_e2db()
{
	debug("e2db_parser", "parse_e2db()");

	ifstream flamedb(e2db[dbfilename]);
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
	int tidx = 0;
	int sidx = 0;
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

		// transponder
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
				txid = tx.tsid + ':' + tx.onid + ':' + tx.dvbns;
				tidx += 1;
			}
			else if (count == 2)
			{
				//  s 11219000:29900000:0:0:130:0:0
				//  s 10949000:29900000:1:7:130:2:0:1:2:0:2

				tx.index = tidx;
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
						debug("e2db_parser", "parse_e2db_lamedb4()", "txtype", "'t'\tTODO");
					break;
					case 'c': // DVB-C
						debug("e2db_parser", "parse_e2db_lamedb4()", "txtype", "'c'\tTODO");
					break;
				}
			}
			else if (count == 3)
			{
				db.transponders.emplace(txid, tx);
				index["txs"].emplace_back(pair (tidx, txid)); //C++ 17
				count = 0;
				txid = "";
				transponder tx;
			}
		}
		// service
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
				txid = ch.tsid + ':' + ch.onid + ':' + ch.dvbns;
				chid = ch.ssid + ':' + ch.tsid + ':' + ch.onid + ':' + ch.dvbns;
				sidx += 1;
			}
			else if (count == 2)
			{
				ch.index = sidx;
				ch.txid = txid;
				ch.chname = line; //TODO FIX "bad chars
			}
			else if (count == 3)
			{
				//TODO 256
				// !p: provider
				//  c: cache
				//  C: ciad
				//  f: flags
				stringstream datas(line);
				string l;
				map<char, vector<string>> data;

				while (getline(datas, l, ','))
				{
					char d = l[0];
					char key = PVDR_DATA.count(d) ? PVDR_DATA.at(d) : d;
					string value = l.substr(2);
					data[key].push_back(value);
				}
				ch.data = data;

				if (db.services.count(chid))
				{
					int m;
					string kchid = 's' + chid;
					if (ch.snum) m = ch.snum;
					else m = collisions[kchid].size();
					chid += ':' + to_string(m);
					collisions[kchid].emplace_back(pair (chid, m)); //C++ 17
				}

				db.services.emplace(chid, ch);
				index["chs"].emplace_back(pair (sidx, chid)); //C++ 17
				count = 0;
				chid = "";
				service ch;
			}
		}
	}
}

//TODO stype DVB-T & DVB-C
void e2db_parser::parse_e2db_lamedb5(ifstream& flamedb)
{
	debug("e2db_parser", "parse_e2db_lamedb5()");

	bool step;
	int tidx = 0;
	int sidx = 0;
	string line;

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
//			cout << data << " " << params << " " << type << endl;

			transponder tx;
			string txid = "";
			tx.index = tidx;
			tx.ttype = params[0];

			int dvbns;
			int tsid;
			int onid;
			dvbns = NULL;
			tsid = NULL;
			onid = NULL;

			sscanf(data.c_str(), "%08x:%04x:%04x", &dvbns, &tsid, &onid);

			tx.dvbns = to_string(dvbns);
			tx.tsid = to_string(tsid);
			tx.onid = to_string(onid);

			switch (tx.ttype)
			{
				case 's':
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

					sscanf(params.substr(2).c_str(), "%8d:%8d:%1d:%1d:%3d:%1d:%1d:%1d:%1d:%1d:%1d", &freq, &sr, &pol, &fec, &pos, &inv, &flgs, &sys, &mod, &rol, &pil);

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
					//TODO ,... other flags
				break;
				case 't': // DVB-T
					debug("e2db_parser", "parse_e2db_lamedb5()", "txtype", "'t'\tTODO");
				break;
				case 'c': // DVB-C
					debug("e2db_parser", "parse_e2db_lamedb5()", "txtype", "'c'\tTODO");
				break;
			}

			txid = tx.tsid + ':' + tx.onid + ':' + tx.dvbns;
			db.transponders.emplace(txid, tx);
			index["txs"].emplace_back(pair (tidx, txid)); //C++ 17
			tidx += 1;
		}
		// service
		else
		{
//			cout << data << " " << params << " " << type << endl;

			service ch;
			string chid = "";
			string txid = "";
			ch.index = sidx;

			char ssid[5];
			int dvbns;
			int tsid;
			int onid;
			int stype;
			int snum;
			int srcid;
			dvbns = NULL;
			stype = NULL;
			snum = NULL;
			srcid = NULL;

			sscanf(data.c_str(), "%4s:%08x:%04x:%04x:%3d:%4d:%d", ssid, &dvbns, &tsid, &onid, &stype, &snum, &srcid);

			ch.tsid = to_string(tsid);
			ch.onid = to_string(onid);
			ch.dvbns = to_string(dvbns);
			txid = ch.tsid + ':' + ch.onid + ':' + ch.dvbns;
			ch.ssid = string (ssid);
			ch.ssid.erase(0, ch.ssid.find_first_not_of('0'));
			ch.dvbns.erase(0, ch.dvbns.find_first_not_of('0'));
			ch.tsid.erase(0, ch.tsid.find_first_not_of('0'));
			ch.onid.erase(0, ch.onid.find_first_not_of('0'));
			ch.stype = stype;
			ch.snum = snum;
			ch.srcid = srcid;
			chid = ch.ssid + ':' + ch.tsid + ':' + ch.onid + ':' + ch.dvbns;

			size_t delimit = params.rfind('"');
			string chname = params.substr(1, delimit - 1);
			string chdata = params.rfind(',') != string::npos ? params.substr(delimit + 2) : "";

			ch.txid = txid;
			ch.chname = chname; //TODO FIX "bad chars

			//TODO 256
			// !p: provider
			//  c: cache
			//  C: ciad
			//  f: flags
			/*if (! chdata.empty())
			{
				stringstream datas(chdata);
				string l;
				map<char, vector<string>> data;

				while (getline(datas, l, ','))
				{
					char d = l[0];
					char key = PVDR_DATA.count(d) ? PVDR_DATA.at(d) : d;
					string value = l.substr(2);
					data[key].push_back(value);
				}
				ch.data = data;
			}*/

			if (db.services.count(chid))
			{
				int m;
				string kchid = 's' + chid;
				if (ch.snum) m = ch.snum;
				else m = collisions[kchid].size();
				chid += ':' + to_string(m);
				collisions[kchid].emplace_back(pair (chid, m)); //C++ 17
			}

			db.services.emplace(chid, ch);
			index["chs"].emplace_back(pair (sidx, chid)); //C++ 17
			sidx += 1;
		}
	}
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
			// i0:reftype:btype:ssid:tsid:onid:dvbns:?:?:?:
			// i0:reftype:btype:anum:0:0:0:?:?:?:

			line = line.substr(9);
			bool sseq = false;
			char cchid[24];
			int i0, i1;
			char anum[5];
			char ssid[5];
			char tsid[5];
			char onid[5];
			int dvbns;
			i0 = NULL;
			i1 = NULL;
			dvbns = NULL;

			transform(line.begin(), line.end(), line.begin(), [](unsigned char c){ return c == ':' ? ' ' : c; });
			sscanf(upCase(line).c_str(), "%d %d %4s %4s %4s %4s %8d", &i0, &i1, anum, ssid, tsid, onid, &dvbns);

			switch (i1) {
				case 64:  // regular marker
				case 320: // numbered marker
				case 512: // hidden marker
				case 832: // hidden marker ? //TODO
					sseq = false;
					sprintf(cchid, "%d:%d:%s:%s", i0, i1, anum, ssid);
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
			ref.reftype = i1;
			ref.refanum = anum;
			ref.index = idx;

			ub.channels[chid] = ref;
			index[bname].emplace_back(pair (idx, chid)); //C++ 17

			if (sseq)
				index[pname].emplace_back(pair (bouquets[pname].count++, chid)); //C++ 17

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
		cout << "txid: " << x.second.txid << endl;
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
		cout << "btype: " << x.second.btype << endl;
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

map<string, e2db_parser::service> e2db_parser::get_services()
{
	debug("e2db_parser", "get_services()");
	return db.services;
}

pair<map<string, e2db_parser::bouquet>, map<string, e2db_parser::userbouquet>> e2db_parser::get_bouquets()
{
	debug("e2db_parser", "get_bouquets()");
	return pair (bouquets, userbouquets); //C++ 17
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
	if (e2db.count(dbfilename) < 1)
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

void e2db_maker::make_lamedb()
{
	debug("e2db_maker", "make_lamedb()");

	make_lamedb4();
}

void e2db_maker::make_lamedb4()
{
	debug("e2db_maker", "make_lamedb4()");

	stringstream ss;

	//TODO
	unordered_map<char, char> PVDR_DATA_DENUM;
	for (auto & x: PVDR_DATA) PVDR_DATA_DENUM[x.second] = x.first;

	ss << "eDVB services /4/" << endl;

	ss << "transponders" << endl;
	for (auto & x: index["txs"])
	{
		transponder tx = db.transponders[x.second];
		string dvbns = tx.dvbns;
		string tsid = lowCase(tx.tsid);
		string onid = lowCase(tx.onid);
		dvbns.insert(dvbns.begin(), 8 - dvbns.length(), '0');
		tsid.insert(tsid.begin(), 4 - tsid.length(), '0');
		onid.insert(onid.begin(), 4 - onid.length(), '0');

		ss << dvbns;
		ss << ':' << tsid;
		ss << ':' << onid;
		ss << endl;
		ss << '\t' << tx.ttype;
		ss << ' ' << to_string(int (stoi(tx.freq) * 1e3));
		ss << ':' << to_string(int (stoi(tx.sr) * 1e3));
		ss << ':' << tx.pol;
		ss << ':' << tx.fec;
		ss << ':' << tx.pos; //TODO satellites.xml
		ss << ':' << tx.inv;
		if (! tx.flgs.empty())
			ss << ':' << tx.flgs;
		if (tx.sys)
			ss << ':' << tx.sys;
		if (tx.mod)
			ss << ':' << tx.mod;
		if (tx.rol) // DVB-S2 only
			ss << ':' << tx.rol;
		if (tx.pil) // DVB-S2 only
			ss << ':' << tx.pil;
		ss << endl << '/' << endl;
	}
	ss << "end" << endl;

	ss << "services" << endl;
	for (auto & x: index["chs"])
	{
		service ch = db.services[x.second];
		string ssid = lowCase(ch.ssid);
		string dvbns = ch.dvbns;
		string tsid = lowCase(ch.tsid);
		string onid = lowCase(ch.onid);
		ssid.insert(ssid.begin(), 4 - ssid.length(), '0');
		dvbns.insert(dvbns.begin(), 8 - dvbns.length(), '0');
		tsid.insert(tsid.begin(), 4 - tsid.length(), '0');
		onid.insert(onid.begin(), 4 - onid.length(), '0');
		ss << ssid;
		ss << ':' << dvbns;
		ss << ':' << tsid;
		ss << ':' << onid;
		ss << ':' << ch.stype;
		ss << ':' << ch.snum;
		ss << endl << ch.chname << endl;
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
		ss << endl;
	}
	ss << "end" << endl;

	ss << "editor: " << get_editor_string() << endl;
	ss << "datetime: " << get_timestamp();
	e2db_out["lamedb"] = ss.str();
}

void e2db_maker::make_lamedb5()
{
	debug("e2db_maker", "make_lamedb5()");

	stringstream ss;

	//TODO
	unordered_map<char, char> PVDR_DATA_DENUM;
	for (auto & x: PVDR_DATA) PVDR_DATA_DENUM[x.second] = x.first;

	ss << "eDVB services /5/" << endl;

	for (auto & x: index["txs"])
	{
		transponder tx = db.transponders[x.second];
		string dvbns = tx.dvbns;
		string tsid = lowCase(tx.tsid);
		string onid = lowCase(tx.onid);
		dvbns.insert(dvbns.begin(), 8 - dvbns.length(), '0');
		tsid.insert(tsid.begin(), 4 - tsid.length(), '0');
		onid.insert(onid.begin(), 4 - onid.length(), '0');

		ss << 't';
		ss << ':' << dvbns;
		ss << ':' << tsid;
		ss << ':' << onid;
		ss << ',';
		ss << tx.ttype;
		ss << ':' << to_string(int (stoi(tx.freq) * 1e3));
		ss << ':' << to_string(int (stoi(tx.sr) * 1e3));
		ss << ':' << tx.pol;
		ss << ':' << tx.fec;
		ss << ':' << tx.pos; //TODO satellites.xml
		ss << ':' << tx.inv;
		if (! tx.flgs.empty())
			ss << ':' << tx.flgs;
		if (tx.sys)
			ss << ':' << tx.sys;
		if (tx.mod)
			ss << ':' << tx.mod;
		if (tx.rol) // DVB-S2 only
			ss << ':' << tx.rol;
		if (tx.pil) // DVB-S2 only
			ss << ':' << tx.pil;
		ss << endl;
	}

	for (auto & x: index["chs"])
	{
		service ch = db.services[x.second];
		string ssid = lowCase(ch.ssid);
		string dvbns = ch.dvbns;
		string tsid = lowCase(ch.tsid);
		string onid = lowCase(ch.onid);
		ssid.insert(ssid.begin(), 4 - ssid.length(), '0');
		dvbns.insert(dvbns.begin(), 8 - dvbns.length(), '0');
		tsid.insert(tsid.begin(), 4 - tsid.length(), '0');
		onid.insert(onid.begin(), 4 - onid.length(), '0');
		ss << 's';
		ss << ':' << ssid;
		ss << ':' << dvbns;
		ss << ':' << tsid;
		ss << ':' << onid;
		ss << ':' << ch.stype;
		ss << ':' << ch.snum;
		ss << ':' << ch.srcid;
		ss << ',';
		ss << '"' << ch.chname << '"';
		if (! ch.data.empty())
		{
			ss << ',';
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
		}
		ss << endl;
	}

	char datetime[80];
	strftime(datetime, 80, "%F %T %Z", _out_tst);

	ss << "# editor: " << get_editor_string() << endl;
	ss << "# datetime: " << get_timestamp();
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
	ss << endl;
	e2db_out[bname] = ss.str();
}

void e2db_maker::make_userbouquet(string bname)
{
	debug("e2db_maker", "make_userbouquet()", "bname", bname);

	userbouquet ub = userbouquets[bname];
	stringstream ss;

	ss << "#NAME " << ub.name << endl;
	for (auto & x: index[bname])
	{
		e2db_parser::reference cref = userbouquets[bname].channels[x.second];
		ss << "#SERVICE ";
		ss << "1:";
		ss << cref.reftype << ':';
		ss << cref.refanum << ':'; //TODO ("global markers index)
		
		if (db.services.count(x.second))
		{
			e2db_parser::service cdata = db.services[x.second];
			ss << cdata.ssid << ':';
			ss << cdata.tsid << ':';
			ss << cdata.onid << ':';
			ss << cdata.dvbns << ':';
			ss << "0:0:0:";
		}
		else
		{
			ss << "0:0:0:0:0:0:0:0:" << endl;
			ss << "#DESCRIPTION " << cref.refval;
		}
		ss << endl;
	}
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

void e2db_maker::set_index(map<string, vector<pair<int, string>>> index)
{
	debug("e2db_maker", "set_index()");
	this->index = index;
}

void e2db_maker::set_transponders(map<string, e2db_maker::transponder> transponders)
{
	debug("e2db_maker", "set_transponders()");
	db.transponders = transponders;
}

void e2db_maker::set_channels(map<string, e2db_maker::service> services)
{
	debug("e2db_maker", "set_channels()");
	db.services = services;
}

void e2db_maker::set_bouquets(pair<map<string, e2db_maker::bouquet>, map<string, e2db_maker::userbouquet>> bouquets)
{
	debug("e2db_maker", "set_bouquets()");
	this->bouquets = bouquets.first;
	this->userbouquets = bouquets.second;
}

void e2db_maker::tester()
{
	debug("e2db_maker", "tester()");

	begin_transaction();
	make_lamedb();
	make_bouquets();
	make_userbouquets();
	end_transaction();
	write_e2db();
}


e2db::e2db()
{
	debug("e2db");
}

map<string, vector<pair<int, string>>> e2db::get_transponders_index()
{
	debug("e2db", "get_transponders_index()");

	map<string, vector<pair<int, string>>> _index;
	_index["txs"] = index["txs"];

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

	return _index;
}

map<string, vector<pair<int, string>>> e2db::get_packages_index()
{
	debug("e2db", "get_packages_index()");

	map<string, vector<pair<int, string>>> _index;

	for (auto & x: index["chs"])
	{
		service ch = db.services[x.second];

		if (ch.data.count(PVDR_DATA.at('p')))
			_index[ch.data[PVDR_DATA.at('p')][0]].emplace_back(x);
		else
			_index[""].emplace_back(x);
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

//TODO
map<string, vector<pair<int, string>>> e2db::get_encryption_index()
{
	debug("e2db", "get_encryption_index() TODO");
	return index;
}

map<string, vector<pair<int, string>>> e2db::get_az_index()
{
	debug("e2db", "get_az_index()");

	map<string, vector<pair<int, string>>> _index;

	for (auto & x: index["chs"])
	{
		service ch = db.services[x.second];
		_index[ch.chname.substr(0, 1)].emplace_back(x);
	}

	return _index;
}
