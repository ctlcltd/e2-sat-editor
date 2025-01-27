/*!
 * e2-sat-editor/src/e2db/e2db_abstract.h
 * 
 * @link https://github.com/ctlcltd/e2-sat-editor
 * @copyright e2 SAT Editor Team
 * @author Leonardo Laureti
 * @version 1.8.0
 * @license MIT License
 * @license GNU GPLv3 License
 */

#include <ctime>
#include <string>
#include <vector>
#include <map>
#include <unordered_map>
#include <unordered_set>
#include <fstream>

#include "../logger/logger.h"

using std::string, std::pair, std::vector, std::map, std::unordered_map, std::unordered_set;

#ifndef e2db_abstract_h
#define e2db_abstract_h
namespace e2se_e2db
{
struct e2db_abstract : protected e2se::log_factory
{
	public:

		inline static bool OVERWRITE_FILE = false;

		inline static bool FIX_BOUQUETS = true;
		inline static bool MARKER_GLOBAL_INDEX = false;
		inline static string USERBOUQUET_FILENAME_SUFFIX = "dbe";


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
			unknown = 0x0000,
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

		// file origin
		enum FORG {
			filesys,  // filesystem
			fileport, // ftp
			fileblob  // blob
		};

		// status
		enum DSTAT {
			d_null,
			d_read,
			d_write
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
			radio = 2
		};

		// fav entry type
		enum ETYPE {
			ecast = 1,
			efile = 2,   // file enigma 1
			evod = 4097, // default player
			eraw = 4114, // raw service
			egstplayer = 5001,
			eexteplayer3 = 5002,
			eservice = 8193,
			eyoutube = 8139,
			eservice2 = 8739 // eservice dreamos
		};

		// fav entry flag type
		enum ATYPE {
			aaaa = 0,
			marker = 64,
			group = 128,
			alternatives = 134,
			marker_numbered = 320,
			marker_hidden_512 = 512,
			marker_hidden_832 = 832,
			marker_hidden = ATYPE::marker_hidden_512,
			marker_regular = ATYPE::marker,
			bouquet_regular = 7,
			bouquet_hidden_519 = 519
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

		// entry type extended - type
		inline static const unordered_map<int, int> ETYPE_EXT_TYPE = {
			{ETYPE::ecast, ETYPE::ecast},
			{ETYPE::efile, ETYPE::efile},
			{ETYPE::evod, ETYPE::evod},
			{ETYPE::egstplayer, ETYPE::egstplayer},
			{ETYPE::eexteplayer3, ETYPE::eexteplayer3},
			{ETYPE::eservice, ETYPE::eservice},
			{ETYPE::eyoutube, ETYPE::eyoutube}
		};

		// entry type extended - label
		inline static const unordered_map<int, string> ETYPE_EXT_LABEL = {
			{ETYPE::ecast, "[broadcast]"},
			{ETYPE::efile, "[file]"},
			{ETYPE::evod, "[vod]"},
			{ETYPE::eraw, "[raw]"},
			{ETYPE::egstplayer, "[gstreamer]"},
			{ETYPE::eexteplayer3, "[exteplayer3]"},
			{ETYPE::eservice, "[eservice]"},
			{ETYPE::eyoutube, "[youtube]"},
			{ETYPE::eservice2, "[eservice dreamos]"}
		};

		// entry flag type extended - type
		inline static const unordered_map<int, int> ATYPE_EXT_TYPE = {
			{ATYPE::aaaa, ATYPE::aaaa},
			{ATYPE::marker, ATYPE::marker},
			{ATYPE::marker_hidden_512, ATYPE::marker_hidden_512},
			{ATYPE::marker_hidden_832, ATYPE::marker_hidden_832},
			{ATYPE::marker_numbered, ATYPE::marker_numbered},
			{ATYPE::group, ATYPE::group}
		};

