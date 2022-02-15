/*!
 * e2-sat-editor/src/e2db.cpp
 * 
 * @author Leonardo Laureti
 * @version 0.1
 * @license MIT License
 */

#include <iostream>
#include <fstream>
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

using namespace std;

/*
string upCase(string str)
{
	int i = 0;
	int len = str.length() + 1;
	char cstr[len];
	std::strcpy(cstr, str.c_str());
	while (i != len)
	{
		cstr[i] = toupper(cstr[i]);
		i++;
	}

	return string (cstr);
}
*/

string lowCase(string str)
{
	transform(str.begin(), str.end(), str.begin(), [](unsigned char c){ return tolower(c); });
	return str;
}

string upCase(string str)
{
	transform(str.begin(), str.end(), str.begin(), [](unsigned char c){ return toupper(c); });
	return str;
}



class e2db_parser
{
	public:
		struct service
		{
			string ssid;
			string dvbns;
			string tsid;
			string onid;
			int stype;
			int snum;
			int index;
			string txid;
			string chname;
			string data;
		};
		struct transponder
		{
			string dvbns;
			string tid;
			string nid;
			string type;
			string data;
		};
		struct bouquet
		{
			string bname;
			string name;
			vector<string> userbouquets;
			map<string, int> channels;
		};
		struct userbouquet
		{
			string bname;
			string name;
			bool orphan;
			map<string, int> channels;
		};
		struct lamedb {
			map<string, transponder> transponders;
			map<string, service> services;
		};
		void parse_e2db();
		void parse_e2db_lamedb(ifstream& flamedb);
		void parse_e2db_bouquet(ifstream& fbouquet, string bname);
		void parse_e2db_userbouquet(ifstream& fuserbouquet, string bname);
		bool get_e2db_localdir(string localdir);
		void load(string localdir);
		void debug();
		string localdir;
		e2db_parser()
		{
			lamedb db;
			map<string, bouquet> bouquets;
			map<string, userbouquet> userbouquets;
		}
	private:
		string _localdir;
		string dirlist;
		unordered_map<string, string> e2db;
		lamedb db;
		map<string, bouquet> bouquets;
		map<string, userbouquet> userbouquets;
};




void e2db_parser::parse_e2db()
{
	cout << "e2db_parser parse_e2db()" << endl;

	ifstream flamedb(e2db["lamedb"]); //TODO ver. 4 & 5
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
			parse_e2db_userbouquet(fuserbouquet, w);
			fuserbouquet.close();
		}
	}
}

//TODO switch stype
void e2db_parser::parse_e2db_lamedb(ifstream& flamedb)
{
	cout << "e2db_parser parse_e2db_lamedb()" << endl;

	int step = 0;
	int count = 0;
	int index = 0;
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
				char tid[5];
				char nid[5];

				//py tx = list(map(lambda a: a.lstrip("0"), line.upper().split(":")))
				sscanf(upCase(line).c_str(), "%8s:%4s:%4s", dvbns, tid, nid);

				tx.dvbns = dvbns;
				tx.dvbns.erase(0, tx.dvbns.find_first_not_of('0'));
				tx.tid = string (tid);
				tx.tid.erase(0, tx.tid.find_first_not_of('0'));
				tx.nid = string (nid);
				tx.nid.erase(0, tx.nid.find_first_not_of('0'));
				txid = tx.tid + ":" + tx.nid + ":" + tx.dvbns;
			}
			else if (count == 2)
			{
				//py txdata = list(map(lambda a: int(a.lstrip("0") or 0), line[3:].split(":")))
				tx.type = line.substr(1, 2);
				tx.data = line.substr(3);
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
				// 0082afc2:0065:0001

				char ssid[5];
				char dvbns[9];
				char tsid[5];
				char onid[5]; //TODO ? onid
				int stype;
				int snum;

				sscanf(upCase(line).c_str(), "%4s:%8s:%4s:%4s:%3d:%4d", ssid, dvbns, tsid, onid, &stype, &snum);

				ch.ssid = string (ssid);
				ch.dvbns = string (dvbns);
				ch.dvbns.erase(0, ch.dvbns.find_first_not_of('0'));
				ch.tsid = string (tsid);
				ch.onid = string (onid);
				ch.onid.erase(0, ch.onid.find_first_not_of('0'));
				ch.stype = stype;
				ch.snum = snum;
				txid = ch.tsid + ":" + ch.onid + ":" + ch.dvbns;
				chid = ch.ssid + ":" + ch.tsid + ":" + ch.onid + ":" + ch.dvbns;
				index += 1;
			}
			else if (count == 2)
			{
				ch.index = index;
				ch.txid = "";
				ch.chname = line;
			}
			else if (count == 3)
			{
				//TODO chdata list
				ch.data = line;

				db.services.emplace(chid, ch);
				count = 0;
				chid = "";
				service ch;
			}
		}
	}
}

