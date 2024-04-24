/*!
 * e2-sat-editor/src/e2db/e2db_abstract.cpp
 * 
 * @link https://github.com/ctlcltd/e2-sat-editor
 * @copyright e2 SAT Editor Team
 * @author Leonardo Laureti
 * @version 1.4.0
 * @license MIT License
 * @license GNU GPLv3 License
 */

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <algorithm>
#include <iostream>
#include <stdexcept>

#if defined(unix) || defined(__unix__) || defined(__unix) || defined(linux) || defined(__linux__) || defined(__APPLE__)
#define PLATFORM_UX
#endif

#if defined(WIN32) || defined(_WIN32)
#define PLATFORM_WIN
#endif

#include "e2db_abstract.h"

using std::string, std::pair, std::hex, std::dec, std::to_string, std::cout, std::endl;

namespace e2se_e2db
{

string e2db_abstract::editor_string(int html)
{
	if (html == 2)
		return "e2 SAT Editor 1.4";
	else if (html == 1)
		return "e2 SAT Editor 1.4 <a href=\"https://github.com/ctlcltd/e2-sat-editor\">https://github.com/ctlcltd/e2-sat-editor</a>";
	else
		return "e2 SAT Editor 1.4 <https://github.com/ctlcltd/e2-sat-editor>";
}

string e2db_abstract::editor_timestamp()
{
	std::time_t ct = std::time(0);
	int zh = 0;

	std::tm* lct = std::localtime(&ct);
	zh = lct->tm_hour;

	char dt[80];
	std::strftime(dt, 80, "%Y-%m-%d %H:%M:%S", lct);
	std::tm* gmt = std::gmtime(&ct);
	zh = (zh - gmt->tm_hour) * 100;

	char tz[7];
	std::snprintf(tz, 7, "%+05d", zh);

	return string (dt) + string (tz);
}

e2db_abstract::FPORTS e2db_abstract::file_type_detect(string path)
{
	string filename = std::filesystem::path(path).filename().u8string();

	if (std::filesystem::is_directory(path))
		return FPORTS::directory;
	else if (filename == "lamedb")
		return FPORTS::all_services; // autodetect
	else if (filename == "lamedb5")
		return FPORTS::all_services__2_5;
	else if (filename == "services")
		return FPORTS::all_services; // autodetect
	else if (filename == "services.xml")
		return FPORTS::all_services_xml; // autodetect
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
	else if (filename.find("userbouquets.") != string::npos)
		return FPORTS::single_bouquet_epl;
	else if (filename == "blacklist")
		return FPORTS::single_parentallock_blacklist;
	else if (filename == "whitelist")
		return FPORTS::single_parentallock_whitelist;
	else if (filename == "services.locked")
		return FPORTS::single_parentallock_locked;
	return FPORTS::unknown;
}

string e2db_abstract::file_mime_detect(FPORTS fpi, string path)
{
	switch (fpi)
	{
		case FPORTS::directory:
		return "application/octet-stream";
		case FPORTS::all_services:
		case FPORTS::all_services__2_4:
		case FPORTS::all_services__2_5:
		case FPORTS::all_services__2_3:
		case FPORTS::all_services__2_2:
		case FPORTS::all_bouquets:
		case FPORTS::all_bouquets_epl:
		case FPORTS::all_userbouquets:
		case FPORTS::single_bouquet:
		case FPORTS::single_bouquet_epl:
		case FPORTS::single_bouquet_all:
		case FPORTS::single_bouquet_all_epl:
		case FPORTS::single_userbouquet:
		case FPORTS::single_parentallock_locked:
		case FPORTS::single_parentallock_blacklist:
		case FPORTS::single_parentallock_whitelist:
		return "text/plain";
		case FPORTS::all_services_xml:
		case FPORTS::all_services_xml__4:
		case FPORTS::all_services_xml__3:
		case FPORTS::all_services_xml__2:
		case FPORTS::all_services_xml__1:
		case FPORTS::all_bouquets_xml:
		case FPORTS::all_bouquets_xml__4:
		case FPORTS::all_bouquets_xml__3:
		case FPORTS::all_bouquets_xml__2:
		case FPORTS::all_bouquets_xml__1:
		case FPORTS::all_tunersets:
		case FPORTS::single_tunersets:
		return "text/xml";
		default:
			if (path.rfind(".xml") != string::npos)
				return "text/xml";
			if (path.rfind(".csv") != string::npos)
				return "text/csv";
			if (path.rfind(".html") != string::npos)
				return "text/html";
			else
				return "";
	}
}

void e2db_abstract::value_channel_reference(string str, channel_reference& chref, service_reference& ref)
{
	int etype, atype, anum, ssid, tsid, onid, dvbns, x7, x8, x9;
	etype = 0, atype = 0, anum = 0, ssid = 0, tsid = 0, onid = 0, dvbns = 0;

	std::sscanf(str.c_str(), "%d:%d:%X:%X:%X:%X:%X:%d:%d:%d", &etype, &atype, &anum, &ssid, &tsid, &onid, &dvbns, &x7, &x8, &x9);

	switch (etype)
	{
		// service or stream
		case ETYPE::ecast:
		// stream
		case ETYPE::ecustom:
		case ETYPE::eservice:
		case ETYPE::eytube:
		break;
		default:
			return;
	}

	switch (atype)
	{
		// marker
		case ATYPE::marker_regular:
		case ATYPE::marker_numbered:
		case ATYPE::marker_hidden_512:
		case ATYPE::marker_hidden_832:
			chref.marker = true;
		break;
		// group
		case ATYPE::group:
		return;
		// service
		default:
			chref.marker = false;
	}

	ref.ssid = ssid;
	ref.tsid = tsid;
	ref.onid = onid;
	ref.dvbns = dvbns;

	chref.etype = etype;
	chref.atype = atype;
	chref.anum = anum;
	chref.x7 = x7;
	chref.x8 = x8;
	chref.x9 = x9;
	chref.ref = ref;
}

string e2db_abstract::value_reference_id(service ch)
{
	int etype = 1;
	int atype = 0;
	int stype = ch.stype;
	int ssid = ch.ssid;
	int tsid = ch.tsid;
	int onid = ch.onid;
	int dvbns = ch.dvbns;

	char refid[50];
	// %1d:%4d:%4X:%4X:%4X:%4X:%X:%d:%d:%d:
	std::snprintf(refid, 50, "%d:%d:%X:%X:%X:%X:%X:0:0:0", etype, atype, stype, ssid, tsid, onid, dvbns);

	return refid;
}

string e2db_abstract::value_reference_id(channel_reference chref)
{
	int etype = chref.etype;
	int atype = chref.atype;
	int anum = chref.anum;
	int ssid = chref.ref.ssid;
	int tsid = chref.ref.tsid;
	int onid =  chref.ref.onid;
	int dvbns = chref.ref.dvbns;
	int x7 = chref.x7;
	int x8 = chref.x8;
	int x9 = chref.x9;

	char refid[50];
	// %1d:%4d:%4X:%4X:%4X:%4X:%X:%d:%d:%d:
	std::snprintf(refid, 50, "%d:%d:%X:%X:%X:%X:%X:%d:%d:%d", etype, atype, anum, ssid, tsid, onid, dvbns, x7, x8, x9);

	return refid;
}

string e2db_abstract::value_reference_id(channel_reference chref, service ch)
{
	int etype = 0;
	int atype = 0;
	int anum = 0;
	int ssid = 0;
	int tsid = 0;
	int onid = 0;
	int dvbns = 0;
	int x7 = 0;
	int x8 = 0;
	int x9 = 0;

	if (chref.marker)
	{
		etype = chref.etype;
		atype = chref.atype;
		anum = chref.anum;
	}
	else if (chref.stream)
	{
		etype = chref.etype;
		atype = chref.atype;
		anum = chref.anum;
		ssid = chref.ref.ssid;
		tsid = chref.ref.tsid;
		onid =  chref.ref.onid;
		dvbns = chref.ref.dvbns;
	}
	else
	{
		etype = chref.etype;
		atype = chref.atype;
		anum = ch.stype;
		ssid = ch.ssid;
		tsid = ch.tsid;
		onid = ch.onid;
		dvbns = ch.dvbns;
	}

	x7 = chref.x7;
	x8 = chref.x8;
	x9 = chref.x9;

	char refid[50];
	// %1d:%4d:%4X:%4X:%4X:%4X:%X:%d:%d:%d:
	std::snprintf(refid, 50, "%d:%d:%X:%X:%X:%X:%X:%d:%d:%d", etype, atype, anum, ssid, tsid, onid, dvbns, x7, x8, x9);

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
	else if (str == "HD")
		return 25;
	else if (str == "H.264")
		return 22;
	else if (str == "H.265")
		return 31;
	else if (str == "UHD")
		return 17;
	else if (str == "MARKER")
		return -1;
	else if (str == "STREAM")
		return -1;
	else
		return STYPE::data;
}

string e2db_abstract::value_service_type(service ch)
{
	return value_service_type(ch.stype);
}

string e2db_abstract::value_service_type(int stype)
{
	if (STYPE_EXT_LABEL.count(stype))
		return STYPE_EXT_LABEL.at(stype);
	return STYPE_EXT_LABEL.at(STYPE::data);
}

int e2db_abstract::value_service_super_type(service ch)
{
	if (STYPE_EXT_TYPE.count(ch.stype))
		return STYPE_EXT_TYPE.at(ch.stype);
	return 0;
}

int e2db_abstract::value_service_super_type(int stype)
{
	if (STYPE_EXT_TYPE.count(stype))
		return STYPE_EXT_TYPE.at(stype);
	return 0;
}

int e2db_abstract::value_favourite_type(string str)
{
	if (str == "[broadcast]")
		return ETYPE::ecast;
	else if (str == "[file]")
		return ETYPE::efile;
	else if (str == "[custom]")
		return ETYPE::ecustom;
	else if (str == "[eservice]")
		return ETYPE::eservice;
	else if (str == "[youtube]")
		return ETYPE::eytube;
	else
		return ETYPE::ecast;
}

string e2db_abstract::value_favourite_type(channel_reference chref)
{
	return value_favourite_type(chref.etype);
}

string e2db_abstract::value_favourite_type(int etype)
{
	if (ETYPE_EXT_LABEL.count(etype))
		return ETYPE_EXT_LABEL.at(etype);
	return ETYPE_EXT_LABEL.at(ETYPE::ecast);
}

int e2db_abstract::value_favourite_flag_type(string str)
{
	if (str == "[null]")
		return ATYPE::aaaa;
	else if (str == "[marker]")
		return ATYPE::marker;
	else if (str == "[marker hidden]")
		return ATYPE::marker_hidden;
	else if (str == "[marker numbered]")
		return ATYPE::marker_numbered;
	else if (str == "[group]")
		return ATYPE::group;
	else if (str == "[marker hidden 512]")
		return ATYPE::marker_hidden_512;
	else if (str == "[marker hidden 832]")
		return ATYPE::marker_hidden_832;
	else if (str == "[marker numbered 320]")
		return ATYPE::marker_numbered;
	else
		return ATYPE::aaaa;
}

string e2db_abstract::value_favourite_flag_type(channel_reference chref)
{
	return value_favourite_flag_type(chref.atype);
}

string e2db_abstract::value_favourite_flag_type(int atype)
{
	if (ATYPE_EXT_LABEL.count(atype))
		return ATYPE_EXT_LABEL.at(atype);
	return ATYPE_EXT_LABEL.at(ATYPE::aaaa);
}

vector<string> e2db_abstract::value_channel_provider(string str)
{
	return {str};
}

string e2db_abstract::value_channel_provider(service ch)
{
	if (ch.data.count(SDATA::p))
		return ch.data[SDATA::p][0];
	return "";
}

string e2db_abstract::value_channel_provider(map<char, vector<string>> data)
{
	if (data.count(SDATA::p))
		return data[SDATA::p][0];
	return "";
}

vector<string> e2db_abstract::value_channel_caid(string str, string separator)
{
	if (str.empty())
		return {};
	else if (str.find("$") != string::npos)
		str = str.substr(str.find("$"));

	vector<string> caid;
	char* token = std::strtok(str.data(), separator.c_str());
	while (token != 0)
	{
		string val = string (token);

		if (val.find("Seca") != string::npos)
			caid.emplace_back(val.substr(5));
		else if (val.find("Irdeto") != string::npos)
			caid.emplace_back(val.substr(5));
		else if (val.find("Viaccess") != string::npos)
			caid.emplace_back(val.substr(9));
		else if (val.find("NDS") != string::npos)
			caid.emplace_back(val.substr(4));
		else if (val.find("Conax") != string::npos)
			caid.emplace_back(val.substr(6));
		else if (val.find("Cryptoworks") != string::npos)
			caid.emplace_back(val.substr(12));
		else if (val.find("PVU") != string::npos)
			caid.emplace_back(val.substr(4));
		else if (val.find("Nagra") != string::npos)
			caid.emplace_back(val.substr(6));
		else if (val.find("BISS") != string::npos)
			caid.emplace_back(val.substr(5));
		else if (val.find("Crypton") != string::npos)
			caid.emplace_back(val.substr(8));
		else if (val.find("DRE") != string::npos)
			caid.emplace_back(val.substr(4));
		else
			caid.emplace_back(val.substr(1));

		token = std::strtok(NULL, separator.c_str());
	}
	return caid;
}

vector<string> e2db_abstract::value_channel_cached(string str, string separator)
{
	if (str.empty())
		return {};

	vector<string> cached;
	char* token = std::strtok(str.data(), separator.c_str());
	while (token != 0)
	{
		cached.emplace_back(token);
		token = std::strtok(NULL, separator.c_str());
	}
	return cached;
}

int e2db_abstract::value_transponder_type(char ty)
{
	switch (ty)
	{
		case 's': return YTYPE::satellite;
		case 't': return YTYPE::terrestrial;
		case 'c': return YTYPE::cable;
		case 'a': return YTYPE::atsc;
		default: return -1;
	}
}

int e2db_abstract::value_transponder_type(string str)
{
	if (str.empty())
		return -1;
	if (str == "DVB-S" || str == "DVB-S2" || str == "DVB-S/S2")
		return YTYPE::satellite;
	else if (str == "DVB-T" || str == "DVB-T2" || str == "DVB-T/T2")
		return YTYPE::terrestrial;
	else if (str == "DVB-C")
		return YTYPE::cable;
	else if (str == "ATSC")
		return YTYPE::atsc;
	return -1;
}

char e2db_abstract::value_transponder_type(int yx)
{
	YTYPE ytype = static_cast<YTYPE>(yx);
	return value_transponder_type(ytype);
}

char e2db_abstract::value_transponder_type(YTYPE ytype)
{
	switch (ytype)
	{
		case YTYPE::satellite: return 's';
		case YTYPE::terrestrial: return 't';
		case YTYPE::cable: return 'c';
		case YTYPE::atsc: return 'a';
		default: return '\0';
	}
}

string e2db_abstract::value_transponder_combo(transponder tx)
{
	string ptxp;
	if (tx.ytype == YTYPE::satellite)
	{
		int pol = tx.pol != -1 ? tx.pol : 0;
		int sr = tx.sr != -1 ? tx.sr : 0;
		ptxp = to_string(tx.freq) + '/' + SAT_POL[pol] + '/' + to_string(sr);
	}
	else if (tx.ytype == YTYPE::terrestrial)
	{
		int tmod = tx.tmod != -1 ? tx.tmod : 3;
		int band = tx.band != -1 ? tx.band : 3;
		ptxp = to_string(tx.freq) + '/' + TER_MOD[tmod] + '/' + TER_BAND[band];
	}
	else if (tx.ytype == YTYPE::cable)
	{
		int cmod = tx.cmod != -1 ? tx.cmod : 0;
		int sr = tx.sr != -1 ? tx.sr : 0;
		ptxp = to_string(tx.freq) + '/' + CAB_MOD[cmod] + '/' + to_string(sr);
	}
	else if (tx.ytype == YTYPE::atsc)
	{
		ptxp = to_string(tx.freq);
	}
	return ptxp;
}

string e2db_abstract::value_transponder_combo(tunersets_transponder tntxp, tunersets_table tn)
{
	string ptxp;
	if (tn.ytype == YTYPE::satellite)
	{
		int pol = tntxp.pol != -1 ? tntxp.pol : 0;
		int sr = tntxp.sr != -1 ? tntxp.sr : 0;
		ptxp = to_string(tntxp.freq) + '/' + SAT_POL[pol] + '/' + to_string(sr);
	}
	else if (tn.ytype == YTYPE::terrestrial)
	{
		int tmod = tntxp.tmod != -1 ? tntxp.tmod : 3;
		int band = tntxp.band != -1 ? tntxp.band : 3;
		ptxp = to_string(tntxp.freq) + '/' + TER_MOD[tmod] + '/' + TER_BAND[band];
	}
	else if (tn.ytype == YTYPE::cable)
	{
		int cmod = tntxp.cmod != -1 ? tntxp.cmod : 0;
		int sr = tntxp.sr != -1 ? tntxp.sr : 0;
		ptxp = to_string(tntxp.freq) + '/' + CAB_MOD[cmod] + '/' + to_string(sr);
	}
	else if (tn.ytype == YTYPE::atsc)
	{
		ptxp = to_string(tntxp.freq);
	}
	return ptxp;
}

string e2db_abstract::value_transponder_onid(int onid)
{
	char conid[9];
	std::snprintf(conid, 9, "%X", onid);

	return conid;
}

int e2db_abstract::value_transponder_dvbns(string str)
{
	try
	{
		return int (std::stoul(str, nullptr, 16));
	}
	// catch (const std::invalid_argument& err)
	catch (...)
	{
		return 0;
	}
}

int e2db_abstract::value_transponder_dvbns(transponder tx)
{
	YTYPE ytype = static_cast<YTYPE>(tx.ytype);
	return value_transponder_dvbns(ytype, tx.tsid, tx.onid, tx.pos, tx.freq);
}

//TODO TEST
int e2db_abstract::value_transponder_dvbns(YTYPE ytype, int tsid, int onid, int pos, int freq)
{
	if (ytype == YTYPE::terrestrial)
		return 0xeeee0000;
	else if (ytype == YTYPE::cable)
		return 0xffff0000;

	int posns = pos;
	int freqns = 0;

	if (posns > 1800)
		posns -= 3600;
	posns = posns << 16;

	bool valid = false;

	if (onid == 0x0000 || onid == 0xffff || onid == 0x1111)
		valid = false;
	else if (onid == 0x0001)
		valid = (tsid > 0x0001);
	else if (onid == 0x00b1)
		valid = (tsid > 0x00b0 || tsid < 0x00b0);
	else if (onid == 0x0002)
		valid = (tsid > 0x07e8 || tsid < 0x07e8);
	else
		valid = true;

	if (! valid)
		freqns = freq << 16 >> 16;

	return posns ?: freqns;
}

/*int e2db_abstract::value_transponder_dvbns(int pos, int onid)
{
	return pos << 16;
}*/

string e2db_abstract::value_transponder_dvbns(int dvbns)
{
	char cdvbns[9];
	std::snprintf(cdvbns, 9, "%X", dvbns);

	return cdvbns;
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

string e2db_abstract::value_transponder_polarization(int pol)
{
	if (pol < 0)
		return "";
	else if (pol < (int (sizeof(SAT_POL) / sizeof(SAT_POL[0]))))
		return SAT_POL[pol];
	return "";
}

string e2db_abstract::value_transponder_sr(int sr)
{
	if (sr != -1)
		return to_string(sr);
	return "";
}

string e2db_abstract::value_transponder_position(transponder tx)
{
	if (tx.ytype == YTYPE::satellite)
		return value_transponder_position(tx.pos);
	return "";
}

string e2db_abstract::value_transponder_position(tunersets_table tn)
{
	if (tn.ytype == YTYPE::satellite)
		return value_transponder_position(tn.pos);
	return "";
}

string e2db_abstract::value_transponder_position(int num)
{
	char cposdeg[6];
	// %3d.%1d%C
	std::snprintf(cposdeg, 6, "%.1f", float (std::abs(num)) / 10);
	return (string (cposdeg) + (num > 0 ? 'E' : 'W'));
}

int e2db_abstract::value_transponder_position(string str)
{
	if (! str.empty())
	{
		try
		{
			size_t pos;
			float posdeg = std::stof(str, &pos);
			char pospoint = str.substr(pos)[0];
			return (int ((pospoint == 'E' ? posdeg : -posdeg) * 10));
		}
		// catch (const std::invalid_argument& err)
		catch (...)
		{
			return -1;
		}
	}
	return -1;
}

int e2db_abstract::value_transponder_system(string str)
{
	if (str == "DVB-S" || str == "DVB-T" || str == "DVB-C" || str == "ATSC")
		return 0;
	else if (str == "DVB-S2" || str == "DVB-T2" || str == "DVB-C ANNEX B")
		return 1;
	else if (str == "DVB-S/S2" || str == "DVB-T/T2")
		return 2;
	return -1;
}

string e2db_abstract::value_transponder_system(transponder tx)
{
	return value_transponder_system(tx.sys, tx.ytype);
}

string e2db_abstract::value_transponder_system(int sys, int yx)
{
	YTYPE ytype = static_cast<YTYPE>(yx);
	return value_transponder_system(sys, ytype);
}

string e2db_abstract::value_transponder_system(int sys, YTYPE ytype)
{
	string psys;
	switch (ytype)
	{
		case YTYPE::satellite:
			psys = sys != -1 ? SAT_SYS[sys] : "DVB-S";
		break;
		case YTYPE::terrestrial:
			psys = sys != -1 ? TER_SYS[sys] : "DVB-T";
		break;
		case YTYPE::cable:
			psys = "DVB-C";
		break;
		case YTYPE::atsc:
			psys = sys != -1 ? ATS_SYS[sys] : "ATSC";
		break;
	}
	return psys;
}

void e2db_abstract::value_transponder_fec(string str, int yx, fec& fec)
{
	if (yx == YTYPE::satellite)
	{
		fec.inner_fec = value_transponder_fec(str, YTYPE::satellite);
	}
	else if (yx == YTYPE::terrestrial)
	{
		size_t pos = str.find("|");
		if (pos != string::npos)
		{
			string hp_fec, lp_fec;
			hp_fec = str.substr(0, pos);
			lp_fec = str.substr(pos + 1);
			fec.hp_fec = value_transponder_fec(hp_fec, YTYPE::terrestrial);
			fec.lp_fec = value_transponder_fec(lp_fec, YTYPE::terrestrial);
		}
	}
	else if (yx == YTYPE::cable)
	{
		fec.inner_fec = value_transponder_fec(str, YTYPE::cable);
	}
}

int e2db_abstract::value_transponder_fec(string str, int yx)
{
	YTYPE ytype = static_cast<YTYPE>(yx);
	return value_transponder_fec(str, ytype);
}

int e2db_abstract::value_transponder_fec(string str, YTYPE ytype)
{
	if (ytype == YTYPE::satellite)
	{
		if (str.empty())
			return -1;
		else if (str == "Auto")
			return 0;
		else if (str == "1/2")
			return 1;
		else if (str == "2/3")
			return 2;
		else if (str == "3/4")
			return 3;
		else if (str == "5/6")
			return 4;
		else if (str == "7/8")
			return 5;
		else if (str == "8/9")
			return 6;
		else if (str == "3/5")
			return 7;
		else if (str == "4/5")
			return 8;
		else if (str == "9/10")
			return 9;
		else if (str == "6/7")
			return 10;
		else if (str == "None")
			return 15;
		return -1;
	}
	else if (ytype == YTYPE::terrestrial)
	{
		if (str.empty())
			return -1;
		else if (str == "Auto")
			return 5;
		else if (str == "1/2")
			return 0;
		else if (str == "2/3")
			return 1;
		else if (str == "3/4")
			return 2;
		else if (str == "5/6")
			return 3;
		else if (str == "7/8")
			return 4;
		else if (str == "8/9")
			return 6;
		else if (str == "None")
			return 15;
		return -1;
	}
	else if (ytype == YTYPE::cable)
	{
		if (str.empty())
			return 0;
		else if (str == "Auto")
			return 0;
		else if (str == "1/2")
			return 1;
		else if (str == "2/3")
			return 2;
		else if (str == "3/4")
			return 3;
		else if (str == "5/6")
			return 4;
		else if (str == "7/8")
			return 5;
		else if (str == "8/9")
			return 6;
		else if (str == "None")
			return 15;
		return -1;
	}
	return -1;
}

string e2db_abstract::value_transponder_fec(int fec, int yx)
{
	YTYPE ytype = static_cast<YTYPE>(yx);
	return value_transponder_fec(fec, ytype);
}

string e2db_abstract::value_transponder_fec(int fec, YTYPE ytype)
{
	if (fec < 0)
		return "";
	if (ytype == YTYPE::satellite && fec < (int (sizeof(SAT_FEC) / sizeof(SAT_FEC[0]))))
		return SAT_FEC[fec];
	else if (ytype == YTYPE::terrestrial && fec < (int (sizeof(TER_FEC) / sizeof(TER_FEC[0]))))
		return TER_FEC[fec];
	else if (ytype == YTYPE::cable && fec < (int (sizeof(CAB_FEC) / sizeof(CAB_FEC[0]))))
		return CAB_FEC[fec];
	else if (fec == 0xf)
		return "None";
	return "";
}

int e2db_abstract::value_transponder_modulation(string str, int yx)
{
	YTYPE ytype = static_cast<YTYPE>(yx);
	return value_transponder_modulation(str, ytype);
}

int e2db_abstract::value_transponder_modulation(string str, YTYPE ytype)
{
	if (ytype == YTYPE::satellite)
	{
		if (str.empty())
			return -1;
		else if (str == "Auto")
			return 0;
		else if (str == "QPSK")
			return 1;
		else if (str == "8PSK")
			return 2;
		else if (str == "QAM16")
			return 3;
		else if (str == "16APSK")
			return 4;
		else if (str == "32APSK")
			return 5;
		return -1;
	}
	else if (ytype == YTYPE::terrestrial)
	{
		if (str.empty())
			return -1;
		else if (str == "Auto")
			return 3;
		else if (str == "QPSK")
			return 0;
		else if (str == "QAM16")
			return 1;
		else if (str == "QAM64")
			return 2;
		else if (str == "QAM256")
			return 4;
		return -1;
	}
	else if (ytype == YTYPE::cable)
	{
		if (str.empty())
			return -1;
		else if (str == "Auto")
			return 0;
		else if (str == "QAM16")
			return 1;
		else if (str == "QAM32")
			return 2;
		else if (str == "QAM64")
			return 3;
		else if (str == "QAM128")
			return 4;
		else if (str == "QAM256")
			return 5;
		return -1;
	}
	else if (ytype == YTYPE::atsc)
	{
		if (str.empty())
			return -1;
		else if (str == "Auto")
			return 0;
		else if (str == "QAM16")
			return 1;
		else if (str == "QAM32")
			return 2;
		else if (str == "QAM64")
			return 3;
		else if (str == "QAM128")
			return 4;
		else if (str == "QAM256")
			return 5;
		else if (str == "8VSB")
			return 6;
		else if (str == "16VSB")
			return 7;
		return -1;
	}
	return -1;
}

string e2db_abstract::value_transponder_modulation(int mod, int yx)
{
	YTYPE ytype = static_cast<YTYPE>(yx);
	return value_transponder_modulation(mod, ytype);
}

string e2db_abstract::value_transponder_modulation(int mod, YTYPE ytype)
{
	if (mod < 0)
		return "";
	else if (ytype == YTYPE::satellite && mod < (int (sizeof(SAT_MOD) / sizeof(SAT_MOD[0]))))
		return SAT_MOD[mod];
	else if (ytype == YTYPE::terrestrial && mod < (int (sizeof(TER_MOD) / sizeof(TER_MOD[0]))))
		return TER_MOD[mod];
	else if (ytype == YTYPE::cable && mod < (int (sizeof(CAB_MOD) / sizeof(CAB_MOD[0]))))
		return CAB_MOD[mod];
	else if (ytype == YTYPE::atsc && mod < (int (sizeof(ATS_MOD) / sizeof(ATS_MOD[0]))))
		return ATS_MOD[mod];
	return "";
}

int e2db_abstract::value_transponder_inversion(string str, int yx)
{
	YTYPE ytype = static_cast<YTYPE>(yx);
	return value_transponder_inversion(str, ytype);
}

int e2db_abstract::value_transponder_inversion(string str, YTYPE ytype)
{
	if (str.empty())
		return -1;
	else if (str == "Off")
		return 0;
	else if (str == "On")
		return 1;
	else if (str == "Auto")
		return 2;
	return -1;
}

string e2db_abstract::value_transponder_inversion(int inv, int yx)
{
	YTYPE ytype = static_cast<YTYPE>(yx);
	return value_transponder_inversion(inv, ytype);
}

string e2db_abstract::value_transponder_inversion(int inv, YTYPE ytype)
{
	if (inv < 0)
		return "";
	if (ytype == YTYPE::satellite && inv < (int (sizeof(SAT_INV) / sizeof(SAT_INV[0]))))
		return SAT_INV[inv];
	else if (ytype == YTYPE::terrestrial && inv < (int (sizeof(TER_INV) / sizeof(TER_INV[0]))))
		return TER_INV[inv];
	else if (ytype == YTYPE::cable && inv < (int (sizeof(CAB_INV) / sizeof(CAB_INV[0]))))
		return CAB_INV[inv];
	else if (ytype == YTYPE::atsc && inv < (int (sizeof(ATS_INV) / sizeof(ATS_INV[0]))))
		return ATS_INV[inv];
	return "";
}

int e2db_abstract::value_transponder_rollof(string str)
{
	if (str.empty())
		return -1;
	else if (str == "0.35")
		return 0;
	else if (str == "0.25")
		return 1;
	else if (str == "0.20")
		return 2;
	return -1;
}

string e2db_abstract::value_transponder_rollof(int rol)
{
	if (rol < 0)
		return "";
	else if (rol < (int (sizeof(SAT_ROL) / sizeof(SAT_ROL[0]))))
		return SAT_ROL[rol];
	return "";
}

int e2db_abstract::value_transponder_pilot(string str)
{
	if (str.empty())
		return -1;
	else if (str == "Off")
		return 0;
	else if (str == "On")
		return 1;
	else if (str == "Auto")
		return 2;
	return -1;
}

string e2db_abstract::value_transponder_pilot(int pil)
{
	if (pil < 0)
		return "";
	else if (pil < (int (sizeof(SAT_PIL) / sizeof(SAT_PIL[0]))))
		return SAT_PIL[pil];
	return "";
}

int e2db_abstract::value_transponder_bandwidth(string str)
{
	if (str.empty())
		return -1;
	else if (str == "Auto")
		return 3;
	else if (str == "8MHz")
		return 0;
	else if (str == "7MHz")
		return 1;
	else if (str == "6MHz")
		return 2;
	else if (str == "5MHz")
		return 4;
	else if (str == "1.712MHz")
		return 5;
	else if (str == "10MHz")
		return 6;
	return -1;
}

string e2db_abstract::value_transponder_bandwidth(int band)
{
	if (band < 0)
		return "";
	else if (band < (int (sizeof(TER_BAND) / sizeof(TER_BAND[0]))))
		return TER_BAND[band];
	return "";
}

int e2db_abstract::value_transponder_guard(string str)
{
	if (str.empty())
		return -1;
	else if (str == "Auto")
		return 4;
	else if (str == "1/32")
		return 0;
	else if (str == "1/16")
		return 1;
	else if (str == "1/8")
		return 2;
	else if (str == "1/4")
		return 3;
	else if (str == "1/128")
		return 5;
	else if (str == "19/128")
		return 6;
	else if (str == "19/256")
		return 7;
	return -1;
}

string e2db_abstract::value_transponder_guard(int guard)
{
	if (guard < 0)
		return "";
	else if (guard < (int (sizeof(TER_GUARD) / sizeof(TER_GUARD[0]))))
		return TER_GUARD[guard];
	return "";
}

int e2db_abstract::value_transponder_hier(string str)
{
	if (str.empty())
		return -1;
	else if (str == "Auto")
		return 4;
	else if (str == "0")
		return 0;
	else if (str == "1")
		return 1;
	else if (str == "2")
		return 2;
	else if (str == "4")
		return 3;
	return -1;
}

string e2db_abstract::value_transponder_hier(int hier)
{
	if (hier < 0)
		return "";
	else if (hier < (int (sizeof(TER_HIER) / sizeof(TER_HIER[0]))))
		return TER_HIER[hier];
	return "";
}

int e2db_abstract::value_transponder_tmx_mode(string str)
{
	if (str.empty())
		return -1;
	else if (str == "Auto")
		return 2;
	else if (str == "2k")
		return 0;
	else if (str == "8k")
		return 1;
	else if (str == "4k")
		return 3;
	else if (str == "1k")
		return 4;
	else if (str == "16k")
		return 5;
	else if (str == "32k")
		return 6;
	return -1;
}

string e2db_abstract::value_transponder_tmx_mode(int tmx)
{
	if (tmx < 0)
		return "";
	else if (tmx < (int (sizeof(TER_TMXMODE) / sizeof(TER_TMXMODE[0]))))
		return TER_TMXMODE[tmx];
	return "";
}

void e2db_abstract::value_transponder_feopts(string str, transponder& tx)
{
	YTYPE ytype = static_cast<YTYPE>(tx.ytype);

	if (ytype == YTYPE::satellite)
	{
		int plsn, isid, plscode, plsmode;
		plsn = -1, isid = -1, plscode = -1, plsmode = -1;

		std::sscanf(str.c_str(), "%d:%d:%d:%d", &plsn, &isid, &plscode, &plsmode);

		tx.plsn = plsn;
		tx.isid = isid;
		tx.plscode = plscode;
		tx.plsmode = plsmode;
	}
	else if (ytype == YTYPE::terrestrial)
	{
		int plpid;
		plpid = -1;

		std::sscanf(str.c_str(), "%d:", &plpid);

		tx.plpid = plpid;
	}
}

string e2db_abstract::value_transponder_feopts(transponder tx)
{
	YTYPE ytype = static_cast<YTYPE>(tx.ytype);

	if (ytype == YTYPE::satellite)
	{
		int plsn = tx.plsn != -1 ? tx.plsn : 0;
		int isid = tx.isid != -1 ? tx.isid : 0;
		int plscode = tx.plscode != -1 ? tx.plscode : 0;
		int plsmode = tx.plsmode != -1 ? tx.plsmode : 0;

		char feopts[37];
		std::snprintf(feopts, 37, "%d:%d:%d:%d", plsn, isid, plscode, plsmode);

		return feopts;
	}
	else if (ytype == YTYPE::terrestrial)
	{
		int plpid = tx.plpid != -1 ? tx.plpid : 0;

		char feopts[37];
		std::snprintf(feopts, 37, "%d:0:0:0", plpid);

		return feopts;
	}
	return "";
}

int e2db_abstract::value_bouquet_type(string str)
{
	if (str.empty())
		return -1;
	else if (str == "TV")
		return STYPE::tv;
	else if (str == "Radio")
		return STYPE::radio;
	return -1;
}

string e2db_abstract::value_bouquet_type(int btype)
{
	switch (btype)
	{
		case STYPE::tv: return "TV";
		case STYPE::radio: return "Radio";
		default: return "";
	}
}

void e2db_abstract::value_markup_attribute(string line, string token, string& key, string& val)
{
	size_t pos;
	size_t n = token.find('=');

	if (n != string::npos)
	{
		key = token.substr(0, n);
		val = token.substr(n + 1);
	}
	else
	{
		key = token;
	}

	pos = val.find('"');
	n = string::npos;

	if (pos != string::npos)
	{
		n = val.rfind('"');
	}

	if (n != string::npos && pos != n)
	{
		val = val.substr(0, n);
		if (pos != string::npos)
			val = val.substr(pos + 1);
	}
	else
	{
		val = line.substr(line.find(key) + key.length());
		pos = val.find('"');
		if (pos != string::npos)
			val = val.substr(pos + 1);

		n = val.find('"');
		if (n != string::npos)
			val = val.substr(0, n);

		std::transform(val.begin(), val.end(), val.begin(), [](unsigned char c) { return c ? c : ' '; });
	}
}

string e2db_abstract::get_reference_id(string chid)
{
	// debug("get_reference_id", "chid", chid);

	if (db.services.count(chid))
		return value_reference_id(db.services[chid]);
	else
		return "0:0:0:0:0:0:0:0:0:0";
}

string e2db_abstract::get_reference_id(channel_reference chref)
{
	// debug("get_reference_id", "chref.chid", chref.chid);

	if (! chref.marker && ! chref.stream && db.services.count(chref.chid))
		return value_reference_id(chref, db.services[chref.chid]);
	else
		return value_reference_id(chref);
}

string e2db_abstract::get_tuner_name(transponder tx)
{
	// debug("get_tuner_name", "txid", tx.txid);

	string txpname;
	if (tx.ytype == YTYPE::satellite)
	{
		if (tuners_pos.count(tx.pos))
		{
			string tnid = tuners_pos.at(tx.pos);
			tunersets_table tns = tuners[0].tables[tnid];
			txpname = tns.name;
		}
	}
	return txpname;
}

void e2db_abstract::add_transponder(int idx, transponder& tx)
{
	char txid[25];
	// %4x:%8x
	std::snprintf(txid, 25, "%x:%x", tx.tsid, tx.dvbns);
	tx.txid = txid;
	tx.index = idx;

	db.transponders.emplace(tx.txid, tx);
	index["txs"].emplace_back(pair (idx, tx.txid));
}

void e2db_abstract::add_service(int idx, service& ch)
{
	char txid[25];
	// %4x:%8x
	std::snprintf(txid, 25, "%x:%x", ch.tsid, ch.dvbns);

	char chid[25];
	// %4x:%4x:%8x
	std::snprintf(chid, 25, "%x:%x:%x", ch.ssid, ch.tsid, ch.dvbns);

	ch.txid = txid;
	ch.chid = chid;

	if (db.services.count(ch.chid))
	{
		string kchid = 's' + ch.chid;
		int m = int (collisions[kchid].size());
		ch.chid += ':' + to_string(m);
		collisions[kchid].emplace_back(pair (ch.chid, m));
	}

	string iname = "chs:" + (STYPE_EXT_TYPE.count(ch.stype) ? to_string(STYPE_EXT_TYPE.at(ch.stype)) : "0");
	ch.index = idx;

	db.services.emplace(ch.chid, ch);
	index["chs"].emplace_back(pair (idx, ch.chid));
	index[iname].emplace_back(pair (idx, ch.chid));
}

void e2db_abstract::add_bouquet(int idx, bouquet& bs)
{
	bs.index = idx;

	bouquets.emplace(bs.bname, bs);
	index["bss"].emplace_back(pair (idx, bs.bname));
}

void e2db_abstract::add_userbouquet(int idx, userbouquet& ub)
{
	ub.index = idx;

	userbouquets.emplace(ub.bname, ub);
	bouquets[ub.pname].userbouquets.emplace_back(ub.bname);
	index["ubs"].emplace_back(pair (idx, ub.bname));
}

void e2db_abstract::add_channel_reference(int idx, userbouquet& ub, channel_reference& chref, service_reference& ref)
{
	char chid[25];

	if (chref.marker)
	{
		if (MARKER_GLOBAL_INDEX)
		{
			chref.inum = db.imarkers + 1;

			// %4d:%4d:%8x:%d
			std::snprintf(chid, 25, "%d:%d:%x:%d", chref.etype, chref.atype, chref.inum, ub.index);
		}
		else
		{
			bool valid = chref.anum != 0;
			char ref_chid[25];

			chref.inum = valid ? chref.anum : db.imarkers + 1;

			// %4d:%4d:%8x:%d
			std::snprintf(ref_chid, 25, "%d:%d:%x:%d", chref.etype, chref.atype, chref.inum, ub.index);

			valid = ! ub.channels.count(ref_chid);

			if (valid)
			{
				std::memcpy(chid, ref_chid, 25);
			}
			else
			{
				chref.inum = db.imarkers + 1;

				// %4d:%4d:%8x:%d
				std::snprintf(chid, 25, "%d:%d:%x:%d", chref.etype, chref.atype, chref.inum, ub.index);
			}
		}
	}
	else if (chref.stream)
	{
		chref.inum = db.istreams + 1;

		// %4d:%4d:%8x:%d
		std::snprintf(chid, 25, "%d:%d:%x:%d", chref.etype, chref.atype, chref.inum, ub.index);
	}
	else
	{
		// %4x:%4x:%8x
		std::snprintf(chid, 25, "%x:%x:%x", ref.ssid, ref.tsid, ref.dvbns);
	}

	chref.chid = chid;
	chref.index = idx;
	chref.ref = ref;

	bouquet& bs = bouquets[ub.pname];
	ub.channels.emplace(chref.chid, chref);
	index[ub.bname].emplace_back(pair (idx, chref.chid));

	if (chref.marker)
		db.imarkers++;
	else if (chref.stream)
		db.istreams++;
	else
		db.iservices++;

	if (chref.marker)
	{
		index["mks"].emplace_back(pair (ub.index, chref.chid));
	}
	else if (! chref.stream && bs.services.count(chref.chid) == 0)
	{
		int idx = int (index[ub.pname].size());
		idx += 1;
		bs.services.emplace(chref.chid);
		index[ub.pname].emplace_back(pair (idx, chref.chid));
	}
}

void e2db_abstract::set_channel_reference_description(userbouquet& ub, channel_reference& chref, string value)
{
	ub.channels[chref.chid].value = value;
}

void e2db_abstract::add_tunersets(tunersets& tv)
{
	tuners.emplace(tv.ytype, tv);
}

void e2db_abstract::add_tunersets_table(int idx, tunersets_table& tn, tunersets& tv)
{
	string iname = "tns:";
	char yname = value_transponder_type(tn.ytype);
	iname += yname;

	char tnid[25];
	std::snprintf(tnid, 25, "%c:%04x", yname, idx);
	tn.tnid = tnid;
	tn.index = idx;

	tv.tables.emplace(tn.tnid, tn);
	index[iname].emplace_back(pair (idx, tn.tnid));
	if (tn.ytype == YTYPE::satellite)
		tuners_pos.emplace(tn.pos, tn.tnid);
}

void e2db_abstract::add_tunersets_transponder(int idx, tunersets_transponder& tntxp, tunersets_table& tn)
{
	char yname = value_transponder_type(tn.ytype);

	char trid[25];
	std::snprintf(trid, 25, "%c:%04x:%04x", yname, tntxp.freq, tntxp.sr);
	tntxp.trid = trid;
	tntxp.index = idx;

	tn.transponders.emplace(tntxp.trid, tntxp);
	index[tn.tnid].emplace_back(pair (idx, tntxp.trid));
}

void e2db_abstract::set_parentallock(string chid, string bname)
{
	if (! bname.empty() && userbouquets.count(bname))
		userbouquets[bname].locked = true;
	else if (! chid.empty() && db.services.count(chid))
		db.services[chid].locked = true;
}

void e2db_abstract::fix_crlf(string& line)
{
	if (line.size() != 0 && line[line.size() - 1] == '\r')
		line = line.substr(0, line.size() - 1);
}

int e2db_abstract::get_e2db_services_type()
{
	debug("get_e2db_services_type");

	return db.type;
}

void e2db_abstract::set_e2db_services_type(int type)
{
	debug("set_e2db_services_type");

	db.type = !! type;
}

int e2db_abstract::get_lamedb_version()
{
	debug("get_lamedb_version", "version", LAMEDB_VER);

	return LAMEDB_VER;
}

void e2db_abstract::set_lamedb_version(int ver)
{
	debug("set_lamedb_version", "version", ver);

	LAMEDB_VER = ver;
}

int e2db_abstract::get_zapit_version()
{
	debug("get_zapit_version", "version", ZAPIT_VER);

	return ZAPIT_VER;
}

void e2db_abstract::set_zapit_version(int ver)
{
	debug("set_zapit_version", "version", ZAPIT_VER);

	ZAPIT_VER = ver;
}

void e2db_abstract::set_index(unordered_map<string, vector<pair<int, string>>> index)
{
	debug("set_index");

	this->index = index;
}

unordered_map<string, vector<pair<int, string>>> e2db_abstract::get_index()
{
	debug("get_index");

	return this->index;
}

void e2db_abstract::set_transponders(unordered_map<string, e2db_abstract::transponder> transponders)
{
	debug("set_transponders");

	this->db.transponders = transponders;
}

unordered_map<string, e2db_abstract::transponder> e2db_abstract::get_transponders()
{
	debug("get_transponders");

	return this->db.transponders;
}

void e2db_abstract::set_services(unordered_map<string, e2db_abstract::service> services)
{
	debug("set_services");

	this->db.services = services;
}

unordered_map<string, e2db_abstract::service> e2db_abstract::get_services()
{
	debug("get_services");

	return this->db.services;
}

void e2db_abstract::set_bouquets(pair<unordered_map<string, e2db_abstract::bouquet>, unordered_map<string, e2db_abstract::userbouquet>> bouquets)
{
	debug("set_bouquets");

	this->bouquets = bouquets.first;
	this->userbouquets = bouquets.second;
}

pair<unordered_map<string, e2db_abstract::bouquet>, unordered_map<string, e2db_abstract::userbouquet>> e2db_abstract::get_bouquets()
{
	debug("get_bouquets");

	return pair (this->bouquets, this->userbouquets);
}

void e2db_abstract::merge(e2db_abstract* dst)
{
	debug("merge");

	error("merge", "Error", "Abstract merge swap objects.");

	this->db = dst->db;
	this->bouquets = dst->bouquets;
	this->userbouquets = dst->userbouquets;
	this->tuners = dst->tuners;
	this->tuners_pos = dst->tuners_pos;
	this->datas = dst->datas;
	this->comments = dst->comments;
	this->index = dst->index;
	this->collisions = dst->collisions;
}

void e2db_abstract::debugger()
{
	debug("debugger");

	cout << "transponders" << endl << endl;
	for (auto & x : db.transponders)
	{
		transponder tx = x.second;

		cout << "txid: " << tx.txid << endl;
		cout << "ytype: " << tx.ytype << endl;
		cout << "pos: " << tx.pos << endl;
		cout << "tsid: " << tx.tsid << endl;
		cout << hex;
		cout << "onid: " << tx.onid << endl;
		cout << "dvbns: " << tx.dvbns << endl;
		cout << dec;
		cout << "sys: " << tx.sys << endl;
		cout << "freq: " << tx.freq << endl;
		cout << "sr: " << tx.sr << endl;
		cout << "pol: " << tx.pol << endl;
		cout << "fec: " << tx.fec << endl;
		cout << "hpfec: " << tx.hpfec << endl;
		cout << "lpfec: " << tx.lpfec << endl;
		cout << "cfec: " << tx.cfec << endl;
		cout << "inv: " << tx.inv << endl;
		cout << "mod: " << tx.mod << endl;
		cout << "tmod: " << tx.tmod << endl;
		cout << "cmod: " << tx.cmod << endl;
		cout << "amod: " << tx.amod << endl;
		cout << "rol: " << tx.rol << endl;
		cout << "pil: " << tx.pil << endl;
		cout << "band: " << tx.band << endl;
		cout << "tmx: " << tx.tmx << endl;
		cout << "guard: " << tx.guard << endl;
		cout << "hier: " << tx.hier << endl;
		cout << "plpid: " << tx.plpid << endl;
		cout << "plsn: " << tx.plsn << endl;
		cout << "plscode: " << tx.plscode << endl;
		cout << "plsmode: " << tx.plsmode << endl;
		cout << "isid: " << tx.isid << endl;
		cout << "flags: " << tx.flags << endl;
		cout << "idx: " << tx.index << endl;
		cout << endl;
	}
	cout << endl;

	cout << "services" << endl << endl;
	for (auto & x : db.services)
	{
		service ch = x.second;

		cout << "chid: " << ch.chid << endl;
		cout << "txid: " << ch.txid << endl;
		cout << "chname: " << ch.chname << endl;
		cout << "ssid: " << ch.ssid << endl;
		cout << "tsid: " << ch.tsid << endl;
		cout << hex;
		cout << "onid: " << ch.onid << endl;
		cout << "dvbns: " << ch.dvbns << endl;
		cout << dec;
		cout << "onid: " << ch.onid << endl;
		cout << "stype: " << ch.stype << endl;
		cout << "snum: " << ch.snum << endl;
		cout << "srcid: " << ch.srcid << endl;
		cout << "data: " << endl << "[" << endl;
		for (auto & q : ch.data)
		{
			cout << q.first << ": " << endl << "[" << endl;
			for (string & w : q.second)
				cout << w << ", ";
			cout << endl << "]";
		}
		cout << "]" << endl;
		cout << "locked: " << ch.locked << endl;
		cout << "idx: " << ch.index << endl;
		cout << endl;
	}
	cout << endl;

	cout << "bouquets" << endl << endl;
	for (auto & x : bouquets)
	{
		bouquet bs = x.second;

		cout << "bname: " << bs.bname << endl;
		cout << "rname: " << bs.rname << endl;
		cout << "btype: " << bs.btype << endl;
		cout << "name: " << bs.name << endl;
		cout << "nname: " << bs.nname << endl;
		cout << "userbouquets: " << endl << "[" << endl;
		for (string & w : bs.userbouquets)
		{
			cout << w << endl;
		}
		cout << "]" << endl;
		cout << "idx: " << bs.index << endl;
		cout << endl;
	}
	cout << endl;

	cout << "userbouquets" << endl << endl;
	for (auto & x : userbouquets)
	{
		userbouquet ub = x.second;

		cout << "bname: " << ub.bname << endl;
		cout << "rname: " << ub.rname << endl;
		cout << "name: " << ub.name << endl;
		cout << "pname: " << ub.pname << endl;
		cout << "channels: " << endl << "[" << endl;
		for (auto & x : ub.channels)
		{
			channel_reference chref = x.second;

			cout << "chid: " << chref.chid << endl;
			cout << "idx: " << chref.index << endl;
			cout << endl;
		}
		cout << "]" << endl;
		cout << "locked: " << ub.locked << endl;
		cout << "hidden: " << ub.hidden << endl;
		cout << "idx: " << ub.index << endl;
		cout << endl;
	}
	cout << endl;

	cout << "tuners" << endl << endl;
	for (auto & x : tuners)
	{
		tunersets tv = x.second;

		cout << "tvid: " << tv.ytype << endl;
		cout << "ytype: " << tv.ytype << endl;
		cout << "charset: " << tv.charset << endl;
		cout << "tables: " << endl << "[" << endl;
		for (auto & x : tv.tables)
		{
			tunersets_table tn = x.second;

			cout << "tnid: " << tn.tnid << endl;
			cout << "ytype: " << tn.ytype << endl;
			cout << "name: " << tn.name << endl;
			cout << "pos: " << tn.pos << endl;
			cout << "country: " << tn.country << endl;
			cout << "feed: " << tn.feed << endl;
			cout << "flags: " << tn.flags << endl;
			cout << "transponders: " << endl << "[" << endl;
			for (auto & x : tn.transponders)
			{
				tunersets_transponder tntxp = x.second;

				cout << "trid: " << tntxp.trid << endl;
				cout << "tnid: " << tn.tnid << endl;
				cout << "ytype: " << tn.ytype << endl;
				cout << "pos: " << tn.pos << endl;
				cout << "sys: " << tntxp.sys << endl;
				cout << "freq: " << tntxp.freq << endl;
				cout << "pol: " << tntxp.pol << endl;
				cout << "sr: " << tntxp.sr << endl;
				cout << "mod: " << tntxp.mod << endl;
				cout << "tmod: " << tntxp.tmod << endl;
				cout << "cmod: " << tntxp.cmod << endl;
				cout << "amod: " << tntxp.amod << endl;
				cout << "band: " << tntxp.band << endl;
				cout << "tmx: " << tntxp.tmx << endl;
				cout << "fec: " << tntxp.fec << endl;
				cout << "hpfec: " << tntxp.hpfec << endl;
				cout << "lpfec: " << tntxp.lpfec << endl;
				cout << "cfec: " << tntxp.cfec << endl;
				cout << "inv: " << tntxp.inv << endl;
				cout << "rol: " << tntxp.rol << endl;
				cout << "pil: " << tntxp.pil << endl;
				cout << "guard: " << tntxp.guard << endl;
				cout << "hier: " << tntxp.hier << endl;
				cout << "plpid: " << tntxp.plpid << endl;
				cout << "plsn: " << tntxp.plsn << endl;
				cout << "plscode: " << tntxp.plscode << endl;
				cout << "plsmode: " << tntxp.plsmode << endl;
				cout << "isid: " << tntxp.isid << endl;
				cout << "mts: " << tntxp.mts << endl;
				cout << "idx: " << tntxp.index << endl;
				cout << endl;
			}
			cout << "]" << endl;
			cout << "idx: " << tn.index << endl;
		}
		cout << "]" << endl;
	}
	cout << endl;
}

}
