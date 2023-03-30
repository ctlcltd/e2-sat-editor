/*!
 * e2-sat-editor/src/e2db/e2db.cpp
 * 
 * @link https://github.com/ctlcltd/e2-sat-editor
 * @copyright e2 SAT Editor Team
 * @author Leonardo Laureti
 * @version 0.5
 * @license MIT License
 * @license GNU GPLv3 License
 */

#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <clocale>
#include <algorithm>
#include <unordered_set>
#include <sstream>
#include <fstream>
#include <iomanip>
#include <filesystem>

#include "e2db.h"

using std::unordered_set, std::stringstream, std::ifstream, std::ofstream, std::to_string, std::setfill, std::setw;

namespace e2se_e2db
{

e2db::e2db()
{
	std::setlocale(LC_NUMERIC, "C");

	this->log = new e2se::logger("e2db", "e2db");
}

void e2db::import_file(vector<string> paths)
{
	debug("import_file", "file path", "multiple");
	debug("import_file", "file input", "auto");

	bool merge = this->get_input().size() != 0 ? true : false;
	auto* dst = merge ? newptr() : this;

	for (string & path : paths)
	{
		if (! std::filesystem::exists(path))
		{
			if (merge) delete dst;
			return error("import_file", "File Error", "File \"" + path + "\" not exists.");
		}
		if (! std::filesystem::is_regular_file(path) && ! std::filesystem::is_directory(path))
		{
			if (merge) delete dst;
			return error("import_file", "File Error", "File \"" + path + "\" is not a valid file.");
		}
		if
		(
			(std::filesystem::status(path).permissions() & std::filesystem::perms::owner_read) == std::filesystem::perms::none &&
			(std::filesystem::status(path).permissions() & std::filesystem::perms::group_read) == std::filesystem::perms::none
		)
		{
			if (merge) delete dst;
			return error("import_file", "File Error", "File \"" + path + "\" is not readable.");
		}

		FPORTS fpi = file_type_detect(path);
		string filename = std::filesystem::path(path).filename().u8string();
		string mime = file_mime_detect(fpi, path);

		e2db_file file;
		file.filename = filename;
		file.mime = mime;

		if (fpi != FPORTS::directory)
		{
			ifstream ifile (path);
			string line;
			while (std::getline(ifile, line))
				file.data.append(line + '\n');
			ifile.close();
		}
		file.size = file.data.size();

		import_file(fpi, dst, file, path);
	}
	if (merge)
	{
		this->merge(dst);
		delete dst;
	}
}

//TODO TEST
void e2db::import_file(FPORTS fpi, e2db* dst, e2db_file file, string path)
{
	debug("import_file", "file path", "singular");
	debug("import_file", "file input", fpi);

	string filename = std::filesystem::path(path).filename().u8string();
	stringstream ifile;
	ifile.write(&file.data[0], file.size);

	switch (fpi)
	{
		case FPORTS::directory:
			dst->read(path);
		break;
		case FPORTS::all_services:
			dst->parse_e2db_lamedb(ifile);
		break;
		case FPORTS::all_services__2_4:
			dst->parse_e2db_lamedbx(ifile, 4);
		break;
		case FPORTS::all_services__2_5:
			dst->parse_e2db_lamedb5(ifile);
		break;
		case FPORTS::all_services__2_3:
			dst->parse_e2db_lamedbx(ifile, 3);
		break;
		case FPORTS::all_services__2_2:
			dst->parse_e2db_lamedbx(ifile, 2);
		break;
		case FPORTS::all_services_xml:
		case FPORTS::all_services_xml__4:
		case FPORTS::all_services_xml__3:
		case FPORTS::all_services_xml__2:
		case FPORTS::all_services_xml__1:
			dst->parse_zapit_services_xml(ifile, filename);
		break;
		case FPORTS::single_tunersets:
		case FPORTS::all_tunersets:
			if (filename == "satellites.xml")
				dst->parse_tunersets_xml(YTYPE::satellite, ifile);
			else if (filename == "terrestrial.xml")
				dst->parse_tunersets_xml(YTYPE::terrestrial, ifile);
			else if (filename == "cables.xml")
				dst->parse_tunersets_xml(YTYPE::cable, ifile);
			else if (filename == "atsc.xml")
				dst->parse_tunersets_xml(YTYPE::atsc, ifile);
		break;
		case FPORTS::single_bouquet:
		case FPORTS::all_bouquets:
			dst->parse_e2db_bouquet(ifile, filename);
		break;
		case FPORTS::single_bouquet_epl:
		case FPORTS::all_bouquets_epl:
			dst->parse_e2db_bouquet(ifile, filename, true);
		break;
		case FPORTS::single_userbouquet:
		case FPORTS::all_userbouquets:
			dst->parse_e2db_userbouquet(ifile, filename);
		break;
		case FPORTS::single_bouquet_all:
		case FPORTS::single_bouquet_all_epl:
			if (file_type_detect(filename) == FPORTS::single_bouquet)
				dst->parse_e2db_bouquet(ifile, filename);
			else if (file_type_detect(filename) == FPORTS::single_bouquet_epl)
				dst->parse_e2db_bouquet(ifile, filename, true);
			else
				dst->parse_e2db_userbouquet(ifile, filename);
		break;
		case FPORTS::all_bouquets_xml:
			if (filename == "bouquets.xml")
				dst->parse_zapit_bouquets_apix_xml(ifile, filename, 1);
			else
				dst->parse_zapit_bouquets_apix_xml(ifile, filename, (ZAPIT_VER != -1 ? ZAPIT_VER : 4));
		break;
		case FPORTS::all_bouquets_xml__4:
			dst->parse_zapit_bouquets_apix_xml(ifile, filename, 4);
		break;
		case FPORTS::all_bouquets_xml__3:
			dst->parse_zapit_bouquets_apix_xml(ifile, filename, 3);
		break;
		case FPORTS::all_bouquets_xml__2:
			dst->parse_zapit_bouquets_apix_xml(ifile, filename, 2);
		break;
		case FPORTS::all_bouquets_xml__1:
			dst->parse_zapit_bouquets_apix_xml(ifile, filename, 1);
		break;
		case FPORTS::single_parentallock_blacklist:
			dst->parse_e2db_parentallock_list(PARENTALLOCK::blacklist, ifile);
		break;
		case FPORTS::single_parentallock_whitelist:
			dst->parse_e2db_parentallock_list(PARENTALLOCK::whitelist, ifile);
		break;
		case FPORTS::single_parentallock_locked:
			dst->parse_e2db_parentallock_list(PARENTALLOCK::locked, ifile);
		break;
		default:
		return error("import_file", "Error", "Unknown import option.");
	}
}

void e2db::export_file(vector<string> paths)
{
	debug("export_file", "file path", "multiple");
	debug("export_file", "file output", "auto");

	for (string & w : paths)
	{
		FPORTS fpo = file_type_detect(w);
		export_file(fpo, w);
	}
}

void e2db::export_file(FPORTS fpo, vector<string> paths)
{
	debug("export_file", "file path", "multiple");
	debug("export_file", "file output", fpo);

	for (string & w : paths)
	{
		export_file(fpo, w);
	}
}

//TODO TEST
void e2db::export_file(FPORTS fpo, string path)
{
	debug("export_file", "file path", "singular");
	debug("export_file", "file output", fpo);

	e2db_file file;
	string filename = std::filesystem::path(path).filename().u8string();

	string fname = LAMEDB_VER == 5 ? "lamedb5" : (LAMEDB_VER < 4 ? "services" : "lamedb");

	switch (fpo)
	{
		case FPORTS::directory:
			write(path);
		return;
		case FPORTS::all_services:
			make_lamedb(fname, file, LAMEDB_VER);
		break;
		case FPORTS::all_services__2_4:
			make_lamedb("lamedb", file, 4);
		break;
		case FPORTS::all_services__2_5:
			make_lamedb("lamedb5", file, 5);
		break;
		case FPORTS::all_services__2_3:
			make_lamedb("services", file, 3);
		break;
		case FPORTS::all_services__2_2:
			make_lamedb("services", file, 2);
		break;
		case FPORTS::all_services_xml:
			make_services_xml(filename, file, (ZAPIT_VER != -1 ? ZAPIT_VER : 4));
		break;
		case FPORTS::all_services_xml__4:
			make_services_xml(filename, file, 4);
		break;
		case FPORTS::all_services_xml__3:
			make_services_xml(filename, file, 3);
		break;
		case FPORTS::all_services_xml__2:
			make_services_xml(filename, file, 2);
		break;
		case FPORTS::all_services_xml__1:
			make_services_xml(filename, file, 1);
		break;
		case FPORTS::single_tunersets:
		case FPORTS::all_tunersets:
			if (filename == "satellites.xml")
				make_tunersets_xml(filename, YTYPE::satellite, file);
			else if (filename == "terrestrial.xml")
				make_tunersets_xml(filename, YTYPE::terrestrial, file);
			else if (filename == "cables.xml")
				make_tunersets_xml(filename, YTYPE::cable, file);
			else if (filename == "atsc.xml")
				make_tunersets_xml(filename, YTYPE::atsc, file);
		break;
		case FPORTS::single_bouquet:
		case FPORTS::all_bouquets:
			make_bouquet(filename, file);
		break;
		case FPORTS::single_bouquet_epl:
		case FPORTS::all_bouquets_epl:
			make_bouquet_epl(filename, file);
		break;
		case FPORTS::single_userbouquet:
		case FPORTS::all_userbouquets:
			make_userbouquet(filename, file);
		break;
		case FPORTS::single_bouquet_all:
		case FPORTS::single_bouquet_all_epl:
			if (file_type_detect(filename) == FPORTS::single_bouquet)
				make_bouquet(filename, file);
			else if (file_type_detect(filename) == FPORTS::single_bouquet_epl)
				make_bouquet_epl(filename, file);
			else
				make_userbouquet(filename, file);
		break;
		case FPORTS::all_bouquets_xml:
			make_bouquets_xml(filename, file, (ZAPIT_VER != -1 ? ZAPIT_VER : 4));
		break;
		case FPORTS::all_bouquets_xml__4:
			make_bouquets_xml(filename, file, 4);
		break;
		case FPORTS::all_bouquets_xml__3:
			make_bouquets_xml(filename, file, 3);
		break;
		case FPORTS::all_bouquets_xml__2:
			make_bouquets_xml(filename, file, 2);
		break;
		case FPORTS::all_bouquets_xml__1:
			make_bouquets_xml(filename, file, 1);
		break;
		case FPORTS::single_parentallock_blacklist:
			make_parentallock_list(filename, PARENTALLOCK::blacklist, file);
		break;
		case FPORTS::single_parentallock_whitelist:
			make_parentallock_list(filename, PARENTALLOCK::whitelist, file);
		break;
		case FPORTS::single_parentallock_locked:
			make_parentallock_list(filename, PARENTALLOCK::locked, file);
		break;
		default:
		return error("export_file", "Error", "Unknown export option.");
	}

	string fpath = path;

	if (filename != file.filename)
	{
		std::filesystem::path fp = std::filesystem::path(path);
		string basedir = fp.parent_path().u8string();
		if (basedir.size() && basedir[basedir.size() - 1] != '/')
			basedir.append("/");

		fpath = basedir + file.filename;
	}

	if (! OVERWRITE_FILE && std::filesystem::exists(fpath))
	{
		return error("export_file", "File Error", "File \"" + fpath + "\" already exists.");
	}
	if
	(
		(std::filesystem::status(fpath).permissions() & std::filesystem::perms::owner_write) == std::filesystem::perms::none &&
		(std::filesystem::status(fpath).permissions() & std::filesystem::perms::group_write) == std::filesystem::perms::none
	)
	{
		return error("export _file", "File Error", "File \"" + path + "\" is not writable.");
	}

	ofstream out (fpath);
	out << file.data;
	out.close();
}

void e2db::add_transponder(transponder& tx)
{
	debug("add_transponder", "txid", tx.txid);

	if (tx.index == -1)
		tx.index = int (index["txs"].size()) + 1;

	e2db_abstract::add_transponder(tx.index, tx);
}

void e2db::edit_transponder(string txid, transponder& tx)
{
	debug("edit_transponder", "txid", txid);

	char nw_txid[25];
	// %4x:8x
	std::snprintf(nw_txid, 25, "%x:%x", tx.tsid, tx.dvbns);
	tx.txid = nw_txid;

	debug("edit_transponder", "new txid", tx.txid);

	if (tx.txid == txid)
	{
		db.transponders[tx.txid] = tx;
	}
	else
	{
		db.transponders.erase(txid);
		db.transponders.emplace(tx.txid, tx);

		for (auto it = index["txs"].begin(); it != index["txs"].end(); it++)
		{
			if (it->second == txid)
				it->second = tx.txid;
		}
	}
}

void e2db::remove_transponder(string txid)
{
	debug("remove_transponder", "txid", txid);

	db.transponders.erase(txid);

	for (auto it = index["txs"].begin(); it != index["txs"].end(); it++)
	{
		if (it->second == txid)
			index["txs"].erase(it);
	}
}

void e2db::add_service(service& ch)
{
	debug("add_service", "chid", ch.chid);

	if (ch.index == -1)
		ch.index = int (index["chs"].size()) + 1;

	e2db_abstract::add_service(ch.index, ch);
}

void e2db::edit_service(string chid, service& ch)
{
	debug("edit_service", "chid", chid);

	if (! db.services.count(chid))
		return error("edit_service", "Error", "Service \"" + chid + "\" not exists.");

	char nw_chid[25];
	char nw_txid[25];
	// %4x:%8x
	std::snprintf(nw_txid, 25, "%x:%x", ch.tsid, ch.dvbns);
	// %4x:%4x:%8x
	std::snprintf(nw_chid, 25, "%x:%x:%x", ch.ssid, ch.tsid, ch.dvbns);
	ch.txid = nw_txid;
	ch.chid = nw_chid;

	debug("edit_service", "new chid", ch.chid);

	if (ch.chid == chid)
	{
		db.services[ch.chid] = ch;
	}
	else
	{
		string kchid = 's' + chid;
		collisions.erase(kchid);

		if (db.services.count(ch.chid))
		{
			string kchid = 's' + ch.chid;
			int m = int (collisions[kchid].size());
			ch.chid += ':' + to_string(m);
			collisions[kchid].emplace_back(pair (ch.chid, m));
		}

		db.services.erase(chid);
		db.services.emplace(ch.chid, ch);

		for (auto & x : index)
		{
			for (auto it = x.second.begin(); it != x.second.end(); it++)
			{
				if (it->second == chid)
					it->second = ch.chid;
			}
		}
		for (auto & x : userbouquets)
		{
			if (x.second.channels.count(chid))
			{
				channel_reference chref = x.second.channels[chid];
				chref.chid = ch.chid;
				x.second.channels.erase(chid);
				x.second.channels.emplace(ch.chid, chref);
			}
		}
	}
}

void e2db::remove_service(string chid)
{
	debug("remove_service", "chid", chid);

	if (! db.services.count(chid))
		return error("remove_service", "Error", "Service \"" + chid + "\" not exists.");

	service ch = db.services[chid];
	string kchid = 's' + chid;
	db.services.erase(chid);

	for (auto & x : index)
	{
		for (auto it = x.second.begin(); it != x.second.end(); it++)
		{
			if (it->second == chid)
				x.second.erase(it);
		}
	}
	for (auto & x : userbouquets)
	{
		x.second.channels.erase(chid);
	}

	collisions.erase(kchid);
}

void e2db::add_bouquet(bouquet& bs)
{
	debug("add_bouquet", "bname", bs.bname);

	if (bs.index == -1)
		bs.index = int (index["bss"].size()) + 1;
	e2db_abstract::add_bouquet(bs.index, bs);
}

void e2db::edit_bouquet(bouquet& bs)
{
	debug("edit_bouquet", "bname", bs.bname);

	if (! bouquets.count(bs.bname))
		return error("edit_bouquet", "Error", "Bouquet \"" + bs.bname + "\" not exists.");

	bouquets[bs.bname] = bs;
}

void e2db::remove_bouquet(string bname)
{
	debug("remove_bouquet", "bname", bname);

	if (! bouquets.count(bname))
		return error("remove_bouquet", "Error", "Bouquet \"" + bname + "\" not exists.");

	for (auto it = index["bss"].begin(); it != index["bss"].end(); it++)
	{
		if (it->second == bname)
			index["bss"].erase(it);
	}
	for (auto it = index["ubs"].begin(); it != index["ubs"].end(); it++)
	{
		userbouquet ub = userbouquets[it->second];

		if (ub.pname == bname)
		{
			index["ubs"].erase(it);
			index.erase(ub.bname);

			userbouquets.erase(ub.bname);
		}
	}

	index.erase(bname);
	bouquets.erase(bname);
}

void e2db::add_userbouquet(userbouquet& ub)
{
	debug("add_userbouquet");

	bouquet bs = bouquets[ub.pname];

	if (ub.index == -1)
	{
		int idx = 0;
		string ktype;
		if (bs.btype == STYPE::tv)
			ktype = "tv";
		else if (bs.btype == STYPE::radio)
			ktype = "radio";

		if (index.count("ubs"))
		{
			for (auto it = index["ubs"].begin(); it != index["ubs"].end(); it++)
			{
				size_t pos0 = it->second.find(".dbe");
				size_t pos1 = it->second.find('.' + ktype);
				size_t len = it->second.length();
				int n = 0;
				if (pos0 != string::npos && pos1 != string::npos)
				{
					n = std::atoi(it->second.substr(pos0 + 4, len - pos1 - 1).data());
					idx = n > idx ? n : idx;
				}
			}
			idx = idx + 1;
		}

		ub.index = idx;
	}

	if (ub.bname.empty())
	{
		string ktype;
		if (bs.btype == STYPE::tv)
			ktype = "tv";
		else if (bs.btype == STYPE::radio)
			ktype = "radio";

		stringstream ub_bname;
		ub_bname << "userbouquet.dbe" << setfill('0') << setw(2) << ub.index << '.' << ktype;

		ub.bname = ub_bname.str();
	}

	e2db_abstract::add_userbouquet(ub.index, ub);
}

void e2db::edit_userbouquet(userbouquet& ub)
{
	debug("edit_userbouquet", "bname", ub.bname);

	if (! userbouquets.count(ub.bname))
		return error("edit_userbouquet", "Error", "Userbouquet \"" + ub.bname + "\" not exists.");

	userbouquets[ub.bname] = ub;
}

void e2db::remove_userbouquet(string bname)
{
	debug("remove_userbouquet", "bname", bname);

	if (! userbouquets.count(bname))
		return error("remove_userbouquet", "Error", "Userbouquet \"" + bname + "\" not exists.");

	userbouquet ub = userbouquets[bname];
	bouquet& bs = bouquets[ub.pname];

	for (auto it = index["ubs"].begin(); it != index["ubs"].end(); it++)
	{
		if (it->second == bname)
			index["ubs"].erase(it);
	}

	vector<string>::iterator pos;
	for (auto it = bs.userbouquets.begin(); it != bs.userbouquets.end(); it++)
	{
		if (*it == bname)
		{
			pos = it;
			break;
		}
	}
	if (pos != bs.userbouquets.end())
	{
		bs.userbouquets.erase(pos);
	}

	index.erase(ub.pname);
	bs.services.clear();

	int idx = 0;
	for (string & w : bs.userbouquets)
	{
		userbouquet ub = userbouquets[w];
		string bname = ub.bname;

		for (auto & x : index[bname])
		{
			channel_reference chref = ub.channels[x.second];

			if (! chref.marker && bs.services.count(chref.chid) == 0)
			{
				idx += 1;
				bs.services.emplace(chref.chid);
				index[ub.pname].emplace_back(pair (idx, chref.chid));
			}
		}
	}

	index.erase(bname);
	userbouquets.erase(bname);
}

void e2db::add_channel_reference(channel_reference& chref, string bname)
{
	debug("add_channel_reference", "chid", chref.chid);

	if (! userbouquets.count(bname))
		return error("add_channel_reference", "Error", "Userbouquet \"" + bname + "\" not exists.");

	userbouquet& ub = userbouquets[bname];
	service_reference ref;

	if (chref.marker)
	{
		if (! chref.atype)
		{
			chref.atype = STYPE::marker;
		}
		if (! chref.anum)
		{
			int anum_count = int (index["mks"].size());
			anum_count++;
			chref.anum = anum_count;
		}
		if (! chref.index)
		{
			int ub_idx = ub.index;
			chref.index = ub_idx;
		}
	}
	else
	{
		if (! db.services.count(chref.chid))
			return error("add_channel_reference", "Error", "Service \"" + chref.chid + "\" not exists.");

		service ch = db.services[chref.chid];

		ref.ssid = ch.ssid;
		ref.dvbns = ch.dvbns;
		ref.tsid = ch.tsid;
	}

	if (chref.index == -1 && ! chref.marker)
		chref.index = int (index[bname].size()) + 1;

	e2db_abstract::add_channel_reference(chref.index, ub, chref, ref);
}

void e2db::edit_channel_reference(string chid, channel_reference& chref, string bname)
{
	debug("edit_channel_reference", "chid", chid);

	if (! userbouquets.count(bname))
		return error("edit_channel_reference", "Error", "Userbouquet \"" + bname + "\" not exists.");

	userbouquet& ub = userbouquets[bname];

	debug("edit_channel_reference", "new chid", chref.chid);

	if (chref.chid == chid)
	{
		if (! chref.marker && ! db.services.count(chref.chid))
			return error("edit_channel_reference", "Error", "Service \"" + chref.chid + "\" not exists.");

		ub.channels[chref.chid] = chref;
	}
	else
	{
		service_reference ref;

		if (! chref.marker)
		{
			if (! db.services.count(chref.chid))
				return error("edit_channel_reference", "Error", "Service \"" + chref.chid + "\" not exists.");

			service ch = db.services[chref.chid];

			ref.ssid = ch.ssid;
			ref.dvbns = ch.dvbns;
			ref.tsid = ch.tsid;
		}

		ub.channels.erase(chid);
		ub.channels.emplace(chref.chid, chref);

		for (auto it = index[bname].begin(); it != index[bname].end(); it++)
		{
			if (it->second == chid)
				it->second = chref.chid;
		}

		if (chref.marker)
		{
			for (auto it = index["mks"].begin(); it != index["mks"].end(); it++)
			{
				if (it->second == chid)
					it->second = chref.chid;
			}
		}
		else
		{
			for (auto it = index[ub.pname].begin(); it != index[ub.pname].end(); it++)
			{
				if (it->second == chid)
					it->second = chref.chid;
			}
		}
	}
}

void e2db::remove_channel_reference(channel_reference chref, string bname)
{
	debug("remove_channel_reference", "chref.chid", chref.chid);

	if (! userbouquets.count(bname))
		return error("remove_channel_reference", "Error", "Userbouquet \"" + bname + "\" not exists.");

	userbouquet& ub = userbouquets[bname];
	int idx = -1;
	string chid;

	for (auto & x : userbouquets[bname].channels)
	{
		if (x.second.index == idx && x.second.chid == chid)
		{
			idx = x.second.index;
			chid = x.first;
			break;
		}
	}

	if (! userbouquets[bname].channels.count(chid))
		return error("remove_channel_reference", "Error", "Channel reference \"" + chid + "\" not exists.");

	vector<pair<int, string>>::iterator pos;
	for (auto it = index[bname].begin(); it != index[bname].end(); it++)
	{
		if (it->first == idx && it->second == chid)
		{
			pos = it;
			break;
		}
	}
	if (pos != index[bname].end())
	{
		index[bname].erase(pos);
	}
	if (chref.marker)
	{
		vector<pair<int, string>>::iterator pos;
		for (auto it = index["mks"].begin(); it != index["mks"].end(); it++)
		{
			if (it->second == chid)
			{
				pos = it;
				break;
			}
		}
		if (pos != index["mks"].end())
		{
			index["mks"].erase(pos);
		}
	}
	else
	{
		vector<pair<int, string>>::iterator pos;
		for (auto it = index[ub.pname].begin(); it != index[ub.pname].end(); it++)
		{
			if (it->second == chid)
			{
				pos = it;
				break;
			}
		}
		if (pos != index[ub.pname].end())
		{
			index[ub.pname].erase(pos);
		}
	}

	{
		unordered_map<string, channel_reference>::iterator pos;
		for (auto it = userbouquets[bname].channels.begin(); it != userbouquets[bname].channels.end(); it++)
		{
			if (it->second.index == idx && it->second.chid == chid)
			{
				pos = it;
				break;
			}
		}
		if (pos != userbouquets[bname].channels.end())
		{
			ub.channels.erase(pos);
		}
	}
}

void e2db::remove_channel_reference(string chid, string bname)
{
	debug("remove_channel_reference", "chid", chid);

	if (! userbouquets.count(bname))
		return error("remove_channel_reference", "Error", "Userbouquet \"" + bname + "\" not exists.");
	if (! userbouquets[bname].channels.count(chid))
		return error("remove_channel_reference", "Error", "Channel reference \"" + chid + "\" not exists.");

	channel_reference chref = userbouquets[bname].channels[chid];
	userbouquet& ub = userbouquets[bname];

	vector<pair<int, string>>::iterator pos;
	for (auto it = index[bname].begin(); it != index[bname].end(); it++)
	{
		if (it->second == chid)
		{
			pos = it;
			break;
		}
	}
	if (pos != index[bname].end())
	{
		index[bname].erase(pos);
	}

	if (chref.marker)
	{
		vector<pair<int, string>>::iterator pos;
		for (auto it = index["mks"].begin(); it != index["mks"].end(); it++)
		{
			if (it->second == chid)
			{
				pos = it;
				break;
			}
		}
		if (pos != index["mks"].end())
		{
			index["mks"].erase(pos);
		}
	}
	else
	{
		vector<pair<int, string>>::iterator pos;
		for (auto it = index[ub.pname].begin(); it != index[ub.pname].end(); it++)
		{
			if (it->second == chid)
			{
				pos = it;
				break;
			}
		}
		if (pos != index[ub.pname].end())
		{
			index[ub.pname].erase(pos);
		}
	}

	ub.channels.erase(chid);
}

void e2db::add_tunersets(tunersets& tv)
{
	debug("add_tunersets", "tvid", tv.ytype);

	if (tv.charset.empty())
		tv.charset = "utf-8";

	e2db_abstract::add_tunersets(tv);
}

void e2db::edit_tunersets(int tvid, tunersets& tv)
{
	debug("edit_tunersets", "tvid", tvid);

	if (tv.charset.empty())
		tv.charset = "utf-8";

	tuners[tvid] = tv;
}

void e2db::remove_tunersets(int tvid)
{
	debug("remove_tunersets", "tvid", tvid);

	tuners.erase(tvid);
}

void e2db::add_tunersets_table(tunersets_table& tn, tunersets tv)
{
	debug("add_tunersets_table", "tnid", tn.tnid);

	string iname = "tns:";
	char yname = value_transponder_type(tn.ytype);
	iname += yname;

	if (tn.index == -1)
		tn.index = index.count(iname) ? int (index[iname].size()) : 0;

	e2db_abstract::add_tunersets_table(tn.index, tn, tv);
	tuners[tv.ytype].tables[tn.tnid] = tn;
}

void e2db::edit_tunersets_table(string tnid, tunersets_table& tn, tunersets tv)
{
	debug("edit_tunersets_table", "tnid", tnid);

	string iname = "tns:";
	char yname = value_transponder_type(tn.ytype);
	iname += yname;

	char nw_tnid[25];
	std::snprintf(nw_tnid, 25, "%c:%04x", yname, tn.index);
	tn.tnid = nw_tnid;

	debug("edit_tunersets_table", "new tnid", tn.tnid);

	if (tn.tnid == tnid)
	{
		tuners[tv.ytype].tables[tn.tnid] = tn;
	}
	else
	{
		tuners[tv.ytype].tables.erase(tnid);
		tuners[tv.ytype].tables.emplace(tn.tnid, tn);

		for (auto it = index[iname].begin(); it != index[iname].end(); it++)
		{
			if (it->second == tnid)
				it->second = tn.tnid;
		}

		if (tn.ytype == YTYPE::satellite)
		{
			tuners_pos.erase(tn.pos);
			tuners_pos.emplace(tn.pos, tn.tnid);
		}
	}
}

void e2db::remove_tunersets_table(string tnid, tunersets tv)
{
	debug("remove_tunersets_table", "tnid", tnid);

	if (! tv.tables.count(tnid))
		return error("remove_tunersets_table", "Error", "Tunersets table \"" + tnid + "\" not exists.");

	tunersets_table tn = tv.tables[tnid];

	string iname = "tns:";
	char yname = value_transponder_type(tn.ytype);
	iname += yname;

	vector<pair<int, string>>::iterator pos;
	for (auto it = index[iname].begin(); it != index[iname].end(); it++)
	{
		if (it->second == tnid)
		{
			pos = it;
			break;
		}
	}
	if (pos != index[iname].end())
	{
		index[iname].erase(pos);
	}

	if (tn.ytype == YTYPE::satellite)
	{
		tuners_pos.erase(tn.pos);
	}

	tuners[tv.ytype].tables.erase(tnid);
}

void e2db::add_tunersets_transponder(tunersets_transponder& tntxp, tunersets_table tn)
{
	debug("add_tunersets_transponder", "trid", tntxp.trid);

	if (tntxp.index == -1)
		tntxp.index = int (index[tn.tnid].size()) + 1;

	e2db_abstract::add_tunersets_transponder(tntxp.index, tntxp, tn);
	tuners[tn.ytype].tables[tn.tnid].transponders[tntxp.trid] = tntxp;
}

void e2db::edit_tunersets_transponder(string trid, tunersets_transponder& tntxp, tunersets_table tn)
{
	debug("edit_tunersets_transponder", "trid", trid);

	char yname = value_transponder_type(tn.ytype);

	char nw_trid[25];
	std::snprintf(nw_trid, 25, "%c:%04x:%04x", yname, tntxp.freq, tntxp.sr);
	tntxp.trid = nw_trid;

	debug("edit_tunersets_transponder", "new trid", tntxp.trid);

	if (tntxp.trid == trid)
	{
		tuners[tn.ytype].tables[tn.tnid].transponders[tntxp.trid] = tntxp;
	}
	else
	{
		tuners[tn.ytype].tables[tn.tnid].transponders.erase(trid);
		tuners[tn.ytype].tables[tn.tnid].transponders.emplace(tntxp.trid, tntxp);

		for (auto it = index[tn.tnid].begin(); it != index[tn.tnid].end(); it++)
		{
			if (it->second == trid)
				it->second = tntxp.trid;
		}
	}
}

void e2db::set_service_parentallock(string chid)
{
	debug("set_service_parentallock", "chid", chid);

	if (! db.services.count(chid))
		return error("set_service_parentallock", "Error", "Service \"" + chid + "\" not exists.");

	service& ch = db.services[chid];
	ch.locked = true;
}

void e2db::unset_service_parentallock(string chid)
{
	debug("unset_service_parentallock", "chid", chid);

	if (! db.services.count(chid))
		return error("unset_service_parentallock", "Error", "Service \"" + chid + "\" not exists.");

	service& ch = db.services[chid];
	ch.locked = false;
}

void e2db::set_userbouquet_parentallock(string bname)
{
	debug("set_userbouquet_parentallock", "bname", bname);

	if (! userbouquets.count(bname))
		return error("set_userbouquet_parentallock", "Error", "Userbouquet \"" + bname + "\" not exists.");

	userbouquet& ub = userbouquets[bname];
	ub.locked = true;
}

void e2db::unset_userbouquet_parentallock(string bname)
{
	debug("unset_userbouquet_parentallock", "bname", bname);

	if (! userbouquets.count(bname))
		return error("unset_userbouquet_parentallock", "Error", "Userbouquet \"" + bname + "\" not exists.");

	userbouquet& ub = userbouquets[bname];
	ub.locked = false;
}

string e2db::get_filepath()
{
	debug("get_filepath");

	return this->filepath;
}

string e2db::get_services_filename()
{
	debug("get_services_filename");

	return this->services_filename;
}

void e2db::remove_tunersets_transponder(string trid, tunersets_table tn)
{
	debug("remove_tunersets_transponder", "trid", trid);

	tuners[tn.ytype].tables[tn.tnid].transponders.erase(trid);

	vector<pair<int, string>>::iterator pos;
	for (auto it = index[tn.tnid].begin(); it != index[tn.tnid].end(); it++)
	{
		if (it->second == trid)
		{
			pos = it;
			break;
		}
	}
	if (pos != index[tn.tnid].end())
	{
		index[tn.tnid].erase(pos);
	}
}

map<string, vector<pair<int, string>>> e2db::get_channels_index()
{
	debug("get_channels_index");

	map<string, vector<pair<int, string>>> _index;

	for (auto & x : index["chs"])
	{
		service ch = db.services[x.second];
		transponder tx = db.transponders[ch.txid];
		_index[to_string(tx.pos)].emplace_back(x);
		_index[ch.txid].emplace_back(x);
	}

	return _index;
}

map<string, vector<pair<int, string>>> e2db::get_transponders_index()
{
	debug("get_transponders_index");

	map<string, vector<pair<int, string>>> _index;
	unordered_set<string> _unique;

	for (auto & x : index["txs"])
	{
		transponder tx = db.transponders[x.second];

		if (! _unique.count(x.second))
		{
			_index[to_string(tx.pos)].emplace_back(x);
			_unique.insert(x.second);
		}
	}

	return _index;
}

map<string, vector<pair<int, string>>> e2db::get_services_index()
{
	debug("get_services_index");

	map<string, vector<pair<int, string>>> _index;
	_index["chs"] = index["chs"];

	return _index;
}

map<string, vector<pair<int, string>>> e2db::get_bouquets_index()
{
	debug("get_bouquets_index");

	map<string, vector<pair<int, string>>> _index;

	for (auto & x : index["bss"])
		_index[x.second] = index[x.second];

	return _index;
}

map<string, vector<pair<int, string>>> e2db::get_userbouquets_index()
{
	debug("get_userbouquets_index");

	map<string, vector<pair<int, string>>> _index;

	for (auto & x : index["ubs"])
		_index[x.second] = index[x.second];

	return _index;
}

map<string, vector<pair<int, string>>> e2db::get_packages_index()
{
	debug("get_packages_index");

	map<string, vector<pair<int, string>>> _index;

	for (auto & x : index["chs"])
	{
		service ch = db.services[x.second];

		if (ch.data.count(SDATA::p))
		{
			string pvdrn = ch.data[SDATA::p][0];

			if (pvdrn.empty()) _index["(Unknown)"].emplace_back(x);
			else _index[pvdrn].emplace_back(x);
		}
	}

	return _index;
}

map<string, vector<pair<int, string>>> e2db::get_resolution_index()
{
	debug("get_resolution_index");

	map<string, vector<pair<int, string>>> _index;

	for (auto & x : index["chs"])
	{
		service ch = db.services[x.second];
		_index[to_string(ch.stype)].emplace_back(x);
	}

	return _index;
}

map<string, vector<pair<int, string>>> e2db::get_encryption_index()
{
	debug("get_encryption_index");

	map<string, vector<pair<int, string>>> _index;
	unordered_set<string> _unique;

	for (auto & x : index["chs"])
	{
		service ch = db.services[x.second];

		if (ch.data.count(SDATA::C))
		{
			for (string & w : ch.data[SDATA::C])
			{
				string caidpx = w.substr(0, 2);
				string cx = caidpx + '|' + to_string(x.first);
				if (SDATA_CAS.count(caidpx) && ! _unique.count(cx))
				{
					_index[SDATA_CAS.at(caidpx)].emplace_back(x);
					_unique.insert(cx);
				}
			}
		}
		else
		{
			_index["(FTA)"].emplace_back(x);
		}
	}

	return _index;
}

map<string, vector<pair<int, string>>> e2db::get_az_index()
{
	debug("get_az_index");

	map<string, vector<pair<int, string>>> _index;

	for (auto & x : index["chs"])
	{
		service ch = db.services[x.second];
		string lt = ch.chname.substr(0, 1);

		if (isdigit(lt[0])) _index["0-9"].emplace_back(x);
		else _index[lt].emplace_back(x);
	}

	return _index;
}

void e2db::merge(e2db_abstract* dst)
{
	debug("merge");

	std::clock_t start = std::clock();

	unordered_map<string, vector<pair<int, string>>> index;

	this->db.transponders.merge(dst->db.transponders);

	this->db.services.merge(dst->db.services);

	for (auto & x : dst->collisions)
	{
		string kchid = x.first;

		if (this->collisions.count(kchid))
		{
			for (auto & x : x.second)
			{
				int m = int (this->collisions[kchid].size());
				collisions[kchid].emplace_back(pair (x.first, m));
			}
		}
		else
		{
			this->collisions.emplace(x);
		}
	}

	for (auto & x : dst->db.services)
	{
		if (this->db.services.count(x.first))
		{
			if (x.second.data != this->db.services[x.first].data)
				this->db.services[x.first].data.merge(x.second.data);
		}
	}

	this->tuners.merge(dst->tuners);
	this->tuners_pos.clear();

	if (this->tuners.count(YTYPE::satellite))
	{
		for (auto & x : tuners[YTYPE::satellite].tables)
			this->tuners_pos.emplace(x.second.pos, x.second.tnid);
	}

	vector<string> i_names;

	for (auto & x : dst->index)
	{
		string iname = x.first;

		if (iname.find("bouquet") != string::npos)
			continue;
		else if (iname.find("chs:") != string::npos)
			continue;
		else if (iname == "bss")
			continue;
		else if (iname == "ubs")
			continue;
		else if (iname == "mks")
			continue;
		else
			i_names.emplace_back(iname);
	}

	for (string & iname : i_names)
	{
		if (this->index.count(iname)) // merge
		{
			unordered_set<string> i_keys;
			vector<pair<int, string>> i_diff;

			for (auto & x : this->index[iname])
			{
				i_keys.emplace(x.second);
			}

			std::copy_if(dst->index[iname].begin(), dst->index[iname].end(), std::inserter(i_diff, i_diff.begin()), [=] (pair<int, string> x) {
				return ! i_keys.count(x.second);
			});

			index[iname] = this->index[iname];
			index[iname].insert(index[iname].end(), i_diff.begin(), i_diff.end());

			int idx = 0;
			for (auto & x : index[iname])
			{
				idx += 1;
				x.first = idx;
			}
		}
		else // append
		{
			index.emplace(iname, dst->index[iname]);
		}
	}

	if (0) // order by tsid|ssid	elapsed time: 84290 ms.
	{
		unordered_map<string, int> txs;
		vector<pair<int, string>> i_chs;

		int idx = 0;
		for (auto & x : index["txs"])
		{
			transponder tx = db.transponders[x.second];

			if (! txs[tx.txid])
			{
				for (auto & x : index["chs"])
				{
					service ch = db.services[x.second];

					if (ch.txid != tx.txid)
						continue;

					string iname = "chs:" + (STYPE_EXT_TYPE.count(ch.stype) ? to_string(STYPE_EXT_TYPE.at(ch.stype)) : "0");
					idx += 1;
					x.first = idx;
					index[iname].emplace_back(x);
					i_chs.emplace_back(x);
				}

				txs[tx.txid]++;
			}
		}

		index["chs"].swap(i_chs);
	}
	else  // unordered append		elapsed time: 11086 ms.
	{
		for (auto & x : index["chs"])
		{
			service ch = this->db.services[x.second];
			string iname = "chs:" + (STYPE_EXT_TYPE.count(ch.stype) ? to_string(STYPE_EXT_TYPE.at(ch.stype)) : "0");
			index[iname].emplace_back(x);
		}
	}

	index["bss"] = this->index["bss"];
	index["ubs"] = this->index["ubs"];

	unordered_map<string, string> ubs_names;
	bool bss_epl = false;

	for (auto & x : this->bouquets)
	{
		string bname = x.second.bname;

		if (bname.rfind(".epl") != string::npos)
		{
			bss_epl = true;
			break;
		}
	}

	for (auto & x : this->userbouquets)
	{
		string bname = x.second.bname;
		bouquet bs = this->bouquets[x.second.pname];

		string qw;
		string ktype;
		if (bs.btype == STYPE::tv)
			ktype = "tv";
		else if (bs.btype == STYPE::radio)
			ktype = "radio";

		if (bname.rfind(".favourites") != string::npos)
			qw = bname;
		else
			qw = x.second.name + '$' + ktype;

		ubs_names.emplace(qw, bname);
		index[bname] = this->index[bname];
	}

	for (auto & x : dst->index["ubs"])
	{
		userbouquet ub = dst->userbouquets[x.second];
		bouquet bs = dst->bouquets[ub.pname];

		string bname = ub.bname;
		string pname;
		string iname = bname;
		int ub_idx = -1;

		string qw;
		string ktype;
		if (bs.btype == STYPE::tv)
		{
			ktype = "tv";
			pname = bss_epl ? "userbouquets.tv.epl" : "bouquets.tv";
		}
		else if (bs.btype == STYPE::radio)
		{
			ktype = "radio";
			pname = bss_epl ? "userbouquets.radio.epl" : "bouquets.radio";
		}

		qw = ub.name + '$' + ktype;

		ub_idx = int (this->bouquets[pname].userbouquets.size());

		bool merge = false;

		if (ubs_names.count(qw)) // merge
		{
			merge = true;
			bname = ubs_names[qw];

			this->userbouquets[bname].channels.merge(ub.channels);
			ub = this->userbouquets[bname];

			unordered_set<string> i_keys;
			vector<pair<int, string>> i_diff;

			for (auto & i : index[bname])
			{
				i_keys.emplace(i.second);
			}

			std::copy_if(dst->index[iname].begin(), dst->index[iname].end(), std::inserter(i_diff, i_diff.begin()), [=] (pair<int, string> x) {
				return ! i_keys.count(x.second);
			});

			index[bname].insert(index[bname].end(), i_diff.begin(), i_diff.end());
		}
		else if (this->userbouquets.count(bname)) // rename append
		{
			ub_idx += 1;

			stringstream ub_bname;
			ub_bname << "userbouquet.dbe" << setfill('0') << setw(2) << ub_idx << '.' << ktype;

			ub.bname = bname = ub_bname.str();
			ub.index = ub_idx;

			this->bouquets[pname].userbouquets.emplace_back(bname);
			this->userbouquets.emplace(bname, ub);
			index[bname] = dst->index[iname];
			index["ubs"].emplace_back(pair (ub_idx, bname));
		}
		else // append
		{
			ub_idx += 1;
			ub.index = ub_idx;

			this->bouquets[pname].userbouquets.emplace_back(bname);
			this->userbouquets.emplace(bname, ub);
			index[bname] = dst->index[iname];
			index["ubs"].emplace_back(pair (ub_idx, bname));
		}

		int idx = 0;
		for (auto & x : index[bname])
		{
			channel_reference& chref = ub.channels[x.second];

			if (! chref.marker)
			{
				if (merge)
				{
					idx += 1;
					chref.index = idx;
					x.first = chref.index;
				}
				if (this->bouquets[pname].services.count(chref.chid) == 0)
				{
					int idx = int (index[pname].size());
					idx += 1;
					this->bouquets[pname].services.emplace(chref.chid);
					index[pname].emplace_back(pair (idx, chref.chid));
				}
			}
		}
	}

	int i = 0;
	for (auto & x : index["ubs"])
	{
		userbouquet& ub = this->userbouquets[x.second];
		string bname = ub.bname;
		unordered_map<string, channel_reference> channels;

		for (auto & x : index[bname])
		{
			channel_reference& chref = ub.channels[x.second];

			if (chref.marker)
			{
				char chid[25];
				// %4d:%2x:%d
				std::snprintf(chid, 25, "%d:%x:%d", chref.atype, i, ub.index);
				chref.chid = chid;
				chref.anum = i;
				x.second = chref.chid;
				index["mks"].emplace_back(pair (ub.index, chid));
				i++;
			}

			channels.emplace(chref.chid, chref);
		}

		ub.channels.swap(channels);
	}

	this->index.swap(index);

	std::clock_t end = std::clock();

	info("merge", "elapsed time", to_string(int (end - start)) + " ms.");
}

}
