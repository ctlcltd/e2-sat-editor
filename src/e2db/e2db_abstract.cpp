/*!
 * e2-sat-editor/src/e2db/e2db_abstract.cpp
 * 
 * @link https://github.com/ctlcltd/e2-sat-editor
 * @copyright e2 SAT Editor Team
 * @author Leonardo Laureti
 * @version 0.1
 * @license MIT License
 * @license GNU GPLv3 License
 */

#include <cstdio>
#include <cstring>
#include <algorithm>

#include "e2db_abstract.h"

using std::string, std::pair, std::hex, std::dec, std::to_string, std::cout, std::endl;

namespace e2se_e2db
{

void e2db_abstract::debug(string msg)
{
	this->log->debug(msg);
}

void e2db_abstract::debug(string msg, string optk, string optv)
{
	this->log->debug(msg, optk, optv);
}

void e2db_abstract::debug(string msg, string optk, int optv)
{
	this->log->debug(msg, optk, std::to_string(optv));
}

void e2db_abstract::info(string msg)
{
	this->log->info(msg);
}

void e2db_abstract::info(string msg, string optk, string optv)
{
	this->log->info(msg, optk, optv);
}

void e2db_abstract::info(string msg, string optk, int optv)
{
	this->log->info(msg, optk, std::to_string(optv));
}

void e2db_abstract::error(string msg)
{
	this->log->error(msg);
}

void e2db_abstract::error(string msg, string optk, string optv)
{
	this->log->error(msg, optk, optv);
}

void e2db_abstract::error(string msg, string optk, int optv)
{
	this->log->error(msg, optk, std::to_string(optv));
}

string e2db_abstract::editor_string(bool html)
{
	if (html)
		return "e2 SAT Editor " + to_string(EDITOR_RELEASE) + " <a href=\"https://github.com/ctlcltd/e2-sat-editor\">https://github.com/ctlcltd/e2-sat-editor</a>";
	else
		return "e2 SAT Editor " + to_string(EDITOR_RELEASE) + " <https://github.com/ctlcltd/e2-sat-editor>";
}

string e2db_abstract::editor_timestamp()
{
	std::time_t ct = std::time(0);
	int zh = 0;
	std::tm* lct = std::localtime(&ct);
	zh = lct->tm_hour + lct->tm_isdst;
	char dt[80];
	std::strftime(dt, 80, "%Y-%m-%d %H:%M:%S", lct);
	std::tm* gmt = std::gmtime(&ct);
	zh = (zh - gmt->tm_hour) * 100;
	char tz[7];
	std::sprintf(tz, "%+05d", zh);
	return string (dt) + string (tz);
}

e2db_abstract::FPORTS e2db_abstract::filetype_detect(string path)
{
	string filename = std::filesystem::path(path).filename().u8string(); //C++17

	if (filename == "lamedb")
		return FPORTS::all_services; // autodetect
	else if (filename == "lamedb5")
		return FPORTS::all_services__2_5;
	else if (filename == "services")
		return FPORTS::all_services; // autodetect
	else if (filename == "satellites.xml")
		return FPORTS::single_tunersets;
	else if (filename == "terrestrial.xml")
		return FPORTS::single_tunersets;
	else if (filename == "cables.xml")
		return FPORTS::single_tunersets;
	else if (filename == "atsc.xml")
		return FPORTS::single_tunersets;
	else if (filename.find("bouquets.") != string::npos)
		return FPORTS::single_bouquet;
	else if (filename.find("userbouquet.") != string::npos)
		return FPORTS::single_userbouquet;
	return FPORTS::fports_empty;
}

void e2db_abstract::value_channel_reference(string str, channel_reference& chref, service_reference& ref)
{
	int i, type, anum, ssid, tsid, onid, dvbns;
	i = 0, type = 0, anum = 0, ssid = 0, tsid = 0, onid = 0, dvbns = 0;
	//TODO onid

	std::sscanf(str.c_str(), "%d:%d:%X:%X:%X:%X:%X", &i, &type, &anum, &ssid, &tsid, &onid, &dvbns);
	//TODO other flags ? "...:%d:%d:%d:"

	switch (type)
	{
		// marker
		case STYPE::regular_marker:
		case STYPE::numbered_marker:
		case STYPE::hidden_marker_1:
		case STYPE::hidden_marker_2:
			chref.marker = true;
		break;
		//TODO group
		// group
		case STYPE::group:
		return;
		// service
		default:
			chref.marker = false;
			ref.ssid = ssid;
			ref.dvbns = dvbns;
			ref.tsid = tsid;
			ref.onid = onid;
	}

	chref.type = type;
	chref.anum = anum;
}


string e2db_abstract::value_reference_id(service ch)
{
	int stype = ch.stype != -1 ? ch.stype : 0;
	int snum = ch.snum != -1 ? ch.snum : 0;
	int ssid = ch.ssid;
	int tsid = ch.tsid;
	string onid = ch.onid.empty() ? "0" : ch.onid;
	std::transform(onid.begin(), onid.end(), onid.begin(), [](unsigned char c) { return toupper(c); });
	int dvbns = ch.dvbns;

	char refid[44];
	// %1d:%4d:%4X:%4X:%4X:%4s:%8X:0:0:0:
	std::sprintf(refid, "%d:%d:%X:%4X:%4X:%s:%8X", 1, stype, snum, ssid, tsid, onid.c_str(), dvbns);
	return refid;
}

string e2db_abstract::value_reference_id(channel_reference chref)
{
	int type = chref.type != -1 ? chref.type : 0;
	int anum = chref.anum != -1 ? chref.anum : 0;
	int ssid = 0;
	int tsid = 0;
	string onid = "0";
	int dvbns = 0;

	char refid[44];
	// %1d:%4d:%4X:%4X:%4X:%4s:%8X:0:0:0:
	std::sprintf(refid, "%d:%d:%X:%X:%X:%s:%X:0:0:0", 1, type, anum, ssid, tsid, onid.c_str(), dvbns);
	return refid;
}

string e2db_abstract::value_reference_id(channel_reference chref, service ch)
{
	int type, anum;
	int ssid = 0;
	int tsid = 0;
	string onid = "0";
	int dvbns = 0;

	if (! chref.marker)
	{
		type = ch.stype != -1 ? ch.stype : 0;
		anum = ch.snum != -1 ? ch.snum : 0;
		ssid = ch.ssid;
		tsid = ch.tsid;
		onid = ch.onid.empty() ? onid : ch.onid;
		std::transform(onid.begin(), onid.end(), onid.begin(), [](unsigned char c) { return toupper(c); });
		dvbns = ch.dvbns;
	}
	else
	{
		type = chref.type != -1 ? chref.type : 0;
		anum = chref.anum != -1 ? chref.anum : 0;
	}

	char refid[44];
	// %1d:%4d:%4X:%4X:%4X:%4s:%8X:0:0:0:
	std::sprintf(refid, "%d:%d:%X:%X:%X:%s:%X:0:0:0", 1, type, anum, ssid, tsid, onid.c_str(), dvbns);
	return refid;
}


int e2db_abstract::value_service_type(string str)
{
	if (str == "Data")
		return STYPE::data;
	else if (str == "TV")
		return STYPE::tv;
	else if (str == "Radio")
		return STYPE::radio;
	else if (str == "MARKER")
		return STYPE::marker;
	else if (str == "HD")
		return 25;
	else if (str == "H.264")
		return 22;
	else if (str == "H.265")
		return 31;
	else if (str == "UHD")
		return 17;
	else
		return STYPE::data;
}

//TODO improve
vector<string> e2db_abstract::value_channel_cas(string str)
{
	if (str.find("$") != string::npos)
		str = str.substr(2);

	vector<string> cas;

	char* token = std::strtok(str.data(), ",");
	while (token != 0)
	{
		string val = string (token);
		val.erase(0, val.find_first_not_of(' '));
		string pid;

		if (val == "Seca")
			pid = "0100";
		else if (val == "Irdeto")
			pid = "06ed";
		else if (val == "Viaccess")
			pid = "0500";
		else if (val == "NDS")
			pid = "091f";
		else if (val == "Conax")
			pid = "0b00";
		else if (val == "Cryptoworks")
			pid = "0d00";
		else if (val == "PVU")
			pid = "0e00";
		else if (val == "Nagra")
			pid = "1800";
		else if (val == "BISS")
			pid = "2600";
		else if (val == "Crypton")
			pid = "4347";
		else if (val == "DRE")
			pid = "4ae0";

		token = std::strtok(NULL, ",");
		cas.emplace_back("C:" + pid);
	}
	return cas;
}

string e2db_abstract::value_transponder_combo(transponder tx)
{
	string ptxp;
	switch (tx.ttype)
	{
		case 's':
			ptxp = to_string(tx.freq) + '/' + SAT_POL[tx.pol] + '/' + to_string(tx.sr);
		break;
		case 't':
			ptxp = to_string(tx.freq) + '/' + TER_MOD[tx.tmod] + '/' + TER_BAND[tx.band];
		break;
		case 'c':
			ptxp = to_string(tx.freq) + '/' + CAB_MOD[tx.cmod] + '/' + to_string(tx.sr);
		break;
		case 'a':
			ptxp = to_string(tx.freq);
		break;
	}
	return ptxp;
}

string e2db_abstract::value_transponder_combo(tunersets_transponder tntxp, tunersets_table tn)
{
	string ptxp;
	switch (tn.ytype)
	{
		case YTYPE::sat:
			ptxp = to_string(tntxp.freq) + '/' + SAT_POL[tntxp.pol] + '/' + to_string(tntxp.sr);
		break;
		case YTYPE::terrestrial:
			ptxp = to_string(tntxp.freq) + '/' + TER_MOD[tntxp.tmod] + '/' + TER_BAND[tntxp.band];
		break;
		case YTYPE::cable:
			ptxp = to_string(tntxp.freq) + '/' + CAB_MOD[tntxp.cmod] + '/' + to_string(tntxp.sr);
		break;
		case YTYPE::atsc:
			ptxp = to_string(tntxp.freq);
		break;
	}
	return ptxp;
}

int e2db_abstract::value_transponder_polarization(string str)
{
	switch (str[0])
	{
		case 'H': return 0;
		case 'V': return 1;
		case 'L': return 2;
		case 'R': return 3;
		default: return -1;
	}
}

string e2db_abstract::value_transponder_position(transponder tx)
{
	return value_transponder_position(tx.pos);
}

string e2db_abstract::value_transponder_position(tunersets_table tn)
{
	return value_transponder_position(tn.pos);
}

int e2db_abstract::value_transponder_position(string str)
{
	std::string::size_type pos;
	float posdeg = std::stof(str, &pos);
	char pospoint = str.substr(pos)[0];
	return (int ((pospoint == 'E' ? posdeg : -posdeg) * 10));
}

string e2db_abstract::value_transponder_position(int num)
{
	char cposdeg[6];
	// %3d.%1d%C
	std::sprintf(cposdeg, "%.1f", float (std::abs(num)) / 10);
	return (string (cposdeg) + (num > 0 ? 'E' : 'W'));
}

int e2db_abstract::value_transponder_system(string str)
{
	if (str == "DVB-S")
		return 0;
	else if (str == "DVB-S2")
		return 1;
	else
		return -1;
}

string e2db_abstract::value_transponder_system(transponder tx)
{
	string psys;
	switch (tx.ttype) {
		case 's':
			psys = tx.sys != -1 ? SAT_SYS[tx.sys] : "DVB-S";
		break;
		case 't':
			psys = "DVB-T";
		break;
		case 'c':
			psys = "DVB-C";
		break;
		case 'a':
			psys = "ATSC";
		break;
	}
	return psys;
}

string e2db_abstract::get_reference_id(string chid)
{
	// debug("get_reference_id()", "chid", chid);

	if (db.services.count(chid))
		return value_reference_id(db.services[chid]);
	else
		return "0:0:0:0:0:0:0:0:0:0";
}

string e2db_abstract::get_reference_id(channel_reference chref)
{
	// debug("get_reference_id()", "chref.chid", chref.chid);

	if (! chref.marker && db.services.count(chref.chid))
		return value_reference_id(chref, db.services[chref.chid]);
	else
		return value_reference_id(chref);
}

string e2db_abstract::get_transponder_name_value(transponder tx)
{
	// debug("get_transponder_name_value()", "txid", tx.txid);

	string ppos;
	if (tx.ttype == 's')
	{
		if (tuners_pos.count(tx.pos))
		{
			string tnid = tuners_pos.at(tx.pos);
			tunersets_table tns = tuners[0].tables[tnid];
			ppos = tns.name;
		}
		else
		{
			ppos = value_transponder_position(tx);
		}
	}
	return ppos;
}

void e2db_abstract::add_transponder(int idx, transponder& tx)
{
	char txid[25];
	// %4x:%8x
	std::sprintf(txid, "%x:%x", tx.tsid, tx.dvbns);
	tx.txid = txid;

	tx.index = idx;
	db.transponders.emplace(tx.txid, tx);
	index["txs"].emplace_back(pair (idx, tx.txid)); //C++17
}

void e2db_abstract::add_service(int idx, service& ch)
{
	char chid[25];
	char txid[25];
	// %4x:%8x
	std::sprintf(txid, "%x:%x", ch.tsid, ch.dvbns);
	// %4x:%4x:%8x
	std::sprintf(chid, "%x:%x:%x", ch.ssid, ch.tsid, ch.dvbns);
	ch.txid = txid;
	ch.chid = chid;

	if (db.services.count(ch.chid))
	{
		int m;
		string kchid = 's' + ch.chid;
		if (ch.snum) m = ch.snum;
		else m = collisions[kchid].size();
		ch.chid += ':' + to_string(m);
		collisions[kchid].emplace_back(pair (ch.chid, m)); //C++17
	}

	string iname = "chs:" + (STYPE_EXT_TYPE.count(ch.stype) ? to_string(STYPE_EXT_TYPE.at(ch.stype)) : "0");
	ch.index = idx;
	db.services.emplace(ch.chid, ch);
	index["chs"].emplace_back(pair (idx, ch.chid)); //C++17
	index[iname].emplace_back(pair (idx, ch.chid)); //C++17
}

void e2db_abstract::add_bouquet(int idx, bouquet& bs)
{
	bs.index = idx;
	bouquets.emplace(bs.bname, bs);
	index["bss"].emplace_back(pair (idx, bs.bname)); //C++17
}

void e2db_abstract::add_userbouquet(int idx, userbouquet& ub)
{
	ub.index = idx;
	userbouquets.emplace(ub.bname, ub);
	bouquets[ub.pname].userbouquets.emplace_back(ub.bname);
	index["ubs"].emplace_back(pair (idx, ub.bname)); //C++17
}

void e2db_abstract::add_channel_reference(int idx, userbouquet& ub, channel_reference& chref, service_reference& ref)
{
	char chid[25];

	if (chref.marker)
		// %4d:%2x:%d
		std::sprintf(chid, "%d:%x:%d", chref.type, chref.anum, ub.index);
	else
		// %4x:%4x:%8x
		std::sprintf(chid, "%x:%x:%x", ref.ssid, ref.tsid, ref.dvbns);

	chref.chid = chid;
	chref.index = idx;

	ub.channels.emplace(chref.chid, chref);
	index[ub.bname].emplace_back(pair (idx, chref.chid)); //C++17

	if (chref.marker)
		index["mks"].emplace_back(pair (ub.index, chref.chid)); //C++17
	else
		index[ub.pname].emplace_back(pair ((index[ub.pname].size() + 1), chref.chid)); //C++17
}

void e2db_abstract::set_channel_reference_marker_value(userbouquet& ub, string chid, string value)
{
	ub.channels[chid].value = value;
}

void e2db_abstract::add_tunersets(tunersets& tv)
{
	tuners.emplace(tv.ytype, tv);
}

void e2db_abstract::add_tunersets_table(int idx, tunersets_table& tn, tunersets& tv)
{
	string iname = "tns:";
	char type;
	switch (tn.ytype)
	{
		case YTYPE::sat: type = 's'; break;
		case YTYPE::terrestrial: type = 't'; break;
		case YTYPE::cable: type = 'c'; break;
		case YTYPE::atsc: type = 'a'; break;
		default: return error("add_tunersets_table()", "Error", "Unknown tuner settings type.");
	}
	iname += type;
	char tnid[25];
	std::sprintf(tnid, "%c:%04x", type, idx);
	tn.tnid = tnid;
	tn.index = idx;
	tv.tables.emplace(tn.tnid, tn);
	index[iname].emplace_back(pair (idx, tn.tnid)); //C++17
	if (tn.ytype == YTYPE::sat)
		tuners_pos.emplace(tn.pos, tn.tnid);
}

void e2db_abstract::add_tunersets_transponder(int idx, tunersets_transponder& tntxp, tunersets_table& tn)
{
	char type;
	switch (tn.ytype)
	{
		case YTYPE::sat: type = 's'; break;
		case YTYPE::terrestrial: type = 't'; break;
		case YTYPE::cable: type = 'c'; break;
		case YTYPE::atsc: type = 'a'; break;
		default: return error("add_tunersets_transponder()", "Error", "Unknown tuner settings type.");
	}
	char trid[25];
	std::sprintf(trid, "%c:%04x:%04x", type, tntxp.freq, tntxp.sr);
	tntxp.trid = trid;
	tntxp.index = idx;
	tn.transponders.emplace(tntxp.trid, tntxp);
	index[tn.tnid].emplace_back(pair (idx, tntxp.trid)); //C++17
}

unordered_map<string, e2db_abstract::transponder> e2db_abstract::get_transponders()
{
	debug("get_transponders()");
	return db.transponders;
}

unordered_map<string, e2db_abstract::service> e2db_abstract::get_services()
{
	debug("get_services()");
	return db.services;
}

pair<unordered_map<string, e2db_abstract::bouquet>, unordered_map<string, e2db_abstract::userbouquet>> e2db_abstract::get_bouquets()
{
	debug("get_bouquets()");
	return pair (bouquets, userbouquets); //C++17
}

void e2db_abstract::debugger()
{
	debug("debugger()");

	cout << "transponders" << endl << endl;
	for (auto & x : db.transponders)
	{
		cout << "txid: " << x.first << endl;
		cout << hex;
		cout << "dvbns: " << x.second.dvbns << endl;
		cout << "tsid: " << x.second.tsid << endl;
		cout << "onid: " << x.second.onid << endl;
		cout << dec;
		cout << "ttype: " << x.second.ttype << endl;
		cout << "freq: " << x.second.freq << endl;
		cout << "sr: " << x.second.sr << endl;
		cout << "pol: " << x.second.pol << endl;
		cout << "fec: " << x.second.fec << endl;
		cout << "hpfec: " << x.second.hpfec << endl;
		cout << "lpfec: " << x.second.lpfec << endl;
		cout << "cfec: " << x.second.cfec << endl;
		cout << "pos: " << x.second.pos << endl;
		cout << "inv: " << x.second.inv << endl;
		cout << "flgs: " << x.second.flgs << endl;
		cout << "sys: " << x.second.sys << endl;
		cout << "mod: " << x.second.mod << endl;
		cout << "tmod: " << x.second.tmod << endl;
		cout << "cmod: " << x.second.cmod << endl;
		cout << "amod: " << x.second.amod << endl;
		cout << "rol: " << x.second.rol << endl;
		cout << "pil: " << x.second.pil << endl;
		cout << "band: " << x.second.band << endl;
		cout << "tmx: " << x.second.tmx << endl;
		cout << "guard: " << x.second.guard << endl;
		cout << "hier: " << x.second.hier << endl;
		cout << "oflgs: " << x.second.oflgs << endl;
		cout << endl;
	}
	cout << endl;
	cout << "services" << endl << endl;
	for (auto & x : db.services)
	{
		cout << "chid: " << x.first << endl;
		cout << "txid: " << x.second.txid << endl;
		cout << hex;
		cout << "ssid: " << x.second.ssid << endl;
		cout << "dvbns: " << x.second.dvbns << endl;
		cout << "tsid: " << x.second.tsid << endl;
		cout << dec;
		cout << "onid: " << x.second.onid << endl;
		cout << "stype: " << x.second.stype << endl;
		cout << "snum: " << x.second.snum << endl;
		cout << "srcid: " << x.second.srcid << endl;
		cout << "chname: " << x.second.chname << endl;
		cout << "data: [" << endl << endl;
		for (auto & q : x.second.data)
		{
			cout << q.first << ": [" << endl;
			for (string & w : q.second)
				cout << w << ", ";
			cout << endl << "]";
		}
		cout << "]" << endl << endl;
		cout << "index: " << x.second.index << endl;
		cout << endl;
	}
	cout << endl;
	cout << "bouquets" << endl << endl;
	for (auto & x : bouquets)
	{
		cout << "filename: " << x.first << endl;
		cout << "name: " << x.second.name << endl;
		cout << "nname: " << x.second.nname << endl;
		cout << "btype: " << x.second.btype << endl;
		cout << "userbouquets: [" << endl << endl;
		for (string & w : x.second.userbouquets)
			cout << w << endl;
		cout << "]" << endl << endl;
	}
	cout << endl;
	cout << "userbouquets" << endl << endl;
	for (auto & x : userbouquets)
	{
		cout << "filename: " << x.first << endl;
		cout << "name: " << x.second.name << endl;
		cout << "channels: [" << endl << endl;
		for (auto & q : x.second.channels)
		{
			cout << "chid: " << q.first << endl;
			cout << "index: " << q.second.index << endl;
		}
		cout << "]" << endl << endl;
	}
	cout << endl;
	cout << "tuners" << endl << endl;
	for (auto & x : tuners)
	{
		cout << "ytype: " << x.first << endl;
		cout << "charset: " << x.first << endl;
		cout << "tables: [" << endl << endl;
		for (auto & q : x.second.tables)
		{
			cout << "tnid: " << q.second.tnid << endl;
			cout << "ytype: " << q.second.ytype << endl;
			cout << "name: " << q.second.name << endl;
			cout << "flags: " << q.second.flgs << endl;
			cout << "pos: " << q.second.pos << endl;
			cout << "index: " << q.second.index << endl;
			cout << "transponders: [" << endl << endl;
			for (auto & x : q.second.transponders)
			{
				cout << "trid: " << x.first << endl;
				cout << "freq: " << x.second.freq << endl;
				cout << "sr: " << x.second.sr << endl;
				cout << "pol: " << x.second.pol << endl;
				cout << "fec: " << x.second.fec << endl;
				cout << "hpfec: " << x.second.hpfec << endl;
				cout << "lpfec: " << x.second.lpfec << endl;
				cout << "cfec: " << x.second.cfec << endl;
				cout << "inv: " << x.second.inv << endl;
				cout << "sys: " << x.second.sys << endl;
				cout << "mod: " << x.second.mod << endl;
				cout << "tmod: " << x.second.tmod << endl;
				cout << "cmod: " << x.second.cmod << endl;
				cout << "amod: " << x.second.amod << endl;
				cout << "band: " << x.second.band << endl;
				cout << "tmx: " << x.second.tmx << endl;
				cout << "guard: " << x.second.guard << endl;
				cout << "hier: " << x.second.hier << endl;
				cout << "rol: " << x.second.rol << endl;
				cout << "pil: " << x.second.pil << endl;
				cout << "isid: " << x.second.isid << endl;
				cout << "plsmode: " << x.second.plsmode << endl;
				cout << "plscode: " << x.second.plscode << endl;
				cout << "index: " << x.second.index << endl;
				cout << endl;
			}
			cout << "]" << endl << endl;
		}
		cout << "]" << endl << endl;
	}
	cout << endl;
}

}
