/*!
 * e2-sat-editor/src/e2db/e2db.cpp
 * 
 * @link https://github.com/ctlcltd/e2-sat-editor
 * @copyright e2 SAT Editor Team
 * @author Leonardo Laureti
 * @version 0.1
 * @license MIT License
 * @license GNU GPLv3 License
 */

#include <algorithm>
#include <unordered_set>
#include <sstream>
#include <fstream>
#include <iostream>
#include <iomanip>

#include "e2db.h"

using std::unordered_set, std::set_difference, std::inserter, std::stringstream, std::ifstream, std::ofstream, std::streamsize, std::numeric_limits, std::to_string, std::setfill, std::setw;

namespace e2se_e2db
{

e2db::e2db()
{
	this->log = new e2se::logger("e2db");

	debug("e2db()");
}

//TODO bname destructive edit
//TODO bname non-destructive edit
void e2db::merge(e2db* dbih)
{
	debug("merge()");

	db.transponders.merge(dbih->db.transponders); //C++17
	db.services.merge(dbih->db.services); //C++17
	tuners.merge(dbih->tuners); //C++17
	bouquets.merge(dbih->bouquets); //C++17
	collisions.merge(dbih->collisions); //C++17

	unordered_map<string, vector<pair<int, string>>> index;
	unordered_map<string, vector<pair<int, string>>> cp_index_0 = this->index;
	unordered_map<string, vector<pair<int, string>>> cp_index_1 = dbih->index;
	unordered_map<string, userbouquet> cp_ubs_0;
	unordered_map<string, userbouquet> cp_ubs_1;
	cp_index_0.erase("mks");
	cp_index_1.erase("mks");
	tuners_pos.clear();

	//TODO refresh cached data
	/*for (auto & chdata : db.services)
	{
	}*/

	for (auto & i : cp_index_0["ubs"])
	{
		userbouquet& ub = userbouquets[i.second];
		bouquet& bs = bouquets[ub.pname];
		string key = "1:7:" + to_string(bs.btype) + ':' + ub.name;
		auto iub = cp_index_0[i.second];
		cp_index_0[key] = iub;
		cp_index_0.erase(i.second);
		cp_ubs_0[key] = ub;
		// cout << "cp_index_0 " << i.second << ' ' << cp_index_0[key].size() << ' ' << key << endl;
		i.second = key;
	}
	for (auto & i : cp_index_1["ubs"])
	{
		userbouquet& ub = dbih->userbouquets[i.second];
		bouquet& bs = dbih->bouquets[ub.pname];
		string key = "1:7:" + to_string(bs.btype) + ':' + ub.name;
		auto iub = cp_index_1[i.second];
		cp_index_1[key] = iub;
		cp_index_1.erase(i.second);
		cp_ubs_1[key] = ub;
		// cout << "cp_index_1 " << i.second << ' ' << cp_index_1[key].size() << ' ' << key << endl;
		i.second = key;
	}

	index = cp_index_1;
	for (auto & i : cp_index_0)
	{
		vector<pair<int, string>> i_diff;
		set_difference(i.second.begin(), i.second.end(), cp_index_1[i.first].begin(), cp_index_1[i.first].end(), inserter(i_diff, i_diff.begin()));
		index[i.first].insert(index[i.first].end(), i_diff.begin(), i_diff.end());
		// cout << "i_diff " << i.first << ' ' << i_diff.size() << endl;
	}

	cp_ubs_0.merge(cp_ubs_1);
	userbouquets.clear();

	for (auto & bsdata : bouquets)
	{
		bsdata.second.userbouquets.clear();
	}
	for (auto & i : index["ubs"])
	{
		userbouquet& ub = cp_ubs_0[i.second];
		bouquet& bs = bouquets[ub.pname];
		//TODO "userbouquet.dbe.01234.tv"
		int idx = bs.userbouquets.size();
		string key = "1:7:" + to_string(bs.btype) + ':' + ub.name;
		string ktype;
		if (bs.btype == 1)
			ktype = "tv";
		else if (bs.btype == 2)
			ktype = "radio";
		//cout << ktype << ' ' << i.second << endl;

		stringstream bname;
		bname << "userbouquet.dbe" << setfill('0') << setw(2) << idx << '.' << ktype;

		ub.bname = bname.str();
		ub.index = idx;

		if (cp_ubs_1.count(i.second))
			ub.channels.merge(cp_ubs_1[i.second].channels);

		index[ub.bname] = index[key];
		index.erase(key);

		idx = 0;
		for (auto & x : index[ub.bname])
		{
			channel_reference& chref = ub.channels[x.second];

			if (chref.marker)
			{
				char chid[25];
				// %4d:%2x:%d
				std::sprintf(chid, "%d:%x:%d", chref.type, chref.anum, ub.index);
				chref.chid = chid;
				index["mks"].emplace_back(pair (ub.index, chid)); //C++17
			}
			else
			{
				idx += 1;
				chref.index = idx;
				x.first = chref.index;
			}
		}

		bs.userbouquets.emplace_back(ub.bname);
		userbouquets[ub.bname] = ub;
		i.first = ub.index;
		i.second = ub.bname;
		// cout << "index " << ub.bname << ' ' << index[key].size() << ' ' << key << endl;
	}

	index.erase("chs:0");
	index.erase("chs:1");
	index.erase("chs:2");
	for (auto & i : index)
	{
		if (i.first == "txs" || i.first == "chs" || i.first.find("bouquets.") != string::npos)
		{
			int idx = 0;
			for (auto & x : i.second)
			{
				idx += 1;
				x.first = idx;
			}
			// cout << i.first << ' ' << i.second.size() << endl;
		}
	}
	for (auto & i : index["chs"])
	{
		service& ch = db.services[i.second];
		string iname = "chs:" + (STYPES.count(ch.stype) ? to_string(STYPES.at(ch.stype).first) : "0");
		index[iname].emplace_back(pair (i.first, ch.chid)); //C++17
	}

	if (tuners.count(YTYPE::sat))
	{
		for (auto & x : tuners[YTYPE::sat].tables)
			tuners_pos.emplace(x.second.pos, x.second.tnid);
	}

	this->index = index;

	//TODO mem
	cp_index_0.clear();
	cp_index_1.clear();
	cp_ubs_0.clear();
	cp_ubs_1.clear();
	index.clear();
}

void e2db::import_file(vector<string> paths)
{
	debug("import_file()", "",  "0");

	bool merge = this->get_input().size() != 0 ? true : false;
	e2db* dbih = merge ? new e2db : this;

	for (string & w : paths)
	{
		FPORTS fpi = filetype_detect(w);
		ifstream ifile (w);

		e2db_file file;
		string line;
		while (std::getline(ifile, line))
			file.append(line + '\n');
		import_file(fpi, dbih, file, w);
	}
	if (merge)
	{
		this->merge(dbih);
		delete dbih;
	}
}

void e2db::import_file(FPORTS fpi, e2db* dbih, e2db_file file, string path)
{
	debug("import_file()", "", "1");

	string filename = std::filesystem::path(path).filename().u8string(); //C++17
	stringstream ifile;
	ifile.write(&file[0], file.size());

	switch (fpi)
	{
		case FPORTS::allServices:
			dbih->parse_e2db_lamedb(ifile);
		break;
		case FPORTS::allServices2_2:
		case FPORTS::allServices2_3:
			return error("import_file()", "Error", "Unsupported services file format.");
		break;
		case FPORTS::allServices2_4:
			dbih->parse_e2db_lamedb4(ifile);
		break;
		case FPORTS::allServices2_5:
			dbih->parse_e2db_lamedb5(ifile);
		break;
		case FPORTS::singleTunersets:
		case FPORTS::allTunersets:
			if (filename == "satellites.xml")
				dbih->parse_tunersets_xml(YTYPE::sat, ifile);
			else if (filename == "terrestrial.xml")
				dbih->parse_tunersets_xml(YTYPE::terrestrial, ifile);
			else if (filename == "cables.xml")
				dbih->parse_tunersets_xml(YTYPE::cable, ifile);
			else if (filename == "atsc.xml")
				dbih->parse_tunersets_xml(YTYPE::atsc, ifile);
		break;
		case FPORTS::singleBouquet:
		case FPORTS::allBouquets:
			dbih->parse_e2db_bouquet(ifile, filename);
		break;
		case FPORTS::singleUserbouquet:
		case FPORTS::allUserbouquets:
			dbih->parse_e2db_userbouquet(ifile, filename);
		break;
		case FPORTS::singleBouquetAll:
			if (filetype_detect(filename) == FPORTS::singleBouquet)
				dbih->parse_e2db_bouquet(ifile, filename);
			else
				dbih->parse_e2db_userbouquet(ifile, filename);
		break;
		case FPORTS::_default:
			dbih->read(path);
			return;
		break;
	}
}

void e2db::export_file(vector<string> paths)
{
	debug("export_file()", "", "0");

	for (string & w : paths)
	{
		FPORTS fpo = filetype_detect(w);
		export_file(fpo, w);
	}
}

void e2db::export_file(FPORTS fpo, vector<string> paths)
{
	debug("export_file()", "", "1");

	for (string & w : paths)
	{
		export_file(fpo, w);
	}
}

void e2db::export_file(FPORTS fpo, string path)
{
	debug("export_file()", "", "2");

	e2db_file file;
	string filename = std::filesystem::path(path).filename().u8string(); //C++17

	switch (fpo)
	{
		case FPORTS::allServices:
			if (LAMEDB_VER == 4)
				file = make_lamedb4("lamedb");
			else if (LAMEDB_VER == 5)
				file = make_lamedb5("lamedb5");
		break;
		case FPORTS::allServices2_2:
		case FPORTS::allServices2_3:
			return error("export_file()", "Error", "Unsupported services file format.");
		break;
		case FPORTS::allServices2_4:
			file = make_lamedb4("lamedb");
		break;
		case FPORTS::allServices2_5:
			file = make_lamedb5("lamedb5");
		break;
		case FPORTS::singleTunersets:
		case FPORTS::allTunersets:
			if (filename == "satellites.xml")
				file = make_tunersets_xml(filename, YTYPE::sat);
			else if (filename == "terrestrial.xml")
				file = make_tunersets_xml(filename, YTYPE::terrestrial);
			else if (filename == "cables.xml")
				file = make_tunersets_xml(filename, YTYPE::cable);
			else if (filename == "atsc.xml")
				file = make_tunersets_xml(filename, YTYPE::atsc);
		break;
		case FPORTS::singleBouquet:
		case FPORTS::allBouquets:
			file = make_bouquet(filename);
		break;
		case FPORTS::singleUserbouquet:
		case FPORTS::allUserbouquets:
			file = make_userbouquet(filename);
		break;
		case FPORTS::singleBouquetAll:
			if (filetype_detect(filename) == FPORTS::singleBouquet)
				file = make_bouquet(filename);
			else
				file = make_userbouquet(filename);
		break;
		case FPORTS::_default:
			write(path, false);
			return;
		break;
	}

	ofstream out (path);
	out << file;
	out.close();
}

e2db::FPORTS e2db::filetype_detect(string path)
{
	string filename = std::filesystem::path(path).filename().u8string();

	if (filename == "lamedb")
		return FPORTS::allServices; // autodetect
	else if (filename == "lamedb5")
		return FPORTS::allServices2_5;
	else if (filename == "services")
		return FPORTS::allServices; // autodetect
	else if (filename == "satellites.xml")
		return FPORTS::singleTunersets;
	else if (filename == "terrestrial.xml")
		return FPORTS::singleTunersets;
	else if (filename == "cables.xml")
		return FPORTS::singleTunersets;
	else if (filename == "atsc.xml")
		return FPORTS::singleTunersets;
	else if (filename.find("bouquets.") != string::npos)
		return FPORTS::singleBouquet;
	else if (filename.find("userbouquet.") != string::npos)
		return FPORTS::singleUserbouquet;
	return FPORTS::_default;
}

void e2db::add_transponder(transponder& tx)
{
	debug("add_transponder()", "txid", tx.txid);

	tx.index = index.count("txs");
	e2db_abstract::add_transponder(tx.index, tx);
}

void e2db::edit_transponder(string txid, transponder& tx)
{
	debug("edit_transponder()", "txid", txid);

	char nw_txid[25];
	// %4x:8x
	std::sprintf(nw_txid, "%x:%x", tx.tsid, tx.dvbns);
	tx.txid = nw_txid;

	debug("edit_service()", "nw_txid", tx.txid);

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
	debug("remove_transponder()", "txid", txid);

	db.transponders.erase(txid);

	for (auto it = index["txs"].begin(); it != index["txs"].end(); it++)
	{
		if (it->second == txid)
			index["txs"].erase(it);
	}
}

void e2db::add_service(service& ch)
{
	debug("add_service()", "chid", ch.chid);

	ch.index = index.count("chs");
	e2db_abstract::add_service(ch.index, ch);
}

void e2db::edit_service(string chid, service& ch)
{
	debug("edit_service()", "chid", chid);

	if (! db.services.count(chid))
		return error("edit_service()", "Error", "Service \"" + chid + "\" not exists.");

	char nw_chid[25];
	char nw_txid[25];
	// %4x:%8x
	std::sprintf(nw_txid, "%x:%x", ch.tsid, ch.dvbns);
	// %4x:%4x:%8x
	std::sprintf(nw_chid, "%x:%x:%x", ch.ssid, ch.tsid, ch.dvbns);
	ch.txid = nw_txid;
	ch.chid = nw_chid;

	debug("edit_service()", "nw_chid", ch.chid);

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
			int m;
			string kchid = 's' + ch.chid;
			if (ch.snum) m = ch.snum;
			else m = collisions[kchid].size();
			ch.chid += ':' + to_string(m);
			collisions[kchid].emplace_back(pair (ch.chid, m)); //C++17
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
	debug("remove_service()", "chid", chid);

	if (! db.services.count(chid))
		return error("edit_service()", "Error", "Service \"" + chid + "\" not exists.");

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
	debug("add_bouquet()", "bname", bs.bname);

	e2db_abstract::add_bouquet(bs.index, bs);
}

void e2db::edit_bouquet(bouquet& bs)
{
	debug("edit_bouquet()", "bname", bs.bname);

	if (! bouquets.count(bs.bname))
		return error("edit_bouquet()", "Error", "Bouquet \"" + bs.bname + "\" not exists.");

	bouquets[bs.bname] = bs;
}

void e2db::remove_bouquet(string bname)
{
	debug("remove_bouquet()", "bname", bname);

	if (! bouquets.count(bname))
		return error("remove_bouquet()", "Error", "Bouquet \"" + bname + "\" not exists.");

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

//TODO bname destructive edit
void e2db::add_userbouquet(userbouquet& ub)
{
	debug("add_userbouquet()");

	bouquet bs = bouquets[ub.pname];

	if (ub.index < 0)
	{
		int idx = 0;
		string ktype;
		if (bs.btype == 1)
			ktype = "tv";
		else if (bs.btype == 2)
			ktype = "radio";

		for (auto it = index["ubs"].begin(); it != index["ubs"].end(); it++)
		{
			unsigned long pos0 = it->second.find(".dbe");
			unsigned long pos1 = it->second.find('.' + ktype);
			int len = it->second.length();
			int n = 0;
			if (pos0 != string::npos && pos1 != string::npos)
			{
				n = atoi(it->second.substr(pos0 + 4, len - pos1 - 1).data());
				idx = n > idx ? n : idx;
			}
		}

		idx = idx ? idx + 1 : 0;
		ub.index = idx;
	}
	if (ub.bname.empty())
	{
		stringstream bname;
		string ktype;
		if (bs.btype == 1)
			ktype = "tv";
		else if (bs.btype == 2)
			ktype = "radio";

		bname << "userbouquet.dbe" << setfill('0') << setw(2) << ub.index << '.' << ktype;
		ub.bname = bname.str();
	}

	e2db_abstract::add_userbouquet(ub.index, ub);
}

void e2db::edit_userbouquet(userbouquet& ub)
{
	debug("edit_userbouquet()", "bname", ub.bname);

	if (! userbouquets.count(ub.bname))
		return error("edit_userbouquet()", "Error", "Userbouquet \"" + ub.bname + "\" not exists.");

	userbouquets[ub.bname] = ub;
}

void e2db::remove_userbouquet(string bname)
{
	debug("remove_userbouquet()", "bname", bname);

	if (! userbouquets.count(bname))
		return error("remove_userbouquet()", "Error", "Userbouquet \"" + bname + "\" not exists.");

	userbouquet ub = userbouquets[bname];
	bouquet& bs = bouquets[ub.pname];

	for (auto it = index["ubs"].begin(); it != index["ubs"].end(); it++)
	{
		if (it->second == bname)
			index["ubs"].erase(it);
	}

	index.erase(ub.pname);

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
	for (string & w : bs.userbouquets)
	{
		for (auto & x : userbouquets[w].channels)
		{
			if (! x.second.marker)
				index[ub.pname].emplace_back(pair ((index[ub.pname].size() + 1), x.first)); //C++17
		}
	}
	index.erase(bname);
	userbouquets.erase(bname);
}

void e2db::add_channel_reference(channel_reference& chref, string bname)
{
	debug("add_channel_reference()", "chid", chref.chid);

	if (! userbouquets.count(bname))
		return error("add_channel_reference()", "Error", "Userbouquet \"" + bname + "\" not exists.");

	userbouquet& ub = userbouquets[bname];
	service_reference ref;

	if (! chref.marker)
	{
		if (! db.services.count(chref.chid))
			return error("add_channel_reference()", "Error", "Service \"" + chref.chid + "\" not exists.");

		service ch = db.services[chref.chid];

		ref.ssid = ch.ssid;
		ref.dvbns = ch.dvbns;
		ref.tsid = ch.tsid;
	}

	e2db_abstract::add_channel_reference(chref.index, ub, chref, ref);
}

void e2db::edit_channel_reference(string chid, channel_reference& chref, string bname)
{
	debug("edit_channel_reference()", "chid", chid);

	if (! userbouquets.count(bname))
		return error("edit_channel_reference()", "Error", "Userbouquet \"" + bname + "\" not exists.");

	userbouquet& ub = userbouquets[bname];

	debug("edit_channel_reference()", "nw_chid", chref.chid);

	if (chref.chid == chid)
	{
		if (! chref.marker && ! db.services.count(chref.chid))
			return error("edit_channel_reference()", "Error", "Service \"" + chref.chid + "\" not exists.");

		ub.channels[chref.chid] = chref;
	}
	else
	{
		service_reference ref;

		if (! chref.marker)
		{
			if (! db.services.count(chref.chid))
				return error("edit_channel_reference()", "Error", "Service \"" + chref.chid + "\" not exists.");

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
	debug("remove_channel_reference()", "chref.chid", chref.chid);

	if (! userbouquets.count(bname))
		return error("remove_channel_reference()", "Error", "Userbouquet \"" + bname + "\" not exists.");

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
		return error("remove_channel_reference()", "Error", "Channel reference \"" + chid + "\" not exists.");

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
	if (true)
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
	debug("remove_channel_reference()", "chid", chid);

	if (! userbouquets.count(bname))
		return error("remove_channel_reference()", "Error", "Userbouquet \"" + bname + "\" not exists.");
	if (! userbouquets[bname].channels.count(chid))
		return error("remove_channel_reference()", "Error", "Channel reference \"" + chid + "\" not exists.");

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

string e2db::get_reference_id(string chid)
{
	// debug("get_reference_id()", "chid", chid);

	char refid[44];
	int ssid, tsid, dvbns, stype, snum;
	ssid = 0, tsid = 0, dvbns = 0;
	stype = 0, snum = 0;
	string onid = "0";

	if (db.services.count(chid))
	{
		service ch = db.services[chid];
		stype = ch.stype != -1 ? ch.stype : 0;
		snum = ch.snum != -1 ? ch.snum : 0;
		ssid = ch.ssid;
		tsid = ch.tsid;
		onid = ch.onid.empty() ? onid : ch.onid;
		std::transform(onid.begin(), onid.end(), onid.begin(), [](unsigned char c) { return toupper(c); });
		dvbns = ch.dvbns;
	}

	// %1d:%4d:%4X:%4X:%4X:%4s:%8X:0:0:0:
	std::sprintf(refid, "%d:%d:%X:%X:%X:%s:%X:0:0:0", 1, stype, snum, ssid, tsid, onid.c_str(), dvbns);
	return refid;
}

string e2db::get_reference_id(channel_reference chref)
{
	// debug("get_reference_id()", "chref.chid", chref.chid);

	char refid[44];
	int ssid, tsid, dvbns;
	int type, anum;
	ssid = 0, tsid = 0, dvbns = 0;
	type = 0, anum = 0;
	string onid = "0";

	if (! chref.marker && db.services.count(chref.chid))
	{
		service ch = db.services[chref.chid];
		ssid = ch.ssid;
		tsid = ch.tsid;
		onid = ch.onid.empty() ? onid : ch.onid;
		std::transform(onid.begin(), onid.end(), onid.begin(), [](unsigned char c) { return toupper(c); });
		dvbns = ch.dvbns;
	}
	type = chref.type != -1 ? chref.type : 0;
	anum = chref.anum != -1 ? chref.anum : 0;

	// %1d:%4d:%4X:%4X:%4X:%4s:%8X:0:0:0:
	std::sprintf(refid, "%d:%d:%X:%X:%X:%s:%X:0:0:0", 1, type, anum, ssid, tsid, onid.c_str(), dvbns);
	return refid;
}

//TODO unique (eg. terrestrial MUX)
map<string, vector<pair<int, string>>> e2db::get_channels_index()
{
	debug("get_channels_index()");

	map<string, vector<pair<int, string>>> _index;

	for (auto & x: index["chs"])
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
	debug("get_transponders_index()");

	map<string, vector<pair<int, string>>> _index;

	for (auto & x: index["txs"])
	{
		transponder tx = db.transponders[x.second];
		_index[to_string(tx.pos)].emplace_back(x);
	}

	return _index;
}

map<string, vector<pair<int, string>>> e2db::get_services_index()
{
	debug("get_services_index()");

	map<string, vector<pair<int, string>>> _index;
	_index["chs"] = index["chs"];

	return _index;
}

map<string, vector<pair<int, string>>> e2db::get_bouquets_index()
{
	debug("get_bouquets_index()");

	map<string, vector<pair<int, string>>> _index;

	for (auto & x: bouquets)
		_index[x.first] = index[x.first];

	return _index;
}

map<string, vector<pair<int, string>>> e2db::get_userbouquets_index()
{
	debug("get_userbouquets_index()");

	map<string, vector<pair<int, string>>> _index;

	for (auto & x: userbouquets)
		_index[x.first] = index[x.first];

	return _index;
}

map<string, vector<pair<int, string>>> e2db::get_packages_index()
{
	debug("get_packages_index()");

	map<string, vector<pair<int, string>>> _index;

	for (auto & x: index["chs"])
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
	debug("get_resolution_index()");

	map<string, vector<pair<int, string>>> _index;

	for (auto & x: index["chs"])
	{
		service ch = db.services[x.second];
		_index[to_string(ch.stype)].emplace_back(x);
	}

	return _index;
}

map<string, vector<pair<int, string>>> e2db::get_encryption_index()
{
	debug("get_encryption_index()");

	map<string, vector<pair<int, string>>> _index;
	unordered_set<string> _unique;

	for (auto & x: index["chs"])
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
	debug("get_az_index()");

	map<string, vector<pair<int, string>>> _index;

	for (auto & x: index["chs"])
	{
		service ch = db.services[x.second];
		string lt = ch.chname.substr(0, 1);

		if (isdigit(lt[0])) _index["0-9"].emplace_back(x);
		else _index[lt].emplace_back(x);
	}

	return _index;
}

}
