/*!
 * e2-sat-editor/src/e2db.h
 * 
 * @author Leonardo Laureti
 * @version 0.1
 * @license MIT License
 */

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <unordered_map>
#include <ctime>

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
			int srcid;
			int index;
			string txid;
			string chname;
			map<char, vector<string>> data;
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
			int index;
		};
		struct reference
		{
			string chid;
			int reftype;
			string refval;
			string refanum;
			int index;
		};
		struct bouquet
		{
			string bname;
			string name;
			string nname;
			int btype;
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
		struct tuner_reference
		{
			string freq;
			string sr;
			int pol;
			int fec;
			int mod;
			int rol;
			int pil;
			int inv;
			int sys;
			int isid;
			int plsmode;
			int plscode;
		};
		struct tuner_sets
		{
			int ytype;
			string name;
			int flgs;
			int pos;
			map<string, tuner_reference> references;
		};
		struct lamedb {
			map<string, transponder> transponders;
			map<string, service> services;
		};
		lamedb db;
		map<string, bouquet> bouquets;
		map<string, userbouquet> userbouquets;
		unordered_map<int, tuner_sets> tuners;
		map<string, vector<pair<int, string>>> index;
		map<string, vector<pair<string, int>>> collisions;
	protected:
		unordered_map<string, string> e2db;
};

class e2db_parser : virtual public e2db_abstract
{
	public:
		e2db_parser();
		void parse_e2db();
		void parse_e2db_lamedb(ifstream& flamedb);
		void parse_e2db_lamedb4(ifstream& flamedb);
		void parse_e2db_lamedb5(ifstream& flamedb);
		void parse_e2db_bouquet(ifstream& fbouquet, string bname);
		void parse_e2db_userbouquet(ifstream& fuserbouquet, string bname, string pname);
		map<string, transponder> get_transponders();
		map<string, service> get_services();
		pair<map<string, bouquet>, map<string, userbouquet>> get_bouquets();
		bool read_localdir(string localdir);
		bool read(string localdir);
		void parse_tunersets_xml(int ytype, ifstream& ftunxml);
		void debugger();
	protected:
		string localdir;
		string dbfilename;
};

class e2db_maker : virtual public e2db_abstract
{
	public:
		e2db_maker();
		void begin_transaction();
		void end_transaction();
		string get_timestamp();
		string get_editor_string();
		void make_lamedb();
		void make_lamedb4();
		void make_lamedb5();
		void make_bouquets();
		void make_userbouquets();
		void make_bouquet(string bname);
		void make_userbouquet(string bname);
		void write_e2db();
		void set_index(map<string, vector<pair<int, string>>> index);
		void set_transponders(map<string, transponder> transponders);
		void set_channels(map<string, service> services);
		void set_bouquets(pair<map<string, bouquet>, map<string, userbouquet>> bouquets);
		void tester();
	private:
		unordered_map<string, string> e2db_out;
		tm* _out_tst;
};

class e2db : public e2db_parser, public e2db_maker
{
	public:
		e2db();
		void merge();
		map<string, vector<pair<int, string>>> get_transponders_index();
		map<string, vector<pair<int, string>>> get_services_index();
		map<string, vector<pair<int, string>>> get_bouquets_index();
		map<string, vector<pair<int, string>>> get_userbouquets_index();
		map<string, vector<pair<int, string>>> get_packages_index();
		map<string, vector<pair<int, string>>> get_resolution_index();
		map<string, vector<pair<int, string>>> get_encryption_index();
		map<string, vector<pair<int, string>>> get_az_index();
};
#endif /* e2db_h */
