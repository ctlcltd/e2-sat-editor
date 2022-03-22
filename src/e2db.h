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
namespace e2db
{
struct e2db_abstract
{
	public:
		//C++17

		inline static bool DEBUG = false;
		inline static bool PARSER_TUNERSETS = true;
		inline static bool PARSER_LAMEDB5_PRIOR = false;
		inline static bool MAKER_LAMEDB5 = false;

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
		inline static const unordered_map<char, char> PVDR_DATA_DENUM = {
			{'0', 'p'},
			{'1', 'c'},
			{'2', 'C'},
			{'3', 'f'}
		};

		inline static const unordered_map<int, pair<int, string>> STYPES = {
			{0, make_pair(0, "Data")},
			{1, make_pair(1, "TV")},
			{2, make_pair(2, "Radio")},
			{10, make_pair(2, "Radio")},
			{12, make_pair(1, "TV")},
			{17, make_pair(1, "UHD")},
			{22, make_pair(1, "H.264")},
			{25, make_pair(1, "HD")},
			{31, make_pair(1, "H.265")}
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
			string chid;
			int ssid;
			int dvbns;
			int tsid;
			string onid; //TODO to int
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
			string txid;
			int dvbns;
			int tsid;
			int onid;
			char ttype;
			int freq;
			int sr; // DVB-S
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
			bool refmrker;
			string refval;
			int refanum;
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
			unordered_map<string, reference> channels;
		};
		struct tuner_reference
		{
			int freq;
			int sr;
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
			unordered_map<string, tuner_reference> references;
		};
		struct lamedb {
			unordered_map<string, transponder> transponders;
			unordered_map<string, service> services;
		};
		lamedb db;
		unordered_map<string, bouquet> bouquets;
		unordered_map<string, userbouquet> userbouquets;
		unordered_map<int, tuner_sets> tuners;
		unordered_map<string, vector<pair<int, string>>> index;
		unordered_map<string, vector<pair<string, int>>> collisions;
		void add_transponder(int idx, transponder& tx);
		void add_service(int idx, service& ch);
	protected:
		unordered_map<string, string> e2db;
		void options();
		void debug(string ns, string cmsg = "", string optk = "", string optv = "", string indt = " ");
		void error(string ns, string cmsg = "", string optk = "", string optv = "", string indt = " ");
		static string loCase(string str);
		static string upCase(string str);
		inline static int LAMEDB_VER = 0;
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
		unordered_map<string, transponder> get_transponders();
		unordered_map<string, service> get_services();
		pair<unordered_map<string, bouquet>, unordered_map<string, userbouquet>> get_bouquets();
		bool read_from_localdir(string localdir);
		bool read(string localdir);
		void parse_tunersets_xml(int ytype, ifstream& ftunxml);
		void debugger();
	protected:
		void parse_lamedb_transponder_params(string data, transponder& tx);
		void parse_lamedb_transponder_feparms(string data, char ttype, transponder& tx);
		void parse_lamedb_service_params(string data, service& ch);
		void parse_lamedb_service_params(string data, service& ch, bool add);
		void parse_lamedb_service_data(string data, service& ch);
		string localdir;
		string dbfilename;
};

class e2db_maker : virtual public e2db_abstract
{
	public:
		inline static const string LAMEDB4_FORMATS[13] = {"", "transponders\n", "services\n", "end\n", "", "", "\n\t", " ", "\n/\n", "", "\n", "", "\n"};
		inline static const string LAMEDB5_FORMATS[13] = {"# ", "", "", "", ":", "t", ",", ":", "\n", "s", ",", "\"", "\n"};

		e2db_maker();
		void make_e2db();
		void begin_transaction();
		void end_transaction();
		string get_timestamp();
		string get_editor_string();
		void make_e2db_lamedb();
		void make_e2db_lamedb4();
		void make_e2db_lamedb5();
		void make_e2db_bouquets();
		void make_e2db_userbouquets();
		void set_index(unordered_map<string, vector<pair<int, string>>> index);
		void set_transponders(unordered_map<string, transponder> transponders);
		void set_channels(unordered_map<string, service> services);
		void set_bouquets(pair<unordered_map<string, bouquet>, unordered_map<string, userbouquet>> bouquets);
		bool write_to_localdir(string localdir, bool overwrite);
		bool write(string localdir, bool overwrite);
	protected:
		void make_lamedb(string filename);
		void make_bouquet(string bname);
		void make_userbouquet(string bname);
	private:
		unordered_map<string, string> e2db_out;
		tm* _out_tst;
};

class e2db : public e2db_parser, public e2db_maker
{
	public:
		e2db();
		void merge();
		map<string, vector<pair<int, string>>> get_channels_index();
		map<string, vector<pair<int, string>>> get_transponders_index();
		map<string, vector<pair<int, string>>> get_services_index();
		map<string, vector<pair<int, string>>> get_bouquets_index();
		map<string, vector<pair<int, string>>> get_userbouquets_index();
		map<string, vector<pair<int, string>>> get_packages_index();
		map<string, vector<pair<int, string>>> get_resolution_index();
		map<string, vector<pair<int, string>>> get_encryption_index();
		map<string, vector<pair<int, string>>> get_az_index();
};
}
#endif /* e2db_h */
