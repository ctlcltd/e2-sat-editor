/*!
 * e2-sat-editor/src/e2db/e2db_maker.cpp
 * 
 * @link https://github.com/ctlcltd/e2-sat-editor
 * @copyright e2 SAT Editor Team
 * @author Leonardo Laureti
 * @version 0.1
 * @license MIT License
 * @license GNU GPLv3 License
 */

#include <algorithm>
#include <sstream>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <filesystem>

#include "e2db_maker.h"

using std::ofstream, std::stringstream, std::hex, std::dec, std::setfill, std::setw, std::uppercase, std::to_string, std::cout, std::endl;

namespace e2se_e2db
{

e2db_maker::e2db_maker()
{
	this->log = new e2se::logger("e2db");

	debug("e2db_maker()");
}

void e2db_maker::make_e2db()
{
	debug("make_e2db()");

	begin_transaction();
	make_e2db_lamedb();
	make_e2db_bouquets();
	make_e2db_userbouquets();
	end_transaction();
}

void e2db_maker::begin_transaction()
{
	debug("begin_transaction()");

	std::time_t curr_tst = std::time(0);
	std::tm* _out_tst = std::localtime(&curr_tst);
	this->_out_tst = _out_tst;
}

void e2db_maker::end_transaction()
{
	debug("end_transaction()");
}

//TODO FIX mingw32 wrong %z %Z
string e2db_maker::get_timestamp()
{
	debug("get_timestamp()");

	char datetime[80];
	// @link https://sourceforge.net/p/mingw-w64/bugs/793/
	std::strftime(datetime, 80, "%Y-%m-%d %H:%M:%S %z", _out_tst);
	return string (datetime);
}

string e2db_maker::get_editor_string()
{
	return "e2-sat-editor 0.1 <https://github.com/ctlcltd/e2-sat-editor>";
}

void e2db_maker::make_e2db_lamedb()
{
	debug("make_e2db_lamedb()");

	make_e2db_lamedb4();

	//TEST
	if (MAKER_LAMEDB5)
		make_e2db_lamedb5();
	//TEST
}

void e2db_maker::make_e2db_lamedb4()
{
	debug("make_e2db_lamedb4()");
	LAMEDB_VER = 4;
	make_lamedb("lamedb");
}

void e2db_maker::make_e2db_lamedb5()
{
	debug("make_e2db_lamedb5()");
	LAMEDB_VER = 5;
	make_lamedb("lamedb5");
}

void e2db_maker::make_lamedb(string filename)
{
	debug("make_lamedb()");

	// formatting
	//
	// [0]  comment
	// [1]  transponders start
	// [2]  services start
	// [3]  section end
	// [4]  delimiter
	// [5]  transponder flag
	// [6]  transponder params separator
	// [7]  transponder space delimiter
	// [8]  transponder endline
	// [9]  service flag
	// [10] service params separator
	// [11] service param escape
	// [12] service endline
	const string (&formats)[13] = LAMEDB_VER < 5 ? LAMEDB4_FORMATS : LAMEDB5_FORMATS;

	stringstream ss;
	ss << "eDVB services /" << LAMEDB_VER << "/" << endl;

	ss << formats[1];
	for (auto & x: index["txs"])
	{
		transponder tx = db.transponders[x.second];
		ss << formats[5] << formats[4];
		ss << hex;
		ss << setfill('0') << setw(8) << tx.dvbns;
		ss << ':' << setfill('0') << setw(4) << tx.tsid;
		ss << ':' << setfill('0') << setw(4) << tx.onid;
		ss << dec;
		ss << formats[6];
		ss << tx.ttype << formats[7];
		switch (tx.ttype)
		{
			case 's': // DVB-S
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
			case 't': // DVB-T
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
			//TODO
			case 'c': // DVB-C
				ss << int (tx.freq * 1e3);
				ss << ':' << int (tx.sr * 1e3);
				ss << ':' << tx.inv;
				ss << ':' << tx.cmod;
				ss << ':' << tx.cfec;
				if (! tx.oflgs.empty())
					ss << tx.oflgs;
			break;
			//TODO
			case 'a': // ATSC
				ss << int (tx.freq * 1e3);
				ss << ':' << tx.inv;
				ss << ':' << tx.amod;
				ss << ':' << tx.flgs;
				ss << ':' << tx.sys;
				if (! tx.oflgs.empty())
					ss << tx.oflgs;
			break;
		}
		ss << formats[8];
	}
	ss << formats[3];

	ss << formats[2];
	for (auto & x: index["chs"])
	{
		service ch = db.services[x.second];
		ss << formats[9] << formats[4];
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
		ss << formats[10];
		ss << formats[11] << ch.chname << formats[11];
		ss << formats[10];
		//TODO 256
		auto last_key = (*prev(ch.data.cend()));
		for (auto & q: ch.data)
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
			for (unsigned int i = 0; i < q.second.size(); i++)
			{
				ss << d << ':' << q.second[i];
				if (! q.second[i].empty() && (i != q.second.size() - 1 || q.first != last_key.first))
					ss << ',';
			}
		}
		ss << formats[12];
	}
	ss << formats[3];

