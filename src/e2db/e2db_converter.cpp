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

void e2db_converter::merge(e2db_abstract* dst)
{
	debug("merge()");

	this->db.transponders.merge(dst->db.transponders); //C++17
	this->db.services.merge(dst->db.services); //C++17
	this->tuners.merge(dst->tuners); //C++17
	this->bouquets.merge(dst->bouquets); //C++17
	this->index.merge(dst->index); //C++17
}

void e2db_converter::import_csv_file(FCONVS fci, fcopts opts, vector<string> paths)
{
	debug("import_csv_file()", "file path", "multiple");
	debug("import_csv_file()", "file input", fci);

	bool merge = this->get_input().size() != 0 ? true : false;
	auto* dst = merge ? new e2db_abstract : this;

	for (string & path : paths)
		import_csv_file(fci, opts, dst, path);
}

void e2db_converter::import_csv_file(FCONVS fci, fcopts opts, string path)
{
	debug("import_csv_file()", "file path", "singular");
	debug("import_csv_file()", "file input", fci);

	bool merge = this->get_input().size() != 0 ? true : false;
	auto* dst = merge ? new e2db_abstract : this;

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

	opts.filename = path;

	if (opts.bname.empty())
		opts.bname = std::filesystem::path(path).filename().replace_extension("").u8string(); //C++17

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
			pull_csv_userbouquets(ifile, dst, opts.bname, opts.btype);
		break;
		case FCONVS::convert_tunersets:
			pull_csv_tunersets(ifile, dst, opts.ytype);
		break;
		default:
			return;
	}

	std::clock_t end = std::clock();

	info("import_csv_file()", "elapsed time", to_string(int (end - start)) + " ms.");
}

void e2db_converter::export_csv_file(FCONVS fco, fcopts opts, string path)
{
	debug("export_csv_file()", "file path", "singular");
	debug("export_csv_file()", "file output", fco);
	
	std::clock_t start = std::clock();

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
			return;
	}

	for (auto & file : files)
	{
		ofstream out (path);
		out << file;
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

	opts.filename = filename;
	string filename = std::filesystem::path(path).filename().u8string(); //C++17

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
			return;
	}

	for (auto & file : files)
	{
		ofstream out (path);
		out << file;
		out.close();
	}

	std::clock_t end = std::clock();

	info("export_html_file()", "elapsed time", to_string(int (end - start)) + " ms.");
}

