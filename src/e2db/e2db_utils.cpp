/*!
 * e2-sat-editor/src/e2db/e2db_utils.cpp
 * 
 * @link https://github.com/ctlcltd/e2-sat-editor
 * @copyright e2 SAT Editor Team
 * @author Leonardo Laureti
 * @version 1.6.0
 * @license MIT License
 * @license GNU GPLv3 License
 */

#include <clocale>

#include "e2db_utils.h"

namespace e2se_e2db
{

e2db_utils::e2db_utils()
{
	std::setlocale(LC_NUMERIC, "C");

	this->log = new e2se::logger("e2db", "e2db_utils");
}

void e2db_utils::remove_orphaned_services()
{
	debug("remove_orphaned_services");

	unordered_set<string> i_services;

	for (auto & x : db.services)
	{
		service& ch = x.second;

		if ((! ch.txid.empty() && ! db.transponders.count(ch.txid)) || ch.tsid < 1 || ch.onid < 1)
			i_services.insert(ch.chid);
	}

	if (i_services.empty())
		return;

	for (const string & chid : i_services)
		db.services.erase(chid);

	unordered_set<string> i_names;

	for (auto & x : index)
	{
		string iname = x.first;

		for (auto & x : index[iname])
		{
			if (i_services.count(x.second))
			{
				i_names.insert(iname);
				break;
			}
		}
	}

	for (auto & x : bouquets)
	{
		bouquet& bs = x.second;

		for (const string & chid : i_services)
			bs.services.erase(chid);
	}

	for (const string & iname : i_names)
	{
		if (! userbouquets.count(iname))
			continue;

		for (const string & chid : i_services)
			userbouquets[iname].channels.erase(chid);

		rebuild_index_userbouquet(iname, i_services);
	}

	rebuild_index_services();
}

void e2db_utils::remove_orphaned_references()
{
	debug("remove_orphaned_references");

	unordered_set<string> i_services;

	for (auto & x : userbouquets)
	{
		unordered_set<string> i_channels;
		userbouquet& ub = x.second;
		string bname = ub.bname;

		for (auto & x : ub.channels)
		{
			channel_reference& chref = x.second;

			if (! chref.marker && ! chref.stream && ! db.services.count(chref.chid))
				i_channels.insert(chref.chid);
		}

		for (const string & chid : i_channels)
			ub.channels.erase(chid);

		rebuild_index_userbouquet(bname, i_channels);

		i_services.insert(i_channels.begin(), i_channels.end());
	}

	if (! i_services.empty())
	{
		for (auto & x : bouquets)
		{
			bouquet& bs = x.second;

			for (const string & chid : i_services)
				bs.services.erase(chid);
		}
	}
}

void e2db_utils::fix_remove_references()
{
	debug("fix_remove_references");

	unordered_set<string> i_services;

	for (auto & x : userbouquets)
	{
		unordered_set<string> i_channels;
		userbouquet& ub = x.second;
		string bname = ub.bname;

		if (! index.count(bname))
		{
			error("fix_remove_references", "Error", msg("Missing index key \"%s\".", bname));

			continue;
		}

		for (auto & x : index[bname])
		{
			string chid = x.second;
			bool ref_error = false;

			if (ub.channels.count(chid))
			{
				channel_reference& chref = ub.channels[chid];

				if (db.services.count(chid))
				{
					service& ch = db.services[chid];

					if (ch.chid != chref.chid)
						ref_error = true;
					if (ch.txid.empty() || ch.tsid < 1 || ch.onid < 1)
						ref_error = true;
				}
				else
				{
					if (chref.marker)
					{
						if (chref.atype != ATYPE::marker && chref.atype != ATYPE::marker_hidden_512 && chref.atype != ATYPE::marker_hidden_832 && chref.atype != ATYPE::marker_numbered)
							ref_error = true;
					}
					else if (chref.stream)
					{
						if (chref.uri.empty())
							ref_error = true;
					}
					else
					{
						ref_error = true;
					}
				}
			}
			else
			{
				ref_error = true;
			}

			if (ref_error)
				i_channels.insert(chid);
		}

		for (const string & chid : i_channels)
			ub.channels.erase(chid);

		rebuild_index_userbouquet(bname, i_channels);

		i_services.insert(i_channels.begin(), i_channels.end());
	}

	if (! i_services.empty())
	{
		for (auto & x : bouquets)
		{
			bouquet& bs = x.second;

			for (const string & chid : i_services)
				bs.services.erase(chid);
		}
	}
}

void e2db_utils::fix_dvbns()
{
	debug("fix_dvbns");

	unordered_map<string, string> txs_changes;

	for (auto & x : db.transponders)
	{
		string txid = x.first;
		transponder& tx = x.second;

		int nw_dvbns = value_transponder_dvbns(tx);

		if (tx.dvbns != nw_dvbns)
		{
			tx.dvbns = nw_dvbns;

			char nw_txid[25];
			// %4x:8x
			std::snprintf(nw_txid, 25, "%x:%x", tx.tsid, tx.dvbns);
			tx.txid = nw_txid;

			txs_changes.emplace(txid, nw_txid);
		}
	}

	if (txs_changes.empty())
		return;

	for (auto & x : txs_changes)
	{
		transponder tx = db.transponders[x.first];
		db.transponders.erase(x.first);
		db.transponders.emplace(x.second, tx);
	}

	for (auto & x : index["txs"])
	{
		string txid = x.second;

		if (txs_changes.count(txid))
			x.second = txs_changes[txid];
	}

	unordered_map<string, string> chs_changes;

	for (auto & x : index["chs"])
	{
		string chid = x.second;

		if (db.services.count(chid))
		{
			service& ch = db.services[chid];

			if (txs_changes.count(ch.txid))
			{
				string chid = ch.chid;
				string nw_txid = txs_changes[ch.txid];
				transponder& tx = db.transponders[nw_txid];

				char nw_chid[25];
				// %4x:%4x:%8x
				std::snprintf(nw_chid, 25, "%x:%x:%x", ch.ssid, tx.tsid, tx.dvbns);
				ch.chid = nw_chid;
				ch.txid = nw_txid;
				ch.dvbns = tx.dvbns;

				chs_changes.emplace(chid, nw_chid);

				x.second = nw_chid;
			}
		}
	}

	for (auto & x : chs_changes)
	{
		service ch = db.services[x.first];
		db.services.erase(x.first);
		db.services.emplace(x.second, ch);
	}

	for (auto & x : userbouquets)
	{
		userbouquet& ub = x.second;
		string bname = ub.bname;

		bool found = false;

		for (auto & x : ub.channels)
		{
			if (chs_changes.count(x.first))
			{
				found = true;
				break;
			}
		}

		if (found)
		{
			if (! index.count(bname))
			{
				error("fix_dvbns", "Error", msg("Missing index key \"%s\".", bname));

				continue;
			}

			unordered_map<string, channel_reference> channels;

			for (auto & x : index[bname])
			{
				string chid = x.second;

				if (chs_changes.count(chid))
				{
					channel_reference chref = ub.channels[x.second];
					string nw_chid = chs_changes[chid];

					if (! chref.stream)
					{
						service& ch = db.services[nw_chid];
						chref.ref.dvbns = ch.dvbns;
					}

					chref.chid = nw_chid;
					x.second = nw_chid;

					channels.emplace(chref.chid, chref);
				}
			}

			ub.channels.swap(channels);
		}
	}
}

void e2db_utils::clear_services_cached()
{
	debug("clear_services_cached");

	for (auto & x : db.services)
	{
		service& ch = x.second;
		ch.data.erase(SDATA::c);
	}
}

void e2db_utils::clear_services_caid()
{
	debug("clear_services_caid");

	for (auto & x : db.services)
	{
		service& ch = x.second;
		ch.data.erase(SDATA::C);
	}
}

void e2db_utils::clear_services_flags()
{
	debug("clear_services_flags");

	for (auto & x : db.services)
	{
		service& ch = x.second;
		ch.data.erase(SDATA::f);
	}
}

void e2db_utils::clear_services_data()
{
	debug("clear_services_data");

	for (auto & x : db.services)
	{
		service& ch = x.second;
		ch.data.clear();
		ch.data = {{'p', {""}}};
	}
}

void e2db_utils::clear_favourites()
{
	debug("clear_favourites");

	for (auto & x : userbouquets)
	{
		unordered_set<string> i_channels;
		userbouquet& ub = x.second;
		string bname = ub.bname;

		for (auto & x : ub.channels)
		{
			string chid = x.first;
			channel_reference& chref = x.second;

			if (chref.stream || (! db.services.count(chref.chid) && ! chref.marker))
				i_channels.insert(chref.chid);
		}

		for (const string & chid : i_channels)
			ub.channels.erase(chid);

		rebuild_index_userbouquet(bname, i_channels);
	}
}

void e2db_utils::clear_bouquets_unused_services()
{
	debug("clear_bouquets_unused_services");

	unordered_set<string> i_channels;
	unordered_set<string> i_services;

	for (auto & x : userbouquets)
	{
		userbouquet& ub = x.second;
		string bname = ub.bname;

		for (auto & x : ub.channels)
		{
			channel_reference& chref = x.second;

			if (db.services.count(chref.chid))
				i_channels.insert(chref.chid);
		}
	}

	if (i_channels.empty())
		return;

	for (auto & x : db.services)
	{
		string chid = x.first;

		if (! i_channels.count(chid))
			i_services.insert(chid);
	}

	for (const string & chid : i_services)
	{
		string kchid = 's' + chid;
		db.services.erase(chid);

		if (collisions.count(kchid))
			collisions.erase(kchid);
	}

	rebuild_index_services(i_channels);
}

void e2db_utils::remove_parentallock()
{
	debug("remove_parentallock");

	remove_parentallock_services();
	remove_parentallock_userbouquets();
}

void e2db_utils::remove_parentallock_services()
{
	debug("remove_parentallock");

	for (auto & x : db.services)
	{
		string chid = x.first;
		service& ch = x.second;

		ch.parental = false;
	}
}

void e2db_utils::remove_parentallock_userbouquets()
{
	debug("remove_parentallock_userbouquets");

	for (auto & x : userbouquets)
	{
		string bname = x.first;
		userbouquet& ub = x.second;

		ub.parental = false;
	}
}

void e2db_utils::remove_bouquets()
{
	debug("remove_bouquets");

	unordered_set<string> i_bouquets;

	for (auto & x : bouquets)
	{
		string bname = x.second.bname;
		i_bouquets.insert(bname);
	}

	for (const string & bname : i_bouquets)
	{
		bouquets.erase(bname);

		if (index.count(bname))
			index.erase(bname);
	}

	index.erase("bss");
}

void e2db_utils::remove_userbouquets()
{
	debug("remove_userbouquets");

	unordered_set<string> i_ubouquets;

	for (auto & x : userbouquets)
	{
		string bname = x.second.bname;
		i_ubouquets.insert(bname);
	}

	for (const string & bname : i_ubouquets)
	{
		userbouquets.erase(bname);

		if (index.count(bname))
			index.erase(bname);
	}

	index.erase("ubs");

	for (auto & x : bouquets)
	{
		bouquet& bs = x.second;
		bs.userbouquets.clear();
	}
}

void e2db_utils::remove_duplicates()
{
	debug("remove_duplicates");

	remove_duplicates_transponders();
	remove_duplicates_services();
	remove_duplicates_references();
	remove_duplicates_markers();
	//TODO duplicates tunersets
}

void e2db_utils::remove_duplicates_transponders()
{
	debug("remove_duplicates_transponders");

	rebuild_index_transponders();
}

void e2db_utils::remove_duplicates_services()
{
	debug("remove_duplicates_services");

	unordered_set<string> i_services;
	unordered_set<string> _unique;

	for (auto & x : collisions)
	{
		string kchid = x.first;
		i_services.insert(kchid);
	}

	for (const string & kchid : i_services)
		db.services.erase(kchid);

	collisions.clear();

	if (! i_services.empty())
	{
		unordered_set<string> i_names;

		for (auto & x : index)
		{
			string iname = x.first;

			for (auto & x : index[iname])
			{
				if (i_services.count(x.second))
				{
					i_names.insert(iname);
					break;
				}
			}
		}

		for (const string & iname : i_names)
		{
			if (! userbouquets.count(iname))
				continue;

			userbouquet& ub = userbouquets[iname];

			for (const string & kchid : i_services)
				ub.channels.erase(kchid);
		}

		for (auto & x : bouquets)
		{
			bouquet& bs = x.second;

			for (const string & kchid : i_services)
				bs.services.erase(kchid);
		}
	}

	for (auto & x : index)
	{
		string iname = x.first;

		rebuild_index_userbouquet(iname);
	}

	rebuild_index_services();
}

void e2db_utils::remove_duplicates_references()
{
	debug("remove_duplicates_references");

	for (auto & x : index)
	{
		string iname = x.first;

		rebuild_index_userbouquet(iname);
	}
}

void e2db_utils::remove_duplicates_markers()
{
	debug("remove_duplicates_markers");

	bool step = false;

	for (auto & x : userbouquets)
	{
		unordered_set<string> i_markers;
		unordered_set<string> _unique;
		userbouquet& ub = x.second;
		string bname = ub.bname;

		for (auto & x : ub.channels)
		{
			channel_reference& chref = x.second;

			if (chref.marker && ! chref.value.empty())
			{
				if (_unique.count(chref.value))
					i_markers.insert(chref.chid);
				else
					_unique.insert(chref.value);
			}
		}

		for (const string & chid : i_markers)
			ub.channels.erase(chid);

		rebuild_index_userbouquet(bname, i_markers);

		if (! step)
			step = ! i_markers.empty();
	}

	if (step)
		rebuild_index_markers();
}

void e2db_utils::transform_tunersets_to_transponders()
{
	debug("transform_tunersets_to_transponders");

	struct tx_part
	{
		string txy;
		transponder tx;
	};

	vector<tx_part> parts;

	for (auto & x : tuners)
	{
		tunersets& tv = x.second;
		int ytype = tv.ytype;

		string iname = "tns:";
		char yname = value_transponder_type(tv.ytype);
		iname += yname;

		if (! index.count(iname))
		{
			error("transform_tunersets_to_transponders", "Error", msg("Missing index key \"%s\".", iname));

			continue;
		}

		for (auto & x : index[iname])
		{
			string tnid = x.second;
			tunersets_table& tn = tv.tables[tnid];

			if (! index.count(tnid))
			{
				error("transform_tunersets_to_transponders", "Error", msg("Missing index key \"%s\".", tnid));

				continue;
			}

			for (auto & x : index[tnid])
			{
				int idx = x.first;
				string trid = x.second;
				tunersets_transponder& tntxp = tn.transponders[trid];

				tx_part txpart;
				transponder& tx = txpart.tx;

				tx.ytype = ytype;
				tx.freq = tntxp.freq;
				tx.sr = tntxp.sr;
				tx.pol = tntxp.pol;
				tx.fec = tntxp.fec;
				tx.hpfec = tntxp.hpfec;
				tx.lpfec = tntxp.lpfec;
				tx.cfec = tntxp.cfec;
				tx.pos = tn.pos != -1 ? tn.pos : 0;
				tx.inv = tntxp.inv;
				tx.flags = tn.flags;
				tx.sys = tntxp.sys;
				tx.mod = tntxp.mod;
				tx.tmod = tntxp.tmod;
				tx.cmod = tntxp.cmod;
				tx.amod = tntxp.amod;
				tx.rol = tntxp.rol;
				tx.pil = tntxp.pil;
				tx.band = tntxp.band;
				tx.tmx = tntxp.tmx;
				tx.guard = tntxp.guard;
				tx.hier = tntxp.hier;
				tx.plpid = tntxp.plpid;
				tx.plsn = tntxp.plsn;
				tx.plsmode = tntxp.plsmode;
				tx.plscode = tntxp.plscode;
				tx.isid = tntxp.isid;
				tx.index = idx;

				char txy[37];
				// %4x:%8x
				std::snprintf(txy, 37, "%d:%d:%x:%x:%x", tx.ytype, tx.pos, tx.freq, tx.sr, tx.pol);

				txpart.txy = txy;

				parts.emplace_back(txpart);
			}
		}
	}

	unordered_map<string, string> _index;

	for (auto & x : index["txs"])
	{
		transponder& tx = db.transponders[x.second];

		char txy[37];
		// %8d:%8d:%8x:%8x:%8x
		std::snprintf(txy, 37, "%d:%d:%x:%x:%x", tx.ytype, tx.pos, tx.freq, tx.sr, tx.pol);

		_index.emplace(pair (txy, tx.txid));
	}

	for (tx_part & txpart : parts)
	{
		string txy = txpart.txy;
		transponder& tx = txpart.tx;

		if (_index.count(txy))
		{
			string txid = _index[txy];
			transponder& txp = db.transponders[txid];

			tx.txid = txp.txid;
			tx.tsid = txp.tsid;
			tx.onid = txp.onid;
			tx.dvbns = txp.dvbns;
			tx.index = txp.index;

			db.transponders[tx.txid] = tx;
		}
		else if (0)
		{
			int idx = tx.index;

			tx.tsid = -idx;
			tx.dvbns = value_transponder_dvbns(tx);

			char txid[25];
			// %4x:%8x
			std::snprintf(txid, 25, "%x:%x", tx.tsid, tx.dvbns);
			tx.txid = txid;

			db.transponders.emplace(tx.txid, tx);
			index["txs"].emplace_back(pair (idx, tx.txid));
		}
	}
}

void e2db_utils::transform_transponders_to_tunersets()
{
	debug("transform_transponders_to_tunersets");

	struct tn_part
	{
		string txy;
		int ytype;
		int pos;
		int flags;
		tunersets_transponder tntxp;
	};
	struct tn_ref
	{
		int ytype;
		string tnid;
		string trid;
	};

	vector<tn_part> parts;

	for (auto & x : index["txs"])
	{
		transponder& tx = db.transponders[x.second];

		tn_part tnpart;

		tnpart.ytype = tx.ytype;
		tnpart.pos = tx.pos;
		tnpart.flags = tx.flags;
		tunersets_transponder& tntxp = tnpart.tntxp;

		tntxp.freq = tx.freq;
		tntxp.sr = tx.sr;
		tntxp.pol = tx.pol;
		tntxp.fec = tx.fec;
		tntxp.hpfec = tx.hpfec;
		tntxp.lpfec = tx.lpfec;
		tntxp.cfec = tx.cfec;
		tntxp.inv = tx.inv;
		tntxp.sys = tx.sys;
		tntxp.mod = tx.mod;
		tntxp.tmod = tx.tmod;
		tntxp.cmod = tx.cmod;
		tntxp.amod = tx.amod;
		tntxp.rol = tx.rol;
		tntxp.pil = tx.pil;
		tntxp.band = tx.band;
		tntxp.tmx = tx.tmx;
		tntxp.guard = tx.guard;
		tntxp.hier = tx.hier;
		tntxp.plpid = tx.plpid;
		tntxp.plsn = tx.plsn;
		tntxp.plsmode = tx.plsmode;
		tntxp.plscode = tx.plscode;
		tntxp.isid = tx.isid;

		char txy[37];
		// %4x:%8x
		std::snprintf(txy, 37, "%d:%d:%x:%x:%x", tx.ytype, tx.pos, tx.freq, tx.sr, tx.pol);

		tnpart.txy = txy; 

		parts.emplace_back(tnpart);
	}

	unordered_map<string, tn_ref> _index;

	for (auto & x : tuners)
	{
		tunersets& tv = x.second;
		int ytype = tv.ytype;

		string iname = "tns:";
		char yname = value_transponder_type(tv.ytype);
		iname += yname;

		if (! index.count(iname))
		{
			error("transform_transponders_to_tunersets", "Error", msg("Missing index key \"%s\".", iname));

			continue;
		}

		for (auto & x : index[iname])
		{
			string tnid = x.second;
			tunersets_table& tn = tv.tables[tnid];

			if (! index.count(tnid))
			{
				error("transform_transponders_to_tunersets", "Error", msg("Missing index key \"%s\".", tnid));

				continue;
			}

			for (auto & x : index[tnid])
			{
				string trid = x.second;
				tunersets_transponder& tntxp = tn.transponders[trid];

				int pos = tn.pos != -1 ? tn.pos : 0;

				tn_ref tnref;
				tnref.ytype = ytype;
				tnref.tnid = tnid;
				tnref.trid = trid;

				char txy[37];
				// %4x:%8x
				std::snprintf(txy, 37, "%d:%d:%x:%x:%x", tn.ytype, pos, tntxp.freq, tntxp.sr, tntxp.pol);

				_index.emplace(pair (txy, tnref));
			}
		}
	}

	for (tn_part & tnpart : parts)
	{
		string txy = tnpart.txy;
		tunersets_transponder& tntxp = tnpart.tntxp;

		if (_index.count(txy))
		{
			debug("transform_transponders_to_tunersets", "found txy", txy);

			tn_ref& tnref = _index[txy];
			int ytype = tnref.ytype;
			string tnid = tnref.tnid;
			string trid = tnref.trid;
			tunersets_transponder& txp = tuners[ytype].tables[tnid].transponders[trid];

			tntxp.trid = txp.trid;
			tntxp.mts = txp.mts;
			tntxp.t2mi_plpid = txp.t2mi_plpid;
			tntxp.t2mi_pid = txp.t2mi_pid;
			tntxp.index = txp.index;

			tuners[ytype].tables[tnid].transponders[trid] = tntxp;
		}
		else
		{
			debug("transform_transponders_to_tunersets", "add txy", txy);

			int ytype = tnpart.ytype;
			int pos = tnpart.pos;

			tunersets tv;
			tunersets_table tn;
			tunersets_transponder& tntxp = tnpart.tntxp;

			if (tuners.count(ytype))
			{
				tv = tuners[ytype];
			}
			else
			{
				tv.ytype = ytype;
				tv.charset = "utf-8";

				tuners.emplace(tv.ytype, tv);
			}

			bool found = false;

			if (ytype == YTYPE::satellite && tuners_pos.count(pos))
			{
				string tnid = tuners_pos[pos];

				if (tuners[0].tables.count(tnid))
				{
					found = true;
					tn = tuners[0].tables[tnid];
				}
			}

			if (! found)
			{
				string iname = "tns:";
				char yname = value_transponder_type(tn.ytype);
				iname += yname;

				int idx = int (index[iname].size());
				idx++;

				char tnid[25];
				std::snprintf(tnid, 25, "%c:%04x", yname, idx);
				tn.tnid = tnid;

				tn.ytype = ytype;
				tn.pos = pos;
				tn.name = value_transponder_position(tn);
				tn.flags = tnpart.flags;
				tn.index = idx;

				index[iname].emplace_back(pair (idx, tn.tnid));

				if (tn.ytype == YTYPE::satellite)
					tuners_pos.emplace(tn.pos, tn.tnid);
			}

			char yname = value_transponder_type(tn.ytype);

			int idx = int (index[tn.tnid].size());
			idx++;

			char trid[25];
			std::snprintf(trid, 25, "%c:%04x:%04x", yname, tntxp.freq, tntxp.sr);
			tntxp.trid = trid;

			tntxp.index = idx;

			tn.transponders.emplace(tntxp.trid, tntxp);
			index[tn.tnid].emplace_back(pair (idx, tntxp.trid));

			tuners[ytype].tables[tn.tnid] = tn;

			tn_ref tnref;
			tnref.ytype = ytype;
			tnref.tnid = tn.tnid;
			tnref.trid = tntxp.trid;

			_index.emplace(pair (txy, tnref));
		}
	}
}

void e2db_utils::sort_transponders()
{
	debug("sort_transponders");
}

void e2db_utils::sort_services()
{
	debug("sort_services");
}

void e2db_utils::sort_userbouquets()
{
	debug("sort_userbouquets");
}

void e2db_utils::sort_references()
{
	debug("sort_references");
}

void e2db_utils::rebuild_index_transponders()
{
	unordered_set<string> _unique;
	vector<pair<int, string>> txis;

	int idx = 0;
	for (auto & x : index["txs"])
	{
		string txid = x.second;

		if (! _unique.count(txid))
		{
			idx += 1;
			txis.emplace_back(pair (idx, txid));
			_unique.insert(txid);
		}
	}

	index["txs"].swap(txis);
}

void e2db_utils::rebuild_index_services()
{
	unordered_set<string> _unique;
	vector<pair<int, string>> chis;

	int idx = 0;
	for (auto & x : index["chs"])
	{
		string chid = x.second;

		if (! _unique.count(chid))
		{
			service& ch = db.services[chid];
			idx += 1;
			ch.index = idx;
			chis.emplace_back(pair (idx, chid));
			_unique.insert(chid);
		}
	}

	index["chs"].swap(chis);

	unordered_set<string> i_names = {"chs:1", "chs:2", "chs:0"};

	for (const string & iname : i_names)
	{
		vector<pair<int, string>> chis;

		for (auto & x : index[iname])
		{
			string chid = x.second;
			chis.emplace_back(pair (idx, chid));
		}

		index[iname].swap(chis);
	}
}

void e2db_utils::rebuild_index_services(unordered_set<string> i_channels)
{
	if (i_channels.empty())
		return;

	unordered_set<string> i_names = {"chs", "chs:1", "chs:2", "chs:0"};

	for (const string & iname : i_names)
	{
		vector<pair<int, string>> chis;

		for (auto & x : index[iname])
		{
			int idx = x.first;
			string chid = x.second;

			if (i_channels.count(chid))
				chis.emplace_back(pair (idx, chid));
		}

		index[iname].swap(chis);
	}
}

void e2db_utils::rebuild_index_userbouquet(string iname)
{
	if (! userbouquets.count(iname))
		return;

	unordered_set<string> _unique;
	vector<pair<int, string>> chis;

	int idx = 0;
	for (auto & x : index[iname])
	{
		string chid = x.second;

		if (! _unique.count(chid))
		{
			channel_reference& chref = userbouquets[iname].channels[x.second];

			if (! (chref.marker && chref.atype != ATYPE::marker_numbered))
			{
				idx += 1;
				chref.index = idx;
				chis.emplace_back(pair (idx, chid));
			}
			else
			{
				chis.emplace_back(pair (0, chid));
			}

			_unique.insert(chid);
		}
	}

	index[iname].swap(chis);
}

void e2db_utils::rebuild_index_userbouquet(string iname, unordered_set<string> i_channels)
{
	if (! userbouquets.count(iname) || i_channels.empty())
		return;

	vector<pair<int, string>> chis;

	int idx = 0;
	for (auto & x : index[iname])
	{
		string chid = x.second;

		if (! i_channels.count(chid))
		{
			channel_reference& chref = userbouquets[iname].channels[x.second];

			if (! (chref.marker && chref.atype != ATYPE::marker_numbered))
			{
				idx += 1;
				chref.index = idx;
				chis.emplace_back(pair (idx, chid));
			}
			else
			{
				chis.emplace_back(pair (0, chid));
			}
		}
	}

	index[iname].swap(chis);
}

void e2db_utils::rebuild_index_markers()
{
	vector<pair<int, string>> mkis;

	for (auto & x : userbouquets)
	{
		userbouquet& ub = x.second;

		for (auto & x : ub.channels)
		{
			string chid = x.first;
			channel_reference& chref = x.second;

			if (chref.marker)
				mkis.emplace_back(pair (ub.index, chid));
		}
	}

	index["mks"].swap(mkis);
}

}
