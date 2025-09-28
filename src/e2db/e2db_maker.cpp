/*!
 * e2-sat-editor/src/e2db/e2db_maker.cpp
 * 
 * @link https://github.com/ctlcltd/e2-sat-editor
 * @copyright e2 SAT Editor Team
 * @author Leonardo Laureti
 * @version 1.8.1
 * @license MIT License
 * @license GNU GPLv3 License
 */

#include <cstdio>
#include <clocale>
#include <algorithm>
#include <chrono>
#include <sstream>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <filesystem>
#include <stdexcept>

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

	auto t_start = std::chrono::high_resolution_clock::now();

	if (LAMEDB_VER == -1 && ZAPIT_VER != -1)
		LAMEDB_VER = db.version - 0x1220;

	make_e2db_lamedb();
	make_e2db_bouquets();
	make_e2db_userbouquets();
	if (MAKER_TUNERSETS)
		make_db_tunersets();
	if (MAKER_PARENTALLOCK_LIST)
		make_e2db_parentallock_list();

	auto t_end = std::chrono::high_resolution_clock::now();
	int elapsed = std::chrono::duration<double, std::micro>(t_end - t_start).count();

	info("make_e2db", "elapsed time", to_string(elapsed) + " μs");
}

void e2db_maker::make_e2db_lamedb()
{
	switch (LAMEDB_VER)
	{
		case 2:
			make_e2db_lamedb("services", 2);
		break;
		case 3:
		case 4:
			make_e2db_lamedb("lamedb", LAMEDB_VER);
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
			case YTYPE::satellite: // DVB-S / DVB-S2
				ss << int (tx.freq * 1e3);
				ss << ':' << (tx.sr != -1 ? int (tx.sr * 1e3) : 0);
				ss << ':' << (tx.pol != -1 ? tx.pol : 0);
				ss << ':' << (tx.fec != -1 ? tx.fec : 0);
				ss << ':' << (tx.pos != -1 ? tx.pos : 0);
				ss << ':' << (tx.inv != -1 ? tx.inv : 0);

				if (ver > 3)
				{
					ss << ':' << (tx.flags != -1 ? tx.flags : 0);
				}

				if (ver >= 3)
				{
					if (tx.sys != -1 || tx.mod != -1 || tx.rol != -1 || (ver > 3 && (tx.mispls || tx.t2mi)))
					{
						ss << ':' << (tx.sys != -1 ? tx.sys : 0);
						ss << ':' << (tx.mod != -1 ? tx.mod : 0);
						ss << ':' << (tx.rol != -1 ? tx.mod : 0);
					}

					if (ver > 3)
					{
						if (tx.pil != -1 || (tx.mispls || tx.t2mi))
						{
							ss << ':' << (tx.pil != -1 ? tx.pil : 0);
						}

						if (tx.mispls || tx.t2mi)
						{
							if (ver == 5 && tx.optsverb)
							{
								if (tx.mispls)
								{
									ss << ',' << "MIS/PLS:";
									ss << ':' << (tx.isid != -1 ? tx.isid : 0);
									ss << ':' << (tx.plscode != -1 ? tx.plscode : 0);
									ss << ':' << (tx.plsmode != -1 ? tx.plsmode : 0);
								}
								if (tx.t2mi)
								{
									ss << ',' << "T2MI:";
									ss << ':' << (tx.t2mi_plpid != -1 ? tx.t2mi_plpid : 0);
									ss << ':' << (tx.t2mi_pid != -1 ? tx.t2mi_pid : 0);
								}
							}
							else
							{
								ss << ':' << (tx.isid != -1 ? tx.isid : 0);
								ss << ':' << (tx.plscode != -1 ? tx.plscode : 0);
								ss << ':' << (tx.plsmode != -1 ? tx.plsmode : 0);
								ss << ':' << (tx.t2mi_plpid != -1 ? tx.t2mi_plpid : 0);
								ss << ':' << (tx.t2mi_pid != -1 ? tx.t2mi_pid : 0);
							}
						}
					}
				}
			break;
			case YTYPE::terrestrial: // DVB-T / DVB-T2
				ss << int (tx.freq * 1e3);
				ss << ':' << (tx.band != -1 ? tx.band : 3);
				ss << ':' << (tx.hpfec != -1 ? tx.hpfec : 5);
				ss << ':' << (tx.lpfec != -1 ? tx.lpfec : 5);
				ss << ':' << (tx.tmod != -1 ? tx.tmod : 3);
				ss << ':' << (tx.tmx != -1 ? tx.tmx : 2);
				ss << ':' << (tx.guard != -1 ? tx.guard : 4);
				ss << ':' << (tx.hier != -1 ? tx.hier : 4);
				ss << ':' << (tx.inv != -1 ? tx.inv : 0);

				if (ver > 3)
				{
					ss << ':' << (tx.flags != -1 ? tx.flags : 0);
					ss << ':' << (tx.sys != -1 ? tx.sys : 0);
					ss << ':' << (tx.plpid != -1 ? tx.plpid : 0);
				}
			break;
			case YTYPE::cable: // DVB-C
				ss << int (tx.freq * 1e3);
				ss << ':' << (tx.sr != -1 ? int (tx.sr * 1e3) : 0);
				ss << ':' << (tx.inv != -1 ? tx.inv : 0);
				ss << ':' << (tx.cmod != -1 ? tx.cmod : 0);
				ss << ':' << (tx.cfec != -1 ? tx.cfec : 0);

				if (ver > 3)
				{
					ss << ':' << (tx.flags != -1 ? tx.flags : 0);
					ss << ':' << (tx.sys != -1 ? tx.sys : 0);
				}
			break;
			case YTYPE::atsc: // ATSC / DVB-C ANNEX B
				ss << int (tx.freq * 1e3);
				ss << ':' << (tx.inv != -1 ? tx.inv : 0);
				ss << ':' << (tx.amod != -1 ? tx.amod : 0);

				if (ver > 3)
				{
					ss << ':' << (tx.flags != -1 ? tx.flags : 0);
					ss << ':' << (tx.sys != -1 ? tx.sys : 0);
				}
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

		if (ch.data.empty())
			ch.data = {{'p', {""}}};

		auto z = std::prev(ch.data.cend());
		for (auto & q : ch.data)
		{
			char d;
			switch (q.first)
			{
				case SDATA::p: d = 'p'; break;
				case SDATA::c: d = 'c'; if (ch.data[SDATA::c].empty()) continue; break;
				case SDATA::C: d = 'C'; if (ch.data[SDATA::C].empty()) continue; break;
				case SDATA::f: d = 'f'; if (ch.data[SDATA::f].empty()) continue; break;
				default: continue;
			}

			for (size_t i = 0; i < q.second.size(); i++)
			{
				ss << d << ':' << q.second[i];
				if (! q.second[i].empty() && (i != q.second.size() - 1 || q.first != (*z).first))
					ss << ',';
			}
		}

		ss << formats[MAKER_FORMAT::b_service_endline];
	}
	ss << formats[MAKER_FORMAT::b_section_end];

	ss << formats[MAKER_FORMAT::b_comment] << "editor: " << editor_string() << endl;
	ss << formats[MAKER_FORMAT::b_comment] << "datetime: " << editor_timestamp() << endl;

	file.origin = FORG::filesys;
	file.filename = filename;
	file.mime = "text/plain";
	file.data = ss.str();
	file.size = file.data.size();
}

