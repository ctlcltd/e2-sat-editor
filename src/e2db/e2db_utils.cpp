/*!
 * e2-sat-editor/src/e2db/e2db_utils.cpp
 * 
 * @link https://github.com/ctlcltd/e2-sat-editor
 * @copyright e2 SAT Editor Team
 * @author Leonardo Laureti
 * @version 1.8.0
 * @license MIT License
 * @license GNU GPLv3 License
 */

#include <clocale>

#include "e2db_utils.h"

using std::to_string;

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

				if (chref.marker)
				{
					if (! is_valid_marker(chref))
						ref_error = true;
				}
				else if (chref.stream)
				{
					if (! is_valid_stream(chref))
						ref_error = true;
				}
				else if (db.services.count(chid))
				{
					service& ch = db.services[chid];

					if (ch.chid != chref.chid)
						ref_error = true;
					if (! is_valid_service(ch))
						ref_error = true;
				}
				else
				{
					ref_error = true;
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

void e2db_utils::fix_bouquets(bool uniq_ubouquets)
{
	debug("fix_bouquets");

	unordered_set<string> _remove;

	for (auto & x : userbouquets)
	{
		userbouquet& ub = x.second;
		string bname = ub.bname;

		if (! index.count(bname))
		{
			if (! ub.channels.empty())
				index[bname]; // touch
			else
				_remove.insert(bname);
		}
	}

	if (! _remove.empty())
	{
		vector<pair<int, string>> i_ubouquets;

		for (auto & x : index["ubs"])
		{
			string bname = x.second;

			if (! _remove.count(bname))
				i_ubouquets.emplace_back(x);
		}

		index["ubs"].swap(i_ubouquets);

		for (auto & x : bouquets)
		{
			bouquet& bs = x.second;

			vector<string> ubouquets;

			for (string & w : bs.userbouquets)
			{
				userbouquet& ub = userbouquets[w];
				string bname = ub.bname;

				if (! _remove.count(bname))
					ubouquets.emplace_back(bname);
			}

			bs.userbouquets.swap(ubouquets);
		}

		for (const string & bname : _remove)
		{
			userbouquets.erase(bname);
		}

		_remove.clear();
	}

	this->::e2se_e2db::e2db_abstract::fix_bouquets(uniq_ubouquets);
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

	if (chs_changes.empty())
		return;

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
				channel_reference chref = ub.channels[chid];

				if (chs_changes.count(chid))
				{
					string nw_chid = chs_changes[chid];

					if (! chref.stream)
					{
						service& ch = db.services[nw_chid];
						chref.ref.dvbns = ch.dvbns;
					}

					chref.chid = nw_chid;
					x.second = nw_chid;
				}

				channels.emplace(chref.chid, chref);
			}

			ub.channels.swap(channels);
		}
	}

	for (auto & x : bouquets)
	{
		bouquet& bs = x.second;
		string bname = bs.bname;

		bs.services.clear();
		index[bname].clear();
	}
	for (auto & x : index["ubs"])
	{
		userbouquet ub = userbouquets[x.second];
		string bname = ub.bname;
		string pname = ub.pname;

		if (! index.count(bname))
		{
			error("fix_dvbns", "Error", msg("Missing index key \"%s\".", bname));

			continue;
		}
		if (! index.count(pname))
		{
			error("fix_dvbns", "Error", msg("Missing index key \"%s\".", pname));

			continue;
		}

		bouquet& bs = bouquets[pname];

		int idx = 0;
		for (auto & x : index[bname])
		{
			channel_reference& chref = ub.channels[x.second];

			if (! chref.marker && ! chref.stream)
			{
				idx = int (index[pname].size());
				idx += 1;
				bs.services.emplace(chref.chid);
				index[pname].emplace_back(pair (idx, chref.chid));
			}
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
		for (auto & x : x.second)
		{
			string chid = x.first;
			i_services.insert(chid);
		}
	}

	for (const string & chid : i_services)
		db.services.erase(chid);

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

			for (const string & chid : i_services)
				ub.channels.erase(chid);
		}

		for (auto & x : bouquets)
		{
			bouquet& bs = x.second;

			for (const string & chid : i_services)
				bs.services.erase(chid);
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
				tx.mispls = tntxp.plscode != -1 || tntxp.plsmode != -1 || tntxp.isid != -1;
				tx.t2mi = tntxp.t2mi_plpid != -1 || tntxp.t2mi_pid != -1;
				tx.isid = tntxp.isid;
				tx.plscode = tntxp.plscode;
				tx.plsmode = tntxp.plsmode;
				tx.t2mi_plpid = tntxp.t2mi_plpid;
				tx.t2mi_pid = tntxp.t2mi_pid;
				tx.index = idx;

				char txy[37];
				// %d:%4d:%8x:%8x:%8x
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
		// %d:%4d:%8x:%8x:%8x
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
		tntxp.plscode = tx.plscode;
		tntxp.plsmode = tx.plsmode;
		tntxp.isid = tx.isid;
		tntxp.t2mi_plpid = tx.t2mi_plpid;
		tntxp.t2mi_pid = tx.t2mi_pid;

		char txy[37];
		// %d:%4d:%8x:%8x:%8x
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
				// %d:%4d:%8x:%8x:%8x
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

			if (ytype == YTYPE::satellite && tnloc.count(pos))
			{
				string tnid = tnloc[pos];

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
					tnloc.emplace(tn.pos, tn.tnid);
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

void e2db_utils::sort_transponders(uoopts& opts)
{
	opts.iname = "txs";
	if (opts.prop.empty()) opts.prop = "tsid";

	debug("sort_transponders", "prop", opts.prop);
	debug("sort_transponders", "order", value_sort_order(opts.order));

	sort_items(SORT_ITEM::item_transponder, opts);
}

void e2db_utils::sort_services(uoopts& opts)
{
	opts.iname = "chs";
	if (opts.prop.empty()) opts.prop = "ssid";

	debug("sort_services", "prop", opts.prop);
	debug("sort_services", "order", value_sort_order(opts.order));

	sort_items(SORT_ITEM::item_service, opts);
}

void e2db_utils::sort_userbouquets(uoopts& opts)
{
	opts.iname = "ubs";
	if (opts.prop.empty()) opts.prop = "bname";

	debug("sort_userbouquets", "prop", opts.prop);
	debug("sort_userbouquets", "order", value_sort_order(opts.order));

	sort_items(SORT_ITEM::item_userbouquet, opts);
}

void e2db_utils::sort_references(uoopts& opts)
{
	if (opts.prop.empty()) opts.prop = "chname";

	if (opts.iname.empty())
	{
		debug("sort_references", "prop", opts.prop);
		debug("sort_references", "order", value_sort_order(opts.order));

		for (auto & x : index["ubs"])
			sort_items(SORT_ITEM::item_reference, opts);
	}
	else
	{
		debug("sort_references", "bname", opts.iname);
		debug("sort_references", "prop", opts.prop);
		debug("sort_references", "order", value_sort_order(opts.order));

		if (! userbouquets.count(opts.iname))
			return error("sort_references", "Error", msg("Userbouquet \"%s\" not exists.", opts.iname));
		if (! index.count(opts.iname))
			return error("sort_references", "Error", msg("Missing index key \"%s\".", opts.iname));

		sort_items(SORT_ITEM::item_reference, opts);
	}
}

void e2db_utils::rebuild_index_transponders()
{
	unordered_set<string> _unique;
	vector<pair<int, string>> txis;

	int idx = 0;
	for (auto & x : index["txs"])
	{
		string txid = x.second;

		if (db.transponders.count(txid) && ! _unique.count(txid))
		{
			transponder& tx = db.transponders[txid];
			idx += 1;
			tx.index = idx;
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

		if (db.services.count(chid) && ! _unique.count(chid))
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

		int idx = 0;
		for (auto & x : index[iname])
		{
			idx = x.first;
			string chid = x.second;

			if (db.services.count(chid))
				chis.emplace_back(pair (idx, chid));
		}

		index[iname].swap(chis);
	}
}

void e2db_utils::rebuild_index_services(unordered_set<string> i_services)
{
	if (i_services.empty())
		return;

	unordered_set<string> i_names = {"chs", "chs:1", "chs:2", "chs:0"};

	for (const string & iname : i_names)
	{
		vector<pair<int, string>> chis;

		int idx = 0;
		for (auto & x : index[iname])
		{
			idx = x.first;
			string chid = x.second;

			if (i_services.count(chid))
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

e2db_utils::sort_data e2db_utils::get_data(SORT_ITEM model, string iname, vector<pair<int, string>> xis, string prop)
{
	sort_data s;

	if (! index.count(iname))
	{
		error("get_data", "Error", msg("Missing index key \"%s\".", iname));

		return s;
	}

	if (model == SORT_ITEM::item_userbouquet)
	{
		for (auto & x : xis)
		{
			userbouquet& ub = userbouquets[x.second];

			if (prop == "ubname")
			{
				if (ub.rname.empty())
					s.push(&ub.bname);
				else
					s.push(&ub.rname);
			}
			else if (prop == "name") s.push(&ub.name);
			else if (prop == "pname") s.push(&ub.pname);
			else if (prop == "bname") s.push(&ub.bname);
			else if (prop == "rname") s.push(&ub.rname);
			else if (prop == "utype") s.push(&ub.utype);
			else if (prop == "parental") s.push(&ub.parental);
			else if (prop == "index") s.push(&ub.index);
		}
	}
	else if (model == SORT_ITEM::item_reference)
	{
		if (userbouquets.count(iname))
		{
			userbouquet& ub = userbouquets[iname];

			for (auto & x : xis)
			{
				channel_reference& chref = ub.channels[x.second];

				if (prop == "chname")
				{
					if (! chref.marker && db.services.count(chref.chid))
					{
						service& ch = db.services[chref.chid];

						s.push(&ch.chname);
					}
					else
					{
						s.push(&chref.value);
					}
				}
				else if (prop == "ssid") s.push(&chref.ref.ssid);
				else if (prop == "tsid") s.push(&chref.ref.tsid);
				else if (prop == "onid") s.push(&chref.ref.onid);
				else if (prop == "dvbns") s.push(&chref.ref.dvbns);
				else if (prop == "url") s.push(&chref.url);
				else if (prop == "value") s.push(&chref.value);
				else if (prop == "inum") s.push(&chref.inum);
				else if (prop == "anum") s.push(&chref.anum);
				else if (prop == "etype") s.push(&chref.etype);
				else if (prop == "atype") s.push(&chref.atype);
				else if (prop == "marker") s.push(&chref.marker);
				else if (prop == "stream") s.push(&chref.stream);
				else if (prop == "index") s.push(&chref.index);
				else if (prop == "txr")
				{
					string id;

					if (db.services.count(chref.chid))
					{
						service& ch = db.services[chref.chid];
						string txid = ch.txid;

						if (db.transponders.count(txid))
						{
							transponder& tx = db.transponders[txid];
							id = to_string(tx.index) + ':' + to_string(ch.ssid);
						}
						else
						{
							id = "i:" + to_string(ch.tsid) + ':' + to_string(ch.ssid);
						}
					}
					else
					{
						id = "i:" + to_string(chref.inum);
					}

					s.store(id);
				}
			}
		}
	}
	else if (model == SORT_ITEM::item_service)
	{
		for (auto & x : xis)
		{
			service& ch = db.services[x.second];

			if (prop == "ssid") s.push(&ch.ssid);
			else if (prop == "tsid") s.push(&ch.tsid);
			else if (prop == "onid") s.push(&ch.onid);
			else if (prop == "dvbns") s.push(&ch.dvbns);
			else if (prop == "chname") s.push(&ch.chname);
			else if (prop == "sdata_p") s.push(&ch.data[SDATA::p][0]);
			else if (prop == "stype") s.push(&ch.stype);
			else if (prop == "snum") s.push(&ch.snum);
			else if (prop == "srcid") s.push(&ch.srcid);
			else if (prop == "parental") s.push(&ch.parental);
			else if (prop == "index") s.push(&ch.index);
			//TODO prop: data
			else if (prop == "txr")
			{
				string id;
				string txid = ch.txid;

				if (db.transponders.count(txid))
				{
					transponder& tx = db.transponders[txid];
					id = to_string(tx.index) + ':' + to_string(ch.ssid);
				}
				else
				{
					id = "i:" + to_string(ch.tsid) + ':' + to_string(ch.ssid);
				}

				s.store(id);
			}
		}
	}
	else if (model == SORT_ITEM::item_transponder)
	{
		for (auto & x : xis)
		{
			transponder& tx = db.transponders[x.second];

			if (prop == "tsid") s.push(&tx.tsid);
			else if (prop == "onid") s.push(&tx.onid);
			else if (prop == "dvbns") s.push(&tx.dvbns);
			else if (prop == "ytype") s.push(&tx.ytype);
			else if (prop == "pos") s.push(&tx.pos);
			else if (prop == "freq") s.push(&tx.freq);
			else if (prop == "sr") s.push(&tx.sr);
			else if (prop == "pol") s.push(&tx.pol);
			else if (prop == "sys") s.push(&tx.sys);
			else if (prop == "index") s.push(&tx.index);
			//TODO type: combo
		}
	}

	return s;
}

void e2db_utils::sort_items(SORT_ITEM model, uoopts& opts)
{
	if (! index.count(opts.iname))
		return error("sort_items", "Error", msg("Missing index key \"%s\".", opts.iname));

	string& iname = opts.iname;
	bool& selecting = opts.selecting;
	vector<int>& selection = opts.selection;
	string& prop = opts.prop;
	SORT_ORDER& order = opts.order;

	int len = int (! selecting ? index[iname].size() : selection.size());
	vector<pair<int, string>> xis;

	if (! selecting)
	{
		xis = index[iname];
	}
	else
	{
		for (auto it = selection.begin(); it != selection.end(); it++)
		{
			int j = *it;
			xis.emplace_back(index[iname][j]);
		}
	}

	vector<int> origin (len);
	vector<pair<sort_data::value*, int>> sorting (len);

	sort_data s = get_data(model, iname, xis, prop);

	for (int i = 0; i < len; ++i)
	{
		sorting[i].first = s.data[i];

		if (! selecting)
		{
			sorting[i].second = i;
			origin[i] = i;
		}
		else
		{
			sorting[i].second = selection[i];
			origin[i] = selection[i];
		}
	}

	sort_compare(sorting, order);

	vector<pair<int, string>>& i_a = index[iname];
	vector<pair<int, string>> i_b = index[iname];

	int i = 0;
	int a, b;
	for (auto it = origin.begin(); it != origin.end(); it++)
	{
		a = *it;
		b = sorting[i++].second;
		i_b[a].first = i_a[b].first;
		i_b[a].second = i_a[b].second;
	}

	int idx = 1;
	for (auto it = i_b.begin(); it != i_b.end(); it++)
	{
		it->first = it->first ? idx++ : 0;
	}

	i_a.swap(i_b);
}

string e2db_utils::value_sort_order(SORT_ORDER order)
{
	if (order == SORT_ORDER::sort_asc) return "asc";
	else if (order == SORT_ORDER::sort_desc) return "desc";
	return "";
}

}
