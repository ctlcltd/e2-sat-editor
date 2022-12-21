/*!
 * e2-sat-editor/src/e2db/e2db_converter.cpp
 * 
 * @link https://github.com/ctlcltd/e2-sat-editor
 * @copyright e2 SAT Editor Team
 * @author Leonardo Laureti
 * @version 0.1
 * @license MIT License
 * @license GNU GPLv3 License
 */

#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <cstring>
#include <unordered_set>
#include <algorithm>
#include <sstream>
#include <fstream>
#include <iostream>
#include <filesystem>

#include "e2db_converter.h"

using std::ifstream, std::ofstream, std::stringstream, std::hex, std::dec, std::to_string, std::unordered_set, std::cout, std::endl;

namespace e2se_e2db
{

/*e2db_converter::e2db_converter(e2se::logger::session log)
{
	this->log = new e2se::logger(log, "e2db");

	debug("e2db_converter()");
}*/

/*e2db_converter::e2db_converter()
{
}*/

void e2db_converter::import_csv_file(FCONVS fci, fcopts opts, vector<string> paths)
{
	debug("import_csv_file()", "file path", "multiple");
	debug("import_csv_file()", "file input", fci);

	bool merge = this->get_input().size() != 0 ? true : false;
	auto* dbih = merge ? newptr() : this;

	// string filename = std::filesystem::path(path).filename().u8string(); //C++17
	// ifstream ifile (path);
}

void e2db_converter::import_csv_file(FCONVS fci, fcopts opts, string path)
{
	debug("import_csv_file()", "file path", "singular");
	debug("import_csv_file()", "file input", fci);

	// bool merge = this->get_input().size() != 0 ? true : false;
	// e2db* dbih = merge ? new e2db : this;

	string filename = std::filesystem::path(path).filename().u8string(); //C++17
	ifstream ifile (path);
}

void e2db_converter::export_csv_file(FCONVS fco, fcopts opts, string path)
{
	debug("export_csv_file()", "file path", "singular");
	debug("export_csv_file()", "file output", fco);

	vector<e2db_file> files;
	string filename = std::filesystem::path(path).filename().u8string(); //C++17
	opts.filename = filename;

	switch (fco)
	{
		case FCONVS::convert_all:
			push_csv_all(files);
		break;
		case FCONVS::convert_services:
			if (opts.fc == FCONVS::convert_current)
				push_csv_services(files, opts.stype);
			else
				push_csv_services(files);
		break;
		case FCONVS::convert_bouquets:
			if (opts.fc == FCONVS::convert_current)
				push_csv_bouquets(files, opts.bname);
			else
				push_csv_bouquets(files);
		break;
		case FCONVS::convert_userbouquets:
			if (opts.fc == FCONVS::convert_current)
				push_csv_userbouquets(files, opts.bname);
			else
				push_csv_userbouquets(files);
		break;
		case FCONVS::convert_tunersets:
			if (opts.fc == FCONVS::convert_current)
				push_csv_tunersets(files, opts.ytype);
			else
				push_csv_tunersets(files);
		break;
		default:
			return;
	}

	for (auto & file : files)
	{
		ofstream out (path);
		out << file;
		out.close();
	}
}

void e2db_converter::export_html_file(FCONVS fco, fcopts opts, string path)
{
	debug("export_html_file()", "file path", "singular");
	debug("export_html_file()", "file output", fco);

	vector<e2db_file> files;
	string filename = std::filesystem::path(path).filename().u8string(); //C++17
	opts.filename = filename;

	switch (fco)
	{
		case FCONVS::convert_all:
			push_html_all(files);
		break;
		case FCONVS::convert_index:
			push_html_index(files);
		break;
		case FCONVS::convert_services:
			if (opts.fc == FCONVS::convert_current)
				push_html_services(files, opts.stype);
			else
				push_html_services(files);
		break;
		case FCONVS::convert_bouquets:
			if (opts.fc == FCONVS::convert_current)
				push_html_bouquets(files, opts.bname);
			else
				push_html_bouquets(files);
		break;
		case FCONVS::convert_userbouquets:
			if (opts.fc == FCONVS::convert_current)
				push_html_userbouquets(files, opts.bname);
			else
				push_html_userbouquets(files);
		break;
		case FCONVS::convert_tunersets:
			if (opts.fc == FCONVS::convert_current)
				push_html_tunersets(files, opts.ytype);
			else
				push_html_tunersets(files);
		break;
		default:
			return;
	}

	for (auto & file : files)
	{
		ofstream out (path);
		out << file;
		out.close();
	}
}

void e2db_converter::push_csv_all(vector<e2db_file>& files)
{
	debug("push_csv_all()");

	push_csv_services(files);
	push_csv_bouquets(files);
	push_csv_userbouquets(files);
	push_csv_tunersets(files);
}

void e2db_converter::push_csv_services(vector<e2db_file>& files)
{
	return push_csv_services(files, -1);
}

void e2db_converter::push_csv_services(vector<e2db_file>& files, int stype)
{
	debug("push_csv_services()");
	
	string filename = std::filesystem::path(get_filename()).filename().u8string(); //C++17
	string iname;
	switch (stype)
	{
		// Data
		case 0:
			iname = "chs:0";
		break;
		// TV
		case 1:
			iname = "chs:1";
		break;
		// Radio
		case 2:
			iname = "chs:2";
		break;
		// All Services
		default:
			iname = "chs";
	}

	string csv;
	csv_channel_list(csv, iname, DOC_VIEW::view_services);

	e2db_file file;
	csv_document(file, csv);
	files.emplace_back(file);
}

void e2db_converter::push_csv_bouquets(vector<e2db_file>& files)
{
	debug("push_csv_bouquet()");

	for (auto & x : index["bss"])
	{
		string bname = x.second;
		bouquet bs = bouquets[bname];
		push_csv_bouquets(files, bname);
	}
}

void e2db_converter::push_csv_bouquets(vector<e2db_file>& files, string bname)
{
	debug("push_csv_bouquets()", "bname", bname);

	string csv;
	csv_bouquet_list(csv, bname);

	e2db_file file;
	csv_document(file, csv);
	files.emplace_back(file);
}

void e2db_converter::push_csv_userbouquets(vector<e2db_file>& files)
{
	debug("push_csv_userbouquet()");

	for (auto & x : index["ubs"])
	{
		string bname = x.second;
		userbouquet ub = userbouquets[bname];
		push_csv_userbouquets(files, bname);
	}
}

void e2db_converter::push_csv_userbouquets(vector<e2db_file>& files, string bname)
{
	debug("push_csv_userbouquet()", "bname", bname);

	string csv;
	csv_channel_list(csv, bname, DOC_VIEW::view_userbouquets);

	e2db_file file;
	csv_document(file, csv);
	files.emplace_back(file);
}

void e2db_converter::push_csv_tunersets(vector<e2db_file>& files)
{
	debug("push_csv_tunersets()");

	for (auto & x : tuners)
	{
		push_csv_tunersets(files, x.first);
	}
}

void e2db_converter::push_csv_tunersets(vector<e2db_file>& files, int ytype)
{
	debug("push_csv_bouquet()", "ytype", ytype);

	string csv;
	csv_tunersets_list(csv, ytype);

	e2db_file file;
	csv_document(file, csv);
	files.emplace_back(file);
}


void e2db_converter::push_html_all(vector<e2db_file>& files)
{
	debug("push_html_all()");

	push_html_index(files);
	push_html_services(files);
	push_html_bouquets(files);
	push_html_userbouquets(files);
	push_html_tunersets(files);
}

void e2db_converter::push_html_index(vector<e2db_file>& files)
{
	debug("push_html_index()");

	string filename = std::filesystem::path(get_localdir()).filename().u8string(); //C++17
	if (filename.empty())
	{
		filename = "Untitled";
	}

	html_page page;
	page_header(page, filename, DOC_VIEW::view_index);
	page_footer(page, filename, DOC_VIEW::view_index);

	vector<string> paths;
	for (auto & x : get_input())
	{
		paths.emplace_back(x.first);
	}
	page_body_index_list(page, paths);

	e2db_file file;
	html_document(file, page);
	files.emplace_back(file);
}

void e2db_converter::push_html_services(vector<e2db_file>& files)
{
	return push_html_services(files, -1);
}

void e2db_converter::push_html_services(vector<e2db_file>& files, int stype)
{
	debug("push_html_services()");
	
	string filename = std::filesystem::path(get_filename()).filename().u8string(); //C++17
	string iname;
	string xname;
	string headname = filename;
	string footname = filename;
	switch (stype)
	{
		// Data
		case 0:
			iname = "chs:0";
			xname = "Data";
		break;
		// TV
		case 1:
			iname = "chs:1";
			xname = "TV";
		break;
		// Radio
		case 2:
			iname = "chs:2";
			xname = "Radio";
		break;
		// All Services
		default:
			iname = "chs";
	}
	if (! xname.empty())
	{
		headname += " <i>" + xname + "</i>";
		footname += " (extract)";
	}

	html_page page;
	page_header(page, headname, DOC_VIEW::view_services);
	page_footer(page, footname, DOC_VIEW::view_services);

	page_body_channel_list(page, iname, DOC_VIEW::view_services);

	e2db_file file;
	html_document(file, page);
	files.emplace_back(file);
}

void e2db_converter::push_html_bouquets(vector<e2db_file>& files)
{
	debug("push_html_bouquet()");

	for (auto & x : index["bss"])
	{
		string bname = x.second;
		bouquet bs = bouquets[bname];
		push_html_bouquets(files, bname);
	}
}

void e2db_converter::push_html_bouquets(vector<e2db_file>& files, string bname)
{
	debug("push_html_bouquets()", "bname", bname);

	html_page page;
	page_header(page, bname, DOC_VIEW::view_bouquets);
	page_footer(page, bname, DOC_VIEW::view_bouquets);

	page_body_bouquet_list(page, bname);

	e2db_file file;
	html_document(file, page);
	files.emplace_back(file);
}

void e2db_converter::push_html_userbouquets(vector<e2db_file>& files)
{
	debug("push_html_userbouquet()");

	for (auto & x : index["ubs"])
	{
		string bname = x.second;
		userbouquet ub = userbouquets[bname];
		push_html_userbouquets(files, bname);
	}
}

void e2db_converter::push_html_userbouquets(vector<e2db_file>& files, string bname)
{
	debug("push_html_userbouquet()", "bname", bname);

	html_page page;
	page_header(page, bname, DOC_VIEW::view_userbouquets);
	page_footer(page, bname, DOC_VIEW::view_userbouquets);

	page_body_channel_list(page, bname, DOC_VIEW::view_userbouquets);

	e2db_file file;
	html_document(file, page);
	files.emplace_back(file);
}

void e2db_converter::push_html_tunersets(vector<e2db_file>& files)
{
	debug("push_html_tunersets()");

	for (auto & x : tuners)
	{
		push_html_tunersets(files, x.first);
	}
}

void e2db_converter::push_html_tunersets(vector<e2db_file>& files, int ytype)
{
	debug("push_html_bouquet()", "ytype", ytype);

	string filename;
	switch (ytype)
	{
		case YTYPE::sat:
			filename = "satellites.xml";
		break;
		case YTYPE::terrestrial:
			filename = "terrestrial.xml";
		break;
		case YTYPE::cable:
			filename = "cables.xml";
		break;
		case YTYPE::atsc:
			filename = "atsc.xml";
		break;
	}

	html_page page;
	page_header(page, filename, DOC_VIEW::view_tunersets);
	page_footer(page, filename, DOC_VIEW::view_tunersets);

	page_body_tunersets_list(page, ytype);

	e2db_file file;
	html_document(file, page);
	files.emplace_back(file);
}

void e2db_converter::csv_channel_list(string& csv, string bname, DOC_VIEW view)
{
	if (index.count(bname))
		debug("csv_channel_list()", "bname", bname);
	else
		error("csv_channel_list()", "bname", bname);
	debug("csv_channel_list()", "view", view);

	stringstream ss;

	if (CSV_HEADER)
	{
		ss << CSV_ESCAPE << "Index" << CSV_ESCAPE << CSV_SEPARATOR;
		ss << CSV_ESCAPE << "Name" << CSV_ESCAPE << CSV_SEPARATOR;
		ss << CSV_ESCAPE << "Reference ID" << CSV_ESCAPE << CSV_SEPARATOR;
		ss << CSV_ESCAPE << "SSID" << CSV_ESCAPE << CSV_SEPARATOR;
		ss << CSV_ESCAPE << "TSID" << CSV_ESCAPE << CSV_SEPARATOR;
		ss << CSV_ESCAPE << "Type" << CSV_ESCAPE << CSV_SEPARATOR;
		ss << CSV_ESCAPE << "CAS" << CSV_ESCAPE << CSV_SEPARATOR;
		ss << CSV_ESCAPE << "Provider" << CSV_ESCAPE << CSV_SEPARATOR;
		ss << CSV_ESCAPE << "Freq" << CSV_ESCAPE << CSV_SEPARATOR;
		ss << CSV_ESCAPE << "Pol" << CSV_ESCAPE << CSV_SEPARATOR;
		ss << CSV_ESCAPE << "SR" << CSV_ESCAPE << CSV_SEPARATOR;
		ss << CSV_ESCAPE << "FEC" << CSV_ESCAPE << CSV_SEPARATOR;
		ss << CSV_ESCAPE << "Pos" << CSV_ESCAPE << CSV_SEPARATOR;
		ss << CSV_ESCAPE << "Sys" << CSV_ESCAPE << CSV_SEPARATOR;
		ss << CSV_ENDLINE;
	}

	for (auto & chdata : index[bname])
	{
		string chid = chdata.second;

		if (db.services.count(chdata.second))
		{
			service ch = db.services[chid];
			transponder tx = db.transponders[ch.txid];

			int idx = chdata.first;
			string chname = ch.chname;
			string refid;

			// services
			if (view == DOC_VIEW::view_services)
			{
				string crefid = get_reference_id(chid);
				refid = crefid;
			}
			// bouquets
			else
			{
				channel_reference chref;
				if (userbouquets.count(bname))
					chref = userbouquets[bname].channels[chid];
				string crefid = get_reference_id(chref);
				refid = crefid;
			}
			string ssid = to_string(ch.ssid);
			string tsid = to_string(ch.tsid);
			string stype = STYPE_EXT_LABEL.count(ch.stype) ? STYPE_EXT_LABEL.at(ch.stype) : STYPE_EXT_LABEL.at(STYPE::data);
			string scas;
			if (ch.data.count(SDATA::C))
			{
				unordered_set<string> _unique;
				vector<string> cas;

				for (string & w : ch.data[SDATA::C])
				{
					string caidpx = w.substr(0, 2);
					if (SDATA_CAS.count(caidpx) && ! _unique.count(caidpx))
					{
						cas.emplace_back(SDATA_CAS.at(caidpx));
						_unique.insert(caidpx);
					}
				}
				scas.append("$");
				scas.append(" ");
				for (unsigned int i = 0; i < cas.size(); i++)
				{
					scas.append(cas[i]);
					if (i != cas.size() - 1)
						scas.append(", ");
				}
			}
			string pname = ch.data.count(SDATA::p) ? ch.data[SDATA::p][0] : "";
			string freq = to_string(tx.freq);
			string pol = tx.pol != -1 ? SAT_POL[tx.pol] : "";
			string sr = to_string(tx.sr);
			string fec = SAT_FEC[tx.fec];
			string ppos = get_transponder_position_text(tx);
			string pos = ppos;
			string psys = get_transponder_system_text(tx);
			string sys = psys;

			ss << idx << CSV_SEPARATOR;
			ss << CSV_ESCAPE << chname << CSV_ESCAPE << CSV_SEPARATOR;
			ss << refid << CSV_SEPARATOR;
			ss << ssid << CSV_SEPARATOR;
			ss << tsid << CSV_SEPARATOR;
			ss << stype << CSV_SEPARATOR;
			ss << CSV_ESCAPE << scas << CSV_ESCAPE << CSV_SEPARATOR;
			ss << CSV_ESCAPE << pname << CSV_ESCAPE << CSV_SEPARATOR;
			ss << freq << CSV_SEPARATOR;
			ss << pol << CSV_SEPARATOR;
			ss << sr << CSV_SEPARATOR;
			ss << fec << CSV_SEPARATOR;
			ss << CSV_ESCAPE << pos << CSV_SEPARATOR;
			ss << sys << CSV_SEPARATOR;
			ss << CSV_ENDLINE;
		}
		else
		{
			channel_reference chref;
			if (userbouquets.count(bname))
				chref = userbouquets[bname].channels[chid];

			if (! chref.marker)
				continue;

			string cchid = chid;
			string value = chref.value;
			//TODO marker index and compatibility
			std::transform(cchid.begin(), cchid.end(), cchid.begin(), [](unsigned char c) { return toupper(c); });
			string refid = "1:" + cchid + ":0:0:0:0:0:0";
			string atype = "MARKER";

			ss << CSV_SEPARATOR;
			ss << CSV_ESCAPE << value << CSV_ESCAPE << CSV_SEPARATOR;
			ss << refid << CSV_SEPARATOR;
			ss << CSV_SEPARATOR;
			ss << CSV_SEPARATOR;
			ss << atype << CSV_SEPARATOR;
			ss << CSV_SEPARATOR;
			ss << CSV_SEPARATOR;
			ss << CSV_SEPARATOR;
			ss << CSV_SEPARATOR;
			ss << CSV_SEPARATOR;
			ss << CSV_SEPARATOR;
			ss << CSV_SEPARATOR;
			ss << CSV_SEPARATOR;
			ss << CSV_ENDLINE;
		}
	}

	csv = ss.str();
}

void e2db_converter::csv_bouquet_list(string& csv, string bname)
{
	if (bouquets.count(bname))
		debug("csv_bouquet_list()", "bname", bname);
	else
		error("csv_bouquet_list()", "bname", bname);

	bouquet bs = bouquets[bname];

	stringstream ss;

	if (CSV_HEADER)
	{
		ss << CSV_ESCAPE << "Index" << CSV_ESCAPE << CSV_SEPARATOR;
		ss << CSV_ESCAPE << "Userbouquet" << CSV_ESCAPE << CSV_SEPARATOR;
		ss << CSV_ESCAPE << "Name" << CSV_ESCAPE << CSV_SEPARATOR;
		ss << CSV_ESCAPE << "Type" << CSV_ESCAPE << CSV_SEPARATOR;
		ss << CSV_ENDLINE;
	}

	int i = 1;
	for (auto & bname : bs.userbouquets)
	{
		userbouquet ub = userbouquets[bname];
		string btype;
		if (bs.btype == 1)
			btype = "TV";
		else if (bs.btype == 2)
			btype = "Radio";
		ss << CSV_ESCAPE << i++ << CSV_SEPARATOR;
		ss << CSV_ESCAPE << bname << CSV_ESCAPE << CSV_SEPARATOR;
		ss << CSV_ESCAPE << ub.name << CSV_ESCAPE << CSV_SEPARATOR;
		ss << btype << CSV_SEPARATOR;
		ss << CSV_ENDLINE;
	}

	csv = ss.str();
}

//TODO improve list
void e2db_converter::csv_tunersets_list(string& csv, int ytype)
{
	debug("csv_tunersets_list()", "ytype", ytype);
	
	stringstream ss;

	tunersets tv = tuners[ytype];
	string iname = "tns:";
	switch (ytype)
	{
		case YTYPE::sat:
			iname += 's';
		break;
		case YTYPE::terrestrial:
			iname += 't';
		break;
		case YTYPE::cable:
			iname += 'c';
		break;
		case YTYPE::atsc:
			iname += 'a';
		break;
	}
	
	for (auto & x : index[iname])
	{
		string tnid = x.second;
		tunersets_table tn = tv.tables[tnid];
		string tnname = tn.name;
		string ppos;
		if (ytype == YTYPE::sat)
		{
			ppos = get_transponder_position_text(tn);
		}
		string pos = ppos;

		if (CSV_HEADER)
		{
			ss << CSV_ESCAPE << "Name" << CSV_ESCAPE << CSV_SEPARATOR;
			ss << CSV_ESCAPE << "Position" << CSV_ESCAPE << CSV_SEPARATOR;
			ss << CSV_ESCAPE << "Index" << CSV_ESCAPE << CSV_SEPARATOR;
			if (ytype == YTYPE::sat)
			{
				ss << CSV_ESCAPE << "Freq" << CSV_ESCAPE << CSV_SEPARATOR;
				ss << CSV_ESCAPE << "Pol" << CSV_ESCAPE << CSV_SEPARATOR;
				ss << CSV_ESCAPE << "SR" << CSV_ESCAPE << CSV_SEPARATOR;
				ss << CSV_ESCAPE << "FEC" << CSV_ESCAPE << CSV_SEPARATOR;
				ss << CSV_ESCAPE << "Sys" << CSV_ESCAPE << CSV_SEPARATOR;
				ss << CSV_ESCAPE << "Mod" << CSV_ESCAPE << CSV_SEPARATOR;
				ss << CSV_ESCAPE << "Inv" << CSV_ESCAPE << CSV_SEPARATOR;
				ss << CSV_ESCAPE << "Pilot" << CSV_ESCAPE << CSV_SEPARATOR;
				ss << CSV_ESCAPE << "Rollof" << CSV_ESCAPE << CSV_SEPARATOR;
			}
			else if (ytype == YTYPE::terrestrial)
			{
				ss << CSV_ESCAPE << "Freq" << CSV_ESCAPE << CSV_SEPARATOR;
				ss << CSV_ESCAPE << "Const" << CSV_ESCAPE << CSV_SEPARATOR;
				ss << CSV_ESCAPE << "Band" << CSV_ESCAPE << CSV_SEPARATOR;
				ss << CSV_ESCAPE << "Sys" << CSV_ESCAPE << CSV_SEPARATOR;
				ss << CSV_ESCAPE << "Tx Mode" << CSV_ESCAPE << CSV_SEPARATOR;
				ss << CSV_ESCAPE << "HP FEC" << CSV_ESCAPE << CSV_SEPARATOR;
				ss << CSV_ESCAPE << "LP FEC" << CSV_ESCAPE << CSV_SEPARATOR;
				ss << CSV_ESCAPE << "Inv" << CSV_ESCAPE << CSV_SEPARATOR;
				ss << CSV_ESCAPE << "Guard" << CSV_ESCAPE << CSV_SEPARATOR;
				ss << CSV_ESCAPE << "Hier" << CSV_ESCAPE << CSV_SEPARATOR;
			}
			else if (ytype == YTYPE::cable)
			{
				ss << CSV_ESCAPE << "Freq" << CSV_ESCAPE << CSV_SEPARATOR;
				ss << CSV_ESCAPE << "Mod" << CSV_ESCAPE << CSV_SEPARATOR;
				ss << CSV_ESCAPE << "SR" << CSV_ESCAPE << CSV_SEPARATOR;
				ss << CSV_ESCAPE << "FEC" << CSV_ESCAPE << CSV_SEPARATOR;
				ss << CSV_ESCAPE << "Inv" << CSV_ESCAPE << CSV_SEPARATOR;
				ss << CSV_ESCAPE << "Sys" << CSV_ESCAPE << CSV_SEPARATOR;
			}
			else if (ytype == YTYPE::atsc)
			{
				ss << CSV_ESCAPE << "Freq" << CSV_ESCAPE << CSV_SEPARATOR;
				ss << CSV_ESCAPE << "Mod" << CSV_ESCAPE << CSV_SEPARATOR;
				ss << CSV_ESCAPE << "Sys" << CSV_ESCAPE << CSV_SEPARATOR;
			}
			ss << CSV_ENDLINE;
		}

		int i = 1;
		for (auto & x : index[tn.tnid])
		{
			string trid = x.second;
			tunersets_transponder tntxp = tn.transponders[trid];

			ss << i++ << CSV_SEPARATOR;
			ss << CSV_ESCAPE << tnname << CSV_ESCAPE << CSV_SEPARATOR;
			ss << pos << CSV_SEPARATOR;
			if (ytype == YTYPE::sat)
			{
				int freq = tntxp.freq;
				string pol = SAT_POL[tntxp.pol];
				int sr = tntxp.sr;
				string fec = SAT_FEC[tntxp.fec];
				string sys = SAT_SYS[tntxp.sys];
				string mod = SAT_MOD[tntxp.mod];
				string inv = SAT_INV[tntxp.inv];
				string pil = SAT_PIL[tntxp.pil];
				string rol = SAT_ROL[tntxp.rol];

				ss << freq << CSV_SEPARATOR;
				ss << pol << CSV_SEPARATOR;
				ss << sr << CSV_SEPARATOR;
				ss << fec << CSV_SEPARATOR;
				ss << sys << CSV_SEPARATOR;
				ss << mod << CSV_SEPARATOR;
				ss << inv << CSV_SEPARATOR;
				ss << pil << CSV_SEPARATOR;
				ss << rol << CSV_SEPARATOR;
			}
			else if (ytype == YTYPE::terrestrial)
			{
				int freq = tntxp.freq;
				string tmod = TER_MOD[tntxp.tmod];
				string band = TER_BAND[tntxp.band];
				string sys = "DVB-T";
				string tmx = TER_TRXMODE[tntxp.tmx];
				string hpfec = TER_HPFEC[tntxp.hpfec];
				string lpfec = TER_LPFEC[tntxp.lpfec];
				string inv = TER_INV[tntxp.inv];
				string guard = TER_GUARD[tntxp.guard];
				string hier = TER_HIER[tntxp.hier];

				ss << freq << CSV_SEPARATOR;
				ss << tmod << CSV_SEPARATOR;
				ss << band << CSV_SEPARATOR;
				ss << sys << CSV_SEPARATOR;
				ss << tmx << CSV_SEPARATOR;
				ss << hpfec << CSV_SEPARATOR;
				ss << lpfec << CSV_SEPARATOR;
				ss << inv << CSV_SEPARATOR;
				ss << guard << CSV_SEPARATOR;
				ss << hier << CSV_SEPARATOR;
			}
			else if (ytype == YTYPE::cable)
			{
				int freq = tntxp.freq;
				string cmod = CAB_MOD[tntxp.cmod];
				int sr = tntxp.sr;
				string cfec = CAB_IFEC[tntxp.cfec];
				string inv = CAB_INV[tntxp.inv];
				string sys = "DVB-C";

				ss << freq << CSV_SEPARATOR;
				ss << cmod << CSV_SEPARATOR;
				ss << sr << CSV_SEPARATOR;
				ss << cfec << CSV_SEPARATOR;
				ss << inv << CSV_SEPARATOR;
				ss << sys << CSV_SEPARATOR;
			}
			else if (ytype == YTYPE::atsc)
			{
				int freq = tntxp.freq;
				int amod = tntxp.amod;
				string sys = "ATSC";

				ss << freq << CSV_SEPARATOR;
				ss << amod << CSV_SEPARATOR;
				ss << sys << CSV_SEPARATOR;
			}
			ss << CSV_ENDLINE;
		}
		ss << "\n";
	}

	csv = ss.str();
}

void e2db_converter::page_header(html_page& page, string filename, DOC_VIEW view)
{
	string name;

	page.header += "<div class=\"header\">\n";
	if (view == DOC_VIEW::view_index)
	{
		name = "Settings";
	}
	else if (view == DOC_VIEW::view_tunersets)
	{
		name = "Tuner Settings";
	}
	else if (view == DOC_VIEW::view_bouquets)
	{
		if (bouquets.count(filename))
		{
			bouquet bs = bouquets[filename];
			name = bs.name;
		}
	}
	else if (view == DOC_VIEW::view_userbouquets)
	{
		if (userbouquets.count(filename))
		{
			userbouquet ub = userbouquets[filename];
			name = ub.name;
		}
	}
	else
	{
		name = "Service List";
	}
	page.header += "<h1>" + name + "</h1>\n";
	page.header += "<h3>" + filename + "</h3>\n";
	page.header += "</div>\n";
	page.title = filename + " - " + name;
}

void e2db_converter::page_footer(html_page& page, string filename, DOC_VIEW view)
{
	//TODO escape html
	string editor = get_editor_string();
	string timestamp = get_timestamp();

	page.footer += "<div class=\"footer\">\n";
	page.footer += "File: <b>" + filename + "</b><br>\n";
	page.footer += "Editor: <b>" + editor + "</b><br>\n";
	page.footer += "Datetime: <b>" + timestamp + "</b>\n";
	page.footer += "</div>\n";
}

void e2db_converter::page_body_index_list(html_page& page, vector<string> paths)
{
	debug("page_body_index_list()");

	page.body += "<div class=\"toc\">\n";
	page.body += "<h4>Table of Contents</h4>\n";
	page.body += "<table>\n";
	page.body += "<thead>\n";
	page.body += "<tr>\n";
	page.body += "<th>Content</th>\n";
	page.body += "<th>Type</th>\n";
	page.body += "</tr>\n";

	page.body += "<tbody>\n";
	for (auto & path : paths)
	{
		string filename = std::filesystem::path(path).filename().u8string(); //C++17
		string fname = filename;
		string ftype;
		FPORTS fpi = filetype_detect(path);
		switch (fpi)
		{
			case FPORTS::single_tunersets:
				ftype = "tunersets";
			break;
			case FPORTS::single_bouquet:
				ftype = "bouquet";
			break;
			case FPORTS::single_userbouquet:
				ftype = "userbouquet";
			break;
			default:
				ftype = "services";
		}

		page.body += "<tr>";
		page.body += "<td class=\"trid\">" + fname + "</td>";
		page.body += "<td>" + ftype + "</td>";
		page.body += "</tr>\n";
	}
	page.body += "</tbody>\n";

	page.body += "</table>\n";
	page.body += "</div>\n";
}

void e2db_converter::page_body_channel_list(html_page& page, string bname, DOC_VIEW view)
{
	if (index.count(bname))
		debug("page_body_channel_list()", "bname", bname);
	else
		error("page_body_channel_list()", "bname", bname);
	debug("page_body_channel_list()", "view", view);
	
	string cssname = view == DOC_VIEW::view_bouquets ? "userbouquet" : "services";
	
	page.body += "<div class=\"" + cssname + "\">\n";
	page.body += "<table>\n";
	page.body += "<thead>\n";
	page.body += "<tr>\n";
	page.body += "<th>Index</th>\n";
	page.body += "<th>Name</th>\n";
	page.body += "<th>Reference ID</th>\n";
	page.body += "<th>SSID</th>\n";
	page.body += "<th>TSID</th>\n";
	page.body += "<th>Type</th>\n";
	page.body += "<th>CAS</th>\n";
	page.body += "<th>Provider</th>\n";
	page.body += "<th>Freq</th>\n";
	page.body += "<th>Pol</th>\n";
	page.body += "<th>SR</th>\n";
	page.body += "<th>FEC</th>\n";
	page.body += "<th>Pos</th>\n";
	page.body += "<th>Sys</th>\n";
	page.body += "</tr>\n";
	page.body += "</thead>\n";

	page.body += "<tbody>\n";
	for (auto & chdata : index[bname])
	{
		string chid = chdata.second;

		if (db.services.count(chdata.second))
		{
			service ch = db.services[chid];
			transponder tx = db.transponders[ch.txid];

			string idx = to_string(chdata.first);
			string chname = ch.chname;
			string refid;

			// services
			if (view == DOC_VIEW::view_services)
			{
				string crefid = get_reference_id(chid);
				refid = crefid;
			}
			// bouquets
			else
			{
				channel_reference chref;
				if (userbouquets.count(bname))
					chref = userbouquets[bname].channels[chid];
				string crefid = get_reference_id(chref);
				refid = crefid;
			}
			string ssid = to_string(ch.ssid);
			string tsid = to_string(ch.tsid);
			string stype = STYPE_EXT_LABEL.count(ch.stype) ? STYPE_EXT_LABEL.at(ch.stype) : STYPE_EXT_LABEL.at(STYPE::data);
			string scas;
			if (ch.data.count(SDATA::C))
			{
				unordered_set<string> _unique;
				vector<string> cas;

				for (string & w : ch.data[SDATA::C])
				{
					string caidpx = w.substr(0, 2);
					if (SDATA_CAS.count(caidpx) && ! _unique.count(caidpx))
					{
						cas.emplace_back(SDATA_CAS.at(caidpx));
						_unique.insert(caidpx);
					}
				}
				scas.append("<b>$</b>");
				scas.append(" ");
				scas.append("<span class=\"cas\">");
				for (unsigned int i = 0; i < cas.size(); i++)
				{
					scas.append(cas[i]);
					if (i != cas.size() - 1)
						scas.append(", ");
				}
				scas.append("</span>");
			}
			string pname = ch.data.count(SDATA::p) ? ch.data[SDATA::p][0] : "";
			string freq = to_string(tx.freq);
			string pol = tx.pol != -1 ? SAT_POL[tx.pol] : "";
			string sr = to_string(tx.sr);
			string fec = SAT_FEC[tx.fec];
			string ppos = get_transponder_position_text(tx);
			string pos = ppos;
			string psys = get_transponder_system_text(tx);
			string sys = psys;

			page.body += "<tr>";
			page.body += "<td class=\"trid\">" + idx + "</td>";
			page.body += "<td class=\"chname\">" + chname + "</td>";
			page.body += "<td class=\"refid\"><span >" + refid + "</span></td>";
			page.body += "<td>" + ssid + "</td>";
			page.body += "<td>" + tsid + "</td>";
			page.body += "<td>" + stype + "</td>";
			page.body += "<td class=\"scas\">" + scas + "</span></td>";
			page.body += "<td class=\"pname\">" + pname + "</td>";
			page.body += "<td>" + freq + "</td>";
			page.body += "<td>" + pol + "</td>";
			page.body += "<td>" + sr + "</td>";
			page.body += "<td>" + fec + "</td>";
			page.body += "<td>" + pos + "</td>";
			page.body += "<td>" + sys + "</td>";
			page.body += "</tr>\n";
		}
		else
		{
			channel_reference chref;
			if (userbouquets.count(bname))
				chref = userbouquets[bname].channels[chid];

			if (! chref.marker)
				continue;

			string cchid = chid;
			string value = chref.value;
			//TODO marker index and compatibility
			std::transform(cchid.begin(), cchid.end(), cchid.begin(), [](unsigned char c) { return toupper(c); });
			string refid = "1:" + cchid + ":0:0:0:0:0:0";
			string atype = "MARKER";

			page.body += "<tr class=\"marker\">";
			page.body += "<td class=\"trid\"></td>";
			page.body += "<td class=\"name\">" + value + "</td>";
			page.body += "<td class=\"refid\">" + refid + "</td>";
			page.body += "<td></td>";
			page.body += "<td></td>";
			page.body += "<td class=\"atype\">" + atype + "</td>";
			page.body += "<td></td>";
			page.body += "<td></td>";
			page.body += "<td></td>";
			page.body += "<td></td>";
			page.body += "<td></td>";
			page.body += "<td></td>";
			page.body += "<td></td>";
			page.body += "<td></td>";
			page.body += "</tr>\n";
		}
	}
	page.body += "</tbody>\n";

	page.body += "</table>\n";
	page.body += "</div>\n";
}

void e2db_converter::page_body_bouquet_list(html_page& page, string bname)
{
	if (bouquets.count(bname))
		debug("page_body_bouquet_list()", "bname", bname);
	else
		error("page_body_bouquet_list()", "bname", bname);

	bouquet bs = bouquets[bname];

	page.body += "<div class=\"bouquet\">\n";
	page.body += "<table>\n";
	page.body += "<thead>\n";
	page.body += "<tr>\n";
	page.body += "<th>Index</th>\n";
	page.body += "<th>Userbouquet</th>\n";
	page.body += "<th>Name</th>\n";
	page.body += "<th>Type</th>\n";
	page.body += "</tr>\n";

	page.body += "<tbody>\n";
	int i = 1;
	for (auto & bname : bs.userbouquets)
	{
		userbouquet ub = userbouquets[bname];
		string btype;
		if (bs.btype == 1)
			btype = "TV";
		else if (bs.btype == 2)
			btype = "Radio";
		page.body += "<tr>";
		page.body += "<td class=\"trid\">" + to_string(i++) + "</td>";
		page.body += "<td>" + bname + "</td>";
		page.body += "<td>" + ub.name + "</td>";
		page.body += "<td>" + btype + "</td>";
		page.body += "</tr>\n";
	}
	page.body += "</tbody>\n";

	page.body += "</table>\n";
	page.body += "</div>\n";
}

//TODO improve list
void e2db_converter::page_body_tunersets_list(html_page& page, int ytype)
{
	debug("page_body_tunersets_list()", "ytype", ytype);

	page.body += "<div class=\"tunersets\">";

	tunersets tv = tuners[ytype];
	string iname = "tns:";
	switch (ytype)
	{
		case YTYPE::sat:
			iname += 's';
		break;
		case YTYPE::terrestrial:
			iname += 't';
		break;
		case YTYPE::cable:
			iname += 'c';
		break;
		case YTYPE::atsc:
			iname += 'a';
		break;
	}
	
	for (auto & x : index[iname])
	{
		string tnid = x.second;
		tunersets_table tn = tv.tables[tnid];
		string tnname = tn.name;
		string ppos;
		if (ytype == YTYPE::sat)
		{
			ppos = get_transponder_position_text(tn);
		}
		string pos = ppos;

		page.body += "<div class=\"transponder\">\n";
		page.body += "<h4>Transponders</h4>\n";
		page.body += "<h2>" + tnname + "</h2>\n";
		if (ytype == YTYPE::sat)
			page.body += "<p>Position: <b>" + pos + "</b></p>\n";
		page.body += "<table>\n";
		page.body += "<thead>\n";
		page.body += "<tr>\n";
		page.body += "<th>Index</th>\n";
		if (ytype == YTYPE::sat)
		{
			page.body += "<th>Freq</th>\n";
			page.body += "<th>Pol</th>\n";
			page.body += "<th>SR</th>\n";
			page.body += "<th>FEC</th>\n";
			page.body += "<th>Sys</th>\n";
			page.body += "<th>Mod</th>\n";
			page.body += "<th>Inv</th>\n";
			page.body += "<th>Pilot</th>\n";
			page.body += "<th>Rollof</th>\n";
		}
		else if (ytype == YTYPE::terrestrial)
		{
			page.body += "<th>Freq</th>\n";
			page.body += "<th>Const</th>\n";
			page.body += "<th>Band</th>\n";
			page.body += "<th>Sys</th>\n";
			page.body += "<th>Tx Mode</th>\n";
			page.body += "<th>HP FEC</th>\n";
			page.body += "<th>LP FEC</th>\n";
			page.body += "<th>Inv</th>\n";
			page.body += "<th>Guard</th>\n";
			page.body += "<th>Hier</th>\n";
		}
		else if (ytype == YTYPE::cable)
		{
			page.body += "<th>Freq</th>\n";
			page.body += "<th>Mod</th>\n";
			page.body += "<th>SR</th>\n";
			page.body += "<th>FEC</th>\n";
			page.body += "<th>Inv</th>\n";
			page.body += "<th>Sys</th>\n";
		}
		else if (ytype == YTYPE::atsc)
		{
			page.body += "<th>Freq</th>\n";
			page.body += "<th>Mod</th>\n";
			page.body += "<th>Sys</th>\n";
		}
		page.body += "</tr>\n";

		page.body += "<tbody>\n";
		int i = 1;
		for (auto & x : index[tn.tnid])
		{
			string trid = x.second;
			tunersets_transponder tntxp = tn.transponders[trid];

			page.body += "<tr>";
			page.body += "<td class=\"trid\">" + to_string(i++) + "</td>";
			if (ytype == YTYPE::sat)
			{
				string freq = to_string(tntxp.freq);
				string pol = SAT_POL[tntxp.pol];
				string sr = to_string(tntxp.sr);
				string fec = SAT_FEC[tntxp.fec];
				string sys = SAT_SYS[tntxp.sys];
				string mod = SAT_MOD[tntxp.mod];
				string inv = SAT_INV[tntxp.inv];
				string pil = SAT_PIL[tntxp.pil];
				string rol = SAT_ROL[tntxp.rol];

				page.body += "<td>" + freq + "</td>";
				page.body += "<td>" + pol + "</td>";
				page.body += "<td>" + sr + "</td>";
				page.body += "<td>" + fec + "</td>";
				page.body += "<td>" + sys + "</td>";
				page.body += "<td>" + mod + "</td>";
				page.body += "<td>" + inv + "</td>";
				page.body += "<td>" + pil + "</td>";
				page.body += "<td>" + rol + "</td>";
			}
			else if (ytype == YTYPE::terrestrial)
			{
				string freq = to_string(tntxp.freq);
				string tmod = TER_MOD[tntxp.tmod];
				string band = TER_BAND[tntxp.band];
				string sys = "DVB-T";
				string tmx = TER_TRXMODE[tntxp.tmx];
				string hpfec = TER_HPFEC[tntxp.hpfec];
				string lpfec = TER_LPFEC[tntxp.lpfec];
				string inv = TER_INV[tntxp.inv];
				string guard = TER_GUARD[tntxp.guard];
				string hier = TER_HIER[tntxp.hier];

				page.body += "<td>" + freq + "</td>";
				page.body += "<td>" + tmod + "</td>";
				page.body += "<td>" + band + "</td>";
				page.body += "<td>" + sys + "</td>";
				page.body += "<td>" + tmx + "</td>";
				page.body += "<td>" + hpfec + "</td>";
				page.body += "<td>" + lpfec + "</td>";
				page.body += "<td>" + inv + "</td>";
				page.body += "<td>" + guard + "</td>";
				page.body += "<td>" + hier + "</td>";
			}
			else if (ytype == YTYPE::cable)
			{
				string freq = to_string(tntxp.freq);
				string cmod = CAB_MOD[tntxp.cmod];
				string sr = to_string(tntxp.sr);
				string cfec = CAB_IFEC[tntxp.cfec];
				string inv = CAB_INV[tntxp.inv];
				string sys = "DVB-C";

				page.body += "<td>" + freq + "</td>";
				page.body += "<td>" + cmod + "</td>";
				page.body += "<td>" + sr + "</td>";
				page.body += "<td>" + cfec + "</td>";
				page.body += "<td>" + inv + "</td>";
				page.body += "<td>" + sys + "</td>";
			}
			else if (ytype == YTYPE::atsc)
			{
				string freq = to_string(tntxp.freq);
				string amod = to_string(tntxp.amod);
				string sys = "ATSC";

				page.body += "<td>" + freq + "</td>";
				page.body += "<td>" + amod + "</td>";
				page.body += "<td>" + sys + "</td>";
			}
			page.body += "</tr>\n";
		}
		page.body += "</tbody>\n";

		page.body += "</table>\n";
		page.body += "</div>\n";
	}

	page.body += "</div>\n";
}

string e2db_converter::doc_html_head(html_page page)
{
	return "<!DOCTYPE html>\n\
<html lang=\"en\">\n\
<head>\n\
<meta charset=\"utf-8\">\n\
<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">\n\
<title>" + page.title + "</title>\n\
<style>\n\
body { margin: .5rem 1rem; font: normal 14px/1.5 sans-serif }\n\
h4 { text-transform: uppercase }\n\
div { margin: 3em 0 5em }\n\
div.footer { margin: 3em 1em }\n\
table { margin: 2em 0; border-collapse: collapse }\n\
td, th { padding: .4em .5em }\n\
table, td, th { border: 1px solid }\n\
th, td.chname, td.name, td.pname, td.refid { white-space: nowrap }\n\
tr.marker td, td.refid { font-weight: bold }\n\
tr.marker td, span.cas { font-size: small }\n\
td.trid { padding-inline-start: .8em; padding-inline-end: 1.4em }\n\
td.refid { font-size: smaller }\n\
span.cas { margin-inline-start: .3em }\n\
</style>\n\
</head>\n";
}

string e2db_converter::doc_html_foot(html_page page)
{
	return "\n</html>";
}

void e2db_converter::csv_document(e2db_file& file, string csv)
{
	debug("csv_document()");

	file = csv;
}

void e2db_converter::html_document(e2db_file& file, html_page page)
{
	debug("html_document()");

	string html;
	html.append(doc_html_head(page));
	html.append("<body>\n");
	html.append(page.header);
	html.append(page.body);
	html.append(page.footer);
	html.append("</body>\n");
	html.append(doc_html_foot(page));
	html.append("\n");
	file = html;
}

}