		// entry flag type extended - label
		inline static const unordered_map<int, string> ATYPE_EXT_LABEL = {
			{ATYPE::aaaa, "[null]"},
			{ATYPE::marker, "[marker]"},
			{ATYPE::marker_hidden_512, "[marker hidden]"},
			{ATYPE::marker_hidden_832, "[marker hidden]"},
			{ATYPE::marker_numbered, "[marker numbered]"},
			{ATYPE::group, "[group]"},
			{ATYPE::alternatives, "[alternative services]"}
		};

		// bouquet entry flag type extended - type
		inline static const unordered_map<int, int> UTYPE_EXT_TYPE = {
			{ATYPE::bouquet_regular, ATYPE::bouquet_regular},
			{ATYPE::bouquet_hidden_519, ATYPE::bouquet_hidden_519}
		};

		// bouquet entry flag type extended - label
		inline static const unordered_map<int, string> UTYPE_EXT_LABEL = {
			{ATYPE::bouquet_regular, "[bouquet]"},
			{ATYPE::bouquet_hidden_519, "[bouquet hidden]"}
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
			int origin;
			string path;
			string filename;
			string data;
			string mime;
			size_t size;
			string t;

			e2db_file()
			{
				t = timestamp();
			}
			static e2db_file status(const e2db_file& file)
			{
				auto s = e2db_file();
				s.origin = file.origin;
				s.path = file.path;
				s.filename = file.filename;
				s.mime = file.mime;
				s.size = file.size;
				s.t = file.t;
				return s;
			}
		};

		struct service
		{
			string chid;
			int ssid = 0;
			int tsid = 0;
			int onid = 0;
			int dvbns = 0;
			int stype = 0;
			int snum = 0;
			int srcid = 0;
			int index = -1;
			bool parental = false;
			bool locked = false;
			string txid;
			string chname;
			// data <field char, vector<value string>>
			map<char, vector<string>> data;
			int flags = 0; // neutrino 
		};

		struct service_reference
		{
			int ssid = 0;
			int tsid = 0;
			int onid = 0;
			int dvbns = 0;
		};

		struct transponder
		{
			string txid;
			int tsid = 0;
			int onid = 0;
			int dvbns = 0;
			int ytype = 0;
			int freq = 0;
			int sr = -1;
			int pol = -1; // DVB-S
			int fec = -1; // DVB-S
			int hpfec = -1; // DVB-T
			int lpfec = -1; // DVB-T
			int cfec = -1; // DVB-C
			int pos = 0; // DVB-S
			int inv = -1;
			int flags = -1;
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
			int plpid = -1; // DVB-T
			bool optsverb = false;
			bool mispls = false; // DVB-S
			bool t2mi = false; // DVB-S
			int isid = -1; // DVB-S
			int plscode = -1; // DVB-S
			int plsmode = -1; // DVB-S
			int t2mi_plpid = -1; // DVB-S
			int t2mi_pid = -1; // DVB-S
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
			int etype = 0;
			bool marker = false;
			bool stream = false;
			int atype = 0;
			int anum = 0; // service: stype, marker: num || 0, stream: stype || 0
			string url;
			string value;
			bool descrval = true;
			bool inlineval = false;
			service_reference ref;
			int x7 = 0;
			int x8 = 0;
			int x9 = 0;
			int inum = -1;
			int index = -1;
		};

		struct bouquet
		{
			string bname;
			string rname;
			string name;
			string nname;
			int btype;
			// userbouquets <bname string>
			vector<string> userbouquets;
			// services <chid string>
			unordered_set<string> services;
			int index = -1;
		};

		struct userbouquet
		{
			string bname;
			string rname;
			string name;
			string pname;
			int utype = 7;
			// channels <chid string, channel_reference struct>
			unordered_map<string, channel_reference> channels;
			bool parental = false;
			bool hidden = false;
			bool locked = false;
			string sref;
			string order;
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
			int plpid = -1; // DVB-T
			int plsn = -1; // DVB-S
			int plscode = -1; // DVB-S
			int plsmode = -1; // DVB-S
			int isid = -1; // DVB-S
			int mts = -1; // DVB-S
			int t2mi_plpid = -1; // DVB-S
			int t2mi_pid = -1; // DVB-S
			int index = -1;
		};