void e2db_converter::pull_csv_services(istream& ifile, e2db_abstract* dst)
{
	debug("pull_csv_services()");
	
	vector<vector<string>> sxv;
	convert_csv(ifile, sxv);

	for (unsigned int x = 0; x < sxv.size(); x++)
	{
		// csv header
		if (x == 0 && sxv[0][0] == "Index")
			continue;

		service ch;
		transponder tx;
		channel_reference chref;
		service_reference ref;

		for (unsigned int i = 0; i < sxv[x].size(); i++)
		{
			// debug("pull_csv_services()", to_string(x), sxv[x][i]);

			string& val = sxv[x][i];

			// idx
			if (i == 0)
				ch.index = tx.index = std::atoi(val.data());
			// name
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
			// stype
			else if (i == 5)
				ch.stype = std::atoi(val.data());
			// scas
			else if (i == 6)
				ch.data[SDATA::C] = value_channel_cas(val);
			// provider
			else if (i == 7)
				ch.data[SDATA::p] = {val};
			// freq
			else if (i == 8)
				tx.freq = std::atoi(val.data());
			// pol
			else if (i == 9)
				tx.pol = value_transponder_polarization(val);
			// sr
			else if (i == 10)
				tx.sr = std::atoi(val.data());
			// fec
			else if (i == 11)
				tx.fec = std::atoi(val.data());
			// pos
			else if (i == 12)
				tx.pos = value_transponder_position(val);
			// sys
			else if (i == 13)
				tx.sys = value_transponder_system(val);
		}

		if (chref.marker)
			continue;
		// x order has priority over ch.index
		if (ch.index != int (x))
			ch.index = tx.index = x;
		// ch.ssid has priority over ref.ssid
		if (! ch.ssid)
			ch.ssid = ref.ssid;
		// ch.tsid has priority over ref.tsid
		if (! ch.tsid)
			ch.tsid = tx.tsid = ref.tsid;
		// ch.stype has priority over chref.type
		if (! ch.stype)
			ch.stype = chref.type;
		ch.dvbns = tx.dvbns = ref.dvbns;
		ch.onid = to_string(ref.onid); //TODO
		tx.onid = ref.onid; //TODO

		char txid[25];
		// %4x:%8x
		std::sprintf(txid, "%x:%x", tx.tsid, tx.dvbns);
		tx.txid = txid;

		char chid[25];
		// %4x:%4x:%8x
		std::sprintf(chid, "%x:%x:%x", ch.ssid, ch.tsid, ch.dvbns);
		ch.chid = chid;
		ch.txid = txid;

		if (! dst->db.transponders.count(tx.txid))
		{
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
}

void e2db_converter::pull_csv_bouquets(istream& ifile, e2db_abstract* dst)
{
	debug("pull_csv_bouquets()");

	vector<vector<string>> sxv;
	convert_csv(ifile, sxv);

	bouquet bs;
	vector<userbouquet> ubs;

	for (unsigned int x = 0; x < sxv.size(); x++)
	{
		// csv header
		if (x == 0 && sxv[0][0] == "Index")
			continue;

		userbouquet ub;

		for (unsigned int i = 0; i < sxv[x].size(); i++)
		{
			debug("pull_csv_bouquets()", to_string(x), sxv[x][i]);

			string& val = sxv[x][i];

			// userbouquet idx
			if (i == 0)
				ub.index = std::atoi(val.data());
			// bouquet name
			else if (i == 1)
				bs.name = ub.pname = val;
			// userbouquet bname
			else if (i == 2)
				ub.bname = val;
			// userbouquet name
			else if (i == 3)
				ub.name = val;
			// btype - parse text
			else if (i == 4)
				bs.btype = std::atoi(val.data());
		}

		// x order has priority over ub.index
		if (ub.index != int (x))
			ub.index = x;
		
		bs.userbouquets.emplace_back(ub.bname);
	}

	if (bs.btype == STYPE::tv)
	{
		bs.bname = "bouquets.tv";
		bs.nname = STYPE_EXT_LABEL.at(STYPE::tv);
	}
	else if (bs.btype == 2)
	{
		bs.bname = "bouquets.radio";
		bs.nname = STYPE_EXT_LABEL.at(STYPE::radio);
	}

	if (! dst->bouquets.count(bs.bname))
	{
		// bouquet idx
		bs.index = dst->index["bss"].size();

		dst->bouquets.emplace(bs.bname, bs);
		dst->index["bss"].emplace_back(pair (bs.index, bs.bname)); //C++17
	}
	for (userbouquet & ub : ubs)
	{
		if (! dst->userbouquets.count(ub.bname))
		{
			dst->userbouquets.emplace(ub.bname, ub);
			dst->index["ubs"].emplace_back(pair (ub.index, ub.bname)); //C++17
		}
	}
}

void e2db_converter::pull_csv_userbouquets(istream& ifile, e2db_abstract* dst, string bname, int btype)
{
	debug("pull_csv_userbouquets()");

	vector<vector<string>> sxv;
	convert_csv(ifile, sxv);

	userbouquet ub;
	ub.bname = bname;

	for (unsigned int x = 0; x < sxv.size(); x++)
	{
		// csv header
		if (x == 0 && sxv[0][0] == "Index")
			continue;

		service ch;
		transponder tx;
		channel_reference chref;
		service_reference ref;

		for (unsigned int i = 0; i < sxv[x].size(); i++)
		{
			debug("pull_csv_userbouquets()", to_string(x), sxv[x][i]);

			string& val = sxv[x][i];

			// idx
			if (i == 0)
				ch.index = tx.index = std::atoi(val.data());
			// name
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
			// stype
			else if (i == 5)
				ch.stype = std::atoi(val.data());
			// scas
			else if (i == 6)
				ch.data[SDATA::C] = value_channel_cas(val);
			// provider
			else if (i == 7)
				ch.data[SDATA::p] = {val};
			// freq
			else if (i == 8)
				tx.freq = std::atoi(val.data());
			// pol
			else if (i == 9)
				tx.pol = value_transponder_polarization(val);
			// sr
			else if (i == 10)
				tx.sr = std::atoi(val.data());
			// fec
			else if (i == 11)
				tx.fec = std::atoi(val.data());
			// pos
			else if (i == 12)
				tx.pos = value_transponder_position(val);
			// sys
			else if (i == 13)
				tx.sys = value_transponder_system(val);
		}

		if (chref.marker)
		{
			// value
			chref.value = ch.chname;
		}
		else
		{
			// x order has priority over ch.index
			if (ch.index != int (x))
				ch.index = tx.index = x;
			// ch.stype has priority over chref.type
			if (! ch.stype)
				ch.stype = chref.type;
			// ch.ssid has priority over ref.ssid
			if (! ch.ssid)
				ch.ssid = ref.ssid;
			// ch.tsid has priority over ref.tsid
			if (! ch.tsid)
				ch.tsid = tx.tsid = ref.tsid;

			ch.dvbns = tx.dvbns = ref.dvbns;
			ch.onid = to_string(ref.onid); //TODO
			tx.onid = ref.onid; //TODO

			char txid[25];
			// %4x:%8x
			std::sprintf(txid, "%x:%x", tx.tsid, tx.dvbns);
			tx.txid = txid;

			char chid[25];
			// %4x:%4x:%8x
			std::sprintf(chid, "%x:%x:%x", ch.ssid, ch.tsid, ch.dvbns);
			ch.chid = chid;
			ch.txid = txid;

			if (! dst->db.transponders.count(tx.txid))
			{
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

		if (! ub.channels.count(chref.chid))
		{
			ub.channels.emplace(chref.chid, chref);
			dst->index[ub.bname].emplace_back(pair (chref.index, chref.chid)); //C++17

			if (chref.marker)
				dst->index["mks"].emplace_back(pair (ub.index, chref.chid)); //C++17
			else
				dst->index[ub.pname].emplace_back(pair ((dst->index[ub.pname].size() + 1), chref.chid)); //C++17
		}
	}

	if (! dst->userbouquets.count(ub.bname))
	{
		dst->userbouquets.emplace(ub.bname, ub);
		dst->index["ubs"].emplace_back(pair (ub.index, ub.bname)); //C++17
	}
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
		bs.userbouquets.emplace_back(bname);
		dst->bouquets[bs.bname] = bs;
	}
	else
	{
		bouquet bs;
		if (bs.btype == STYPE::tv)
		{
			bs.bname = "bouquets.tv";
			bs.name = "User - bouquet (TV)";
			bs.nname = STYPE_EXT_LABEL.at(STYPE::tv);
		}
		else if (bs.btype == 2)
		{
			bs.bname = "bouquets.radio";
			bs.name = "User - bouquet (Radio)";
			bs.nname = STYPE_EXT_LABEL.at(STYPE::radio);
		}
		bs.index = dst->index["bss"].size();
		dst->bouquets.emplace(bs.bname, bs);
	}
}

//TODO
void e2db_converter::pull_csv_tunersets(istream& ifile, e2db_abstract* dst, int ytype)
{
	debug("pull_csv_tunersets()");

	vector<vector<string>> sxv;
	convert_csv(ifile, sxv);

	tunersets tv;

	tv.charset = "utf-8";
	tv.ytype = ytype;

	for (unsigned int x = 0; x < sxv.size(); x++)
	{
		// ytype autodetect
		if (x == 0)
		{
			switch (sxv[x].size())
			{
				case 12: ytype = YTYPE::sat; break;
				case 13: ytype = YTYPE::terrestrial; break;
				case 9: ytype = YTYPE::cable; break;
				case 6: ytype = YTYPE::atsc; break;
			}
		}
		// csv header
		if (x == 0 && sxv[0][0] == "Name")
		{
			continue;
		}

		tunersets_table tn;
		// TODO
		tunersets_transponder tntxp;

		for (unsigned int i = 0; i < sxv[x].size(); i++)
		{
			debug("pull_csv_tunersets()", to_string(x), sxv[x][i]);

			string& val = sxv[x][i];

			// name
			if (i == 0)
				tn.name = val;
			// pos
			else if (i == 1)
				tn.pos = ytype == YTYPE::sat ? std::atoi(val.data()) : -1;
			// idx
			else if (i == 2)
				tntxp.index = std::atoi(val.data());

			if (ytype == YTYPE::sat)
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
					tntxp.fec = -1; //TODO
				// sys
				else if (i == 7)
					tntxp.sys = value_transponder_system(val);
				// mod
				else if (i == 8)
					tntxp.mod = -1; //TODO
				// inv
				else if (i == 9)
					tntxp.inv = -1; //TODO
				// pil
				else if (i == 10)
					tntxp.pil = -1; //TODO
				// rol
				else if (i == 11)
					tntxp.rol = -1; //TODO
			}
			else if (ytype == YTYPE::terrestrial)
			{
				// freq
				if (i == 3)
					tntxp.freq = std::atoi(val.data());
				// tmod
				else if (i == 4)
					tntxp.tmod = -1; //TODO
				// band
				else if (i == 5)
					tntxp.band = std::atoi(val.data());
				// sys
				else if (i == 6)
					tntxp.sys = 0;
				// tmx
				else if (i == 7)
					tntxp.tmx = -1; //TODO
				// hpfec
				else if (i == 8)
					tntxp.hpfec = -1; //TODO
				// lpfe
				else if (i == 9)
					tntxp.lpfec = -1; //TODO
				// inv
				else if (i == 10)
					tntxp.inv = -1; //TODO
				// guard
				else if (i == 11)
					tntxp.guard = -1; //TODO
				// hier
				else if (i == 11)
					tntxp.hier = -1; //TODO
			}
			else if (ytype == YTYPE::cable)
			{
				// freq
				if (i == 3)
					tntxp.freq = std::atoi(val.data());
				// mod
				else if (i == 4)
					tntxp.mod = -1; //TODO
				// sr
				else if (i == 5)
					tntxp.sr = std::atoi(val.data());
				// fec
				else if (i == 6)
					tntxp.fec = -1; //TODO
				// inv
				else if (i == 7)
					tntxp.inv = -1; //TODO 
				// sys
				else if (i == 8)
					tntxp.sys = -1;
			}
			else if (ytype == YTYPE::atsc)
			{
				// freq
				if (i == 3)
					tntxp.freq = std::atoi(val.data());
				// mod
				else if (i == 4)
					tntxp.mod = -1; //TODO
				// sys
				else if (i == 5)
					tntxp.sys = -1;
			}
		}
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
		case STYPE::data:
			iname = "chs:0";
		break;
		// TV
		case STYPE::tv:
			iname = "chs:1";
		break;
		// Radio
		case STYPE::radio:
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
	debug("push_csv_tunersets()", "ytype", ytype);

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
		case STYPE::data:
			iname = "chs:0";
			xname = STYPE_EXT_LABEL.at(STYPE::data);
		break;
		// TV
		case STYPE::tv:
			iname = "chs:1";
			xname = STYPE_EXT_LABEL.at(STYPE::tv);
		break;
		// Radio
		case STYPE::radio:
			iname = "chs:2";
			xname = STYPE_EXT_LABEL.at(STYPE::radio);
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
	debug("push_html_tunersets()", "ytype", ytype);

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

void e2db_converter::convert_csv(istream& ifile, vector<vector<string>>& sxv)
{
	debug("convert_csv()");

	const char dlm = CSV_ENDLINE;
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
		ss << CSV_ESCAPE << "Sys" << CSV_ESCAPE;
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
			string ppos = value_transponder_position(tx);
			string pos = ppos;
			string psys = value_transponder_system(tx);
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
			ss << CSV_ESCAPE << pos << CSV_ESCAPE << CSV_SEPARATOR;
			ss << sys;
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
			ss << "";
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
		ss << CSV_ENDLINE;
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
			ppos = value_transponder_position(tn);
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
				ss << CSV_ESCAPE << "Rollof" << CSV_ESCAPE;
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
				ss << CSV_ESCAPE << "Hier" << CSV_ESCAPE;
			}
			else if (ytype == YTYPE::cable)
			{
				ss << CSV_ESCAPE << "Freq" << CSV_ESCAPE << CSV_SEPARATOR;
				ss << CSV_ESCAPE << "Mod" << CSV_ESCAPE << CSV_SEPARATOR;
				ss << CSV_ESCAPE << "SR" << CSV_ESCAPE << CSV_SEPARATOR;
				ss << CSV_ESCAPE << "FEC" << CSV_ESCAPE << CSV_SEPARATOR;
				ss << CSV_ESCAPE << "Inv" << CSV_ESCAPE << CSV_SEPARATOR;
				ss << CSV_ESCAPE << "Sys" << CSV_ESCAPE;
			}
			else if (ytype == YTYPE::atsc)
			{
				ss << CSV_ESCAPE << "Freq" << CSV_ESCAPE << CSV_SEPARATOR;
				ss << CSV_ESCAPE << "Mod" << CSV_ESCAPE << CSV_SEPARATOR;
				ss << CSV_ESCAPE << "Sys" << CSV_ESCAPE;
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
				ss << rol;
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
				ss << hier;
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
				ss << sys;
			}
			else if (ytype == YTYPE::atsc)
			{
				int freq = tntxp.freq;
				int amod = tntxp.amod;
				string sys = "ATSC";

				ss << freq << CSV_SEPARATOR;
				ss << amod << CSV_SEPARATOR;
				ss << sys;
			}
			ss << CSV_ENDLINE;
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
		name = "Service List";
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
			string ppos = value_transponder_position(tx);
			string pos = ppos;
			string psys = value_transponder_system(tx);
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
			ppos = value_transponder_position(tn);
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