void e2db_maker::make_e2db_bouquets()
{
	make_e2db_bouquets(LAMEDB_VER);
}

void e2db_maker::make_e2db_bouquets(int ver)
{
	debug("make_e2db_bouquets", "version", ver);

	for (auto & x : index["bss"])
	{
		bouquet bs = bouquets[x.second];

		string filename = bs.rname.empty() ? bs.bname : bs.rname;
		string bname = bs.bname;

		e2db_file file;

		if (ver < 3 || filename.rfind(".epl") != string::npos)
			make_bouquet_epl(bname, file, ver);
		else
			make_bouquet(bname, file, ver);

		filename = file.filename;

		this->e2db_out[filename] = file;
	}

	//TODO bouquets file
	if (ver < 3)
	{
		e2db_file file;
		file.origin = FORG::filesys;
		file.data = "eDVB bouquets /2/\nbouquets\nend\n";
		file.filename = "bouquets";
		file.mime = "text/plain";
		file.size = file.data.size();

		this->e2db_out[file.filename] = file;
	}
}

void e2db_maker::make_e2db_userbouquets()
{
	make_e2db_userbouquets(LAMEDB_VER);
}

void e2db_maker::make_e2db_userbouquets(int ver)
{
	debug("make_e2db_userbouquets", "version", ver);

	this->marker_count = 0;

	for (auto & x : index["ubs"])
	{
		userbouquet ub = userbouquets[x.second];

		string filename = ub.rname.empty() ? ub.bname : ub.rname;
		string bname = ub.bname;

		e2db_file file;

		make_userbouquet(bname, file, ver);

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

	auto t_start = std::chrono::high_resolution_clock::now();

	if (ZAPIT_VER == -1 && LAMEDB_VER != -1)
		ZAPIT_VER = db.version - 0x1010;

	make_zapit_services();
	make_zapit_bouquets();
	if (MAKER_TUNERSETS)
		make_db_tunersets();

	auto t_end = std::chrono::high_resolution_clock::now();
	int elapsed = std::chrono::duration<double, std::micro>(t_end - t_start).count();

	info("make_zapit", "elapsed time", to_string(elapsed) + " μs");
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
		e2db_file file;
		file.origin = FORG::filesys;
		file.data = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<zapit>\n</zapit>\n";
		file.filename = "bouquets.xml";
		file.mime = "text/xml";
		file.size = file.data.size();

		this->e2db_out[file.filename] = file;
	}
}