		struct tunersets_table
		{
			string tnid;
			int ytype = 0;
			string name;
			int flags = -1;
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

		struct zapit_data
		{
			string fname;
			int itype; // 0: services, 1: bouquets
			string charset;
		};

		struct zapit_table
		{
			string zyid;
			int ytype = 0;
			int pos = 0;
			string name;
			int diseqc = -1;
			int uncomtd = -1;
			// transponders <txid string>
			vector<string> transponders;
			int index = -1;
		};

		struct datadb
		{
			int type = 0; // 0: lamedb, 1: zapit
			int version = 0x1224;
			// transponders <txid string, transponder struct>
			unordered_map<string, transponder> transponders;
			// services <chid string, service struct>
			unordered_map<string, service> services;
			// parental lock type
			PARENTALLOCK parental = PARENTALLOCK::blacklist;
			int iservices = 0;
			int imarkers = 0;
			int istreams = 0;
			DSTAT dstat = DSTAT::d_null;
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
		// tnloc <pos int, tnid string>
		map<int, string> tnloc;
		// zxdata <fname string, zapit_data struct>
		unordered_map<string, zapit_data> zxdata;
		// zytables <string zyid, zapit_table struct>
		unordered_map<string, zapit_table> zytables;
		// comments <iname string, vector<comment struct>>
		unordered_map<string, vector<comment>> comments;
		// index <iname string, vector<pair<src-idx||count int, chid string>>>
		unordered_map<string, vector<pair<int, string>>> index;
		// collisions <chid string, vector<pair<chid string, increment int>>>
		unordered_map<string, vector<pair<string, int>>> collisions;
		// changes <chid string, string>
		unordered_map<string, string> changes;

		virtual ~e2db_abstract() = default;
		static string editor_string(int html = 0);
		static string editor_timestamp();
		static string timestamp();
		static FPORTS file_type_detect(string path);
		static string file_mime_value(FPORTS fpi, string path);
		static bool is_valid_transponder(transponder tx);
		static bool is_valid_service(service ch);
		static bool is_valid_marker(channel_reference chref);
		static bool is_valid_stream(channel_reference chref);
		static bool is_valid_channel_reference(channel_reference chref);
		static bool is_valid_tunersets(tunersets tv);
		static bool is_valid_tunersets_table(tunersets_table tn);
		static bool is_valid_tunersets_transponder(tunersets_table tn, tunersets_transponder tntxp);
		static bool is_valid_dvbns(transponder tx);
		static bool is_valid_dvbns(YTYPE ytype, int dvbns, int tsid, int onid, int pos, int freq);
		static bool is_valid_onid(transponder tx);
		static bool is_valid_onid(YTYPE ytype, int onid, int tsid);
		static bool is_valid_tsid(int tsid);
		static bool is_valid_ssid(int ssid);
		static void value_channel_reference(string str, channel_reference& chref, service_reference& ref);
		static string value_reference_id(service ch);
		static string value_reference_id(channel_reference chref);
		static string value_reference_id(channel_reference chref, service ch);
		static int value_service_type(string str);
		static string value_service_type(service ch);
		static string value_service_type(int stype);
		static int value_service_super_type(service ch);
		static int value_service_super_type(int stype);
		static int value_favourite_type(string str);
		static string value_favourite_type(channel_reference chref);
		static string value_favourite_type(int etype);
		static int value_favourite_flag_type(string str);
		static string value_favourite_flag_type(channel_reference chref);
		static string value_favourite_flag_type(int atype);
		static vector<string> value_channel_provider(string str);
		static string value_channel_provider(service ch);
		static string value_channel_provider(map<char, vector<string>> data);
		static vector<string> value_channel_caid(string str, string separator = "|");
		static vector<string> value_channel_cached(string str, string separator = "|");
		static int value_transponder_type(char ty);
		static int value_transponder_type(string str);
		static char value_transponder_type(int yx);
		static char value_transponder_type(YTYPE ttype);
		static string value_transponder_combo(transponder tx);
		static string value_transponder_combo(tunersets_transponder tntxp, tunersets_table tn);
		static string value_transponder_onid(int onid);
		static int value_transponder_dvbns(string str);
		static int value_transponder_dvbns(transponder tx);
		static int value_transponder_dvbns(YTYPE ytype, int tsid, int onid, int pos, int freq);
		static string value_transponder_dvbns(int dvbns);
		static int value_transponder_polarization(string str);
		static string value_transponder_polarization(int pol);
		static string value_transponder_sr(int sr);
		static int value_transponder_position(string str);
		static string value_transponder_position(transponder tx);
		static string value_transponder_position(tunersets_table tn);
		static string value_transponder_position(int pos);
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
		static void value_transponder_feopts(string str, transponder& tx);
		static string value_transponder_feopts(transponder tx);
		static int value_bouquet_type(string str);
		static string value_bouquet_type(int btype);
		static void value_markup_attribute(string line, string token, string& key, string& val);
		string get_reference_id(string chid);
		string get_reference_id(channel_reference chref);
		string get_tuner_name(transponder tx);
		int get_e2db_services_type();
		void set_e2db_services_type(int type);
		int get_e2db_services_version();
		void set_e2db_services_version(int version);
		int get_lamedb_version();
		void set_lamedb_version(int ver);
		int get_zapit_version();
		void set_zapit_version(int ver);
		virtual string get_filepath() { return this->filepath; }
		virtual string get_services_filename() { return this->services_filename; }
		void set_index(unordered_map<string, vector<pair<int, string>>> index);
		unordered_map<string, vector<pair<int, string>>> get_index();
		void set_transponders(unordered_map<string, transponder> transponders);
		unordered_map<string, transponder> get_transponders();
		unordered_map<string, service> get_services();
		void set_services(unordered_map<string, service> services);
		void set_bouquets(pair<unordered_map<string, bouquet>, unordered_map<string, userbouquet>> bouquets);
		pair<unordered_map<string, bouquet>, unordered_map<string, userbouquet>> get_bouquets();
		virtual unordered_map<string, e2db_file> get_input() { return this->e2db_in; }
		virtual unordered_map<string, e2db_file> get_output() { return this->e2db_out; }
		virtual void fix_bouquets(bool uniq_ubouquets = false);
		virtual void merge(e2db_abstract* dst);
		virtual void dump();

