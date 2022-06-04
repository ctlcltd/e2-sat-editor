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
#include <iostream>
#include <ctime>

#include "logger.h"

using std::string, std::pair, std::make_pair, std::vector, std::map, std::unordered_map, std::istream;

#ifndef e2db_h
#define e2db_h
namespace e2se_e2db
{
using e2db_file = std::string;

struct e2db_abstract
{
	public:

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

		// tuner settings type
		enum YTYPE {
			sat,
			terrestrial,
			cable,
			atsc
		};

		//TODO
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

		// service data
		// p provider
		// c cache
		// C CAS
		// f flags
		enum SDATA {
			p,
			c,
			C,
			f
		};

		// service data CAS
		// XXxxx encryption
		inline static const unordered_map<string, string> SDATA_CAS = {
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

		// service data flags
		// f01 fkeep
		// f02 fhide
		// f04 fpid
		// f08 fname
		// f40 fnew
		enum SDATA_FLAGS {
			fkeep = 1, // dxNoSDT
			fhide = 2, // dxDontshow
			fpid  = 4, // dxNoDVB
			fname = 8, // dxHoldName
			fnew  = 64 // dxNewFound
		};

		// service data cache PIDs
		// 00 video VPID
		// 01 mpeg audio APID MPEGAPID
		// 02 tp TPID
		// 03 pcr PCRPID PPID
		// 04 ac3 AC3PID 3PID
		// 05 video type
		// 06 audio channel
		// 06 ac3 delay
		// 07 pcm delay
		// 08 subtitle teletext
		// 09 audio type
		// 10 audio APID
		// 11 cache max
		enum SDATA_PIDS {
			vpid,
			mpegapid,
			tpid,
			pcrpid,
			ac3pid,
			vtype,
			achannel,
			ac3delay,
			pcmdelay,
			subtitle,
			atype,
			apid,
			cmax
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
			// data <field char, vector<value string>>
			map<char, vector<string>> data;
		};
		struct service_reference
		{
			int ssid;
			int dvbns;
			int tsid;
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
		struct channel_reference
		{
			string chid;
			int type;
			bool marker = false;
			string value;
			int anum;
			int index;
		};
		struct bouquet
		{
			string bname;
			string name;
			string nname;
			int btype;
			// userbouquets <bname string>
			vector<string> userbouquets;
			int index;
		};
		struct userbouquet
		{
			string bname;
			string name;
			string pname;
			// channels <chid string, channel_reference struct>
			unordered_map<string, channel_reference> channels;
			int index;
		};
		struct tuner_transponder
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
			// transponders <trid string, tuner_transponder struct>
			unordered_map<string, tuner_transponder> transponders;
		};
		struct lamedb
		{
			// transponders <txid string, transponder struct>
			unordered_map<string, transponder> transponders;
			// services <chid string, service struct>
			unordered_map<string, service> services;
		};
		lamedb db;
		// bouquets <bname string, bouquet struct>
		unordered_map<string, bouquet> bouquets;
		// userbouquets <bname string, userbouquet struct>
		unordered_map<string, userbouquet> userbouquets;
		// tuners <pos int, tuner_sets struct>
		unordered_map<int, tuner_sets> tuners;
		//TODO coherence src-idx||count
		// index <name string, vector<pair<src-idx||count int, chid string>>>
		unordered_map<string, vector<pair<int, string>>> index;
		// collisions <chid string, vector<pair<string chid, increment int>>>
		unordered_map<string, vector<pair<string, int>>> collisions;
		void add_transponder(int idx, transponder& tx);
		void add_service(int idx, service& ch);
		void add_bouquet(int idx, bouquet& bs);
		void add_userbouquet(int idx, userbouquet& ub);
		void add_channel_reference(int idx, userbouquet& ub, channel_reference& chref, service_reference& ch);
		void set_channel_reference_marker_value(userbouquet& ub, string chid, string value);
	protected:
		// e2db <filename string, full-path string>
		unordered_map<string, string> e2db;
		void options();
		inline static int LAMEDB_VER = 0;
		virtual void debug(string cmsg);
		virtual void debug(string cmsg, string optk, string optv);
		virtual void error(string cmsg);
		virtual void error(string cmsg, string optk, string optv);
		e2se::logger* log;
};

class e2db_parser : virtual public e2db_abstract
{
	public:
		e2db_parser();
		virtual ~e2db_parser() = default;
		void parse_e2db();
		void parse_e2db(unordered_map<string, e2db_file> files);
		void parse_e2db_lamedb(istream& ilamedb);
		void parse_e2db_lamedb4(istream& ilamedb);
		void parse_e2db_lamedb5(istream& ilamedb);
		void parse_e2db_bouquet(istream& ibouquet, string bname);
		void parse_e2db_userbouquet(istream& iuserbouquet, string bname);
		void parse_tunersets_xml(int ytype, istream& itunxml);
		unordered_map<string, transponder> get_transponders();
		unordered_map<string, service> get_services();
		pair<unordered_map<string, bouquet>, unordered_map<string, userbouquet>> get_bouquets();
		bool list_localdir(string localdir);
		bool read(string localdir);
		unordered_map<string, string> get_input();
		void debugger();
	protected:
		void parse_lamedb_transponder_params(string data, transponder& tx);
		void parse_lamedb_transponder_feparms(string data, char ttype, transponder& tx);
		void parse_lamedb_service_params(string data, service& ch);
		void parse_lamedb_service_data(string data, service& ch);
		void append_lamedb_service_name(string data, service& ch);
		void parse_userbouquet_reference(string data, userbouquet& ub);
		void parse_channel_reference(string data, channel_reference& chref, service_reference& ref);
		string localdir;
		string dbfilename;
};

class e2db_maker : virtual public e2db_abstract
{
	public:
		inline static const string LAMEDB4_FORMATS[13] = {"", "transponders\n", "services\n", "end\n", "", "", "\n\t", " ", "\n/\n", "", "\n", "", "\n"};
		inline static const string LAMEDB5_FORMATS[13] = {"# ", "", "", "", ":", "t", ",", ":", "\n", "s", ",", "\"", "\n"};

		e2db_maker();
		virtual ~e2db_maker() = default;
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
		unordered_map<string, e2db_file> get_output();
	protected:
		void make_lamedb(string filename);
		void make_bouquet(string bname);
		void make_userbouquet(string bname);
	private:
		// e2db_out <filename string, e2db_file alias(string)>
		unordered_map<string, e2db_file> e2db_out;
		std::tm* _out_tst;
};

class e2db : public e2db_parser, public e2db_maker
{
	public:
		e2db();
		virtual ~e2db() = default;
		void merge(e2db* dbih);
		void import_file();
		void export_file();
		void add_transponder(transponder& tx);
		void edit_transponder(string txid, transponder& tx);
		void remove_transponder(string txid);
		void add_service(service& ch);
		void edit_service(string chid, service& ch);
		void remove_service(string chid);
		void add_bouquet(bouquet& bs);
		void edit_bouquet(bouquet& bs);
		void remove_bouquet(string bname);
		void add_userbouquet(userbouquet& ub);
		void edit_userbouquet(userbouquet& ub);
		void remove_userbouquet(string bname);
		void add_channel_reference(channel_reference& chref, string bname);
		void edit_channel_reference(string chid, channel_reference& chref, string bname);
		void remove_channel_reference(string chid, string bname);
		string get_reference_id(string chid);
		string get_reference_id(channel_reference chref);
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
