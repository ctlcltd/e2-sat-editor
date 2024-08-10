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

#include <cstdio>
#include <clocale>
#include <stdexcept>

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
		string chid = x.first;
		service& ch = x.second;

		if (ch.txid.empty() || ch.tsid < 1 || ch.onid < 1)
			i_services.insert(ch.chid);
	}

	for (const string & chid : i_services)
	{
		db.services.erase(chid);
	}

	unordered_set<string> i_names;

	for (auto & x : index)
	{
		vector<vector<pair<int, string>>::iterator> itx;
		for (auto it = x.second.begin(); it != x.second.end(); it++)
		{
			if (i_services.count(it->second))
			{
				itx.push_back(it);
				i_names.insert(x.first);
			}
		}
		for (auto it = itx.begin(); it != itx.end(); it++)
		{
			x.second.erase(*it);
		}
	}
	for (const string & iname : i_names)
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

		for (const string & chid : i_services)
		{
			userbouquets[iname].channels.erase(chid);
		}
	}
}

void e2db_utils::remove_orphaned_references()
{
	debug("remove_orphaned_references");

	unordered_set<string> i_channels;
	unordered_set<string> i_names;

	for (auto & x : userbouquets)
	{
		userbouquet& ub = x.second;

		for (auto & x : ub.channels)
		{
			string chid = x.first;
			channel_reference& chref = x.second;

			if (! chref.marker && ! chref.stream && ! db.services.count(chref.chid))
			{
				i_channels.insert(chref.chid);
				i_names.insert(ub.bname);
			}
		}
	}
	for (const string & iname : i_names)
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

		for (const string & chid : i_channels)
		{
			userbouquets[iname].channels.erase(chid);
		}
	}
}

void e2db_utils::fix_remove_references()
{
	debug("fix_remove_references");

	unordered_set<string> i_channels;
	unordered_set<string> i_names;

	for (auto & x : userbouquets)
	{
		userbouquet& ub = x.second;
		string bname = ub.bname;

		if (! index.count(bname))
			return;

		for (auto & x : index[bname])
		{
			string chid = x.second;
			bool ref_error = false;

			if (userbouquets[bname].channels.count(chid))
			{
				channel_reference& chref = userbouquets[bname].channels[chid];

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
			{
				i_channels.insert(chid);
				i_names.insert(bname);
			}
		}
	}
	for (const string & iname : i_names)
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

		for (const string & chid : i_channels)
		{
			userbouquets[iname].channels.erase(chid);
		}
	}
}

void e2db_utils::fix_dvbns()
{
	debug("fix_dvbns");

	unordered_set<string> i_transponders;

	for (auto & x : db.transponders)
	{
		string txid = x.first;
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
		ch.data = {};
	}
}

void e2db_utils::clear_favourites()
{
	debug("clear_favourites");

	unordered_set<string> i_channels;
	unordered_set<string> i_names;

	for (auto & x : userbouquets)
	{
		userbouquet& ub = x.second;

		for (auto & x : ub.channels)
		{
			string chid = x.first;
			channel_reference& chref = x.second;

			if (chref.stream || ! db.services.count(chref.chid))
			{
				i_channels.insert(chref.chid);
				i_names.insert(ub.bname);
			}
		}
	}
	for (const string & iname : i_names)
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

		for (const string & chid : i_channels)
		{
			userbouquets[iname].channels.erase(chid);
		}
	}
}

void e2db_utils::clear_bouquets_unused_services()
{
	debug("clear_bouquets_unused_services");

	unordered_set<string> i_channels;

	for (auto & x : userbouquets)
	{
		userbouquet& ub = x.second;
		string bname = ub.bname;

		for (auto & x : userbouquets[bname].channels)
		{
			channel_reference& chref = x.second;

			if (! db.services.count(chref.chid))
			{
				i_channels.insert(chref.chid);
			}
		}
	}
	for (const string & chid : i_channels)
	{
		db.services.erase(chid);

		for (auto & x : userbouquets)
		{
			x.second.channels.erase(chid);
		}
	}
	for (auto & x : index)
	{
		vector<vector<pair<int, string>>::iterator> itx;
		for (auto it = x.second.begin(); it != x.second.end(); it++)
		{
			if (! i_channels.count(it->second))
			{
				itx.push_back(it);
			}
		}
		for (auto it = itx.begin(); it != itx.end(); it++)
		{
			x.second.erase(*it);
		}
	}
}

//TODO rename
void e2db_utils::remove_parentallock_lists()
{
	debug("remove_parentallock_lists");

	for (auto & x : db.services)
	{
		string chid = x.first;
		service& ch = x.second;

		ch.parental = false;
	}
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
}

void e2db_utils::remove_duplicates()
{
	debug("remove_duplicates");
}

void e2db_utils::remove_duplicates_transponders()
{
	debug("remove_duplicates_transponders");
}

void e2db_utils::remove_duplicates_services()
{
	debug("remove_duplicates_services");
}

void e2db_utils::remove_duplicates_references()
{
	debug("remove_duplicates_references");
}

void e2db_utils::remove_duplicates_markers()
{
	debug("remove_duplicates_markers");
}

void e2db_utils::transform_tunersets_to_transponders()
{
	debug("transform_tunersets_to_transponders");
}

void e2db_utils::transform_transponders_to_tunersets()
{
	debug("transform_transponders_to_tunersets");
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

}
