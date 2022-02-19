/*!
 * e2-sat-editor/src/e2db.h
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
#include <regex>
#include <cstdio>
#include <cstring>

using namespace std;

#ifndef e2db_h
#define e2db_h
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
			unordered_map<char, vector<string>> data;
		};
		struct transponder
		{
			string dvbns;
			string tsid;
			string onid;
			char ttype;
			string freq;
			string sr;
			int pol;
			int fec;
			int pos;
			int inv;
			string flgs;
			int sys;
			int mod;
			int rol;
			int pil;
		};
		struct reference
		{
			string chid;
			int reftype;
			string refval;
			bool orphan;
			int index;
		};
		struct bouquet
		{
			string bname;
			string name;
			string nname;
			vector<string> userbouquets;
			map<string, reference> channels;
		};
		struct userbouquet
		{
			string bname;
			string name;
			bool orphan;
			map<string, reference> channels;
		};
		struct lamedb {
			map<string, transponder> transponders;
			map<string, service> services;
		};
		void parse_e2db();
		void parse_e2db_lamedb(ifstream& flamedb);
		void parse_e2db_lamedb4(ifstream& flamedb);
		void parse_e2db_bouquet(ifstream& fbouquet, string bname);
		void parse_e2db_userbouquet(ifstream& fuserbouquet, string bname);
		map<string, transponder> get_transponders();
		map<string, service> get_channels();
		pair<map<string, bouquet>, map<string, userbouquet>> get_bouquets();
		bool get_e2db_localdir(string localdir); //TODO rename no getter
		void load(string localdir); //TODO rename
		void debugger();
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
#endif /* e2db_h */
