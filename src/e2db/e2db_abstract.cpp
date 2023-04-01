/*!
 * e2-sat-editor/src/e2db/e2db_abstract.cpp
 * 
 * @link https://github.com/ctlcltd/e2-sat-editor
 * @copyright e2 SAT Editor Team
 * @author Leonardo Laureti
 * @version 0.6
 * @license MIT License
 * @license GNU GPLv3 License
 */

#include <cstdio>
#include <cstring>
#include <stdexcept>
#include <filesystem>
#include <algorithm>
#include <iostream>

#include "e2db_abstract.h"

using std::string, std::pair, std::hex, std::dec, std::to_string, std::cout, std::endl;

namespace e2se_e2db
{

string e2db_abstract::editor_string(int html)
{
	if (html == 2)
		return "e2 SAT Editor 0.6";
	else if (html == 1)
		return "e2 SAT Editor 0.6 <a href=\"https://github.com/ctlcltd/e2-sat-editor\">https://github.com/ctlcltd/e2-sat-editor</a>";
	else
		return "e2 SAT Editor 0.6 <https://github.com/ctlcltd/e2-sat-editor>";
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
	return FPORTS::unsupported;
}

string e2db_abstract::file_mime_detect(FPORTS fpi, string path)
{
	switch (fpi)
	{
		case FPORTS::directory:
		case FPORTS::unsupported:
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
				return "application/octet-stream";
	}
}

void e2db_abstract::value_channel_reference(string str, channel_reference& chref, service_reference& ref)
{
	int i, atype, anum, ssid, tsid, onid, dvbns;
	i = 0, atype = 0, anum = 0, ssid = 0, tsid = 0, onid = 0, dvbns = 0;

	std::sscanf(str.c_str(), "%d:%d:%X:%X:%X:%X:%X", &i, &atype, &anum, &ssid, &tsid, &onid, &dvbns);

	switch (atype)
	{
		// marker
		case STYPE::regular_marker:
		case STYPE::numbered_marker:
		case STYPE::hidden_marker_1:
		case STYPE::hidden_marker_2:
			chref.marker = true;
		break;
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

	chref.atype = atype;
	chref.anum = anum;
}

string e2db_abstract::value_reference_id(service ch)
{
	int stype = ch.stype != -1 ? ch.stype : 0;
	int snum = ch.snum != -1 ? ch.snum : 0;
	int ssid = ch.ssid;
	int tsid = ch.tsid;
	int onid = ch.onid;
	int dvbns = ch.dvbns;

	char refid[44];
	// %1d:%4d:%4X:%4X:%4X:%4X:%6X:0:0:0:
	std::snprintf(refid, 44, "%d:%d:%X:%X:%X:%X:%X:0:0:0", 1, stype, snum, ssid, tsid, onid, dvbns);
	return refid;
}

string e2db_abstract::value_reference_id(channel_reference chref)
{
	int atype = chref.atype != -1 ? chref.atype : 0;
	int anum = chref.anum != -1 ? chref.anum : 0;
	int ssid = 0;
	int tsid = 0;
	int onid = 0;
	int dvbns = 0;

	char refid[44];
	// %1d:%4d:%4X:%4X:%4X:%4X:%6X:0:0:0:
	std::snprintf(refid, 44, "%d:%d:%X:%X:%X:%X:%X:0:0:0", 1, atype, anum, ssid, tsid, onid, dvbns);
	return refid;
}

string e2db_abstract::value_reference_id(channel_reference chref, service ch)
{
	int atype, anum;
	int ssid = 0;
	int tsid = 0;
	int onid = 0;
	int dvbns = 0;

	if (! chref.marker)
	{
		atype = ch.stype != -1 ? ch.stype : 0;
		anum = ch.snum != -1 ? ch.snum : 0;
		ssid = ch.ssid;
		tsid = ch.tsid;
		onid = ch.onid;
		dvbns = ch.dvbns;
	}
	else
	{
		atype = chref.atype != -1 ? chref.atype : 0;
		anum = chref.anum != -1 ? chref.anum : 0;
	}

	char refid[44];
	// %1d:%4d:%4X:%4X:%4X:%4X:%6X:0:0:0:
	std::snprintf(refid, 44, "%d:%d:%X:%X:%X:%X:%X:0:0:0", 1, atype, anum, ssid, tsid, onid, dvbns);
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

vector<string> e2db_abstract::value_channel_caid(string str)
{
	vector<string> caid;
	char* token = std::strtok(str.data(), "|");
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

		token = std::strtok(NULL, "|");
	}
	return caid;
}

vector<string> e2db_abstract::value_channel_cached(string str)
{
	vector<string> cached;
	char* token = std::strtok(str.data(), "|");
	while (token != 0)
	{
		cached.emplace_back(token);
		token = std::strtok(NULL, "|");
	}
	return cached;
}

int e2db_abstract::value_transponder_type(char ty)
{
	switch (ty) {
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
	switch (ytype) {
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
	switch (tx.ytype)
	{
		case YTYPE::satellite:
			ptxp = to_string(tx.freq) + '/' + SAT_POL[tx.pol] + '/' + to_string(tx.sr);
		break;
		case YTYPE::terrestrial:
			ptxp = to_string(tx.freq) + '/' + TER_MOD[tx.tmod] + '/' + TER_BAND[tx.band];
		break;
		case YTYPE::cable:
			ptxp = to_string(tx.freq) + '/' + CAB_MOD[tx.cmod] + '/' + to_string(tx.sr);
		break;
		case YTYPE::atsc:
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
		case YTYPE::satellite:
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

int e2db_abstract::value_transponder_dvbns(string str)
{
	int dvbns = 0;
	std::sscanf(str.c_str(), "%X", &dvbns);
	return dvbns;
}

int e2db_abstract::value_transponder_dvbns(transponder tx)
{
	return value_transponder_dvbns(tx.pos, tx.onid, tx.freq, tx.pol);
}

//TODO
int e2db_abstract::value_transponder_dvbns(int pos, int onid, int freq, int pol)
{
	return value_transponder_dvbns(pos, onid);
}

int e2db_abstract::value_transponder_dvbns(int pos, int onid)
{
	return pos << 16;
}

string e2db_abstract::value_transponder_dvbns(int dvbns)
{
	char cdvbns[7];
	std::snprintf(cdvbns, 7, "%6X", dvbns);
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
		catch (const std::invalid_argument& err)
		{
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
	switch (ytype) {
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
			lp_fec = str.substr(pos);
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

	if (! chref.marker && db.services.count(chref.chid))
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
	char chid[25];
	char txid[25];
	// %4x:%8x
	std::snprintf(txid, 25, "%x:%x", ch.tsid, ch.dvbns);
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
		// %4d:%2x:%d
		std::snprintf(chid, 25, "%d:%x:%d", chref.atype, chref.anum, ub.index);
	else
		// %4x:%4x:%8x
		std::snprintf(chid, 25, "%x:%x:%x", ref.ssid, ref.tsid, ref.dvbns);

	chref.chid = chid;
	chref.index = idx;

	bouquet& bs = bouquets[ub.pname];
	ub.channels.emplace(chref.chid, chref);
	index[ub.bname].emplace_back(pair (idx, chref.chid));

	if (chref.marker)
	{
		index["mks"].emplace_back(pair (ub.index, chref.chid));
	}
	else if (bs.services.count(chref.chid) == 0)
	{
		int idx = int (index[ub.pname].size());
		idx += 1;
		bs.services.emplace(chref.chid);
		index[ub.pname].emplace_back(pair (idx, chref.chid));
	}
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

int e2db_abstract::get_e2db_services_type() {
	debug("get_e2db_services_type");

	return db.type;
}

void e2db_abstract::set_e2db_services_type(int type) {
	debug("set_e2db_services_type");

	db.type = !! type;
}

int e2db_abstract::get_lamedb_version() {
	debug("get_lamedb_version", "version", LAMEDB_VER);

	return LAMEDB_VER;
}

void e2db_abstract::set_lamedb_version(int ver) {
	debug("set_lamedb_version", "version", ver);

	LAMEDB_VER = ver;
}

int e2db_abstract::get_zapit_version() {
	debug("get_zapit_version", "version", ZAPIT_VER);

	return ZAPIT_VER;
}

void e2db_abstract::set_zapit_version(int ver) {
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
		cout << "txid: " << x.first << endl;
		cout << hex;
		cout << "dvbns: " << x.second.dvbns << endl;
		cout << "tsid: " << x.second.tsid << endl;
		cout << "onid: " << x.second.onid << endl;
		cout << dec;
		cout << "ytype: " << x.second.ytype << endl;
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

string e2db_abstract::trs(string str)
{
	return str;
}

string e2db_abstract::trf(string str, string param)
{
	size_t tsize = str.size() + param.size();
	char tstr[tsize];
	std::snprintf(tstr, tsize, str.c_str(), param.c_str());
	return string (tstr);
}

}
