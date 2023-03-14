/*!
 * e2-sat-editor/src/e2db/e2db_abstract.h
 * 
 * @link https://github.com/ctlcltd/e2-sat-editor
 * @copyright e2 SAT Editor Team
 * @author Leonardo Laureti
 * @version 0.5
 * @license MIT License
 * @license GNU GPLv3 License
 */

#include <ctime>
#include <string>
#include <vector>
#include <map>
#include <unordered_map>
#include <fstream>

#include "../logger/logger.h"

using std::string, std::pair, std::vector, std::map, std::unordered_map;

#ifndef e2db_abstract_h
#define e2db_abstract_h
namespace e2se_e2db
{
struct e2db_abstract : protected e2se::log_factory
{
	public:

		inline static bool OVERWRITE_FILE = false;

		inline static const string SAT_POL[4] = {"H", "V", "L", "R"};
		inline static const string SAT_MOD[6] = {"Auto", "QPSK", "8PSK", "QAM16", "16APSK", "32APSK"};
		inline static const string SAT_INV[3] = {"Off", "On", "Auto"};
		inline static const string SAT_FEC[11] = {"Auto", "1/2", "2/3", "3/4", "5/6", "7/8", "8/9", "3/5", "4/5", "9/10", "6/7"};
		inline static const string SAT_ROL[3] = {"0.35", "0.25", "0.20"};
		inline static const string SAT_PIL[3] = {"Off", "On", "Auto"};
		inline static const string SAT_SYS[3] = {"DVB-S", "DVB-S2", "DVB-S/S2"};

		inline static const string TER_BAND[7] = {"8MHz", "7MHz", "6MHz", "Auto", "5MHz", "1.712MHz", "10MHz"};
		inline static const string TER_MOD[5] = {"QPSK", "QAM16", "QAM64", "Auto", "QAM256"};
		inline static const string TER_FEC[10] = {"1/2", "2/3", "3/4", "5/6", "7/8", "Auto", "6/7", "8/9", "3/5", "4/5"};
		inline static const string TER_INV[3] = {"Off", "On", "Auto"};
		inline static const string TER_GUARD[8] = {"1/32", "1/16", "1/8", "1/4", "Auto", "1/128", "19/128", "19/256"};
		inline static const string TER_HIER[5] = {"0", "1", "2", "4", "Auto"};
		inline static const string TER_TMXMODE[7] = {"2k", "8k", "Auto", "4k", "1k", "16k", "32k"};
		inline static const string TER_SYS[3] = {"DVB-T", "DVB-T2", "DVB-T/T2"};

		inline static const string CAB_MOD[6] = {"Auto", "QAM16", "QAM32", "QAM64", "QAM128", "QAM256"};
		inline static const string CAB_INV[3] = {"Off", "On", "Auto"};
		inline static const string CAB_FEC[7] = {"Auto", "1/2", "2/3", "3/4", "5/6", "7/8", "8/9"};

		inline static const string ATS_MOD[8] = {"Auto", "QAM16", "QAM32", "QAM64", "QAM128", "QAM256", "8VSB", "16VSB"};
		inline static const string ATS_INV[3] = {"Off", "On", "Auto"};
		inline static const string ATS_SYS[2] = {"ATSC", "DVB-C ANNEX B"};

		// import / export enum
		enum FPORTS {
			unsupported = 0x0000,
			directory = 0x0001,
			all_services = 0x1000,
			all_services__2_4 = 0x1224,
			all_services__2_5 = 0x1225,
			all_services__2_3 = 0x1223,
			all_services__2_2 = 0x1222,
			all_services_xml = 0x1010,
			all_services_xml__4 = 0x1014,
			all_services_xml__3 = 0x1013,
			all_services_xml__2 = 0x1012,
			all_services_xml__1 = 0x1011,
			all_bouquets = 0x2000,
			all_bouquets_epl = 0x2020,
			all_userbouquets = 0x4000,
			all_bouquets_xml = 0x4010,
			all_bouquets_xml__4 = 0x4014,
			all_bouquets_xml__3 = 0x4013,
			all_bouquets_xml__2 = 0x4012,
			all_bouquets_xml__1 = 0x4011,
			all_tunersets = 0x8000,
			single_bouquet = 0x0002,
			single_bouquet_epl = 0x0020,
			single_bouquet_all = 0x0040,
			single_bouquet_all_epl = 0x0400,
			single_userbouquet = 0x0004,
			single_tunersets = 0x0008,
			single_parentallock_locked = 0xff,
			single_parentallock_blacklist = 0xfa,
			single_parentallock_whitelist = 0xfe
		};