void e2db_parser::parse_e2db_bouquet(ifstream& fbouquet, string bname)
{
	cout << "e2db_parser parse_e2db_bouquet()" << endl;

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
		}
	}

	bouquets.emplace(bname, bs);
}

//TODO userbouquet struct
void e2db_parser::parse_e2db_userbouquet(ifstream& fuserbouquet, string bname)
{
	cout << "e2db_parser parse_e2db_userbouquet()" << endl;

	bool step = false;
	int index = 0;
	string line;
	string chid = "";
	userbouquet ub;

	while (getline(fuserbouquet, line))
	{
		if (step && line.find("#SORT") != string::npos)
		{
			step = false;
			continue;
		}
		else if (! step && line.find("#NAME") != string::npos)
		{
			ub.name = line.substr(6);
			step = true;
			continue;
		}
		else if (step && line.find("#DESCRIPTION") != string::npos)
		{
			// ub.channels[chid] = line.substr(13); //TODO FIX index
			continue;
		}

		if (step)
		{
			// 1:0:2:32:2E18:B0:820000:0:0:0:
			// 1:0:2:2CB:1B58:13E:820000:0:0:0:
			// %d:%d:btype:ssid:tsid:onid:dvbns:?:?:?:
			// %d:%d:btype:anum:0:0:0:?:?:?: //TODO 64 is marker ...

			//py line[9:-7]
			line = line.substr(9);
			int i0, i1;
			int btype;
			char ssid[5];
			char tsid[5];
			char onid[5]; //TODO ? onid
			int dvbns;

			transform(line.begin(), line.end(), line.begin(), [](unsigned char c){ return c == ':' ? ' ' : toupper(c); });
			sscanf(line.c_str(), "%d %d %3d %4s %4s %4s %8d", &i0, &i1, &btype, ssid, tsid, onid, &dvbns);

			//py len(line[9:-7].upper().split(':')) > 6
			char c_chid[24];

			//TODO switch btype
			if (i1 != 64)
			{
				index += 1;
				sprintf(c_chid, "%s:%s:%s:%d", ssid, tsid, onid, dvbns);
			}
			else
			{
				sprintf(c_chid, "%d:%d:%d:%s", i0, i1, btype, ssid);
			}

			chid = string (c_chid);

			//TODO ? chid in services and not in userbouquets list
			ub.channels[chid] = index;
		}
	}

	userbouquets.emplace(bname, ub);
}

void e2db_parser::debug()
{
	cout << "db" << endl;
	cout << endl;
	for (auto & x: db.transponders)
	{
		cout << "txid: " << x.first << endl;
		cout << "dvbns: " << x.second.dvbns << endl;
		cout << "tid: " << x.second.tid << endl;
		cout << "nid: " << x.second.nid << endl;
		cout << "type: " << x.second.type << endl;
		cout << "data: " << x.second.data << endl;
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
		cout << "data: " << x.second.data << endl;
		cout << "index: " << x.second.index << endl;
		cout << endl;
	}
	cout << endl;
	for (auto & x: bouquets)
	{
		cout << "filename: " << x.first << endl;
		cout << "name: " << x.second.name << endl;
		cout << "userbouquets: [";
		for (auto & w: x.second.userbouquets)
		{
			cout << "\"" << w << "\",";
		}
		cout << "]" << endl;
		cout << endl;
	}
	cout << endl;
	for (auto & x: userbouquets)
	{
		cout << "bname: " << x.first << endl;
		cout << "name: " << x.second.name << endl;
		cout << "channels: [" << endl;
		for (auto & q: x.second.channels)
		{
			cout << "{\"chid\":\"" << q.first << "\",\"index\":" << q.second << "}," << endl;
		}
		cout << "]" << endl;
		cout << endl;
	}
}

// C++17
bool e2db_parser::get_e2db_localdir(string _localdir)
{
	cout << "e2db_parser get_e2db_localdir() " << _localdir << endl;

	if (! filesystem::exists(_localdir))
	{
		cout << "File not exists: " << _localdir << endl;

		return false;
	}

	filesystem::directory_iterator dirlist(_localdir);

	for (const auto & entry : dirlist)
	{
		//TODO is file & permissions check ...
		string path = entry.path();
		string filename = filesystem::path(path).filename(); //TODO FIX
		e2db[filename] = path;
	}

	return true;
}

void e2db_parser::load(string localdir)
{
	cout << "e2db_parser load() " << localdir << endl;

	get_e2db_localdir(localdir);
	parse_e2db();

	//py chdata = get_channels_data(channels);
	//py txdata = get_transponders_data(channels);
}
