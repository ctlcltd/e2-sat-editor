/*!
 * e2-sat-editor/src/e2db/e2db_maker.cpp
 * 
 * @link https://github.com/ctlcltd/e2-sat-editor
 * @copyright e2 SAT Editor Team
 * @author Leonardo Laureti
 * @version 0.3
 * @license MIT License
 * @license GNU GPLv3 License
 */

#include <cstdio>
#include <ctime>
#include <clocale>
#include <algorithm>
#include <sstream>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <filesystem>

#include "e2db_maker.h"

using std::ofstream, std::stringstream, std::hex, std::dec, std::setfill, std::setw, std::uppercase, std::to_string, std::endl;

namespace e2se_e2db
{

e2db_maker::e2db_maker()
{
	std::setlocale(LC_NUMERIC, "C");

	this->log = new e2se::logger("e2db", "e2db_maker");
}

void e2db_maker::make_e2db()
{
	debug("make_e2db");

	std::clock_t start = std::clock();

	make_e2db_lamedb();
	make_e2db_bouquets();
	make_e2db_userbouquets();
	if (MAKER_TUNERSETS)
		make_db_tunersets();
	if (MAKER_PARENTALLOCK_LIST)
		make_e2db_parentallock_list();

	std::clock_t end = std::clock();

	info("make_e2db", "elapsed time", to_string(int (end - start)) + " ms.");
}

void e2db_maker::make_e2db_lamedb()
{
	debug("make_e2db_lamedb");

	switch (LAMEDB_VER)
	{
		case 2:
		case 3:
			make_e2db_lamedb("services", LAMEDB_VER);
		break;
		case 4:
			make_e2db_lamedb("lamedb", 4);
		break;
		case 5:
			make_e2db_lamedb("lamedb5", 5);
		break;
		default:
		return error("make_e2db_lamedb", "Maker Error", "Unknown Lamedb services file format.");
	}

	//TEST
	if (MAKER_LAMEDB5)
		make_e2db_lamedb("lamedb5", 5);
	//TEST
}

void e2db_maker::make_e2db_lamedb(string filename, int ver)
{
	debug("make_e2db_lamedb", "version", ver);

	e2db_file file;
	make_lamedb(filename, file, ver);
	this->e2db_out[filename] = file;
}

void e2db_maker::make_lamedb(string filename, e2db_file& file, int ver)
{
	debug("make_lamedb", "version", ver);

	const string (&formats)[13] = ver < 5 ? LAMEDBX_FORMATS : LAMEDB5_FORMATS;

	stringstream ss;
	ss << "eDVB services /" << ver << "/" << endl;

	ss << formats[MAKER_FORMAT::b_transponders_start];
	for (auto & x : index["txs"])
	{
		transponder tx = db.transponders[x.second];
		ss << formats[MAKER_FORMAT::b_transponder_flag] << formats[MAKER_FORMAT::b_delimiter];
		ss << hex;
		ss << setfill('0') << setw(8) << tx.dvbns;
		ss << ':' << setfill('0') << setw(4) << tx.tsid;
		ss << ':' << setfill('0') << setw(4) << tx.onid;
		ss << dec;
		ss << formats[MAKER_FORMAT::b_transponder_params_separator];
		ss << value_transponder_type(tx.ytype) << formats[MAKER_FORMAT::b_transponder_space_delimiter];
		switch (tx.ytype)
		{
			case YTYPE::satellite: // DVB-S
				ss << int (tx.freq * 1e3);
				ss << ':' << int (tx.sr * 1e3);
				ss << ':' << tx.pol;
				ss << ':' << tx.fec;
				ss << ':' << tx.pos;
				ss << ':' << tx.inv;
				ss << ':' << tx.flgs;
				if (tx.sys != -1)
					ss << ':' << tx.sys;
				if (tx.mod != -1)
					ss << ':' << tx.mod;
				if (tx.rol != -1)
					ss << ':' << tx.rol;
				if (tx.pil != -1)
					ss << ':' << tx.pil;
			break;
			case YTYPE::terrestrial: // DVB-T
				ss << int (tx.freq * 1e3);
				ss << ':' << tx.band;
				ss << ':' << tx.hpfec;
				ss << ':' << tx.lpfec;
				ss << ':' << tx.tmod;
				ss << ':' << tx.tmx;
				ss << ':' << tx.guard;
				ss << ':' << tx.hier;
				ss << ':' << tx.inv;
				if (! tx.oflgs.empty())
					ss << tx.oflgs;
			break;
			//TODO test params and freq round
			case YTYPE::cable: // DVB-C
				ss << int (tx.freq * 1e3);
				ss << ':' << int (tx.sr * 1e3);
				ss << ':' << tx.inv;
				ss << ':' << tx.cmod;
				ss << ':' << tx.cfec;
				if (! tx.oflgs.empty())
					ss << tx.oflgs;
			break;
			//TODO test params and freq round
			case YTYPE::atsc: // ATSC
				ss << int (tx.freq * 1e3);
				ss << ':' << tx.inv;
				ss << ':' << tx.amod;
				ss << ':' << tx.flgs;
				ss << ':' << tx.sys;
				if (! tx.oflgs.empty())
					ss << tx.oflgs;
			break;
			default:
			return error("make_lamedb", "Maker Error", "Unknown transponder type.");
		}
		ss << formats[MAKER_FORMAT::b_transponder_endline];
	}
	ss << formats[MAKER_FORMAT::b_section_end];

	ss << formats[MAKER_FORMAT::b_services_start];
	for (auto & x : index["chs"])
	{
		service ch = db.services[x.second];
		ss << formats[MAKER_FORMAT::b_service_flag] << formats[MAKER_FORMAT::b_delimiter];
		ss << hex;
		ss << setfill('0') << setw(4) << ch.ssid;
		ss << ':' << setfill('0') << setw(8) << ch.dvbns;
		ss << ':' << setfill('0') << setw(4) << ch.tsid;
		ss << ':' << setfill('0') << setw(4) << ch.onid;
		ss << dec;
		ss << ':' << ch.stype;
		ss << ':' << ch.snum;
		if (LAMEDB_VER == 5)
			ss << ':' << ch.srcid;
		ss << formats[MAKER_FORMAT::b_service_params_separator];
		ss << formats[MAKER_FORMAT::b_service_param_escape] << ch.chname << formats[MAKER_FORMAT::b_service_param_escape];
		ss << formats[MAKER_FORMAT::b_service_params_separator];
		//TODO max length 256 EOL
		auto last_key = (*prev(ch.data.cend()));
		for (auto & q : ch.data)
		{
			char d;
			switch (q.first)
			{
				case SDATA::p: d = 'p'; break;
				case SDATA::c: d = 'c'; break;
				case SDATA::C: d = 'C'; break;
				case SDATA::f: d = 'f'; break;
				default: d = q.first;
			}
			for (size_t i = 0; i < q.second.size(); i++)
			{
				ss << d << ':' << q.second[i];
				if (! q.second[i].empty() && (i != q.second.size() - 1 || q.first != last_key.first))
					ss << ',';
			}
		}
		ss << formats[MAKER_FORMAT::b_service_endline];
	}
	ss << formats[MAKER_FORMAT::b_section_end];

	ss << formats[MAKER_FORMAT::b_comment] << "editor: " << editor_string() << endl;
	ss << formats[MAKER_FORMAT::b_comment] << "datetime: " << editor_timestamp() << endl;

	file.filename = filename;
	file.data = ss.str();
	file.mime = "text/plain";
	file.size = file.data.size();
}

void e2db_maker::make_e2db_bouquets()
{
	debug("make_e2db_bouquets");

	for (auto & x : bouquets)
	{
		e2db_file file;
		if (LAMEDB_VER < 4)
			make_bouquet_epl(x.first, file);
		else
			make_bouquet(x.first, file);
		this->e2db_out[x.first] = file;
	}
}

void e2db_maker::make_e2db_userbouquets()
{
	debug("make_e2db_userbouquets");

	for (auto & x : userbouquets)
	{
		e2db_file file;
		make_userbouquet(x.first, file);
		this->e2db_out[x.first] = file;
	}
}

void e2db_maker::make_db_tunersets()
{
	debug("make_db_tunersets");

	for (auto & x : tuners)
	{
		string filename;
		switch (x.first)
		{
			case YTYPE::satellite:
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
			default:
			return error("make_db_tunersets", "Maker Error", "These settings are not supported.");
		}
		e2db_file file;
		make_tunersets_xml(filename, x.first, file);
		this->e2db_out[filename] = file;
	}
}

void e2db_maker::make_e2db_parentallock_list()
{
	debug("make_e2db_parentallock_list");

	if (db.parental == PARENTALLOCK::locked)
	{
		e2db_file file;
		make_parentallock_list("services.locked", PARENTALLOCK::locked, file);
		this->e2db_out["services.locked"] = file;
	}
	else
	{
		string filename = db.parental ? "blacklist" : "whitelist";

		e2db_file file;
		make_parentallock_list(filename, db.parental, file);
		this->e2db_out[filename] = file;

		filename = db.parental ? "whitelist" : "blacklist";
		e2db_file empty;
		empty.filename = filename;
		empty.mime = "text/plain";
		empty.size = 0;
		this->e2db_out[filename] = empty;
	}
}

void e2db_maker::make_bouquet(string bname, e2db_file& file)
{
	debug("make_bouquet", "bname", bname);

	bouquet bs = bouquets[bname];
	stringstream ss;

	ss << "#NAME " << bs.name << endl;
	for (string & w : bs.userbouquets)
	{
		ss << "#SERVICE ";
		ss << "1:7:" << bs.btype << ":0:0:0:0:0:0:0:";
		ss << "FROM BOUQUET ";
		ss << "\"" << w << "\" ";
		ss << "ORDER BY bouquet";
		ss << endl;
	}

	file.filename = bname;
	file.data = ss.str();
	file.mime = "text/plain";
	file.size = file.data.size();
}

void e2db_maker::make_bouquet_epl(string bname, e2db_file& file)
{
	debug("make_bouquet_epl", "bname", bname);

	bouquet bs = bouquets[bname];
	stringstream ss;

	ss << "#NAME " << bs.name << endl;
	for (string & w : bs.userbouquets)
	{
		size_t pos = w.find('.');
		size_t len = w.rfind('.');
		string name;
		string path = MAKER_BPATH + '/' + w;

		if (pos != string::npos && len != string::npos)
			name = w.substr(pos, len);

		ss << "#SERVICE: ";
		ss << "4097:7:0:" << name << ":0:0:0:0:0:0:";
		ss << path << endl;
		ss << "#TYPE 16385" << endl;
		ss << path << endl;
	}

	file.filename = bname;
	file.data = ss.str();
	file.mime = "text/plain";
	file.size = file.data.size();
}

void e2db_maker::make_userbouquet(string bname, e2db_file& file)
{
	debug("make_userbouquet", "bname", bname);

	userbouquet ub = userbouquets[bname];
	stringstream ss;

	ss << "#NAME " << ub.name << endl;
	for (auto & x : index[bname])
	{
		channel_reference chref = userbouquets[bname].channels[x.second];
		ss << "#SERVICE ";
		ss << "1:";
		ss << chref.atype << ':';
		ss << hex;
		ss << uppercase << chref.anum << ':';

		if (db.services.count(x.second))
		{
			service ch = db.services[x.second];

			ss << uppercase << ch.ssid << ':';
			ss << uppercase << ch.tsid << ':';
			ss << uppercase << ch.onid << ':';
			ss << uppercase << ch.dvbns << ':';
			ss << "0:0:0:";
		}
		else
		{
			if (chref.marker)
			{
				ss << "0:0:0:0:0:0:0:0:" << endl;
				ss << "#DESCRIPTION " << chref.value;
			}
			else
			{
				error("make_userbouquet", "Maker Error", "Missing channel_reference \"" + x.second + "\".");
			}
		}
		ss << dec;
		ss << endl;
	}

	file.filename = bname;
	file.data = ss.str();
	file.mime = "text/plain";
	file.size = file.data.size();
}

//TODO value xml entities
void e2db_maker::make_tunersets_xml(string filename, int ytype, e2db_file& file)
{
	debug("make_tunersets_xml", "ytype", ytype);

	switch (ytype)
	{
		case YTYPE::satellite:
		case YTYPE::terrestrial:
		case YTYPE::cable:
		case YTYPE::atsc:
		break;
		default:
		return error("make_tunersets_xml", "Maker Error", "These settings are not supported.");
	}

	tunersets tv = tuners[ytype];
	stringstream ss;

	string yname = "tns:";
	unordered_map<int, string> tags;
	switch (ytype)
	{
		case YTYPE::satellite:
			yname += 's';
			tags[0] = "satellites";
			tags[1] = "sat";
		break;
		case YTYPE::terrestrial:
			yname += 't';
			tags[0] = "locations";
			tags[1] = "terrestrial";
		break;
		case YTYPE::cable:
			yname += 'c';
			tags[0] = "cables";
			tags[1] = "cable";
		break;
		case YTYPE::atsc:
			yname += 'a';
			tags[0] = "locations";
			tags[1] = "atsc";
		break;
	}
	tags[2] = "transponder";

	ss << "<?xml version=\"1.0\" encoding=\"" << tv.charset << "\"?>" << endl;
	ss << '<' << tags[0] << '>' << endl;
	for (auto & x : index[yname])
	{
		tunersets_table tn = tv.tables[x.second];

		ss << "\t" << '<' << tags[1];
		if (! tn.name.empty())
			ss << ' ' << "name=\"" << tn.name << "\"";
		if (tn.feed != -1)
			ss << ' ' << "satfeed=\"" << (tn.feed ? "true" : "false") << "\"";
		if (tn.flgs != -1)
			ss << ' ' << "flags=\"" << tn.flgs << "\"";
		if (tn.pos != -1)
			ss << ' ' << "position=\"" << tn.pos << "\"";
		if (! tn.country.empty())
			ss << ' ' << "countrycode=\"" << tn.country << "\"";
		ss << '>' << endl;

		for (auto & x : index[tn.tnid])
		{
			tunersets_transponder tntxp = tn.transponders[x.second];

			ss << "\t\t" << '<' << tags[2];
			switch (tn.ytype)
			{
				case YTYPE::satellite:
					if (tntxp.freq != -1)
						ss << ' ' << "frequency=\"" << int (tntxp.freq * 1e3) << "\"";
					if (tntxp.sr != -1)
						ss << ' ' << "symbol_rate=\"" << int (tntxp.sr * 1e3) << "\"";
					if (tntxp.pol != -1)
						ss << ' ' << "polarization=\"" << tntxp.pol << "\"";
					if (tntxp.fec != -1)
						ss << ' ' << "fec_inner=\"" << tntxp.fec << "\"";
					if (tntxp.inv != -1)
						ss << ' ' << "inversion=\"" << tntxp.inv << "\"";
					if (tntxp.sys != -1)
						ss << ' ' << "system=\"" << tntxp.sys << "\"";
					if (tntxp.mod != -1)
						ss << ' ' << "modulation=\"" << tntxp.mod << "\"";
					if (tntxp.rol != -1)
						ss << ' ' << "rolloff=\"" << tntxp.rol << "\"";
					if (tntxp.pil != -1)
						ss << ' ' << "pilot=\"" << tntxp.pil << "\"";
					if (tntxp.isid != -1)
						ss << ' ' << "is_id=\"" << tntxp.isid << "\"";
					if (tntxp.mts != -1)
						ss << ' ' << "mts=\"" << tntxp.mts << "\"";
					if (tntxp.plsmode != -1)
						ss << ' ' << "pls_mode=\"" << tntxp.plsmode << "\"";
					if (tntxp.plscode != -1)
						ss << ' ' << "pls_code=\"" << tntxp.plscode << "\"";
					if (tntxp.plsn != -1)
						ss << ' ' << "plsn=\"" << tntxp.plsn << "\"";
				break;
				case YTYPE::terrestrial:
					if (tntxp.freq != -1)
						ss << ' ' << "centre_frequency=\"" << int (tntxp.freq * 1e3) << "\"";
					if (tntxp.band != -1)
						ss << ' ' << "bandwidth=\"" << tntxp.band << "\"";
					if (tntxp.hpfec != -1)
						ss << ' ' << "code_rate_hp=\"" << tntxp.hpfec << "\"";
					if (tntxp.lpfec != -1)
						ss << ' ' << "code_rate_lp=\"" << tntxp.lpfec << "\"";
					if (tntxp.inv != -1)
						ss << ' ' << "inversion=\"" << tntxp.inv << "\"";
					if (tntxp.sys != -1)
						ss << ' ' << "system=\"" << tntxp.sys << "\"";
					if (tntxp.tmod != -1)
						ss << ' ' << "constellation=\"" << tntxp.tmod << "\"";
					if (tntxp.tmx != -1)
						ss << ' ' << "transmission_mode=\"" << tntxp.tmx << "\"";
					if (tntxp.guard != -1)
						ss << ' ' << "guard_interval=\"" << tntxp.guard << "\"";
					if (tntxp.hier != -1)
						ss << ' ' << "hierarchy_information=\"" << tntxp.hier << "\"";
				break;
				case YTYPE::cable:
					if (tntxp.freq != -1)
						ss << ' ' << "frequency=\"" << int (tntxp.freq * 1e3) << "\"";
					if (tntxp.sr != -1)
						ss << ' ' << "symbol_rate=\"" << int (tntxp.sr * 1e3) << "\"";
					if (tntxp.cfec != -1)
						ss << ' ' << "fec_inner=\"" << tntxp.cfec << "\"";
					if (tntxp.inv != -1)
						ss << ' ' << "inversion=\"" << tntxp.inv << "\"";
					if (tntxp.cmod != -1)
						ss << ' ' << "modulation=\"" << tntxp.cmod << "\"";
				break;
				case YTYPE::atsc:
					if (tntxp.freq != -1)
						ss << ' ' << "frequency=\"" << int (tntxp.freq * 1e3) << "\"";
					if (tntxp.inv != -1)
						ss << ' ' << "inversion=\"" << tntxp.inv << "\"";
					if (tntxp.amod != -1)
						ss << ' ' << "modulation=\"" << tntxp.amod << "\"";
				break;
			}
			ss << '/' << '>' << endl;
		}

		ss << "\t" << '<' << '/' << tags[1] << '>' << endl;
	}
	ss << '<' << '/' << tags[0] << '>' << endl;

	string str = ss.str();
	if (comments.count(yname))
	{
		int i = 0;
		unsigned long pos = 0;
		for (auto & s : comments[yname])
		{
			string line;
			while (s.ln != i)
			{
				std::getline(ss, line, '>');
				pos += line.size() + 1;
				i++;
			}
			line = "<!--" + s.text + "-->";
			if (s.type) // multiline
				line = '\n' + line;
			str = str.substr(0, pos) + line + str.substr(pos);
			pos += line.size();
		}
	}

	file.filename = filename;
	file.data = str;
	file.mime = "text/xml";
	file.size = file.data.size();
}

void e2db_maker::make_parentallock_list(string filename, PARENTALLOCK ltype, e2db_file& file)
{
	debug("make_parentallock_list", "ltype", ltype);

	stringstream ss;

	if (ltype == PARENTALLOCK::locked)
	{
		ss << "Parentallocked Services" << endl;

		for (auto & x : userbouquets)
		{
			userbouquet& ub = x.second;

			if (ub.locked)
			{
				size_t pos = ub.bname.find(".");
				size_t len = ub.bname.rfind(".");
				string name;
				string path = MAKER_BPATH + '/' + ub.bname;

				if (pos != string::npos && len != string::npos)
					name = ub.bname.substr(pos, len);

				ss << "4097:7:0:" << name << ":0:0:0:0:0:0:";
				ss << path << endl;
			}
		}
	}

	for (auto & x : db.services)
	{
		service& ch = x.second;

		if (ch.locked)
		{
			ss << "1:0:1:";
			ss << hex;
			ss << uppercase << ch.ssid << ':';
			ss << uppercase << ch.tsid << ':';
			ss << uppercase << ch.onid << ':';
			ss << uppercase << ch.dvbns << ':';
			ss << "0:0:0:";
			ss << dec;
			ss << endl;
		}
	}

	file.filename = filename;
	file.data = ss.str();
	file.mime = "text/plain";
	file.size = file.data.size();
}

bool e2db_maker::push_file(string path)
{
	debug("push_file", "path", path);

	if (std::filesystem::is_directory(path)) //C++17
	{
		if (! OVERWRITE_FILE)
		{
			error("push_file", "File Error", "File \"" + path + "\" already exists.");
			return false;
		}
	}
	else
	{
		std::filesystem::create_directory(path); //C++17
	}
	if ((std::filesystem::status(path).permissions() & std::filesystem::perms::group_write)  == std::filesystem::perms::none) //C++17
	{
		error("push_file", "File Error", "File \"" + path + "\" is not writable.");
		return false;
	}
	for (auto & o: this->e2db_out)
	{
		string fpath = path + '/' + o.first;

		if (! OVERWRITE_FILE && std::filesystem::exists(fpath)) //C++17
		{
			error("push_file", "File Error", "File \"" + fpath + "\" already exists.");
			return false;
		}
		if ((std::filesystem::status(fpath).permissions() & std::filesystem::perms::group_write)  == std::filesystem::perms::none) //C++17
		{
			error("push_file", "File Error", "File \"" + fpath + "\" is not writable.");
			return false;
		}

		ofstream out (fpath);
		out << o.second.data;
		out.close();
	}

	return true;
}

bool e2db_maker::write(string path)
{
	debug("write", "filename", path);

	make_e2db();

	if (push_file(path))
		return true;
	else
		return false;
}

unordered_map<string, e2db_abstract::e2db_file> e2db_maker::get_output() {
	debug("get_output");

	make_e2db();

	return this->e2db_out;
}

}