		// tuner settings type
		enum YTYPE {
			satellite,
			terrestrial,
			cable,
			atsc
		};

		// service type
		enum STYPE {
			data = 0,
			tv = 1,
			radio = 2,
			marker = 64,
			group = 128,
			hidden_marker_1 = 512,
			hidden_marker_2 = 832,
			numbered_marker = 320,
			hidden_marker = STYPE::hidden_marker_1,
			regular_marker = STYPE::marker
		};

		// parental lock
		enum PARENTALLOCK {
			blacklist,
			whitelist,
			locked
		};

		// service type extended - type
		inline static const unordered_map<int, int> STYPE_EXT_TYPE = {
			{STYPE::data, STYPE::data},
			{STYPE::tv, STYPE::tv},
			{STYPE::radio, STYPE::radio},
			{10, STYPE::radio},
			{12, STYPE::tv},
			{17, STYPE::tv},
			{22, STYPE::tv},
			{25, STYPE::tv},
			{31, STYPE::tv}
		};

		// service type extended - label
		inline static const unordered_map<int, string> STYPE_EXT_LABEL = {
			{STYPE::data, "Data"},
			{STYPE::tv, "TV"},
			{STYPE::radio, "Radio"},
			{10, "Radio"},
			{12, "TV"},
			{17, "UHD"},
			{22, "H.264"},
			{25, "HD"},
			{31, "H.265"}
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

		//TODO improve
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
			fpid = 4,  // dxNoDVB
			fname = 8, // dxHoldName
			fnew = 64  // dxNewFound
		};

		// service data cache PIDs
		// 00 video VPID
		// 01 mpeg audio APID MPEGAPID
		// 02 teletext TPID TXTPID
		// 03 pcr PPID PCRPID
		// 04 ac3 3PID AC3PID
		// 05 video type
		// 06 audio channel
		// 06 ac3 delay
		// 07 pcm delay
		// 08 subtitle
		// 09 audio type
		// 10 audio APID
		// 11 cache max
		// 16 pmt PMTPID
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
			cmax,
			pmt = 16
		};

		struct e2db_file
		{
			string data;
			string mime;
			string filename;
			size_t size;
		};

		struct service
		{
			string chid;
			int ssid = 0;
			int dvbns = 0;
			int tsid = 0;
			int onid = 0;
			int stype = 0;
			int snum = 0;
			int srcid = -1;
			int index = -1;
			bool locked = false;
			string txid;
			string chname;
			// data <field char, vector<value string>>
			map<char, vector<string>> data;
		};

		struct service_reference
		{
			int ssid = 0;
			int dvbns = 0;
			int tsid = 0;
			int onid = 0;
		};

		struct transponder
		{
			string txid;
			int dvbns = 0;
			int tsid = 0;
			int onid = 0;
			int ytype = 0;
			int freq = 0;
			int sr = -1;
			int pol = -1; // DVB-S
			int fec = -1; // DVB-S
			int hpfec = -1; // DVB-T
			int lpfec = -1; // DVB-T
			int cfec = -1; // DVB-C
			int pos = -1; // DVB-S
			int inv = -1;
			int flgs = -1; // DVB-S
			int sys = -1;
			int mod = -1; // DVB-S
			int tmod = -1; // DVB-T
			int cmod = -1; // DVB-C
			int amod = -1; // ATSC
			int rol = -1; // DVB-S2
			int pil = -1; // DVB-S2
			int band = -1; // DVB-T
			int tmx = -1; // DVB-T
			int guard = -1; // DVB-T
			int hier = -1; // DVB-T
			string oflgs; // ?
			int index = -1;
		};

		struct fec
		{
			int inner_fec = -1;
			int hp_fec = -1;
			int lp_fec = -1;
		};

		struct channel_reference
		{
			string chid;
			bool marker = false;
			int atype;
			int anum;
			string value;
			int index = -1;
		};

		struct bouquet
		{
			string bname;
			string name;
			string nname;
			int btype;
			// userbouquets <bname string>
			vector<string> userbouquets;
			int index = -1;
		};

		struct userbouquet
		{
			string bname;
			string name;
			string pname;
			// channels <chid string, channel_reference struct>
			unordered_map<string, channel_reference> channels;
			bool locked = false;
			bool hidden = false;
			int index = -1;
		};

