/*!
 * e2-sat-editor/src/e2db.h
 * 
 * @author Leonardo Laureti
 * @version 0.1
 * @license MIT License
 */

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <unordered_map>

using namespace std;

#ifndef e2db_h
#define e2db_h
struct e2db_abstract
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
		//TODO reference "reserved
		struct reference
		{
			string chid;
			int reftype;
			string refval;
			int index;
		};
		struct bouquet
		{
			string bname;
			string name;
			string nname;
			vector<string> userbouquets;
			int count;
		};
		struct userbouquet
		{
			string bname;
			string name;
			string pname;
			map<string, reference> channels;
		};
		struct lamedb {
			map<string, transponder> transponders;
			map<string, service> services;
		};
		map<string, vector<pair<int, string>>> index;
	protected:
		unordered_map<string, string> e2db;
		lamedb db;
		map<string, bouquet> bouquets;
		map<string, userbouquet> userbouquets;
};

class e2db_parser : public e2db_abstract
{
	public:
		void parse_e2db();
		void parse_e2db_lamedb(ifstream& flamedb);
		void parse_e2db_lamedb4(ifstream& flamedb);
		void parse_e2db_lamedb5(ifstream& flamedb);
		void parse_e2db_bouquet(ifstream& fbouquet, string bname);
		void parse_e2db_userbouquet(ifstream& fuserbouquet, string bname, string pname);
		map<string, transponder> get_transponders();
		map<string, service> get_channels();
		pair<map<string, bouquet>, map<string, userbouquet>> get_bouquets();
		bool read_localdir(string localdir);
		bool read(string localdir);
		void debugger();
		e2db_parser()
		{
			lamedb db;
			map<string, bouquet> bouquets;
			map<string, userbouquet> userbouquets;
		}
	protected:
		string localdir;
};

class e2db_maker : public e2db_abstract
{
	void make_lamedb();
	void make_lamedb4();
	void make_lamedb5();
	void make_bouquet();
	void make_userbouquet();
	void write_e2db();
	public:
		e2db_maker()
		{
			debug("e2db_maker");
		}
};

class e2db : e2db_parser
{
	void merge();
	public:
		e2db()
		{
			debug("e2db");
		}
};
#endif /* e2db_h */
