/*!
 * e2-sat-editor/src/e2db/e2db_converter.cpp
 * 
 * @link https://github.com/ctlcltd/e2-sat-editor
 * @copyright e2 SAT Editor Team
 * @author Leonardo Laureti
 * @version 0.2
 * @license MIT License
 * @license GNU GPLv3 License
 */

#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <cstring>
#include <clocale>
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

e2db_converter::e2db_converter()
{
	std::setlocale(LC_NUMERIC, "C");
}

e2db_converter::e2db_converter(e2se::logger::session* log)
{
	std::setlocale(LC_NUMERIC, "C");

	this->log = new e2se::logger(log, "e2db");
	debug("e2db_converter()");
}

void e2db_converter::import_csv_file(FCONVS fci, fcopts opts, vector<string> paths)
{
	debug("import_csv_file()", "file path", "multiple");
	debug("import_csv_file()", "file input", fci);

	bool merge = this->get_input().size() != 0 ? true : false;
	auto* dst = merge ? newptr() : this;

	for (string & path : paths)
	{
		import_csv_file(fci, opts, dst, path);
	}
	if (merge)
	{
		this->merge(dst);
		delete dst;
	}
}

void e2db_converter::import_csv_file(FCONVS fci, fcopts opts, string path)
{
	debug("import_csv_file()", "file path", "singular");
	debug("import_csv_file()", "file input", fci);

	bool merge = this->get_input().size() != 0 ? true : false;
	auto* dst = merge ? newptr() : this;

	import_csv_file(fci, opts, dst, path);

	if (merge)
	{
		this->merge(dst);
		delete dst;
	}
}

void e2db_converter::import_csv_file(FCONVS fci, fcopts opts, e2db_abstract* dst, string path)
{
	debug("import_csv_file()", "file path", "singular");
	debug("import_csv_file()", "file input", fci);

	std::clock_t start = std::clock();

	if (! std::filesystem::exists(path)) //C++17
	{
		return error("import_csv_file()", "File Error", "File \"" + path + "\" not exists.");
	}
	if (! std::filesystem::is_regular_file(path)) //C++17
	{
		return error("import_csv_file()", "File Error", "File \"" + path + "\" is not a valid file.");
	}
	if ((std::filesystem::status(path).permissions() & std::filesystem::perms::group_read)  == std::filesystem::perms::none) //C++17
	{
		return error("import_csv_file()", "File Error", "File \"" + path + "\" is not readable.");
	}

	ifstream ifile (path);

	switch (fci)
	{
		case FCONVS::convert_services:
			pull_csv_services(ifile, dst);
		break;
		case FCONVS::convert_bouquets:
			pull_csv_bouquets(ifile, dst);
		break;
		case FCONVS::convert_userbouquets:
			pull_csv_userbouquets(ifile, dst);
		break;
		case FCONVS::convert_tunersets:
			pull_csv_tunersets(ifile, dst);
		break;
		default:
			ifile.close();
		return error("import_csv_file()", "Error", "Unknown import option.");
	}

	ifile.close();

	std::clock_t end = std::clock();

	info("import_csv_file()", "elapsed time", to_string(int (end - start)) + " ms.");
}

void e2db_converter::export_csv_file(FCONVS fco, fcopts opts, string path)
{
	debug("export_csv_file()", "file path", "singular");
	debug("export_csv_file()", "file output", fco);
	
	std::clock_t start = std::clock();

	std::filesystem::path fpath = std::filesystem::path(path); //C++17
	string base = fpath.parent_path().u8string(); //C++17
	string filename = fpath.filename().u8string(); //C++17
	opts.filename = filename;

	vector<e2db_file> files;

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
		return error("export_csv_file()", "Error", "Unknown export option.");
	}

	bool once = !! files.size();
	for (auto & file : files)
	{
		string fpath;
		if (once)
			fpath = base + '/' + file.filename;
		else
			fpath = base + '/' + opts.filename;

		if (! OVERWRITE_FILE && std::filesystem::exists(fpath)) //C++17
		{
			return error("export_csv_file()", "File Error", "File \"" + fpath + "\" already exists.");
		}
		if ((std::filesystem::status(fpath).permissions() & std::filesystem::perms::group_write)  == std::filesystem::perms::none) //C++17
		{
			return error("export_csv_file()", "File Error", "File \"" + fpath + "\" is not writable.");
		}

		ofstream out (fpath);
		out << file.data;
		out.close();
	}

	std::clock_t end = std::clock();

	info("export_csv_file()", "elapsed time", to_string(int (end - start)) + " ms.");
}

void e2db_converter::export_html_file(FCONVS fco, fcopts opts, string path)
{
	debug("export_html_file()", "file path", "singular");
	debug("export_html_file()", "file output", fco);

	std::clock_t start = std::clock();

	std::filesystem::path fpath = std::filesystem::path(path); //C++17
	string base = fpath.parent_path().u8string(); //C++17
	string filename = fpath.filename().u8string(); //C++17
	opts.filename = filename;

	vector<e2db_file> files;

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
		return error("export_html_file()", "Error", "Unknown export option.");
	}

	bool once = !! files.size();
	for (e2db_file & file : files)
	{
		string fpath;
		if (once)
			fpath = base + '/' + file.filename;
		else
			fpath = base + '/' + opts.filename;

		if (! OVERWRITE_FILE && std::filesystem::exists(fpath)) //C++17
		{
			return error("export_html_file()", "File Error", "File \"" + fpath + "\" already exists.");
		}
		if ((std::filesystem::status(fpath).permissions() & std::filesystem::perms::group_write)  == std::filesystem::perms::none) //C++17
		{
			return error("export_html_file()", "File Error", "File \"" + fpath + "\" is not writable.");
		}

		ofstream out (fpath);
		out << file.data;
		out.close();
	}

	std::clock_t end = std::clock();

	info("export_html_file()", "elapsed time", to_string(int (end - start)) + " ms.");
}

void e2db_converter::pull_csv_services(istream& ifile, e2db_abstract* dst)
{
	debug("pull_csv_services()");
	
	vector<vector<string>> sxv;
	parse_csv(ifile, sxv);

	if (CONVERTER_EXTENDED_FIELDS)
		convert_csv_channel_list_extended(sxv, dst, DOC_VIEW::view_services);
	else
		convert_csv_channel_list(sxv, dst, DOC_VIEW::view_services);
}

void e2db_converter::pull_csv_bouquets(istream& ifile, e2db_abstract* dst)
{
	debug("pull_csv_bouquets()");

	vector<vector<string>> sxv;
	parse_csv(ifile, sxv);

	convert_csv_bouquet_list(sxv, dst);
}

void e2db_converter::pull_csv_userbouquets(istream& ifile, e2db_abstract* dst)
{
	debug("pull_csv_userbouquets()");

	vector<vector<string>> sxv;
	parse_csv(ifile, sxv);

	if (CONVERTER_EXTENDED_FIELDS)
		convert_csv_channel_list_extended(sxv, dst, DOC_VIEW::view_userbouquets);
	else
		convert_csv_channel_list(sxv, dst, DOC_VIEW::view_userbouquets);
}