	ss << formats[0] << "editor: " << get_editor_string() << endl;
	ss << formats[0] << "datetime: " << get_timestamp() << endl;
	e2db_out[filename] = ss.str();
}

void e2db_maker::make_e2db_bouquets()
{
	debug("make_e2db_bouquets()");

	for (auto & x: bouquets)
		make_bouquet(x.first);
}

void e2db_maker::make_e2db_userbouquets()
{
	debug("make_e2db_userbouquets()");

	for (auto & x: userbouquets)
		make_userbouquet(x.first);
}

void e2db_maker::make_bouquet(string bname)
{
	debug("make_bouquet()", "bname", bname);

	bouquet bs = bouquets[bname];
	stringstream ss;

	ss << "#NAME " << bs.name << endl;
	for (auto & w: bs.userbouquets)
	{
		ss << "#SERVICE ";
		ss << "1:7:" << bs.btype << ":0:0:0:0:0:0:0:";
		ss << "FROM BOUQUET ";
		ss << "\"" << w << "\" ";
		ss << "ORDER BY bouquet";
		ss << endl;
	}
	// ss << endl;
	e2db_out[bname] = ss.str();
}

//TODO upCase or loCase
void e2db_maker::make_userbouquet(string bname)
{
	debug("make_userbouquet()", "bname", bname);

	userbouquet ub = userbouquets[bname];
	stringstream ss;

	ss << "#NAME " << ub.name << endl;
	for (auto & x: index[bname])
	{
		channel_reference chref = userbouquets[bname].channels[x.second];
		ss << "#SERVICE ";
		ss << "1:";
		ss << chref.type << ':';
		ss << hex;
		ss << uppercase << chref.anum << ':'; //TODO ("global markers index)
		
		if (db.services.count(x.second))
		{
			service ch = db.services[x.second];
			string onid = ch.onid.empty() ? "0" : ch.onid;
			std::transform(onid.begin(), onid.end(), onid.begin(), [](unsigned char c) { return toupper(c); });

			ss << uppercase << ch.ssid << ':';
			ss << uppercase << ch.tsid << ':';
			ss << onid << ':';
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
				error("make_userbouquet()", "Error", "Missing channel_reference \"" + x.second + "\".");
			}
		}
		ss << dec;
		ss << endl;
	}
	// ss << endl;
	e2db_out[bname] = ss.str();
}

bool e2db_maker::write_to_localdir(string localdir, bool overwrite)
{
	debug("write_to_localdir()", "localdir", localdir);

	if (! std::filesystem::is_directory(localdir)) //C++17
	{
		error("write_to_localdir()", "Error", "Directory \"" + localdir + "\" not exists.");
		return false;
	}
	//TODO file exists and (force) overwrite
	else if (! overwrite)
	{
		std::filesystem::create_directory(localdir); //C++17
	}
	//TODO permission check ...
	for (auto & o: e2db_out)
	{
		string localfile = localdir + '/' + o.first;

		ofstream out (localfile);
		out << o.second;
		out.close();
	}

	return true;
}

bool e2db_maker::write(string localdir, bool overwrite)
{
	debug("write()", "localdir", localdir);

	make_e2db();

	if (write_to_localdir(localdir, overwrite))
		return true;
	else
		return false;
}

unordered_map<string, e2db_file> e2db_maker::get_output() {
	debug("get_output()");
	
	make_e2db();

	return e2db_out;
}

void e2db_maker::set_index(unordered_map<string, vector<pair<int, string>>> index)
{
	debug("set_index()");
	this->index = index;
}

void e2db_maker::set_transponders(unordered_map<string, e2db_maker::transponder> transponders)
{
	debug("set_transponders()");
	db.transponders = transponders;
}

void e2db_maker::set_channels(unordered_map<string, e2db_maker::service> services)
{
	debug("set_channels()");
	db.services = services;
}

void e2db_maker::set_bouquets(pair<unordered_map<string, e2db_maker::bouquet>, unordered_map<string, e2db_maker::userbouquet>> bouquets)
{
	debug("set_bouquets()");
	this->bouquets = bouquets.first;
	this->userbouquets = bouquets.second;
}

}