		struct table
		{
			string name;
			int pos = -1;
			int diseqc = -1;
			int uncomtd = -1;
			// transponders <txid string>
			vector<string> transponders;
			int index = -1;
		};

		struct tunersets_transponder
		{
			string trid;
			int freq = 0;
			int sr = -1;
			int pol = -1; // DVB-S
			int fec = -1; // DVB-S
			int hpfec = -1; // DVB-T
			int lpfec = -1; // DVB-T
			int cfec = -1; // DVB-C
			int inv = -1;
			int sys = -1;
			int mod = -1; // DVB-S
			int tmod = -1; // DVB-T
			int cmod = -1; // DVB-C
			int amod = -1; // ATSC
			int rol = -1; // DVB-S2
			int pil = -1; // DVB-S2
			int band = -1; // DVB-T
			int tmx = -1; // DVB-T
			int guard = -1; // DVB-T
			int hier = -1; // DVB-T
			int isid = -1; // DVB-S
			int mts = -1; // DVB-S
			int plsmode = -1; // DVB-S
			int plscode = -1; // DVB-S
			int plsn = -1; // DVB-S
			int index = -1;
		};

		struct tunersets_table
		{
			string tnid;
			int ytype = 0;
			string name;
			int flgs = -1;
			int pos = -1; // DVB-S
			string country;
			int feed = -1;
			// transponders <trid string, tunersets_transponder struct>
			unordered_map<string, tunersets_transponder> transponders;
			int index = -1;
		};

		struct tunersets
		{
			int ytype;
			string charset;
			// tables <tnid string, tunersets_table struct>
			unordered_map<string, tunersets_table> tables;
		};

		struct datasets
		{
			string dname;
			int itype; // 0: services, 1: bouquets
			string charset;
		};

		struct datadb
		{
			int type = 0; // 0: lamedb, 1: zapit
			// tables <pos int, table struct>
			unordered_map<int, table> tables;
			// transponders <txid string, transponder struct>
			unordered_map<string, transponder> transponders;
			// services <chid string, service struct>
			unordered_map<string, service> services;
			// parental lock type
			PARENTALLOCK parental;
		};

		struct comment
		{
			int type; // 0: inline, 1: multiline
			int ln; // line
			string text;
		};

		datadb db;
		// bouquets <bname string, bouquet struct>
		unordered_map<string, bouquet> bouquets;
		// userbouquets <bname string, userbouquet struct>
		unordered_map<string, userbouquet> userbouquets;
		// tunersets <tvid int, tunersets struct>
		unordered_map<int, tunersets> tuners;
		// tunersets_pos <pos int, tnid string>
		map<int, string> tuners_pos;
		// datasets <dname string, datasets struct>
		unordered_map<string, datasets> datas;
		// comments <iname string, vector<comment struct>>
		unordered_map<string, vector<comment>> comments;
		// index <iname string, vector<pair<src-idx||count int, chid string>>>
		unordered_map<string, vector<pair<int, string>>> index;
		// collisions <chid string, vector<pair<chid string, increment int>>>
		unordered_map<string, vector<pair<string, int>>> collisions;