void e2db_converter::pull_csv_tunersets(istream& ifile, e2db_abstract* dst)
{
	debug("pull_csv_tunersets()");

	vector<vector<string>> sxv;
	parse_csv(ifile, sxv);

	convert_csv_tunersets_list(sxv, dst);
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
	
	string filename = "services";
	string iname;
	switch (stype)
	{
		// Data
		case STYPE::data:
			iname = "chs:0";
			filename += "-data";
		break;
		// TV
		case STYPE::tv:
			iname = "chs:1";
			filename += "-tv";
		break;
		// Radio
		case STYPE::radio:
			iname = "chs:2";
			filename += "-radio";
		break;
		// All Services
		default:
			iname = "chs";
	}
	filename += ".csv";

	string csv;
	if (CONVERTER_EXTENDED_FIELDS)
		csv_channel_list_extended(csv, iname, DOC_VIEW::view_services);
	else
		csv_channel_list(csv, iname, DOC_VIEW::view_services);

	e2db_file file;
	file.filename = filename;
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

	string filename = bname + ".csv";

	string csv;
	csv_bouquet_list(csv, bname);

	e2db_file file;
	file.filename = filename;
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

	string filename = bname + ".csv";

	string csv;
	if (CONVERTER_EXTENDED_FIELDS)
		csv_channel_list_extended(csv, bname, DOC_VIEW::view_userbouquets);
	else
		csv_channel_list(csv, bname, DOC_VIEW::view_userbouquets);

	e2db_file file;
	file.filename = filename;
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
	debug("push_csv_tunersets()", "ytype", ytype);

	string filename;
	switch (ytype)
	{
		case YTYPE::satellite:
			filename = "satellites";
		break;
		case YTYPE::terrestrial:
			filename = "terrestrial";
		break;
		case YTYPE::cable:
			filename = "cables";
		break;
		case YTYPE::atsc:
			filename = "atsc";
		break;
	}
	filename += ".csv";

	string csv;
	csv_tunersets_list(csv, ytype);

	e2db_file file;
	file.filename = filename;
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

	string filename = "index";
	string fname = std::filesystem::path(get_filepath()).filename().u8string(); //C++17
	if (filename.empty())
	{
		fname = "Untitled";
	}
	filename += ".html";

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
	file.filename = filename;
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

	string filename = "services";
	string fname = std::filesystem::path(get_services_filename()).filename().u8string(); //C++17
	string iname;
	string xname;
	string headname = fname;
	string footname = fname;
	switch (stype)
	{
		// Data
		case STYPE::data:
			iname = "chs:0";
			xname = STYPE_EXT_LABEL.at(STYPE::data);
			filename += "-data";
		break;
		// TV
		case STYPE::tv:
			iname = "chs:1";
			xname = STYPE_EXT_LABEL.at(STYPE::tv);
			filename += "-tv";
		break;
		// Radio
		case STYPE::radio:
			iname = "chs:2";
			xname = STYPE_EXT_LABEL.at(STYPE::radio);
			filename += "-radio";
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
	filename += ".html";

	html_page page;
	page_header(page, headname, DOC_VIEW::view_services);
	page_footer(page, footname, DOC_VIEW::view_services);

	page_body_channel_list(page, iname, DOC_VIEW::view_services);

	e2db_file file;
	file.filename = filename;
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

	string filename = bname + ".html";

	html_page page;
	page_header(page, bname, DOC_VIEW::view_bouquets);
	page_footer(page, bname, DOC_VIEW::view_bouquets);

	page_body_bouquet_list(page, bname);

	e2db_file file;
	file.filename = filename;
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

	string filename = bname + ".html";

	html_page page;
	page_header(page, bname, DOC_VIEW::view_userbouquets);
	page_footer(page, bname, DOC_VIEW::view_userbouquets);

	page_body_channel_list(page, bname, DOC_VIEW::view_userbouquets);

	e2db_file file;
	file.filename = filename;
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
	debug("push_html_tunersets()", "ytype", ytype);

	string fname;
	switch (ytype)
	{
		case YTYPE::satellite:
			fname = "satellites";
		break;
		case YTYPE::terrestrial:
			fname = "terrestrial";
		break;
		case YTYPE::cable:
			fname = "cables";
		break;
		case YTYPE::atsc:
			fname = "atsc";
		break;
	}
	string filename = fname + ".html";
	fname += ".xml";

	html_page page;
	page_header(page, fname, DOC_VIEW::view_tunersets);
	page_footer(page, fname, DOC_VIEW::view_tunersets);

	page_body_tunersets_list(page, ytype);

	e2db_file file;
	file.filename = filename;
	html_document(file, page);
	files.emplace_back(file);
}

void e2db_converter::parse_csv(istream& ifile, vector<vector<string>>& sxv)
{
	debug("parse_csv()");

	const char dlm = CSV_DELIMITER; //TODO win32 transform
	const char sep = CSV_SEPARATOR;
	const char esp = CSV_ESCAPE;

	string line;

	while (std::getline(ifile, line, dlm))
	{
		vector<string> values;
		stringstream ss (line);
		string str;
		bool yey = false;

		while (std::getline(ss, str, sep))
		{
			string val;
			size_t pos = str.find(esp);
			size_t len;

			if (pos != string::npos)
			{
				pos += 1;
				val = str.substr(pos);
				len = val.rfind(esp);

				if (len != string::npos)
				{
					val = val.substr(0, len);
				}
				else if (yey)
				{
					yey = false;
					continue;
				}
				else
				{
					pos = line.find(val);
					val = line.substr(pos);
					len = val.find(esp);

					if (len != string::npos)
					{
						yey = true;
						val = val.substr(0, len);
					}
				}
			}
			else if (! yey)
			{
				val = str;
			}
			else
			{
				continue;
			}
			values.emplace_back(val);
		}
		sxv.emplace_back(values);
	}
}

void e2db_converter::convert_csv_channel_list(vector<vector<string>> sxv, e2db_abstract* dst, DOC_VIEW view)
{
	debug("convert_csv_channel_list()");

	unordered_map<string, userbouquet> userbouquets;

	for (size_t x = 0; x < sxv.size(); x++)
	{
		// csv header
		if (x == 0 && sxv[0][0] == "Index")
			continue;

		userbouquet ub;
		service ch;
		transponder tx;
		channel_reference chref;
		service_reference ref;
		fec fec;

		for (size_t i = 0; i < sxv[x].size(); i++)
		{
			// debug("convert_csv_channel_list()", to_string(x), sxv[x][i]);

			string& val = sxv[x][i];

			// idx
			if (i == 0)
				ch.index = std::atoi(val.data());
			// chname
			else if (i == 1)
				ch.chname = val;
			// refid
			else if (i == 2)
				value_channel_reference(val, chref, ref);
			// ssid
			else if (i == 3)
				ch.ssid = std::atoi(val.data());
			// tsid
			else if (i == 4)
				ch.tsid = tx.tsid = std::atoi(val.data());
			// onid
			else if (i == 5)
				ch.onid = tx.onid = std::atoi(val.data());
			// dvbns
			else if (i == 6)
				ch.dvbns = std::atoi(val.data());
			// stype
			else if (i == 7)
				ch.stype = value_service_type(val);
			// snum
			else if (i == 8)
				ch.snum = std::atoi(val.data());
			// cas
			else if (i == 9)
			{
				if (! val.empty())
					ch.data[SDATA::C];
			}
			// caid
			else if (i == 10)
				ch.data[SDATA::C] = value_channel_caid(val);
			// provider
			else if (i == 11)
				ch.data[SDATA::p] = value_channel_provider(val);
			// ytype
			// sys
			else if (i == 12)
			{
				tx.ytype = value_transponder_type(val);
				tx.sys = value_transponder_system(val);
			}
			// pos
			else if (i == 13)
				tx.pos = value_transponder_position(val);
			// tuner name
			else if (i == 14)
				continue;
			// freq
			else if (i == 15)
				tx.freq = std::atoi(val.data());
			// pol
			else if (i == 16)
				tx.pol = value_transponder_polarization(val);
			// sr
			else if (i == 17)
				tx.sr = std::atoi(val.data());
			// fec condensed
			else if (i == 18)
				value_transponder_fec(val, tx.ytype, fec);
			// userbouquet bname
			else if (i == 19)
				ub.bname = val;
			// userbouquet name
			else if (i == 20)
				ub.name = val;
		}

		if (chref.marker)
		{
			// chref.value
			chref.value = ch.chname;

			char chid[25];
			// %4d:%2x:%d
			std::sprintf(chid, "%d:%x:%d", chref.atype, chref.anum, 0);
			chref.chid = chid;
		}
		else
		{
			// x order has priority over ch.index
			if (ch.index != int (x))
				ch.index = (x + 1);
			// ssid has priority over ref.ssid
			if (! ch.ssid)
				ch.ssid = ref.ssid;
			// tsid has priority over ref.tsid
			if (! ch.tsid)
				ch.tsid = tx.tsid = ref.tsid;
			// onid has priority over ref.onid
			if (! ch.onid)
				ch.onid = tx.onid = ref.onid;
			// dvbns has priority over ref.dvbns
			if (! ch.onid)
				ch.onid = tx.onid = ref.onid;
			// stype has priority over chref.atype
			if (! ch.stype)
				ch.stype = chref.atype;
			// snum has priority over chref.anum
			if (! ch.snum)
				ch.snum = chref.anum;
			if (ch.data[SDATA::C].empty())
				ch.data.erase(SDATA::C);

			// fec condensed
			if (tx.ytype == YTYPE::satellite)
			{
				tx.fec = fec.inner_fec;
			}
			else if (tx.ytype == YTYPE::terrestrial)
			{
				tx.hpfec = fec.hp_fec;
				tx.lpfec = fec.lp_fec;
			}
			else if (tx.ytype == YTYPE::cable)
			{
				tx.fec = fec.inner_fec;
			}

			if (ch.data[SDATA::C].empty())
				ch.data.erase(SDATA::f);
			if (ch.data[SDATA::c].empty())
				ch.data.erase(SDATA::c);

			char txid[25];
			// %4x:%8x
			std::sprintf(txid, "%x:%x", tx.tsid, tx.dvbns);
			tx.txid = txid;

			char chid[25];
			// %4x:%4x:%8x
			std::sprintf(chid, "%x:%x:%x", ch.ssid, ch.tsid, ch.dvbns);
			ch.chid = chref.chid = chid;
			ch.txid = txid;

			if (! dst->db.transponders.count(tx.txid))
			{
				// tx idx
				tx.index = (dst->index["txs"].size() + 1);

				dst->db.transponders.emplace(tx.txid, tx);
				dst->index["txs"].emplace_back(pair (tx.index, tx.txid)); //C++17
			}
			if (! dst->db.services.count(ch.chid))
			{
				dst->db.services.emplace(ch.chid, ch);
				string iname = "chs:" + (STYPE_EXT_TYPE.count(ch.stype) ? to_string(STYPE_EXT_TYPE.at(ch.stype)) : "0");
				dst->index["chs"].emplace_back(pair (ch.index, ch.chid)); //C++17
				dst->index[iname].emplace_back(pair (ch.index, ch.chid)); //C++17
			}
		}

		if (view == DOC_VIEW::view_userbouquets)
		{
			if (! userbouquets.count(ub.bname))
			{
				userbouquets.emplace(ub.bname, ub);
			}
			if (! userbouquets[ub.bname].channels.count(chref.chid))
			{
				// chref idx
				chref.index = (userbouquets[ub.bname].channels.size() + 1);

				userbouquets[ub.bname].channels.emplace(chref.chid, chref);

				dst->index[ub.bname].emplace_back(pair (chref.index, chref.chid)); //C++17
				if (chref.marker)
					dst->index["mks"].emplace_back(pair (ub.index, chref.chid)); //C++17
			}
		}
	}

	if (view == DOC_VIEW::view_userbouquets)
	{
		// btype autodetect
		int btype;
		if (dst->index["chs:1"].size() > dst->index["chs:2"].size())
			btype = STYPE::tv;
		else
			btype = STYPE::radio;

		for (auto & x : userbouquets)
		{
			userbouquet& ub = x.second;

			if (dst->index.count("bss"))
			{
				bouquet bs;
				for (auto & x : dst->bouquets)
				{
					if (x.second.btype == btype)
					{
						bs = x.second;
						break;
					}
				}
				// userbouquet pname
				ub.pname = bs.bname;
			}
			else
			{
				bouquet bs;
				if (btype == STYPE::tv)
				{
					bs.bname = "bouquets.tv";
					bs.name = "User - bouquet (TV)";
					bs.nname = STYPE_EXT_LABEL.at(STYPE::tv);
				}
				else if (btype == STYPE::radio)
				{
					bs.bname = "bouquets.radio";
					bs.name = "User - bouquet (Radio)";
					bs.nname = STYPE_EXT_LABEL.at(STYPE::radio);
				}
				// bouquet idx
				bs.index = dst->index["bss"].size();
				// userbouquet pname
				ub.pname = bs.bname;

				dst->bouquets.emplace(bs.bname, bs);
				dst->index["bss"].emplace_back(pair (bs.index, bs.bname)); //C++17
			}
			if (! dst->userbouquets.count(ub.bname))
			{
				dst->userbouquets.emplace(ub.bname, ub);
				dst->index["ubs"].emplace_back(pair (ub.index, ub.bname)); //C++17
				dst->bouquets[ub.pname].userbouquets.emplace_back(ub.bname);

				for (auto & x : dst->index[ub.bname])
				{
					channel_reference& chref = ub.channels[x.second];

					if (! chref.marker)
						dst->index[ub.pname].emplace_back(pair ((dst->index[ub.pname].size() + 1), chref.chid)); //C++17
				}
			}
		}
		userbouquets.clear();
	}
}

void e2db_converter::convert_csv_channel_list_extended(vector<vector<string>> sxv, e2db_abstract* dst, DOC_VIEW view)
{
	debug("convert_csv_channel_list_extended()");

	unordered_map<string, userbouquet> userbouquets;

	for (size_t x = 0; x < sxv.size(); x++)
	{
		// csv header
		if (x == 0 && sxv[0][0] == "Index")
			continue;

		userbouquet ub;
		service ch;
		transponder tx;
		channel_reference chref;
		service_reference ref;

		for (size_t i = 0; i < sxv[x].size(); i++)
		{
			// debug("convert_csv_channel_list_extended()", to_string(x), sxv[x][i]);

			string& val = sxv[x][i];

			// idx
			if (i == 0)
				ch.index = std::atoi(val.data());
			// chname
			else if (i == 1)
				ch.chname = val;
			// refid
			else if (i == 2)
				value_channel_reference(val, chref, ref);
			// ssid
			else if (i == 3)
				ch.ssid = std::atoi(val.data());
			// tsid
			else if (i == 4)
				ch.tsid = tx.tsid = std::atoi(val.data());
			// onid
			else if (i == 5)
				ch.onid = tx.onid = std::atoi(val.data());
			// dvbns
			else if (i == 6)
				ch.dvbns = std::atoi(val.data());
			// stype
			else if (i == 7)
				ch.stype = value_service_type(val);
			// snum
			else if (i == 8)
				ch.snum = std::atoi(val.data());
			// cas
			else if (i == 9)
			{
				if (! val.empty())
					ch.data[SDATA::C];
			}
			// caid
			else if (i == 10)
			{
				ch.data[SDATA::C] = value_channel_caid(val);
			}
			// provider
			else if (i == 11)
				ch.data[SDATA::p] = value_channel_provider(val);
			// srcid
			else if (i == 12)
				ch.srcid = std::atoi(val.data());
			// ytype
			// sys
			else if (i == 13)
			{
				tx.ytype = value_transponder_type(val);
				tx.sys = value_transponder_system(val);
			}
			// pos
			else if (i == 14)
				tx.pos = value_transponder_position(val);
			// tuner name
			else if (i == 15)
				continue;
			// freq
			else if (i == 16)
				tx.freq = std::atoi(val.data());
			// pol
			else if (i == 17)
				tx.pol = value_transponder_polarization(val);
			// sr
			else if (i == 18)
				tx.sr = std::atoi(val.data());
			// ifec
			else if (i == 19)
				tx.fec = value_transponder_fec(val, tx.ytype);
			// hpfec
			else if (i == 20)
				tx.hpfec = value_transponder_fec(val, tx.ytype);
			// lpfec
			else if (i == 21)
				tx.lpfec = value_transponder_fec(val, tx.ytype);
			// mod
			else if (i == 22)
				tx.mod = value_transponder_modulation(val, tx.ytype);
			// band
			else if (i == 23)
				tx.band = value_transponder_bandwidth(val);
			// tmx
			else if (i == 24)
				tx.tmx = value_transponder_tmx_mode(val);
			// inv
			else if (i == 25)
				tx.inv = value_transponder_inversion(val, tx.ytype);
			// rol
			else if (i == 26)
				tx.rol = value_transponder_rollof(val);
			// pil
			else if (i == 27)
				tx.pil = value_transponder_pilot(val);
			// guard
			else if (i == 28)
				tx.guard = value_transponder_guard(val);
			// hier
			else if (i == 29)
				tx.hier = value_transponder_hier(val);
			// flgs
			else if (i == 30)
				tx.flgs = std::atoi(val.data());
			// txp index
			else if (i == 31)
				tx.index = std::atoi(val.data());
			// txp flgs
			else if (i == 32)
				tx.oflgs = val;
			// cached
			else if (i == 33)
				ch.data[SDATA::c] = value_channel_cached(val);
			// userbouquet bname
			else if (i == 34)
				ub.bname = val;
			// userbouquet name
			else if (i == 35)
				ub.name = val;
		}

		if (chref.marker)
		{
			// chref.value
			chref.value = ch.chname;

			char chid[25];
			// %4d:%2x:%d
			std::sprintf(chid, "%d:%x:%d", chref.atype, chref.anum, 0);
			chref.chid = chid;
		}
		else
		{
			// x order has priority over ch.index
			if (ch.index != int (x))
				ch.index = (x + 1);
			// ssid has priority over ref.ssid
			if (! ch.ssid)
				ch.ssid = ref.ssid;
			// tsid has priority over ref.tsid
			if (! ch.tsid)
				ch.tsid = tx.tsid = ref.tsid;
			// onid has priority over ref.onid
			if (! ch.onid)
				ch.onid = tx.onid = ref.onid;
			// dvbns has priority over ref.dvbns
			if (! ch.onid)
				ch.onid = tx.onid = ref.onid;
			// stype has priority over chref.atype
			if (! ch.stype)
				ch.stype = chref.atype;
			// snum has priority over chref.anum
			if (! ch.snum)
				ch.snum = chref.anum;

			if (ch.data[SDATA::C].empty())
				ch.data.erase(SDATA::f);
			if (ch.data[SDATA::c].empty())
				ch.data.erase(SDATA::c);

			char txid[25];
			// %4x:%8x
			std::sprintf(txid, "%x:%x", tx.tsid, tx.dvbns);
			tx.txid = txid;

			char chid[25];
			// %4x:%4x:%8x
			std::sprintf(chid, "%x:%x:%x", ch.ssid, ch.tsid, ch.dvbns);
			ch.chid = chref.chid = chid;
			ch.txid = txid;

			if (! dst->db.transponders.count(tx.txid))
			{
				// tx idx
				tx.index = (dst->index["txs"].size() + 1);

				dst->db.transponders.emplace(tx.txid, tx);
				dst->index["txs"].emplace_back(pair (tx.index, tx.txid)); //C++17
			}
			if (! dst->db.services.count(ch.chid))
			{
				dst->db.services.emplace(ch.chid, ch);
				string iname = "chs:" + (STYPE_EXT_TYPE.count(ch.stype) ? to_string(STYPE_EXT_TYPE.at(ch.stype)) : "0");
				dst->index["chs"].emplace_back(pair (ch.index, ch.chid)); //C++17
				dst->index[iname].emplace_back(pair (ch.index, ch.chid)); //C++17
			}
		}
	
		if (view == DOC_VIEW::view_userbouquets)
		{
			if (! userbouquets.count(ub.bname))
			{
				userbouquets.emplace(ub.bname, ub);
			}
			if (! userbouquets[ub.bname].channels.count(chref.chid))
			{
				// chref idx
				chref.index = (userbouquets[ub.bname].channels.size() + 1);

				userbouquets[ub.bname].channels.emplace(chref.chid, chref);
				dst->index[ub.bname].emplace_back(pair (chref.index, chref.chid)); //C++17
				if (chref.marker)
					dst->index["mks"].emplace_back(pair (ub.index, chref.chid)); //C++17
			}
		}
	}

	if (view == DOC_VIEW::view_userbouquets)
	{
		// btype autodetect
		int btype;
		if (dst->index["chs:1"].size() > dst->index["chs:2"].size())
			btype = STYPE::tv;
		else
			btype = STYPE::radio;

		for (auto & x : userbouquets)
		{
			userbouquet& ub = x.second;

			if (! dst->index.count("bss"))
			{
				bouquet bs;
				for (auto & x : dst->bouquets)
				{
					if (x.second.btype == btype)
					{
						bs = x.second;
						break;
					}
				}
				// userbouquet pname
				ub.pname = bs.bname;
			}
			else
			{
				bouquet bs;
				if (btype == STYPE::tv)
				{
					bs.bname = "bouquets.tv";
					bs.name = "User - bouquet (TV)";
					bs.nname = STYPE_EXT_LABEL.at(STYPE::tv);
				}
				else if (btype == STYPE::radio)
				{
					bs.bname = "bouquets.radio";
					bs.name = "User - bouquet (Radio)";
					bs.nname = STYPE_EXT_LABEL.at(STYPE::radio);
				}
				// bouquet idx
				bs.index = dst->index["bss"].size();
				// userbouquet pname
				ub.pname = bs.bname;

				dst->bouquets.emplace(bs.bname, bs);
				dst->index["bss"].emplace_back(pair (bs.index, bs.bname)); //C++17
			}
			if (! dst->userbouquets.count(ub.bname))
			{
				dst->userbouquets.emplace(ub.bname, ub);
				dst->index["ubs"].emplace_back(pair (ub.index, ub.bname)); //C++17
				dst->bouquets[ub.pname].userbouquets.emplace_back(ub.bname);

				for (auto & x : dst->index[ub.bname])
				{
					channel_reference& chref = ub.channels[x.second];

					if (! chref.marker)
						dst->index[ub.pname].emplace_back(pair ((dst->index[ub.pname].size() + 1), chref.chid)); //C++17
				}
			}
		}
		userbouquets.clear();
	}
}

void e2db_converter::convert_csv_bouquet_list(vector<vector<string>> sxv, e2db_abstract* dst)
{
	debug("convert_csv_bouquet_list()");

	bouquet bs;
	vector<userbouquet> userbouquets;

	for (size_t x = 0; x < sxv.size(); x++)
	{
		// csv header
		if (x == 0 && sxv[0][0] == "Index")
			continue;

		userbouquet ub;

		for (size_t i = 0; i < sxv[x].size(); i++)
		{
			// debug("convert_csv_bouquet_list()", to_string(x), sxv[x][i]);

			string& val = sxv[x][i];

			// userbouquet idx
			if (i == 0)
				ub.index = std::atoi(val.data());
			// bouquet name
			else if (i == 1)
				bs.name = val;
			// userbouquet bname
			else if (i == 2)
				ub.bname = val;
			// userbouquet name
			else if (i == 3)
				ub.name = val;
			// btype
			else if (i == 4)
				bs.btype = value_bouquet_type(val);
		}

		// x order has priority over ub.index
		if (ub.index != int (x))
			ub.index = (x + 1);
		// bouquet bname
		// userbouquet pname
		// bouquet nname
		if (bs.btype == STYPE::tv)
		{
			bs.bname = ub.pname = "bouquets.tv";
			bs.nname = STYPE_EXT_LABEL.at(STYPE::tv);
		}
		else if (bs.btype == 2)
		{
			bs.bname = ub.pname = "bouquets.radio";
			bs.nname = STYPE_EXT_LABEL.at(STYPE::radio);
		}

		userbouquets.emplace_back(ub);
	}

	if (! dst->bouquets.count(bs.bname))
	{
		// bouquet idx
		bs.index = dst->index["bss"].size();

		dst->bouquets.emplace(bs.bname, bs);
		dst->index["bss"].emplace_back(pair (bs.index, bs.bname)); //C++17
	}
	for (userbouquet & ub : userbouquets)
	{
		if (! dst->userbouquets.count(ub.bname))
		{
			dst->userbouquets.emplace(ub.bname, ub);
			dst->index["ubs"].emplace_back(pair (ub.index, ub.bname)); //C++17
			dst->bouquets[ub.pname].userbouquets.emplace_back(ub.bname);
		}
	}
}

void e2db_converter::convert_csv_tunersets_list(vector<vector<string>> sxv, e2db_abstract* dst)
{
	debug("convert_csv_tunersets_list()");

	YTYPE ytype = YTYPE::satellite;

	tunersets tv;
	tv.charset = "utf-8";

	for (size_t x = 0; x < sxv.size(); x++)
	{
		// ytype autodetect
		if (x == 0)
		{
			switch (sxv[x].size())
			{
				case 12: ytype = YTYPE::satellite; break;
				case 13: ytype = YTYPE::terrestrial; break;
				case 9: ytype = YTYPE::cable; break;
				case 6: ytype = YTYPE::atsc; break;
			}
			tv.ytype = ytype;
		}
		// csv header
		if (x == 0 && sxv[0][0] == "Index")
		{
			continue;
		}

		tunersets_table tn;
		tunersets_transponder tntxp;

		for (size_t i = 0; i < sxv[x].size(); i++)
		{
			// debug("convert_csv_tunersets_list()", to_string(x), sxv[x][i]);

			string& val = sxv[x][i];

			// transponder idx
			if (i == 0)
				tntxp.index = std::atoi(val.data());
			// table name
			else if (i == 1)
				tn.name = val;
			// table pos
			else if (i == 2)
				tn.pos = value_transponder_position(val);

			if (ytype == YTYPE::satellite)
			{
				// freq
				if (i == 3)
					tntxp.freq = std::atoi(val.data());
				// pol
				else if (i == 4)
					tntxp.pol = value_transponder_polarization(val);
				// sr
				else if (i == 5)
					tntxp.sr = std::atoi(val.data());
				// fec
				else if (i == 6)
					tntxp.fec = value_transponder_fec(val, YTYPE::satellite);
				// sys
				else if (i == 7)
					tntxp.sys = value_transponder_system(val);
				// mod
				else if (i == 8)
					tntxp.mod = value_transponder_modulation(val, YTYPE::satellite);
				// inv
				else if (i == 9)
					tntxp.inv = value_transponder_inversion(val, YTYPE::satellite);
				// rol
				else if (i == 10)
					tntxp.rol = value_transponder_rollof(val);
				// pil
				else if (i == 11)
					tntxp.pil = value_transponder_pilot(val);
			}
			else if (ytype == YTYPE::terrestrial)
			{
				// freq
				if (i == 3)
					tntxp.freq = std::atoi(val.data());
				// tmod
				else if (i == 4)
					tntxp.tmod = value_transponder_modulation(val, YTYPE::terrestrial);
				// band
				else if (i == 5)
					tntxp.band = std::atoi(val.data());
				// sys
				else if (i == 6)
					tntxp.sys = value_transponder_system(val);
				// tmx
				else if (i == 7)
					tntxp.tmx = value_transponder_tmx_mode(val);
				// hpfec
				else if (i == 8)
					tntxp.hpfec = value_transponder_fec(val, YTYPE::terrestrial);
				// lpfe
				else if (i == 9)
					tntxp.lpfec = value_transponder_fec(val, YTYPE::terrestrial);
				// inv
				else if (i == 10)
					tntxp.inv = value_transponder_inversion(val, YTYPE::terrestrial);
				// guard
				else if (i == 11)
					tntxp.guard = value_transponder_guard(val);
				// hier
				else if (i == 11)
					tntxp.hier = value_transponder_hier(val);
			}
			else if (ytype == YTYPE::cable)
			{
				// freq
				if (i == 3)
					tntxp.freq = std::atoi(val.data());
				// mod
				else if (i == 4)
					tntxp.cmod = value_transponder_modulation(val, YTYPE::cable);
				// sr
				else if (i == 5)
					tntxp.sr = std::atoi(val.data());
				// fec
				else if (i == 6)
					tntxp.cfec = value_transponder_fec(val, YTYPE::cable);
				// inv
				else if (i == 7)
					tntxp.inv = value_transponder_inversion(val, YTYPE::cable);
				// sys
				else if (i == 8)
					tntxp.sys = value_transponder_system(val);
			}
			else if (ytype == YTYPE::atsc)
			{
				// freq
				if (i == 3)
					tntxp.freq = std::atoi(val.data());
				// mod
				else if (i == 4)
					tntxp.amod = value_transponder_modulation(val, YTYPE::atsc);
				// sys
				else if (i == 5)
					tntxp.sys = value_transponder_system(val);
			}
		}

		string iname = "tns:";
		char yname = value_transponder_type(ytype);
		iname += yname;

		if (! dst->tuners.count(tv.ytype))
		{
			dst->tuners.emplace(tv.ytype, tv);
		}

		int idx = index.count(iname) ? index[iname].size() : 0;
		for (auto & x : dst->tuners[tv.ytype].tables)
		{
			if (x.second.name == tn.name)
			{
				idx = tn.index;
				break;
			}
		}

		// table idx
		tn.index = idx;
		// table ytype
		tn.ytype = ytype;

		char tnid[25];
		std::sprintf(tnid, "%c:%04x", yname, tn.index);
		tn.tnid = tnid;

		char trid[25];
		std::sprintf(trid, "%c:%04x:%04x", yname, tntxp.freq, tntxp.sr);
		tntxp.trid = trid;

		if (! dst->tuners[tv.ytype].tables.count(tn.tnid))
		{
			dst->tuners[tv.ytype].tables.emplace(tn.tnid, tn);
			if (tn.ytype == YTYPE::satellite)
				dst->tuners_pos.emplace(tn.pos, tn.tnid);
			dst->index[iname].emplace_back(pair (tn.index, tn.tnid)); //C++17
		}
		if (! dst->tuners[tv.ytype].tables[tn.tnid].transponders.count(tntxp.trid))
		{
			dst->tuners[tv.ytype].tables[tn.tnid].transponders.emplace(tntxp.trid, tntxp);
			dst->index[tn.tnid].emplace_back(pair (tntxp.index, tntxp.trid)); //C++17
		}
	}
}

void e2db_converter::csv_channel_list(string& csv, string bname, DOC_VIEW view)
{
	if (index.count(bname))
		debug("csv_channel_list()", "bname", bname);
	else
		error("csv_channel_list()", "bname", bname);
	debug("csv_channel_list()", "view", view);

	string ub_name;
	if (userbouquets.count(bname))
	{
		userbouquet ub = userbouquets[bname];
		ub_name = ub.name;
	}

	stringstream ss;

	if (CSV_HEADER)
	{
		ss << CSV_ESCAPE << "Index" << CSV_ESCAPE << CSV_SEPARATOR;
		ss << CSV_ESCAPE << "Ch Name" << CSV_ESCAPE << CSV_SEPARATOR;
		ss << CSV_ESCAPE << "Reference ID" << CSV_ESCAPE << CSV_SEPARATOR;
		ss << CSV_ESCAPE << "Service ID" << CSV_ESCAPE << CSV_SEPARATOR;
		ss << CSV_ESCAPE << "Transport ID" << CSV_ESCAPE << CSV_SEPARATOR;
		ss << CSV_ESCAPE << "Network ID" << CSV_ESCAPE << CSV_SEPARATOR;
		ss << CSV_ESCAPE << "DVB Namespace" << CSV_ESCAPE << CSV_SEPARATOR;
		ss << CSV_ESCAPE << "Service Type" << CSV_ESCAPE << CSV_SEPARATOR;
		ss << CSV_ESCAPE << "Service Number" << CSV_ESCAPE << CSV_SEPARATOR;
		ss << CSV_ESCAPE << "CAS" << CSV_ESCAPE << CSV_SEPARATOR;
		ss << CSV_ESCAPE << "CAID" << CSV_ESCAPE << CSV_SEPARATOR;
		ss << CSV_ESCAPE << "Provider" << CSV_ESCAPE << CSV_SEPARATOR;
		ss << CSV_ESCAPE << "System" << CSV_ESCAPE << CSV_SEPARATOR;
		ss << CSV_ESCAPE << "Position" << CSV_ESCAPE << CSV_SEPARATOR;
		ss << CSV_ESCAPE << "Tuner" << CSV_ESCAPE << CSV_SEPARATOR;
		ss << CSV_ESCAPE << "Frequency" << CSV_ESCAPE << CSV_SEPARATOR;
		ss << CSV_ESCAPE << "Polarization" << CSV_ESCAPE << CSV_SEPARATOR;
		ss << CSV_ESCAPE << "Symbol Rate" << CSV_ESCAPE << CSV_SEPARATOR;
		ss << CSV_ESCAPE << "FEC" << CSV_ESCAPE;
		if (view == DOC_VIEW::view_userbouquets)
		{
			ss << CSV_SEPARATOR;
			ss << CSV_ESCAPE << "Userbouquet" << CSV_ESCAPE << CSV_SEPARATOR;
			ss << CSV_ESCAPE << "Userbouquet Name" << CSV_ESCAPE;
		}
		ss << CSV_DELIMITER;
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
			int ssid = ch.ssid;
			int tsid = ch.tsid;
			int onid = ch.onid;
			int dvbns = ch.dvbns;
			string stype = value_service_type(ch);
			int snum = ch.snum;
			string scas;
			string scaid;
			if (ch.data.count(SDATA::C))
			{
				scas = "$";
				vector<string> cas;
				for (string & w : ch.data[SDATA::C])
				{
					string caidpx = w.substr(0, 2);
					if (SDATA_CAS.count(caidpx))
						cas.emplace_back(SDATA_CAS.at(caidpx) + ':' + w);
					else
						cas.emplace_back(w);
				}
				for (size_t i = 0; i < cas.size(); i++)
				{
					scaid.append(cas[i]);
					if (i != cas.size() - 1)
						scaid.append("|");
				}
			}
			string pname = value_channel_provider(ch);
			string sys = value_transponder_system(tx);
			string pos = value_transponder_position(tx);
			string tname = get_tuner_name(tx);
			int freq = tx.freq;
			string pol = tx.pol != -1 ? SAT_POL[tx.pol] : "";
			int sr = tx.sr;
			string fec;
			switch (tx.ytype)
			{
				case YTYPE::satellite:
					fec = SAT_FEC[tx.fec];
				break;
				case YTYPE::terrestrial:
					fec = TER_FEC[tx.hpfec] + '|' + TER_FEC[tx.lpfec];
				break;
				case YTYPE::cable:
					fec = CAB_FEC[tx.cfec];
				break;
				case YTYPE::atsc:
					fec = "";
				break;
			}

			ss << idx << CSV_SEPARATOR;
			ss << CSV_ESCAPE << chname << CSV_ESCAPE << CSV_SEPARATOR;
			ss << refid << CSV_SEPARATOR;
			ss << ssid << CSV_SEPARATOR;
			ss << tsid << CSV_SEPARATOR;
			ss << onid << CSV_SEPARATOR;
			ss << dvbns << CSV_SEPARATOR;
			ss << stype << CSV_SEPARATOR;
			ss << snum << CSV_SEPARATOR;
			ss << scas << CSV_SEPARATOR;
			ss << CSV_ESCAPE << scaid << CSV_ESCAPE << CSV_SEPARATOR;
			ss << CSV_ESCAPE << pname << CSV_ESCAPE << CSV_SEPARATOR;
			ss << sys << CSV_SEPARATOR;
			ss << pos << CSV_SEPARATOR;
			ss << tname << CSV_SEPARATOR;
			ss << freq << CSV_SEPARATOR;
			ss << pol << CSV_SEPARATOR;
			ss << sr << CSV_SEPARATOR;
			ss << fec;
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
			ss << CSV_SEPARATOR;
			ss << CSV_SEPARATOR;
			ss << atype << CSV_SEPARATOR;
			ss << CSV_SEPARATOR;
			ss << CSV_SEPARATOR;
			ss << CSV_ESCAPE << CSV_ESCAPE << CSV_SEPARATOR;
			ss << CSV_ESCAPE << CSV_ESCAPE << CSV_SEPARATOR;
			ss << CSV_SEPARATOR;
			ss << CSV_SEPARATOR;
			ss << CSV_SEPARATOR;
			ss << CSV_SEPARATOR;
			ss << CSV_SEPARATOR;
			ss << "";
		}
		if (view == DOC_VIEW::view_userbouquets)
		{
			ss << CSV_SEPARATOR;
			ss << CSV_ESCAPE << bname << CSV_ESCAPE << CSV_SEPARATOR;
			ss << CSV_ESCAPE << ub_name << CSV_ESCAPE;
		}
		ss << CSV_DELIMITER;
	}

	csv = ss.str();
}