void e2db_maker::make_e2db_parentallock_list()
{
	debug("make_e2db_parentallock_list");

	if (LAMEDB_VER < 3)
	{
		e2db_file file;

		make_parentallock_list("services.locked", PARENTALLOCK::locked, file);

		this->e2db_out["services.locked"] = file;
	}
	else
	{
		{
			string filename = db.parental ? "whitelist" : "blacklist";
			e2db_file file;

			make_parentallock_list(filename, db.parental, file);

			this->e2db_out[filename] = file;
		}
		{
			string filename = db.parental ? "blacklist" : "whitelist";
			e2db_file file;
			file.origin = FORG::filesys;
			file.filename = filename;
			file.mime = "text/plain";
			file.size = 0;

			this->e2db_out[file.filename] = file;
		}
	}
}

void e2db_maker::make_bouquet(string bname, e2db_file& file, int ver)
{
	debug("make_bouquet", "bname", bname);

	bool compat = ver == 5 && ! MAKER_COMPAT_LAMEDB5;

	bouquet bs = bouquets[bname];

	string filename = bs.rname.empty() ? bs.bname : bs.rname;

	stringstream ss;

	ss << "#NAME " << bs.name << endl;

	for (string & bname : bs.userbouquets)
	{
		userbouquet ub = userbouquets[bname];

		ss << "#SERVICE";
		if (ver > 4 && ! compat)
			ss << ':';
		ss << ' ';
		if (ub.sref.empty())
		{
 			ss << 1 << ':';
 			ss << ub.utype << ':';
 			ss << bs.btype << ':';
 			ss << "0:0:0:0:0:0:0:";
		}
		else if (! ub.bname.empty())
		{
			if (ver < 5 || compat || (! ub.order.empty() && ub.order != "bouquet"))
			{
				{
					int x0 = 0, x1 = 0, x2 = 0, x3 = 0, x4 = 0, x5 = 0, x6 = 0, x7 = 0, x8 = 0, x9 = 0;

					std::sscanf(ub.sref.c_str(), "%d:%d:%d:%d:%d:%d:%d:%d:%d:%d:", &x0, &x1, &x2, &x3, &x4, &x5, &x6, &x7, &x8, &x9);

					x1 = ub.utype;

					ss << x0 << ':';
					ss << x1 << ':';
					ss << x2 << ':';
					ss << x3 << ':';
					ss << x4 << ':';
					ss << x5 << ':';
					ss << x6 << ':';
					ss << x7 << ':';
					ss << x8 << ':';
					ss << x9 << ':';
				}
				ss << "FROM BOUQUET ";
				ss << "\"" << ub.bname << "\" ";
				ss << "ORDER BY ";
				if (ub.order.empty())
					ss << "bouquet";
				else
					ss << ub.order;
			}
			else
			{
	 			ss << 1 << ':';
	 			ss << ub.utype << ':';
	 			ss << bs.btype << ':';
	 			ss << "0:0:0:0:0:0:0:";
	 			ss << ub.bname;
			}
		}
		else
		{
			ss << ub.sref;
		}
		ss << endl;
	}

	file.origin = FORG::filesys;
	file.filename = filename;
	file.mime = "text/plain";
	file.data = ss.str();
	file.size = file.data.size();
}

void e2db_maker::make_bouquet_epl(string bname, e2db_file& file, int ver)
{
	debug("make_bouquet_epl", "bname", bname);

	bouquet bs = bouquets[bname];

	string filename = bs.rname.empty() ? bs.bname : bs.rname;

	if (filename.rfind(".epl") == string::npos)
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
		userbouquet ub = userbouquets[w];
		string ub_bname = ub.bname;

		size_t pos = ub_bname.find('.');
		size_t n = ub_bname.rfind('.');

		string name;
		string basedir = MAKER_BPATH;

		if (basedir.size() && basedir[basedir.size() - 1] != '/')
			basedir.append("/");

		string path = basedir + ub_bname;

		if (pos != string::npos && n != string::npos)
		{
			name = ub_bname.substr(0, n);
			name = name.substr(pos + 1);
		}

		ss << "#SERVICE: ";
		ss << "4097:7:0:" << name << ":0:0:0:0:0:0:";
		ss << path << '\n';
		ss << "#TYPE 16385" << '\n';
		ss << path << endl;
	}

	file.origin = FORG::filesys;
	file.filename = filename;
	file.mime = "text/plain";
	file.data = ss.str();
	file.size = file.data.size();
}

void e2db_maker::make_userbouquet(string bname, e2db_file& file, bool exact_marker_index, int ver)
{
	if (exact_marker_index)
	{
		this->marker_count = 0;

		for (auto & x : index["ubs"])
		{
			userbouquet ub = userbouquets[x.second];
			string _bname = ub.bname;

			for (auto & x : index[_bname])
			{
				channel_reference chref = userbouquets[bname].channels[x.second];

				if (chref.marker)
					this->marker_count++;
			}

			if (_bname == bname)
				break;
		}
	}

	make_userbouquet(bname, file, ver);
}

