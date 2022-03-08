/*!
 * e2-sat-editor/src/e2db.h
 * 
 * @link https://github.com/ctlcltd/e2-sat-editor
 * @copyright e2 SAT Editor Team
 * @author Leonardo Laureti
 * @version 0.1
 * @license MIT License
 * @license GNU GPLv3 License
 */

#include <string>
#include <vector>
#include <map>
#include <unordered_map>
#include <fstream>
#include <sstream>
#include <iostream>
#include <ctime>

using namespace std;

#ifndef e2db_h
#define e2db_h
struct e2db_abstract
{
	public:
		//C++17

		inline static bool DEBUG = false;
		inline static bool PARSE_TUNERSETS = true;
		inline static bool PARSE_LAMEDB5_PRIOR = false;

		inline static const string SAT_POL[4] = {"H", "V", "L", "R"};
		inline static const string SAT_FEC[11] = {"", "Auto", "1/2", "2/3", "3/4", "5/6", "7/8", "3/5", "4/5", "8/9", "9/10"};
		inline static const string SAT_INV[3] = {"Auto", "On", "Off"};
		inline static const string SAT_SYS[2] = {"DVB-S", "DVB-S2"};
		inline static const string SAT_MOD[4] = {"Auto", "QPSK", "QAM16", "8PSK"};
		inline static const string SAT_ROL[4] = {"Auto", "QPSK", "QAM16", "8PSK"};
		inline static const string SAT_PIL[3] = {"Auto", "Off", "On"};

		inline static const string TER_BAND[4] = {"Auto", "8Mhz", "7Mhz", "6Mhz"};
		inline static const string TER_HPFEC[6] = {"Auto", "1/2", "2/3", "3/4", "5/6", "7/8"};
		inline static const string TER_LPFEC[6] = {"Auto", "1/2", "2/3", "3/4", "5/6", "7/8"};
		inline static const string TER_MOD[4] = {"Auto", "QPSK", "QAM16", "QAM64"};
		inline static const string TER_TRXMODE[5] = {"Auto", "2k", "8k"};
		inline static const string TER_GUARD[5] = {"Auto", "1/32", "1/16", "1/8", "1/4"};
		inline static const string TER_HIER[5] = {"Auto", "", "1", "2", "4"};
		inline static const string TER_INV[3] = {"Auto", "On", "Off"};

		inline static const string CAB_INV[3] = {"Auto", "On", "Off"};
		inline static const string CAB_MOD[6] = {"Auto", "QAM16", "QAM32", "QAM64", "QAM128", "QAM256"};
		inline static const string CAB_IFEC[8] = {"", "Auto", "1/2", "2/3", "3/4", "5/6", "7/8", "8/9"};

		inline static const unordered_map<char, char> PVDR_DATA = {
			{'p', '0'},
			{'c', '1'},
			{'C', '2'},
			{'f', '3'}
		};

		inline static const unordered_map<int, string> STYPES = {
			{0, "Data"},
			{1, "TV"},
			{2, "Radio"},
			{10, "Radio"},
			{12, "TV"},
			{17, "UHD"},
			{22, "H.264"},
			{25, "HD"},
			{31, "UHD"}
		};

		inline static const unordered_map<string, string> SCAS = {
			{"01", "Seca"},
			{"06", "Irdeto"},
			{"05", "Viaccess"},
			{"09", "NDS"},
			{"0b", "Conax"},
			{"0d", "Cryptoworks"},
			{"0e", "PVU"},
			{"18", "Nagra"},
			{"26", "BISS"},
			{"43", "Crypton"},
			{"4a", "DRE"}
		};

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
			string sr; // DVB-S
			int pol; // DVB-S
			int fec; // DVB-S
			int hpfec; // DVB-T
			int lpfec; // DVB-T
			int ifec; // DVB-C
			int pos; // DVB-S
			int inv;
			int flgs; // DVB-S
			int sys; // ? DVB-S
			int mod; // ? DVB-S
			int termod; // DVB-T
			int cabmod; // DVB-C
			int rol; // DVB-S2
			int pil; // DVB-S2
			int band; // DVB-T
			int trxmod; // DVB-T
			int guard; // DVB-T
			int hier; // DVB-T
			string oflgs; // ?
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
		void options();
		void debug(string ns, string cmsg = "", string optk = "", string optv = "", string indt = " ");
		void error(string ns, string cmsg = "", string optk = "", string optv = "", string indt = " ");
		static string lowCase(string str);
		static string upCase(string str);
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