void e2db_converter::csv_channel_list_extended(string& csv, string bname, DOC_VIEW view)
{
	if (index.count(bname))
		debug("csv_channel_list_extended()", "bname", bname);
	else
		error("csv_channel_list_extended()", "bname", bname);
	debug("csv_channel_list_extended()", "view", view);

	string ub_name;
	if (userbouquets.count(bname))
	{
		userbouquet ub = userbouquets[bname];
		ub_name = ub.name;
	}

	stringstream ss;

	if (CSV_HEADER)
	{
		ss << CSV_ESCAPE << "Index" << CSV_ESCAPE << CSV_SEPARATOR;
		ss << CSV_ESCAPE << "Ch Name" << CSV_ESCAPE << CSV_SEPARATOR;
		ss << CSV_ESCAPE << "Reference ID" << CSV_ESCAPE << CSV_SEPARATOR;
		ss << CSV_ESCAPE << "Service ID" << CSV_ESCAPE << CSV_SEPARATOR;
		ss << CSV_ESCAPE << "Transport ID" << CSV_ESCAPE << CSV_SEPARATOR;
		ss << CSV_ESCAPE << "Network ID" << CSV_ESCAPE << CSV_SEPARATOR;
		ss << CSV_ESCAPE << "DVB Namespace" << CSV_ESCAPE << CSV_SEPARATOR;
		ss << CSV_ESCAPE << "Service Type" << CSV_ESCAPE << CSV_SEPARATOR;
		ss << CSV_ESCAPE << "Service Number" << CSV_ESCAPE << CSV_SEPARATOR;
		ss << CSV_ESCAPE << "CAS" << CSV_ESCAPE << CSV_SEPARATOR;
		ss << CSV_ESCAPE << "Provider" << CSV_ESCAPE << CSV_SEPARATOR;
		ss << CSV_ESCAPE << "CAID" << CSV_ESCAPE << CSV_SEPARATOR;
		ss << CSV_ESCAPE << "Src ID" << CSV_ESCAPE << CSV_SEPARATOR;
		ss << CSV_ESCAPE << "System" << CSV_ESCAPE << CSV_SEPARATOR;
		ss << CSV_ESCAPE << "Position" << CSV_ESCAPE << CSV_SEPARATOR;
		ss << CSV_ESCAPE << "Tuner" << CSV_ESCAPE << CSV_SEPARATOR;
		ss << CSV_ESCAPE << "Frequency" << CSV_ESCAPE << CSV_SEPARATOR;
		ss << CSV_ESCAPE << "Polarization" << CSV_ESCAPE << CSV_SEPARATOR;
		ss << CSV_ESCAPE << "Symbol Rate" << CSV_ESCAPE << CSV_SEPARATOR;
		ss << CSV_ESCAPE << "Inner FEC" << CSV_ESCAPE << CSV_SEPARATOR;
		ss << CSV_ESCAPE << "HP FEC" << CSV_ESCAPE << CSV_SEPARATOR;
		ss << CSV_ESCAPE << "LP FEC" << CSV_ESCAPE << CSV_SEPARATOR;
		ss << CSV_ESCAPE << "Modulation" << CSV_ESCAPE << CSV_SEPARATOR;
		ss << CSV_ESCAPE << "Bandwidth" << CSV_ESCAPE << CSV_SEPARATOR;
		ss << CSV_ESCAPE << "Tmx Mode" << CSV_ESCAPE << CSV_SEPARATOR;
		ss << CSV_ESCAPE << "Inversion" << CSV_ESCAPE << CSV_SEPARATOR;
		ss << CSV_ESCAPE << "Roll Offset" << CSV_ESCAPE << CSV_SEPARATOR;
		ss << CSV_ESCAPE << "Pilot" << CSV_ESCAPE << CSV_SEPARATOR;
		ss << CSV_ESCAPE << "Guard Interval" << CSV_ESCAPE << CSV_SEPARATOR;
		ss << CSV_ESCAPE << "Hierarchy" << CSV_ESCAPE << CSV_SEPARATOR;
		ss << CSV_ESCAPE << "SAT Flags" << CSV_ESCAPE << CSV_SEPARATOR;
		ss << CSV_ESCAPE << "Txp Index" << CSV_ESCAPE << CSV_SEPARATOR;
		ss << CSV_ESCAPE << "Txp Flags" << CSV_ESCAPE << CSV_SEPARATOR;
		ss << CSV_ESCAPE << "Service Cached" << CSV_ESCAPE;
		if (view == DOC_VIEW::view_userbouquets)
		{
			ss << CSV_SEPARATOR;
			ss << CSV_ESCAPE << "Userbouquet" << CSV_ESCAPE << CSV_SEPARATOR;
			ss << CSV_ESCAPE << "Userbouquet Name" << CSV_ESCAPE;
		}
		ss << CSV_DELIMITER;
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
			int ssid = ch.ssid;
			int tsid = ch.tsid;
			int dvbns = ch.dvbns;
			int onid = ch.onid;
			string stype = value_service_type(ch);
			int snum = ch.snum;
			string scas;
			string scaid;
			if (ch.data.count(SDATA::C))
			{
				scas = "$";
				vector<string> cas;

				for (string & w : ch.data[SDATA::C])
				{
					string caidpx = w.substr(0, 2);
					if (SDATA_CAS.count(caidpx))
						cas.emplace_back(SDATA_CAS.at(caidpx) + ':' + w);
					else
						cas.emplace_back(w);
				}
				for (size_t i = 0; i < cas.size(); i++)
				{
					scaid.append(cas[i]);
					if (i != cas.size() - 1)
						scaid.append("|");
				}
			}
			string pname = value_channel_provider(ch);
			int srcid = ch.srcid;
			string sys = value_transponder_system(tx);
			string pos = value_transponder_position(tx);
			string tname = get_tuner_name(tx);
			int freq = tx.freq;
			int sr = tx.sr;
			string pol, ifec, mod, rol, pil, hpfec, lpfec, band, txm, guard, hier, inv;
			switch (tx.ytype)
			{
				case YTYPE::satellite:
					pol = SAT_POL[tx.pol];
					ifec = SAT_FEC[tx.fec];
					mod = SAT_MOD[tx.mod];
					rol = SAT_ROL[tx.rol];
					pil = SAT_PIL[tx.pil];
					inv = SAT_INV[tx.inv];
				break;
				case YTYPE::terrestrial:
					hpfec = TER_FEC[tx.hpfec];
					lpfec = TER_FEC[tx.lpfec];
					mod = TER_MOD[tx.tmod];
					band = TER_BAND[tx.band];
					txm = TER_TRXMODE[tx.tmx];
					guard = TER_GUARD[tx.guard];
					hier = TER_HIER[tx.hier];
					inv = TER_INV[tx.inv];
				break;
				case YTYPE::cable:
					ifec = CAB_FEC[tx.cfec];
					mod = CAB_MOD[tx.mod];
					inv = CAB_INV[tx.inv];
				break;
				case YTYPE::atsc:
					mod = tx.amod;
					inv = tx.inv;
				break;
			}
			int flgs = tx.flgs;
			int txid = tx.index;
			string txflgs = tx.oflgs;
			string cached;
			if (ch.data.count(SDATA::c))
			{
				for (size_t i = 0; i < ch.data[SDATA::c].size(); i++)
				{
					cached.append(ch.data[SDATA::c][i]);
					if (i != ch.data[SDATA::c].size() - 1)
						cached.append("|");
				}
			}

			ss << idx << CSV_SEPARATOR;
			ss << CSV_ESCAPE << chname << CSV_ESCAPE << CSV_SEPARATOR;
			ss << refid << CSV_SEPARATOR;
			ss << ssid << CSV_SEPARATOR;
			ss << tsid << CSV_SEPARATOR;
			ss << onid << CSV_SEPARATOR;
			ss << dvbns << CSV_SEPARATOR;
			ss << stype << CSV_SEPARATOR;
			ss << snum << CSV_SEPARATOR;
			ss << scas <<CSV_SEPARATOR;
			ss << CSV_ESCAPE << scaid << CSV_ESCAPE << CSV_SEPARATOR;
			ss << CSV_ESCAPE << pname << CSV_ESCAPE << CSV_SEPARATOR;
			ss << srcid << CSV_SEPARATOR;
			ss << sys << CSV_SEPARATOR;
			ss << pos << CSV_SEPARATOR;
			ss << tname << CSV_SEPARATOR;
			ss << freq << CSV_SEPARATOR;
			ss << pol << CSV_SEPARATOR;
			ss << sr << CSV_SEPARATOR;
			ss << ifec << CSV_SEPARATOR;
			ss << hpfec << CSV_SEPARATOR;
			ss << lpfec << CSV_SEPARATOR;
			ss << mod << CSV_SEPARATOR;
			ss << band << CSV_SEPARATOR;
			ss << txm << CSV_SEPARATOR;
			ss << inv << CSV_SEPARATOR;
			ss << rol << CSV_SEPARATOR;
			ss << pil << CSV_SEPARATOR;
			ss << guard << CSV_SEPARATOR;
			ss << hier << CSV_SEPARATOR;
			ss << flgs << CSV_SEPARATOR;
			ss << txid << CSV_SEPARATOR;
			ss << txflgs << CSV_SEPARATOR;
			ss << CSV_ESCAPE << cached << CSV_ESCAPE;
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
			ss << CSV_SEPARATOR;
			ss << CSV_SEPARATOR;
			ss << atype << CSV_SEPARATOR;
			ss << CSV_SEPARATOR;
			ss << CSV_SEPARATOR;
			ss << CSV_ESCAPE << CSV_ESCAPE << CSV_SEPARATOR;
			ss << CSV_ESCAPE << CSV_ESCAPE << CSV_SEPARATOR;
			ss << CSV_SEPARATOR;
			ss << CSV_SEPARATOR;
			ss << CSV_SEPARATOR;
			ss << CSV_SEPARATOR;
			ss << CSV_SEPARATOR;
			ss << CSV_SEPARATOR;
			ss << CSV_SEPARATOR;
			ss << CSV_SEPARATOR;
			ss << CSV_SEPARATOR;
			ss << CSV_SEPARATOR;
			ss << CSV_SEPARATOR;
			ss << CSV_SEPARATOR;
			ss << CSV_SEPARATOR;
			ss << CSV_SEPARATOR;
			ss << CSV_SEPARATOR;
			ss << CSV_SEPARATOR;
			ss << CSV_SEPARATOR;
			ss << CSV_SEPARATOR;
			ss << CSV_SEPARATOR;
			ss << CSV_SEPARATOR;
			ss << CSV_SEPARATOR;
			ss << CSV_ESCAPE << CSV_ESCAPE;
		}
		if (view == DOC_VIEW::view_userbouquets)
		{
			ss << CSV_SEPARATOR;
			ss << CSV_ESCAPE << bname << CSV_ESCAPE << CSV_SEPARATOR;
			ss << CSV_ESCAPE << ub_name << CSV_ESCAPE;
		}
		ss << CSV_DELIMITER;
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
	string btype;
	if (bs.btype == STYPE::tv)
		btype = "TV";
	else if (bs.btype == STYPE::radio)
		btype = "Radio";

	stringstream ss;

	if (CSV_HEADER)
	{
		ss << CSV_ESCAPE << "Index" << CSV_ESCAPE << CSV_SEPARATOR;
		ss << CSV_ESCAPE << "Bouquet" << CSV_ESCAPE << CSV_SEPARATOR;
		ss << CSV_ESCAPE << "Userbouquet" << CSV_ESCAPE << CSV_SEPARATOR;
		ss << CSV_ESCAPE << "Name" << CSV_ESCAPE << CSV_SEPARATOR;
		ss << CSV_ESCAPE << "Type" << CSV_ESCAPE;
		ss << CSV_DELIMITER;
	}

	int i = 1;
	for (auto & bname : bs.userbouquets)
	{
		userbouquet ub = userbouquets[bname];
		ss << i++ << CSV_SEPARATOR;
		ss << CSV_ESCAPE << bs.name << CSV_ESCAPE << CSV_SEPARATOR;
		ss << CSV_ESCAPE << bname << CSV_ESCAPE << CSV_SEPARATOR;
		ss << CSV_ESCAPE << ub.name << CSV_ESCAPE << CSV_SEPARATOR;
		ss << btype;
		ss << CSV_DELIMITER;
	}

	csv = ss.str();
}