		virtual ~e2db_abstract() = default;
		static string editor_string(int html = 0);
		static string editor_timestamp();
		static FPORTS filetype_detect(string path);
		static void value_channel_reference(string str, channel_reference& chref, service_reference& ref);
		static string value_reference_id(service ch);
		static string value_reference_id(channel_reference chref);
		static string value_reference_id(channel_reference chref, service ch);
		static int value_service_type(string str);
		static string value_service_type(service ch);
		static string value_service_type(int stype);
		static int value_service_super_type(service ch);
		static int value_service_super_type(int stype);
		static vector<string> value_channel_provider(string str);
		static string value_channel_provider(service ch);
		static string value_channel_provider(map<char, vector<string>> data);
		static vector<string> value_channel_caid(string str);
		static vector<string> value_channel_cached(string str);
		static int value_transponder_type(char ty);
		static int value_transponder_type(string str);
		static char value_transponder_type(int yx);
		static char value_transponder_type(YTYPE ttype);
		static string value_transponder_combo(transponder tx);
		static string value_transponder_combo(tunersets_transponder tntxp, tunersets_table tn);
		static int value_transponder_dvbns(string str);
		static int value_transponder_dvbns(transponder tx);
		static int value_transponder_dvbns(int pos, int onid, int freq, int pol);
		static int value_transponder_dvbns(int pos, int onid);
		static string value_transponder_dvbns(int dvbns);
		static int value_transponder_polarization(string str);
		static string value_transponder_polarization(int pol);
		static string value_transponder_sr(int sr);
		static int value_transponder_position(string str);
		static string value_transponder_position(transponder tx);
		static string value_transponder_position(tunersets_table tn);
		static string value_transponder_position(int num);
		static int value_transponder_system(string str);
		static string value_transponder_system(transponder tx);
		static string value_transponder_system(int sys, int yx);
		static string value_transponder_system(int sys, YTYPE ytype);
		static void value_transponder_fec(string str, int yx, fec& fec);
		static int value_transponder_fec(string str, int yx);
		static int value_transponder_fec(string str, YTYPE ytype);
		static string value_transponder_fec(int fec, int yx);
		static string value_transponder_fec(int fec, YTYPE ytype);
		static int value_transponder_modulation(string str, int yx);
		static int value_transponder_modulation(string str, YTYPE ytype);
		static string value_transponder_modulation(int mod, int yx);
		static string value_transponder_modulation(int mod, YTYPE ytype);
		static int value_transponder_inversion(string str, int yx);
		static int value_transponder_inversion(string str, YTYPE ytype);
		static string value_transponder_inversion(int inv, int yx);
		static string value_transponder_inversion(int inv, YTYPE ytype);
		static int value_transponder_rollof(string str);
		static string value_transponder_rollof(int rol);
		static int value_transponder_pilot(string str);
		static string value_transponder_pilot(int pil);
		static int value_transponder_bandwidth(string str);
		static string value_transponder_bandwidth(int band);
		static int value_transponder_guard(string str);
		static string value_transponder_guard(int guard);
		static int value_transponder_hier(string str);
		static string value_transponder_hier(int hier);
		static int value_transponder_tmx_mode(string str);
		static string value_transponder_tmx_mode(int tmx);
		static int value_bouquet_type(string str);
		static string value_bouquet_type(int btype);
		string get_reference_id(string chid);
		string get_reference_id(channel_reference chref);
		string get_tuner_name(transponder tx);
		int get_e2db_services_type();
		void set_e2db_services_type(int type);
		int get_lamedb_version();
		void set_lamedb_version(int ver);
		int get_zapit_version();
		void set_zapit_version(int ver);
		virtual string get_filepath() { return this->filepath; };
		virtual string get_services_filename() { return this->services_filename; };
		void set_index(unordered_map<string, vector<pair<int, string>>> index);
		unordered_map<string, vector<pair<int, string>>> get_index();
		void set_transponders(unordered_map<string, transponder> transponders);
		unordered_map<string, transponder> get_transponders();
		unordered_map<string, service> get_services();
		void set_services(unordered_map<string, service> services);
		void set_bouquets(pair<unordered_map<string, bouquet>, unordered_map<string, userbouquet>> bouquets);
		pair<unordered_map<string, bouquet>, unordered_map<string, userbouquet>> get_bouquets();
		virtual unordered_map<string, string> get_input() { return this->e2db; };
		virtual unordered_map<string, e2db_file> get_output() { return this->e2db_out; };
		virtual void merge(e2db_abstract* dst);
		virtual void debugger();

	protected:

		inline static int LAMEDB_VER = -1;
		inline static int ZAPIT_VER = -1;

		void add_transponder(int idx, transponder& tx);
		void add_service(int idx, service& ch);
		void add_bouquet(int idx, bouquet& bs);
		void add_userbouquet(int idx, userbouquet& ub);
		void add_channel_reference(int idx, userbouquet& ub, channel_reference& chref, service_reference& ref);
		void set_channel_reference_marker_value(userbouquet& ub, string chid, string value);
		void add_tunersets(tunersets& tv);
		void add_tunersets_table(int idx, tunersets_table& tn, tunersets& tv);
		void add_tunersets_transponder(int idx, tunersets_transponder& tntxp, tunersets_table& tn);
		void set_parentallock(string chid, string bname = "");

		// e2db <filename string, full-path string>
		unordered_map<string, string> e2db;
		// e2db_out <filename string, e2db_file>
		unordered_map<string, e2db_file> e2db_out;
		string filepath;
		string services_filename;
};
}
#endif /* e2db_abstract_h */
