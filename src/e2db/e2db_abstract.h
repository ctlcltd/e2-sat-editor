/*!
 * e2-sat-editor/src/e2db/e2db_abstract.h
 * 
 * @link https://github.com/ctlcltd/e2-sat-editor
 * @copyright e2 SAT Editor Team
 * @author Leonardo Laureti
 * @version 0.1
 * @license MIT License
 * @license GNU GPLv3 License
 */

#include <ctime>
#include <string>
#include <vector>
#include <map>
#include <unordered_map>
#include <fstream>
#include <iostream>

#include "../logger/logger.h"

using std::string, std::pair, std::make_pair, std::vector, std::map, std::unordered_map;

#ifndef e2db_abstract_h
#define e2db_abstract_h
namespace e2se_e2db
{
using e2db_file = std::string;

struct e2db_abstract
{
	public:

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
			int sr;
			int pol; // DVB-S
			int fec; // DVB-S
			int hpfec; // DVB-T
			int lpfec; // DVB-T
			int cfec; // DVB-C
			int pos; // DVB-S
			int inv;
			int flgs; // DVB-S
			int sys; // ? DVB-S
			int mod; // ? DVB-S
			int tmod; // DVB-T
			int cmod; // DVB-C
			int amod; // ATSC
			int rol; // DVB-S2
			int pil; // DVB-S2
			int band; // DVB-T
			int tmx; // DVB-T
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
		struct tunersets_transponder
		{
			string trid;
			int freq;
			int sr;
			int pol; // DVB-S
			int fec; // DVB-S
			int hpfec; // DVB-T
			int lpfec; // DVB-T
			int cfec; // DVB-C
			int inv;
			int sys;
			int mod; // DVB-S
			int tmod; // DVB-T
			int cmod; // DVB-C
			int amod; // ATSC
			int rol; // DVB-S2
			int pil; // DVB-S2
			int band; // DVB-T
			int tmx; // DVB-T
			int guard; // DVB-T
			int hier; // DVB-T
			int isid; // DVB-S
			int mts; // ? DVB-S
			int plsmode; // DVB-S
			int plscode; // DVB-S
			int plsn; // ? DVB-S
			int index;
		};
		struct tunersets_table
		{
			string tnid;
			int ytype;
			string name;
			int flgs;
			int pos; // DVB-S
			string country;
			int feed;
			// transponders <trid string, tunersets_transponder struct>
			unordered_map<string, tunersets_transponder> transponders;
			int index;
		};
		struct tunersets
		{
			int ytype;
			string charset;
			// tables <tnid string, tunersets_table struct>
			unordered_map<string, tunersets_table> tables;
		};
		struct lamedb
		{
			// transponders <txid string, transponder struct>
			unordered_map<string, transponder> transponders;
			// services <chid string, service struct>
			unordered_map<string, service> services;
		};
		struct comment
		{
			int type; // 0: inline, 1: multiline
			int ln; // line
			string text;
		};
		lamedb db;
		// bouquets <bname string, bouquet struct>
		unordered_map<string, bouquet> bouquets;
		// userbouquets <bname string, userbouquet struct>
		unordered_map<string, userbouquet> userbouquets;
		// tunersets <tvid int, tunersets struct>
		unordered_map<int, tunersets> tuners;
		// tunersets_pos <pos int, tnid string>
		map<int, string> tuners_pos;
		// comments <iname string, vector<comment struct>>
		unordered_map<string, vector<comment>> comments;
		//TODO coherence src-idx||count
		// index <iname string, vector<pair<src-idx||count int, chid string>>>
		unordered_map<string, vector<pair<int, string>>> index;
		// collisions <chid string, vector<pair<chid string, increment int>>>
		unordered_map<string, vector<pair<string, int>>> collisions;
	protected:
		// e2db <filename string, full-path string>
		unordered_map<string, string> e2db;
		inline static int LAMEDB_VER = 0;
		void options();
		virtual void debug(string msg);
		virtual void debug(string msg, string optk, string optv);
		virtual void debug(string msg, string optk, int optv);
		virtual void debug(string msg, string optk, bool optv);
		virtual void error(string msg);
		virtual void error(string msg, string optk, string optv);
		void add_transponder(int idx, transponder& tx);
		void add_service(int idx, service& ch);
		void add_bouquet(int idx, bouquet& bs);
		void add_userbouquet(int idx, userbouquet& ub);
		void add_channel_reference(int idx, userbouquet& ub, channel_reference& chref, service_reference& ref);
		void set_channel_reference_marker_value(userbouquet& ub, string chid, string value);
		void add_tunersets(tunersets& tv);
		void add_tunersets_table(int idx, tunersets& tv, tunersets_table& tn);
		void add_tunersets_transponder(int idx, tunersets_table& tn, tunersets_transponder& tntxp);
		e2se::logger* log;
};
}
#endif /* e2db_abstract_h */