void e2db_maker::make_userbouquet(string bname, e2db_file& file, int ver)
{
	debug("make_userbouquet", "bname", bname);

	userbouquet ub = userbouquets[bname];

	string filename = ub.rname.empty() ? ub.bname : ub.rname;

	stringstream ss;
	int ln = 0;

	ss << "#NAME " << ub.name << endl;

	for (auto & x : index[bname])
	{
		ln++;

		channel_reference chref = userbouquets[bname].channels[x.second];

		ss << dec;
		ss << "#SERVICE";
		if (ver < 3)
			ss << ':';
		ss << ' ';
		ss << chref.etype << ':';
		ss << chref.atype << ':';

		if (chref.marker)
		{
			ss << hex;
			if (MARKER_GLOBAL_INDEX)
				ss << uppercase << this->marker_count + 1 << ':';
			else
				ss << uppercase << chref.anum << ':';
			ss << dec;
			ss << "0:0:0:0:0:0:0:";

			if (! chref.value.empty())
			{
				if (chref.inlineval)
				{
					ss << ':';
					ss << conv_uri_value(chref.value);
				}
				if (chref.descrval)
				{
					ln++;

					ss << '\n';
					ss << "#DESCRIPTION";
					if (ver < 3)
						ss << ':';
					ss << ' ' << chref.value;
				}
			}

			this->marker_count++;
		}
		else if (db.services.count(x.second))
		{
			service ch = db.services[x.second];

			ss << hex;
			ss << uppercase << ch.stype << ':';
			ss << uppercase << ch.ssid << ':';
			ss << uppercase << ch.tsid << ':';
			ss << uppercase << ch.onid << ':';
			ss << uppercase << ch.dvbns << ':';
			ss << dec;
			ss << chref.x7 << ':';
			ss << chref.x8 << ':';
			ss << chref.x9 << ':';

			if (! chref.marker && ! chref.value.empty())
			{
				if (chref.inlineval)
				{
					ss << conv_uri_value(chref.value);
				}
				if (chref.descrval)
				{
					ln++;

					ss << '\n';
					ss << "#DESCRIPTION";
					if (ver < 3)
						ss << ':';
					ss << ' ' << chref.value;
				}
			}
		}
		else
		{
			ss << hex;
			ss << uppercase << chref.anum << ':';
			ss << uppercase << chref.ref.ssid << ':';
			ss << uppercase << chref.ref.tsid << ':';
			ss << uppercase << chref.ref.onid << ':';
			ss << uppercase << chref.ref.dvbns << ':';
			ss << dec;
			ss << chref.x7 << ':';
			ss << chref.x8 << ':';
			ss << chref.x9 << ':';

			if (! chref.url.empty())
			{
				ss << conv_uri_value(chref.url);
			}
			if (! chref.value.empty())
			{
				if (chref.inlineval)
				{
					ss << ':';
					ss << conv_uri_value(chref.value);
				}
				if (chref.descrval)
				{
					ln++;

					ss << '\n';
					ss << "#DESCRIPTION";
					if (ver < 3)
						ss << ':';
					ss << ' ' << chref.value;
				}
			}

			if (! chref.stream)
				error("make_userbouquet", "Maker Error", msg("reference (%s)", bname + ':' + x.second + ':' + to_string(ln)));
		}

		ss << endl;
	}

	file.origin = FORG::filesys;
	file.filename = filename;
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
		if (tn.flags != -1)
			ss << ' ' << "flags=\"" << tn.flags << "\"";
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
					if (tntxp.t2mi_plpid != -1)
						ss << ' ' << "t2mi_plp_id=\"" << tntxp.t2mi_plpid << "\"";
					if (tntxp.t2mi_pid != -1)
						ss << ' ' << "t2mi_pid=\"" << tntxp.t2mi_pid << "\"";
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
					if (tntxp.plpid != -1)
						ss << ' ' << "plp_id=\"" << tntxp.plpid << "\"";
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

	//TODO FIX out of range substr (ie. merge without services file)

	try
	{
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
	}
	catch (...)
	{
	}

	file.origin = FORG::filesys;
	file.filename = filename;
	file.mime = "text/xml";
	file.data = str;
	file.size = file.data.size();
}

