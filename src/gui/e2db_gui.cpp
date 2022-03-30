/*!
 * e2-sat-editor/src/gui/e2db_gui.cpp
 * 
 * @link https://github.com/ctlcltd/e2-sat-editor
 * @copyright e2 SAT Editor Team
 * @author Leonardo Laureti
 * @version 0.1
 * @license MIT License
 * @license GNU GPLv3 License
 */

#include <cstdio>
#include <cmath>

#include "e2db_gui.h"

using std::to_string;

namespace e2se_gui
{
e2db::e2db()
{
	debug("gui.e2db()");

	this->sets = new QSettings;
	options();
}

void e2db::options()
{
	debug("options()");

	// e2db::DEBUG = sets->value("debug", true).toBool();
	e2db::PARSER_TUNERSETS = sets->value("application/parserTunerset", true).toBool();
	e2db::PARSER_LAMEDB5_PRIOR = sets->value("application/parserLamedb5", false).toBool();
	e2db::MAKER_LAMEDB5 = sets->value("application/makerLamedb5", true).toBool();
}

string e2db::add_transponder(transponder& tx)
{
	debug("add_transponder()", "txid", tx.txid);

	this->::e2se_e2db::e2db::add_transponder(tx);
	tx = db.transponders[tx.txid];
	entries.transponders[tx.txid] = entry_transponder(tx);
	return tx.txid;
}

string e2db::edit_transponder(string txid, transponder& tx)
{
	debug("edit_transponder()", "txid", txid);

	this->::e2se_e2db::e2db::edit_transponder(txid, tx);
	tx = db.transponders[tx.txid];
	entries.transponders[tx.txid] = entry_transponder(tx);
	return tx.txid;
}

void e2db::remove_transponder(string txid)
{
	debug("remove_transponder()", "txid", txid);

	this->::e2se_e2db::e2db::remove_transponder(txid);
	entries.transponders.erase(txid);
}

string e2db::add_service(service& ch)
{
	debug("add_service()", "chid", ch.chid);

	this->::e2se_e2db::e2db::add_service(ch);
	ch = db.services[ch.chid];
	entries.services[ch.chid] = entry_service(ch);
	return ch.chid;
}

string e2db::edit_service(string chid, service& ch)
{
	debug("edit_service()", "chid", chid);

	this->::e2se_e2db::e2db::edit_service(chid, ch);
	ch = db.services[ch.chid];
	entries.services[ch.chid] = entry_service(ch);
	return ch.chid;
}

void e2db::remove_service(string chid)
{
	debug("remove_service()", "chid", chid);

	this->::e2se_e2db::e2db::remove_service(chid);
	entries.services.erase(chid);
}

bool e2db::prepare(string localdir)
{
	debug("prepare()");

	if (! this->read(localdir))
		return false;

	if (sets->value("application/parserDebugger", false).toBool())
		this->debugger();

	this->gindex = index;

	for (auto & txdata : db.transponders)
	{
		entries.transponders[txdata.first] = entry_transponder(txdata.second);
	}
	for (auto & chdata : db.services)
	{
		entries.services[chdata.first] = entry_service(chdata.second);
	}
	return true;
}

bool e2db::write(string localdir, bool overwrite)
{
	debug("write()");

	this->set_index(gindex);

	if (! this->::e2se_e2db::e2db::write(localdir, overwrite))
		return false;

	return true;
}

void e2db::updateUserbouquetIndexes()
{
	debug("updateUserbouquetIndexes()");

	this->gindex = index;
}

void e2db::updateUserbouquetIndexes(string chid, string nw_chid)
{
	debug("updateUserbouquetIndexes()", "chid|nw_chid", chid + '|' + nw_chid);

	for (auto & g : gindex)
	{
		if (g.first.find("chs") != string::npos || g.first == "txs")
			continue;

		for (auto & s : g.second)
		{
			if (s.second == chid)
				s.second = nw_chid;
		}
	}
}

QStringList e2db::entry_transponder(transponder tx)
{
	QString freq = QString::fromStdString(to_string(tx.freq));
	QString pol = QString::fromStdString(tx.pol != -1 ? e2db::SAT_POL[tx.pol] : "");
	QString sr = QString::fromStdString(to_string(tx.sr));
	QString fec = QString::fromStdString(e2db::SAT_FEC[tx.fec]);
	string ppos;
	if (tx.ttype == 's')
	{
		if (tuners.count(tx.pos))
		{
			ppos = tuners.at(tx.pos).name;
		} else {
			char cposdeg[5];
			sprintf(cposdeg, "%.1f", float (abs (tx.pos)) / 10);
			ppos = (string (cposdeg) + (tx.pos > 0 ? 'E' : 'W'));
		}
	}
	QString pos = QString::fromStdString(ppos);
	string psys;
	if (tx.ttype == 's')
		psys = tx.sys != -1 ? e2db::SAT_SYS[tx.sys] : "DVB-S";
	else if (tx.ttype == 't')
		psys = "DVB-T"; //TODO terrestrial.xml
	else if (tx.ttype == 'c')
		psys = "DVB-C";
	else if (tx.ttype == 'a')
		psys = "ATSC";
	QString sys = QString::fromStdString(psys);

	return QStringList ({freq, pol, sr, fec, pos, sys});
}

QStringList e2db::entry_service(service ch)
{
	// macos: unwanted chars [qt.qpa.fonts] Menlo notice
	QString chname;
	if (sets->value("preference/fixUnicodeChars").toBool())
		chname = QString::fromStdString(ch.chname).remove(QRegularExpression("[^\\p{L}\\p{N}\\p{Sm}\\p{M}\\p{P}\\s]+"));
	else
		chname = QString::fromStdString(ch.chname);
	QString chid = QString::fromStdString(ch.chid);
	QString txid = QString::fromStdString(ch.txid);
	QString stype = e2db::STYPES.count(ch.stype) ? QString::fromStdString(e2db::STYPES.at(ch.stype).second) : "Data";
	QString pname = QString::fromStdString(ch.data.count(e2db::PVDR_DATA.at('p')) ? ch.data[e2db::PVDR_DATA.at('p')][0] : "");

	QStringList entry = QStringList ({chname, chid, txid, stype, pname});
	entry.append(entries.transponders[ch.txid]);
	return entry;
}

//TODO marker QWidget ?
QStringList e2db::entry_marker(reference cref)
{
	QString chid = QString::fromStdString(cref.chid);
	QString refval = QString::fromStdString(cref.refval);

	return QStringList({"", refval, chid, "", "MARKER"});
}

}
