/*!
 * e2-sat-editor/src/e2db/e2db_maker.cpp
 * 
 * @link https://github.com/ctlcltd/e2-sat-editor
 * @copyright e2 SAT Editor Team
 * @author Leonardo Laureti
 * @version 0.5
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

	if (LAMEDB_VER == -1 && ZAPIT_VER != -1)
		LAMEDB_VER = 4;

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
			case YTYPE::cable: // DVB-C
				ss << int (tx.freq * 1e3);
				ss << ':' << int (tx.sr * 1e3);
				ss << ':' << tx.inv;
				ss << ':' << tx.cmod;
				ss << ':' << tx.cfec;
				if (! tx.oflgs.empty())
					ss << tx.oflgs;
			break;
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
		if (ver == 5)
			ss << ':' << ch.srcid;
		ss << formats[MAKER_FORMAT::b_service_params_separator];
		ss << formats[MAKER_FORMAT::b_service_param_escape] << ch.chname << formats[MAKER_FORMAT::b_service_param_escape];
		ss << formats[MAKER_FORMAT::b_service_params_separator];

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
	file.mime = "text/plain";
	file.data = ss.str();
	file.size = file.data.size();
}

void e2db_maker::make_e2db_bouquets()
{
	debug("make_e2db_bouquets");

	for (auto & x : bouquets)
	{
		string filename = x.first;
		string bname = x.first;

		if (LAMEDB_VER < 4 && filename.find(".epl") == string::npos)
		{
			bouquet bs = x.second;

			string ktype;
			if (bs.btype == STYPE::tv)
				ktype = "tv";
			else if (bs.btype == STYPE::radio)
				ktype = "radio";
			filename = "userbouquets." + ktype + ".epl";
		}

		e2db_file file;
		if (LAMEDB_VER < 4)
			make_bouquet_epl(bname, file);
		else
			make_bouquet(bname, file);
		this->e2db_out[filename] = file;
	}

	//TODO
	if (LAMEDB_VER < 4)
	{
		e2db_file empty;
		empty.data = "eDVB bouquets /2/\nbouquets\nend\n";
		empty.filename = "bouquets";
		empty.mime = "text/plain";
		empty.size = empty.data.size();
		this->e2db_out[empty.filename] = empty;
	}
}

void e2db_maker::make_e2db_userbouquets()
{
	debug("make_e2db_userbouquets");

	for (auto & x : userbouquets)
	{
		string filename = x.first;

		e2db_file file;
		make_userbouquet(filename, file);
		this->e2db_out[filename] = file;
	}
}

void e2db_maker::make_db_tunersets()
{
	debug("make_db_tunersets");

	for (auto & x : tuners)
	{
		if (ZAPIT_VER != -1 && x.first != YTYPE::satellite)
			continue;

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

void e2db_maker::make_zapit()
{
	debug("make_zapit");

	std::clock_t start = std::clock();

	if (ZAPIT_VER == -1 && LAMEDB_VER != -1)
		ZAPIT_VER = 4;

	make_zapit_services();
	make_zapit_bouquets();
	if (MAKER_TUNERSETS)
		make_db_tunersets();

	std::clock_t end = std::clock();

	info("make_zapit", "elapsed time", to_string(int (end - start)) + " ms.");
}

void e2db_maker::make_zapit_services()
{
	debug("make_zapit_services");

	switch (ZAPIT_VER)
	{
		case 1:
		case 2:
		case 3:
		case 4:
			make_zapit_services(ZAPIT_VER);
		break;
		default:
		return error("make_zapit_servies", "Maker Error", "Unknown services file format.");
	}
}

void e2db_maker::make_zapit_services(int ver)
{
	debug("make_zapit_services", "version", ver);

	e2db_file file;
	make_services_xml("services.xml", file, ver);
	this->e2db_out["services.xml"] = file;
}

void e2db_maker::make_zapit_bouquets()
{
	debug("make_zapit_bouquets");

	switch (ZAPIT_VER)
	{
		case 1:
		case 2:
		case 3:
		case 4:
			make_zapit_bouquets(ZAPIT_VER);
		break;
		default:
		return error("make_zapit_bouquets", "Maker Error", "Unknown bouquets file format.");
	}
}

void e2db_maker::make_zapit_bouquets(int ver)
{
	debug("make_zapit_bouquets", "version", ver);

	string filename;

	if (ver > 1)
		filename = "ubouquets.xml";
	else
		filename = "bouquets.xml";

	e2db_file file;
	make_bouquets_xml(filename, file, ver);
	this->e2db_out[filename] = file;

	if (ver > 1)
	{
		e2db_file empty;
		empty.data = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<zapit>\n</zapit>\n";
		empty.filename = "bouquets.xml";
		empty.mime = "text/xml";
		empty.size = empty.data.size();
		this->e2db_out[empty.filename] = empty;
	}
}

void e2db_maker::make_e2db_parentallock_list()
{
	debug("make_e2db_parentallock_list");

	if (LAMEDB_VER < 4)
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
		this->e2db_out[empty.filename] = empty;
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
	file.mime = "text/plain";
	file.data = ss.str();
	file.size = file.data.size();
}

void e2db_maker::make_bouquet_epl(string bname, e2db_file& file)
{
	debug("make_bouquet_epl", "bname", bname);

	string filename = bname;
	bouquet bs = bouquets[bname];

	if (bname.find(".epl") == string::npos)
	{
		string ktype;
		if (bs.btype == STYPE::tv)
			ktype = "tv";
		else if (bs.btype == STYPE::radio)
			ktype = "radio";
		filename = "userbouquets." + ktype + ".epl";
	}

	stringstream ss;

	ss << "#NAME " << bs.name << endl;
	for (string & w : bs.userbouquets)
	{
		size_t pos = w.find('.');
		size_t n = w.rfind('.');

		string name;
		string basedir = MAKER_BPATH;
		if (basedir.rfind('/') == string::npos)
			basedir.append("/");

		string path = basedir + w;

		if (pos != string::npos && n != string::npos)
		{
			name = w.substr(0, n);
			name = name.substr(pos + 1);
		}

		ss << "#SERVICE: ";
		ss << "4097:7:0:" << name << ":0:0:0:0:0:0:";
		ss << path << endl;
		ss << "#TYPE 16385" << endl;
		ss << path << endl;
	}

	file.filename = filename;
	file.mime = "text/plain";
	file.data = ss.str();
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
				error("make_userbouquet", "Maker Error", "Missing channel reference \"" + x.second + "\".");
			}
		}
		ss << dec;
		ss << endl;
	}

	file.filename = bname;
	file.mime = "text/plain";
	file.data = ss.str();
	file.size = file.data.size();
}

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

	string iname = "tns:";
	char yname;
	unordered_map<int, string> tags;
	switch (ytype)
	{
		case YTYPE::satellite:
			yname = 's';
			tags[0] = "satellites";
			tags[1] = "sat";
		break;
		case YTYPE::terrestrial:
			yname = 't';
			tags[0] = "locations";
			tags[1] = "terrestrial";
		break;
		case YTYPE::cable:
			yname = 'c';
			tags[0] = "cables";
			tags[1] = "cable";
		break;
		case YTYPE::atsc:
			yname = 'a';
			tags[0] = "locations";
			tags[1] = "atsc";
		break;
	}
	tags[2] = "transponder";
	iname += yname;

	ss << "<?xml version=\"1.0\" encoding=\"" << tv.charset << "\"?>" << endl;
	ss << '<' << tags[0] << '>' << endl;

	for (auto & x : index[iname])
	{
		tunersets_table tn = tv.tables[x.second];

		ss << "\t" << '<' << tags[1];
		if (! tn.name.empty())
			ss << ' ' << "name=\"" << conv_xml_value(tn.name) << "\"";
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

	if (comments.count(iname))
	{
		int i = 0;
		size_t pos = 0;
		for (auto & s : comments[iname])
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
	file.mime = "text/xml";
	file.data = str;
	file.size = file.data.size();
}

void e2db_maker::make_services_xml(string filename, e2db_file& file, int ver)
{
	debug("make_services_xml", "filename", filename);

	string dname;

	if (datas.count("services.xml"))
		dname = "services.xml";

	if (dname.empty())
	{
		datasets dat;
		dat.dname = dname = "services.xml";
		dat.itype = 0;
		dat.charset = "UTF-8";

		datas.emplace(dat.dname, dat);
	}
	if (db.tables.size() == 0)
	{
		for (auto & x : index["txs"])
		{
			transponder tx = db.transponders[x.second];
			table& tr = db.tables[tx.pos];
			tr.pos = tx.pos;

			if (tuners_pos.count(tx.pos))
			{
				string tnid = tuners_pos.at(tx.pos);
				tunersets_table tn = tuners[0].tables[tnid];
				tr.name = tn.name;
			}
			else
			{
				tr.name = tx.pos == -1 ? "NaN" : value_transponder_position(tx.pos);
			}

			tr.transponders.emplace_back(tx.txid);

			db.tables[tx.pos] = tr;
		}

		int idx = 0;
		for (auto & x : db.tables)
		{
			idx++;
			table& tr = x.second;
			tr.index = idx;

			db.tables[tr.pos] = tr;
			index["trs"].emplace_back(pair (tr.index, to_string(tr.pos))); //C++17
		}
	}

	datasets dat = datas[dname];
	stringstream ss;

	string iname = dname;
	unordered_map<int, string> tags;
	tags[0] = "zapit";
	tags[1] = "sat";
	if (ver > 1)
	{
		tags[2] = "TS";
		tags[3] = "S";
	}
	else
	{
		tags[2] = "transponder";
		tags[3] = "channel";
	}

	unordered_map<string, int> txs;

	ss << "<?xml version=\"1.0\" encoding=\"" << dat.charset << "\"?>" << endl;
	ss << '<' << tags[0];
	if (ver > 1)
		ss << ' ' << "api=\"" << ver << "\"";
	ss << '>' << endl;

	for (auto & x : index["trs"])
	{
		int pos = std::stoi(x.second);
		table tr = db.tables[pos];

		ss << "\t" << '<' << tags[1];
		ss << ' ' << "name=\"" << conv_xml_value(tr.name) << "\"";
		if (ver > 2)
		{
			ss << ' ' << "position=\"" << (tr.pos != -1 ? tr.pos : 0) << "\"";
			ss << ' ' << "diseqc=\"" << (tr.diseqc != -1 ? tr.diseqc : 0) << "\"";
			ss << ' ' << "uncommited=\"" << (tr.uncomtd != -1 ? tr.uncomtd : 0) << "\"";
		}
		else
		{
			ss << ' ' << "diseqc=\"" << (tr.diseqc != -1 ? tr.diseqc : 0) << "\"";
			ss << ' ' << "position=\"" << (tr.pos != -1 ? tr.pos : 0) << "\"";
		}
		ss << '>' << endl;

		for (auto & w : tr.transponders)
		{
			transponder tx = db.transponders[w];

			ss << "\t\t" << '<' << tags[2];
			if (ver > 1)
			{
				ss << ' ' << "id=\"" << hex << setfill('0') << setw(4) << tx.tsid << dec << "\"";
				ss << ' ' << "on=\"" << hex << setfill('0') << setw(4) << tx.onid << dec << "\"";
				ss << ' ' << "frq=\"" << int (tx.freq * 1e3) << "\"";
				ss << ' ' << "inv=\"" << (ver < 3 && tx.inv != 2 ? tx.inv : 0) << "\"";
				ss << ' ' << "sr=\"" << int (tx.sr * 1e3) << "\"";
				{
					int i = 0;
					if (ver == 4)
					{
						if (tx.fec == 0)
							i = 9;
						else if (tx.fec < 4)
							i = tx.fec;
						else if (tx.fec == 4)
							i = 5;
						else if (tx.fec == 5)
							i = 7;
						else if (tx.fec == 6)
							i = 8;
						else if (tx.fec == 7)
							i = 10;
						else if (tx.fec == 8)
							i = 4;
						else if (tx.fec == 9)
							i = 11;
						else if (tx.fec == 10)
							i = 6;
					}
					else if (ver == 3)
					{
						if (tx.fec < 4)
							i = tx.fec;
						else if (i == 4)
							i = 5;
						else if (i == 5)
							i = 7;
					}
					else if (ver == 2)
					{
						if (i < 6)
							i = tx.fec;
					}
					ss << ' ' << "fec=\"" << i << "\"";
				}
				ss << ' ' << "pol=\"" << tx.pol << "\"";
				if (ver > 3)
				{
					ss << ' ' << "mod=\"" << (tx.mod != -1 ? tx.mod : 0) << "\"";
					ss << ' ' << "sys=\"" << (tx.sys != -1 ? tx.sys : 0) << "\"";
				}
			}
			else
			{
				ss << ' ' << "id=\"" << hex << setfill('0') << setw(4) << tx.tsid << dec << "\"";
				ss << ' ' << "onid=\"" << hex << setfill('0') << setw(4) << tx.onid << dec << "\"";
				ss << ' ' << "frequency=\"" << int (tx.freq * 1e3) << "\"";
				ss << ' ' << "inversion=\"" << (tx.inv != 2 ? tx.inv : 0) << "\"";
				ss << ' ' << "symbol_rate=\"" << int (tx.sr * 1e3) << "\"";
				{
					int i = 0;
					if (tx.fec == 0)
						i = 9;
					else if (tx.fec < 4)
						i = tx.fec;
					else if (tx.fec == 4)
						i = 5;
					else if (tx.fec == 5)
						i = 7;
					else if (tx.fec == 6)
						i = 8;
					else if (tx.fec == 8)
						i = 4;
					else if (tx.fec == 10)
						i = 6;
					ss << ' ' << "fec_inner=\"" << i << "\"";
				}
				ss << ' ' << "polarization=\"" << tx.pol << "\"";
			}

			ss << '>' << endl;

			if (! txs[tx.txid])
			{
				for (auto & x : index["chs"])
				{
					service ch = db.services[x.second];

					if (ch.txid != tx.txid)
						continue;

					ss << "\t\t\t" << '<' << tags[3];
					if (ver > 1)
					{
						ss << ' ' << "i=\"" << hex << setfill('0') << setw(4) << ch.ssid << dec << "\"";
						ss << ' ' << "n=\"" << conv_xml_value(ch.chname) << "\"";
						{
							int cval = 0;
							string cpx = (SDATA_PIDS::vpid > 9 ? "" : "0") + to_string(SDATA_PIDS::vpid);
							for (string & w : ch.data[SDATA::c])
								if (w.substr(0, 2) == cpx)
									cval = int (std::strtol(w.substr(2).data(), NULL, 16));
							ss << ' ' << "v=\"" << hex << cval << dec << "\"";
						}
						{
							int cval = 0;
							string cpx = (SDATA_PIDS::mpegapid > 9 ? "" : "0") + to_string(SDATA_PIDS::mpegapid);
							for (string & w : ch.data[SDATA::c])
								if (w.substr(0, 2) == cpx)
									cval = int (std::strtol(w.substr(2).data(), NULL, 16));
							ss << ' ' << "a=\"" << hex << cval << dec << "\"";
						}
						{
							int cval = 0;
							string cpx = (SDATA_PIDS::pcrpid > 9 ? "" : "0") + to_string(SDATA_PIDS::pcrpid);
							for (string & w : ch.data[SDATA::c])
								if (w.substr(0, 2) == cpx)
									cval = int (std::strtol(w.substr(2).data(), NULL, 16));
							ss << ' ' << "p=\"" << hex << cval << dec << "\"";
						}
						{
							int cval = 0;
							string cpx = (SDATA_PIDS::pmt > 9 ? "" : "0") + to_string(SDATA_PIDS::pmt);
							for (string & w : ch.data[SDATA::c])
								if (w.substr(0, 2) == cpx)
									cval = int (std::strtol(w.substr(2).data(), NULL, 16));
							ss << ' ' << "pmt=\"" << hex << cval << dec << "\"";
						}
						{
							int cval = 0;
							string cpx = (SDATA_PIDS::tpid > 9 ? "" : "0") + to_string(SDATA_PIDS::tpid);
							for (string & w : ch.data[SDATA::c])
								if (w.substr(0, 2) == cpx)
									cval = int (std::strtol(w.substr(2).data(), NULL, 16));
							ss << ' ' << "tx=\"" << hex << cval << dec << "\"";
						}
						ss << ' ' << "t=\"" << hex << ch.stype << dec << "\"";
						if (ver > 2)
						{
							{
								int cval = 0;
								string cpx = (SDATA_PIDS::vtype > 9 ? "" : "0") + to_string(SDATA_PIDS::vtype);
								for (string & w : ch.data[SDATA::c])
									if (w.substr(0, 2) == cpx)
										cval = int (std::strtol(w.substr(2).data(), NULL, 16));
								ss << ' ' << "vt=\"" << hex << cval << dec << "\"";
							}
							ss << ' ' << "s=\"" << (ch.data[SDATA::C].empty() ? 0 : 1) << "\"";
							ss << ' ' << "num=\"" << ch.snum << "\"";
							//TODO
							ss << ' ' << "f=\"" << 0 << "\"";
						}
					}
					else
					{
						ss << ' ' << "service_id=\"" << hex << setfill('0') << setw(4) << ch.ssid << dec << "\"";
						ss << ' ' << "name=\"" << conv_xml_value(ch.chname) << "\"";
						ss << ' ' << "service_type=\"" << hex << setfill('0') << setw(4) << ch.stype << dec << "\"";
					}

					ss << '/' << '>' << endl;
				}
			}

			ss << "\t\t" << '<' << '/' << tags[2] << '>' << endl;

			txs[tx.txid]++;
		}

		ss << "\t" << '<' << '/' << tags[1] << '>' << endl;

		txs.clear();
	}
	ss << '<' << '/' << tags[0] << '>' << endl;

	string str = ss.str();

	if (comments.count(iname))
	{
		int i = 0;
		size_t pos = 0;
		for (auto & s : comments[iname])
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
	else
	{
		string editor = editor_string();
		string timestamp = editor_timestamp();

		str += "<!-- Editor: " + editor + " -->\n";
		str += "<!-- Datetime: " + timestamp + " -->\n";
	}

	file.filename = filename;
	file.mime = "text/xml";
	file.data = str;
	file.size = file.data.size();
}

void e2db_maker::make_bouquets_xml(string filename, e2db_file& file, int ver)
{
	debug("make_bouquets_xml", "filename", filename);

	string dname;

	if (datas.count("ubouquets.xml"))
		dname = "ubouquets.xml";
	else if (datas.count("ubouquets.xml"))
		dname = "bouquets.xml";

	if (dname.empty())
	{
		datasets dat;
		dat.dname = dname = ver > 1 ? "ubouquets.xml" : "bouquets.xml";
		dat.itype = 1;
		dat.charset = "UTF-8";

		datas.emplace(dat.dname, dat);
	}

	datasets dat = datas[dname];
	stringstream ss;

	string iname = dname;
	unordered_map<int, string> tags;
	tags[0] = "zapit";
	tags[1] = "Bouquet";
	if (ver > 1)
	{
		tags[2] = "S";
	}
	else
	{
		tags[2] = "channel";
	}

	ss << "<?xml version=\"1.0\" encoding=\"" << dat.charset << "\"?>" << endl;
	ss << '<' << tags[0] << '>' << endl;

	vector<string> ubindex;
	for (auto & x : index["bss"])
	{
		bouquet bs = bouquets[x.second];
		ubindex.insert(ubindex.end(), bs.userbouquets.begin(), bs.userbouquets.end());
	}

	for (auto & w : ubindex)
	{
		userbouquet ub = userbouquets[w];

		ss << "\t" << '<' << tags[1];
		ss << ' ' << "name=\"" << conv_xml_value(ub.name) << "\"";
		ss << ' ' << "hidden=\"" << ub.hidden << "\"";
		ss << ' ' << "locked=\"" << ub.locked << "\"";
		ss << '>' << endl;

		for (auto & x : index[ub.bname])
		{
			channel_reference chref = userbouquets[ub.bname].channels[x.second];

			if (db.services.count(x.second))
			{
				service ch = db.services[x.second];
				transponder tx = db.transponders[ch.txid];

				ss << "\t\t" << '<' << tags[2];
				if (ver > 1)
				{
					ss << ' ' << "i=\"" << hex << ch.ssid << dec << "\"";
					ss << ' ' << "n=\"" << conv_xml_value(ch.chname) << "\"";
					ss << ' ' << "t=\"" << hex << setfill('0') << setw(4) << ch.tsid << dec << "\"";
					ss << ' ' << "on=\"" << hex << ch.onid << dec << "\"";
					ss << ' ' << "s=\"" << tx.pos << "\"";
					ss << ' ' << "frq=\"" << tx.freq << "\"";
					if (ver > 3)
					{
						ss << ' ' << "l=\"" << ch.locked << "\"";
					}
				}
				else
				{
					ss << ' ' << "serviceID=\"" << hex << setfill('0') << setw(4) << ch.ssid << dec << "\"";
					ss << ' ' << "name=\"" << conv_xml_value(ch.chname) << "\"";
					ss << ' ' << "tsid=\"" << hex << setfill('0') << setw(4) << ch.tsid << dec << "\"";
					ss << ' ' << "onid=\"" << hex << setfill('0') << setw(4) << ch.onid << dec << "\"";
					ss << ' ' << "sat_position=\"" << tx.pos << "\"";
				}

				ss << '/' << '>' << endl;
			}
			else
			{
				if (! chref.marker)
				{
					error("make_bouquets_xml", "Maker Error", "Missing channel reference \"" + x.second + "\".");
				}
			}
		}

		ss << "\t" << '<' << '/' << tags[1] << '>' << endl;
	}
	ss << '<' << '/' << tags[0] << '>' << endl;

	string str = ss.str();
	if (comments.count(iname))
	{
		int i = 0;
		size_t pos = 0;
		for (auto & s : comments[iname])
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
	file.mime = "text/xml";
	file.data = str;
	file.size = file.data.size();
}

string e2db_maker::conv_xml_value(string str)
{
	unordered_map<char, string> xmlents = {
		{'&', "&amp;"},
		{'"', "&quot;"},
		{'\'', "&apos;"},
		{'<', "&lt;"},
		{'>', "&gt;"}
	};

	size_t n = 0;

	while (n != str.size())
	{
		if (xmlents.count(str[n]))
		{
			string w = xmlents.at(str[n]);
			str = str.substr(0, n) + w + str.substr(n + 1);
			n += w.size() - 1;
		}
		n++;
	}

	return str;
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
			userbouquet ub = x.second;
			bool locked = db.parental == PARENTALLOCK::whitelist ? ! ub.locked : ub.locked;

			if (locked)
			{
				size_t pos = ub.bname.find(".");
				size_t n = ub.bname.rfind(".");
				string name;
				string path = MAKER_BPATH + '/' + ub.bname;

				if (pos != string::npos && n != string::npos)
					name = ub.bname.substr(pos, n);

				ss << "4097:7:0:" << name << ":0:0:0:0:0:0:";
				ss << path << endl;
			}
		}
	}
	else
	{
		for (auto & x : userbouquets)
		{
			userbouquet ub = x.second;
			bool locked = db.parental == PARENTALLOCK::whitelist ? ! ub.locked : ub.locked;

			if (locked)
			{
				string bname = ub.bname;

				for (auto & x : index[bname])
				{
					channel_reference chref = ub.channels[x.second];

					if (db.services.count(x.second))
					{
						service ch = db.services[x.second];

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
			}
		}
	}

	for (auto & x : db.services)
	{
		service ch = x.second;
		bool locked = db.parental == PARENTALLOCK::whitelist ? ! ch.locked : ch.locked;

		if (locked)
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
	file.mime = "text/plain";
	file.data = ss.str();
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
	if
	(
		(std::filesystem::status(path).permissions() & std::filesystem::perms::owner_write) == std::filesystem::perms::none &&
		(std::filesystem::status(path).permissions() & std::filesystem::perms::group_write) == std::filesystem::perms::none
	) //C++17
	{
		error("push_file", "File Error", "File \"" + path + "\" is not writable.");
		return false;
	}
	for (auto & o: this->e2db_out)
	{
		if (path.rfind('/') == string::npos)
			path.append("/");

		string fpath = path + o.first;

		if (! OVERWRITE_FILE && std::filesystem::exists(fpath)) //C++17
		{
			error("push_file", "File Error", "File \"" + fpath + "\" already exists.");
			return false;
		}
		if
		(
			(std::filesystem::status(fpath).permissions() & std::filesystem::perms::owner_write) == std::filesystem::perms::none &&
			(std::filesystem::status(fpath).permissions() & std::filesystem::perms::group_write) == std::filesystem::perms::none
		) //C++17
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

	if (LAMEDB_VER != -1)
		make_e2db();
	else if (ZAPIT_VER != -1)
		make_zapit();
	else
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