void e2db_maker::make_services_xml(string filename, e2db_file& file, int ver)
{
	debug("make_services_xml", "filename", filename);

	string fname;

	if (zxdata.count("services.xml"))
		fname = "services.xml";

	if (fname.empty())
	{
		zapit_data zx;
		zx.fname = fname = "services.xml";
		zx.itype = 0;
		zx.charset = "UTF-8";

		zxdata.emplace(zx.fname, zx);
	}

	vector<string> zyi;
	unordered_map<string, zapit_table> x_tables;

	for (auto & x : index["txs"])
	{
		transponder tx = db.transponders[x.second];

		char zyid[25];
		// %1x:%8x
		std::snprintf(zyid, 25, "%x:%x", tx.ytype, tx.pos);

		zapit_table& zy = zytables[zyid];
		zy.zyid = zyid;
		zy.ytype = tx.ytype;
		zy.pos = tx.pos;

		if (tx.ytype == YTYPE::satellite && tnloc.count(tx.pos))
		{
			string tnid = tnloc.at(tx.pos);
			tunersets_table tn = tuners[0].tables[tnid];
			zy.name = tn.name;
		}
		else if (tx.ytype == YTYPE::terrestrial)
			zy.name = "Terrestrial";
		else if (tx.ytype == YTYPE::cable)
			zy.name = "Cable";
		else if (tx.ytype == YTYPE::atsc)
			zy.name = "ATSC";
		else
			zy.name = tx.pos == -1 ? "NaN" : value_transponder_position(tx.pos);

		zy.transponders.emplace_back(tx.txid);
		x_tables[zy.zyid] = zy;
	}

	{
		unordered_set<string> _unique;
		map<int, string> tmp_i0;
		map<int, string> tmp_i1;
		map<int, string> tmp_i2;

		int i = 0;
		for (auto & x : x_tables)
		{
			string zyid = x.first;
			zapit_table& zy = x_tables[zyid];
			int ytype = zy.ytype;
			int pos = zy.pos;
			int idx = zy.index;

			if (! _unique.count(zyid))
			{
				if (idx == -1)
				{
					if (ytype == YTYPE::satellite)
						tmp_i0.emplace(pos, zyid);
					else
						tmp_i2.emplace(i++, zyid);
				}
				else
				{
					if (ytype == YTYPE::satellite)
						tmp_i1.emplace(i++, zyid);
					else
						tmp_i2.emplace(i++, zyid);
				}

				_unique.emplace(zyid);
			}
		}

		for (auto & x : tmp_i0)
			zyi.emplace_back(x.second);
		for (auto & x : tmp_i1)
			zyi.emplace_back(x.second);
		for (auto & x : tmp_i2)
			zyi.emplace_back(x.second);
	}
	{
		int idx = 0;
		for (string & zyid : zyi)
		{
			zapit_table& zy = x_tables[zyid];
			zy.index = idx++;
		}
	}

	zytables.swap(x_tables);

	zapit_data zx = zxdata[fname];

	stringstream ss;

	string iname = fname;
	unordered_map<int, string> tags;
	tags[0] = "zapit";
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

	unordered_set<string> x_transponders;

	ss << "<?xml version=\"1.0\" encoding=\"" << zx.charset << "\"?>" << endl;
	ss << '<' << tags[0];
	if (ver > 1)
		ss << ' ' << "api=\"" << ver << "\"";
	ss << '>' << endl;

	for (string & zyid : zyi)
	{
		zapit_table zy = zytables[zyid];

		if (ver > 2 && zy.ytype == YTYPE::terrestrial)
			tags[1] = "terrestrial";
		else
			tags[1] = "sat";

		ss << "\t" << '<' << tags[1];
		ss << ' ' << "name=\"" << conv_xml_value(zy.name) << "\"";
		if (ver > 2)
		{
			ss << ' ' << "position=\"" << (zy.pos != -1 ? zy.pos : 0) << "\"";

			if (zy.ytype == YTYPE::satellite)
			{
				ss << ' ' << "diseqc=\"" << (zy.diseqc != -1 ? zy.diseqc : -1) << "\"";
				ss << ' ' << "uncommited=\"" << (zy.uncomtd != -1 ? zy.uncomtd : -1) << "\"";
			}
		}
		else
		{
			ss << ' ' << "diseqc=\"" << (zy.diseqc != -1 ? zy.diseqc : 0) << "\"";
			ss << ' ' << "position=\"" << (zy.pos != -1 ? zy.pos : 0) << "\"";
		}
		ss << '>' << endl;

		for (auto & w : zy.transponders)
		{
			transponder tx = db.transponders[w];

			ss << "\t\t" << '<' << tags[2];
			if (ver > 1)
			{
				ss << ' ' << "id=\"" << hex << setfill('0') << setw(4) << tx.tsid << dec << "\"";
				ss << ' ' << "on=\"" << hex << setfill('0') << setw(4) << tx.onid << dec << "\"";
				if (ver > 2 && zy.ytype == YTYPE::terrestrial)
					ss << ' ' << "frq=\"" << int (tx.freq * 1e2) << "\"";
				else
					ss << ' ' << "frq=\"" << int (tx.freq * 1e3) << "\"";
				ss << ' ' << "inv=\"" << (tx.inv != -1 && tx.inv != 0 ? tx.inv : 2) << "\"";
				if (zy.ytype == YTYPE::terrestrial)
				{
					if (ver < 3)
						ss << ' ' << "sr=\"0\"";
				}
				else
				{
					ss << ' ' << "sr=\"" << (tx.sr != -1 && tx.sr != 0 ? int (tx.sr * 1e3) : 0) << "\"";
				}
				if (ver > 2 && zy.ytype == YTYPE::terrestrial)
				{
					ss << ' ' << "band=\"" << (tx.band != -1 ? tx.band : 0) << "\"";
					ss << ' ' << "HP=\"" << (tx.hpfec != -1 ? tx.hpfec : 5) << "\"";
					ss << ' ' << "LP=\"" << (tx.lpfec != -1 ? tx.lpfec : 5) << "\"";
					if (tx.tmod != -1)
						ss << ' ' << "const=\"" << 1 << "\"";
					ss << ' ' << "trans=\"" << (tx.tmx != -1 ? tx.tmx : 2) << "\"";
					ss << ' ' << "guard=\"" << (tx.guard != -1 ? tx.guard : 4) << "\"";
					ss << ' ' << "hierarchy=\"" << (tx.hier != -1 ? tx.hier : 4) << "\"";
				}
				else
				{
					int i = 0;
					if (ver == 4)
					{
						if (tx.fec <= 0)
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
						if (tx.fec <= 0)
							i = 0;
						else if (tx.fec < 4)
							i = tx.fec;
						else if (tx.fec == 4)
							i = 5;
						else if (tx.fec == 5)
							i = 7;
					}
					else if (ver == 2)
					{
						if (tx.fec <= 0)
							i = 0;
						else if (tx.fec < 6)
							i = tx.fec;
					}
					ss << ' ' << "fec=\"" << i << "\"";
				}
				if (zy.ytype == YTYPE::satellite)
					ss << ' ' << "pol=\"" << (tx.pol != -1 ? tx.pol : 0) << "\"";
				else if (ver == 2)
					ss << ' ' << "pol=\"" << 0 << "\"";
				if (ver > 3)
				{
					//TODO
					if (zy.ytype == YTYPE::terrestrial)
					{
						ss << ' ' << "mod=\"" << (tx.tmod != -1 ? tx.tmod * 2 : 6) << "\"";
						ss << ' ' << "sys=\"" << 4 << "\"";
					}
					else
					{
						ss << ' ' << "mod=\"" << (tx.mod != -1 ? tx.mod : 0) << "\"";
						ss << ' ' << "sys=\"" << (tx.sys != -1 ? tx.sys : 0) << "\"";
					}
				}
			}
			else
			{
				ss << ' ' << "id=\"" << hex << setfill('0') << setw(4) << tx.tsid << dec << "\"";
				ss << ' ' << "onid=\"" << hex << setfill('0') << setw(4) << tx.onid << dec << "\"";
				ss << ' ' << "frequency=\"" << int (tx.freq * 1e3) << "\"";
				ss << ' ' << "inversion=\"" << (tx.inv != 2 ? tx.inv : 0) << "\"";
				ss << ' ' << "symbol_rate=\"" << (tx.sr != -1 && tx.sr != 0 ? int (tx.sr * 1e3) : 0) << "\"";
				{
					int i = 0;
					if (tx.fec <= 0)
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
				ss << ' ' << "polarization=\"" << (tx.pol != -1 ? tx.pol : 0) << "\"";
			}

			ss << '>' << endl;

			if (! x_transponders.count(tx.txid))
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
						if (zy.ytype != YTYPE::terrestrial)
						{
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
						}
						ss << ' ' << "t=\"" << hex << ch.stype << dec << "\"";
						if (ver > 2)
						{
							if (zy.ytype != YTYPE::terrestrial)
							{
								{
									int cval = 0;
									string cpx = (SDATA_PIDS::vtype > 9 ? "" : "0") + to_string(SDATA_PIDS::vtype);
									for (string & w : ch.data[SDATA::c])
										if (w.substr(0, 2) == cpx)
											cval = int (std::strtol(w.substr(2).data(), NULL, 16));
									ss << ' ' << "vt=\"" << hex << cval << dec << "\"";
								}
							}
							ss << ' ' << "s=\"" << (ch.data.count(SDATA::C) ? 1 : 0) << "\"";
							ss << ' ' << "num=\"" << ch.snum << "\"";
							ss << ' ' << "f=\"" << ch.flags << "\"";
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

			x_transponders.emplace(tx.txid);
		}

		ss << "\t" << '<' << '/' << tags[1] << '>' << endl;

		x_transponders.clear();
	}
	ss << '<' << '/' << tags[0] << '>' << endl;

	string str = ss.str();

	if (comments.count(iname))
	{
		int i = 0;
		size_t pos = 0;
		for (auto & s : comments[iname])
		{
			if (s.text.find("Editor:") == 1 || s.text.find("Datetime:") == 1)
				continue;

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

	string editor = editor_string();
	string timestamp = editor_timestamp();

	str += "<!-- Editor: " + editor + " -->\n";
	str += "<!-- Datetime: " + timestamp + " -->\n";

	file.origin = FORG::filesys;
	file.filename = filename;
	file.mime = "text/xml";
	file.data = str;
	file.size = file.data.size();
}

void e2db_maker::make_bouquets_xml(string filename, e2db_file& file, int ver)
{
	debug("make_bouquets_xml", "filename", filename);

	string fname;

	if (zxdata.count("ubouquets.xml"))
		fname = "ubouquets.xml";
	else if (zxdata.count("ubouquets.xml"))
		fname = "bouquets.xml";

	if (fname.empty())
	{
		zapit_data zx;
		zx.fname = fname = ver > 1 ? "ubouquets.xml" : "bouquets.xml";
		zx.itype = 1;
		zx.charset = "UTF-8";

		zxdata.emplace(zx.fname, zx);
	}

	zapit_data zx = zxdata[fname];

	stringstream ss;
	int ln = 2;

	string iname = fname;
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

	ss << "<?xml version=\"1.0\" encoding=\"" << zx.charset << "\"?>" << endl;
	ss << '<' << tags[0] << '>' << endl;

	vector<string> ubindex;
	for (auto & x : index["bss"])
	{
		bouquet bs = bouquets[x.second];
		ubindex.insert(ubindex.end(), bs.userbouquets.begin(), bs.userbouquets.end());
	}

	for (auto & w : ubindex)
	{
		ln++;

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
				ln++;

				service ch = db.services[x.second];
				transponder tx = db.transponders[ch.txid];

				ss << "\t\t" << '<' << tags[2];
				if (ver > 1)
				{
					ss << ' ' << "i=\"" << hex << ch.ssid << dec << "\"";
					ss << ' ' << "n=\"" << conv_xml_value(ch.chname) << "\"";
					ss << ' ' << "t=\"" << hex << ch.tsid << dec << "\"";
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
				if (! chref.marker && ! chref.stream)
					error("make_bouquets_xml", "Maker Error", msg("reference (%s)", filename + ':' + x.second + ':' + to_string(ln)));
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

	file.origin = FORG::filesys;
	file.filename = filename;
	file.mime = "text/xml";
	file.data = str;
	file.size = file.data.size();
}

string e2db_maker::conv_uri_value(string str)
{
	if (str.find(':') != string::npos)
	{
		size_t n = 0;

		while (n != str.size())
		{
			if (str[n] == ':')
			{
				str = str.substr(0, n) + "%3a" + str.substr(n + 1);
				n += 2;
			}
			n++;
		}
	}

	return str;
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

//TODO improve parental by userbouquet and compatibility
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
			bool parental = db.parental == PARENTALLOCK::whitelist ? ! ub.parental : ub.parental;

			if (parental)
			{
				size_t pos = ub.bname.find('.');
				size_t n = ub.bname.rfind('.');
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
			bool parental = db.parental == PARENTALLOCK::whitelist ? ! ub.parental : ub.parental;

			if (parental)
			{
				string bname = ub.bname;

				for (auto & x : index[bname])
				{
					channel_reference chref = ub.channels[x.second];

					if (db.services.count(x.second))
					{
						service ch = db.services[x.second];

						ss << chref.etype << ':';
						ss << chref.atype << ':';
						ss << hex;
						ss << uppercase << ch.stype << ':';
						ss << uppercase << ch.ssid << ':';
						ss << uppercase << ch.tsid << ':';
						ss << uppercase << ch.onid << ':';
						ss << uppercase << ch.dvbns << ':';
						ss << "0:0:0:";
						ss << dec;
						ss << endl;
					}
					else if (chref.stream)
					{
						ss << chref.etype << ':';
						ss << chref.atype << ':';
						ss << hex;
						ss << uppercase << chref.anum << ':';
						ss << uppercase << chref.ref.ssid << ':';
						ss << uppercase << chref.ref.tsid << ':';
						ss << uppercase << chref.ref.onid << ':';
						ss << uppercase << chref.ref.dvbns << ':';
						ss << dec;
						ss << chref.x7 << ':';
						ss << chref.x8 << ':';
						ss << chref.x9 << ':';

						if (! chref.url.empty())
						{
							ss << conv_uri_value(chref.url);
						}
						if (! chref.value.empty())
						{
							ss << ':';
							ss << conv_uri_value(chref.value);
						}

						ss << endl;
					}
				}
			}
		}
	}

	for (auto & x : db.services)
	{
		service ch = x.second;
		bool parental = db.parental == PARENTALLOCK::whitelist ? ! ch.parental : ch.parental;

		if (parental)
		{
			ss << "1:0:";
			ss << hex;
			ss << uppercase << ch.stype << ':';
			ss << uppercase << ch.ssid << ':';
			ss << uppercase << ch.tsid << ':';
			ss << uppercase << ch.onid << ':';
			ss << uppercase << ch.dvbns << ':';
			ss << dec;
			ss << "0:0:0:";
			ss << endl;
		}
	}

	file.origin = FORG::filesys;
	file.filename = filename;
	file.mime = "text/plain";
	file.data = ss.str();
	file.size = file.data.size();
}

bool e2db_maker::push_file(string path)
{
	debug("push_file", "path", path);

	if (std::filesystem::is_directory(path))
	{
		if (! OVERWRITE_FILE)
		{
			error("push_file", "File Error", msg("File \"%s\" already exists.", path));

			return false;
		}
	}
	else
	{
		std::filesystem::create_directory(path);
	}
	if
	(
		(std::filesystem::status(path).permissions() & std::filesystem::perms::owner_write) == std::filesystem::perms::none &&
		(std::filesystem::status(path).permissions() & std::filesystem::perms::group_write) == std::filesystem::perms::none
	)
	{
		error("push_file", "File Error", msg("File \"%s\" is not writable.", path));

		return false;
	}
	for (auto & o: this->e2db_out)
	{
		if (path.size() && path[path.size() - 1] != '/')
			path.append("/");

		string fpath = path + o.first;

		if (! OVERWRITE_FILE && std::filesystem::exists(fpath))
		{
			error("push_file", "File Error", msg("File \"%s\" already exists.", fpath));

			return false;
		}
		if
		(
			(std::filesystem::status(fpath).permissions() & std::filesystem::perms::owner_write) == std::filesystem::perms::none &&
			(std::filesystem::status(fpath).permissions() & std::filesystem::perms::group_write) == std::filesystem::perms::none
		)
		{
			error("push_file", "File Error", msg("File \"%s\" is not writable.", fpath));

			return false;
		}

		std::ios_base::openmode fmode = std::ios_base::out;

		if (MAKER_FIX_CRLF && check_crlf()) fmode |= std::ios_base::binary;

		ofstream out (fpath, fmode);
		out.exceptions(ofstream::failbit | ofstream::badbit);
		out << o.second.data;
		out.close();

		o.second.path = fpath;
	}

	return true;
}

bool e2db_maker::write(string path)
{
	debug("write", "filename", path);

	try
	{
		this->e2db_out.clear();

		if (db.type == 0)
			make_e2db();
		else if (db.type == 1)
			make_zapit();

		if (db.type == 0)
		{
			int lamedb_ver = LAMEDB_VER;

			ZAPIT_VER = -1;
			LAMEDB_VER = lamedb_ver;
			db.version = 0x1220 + lamedb_ver;
		}
		else if (db.type == 1)
		{
			int zapit_ver = ZAPIT_VER;

			LAMEDB_VER = -1;
			ZAPIT_VER = zapit_ver;
			db.version = 0x1010 + zapit_ver;
		}

		if (push_file(path))
		{
			this->e2db_in = this->e2db_out;
			this->e2db_out.clear();
			this->e2db_files.clear();

			for (auto & x : this->e2db_in)
				this->e2db_files.emplace_back(e2db_file::status(x.second));

			db.dstat = DSTAT::d_write;

			return true;
		}
		else
		{
			return false;
		}
	}
	catch (const std::invalid_argument& err)
	{
		exception("write", "Maker Error", msg(MSG::except_invalid_argument, err.what()));
	}
	catch (const std::out_of_range& err)
	{
		exception("write", "Maker Error", msg(MSG::except_out_of_range, err.what()));
	}
	catch (const std::filesystem::filesystem_error& err)
	{
		exception("write", "Maker Error", msg(MSG::except_filesystem, err.what()));
	}
	catch (const std::ofstream::failure& err)
	{
		exception("write", "Maker Error", msg("File \"%s\" is not writable.", path));
	}
	catch (...)
	{
		exception("write", "Maker Error", msg(MSG::except_uncaught));
	}

	return false;
}

unordered_map<string, e2db_abstract::e2db_file> e2db_maker::get_output()
{
	debug("get_output");

	try
	{
		this->e2db_out.clear();

		if (db.type == 0) 
			make_e2db();
		else if (db.type == 1)
			make_zapit();

		return this->e2db_out;
	}
	catch (const std::invalid_argument& err)
	{
		exception("get_output", "Maker Error", msg(MSG::except_invalid_argument, err.what()));
	}
	catch (const std::out_of_range& err)
	{
		exception("get_output", "Maker Error", msg(MSG::except_out_of_range, err.what()));
	}
	catch (const std::filesystem::filesystem_error& err)
	{
		exception("get_output", "Maker Error", msg(MSG::except_filesystem, err.what()));
	}
	catch (...)
	{
		exception("get_output", "Maker Error", msg(MSG::except_uncaught));
	}

	unordered_map<string, e2db_abstract::e2db_file> kxnul;
	return kxnul;
}

}