//TODO improve list
void e2db_converter::csv_tunersets_list(string& csv, int ytype)
{
	debug("csv_tunersets_list()", "ytype", ytype);

	tunersets tv = tuners[ytype];
	string iname = "tns:";
	char yname = value_transponder_type(ytype);
	iname += yname;

	stringstream ss;

	if (CSV_HEADER)
	{
		ss << CSV_ESCAPE << "Index" << CSV_ESCAPE << CSV_SEPARATOR;
		ss << CSV_ESCAPE << "Name" << CSV_ESCAPE << CSV_SEPARATOR;
		ss << CSV_ESCAPE << "Position" << CSV_ESCAPE << CSV_SEPARATOR;
		if (ytype == YTYPE::satellite)
		{
			ss << CSV_ESCAPE << "Frequency" << CSV_ESCAPE << CSV_SEPARATOR;
			ss << CSV_ESCAPE << "Polarization" << CSV_ESCAPE << CSV_SEPARATOR;
			ss << CSV_ESCAPE << "Symbol Rate" << CSV_ESCAPE << CSV_SEPARATOR;
			ss << CSV_ESCAPE << "FEC" << CSV_ESCAPE << CSV_SEPARATOR;
			ss << CSV_ESCAPE << "System" << CSV_ESCAPE << CSV_SEPARATOR;
			ss << CSV_ESCAPE << "Modulation" << CSV_ESCAPE << CSV_SEPARATOR;
			ss << CSV_ESCAPE << "Inversion" << CSV_ESCAPE << CSV_SEPARATOR;
			ss << CSV_ESCAPE << "Pilot" << CSV_ESCAPE << CSV_SEPARATOR;
			ss << CSV_ESCAPE << "Roll Offset" << CSV_ESCAPE;
		}
		else if (ytype == YTYPE::terrestrial)
		{
			ss << CSV_ESCAPE << "Frequency" << CSV_ESCAPE << CSV_SEPARATOR;
			ss << CSV_ESCAPE << "Constellation" << CSV_ESCAPE << CSV_SEPARATOR;
			ss << CSV_ESCAPE << "Bandwidth" << CSV_ESCAPE << CSV_SEPARATOR;
			ss << CSV_ESCAPE << "System" << CSV_ESCAPE << CSV_SEPARATOR;
			ss << CSV_ESCAPE << "Tmx Mode" << CSV_ESCAPE << CSV_SEPARATOR;
			ss << CSV_ESCAPE << "HP FEC" << CSV_ESCAPE << CSV_SEPARATOR;
			ss << CSV_ESCAPE << "LP FEC" << CSV_ESCAPE << CSV_SEPARATOR;
			ss << CSV_ESCAPE << "Inversion" << CSV_ESCAPE << CSV_SEPARATOR;
			ss << CSV_ESCAPE << "Guard Interval" << CSV_ESCAPE << CSV_SEPARATOR;
			ss << CSV_ESCAPE << "Hierarchy" << CSV_ESCAPE;
		}
		else if (ytype == YTYPE::cable)
		{
			ss << CSV_ESCAPE << "Frequency" << CSV_ESCAPE << CSV_SEPARATOR;
			ss << CSV_ESCAPE << "Modulation" << CSV_ESCAPE << CSV_SEPARATOR;
			ss << CSV_ESCAPE << "Symbol Rate" << CSV_ESCAPE << CSV_SEPARATOR;
			ss << CSV_ESCAPE << "FEC" << CSV_ESCAPE << CSV_SEPARATOR;
			ss << CSV_ESCAPE << "Inversion" << CSV_ESCAPE << CSV_SEPARATOR;
			ss << CSV_ESCAPE << "System" << CSV_ESCAPE;
		}
		else if (ytype == YTYPE::atsc)
		{
			ss << CSV_ESCAPE << "Frequency" << CSV_ESCAPE << CSV_SEPARATOR;
			ss << CSV_ESCAPE << "Modulation" << CSV_ESCAPE << CSV_SEPARATOR;
			ss << CSV_ESCAPE << "System" << CSV_ESCAPE;
		}
		ss << CSV_DELIMITER;
	}

	for (auto & x : index[iname])
	{
		string tnid = x.second;
		tunersets_table tn = tv.tables[tnid];
		string tnname = tn.name;
		string pos;
		if (ytype == YTYPE::satellite)
		{
			pos = value_transponder_position(tn);
		}

		int i = 1;
		for (auto & x : index[tn.tnid])
		{
			string trid = x.second;
			tunersets_transponder tntxp = tn.transponders[trid];

			ss << i++ << CSV_SEPARATOR;
			ss << CSV_ESCAPE << tnname << CSV_ESCAPE << CSV_SEPARATOR;
			ss << pos << CSV_SEPARATOR;
			if (ytype == YTYPE::satellite)
			{
				int freq = tntxp.freq;
				string pol = value_transponder_polarization(tntxp.pol);
				int sr = tntxp.sr;
				string fec = value_transponder_fec(tntxp.fec, YTYPE::satellite);
				string sys = value_transponder_system(tntxp.sys, YTYPE::satellite);
				string mod = value_transponder_modulation(tntxp.mod, YTYPE::satellite);
				string inv = value_transponder_inversion(tntxp.inv, YTYPE::satellite);
				string rol = value_transponder_rollof(tntxp.rol);
				string pil = value_transponder_pilot(tntxp.pil);

				ss << freq << CSV_SEPARATOR;
				ss << pol << CSV_SEPARATOR;
				ss << sr << CSV_SEPARATOR;
				ss << fec << CSV_SEPARATOR;
				ss << sys << CSV_SEPARATOR;
				ss << mod << CSV_SEPARATOR;
				ss << inv << CSV_SEPARATOR;
				ss << rol << CSV_SEPARATOR;
				ss << pil;
			}
			else if (ytype == YTYPE::terrestrial)
			{
				int freq = tntxp.freq;
				string tmod = value_transponder_modulation(tntxp.tmod, YTYPE::terrestrial);
				string band = value_transponder_bandwidth(tntxp.band);
				string sys = value_transponder_system(tntxp.sys, YTYPE::terrestrial);
				string tmx = value_transponder_tmx_mode(tntxp.tmx);
				string hpfec = value_transponder_fec(tntxp.hpfec, YTYPE::terrestrial);
				string lpfec = value_transponder_fec(tntxp.lpfec, YTYPE::terrestrial);
				string inv = value_transponder_inversion(tntxp.inv, YTYPE::terrestrial);
				string guard = value_transponder_guard(tntxp.guard);
				string hier = value_transponder_hier(tntxp.hier);

				ss << freq << CSV_SEPARATOR;
				ss << tmod << CSV_SEPARATOR;
				ss << band << CSV_SEPARATOR;
				ss << sys << CSV_SEPARATOR;
				ss << tmx << CSV_SEPARATOR;
				ss << hpfec << CSV_SEPARATOR;
				ss << lpfec << CSV_SEPARATOR;
				ss << inv << CSV_SEPARATOR;
				ss << guard << CSV_SEPARATOR;
				ss << hier;
			}
			else if (ytype == YTYPE::cable)
			{
				int freq = tntxp.freq;
				string cmod = value_transponder_modulation(tntxp.cmod, YTYPE::cable);
				int sr = tntxp.sr;
				string cfec = value_transponder_fec(tntxp.cfec, YTYPE::cable);
				string inv = value_transponder_inversion(tntxp.inv, YTYPE::cable);
				string sys = value_transponder_system(tntxp.sys, YTYPE::cable);

				ss << freq << CSV_SEPARATOR;
				ss << cmod << CSV_SEPARATOR;
				ss << sr << CSV_SEPARATOR;
				ss << cfec << CSV_SEPARATOR;
				ss << inv << CSV_SEPARATOR;
				ss << sys;
			}
			else if (ytype == YTYPE::atsc)
			{
				int freq = tntxp.freq;
				int amod = tntxp.amod;
				string sys = value_transponder_system(tntxp.sys, YTYPE::atsc);

				ss << freq << CSV_SEPARATOR;
				ss << amod << CSV_SEPARATOR;
				ss << sys;
			}
			ss << CSV_DELIMITER;
		}
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
		name = "Services List";
	}
	page.header += "<h1>" + name + "</h1>\n";
	page.header += "<h3>" + filename + "</h3>\n";
	page.header += "</div>\n";
	page.title = filename + " - " + name;
}