	protected:

		inline static int LAMEDB_VER = -1;
		inline static int ZAPIT_VER = -1;

		void add_transponder(int idx, transponder& tx);
		void add_service(int idx, service& ch);
		void add_bouquet(int idx, bouquet& bs);
		void add_userbouquet(int idx, userbouquet& ub);
		void add_channel_reference(int idx, userbouquet& ub, channel_reference& chref, service_reference& ref);
		void set_channel_reference_description(userbouquet& ub, channel_reference& chref, string value);
		void add_tunersets(tunersets& tv);
		void add_tunersets_table(int idx, tunersets_table& tn, tunersets& tv);
		void add_tunersets_transponder(int idx, tunersets_transponder& tntxp, tunersets_table& tn);
		void set_parentallock(string chid, string bname = "");
		void transform_crlf(e2db_file& file);
		bool check_crlf();
		bool check_crlf(bool& ctx, string& line);
		static void fix_crlf(string& line);

		using MSG = e2se::logger::MSG;
		virtual string msg(string str, string param) { return e2se::logger::msg(str, param); }
		virtual string msg(string str) { return e2se::logger::msg(str); }
		virtual string msg(e2se::logger::MSG msg, const char* param) { return e2se::logger::msg(msg, param); }
		virtual string msg(e2se::logger::MSG msg) { return e2se::logger::msg(msg); }

		// e2db_files <e2db_file>
		vector<e2db_file> e2db_files;
		// e2db_in <filename string, e2db_file>
		unordered_map<string, e2db_file> e2db_in;
		// e2db_out <filename string, e2db_file>
		unordered_map<string, e2db_file> e2db_out;
		string filepath;
		string services_filename;

		bool blobctx_crlf = false;
		bool currctx_crlf = false;
};
}
#endif /* e2db_abstract_h */
