/*!
 * e2-sat-editor/src/e2db/e2db.cpp
 * 
 * @link https://github.com/ctlcltd/e2-sat-editor
 * @copyright e2 SAT Editor Team
 * @author Leonardo Laureti
 * @version 1.6.0
 * @license MIT License
 * @license GNU GPLv3 License
 */

#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <clocale>
#include <algorithm>
#include <chrono>
#include <unordered_set>
#include <sstream>
#include <fstream>
#include <iomanip>
#include <filesystem>
#include <stdexcept>

#include "e2db.h"

using std::unordered_set, std::stringstream, std::ifstream, std::ofstream, std::to_string, std::setfill, std::setw;

namespace e2se_e2db
{

e2db::e2db()
{
	std::setlocale(LC_NUMERIC, "C");

	this->log = new e2se::logger("e2db", "e2db");

	if (! FIX_CRLF)
	{
		PARSER_FIX_CRLF = false;
		MAKER_FIX_CRLF = false;
		CONVERTER_IN_FIX_CRLF = false;
		CONVERTER_OUT_CSV_FIX_CRLF = false;
		CONVERTER_OUT_M3U_FIX_CRLF = false;
		CONVERTER_OUT_HTML_FIX_CRLF = false;
	}

	index["chs"]; // touch index["chs"]
	index["txs"]; // touch index["txs"]
}

void e2db::import_file(vector<string> paths)
{
	debug("import_file", "file path", "multiple");
	debug("import_file", "file input", "auto");

	import_file(FPORTS::unknown, paths);
}

//TODO FIX merge import and input empty map
void e2db::import_file(FPORTS fpi, vector<string> paths)
{
	debug("import_file", "file path", "multiple");
	debug("import_file", "file input", fpi);

	try
	{
		bool merge = this->get_input().size() != 0 ? true : false;
		auto* dst = merge ? newptr() : this;

		try
		{
			for (string & path : paths)
			{
				if (! std::filesystem::exists(path))
				{
					if (merge) delete dst;
					return error("import_file", "File Error", msg("File \"%s\" not exists.", path));
				}
				if (! std::filesystem::is_regular_file(path) && ! std::filesystem::is_directory(path))
				{
					if (merge) delete dst;
					return error("import_file", "File Error", msg("File \"%s\" is not a valid file.", path));
				}
				if
					(
					 (std::filesystem::status(path).permissions() & std::filesystem::perms::owner_read) == std::filesystem::perms::none &&
					 (std::filesystem::status(path).permissions() & std::filesystem::perms::group_read) == std::filesystem::perms::none
					 )
				{
					if (merge) delete dst;
					return error("import_file", "File Error", msg("File \"%s\" is not readable.", path));
				}

				FPORTS _fpi = FPORTS::unknown;

				if (fpi == FPORTS::unknown)
					_fpi = file_type_detect(path);

				string filename = std::filesystem::path(path).filename().u8string();
				string mime = file_mime_value(_fpi, path);

				e2db_file file;
				file.origin = e2db::FORG::filesys;
				file.path = path;
				file.filename = filename;
				file.mime = mime;

				//TODO improve
				if (fpi != FPORTS::directory)
				{
					ifstream ifile (path);
					string line;
					while (std::getline(ifile, line))
						file.data.append(line + '\n');
					ifile.close();
				}
				file.size = file.data.size();

				// debug("import_file", "file path", file.path);
				// debug("import_file", "file size", file.size);

				import_file(_fpi, dst, file, path);
			}

			if (merge)
			{
				this->merge(dst);
				delete dst;
			}

			if (FIX_BOUQUETS) fix_bouquets(true);
		}
		catch (...)
		{
			if (merge) delete dst;

			if (FIX_BOUQUETS) fix_bouquets(true);

			throw;
		}
	}
	catch (const std::invalid_argument& err)
	{
		exception("import_file", "Error", msg(MSG::except_invalid_argument, err.what()));
	}
	catch (const std::out_of_range& err)
	{
		exception("import_file", "Error", msg(MSG::except_out_of_range, err.what()));
	}
	catch (const std::filesystem::filesystem_error& err)
	{
		exception("import_file", "Error", msg(MSG::except_filesystem, err.what()));
	}
	catch (...)
	{
		exception("import_file", "Error", msg(MSG::except_uncaught));
	}
}

//TODO improve stringstream memory
void e2db::import_file(FPORTS fpi, e2db* dst, e2db_file file, string path)
{
	debug("import_file", "file path", "singular");
	debug("import_file", "file input", fpi);

	auto t_start = std::chrono::high_resolution_clock::now();

	try
	{
		string fname = std::filesystem::path(path).filename().u8string();

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
				dst->parse_zapit_services_xml(ifile, fname);
			break;
			case FPORTS::single_tunersets:
			case FPORTS::all_tunersets:
				if (fname == "satellites.xml")
					dst->parse_tunersets_xml(YTYPE::satellite, ifile);
				else if (fname == "terrestrial.xml")
					dst->parse_tunersets_xml(YTYPE::terrestrial, ifile);
				else if (fname == "cables.xml")
					dst->parse_tunersets_xml(YTYPE::cable, ifile);
				else if (fname == "atsc.xml")
					dst->parse_tunersets_xml(YTYPE::atsc, ifile);
			break;
			case FPORTS::single_bouquet:
			case FPORTS::all_bouquets:
				dst->parse_e2db_bouquet(ifile, fname);
			break;
			case FPORTS::single_bouquet_epl:
			case FPORTS::all_bouquets_epl:
				dst->parse_e2db_bouquet(ifile, fname, true);
			break;
			case FPORTS::single_userbouquet:
			case FPORTS::all_userbouquets:
				dst->parse_e2db_userbouquet(ifile, fname);
			break;
			case FPORTS::single_bouquet_all:
			case FPORTS::single_bouquet_all_epl:
				if (file_type_detect(fname) == FPORTS::single_bouquet)
					dst->parse_e2db_bouquet(ifile, fname);
				else if (file_type_detect(fname) == FPORTS::single_bouquet_epl)
					dst->parse_e2db_bouquet(ifile, fname, true);
				else
					dst->parse_e2db_userbouquet(ifile, fname);
			break;
			case FPORTS::all_bouquets_xml:
				if (fname == "bouquets.xml")
					dst->parse_zapit_bouquets_apix_xml(ifile, fname, 1);
				else
					dst->parse_zapit_bouquets_apix_xml(ifile, fname, (ZAPIT_VER != -1 ? ZAPIT_VER : 4));
			break;
			case FPORTS::all_bouquets_xml__4:
				dst->parse_zapit_bouquets_apix_xml(ifile, fname, 4);
			break;
			case FPORTS::all_bouquets_xml__3:
				dst->parse_zapit_bouquets_apix_xml(ifile, fname, 3);
			break;
			case FPORTS::all_bouquets_xml__2:
				dst->parse_zapit_bouquets_apix_xml(ifile, fname, 2);
			break;
			case FPORTS::all_bouquets_xml__1:
				dst->parse_zapit_bouquets_apix_xml(ifile, fname, 1);
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
	catch (const std::invalid_argument& err)
	{
		exception("import_file", "Error", msg(MSG::except_invalid_argument, err.what()));
	}
	catch (const std::out_of_range& err)
	{
		exception("import_file", "Error", msg(MSG::except_out_of_range, err.what()));
	}
	catch (const std::filesystem::filesystem_error& err)
	{
		exception("import_file", "Error", msg(MSG::except_filesystem, err.what()));
	}
	catch (...)
	{
		exception("import_file", "Error", msg(MSG::except_uncaught));
	}

	auto t_end = std::chrono::high_resolution_clock::now();
	int elapsed = std::chrono::duration<double, std::micro>(t_end - t_start).count();

	info("import_file", "elapsed time", to_string(elapsed) + " μs");
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

void e2db::export_file(FPORTS fpo, string path, string filename)
{
	debug("export_file", "file path", "singular");
	debug("export_file", "file output", fpo);

	auto t_start = std::chrono::high_resolution_clock::now();

	try
	{
		e2db_file file;

		string fname = std::filesystem::path(path).filename().u8string();
		string chs_fname = LAMEDB_VER == 5 ? "lamedb5" : (LAMEDB_VER < 4 ? "services" : "lamedb");

		if (filename.empty())
			filename = fname;
		else
			filename = std::filesystem::path(filename).filename().u8string();

		switch (fpo)
		{
			case FPORTS::directory:
				write(path);
			return;
			case FPORTS::all_services:
				make_lamedb(chs_fname, file, (LAMEDB_VER != -1 ? LAMEDB_VER : 4));
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
				make_services_xml(fname, file, (ZAPIT_VER != -1 ? ZAPIT_VER : 4));
			break;
			case FPORTS::all_services_xml__4:
				make_services_xml(fname, file, 4);
			break;
			case FPORTS::all_services_xml__3:
				make_services_xml(fname, file, 3);
			break;
			case FPORTS::all_services_xml__2:
				make_services_xml(fname, file, 2);
			break;
			case FPORTS::all_services_xml__1:
				make_services_xml(fname, file, 1);
			break;
			case FPORTS::single_tunersets:
			case FPORTS::all_tunersets:
				if (fname == "satellites.xml")
					make_tunersets_xml(fname, YTYPE::satellite, file);
				else if (fname == "terrestrial.xml")
					make_tunersets_xml(fname, YTYPE::terrestrial, file);
				else if (fname == "cables.xml")
					make_tunersets_xml(fname, YTYPE::cable, file);
				else if (fname == "atsc.xml")
					make_tunersets_xml(fname, YTYPE::atsc, file);
			break;
			case FPORTS::single_bouquet:
			case FPORTS::all_bouquets:
				make_bouquet(fname, file, (LAMEDB_VER != -1 ? LAMEDB_VER : 4));
			break;
			case FPORTS::single_bouquet_epl:
			case FPORTS::all_bouquets_epl:
				make_bouquet_epl(fname, file, (LAMEDB_VER != -1 ? LAMEDB_VER : 4));
			break;
			case FPORTS::single_userbouquet:
			case FPORTS::all_userbouquets:
				make_userbouquet(fname, file, MARKER_GLOBAL_INDEX, (LAMEDB_VER != -1 ? LAMEDB_VER : 4));
			break;
			case FPORTS::single_bouquet_all:
			case FPORTS::single_bouquet_all_epl:
				if (file_type_detect(fname) == FPORTS::single_bouquet)
					make_bouquet(fname, file, (LAMEDB_VER != -1 ? LAMEDB_VER : 4));
				else if (file_type_detect(fname) == FPORTS::single_bouquet_epl)
					make_bouquet_epl(fname, file, (LAMEDB_VER != -1 ? LAMEDB_VER : 4));
				else
					make_userbouquet(fname, file, MARKER_GLOBAL_INDEX, (LAMEDB_VER != -1 ? LAMEDB_VER : 4));
			break;
			case FPORTS::all_bouquets_xml:
				make_bouquets_xml(fname, file, (ZAPIT_VER != -1 ? ZAPIT_VER : 4));
			break;
			case FPORTS::all_bouquets_xml__4:
				make_bouquets_xml(fname, file, 4);
			break;
			case FPORTS::all_bouquets_xml__3:
				make_bouquets_xml(fname, file, 3);
			break;
			case FPORTS::all_bouquets_xml__2:
				make_bouquets_xml(fname, file, 2);
			break;
			case FPORTS::all_bouquets_xml__1:
				make_bouquets_xml(fname, file, 1);
			break;
			case FPORTS::single_parentallock_blacklist:
				make_parentallock_list(fname, PARENTALLOCK::blacklist, file);
			break;
			case FPORTS::single_parentallock_whitelist:
				make_parentallock_list(fname, PARENTALLOCK::whitelist, file);
			break;
			case FPORTS::single_parentallock_locked:
				make_parentallock_list(fname, PARENTALLOCK::locked, file);
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

			fpath = basedir + filename;
		}

		if (! OVERWRITE_FILE && std::filesystem::exists(fpath))
		{
			return error("export_file", "File Error", msg("File \"%s\" already exists.", fpath));
		}
		if
		(
			(std::filesystem::status(fpath).permissions() & std::filesystem::perms::owner_write) == std::filesystem::perms::none &&
			(std::filesystem::status(fpath).permissions() & std::filesystem::perms::group_write) == std::filesystem::perms::none
		)
		{
			return error("export_file", "File Error", msg("File \"%s\" is not writable.", path));
		}

		// debug("export_file", "file path", fpath);
		// debug("export_file", "file size", file.size);

		std::ios_base::openmode fmode = std::ios_base::out;

		if (MAKER_FIX_CRLF && check_crlf()) fmode |= std::ios_base::binary;

		ofstream out (fpath, fmode);
		out << file.data;
		out.close();
	}
	catch (const std::invalid_argument& err)
	{
		exception("export_file", "Error", msg(MSG::except_invalid_argument, err.what()));
	}
	catch (const std::out_of_range& err)
	{
		exception("export_file", "Error", msg(MSG::except_out_of_range, err.what()));
	}
	catch (const std::filesystem::filesystem_error& err)
	{
		exception("export_file", "Error", msg(MSG::except_filesystem, err.what()));
	}
	catch (...)
	{
		exception("export_file", "Error", msg(MSG::except_uncaught));
	}

	auto t_end = std::chrono::high_resolution_clock::now();
	int elapsed = std::chrono::duration<double, std::micro>(t_end - t_start).count();

	info("export_file", "elapsed time", to_string(elapsed) + " μs");
}

//TODO FIX merge import and input empty map
void e2db::import_blob(unordered_map<string, e2db_file> files)
{
	debug("import_blob", "size", int (files.size()));

	auto t_start = std::chrono::high_resolution_clock::now();

	blobctx_crlf = true;

	try
	{
		bool merge = this->get_input().size() != 0 ? true : false;
		bool uniq_ubouquets = index["ubs"].size() != 0;

		if (merge)
		{
			auto* dst = newptr();

			try
			{
				dst->parse_e2db(files);

				this->merge(dst);
				delete dst;

				if (FIX_BOUQUETS) fix_bouquets(uniq_ubouquets);
			}
			catch (...)
			{
				delete dst;

				if (FIX_BOUQUETS) fix_bouquets(uniq_ubouquets);

				throw;
			}

			if (! db.dstat)
				db.dstat = DSTAT::d_read;
		}
		else
		{
			try
			{
				parse_e2db(files);

				db.dstat = DSTAT::d_read;

				if (FIX_BOUQUETS) fix_bouquets(uniq_ubouquets);
			}
			catch (...)
			{
				if (FIX_BOUQUETS) fix_bouquets(uniq_ubouquets);

				throw;
			}
		}
	}
	catch (const std::invalid_argument& err)
	{
		exception("import_blob", "Error", msg(MSG::except_invalid_argument, err.what()));
	}
	catch (const std::out_of_range& err)
	{
		exception("import_blob", "Error", msg(MSG::except_out_of_range, err.what()));
	}
	catch (const std::filesystem::filesystem_error& err)
	{
		exception("import_blob", "Error", msg(MSG::except_filesystem, err.what()));
	}
	catch (...)
	{
		exception("import_blob", "Error", msg(MSG::except_uncaught));
	}

	blobctx_crlf = false;

	auto t_end = std::chrono::high_resolution_clock::now();
	int elapsed = std::chrono::duration<double, std::micro>(t_end - t_start).count();

	info("import_blob", "elapsed time", to_string(elapsed) + " μs");
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

		changes.clear();

		for (auto & x : db.services)
		{
			service& ch = x.second;

			if (ch.txid == txid)
			{
				string chid = x.first;
				char nw_chid[25];
				// %4x:%4x:%8x
				std::snprintf(nw_chid, 25, "%x:%x:%x", ch.ssid, tx.tsid, tx.dvbns);

				changes.emplace(ch.chid, nw_chid);

				ch.chid = nw_chid;
				ch.txid = tx.txid;
				ch.tsid = tx.tsid;
				ch.onid = tx.onid;
				ch.dvbns = tx.dvbns;
			}
		}

		for (auto & x : userbouquets)
		{
			for (auto & q : x.second.channels)
			{
				channel_reference& chref = q.second;

				if (changes.count(chref.chid) && ! chref.marker)
				{
					string chid = chref.chid;
					string nw_chid = changes[chref.chid];

					chref.chid = nw_chid;

					{
						chref.ref.tsid = tx.tsid;
						chref.ref.onid = tx.onid;
						chref.ref.dvbns = tx.dvbns;
					}
				}
			}
		}

		vector<string> i_names;

		for (auto & x : index)
		{
			string iname = x.first;

			if (iname == "txs")
				continue;
			else if (iname == "mks")
				continue;
			else if (iname.find("tns:") != string::npos)
				continue;
			else
				i_names.emplace_back(iname);
		}

		for (string & iname : i_names)
		{
			for (auto & x : index[iname])
			{
				if (changes.count(x.second))
				{
					string chid = x.second;
					string nw_chid = changes[x.second];

					x.second = nw_chid;
				}
			}
		}

		for (auto & i : changes)
		{
			service ch = db.services[i.first];

			db.services.erase(i.first);
			db.services.emplace(i.second, ch);
		}

		for (auto & x : userbouquets)
		{
			for (auto & i : changes)
			{
				if (x.second.channels.count(i.first))
				{
					channel_reference chref = x.second.channels[i.first];

					x.second.channels.erase(i.first);
					x.second.channels.emplace(i.second, chref);
				}
			}
		}
	}
}

void e2db::remove_transponder(string txid)
{
	debug("remove_transponder", "txid", txid);

	db.transponders.erase(txid);

	vector<pair<int, string>>::iterator pos;
	for (auto it = index["txs"].begin(); it != index["txs"].end(); it++)
	{
		if (it->second == txid)
		{
			pos = it;
			break;
		}
	}
	if (pos != index["txs"].end())
	{
		index["txs"].erase(pos);
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
		return error("edit_service", "Error", msg("Service \"%s\" not exists.", chid));

	char nw_txid[25];
	// %4x:%8x
	std::snprintf(nw_txid, 25, "%x:%x", ch.tsid, ch.dvbns);

	char nw_chid[25];
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
		return error("remove_service", "Error", msg("Service \"%s\" not exists.", chid));

	service ch = db.services[chid];
	string kchid = 's' + chid;

	db.services.erase(chid);

	vector<string> i_names;

	for (auto & x : index)
	{
		bool found = false;
		vector<vector<pair<int, string>>::iterator> itx;

		for (auto it = x.second.begin(); it != x.second.end(); it++)
		{
			if (it->second == chid)
			{
				found = true;
				itx.push_back(it);
				i_names.push_back(x.first);
			}
		}
		if (found)
		{
			for (auto pos = itx.begin(); pos != itx.end(); pos++)
			{
				x.second.erase(*pos);
			}
		}
	}
	for (string & iname : i_names)
	{
		int idx = 0;
		for (auto & x : index[iname])
		{
			channel_reference& chref = userbouquets[iname].channels[x.second];

			if (! chref.marker)
			{
				idx += 1;
				chref.index = idx;
				x.first = idx;
			}
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

	if (! bs.rname.empty())
	{
		bool found = false;

		for (auto it = index["bss"].begin(); it != index["bss"].end(); it++)
		{
			if (it->second == bs.rname)
			{
				found = true;
				break;
			}
		}

		if (found)
			error("add_bouquet", "Error", msg("Bouquet \"%s\" already exists.", bs.rname));
		else
			bs.bname = bs.rname;
	}

	if (bouquets.count(bs.bname))
		return error("add_bouquet", "Error", msg("Bouquet \"%s\" already exists.", bs.bname));

	e2db_abstract::add_bouquet(bs.index, bs);
}

void e2db::edit_bouquet(bouquet& bs)
{
	debug("edit_bouquet", "bname", bs.bname);

	if (! bouquets.count(bs.bname))
		return error("edit_bouquet", "Error", msg("Bouquet \"%s\" not exists.", bs.bname));

	if (! bs.rname.empty())
	{
		bool found = false;

		for (auto it = index["bss"].begin(); it != index["bss"].end(); it++)
		{
			if (it->second == bs.rname)
			{
				found = true;
				break;
			}
		}

		if (found)
			error("edit_bouquet", "Error", msg("Bouquet \"%s\" already exists.", bs.rname));
	}

	bouquets[bs.bname] = bs;
}

void e2db::remove_bouquet(string bname)
{
	debug("remove_bouquet", "bname", bname);

	if (! bouquets.count(bname))
		return error("remove_bouquet", "Error", msg("Bouquet \"%s\" not exists.", bname));

	vector<pair<int, string>>::iterator pos;
	for (auto it = index["bss"].begin(); it != index["bss"].end(); it++)
	{
		if (it->second == bname)
		{
			pos = it;
			break;
		}
	}
	if (pos != index["bss"].end())
	{
		index["bss"].erase(pos);
	}

	vector<string> i_names;
	vector<vector<pair<int, string>>::iterator> itx;
	for (auto it = index["ubs"].begin(); it != index["ubs"].end(); it++)
	{
		userbouquet& ub = userbouquets[it->second];

		if (ub.pname == bname)
		{
			itx.push_back(it);
			i_names.push_back(ub.bname);
		}
	}
	for (auto pos = itx.begin(); pos != itx.end(); pos++)
	{
		index["ubs"].erase(*pos);
	}
	for (string & bname : i_names)
	{
		index.erase(bname);
		userbouquets.erase(bname);
	}

	index.erase(bname);
	bouquets.erase(bname);
}

void e2db::add_userbouquet(userbouquet& ub)
{
	debug("add_userbouquet");

	bouquet bs = bouquets[ub.pname];

	string ub_fname_suffix = USERBOUQUET_FILENAME_SUFFIX;

	string ktype;
	if (bs.btype == STYPE::tv)
		ktype = "tv";
	else if (bs.btype == STYPE::radio)
		ktype = "radio";

	if (ub.index == -1)
	{
		int idx = 0;

		if (index.count("ubs"))
		{
			int nmax = -1;

			for (auto it = bs.userbouquets.begin(); it != bs.userbouquets.end(); it++)
			{
				string bname = *it;
				size_t len = bname.rfind('.' + ktype);

				if (len != string::npos)
				{
					size_t pos = bname.rfind('.', len - 1);
					string str = bname.substr(pos + 1, len - pos - 1);
					string prefix, num;

					size_t i = 0;
					while (i != str.size())
					{
						if (std::isdigit(str[i]))
							num += str[i];
						else
							prefix += str[i];
						i++;
					}

					if (! num.empty())
					{
						int n = std::atoi(num.data());
						nmax = n > nmax ? n : nmax;
					}
				}
			}

			if (nmax == -1)
				nmax = int (bs.userbouquets.size());

			idx = nmax + 1;
		}

		ub.index = idx;
	}

	if (! ub.rname.empty())
	{
		bool found = false;

		for (auto it = index["ubs"].begin(); it != index["ubs"].end(); it++)
		{
			if (it->second == ub.rname)
			{
				found = true;
				break;
			}
		}

		if (found)
			error("add_userbouquet", "Error", msg("Userbouquet \"%s\" already exists.", ub.rname));
		else
			ub.bname = ub.rname;
	}

	if (ub.bname.empty())
	{
		stringstream ub_bname;
		ub_bname << "userbouquet." << ub_fname_suffix << setfill('0') << setw(2) << ub.index << '.' << ktype;

		ub.bname = ub_bname.str();
	}

	if (userbouquets.count(ub.bname))
		return error("edit_userbouquet", "Error", msg("Userbouquet \"%s\" already exists.", ub.bname));

	e2db_abstract::add_userbouquet(ub.index, ub);
}

void e2db::edit_userbouquet(userbouquet& ub)
{
	debug("edit_userbouquet", "bname", ub.bname);

	if (! userbouquets.count(ub.bname))
		return error("edit_userbouquet", "Error", msg("Userbouquet \"%s\" not exists.", ub.bname));

	bouquet bs = bouquets[ub.pname];

	if (! ub.rname.empty())
	{
		bool found = false;

		for (auto it = index["ubs"].begin(); it != index["ubs"].end(); it++)
		{
			if (it->second == ub.rname)
			{
				found = true;
				break;
			}
		}

		if (found)
			error("edit_userbouquet", "Error", msg("Userbouquet \"%s\" already exists.", ub.rname));
	}

	userbouquets[ub.bname] = ub;
}

void e2db::remove_userbouquet(string bname)
{
	debug("remove_userbouquet", "bname", bname);

	if (! userbouquets.count(bname))
		return error("remove_userbouquet", "Error", msg("Userbouquet \"%s\" not exists.", bname));

	userbouquet ub = userbouquets[bname];
	bouquet& bs = bouquets[ub.pname];

	{
		vector<pair<int, string>>::iterator pos;
		for (auto it = index["ubs"].begin(); it != index["ubs"].end(); it++)
		{
			if (it->second == bname)
			{
				pos = it;
				break;
			}
		}
		if (pos != index["ubs"].end())
		{
			index["ubs"].erase(pos);
		}
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

			if (! chref.marker && ! chref.stream && bs.services.count(chref.chid) == 0)
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
		return error("add_channel_reference", "Error", msg("Userbouquet \"%s\" not exists.", bname));

	userbouquet& ub = userbouquets[bname];
	service_reference& ref = chref.ref;

	if (chref.marker)
	{
		if (! chref.anum)
		{
			chref.anum = db.imarkers + 1;
		}
		if (chref.index == -1)
		{
			int ub_idx = ub.index;
			chref.index = ub_idx;
		}
	}
	else if (chref.stream)
	{
		if (chref.index == -1)
		{
			int ub_idx = ub.index;
			chref.index = ub_idx;
		}
	}
	else if (db.services.count(chref.chid))
	{
		service ch = db.services[chref.chid];

		ref.ssid = ch.ssid;
		ref.tsid = ch.tsid;
		ref.onid = ch.onid;
		ref.dvbns = ch.dvbns;
	}

	if (chref.index == -1 && ! chref.marker && ! chref.stream)
		chref.index = int (index[bname].size()) + 1;

	e2db_abstract::add_channel_reference(chref.index, ub, chref, ref);
}

void e2db::edit_channel_reference(string chid, channel_reference& chref, string bname)
{
	debug("edit_channel_reference", "chid", chid);

	if (! userbouquets.count(bname))
		return error("edit_channel_reference", "Error", msg("Userbouquet \"%s\" not exists.", bname));

	userbouquet& ub = userbouquets[bname];

	char nw_chid[25];

	if (chref.marker)
	{
		if (chref.inum == -1)
		{
			chref.inum = db.istreams + 1;
		}
		if (chref.index == -1)
		{
			int ub_idx = ub.index;
			chref.index = ub_idx;
		}

		// %4d:%4d:%8x:%d
		std::snprintf(nw_chid, 25, "%d:%d:%x:%d", chref.etype, chref.atype, chref.inum, ub.index);

		chref.chid = nw_chid;
	}
	else if (chref.stream)
	{
		if (chref.inum == -1)
		{
			chref.inum = db.istreams + 1;
		}
		if (chref.index == -1)
		{
			int ub_idx = ub.index;
			chref.index = ub_idx;
		}

		// %4d:%4d:%8x:%d
		std::snprintf(nw_chid, 25, "%d:%d:%x:%d", chref.etype, chref.atype, chref.inum, ub.index);

		chref.chid = nw_chid;
	}
	else if (chref.chid != chid)
	{
		// %4x:%4x:%8x
		std::snprintf(nw_chid, 25, "%x:%x:%x", chref.ref.ssid, chref.ref.tsid, chref.ref.dvbns);

		chref.chid = nw_chid;
	}

	debug("edit_channel_reference", "new chid", chref.chid);

	if (chref.chid == chid)
	{
		ub.channels[chref.chid] = chref;
	}
	else
	{
		service_reference& ref = chref.ref;

		if (! chref.marker && ! chref.stream && db.services.count(chref.chid))
		{
			service ch = db.services[chref.chid];

			ref.ssid = ch.ssid;
			ref.tsid = ch.tsid;
			ref.onid = ch.onid;
			ref.dvbns = ch.dvbns;
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
		else if (! chref.stream)
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
		return error("remove_channel_reference", "Error", msg("Userbouquet \"%s\" not exists.", bname));

	userbouquet& ub = userbouquets[bname];
	int idx = chref.index;
	string chid = chref.chid;

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
		return error("remove_channel_reference", "Error", msg("Channel reference \"%s\" not exists.", chid));

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
		int count = 0;
		for (auto & x : userbouquets)
		{
			if (x.second.channels.count(chid))
				count++;
		}

		if (count && count != 1)
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
			//TODO TEST EXC_BAD_ACCESS
			try
			{
				if (pos != index[ub.pname].end())
				{
					index[ub.pname].erase(pos);
				}
			}
			catch (...)
			{
				debug("remove_channel_reference", "fault", "parent index remove position");
			}
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
		return error("remove_channel_reference", "Error", msg("Userbouquet \"%s\" not exists.", bname));
	if (! userbouquets[bname].channels.count(chid))
		return error("remove_channel_reference", "Error", msg("Channel reference \"%s\" not exists.", chid));

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
		int count = 0;
		for (auto & x : userbouquets)
		{
			if (x.second.channels.count(chid))
				count++;
		}

		if (count && count != 1)
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
			//TODO TEST EXC_BAD_ACCESS
			try
			{
				if (pos != index[ub.pname].end())
				{
					index[ub.pname].erase(pos);
				}
			}
			catch (...)
			{
				debug("remove_channel_reference", "fault", "parent index remove position");
			}
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
		return error("remove_tunersets_table", "Error", msg("Tunersets table \"%s\" not exists.", tnid));

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
		return error("set_service_parentallock", "Error", msg("Service \"%s\" not exists.", chid));

	service& ch = db.services[chid];
	ch.parental = true;
}

void e2db::unset_service_parentallock(string chid)
{
	debug("unset_service_parentallock", "chid", chid);

	if (! db.services.count(chid))
		return error("unset_service_parentallock", "Error", msg("Service \"%s\" not exists.", chid));

	service& ch = db.services[chid];
	ch.parental = false;
}

void e2db::set_userbouquet_parentallock(string bname)
{
	debug("set_userbouquet_parentallock", "bname", bname);

	if (! userbouquets.count(bname))
		return error("set_userbouquet_parentallock", "Error", msg("Userbouquet \"%s\" not exists.", bname));

	userbouquet& ub = userbouquets[bname];
	ub.parental = true;
}

void e2db::unset_userbouquet_parentallock(string bname)
{
	debug("unset_userbouquet_parentallock", "bname", bname);

	if (! userbouquets.count(bname))
		return error("unset_userbouquet_parentallock", "Error", msg("Userbouquet \"%s\" not exists.", bname));

	userbouquet& ub = userbouquets[bname];
	ub.parental = false;
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

//TODO improve options
void e2db::merge(e2db_abstract* dst)
{
	debug("merge");

	auto t_start = std::chrono::high_resolution_clock::now();

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

	this->datas.clear();
	this->comments.clear();
	this->changes.clear();

	dst->datas.clear();
	dst->comments.clear();

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

	if (MERGE_SORT_ID) // order by tsid|ssid	elapsed time: 84290
	{
		unordered_map<string, int> txs;
		vector<pair<int, string>> chis;

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
					chis.emplace_back(x);
				}

				txs[tx.txid]++;
			}
		}

		index["chs"].swap(chis);
	}
	else  // unordered append		elapsed time: 11086
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
		string ub_fname_suffix = USERBOUQUET_FILENAME_SUFFIX;

		if (ubs_names.count(qw)) // merge
		{
			merge = true;
			bname = ubs_names[qw];

			/*unordered_map<string, channel_reference> channels;
			vector<pair<int, string>> i_ub;
			int idx = int (dst->index[iname].size() - 1);

			for (auto & x : dst->index[iname])
			{
				channel_reference& chref = ub.channels[x.second];
				idx += 1;

				if (chref.marker || chref.stream)
				{
					char chid[25];
					// %4d:%4d:%2x:%d
					std::snprintf(chid, 25, "%d:%d:%x:%d", chref.etype, chref.atype, idx, ub.index);

					chref.chid = chid;
					chref.inum = idx;

					i_ub.emplace_back(pair (idx, chref.chid));
				}
				else
				{
					i_ub.emplace_back(pair (idx, x.second));
				}

				i_ub.emplace_back(pair (idx, x.second));

				channels.emplace(chref.chid, chref);
			}

			ub.channels.swap(channels);
			dst->index[iname].swap(i_ub);*/

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
			ub_bname << "userbouquet." << ub_fname_suffix << setfill('0') << setw(2) << ub_idx << '.' << ktype;

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

			if (! chref.marker && ! chref.stream)
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

	this->db.imarkers = 0;
	this->db.istreams = 0;

	//TODO improve reverse sort tv, radio

	for (auto & x : index["ubs"])
	{
		userbouquet& ub = this->userbouquets[x.second];
		string bname = ub.bname;
		unordered_map<string, channel_reference> channels;
		vector<pair<int, string>> i_ub;

		int i = 0;
		int idx = 0;

		for (auto & x : index[bname])
		{
			channel_reference& chref = ub.channels[x.second];

			if (chref.marker || chref.stream)
			{
				char chid[25];

				if (chref.marker)
				{
					chref.inum = this->db.imarkers + 1;
					chref.anum = chref.inum;

					// %4d:%4d:%2x:%d
					std::snprintf(chid, 25, "%d:%d:%x:%d", chref.etype, chref.atype, chref.inum, ub.index);

					if (chref.atype == e2db::ATYPE::marker_numbered)
						idx = i += 1;
					else
						idx = 0;

					index["mks"].emplace_back(pair (ub.index, chid));

					this->db.imarkers++;
				}
				else if (chref.stream)
				{
					chref.inum = this->db.istreams + 1;

					// %4d:%4d:%2x:%d
					std::snprintf(chid, 25, "%d:%d:%x:%d", chref.etype, chref.atype, chref.inum, ub.index);

					idx = i += 1;

					this->db.istreams++;
				}

				chref.chid = chid;
				i_ub.emplace_back(pair (idx, chid));
			}
			else
			{
				idx = i += 1;

				i_ub.emplace_back(pair (idx, x.second));
			}

			channels.emplace(chref.chid, chref);
		}

		ub.channels.swap(channels);
		index[bname].swap(i_ub);
	}

	this->index.swap(index);

	auto t_end = std::chrono::high_resolution_clock::now();
	int elapsed = std::chrono::duration<double, std::micro>(t_end - t_start).count();

	info("merge", "elapsed time", to_string(elapsed) + " μs");
}

}
