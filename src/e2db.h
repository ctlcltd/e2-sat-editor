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
#include <tuple>
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
#endif /* e2db_h */