void e2db_converter::page_footer(html_page& page, string filename, DOC_VIEW view)
{
	string editor = editor_string(true);
	string timestamp = editor_timestamp();

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
	page.body += "<th>Sys</th>\n";
	page.body += "<th>Pos</th>\n";
	page.body += "<th>Tuner</th>\n";
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
				for (size_t i = 0; i < cas.size(); i++)
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
			string pos = value_transponder_position(tx);
			string sys = value_transponder_system(tx);
			string tname = get_tuner_name(tx);

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
			page.body += "<td>" + sys + "</td>";
			page.body += "<td>" + pos + "</td>";
			page.body += "<td>" + tname + "</td>";
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
	string btype;
	if (bs.btype == STYPE::tv)
		btype = "TV";
	else if (bs.btype == STYPE::radio)
		btype = "Radio";
	page.body += "<tr>";

	page.body += "<div class=\"bouquet\">\n";
	page.body += "<table>\n";
	page.body += "<thead>\n";
	page.body += "<tr>\n";
	page.body += "<th>Index</th>\n";
	page.body += "<th>Bouquet</th>\n";
	page.body += "<th>Userbouquet</th>\n";
	page.body += "<th>Name</th>\n";
	page.body += "<th>Type</th>\n";
	page.body += "</tr>\n";

	page.body += "<tbody>\n";
	int i = 1;
	for (auto & bname : bs.userbouquets)
	{
		userbouquet ub = userbouquets[bname];
		page.body += "<td class=\"trid\">" + to_string(i++) + "</td>";
		page.body += "<td>" + bs.name + "</td>";
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

	page.body += "<div class=\"tunersets\">\n";

	tunersets tv = tuners[ytype];
	string iname = "tns:";
	char yname = value_transponder_type(ytype);
	iname += yname;
	
	for (auto & x : index[iname])
	{
		string tnid = x.second;
		tunersets_table tn = tv.tables[tnid];
		string tnname = tn.name;
		string pos;
		if (ytype == YTYPE::satellite)
		{
			pos = value_transponder_position(tn);
		}

		page.body += "<div class=\"transponder\">\n";
		page.body += "<h4>Transponders</h4>\n";
		page.body += "<h2>" + tnname + "</h2>\n";
		if (ytype == YTYPE::satellite)
			page.body += "<p>Position: <b>" + pos + "</b></p>\n";
		page.body += "<table>\n";
		page.body += "<thead>\n";
		page.body += "<tr>\n";
		page.body += "<th>Index</th>\n";
		if (ytype == YTYPE::satellite)
		{
			page.body += "<th>Freq</th>\n";
			page.body += "<th>Pol</th>\n";
			page.body += "<th>SR</th>\n";
			page.body += "<th>FEC</th>\n";
			page.body += "<th>Sys</th>\n";
			page.body += "<th>Mod</th>\n";
			page.body += "<th>Inv</th>\n";
			page.body += "<th>Rollof</th>\n";
			page.body += "<th>Pilot</th>\n";
		}
		else if (ytype == YTYPE::terrestrial)
		{
			page.body += "<th>Freq</th>\n";
			page.body += "<th>Const</th>\n";
			page.body += "<th>Band</th>\n";
			page.body += "<th>Sys</th>\n";
			page.body += "<th>Tmx Mode</th>\n";
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
			if (ytype == YTYPE::satellite)
			{
				string freq = to_string(tntxp.freq);
				string pol = value_transponder_polarization(tntxp.pol);
				string sr = to_string(tntxp.sr);
				string fec = value_transponder_fec(tntxp.fec, YTYPE::satellite);
				string sys = value_transponder_system(tntxp.sys, YTYPE::satellite);
				string mod = value_transponder_modulation(tntxp.mod, YTYPE::satellite);
				string inv = value_transponder_inversion(tntxp.inv, YTYPE::satellite);
				string rol = value_transponder_rollof(tntxp.rol);
				string pil = value_transponder_pilot(tntxp.pil);

				page.body += "<td>" + freq + "</td>";
				page.body += "<td>" + pol + "</td>";
				page.body += "<td>" + sr + "</td>";
				page.body += "<td>" + fec + "</td>";
				page.body += "<td>" + sys + "</td>";
				page.body += "<td>" + mod + "</td>";
				page.body += "<td>" + inv + "</td>";
				page.body += "<td>" + rol + "</td>";
				page.body += "<td>" + pil + "</td>";
			}
			else if (ytype == YTYPE::terrestrial)
			{
				string freq = to_string(tntxp.freq);
				string tmod = value_transponder_modulation(tntxp.tmod, YTYPE::terrestrial);
				string band = value_transponder_bandwidth(tntxp.band);
				string sys = value_transponder_system(tntxp.sys, YTYPE::terrestrial);
				string tmx = value_transponder_tmx_mode(tntxp.tmx);
				string hpfec = value_transponder_fec(tntxp.hpfec, YTYPE::terrestrial);
				string lpfec = value_transponder_fec(tntxp.lpfec, YTYPE::terrestrial);
				string inv = value_transponder_inversion(tntxp.inv, YTYPE::terrestrial);
				string guard = value_transponder_guard(tntxp.guard);
				string hier = value_transponder_hier(tntxp.hier);

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
				string cmod = value_transponder_modulation(tntxp.cmod, YTYPE::cable);
				string sr = to_string(tntxp.sr);
				string cfec = value_transponder_fec(tntxp.cfec, YTYPE::cable);
				string inv = value_transponder_inversion(tntxp.inv, YTYPE::cable);
				string sys = value_transponder_system(tntxp.sys, YTYPE::cable);

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
				string sys = value_transponder_system(tntxp.sys, YTYPE::atsc);

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
	string editor = editor_string(2);

	return "<!DOCTYPE html>\n\
<html lang=\"en\">\n\
<head>\n\
<meta charset=\"utf-8\">\n\
<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">\n\
<meta name=\"generator\" content=\"" + editor + "\">\n\
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
	return "</html>";
}

void e2db_converter::csv_document(e2db_file& file, string csv)
{
	debug("csv_document()");

	file.data = csv;
	file.mime = "text/plain";
	file.size = file.data.size();
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
	file.data = html;
	file.mime = "text/html";
	file.size = file.data.size();
}

}
