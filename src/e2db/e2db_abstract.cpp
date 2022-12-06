/*!
 * e2-sat-editor/src/e2db/e2db_abstract.cpp
 * 
 * @link https://github.com/ctlcltd/e2-sat-editor
 * @copyright e2 SAT Editor Team
 * @author Leonardo Laureti
 * @version 0.1
 * @license MIT License
 * @license GNU GPLv3 License
 */

#include <cstdio>
#include <cstring>
#include <algorithm>

#include "e2db_abstract.h"

using std::string, std::pair, std::to_string, std::cout, std::endl;

namespace e2se_e2db
{

void e2db_abstract::debug(string msg)
{
	this->log->debug(msg);
}

void e2db_abstract::debug(string msg, string optk, string optv)
{
	this->log->debug(msg, optk, optv);
}

void e2db_abstract::debug(string msg, string optk, int optv)
{
	this->log->debug(msg, optk, std::to_string(optv));
}

void e2db_abstract::info(string msg)
{
	this->log->info(msg);
}

void e2db_abstract::info(string msg, string optk, string optv)
{
	this->log->info(msg, optk, optv);
}

void e2db_abstract::info(string msg, string optk, int optv)
{
	this->log->info(msg, optk, std::to_string(optv));
}

void e2db_abstract::error(string msg)
{
	this->log->error(msg);
}

void e2db_abstract::error(string msg, string optk, string optv)
{
	this->log->error(msg, optk, optv);
}

void e2db_abstract::error(string msg, string optk, int optv)
{
	this->log->error(msg, optk, std::to_string(optv));
}

void e2db_abstract::add_transponder(int idx, transponder& tx)
{
	char txid[25];
	// %4x:%8x
	std::sprintf(txid, "%x:%x", tx.tsid, tx.dvbns);
	tx.txid = txid;

	tx.index = idx;
	db.transponders.emplace(tx.txid, tx);
	index["txs"].emplace_back(pair (idx, tx.txid)); //C++17
}

void e2db_abstract::add_service(int idx, service& ch)
{
	char chid[25];
	char txid[25];
	// %4x:%8x
	std::sprintf(txid, "%x:%x", ch.tsid, ch.dvbns);
	// %4x:%4x:%8x
	std::sprintf(chid, "%x:%x:%x", ch.ssid, ch.tsid, ch.dvbns);
	ch.txid = txid;
	ch.chid = chid;

	if (db.services.count(ch.chid))
	{
		int m;
		string kchid = 's' + ch.chid;
		if (ch.snum) m = ch.snum;
		else m = collisions[kchid].size();
		ch.chid += ':' + to_string(m);
		collisions[kchid].emplace_back(pair (ch.chid, m)); //C++17
	}

	string iname = "chs:" + (STYPES.count(ch.stype) ? to_string(STYPES.at(ch.stype).first) : "0");
	ch.index = idx;
	db.services.emplace(ch.chid, ch);
	index["chs"].emplace_back(pair (idx, ch.chid)); //C++17
	index[iname].emplace_back(pair (idx, ch.chid)); //C++17
}

void e2db_abstract::add_bouquet(int idx, bouquet& bs)
{
	bs.index = idx;
	index["bss"].emplace_back(pair (idx, bs.bname)); //C++17
	bouquets.emplace(bs.bname, bs);
}

void e2db_abstract::add_userbouquet(int idx, userbouquet& ub)
{
	ub.index = idx;
	index["ubs"].emplace_back(pair (idx, ub.bname)); //C++17
	bouquets[ub.pname].userbouquets.emplace_back(ub.bname);
	userbouquets.emplace(ub.bname, ub);
}

void e2db_abstract::add_channel_reference(int idx, userbouquet& ub, channel_reference& chref, service_reference& ref)
{
	char chid[25];

	if (chref.marker)
		// %4d:%2x:%d
		std::sprintf(chid, "%d:%x:%d", chref.type, chref.anum, ub.index);
	else
		// %4x:%4x:%8x
		std::sprintf(chid, "%x:%x:%x", ref.ssid, ref.tsid, ref.dvbns);

	chref.chid = chid;
	chref.index = idx;

	ub.channels.emplace(chref.chid, chref);
	index[ub.bname].emplace_back(pair (idx, chref.chid)); //C++17

	if (chref.marker)
		index["mks"].emplace_back(pair (ub.index, chref.chid)); //C++17
	else
		index[ub.pname].emplace_back(pair ((index[ub.pname].size() + 1), chref.chid)); //C++17
}

void e2db_abstract::set_channel_reference_marker_value(userbouquet& ub, string chid, string value)
{
	ub.channels[chid].value = value;
}

void e2db_abstract::add_tunersets(tunersets& tv)
{
	tuners.emplace(tv.ytype, tv);
}

void e2db_abstract::add_tunersets_table(int idx, tunersets& tv, tunersets_table& tn)
{
	string iname = "tns:";
	char type;
	switch (tn.ytype)
	{
		case YTYPE::sat: type = 's'; break;
		case YTYPE::terrestrial: type = 't'; break;
		case YTYPE::cable: type = 'c'; break;
		case YTYPE::atsc: type = 'a'; break;
		default: return error("add_tunersets_table()", "Error", "Unknown tuner settings type.");
	}
	iname += type;
	char tnid[25];
	std::sprintf(tnid, "%c:%04x", type, idx);
	tn.tnid = tnid;
	tn.index = idx;
	tv.tables.emplace(tn.tnid, tn);
	index[iname].emplace_back(pair (idx, tn.tnid)); //C++17
	if (tn.ytype == YTYPE::sat)
		tuners_pos.emplace(tn.pos, tn.tnid);
}

void e2db_abstract::add_tunersets_transponder(int idx, tunersets_table& tn, tunersets_transponder& tntxp)
{
	char type;
	switch (tn.ytype)
	{
		case YTYPE::sat: type = 's'; break;
		case YTYPE::terrestrial: type = 't'; break;
		case YTYPE::cable: type = 'c'; break;
		case YTYPE::atsc: type = 'a'; break;
		default: return error("add_tunersets_transponder()", "Error", "Unknown tuner settings type.");
	}
	char trid[25];
	std::sprintf(trid, "%c:%04x:%04x", type, tntxp.freq, tntxp.sr);
	tntxp.trid = trid;
	tntxp.index = idx;
	tn.transponders.emplace(tntxp.trid, tntxp);
	index[tn.tnid].emplace_back(pair (idx, tntxp.trid)); //C++17
}

}
