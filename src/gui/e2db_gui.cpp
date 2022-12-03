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
#include <ctime>
#include <clocale>
#include <cmath>
#include <unordered_set>

#include <QMessageBox>

#include "e2db_gui.h"

using std::to_string, std::unordered_set;

namespace e2se_gui
{

e2db::e2db(e2se::logger::session* log)
{
	std::setlocale(LC_NUMERIC, "C");

	this->log = new e2se::logger(log, "e2db");
	debug("gui.e2db()");

	this->sets = new QSettings;
	options();
	plain();
}

void e2db::options()
{
	debug("options()");

	e2db::PARSER_LAMEDB5_PRIOR = sets->value("application/parserLamedb5", false).toBool();
	e2db::MAKER_LAMEDB5 = sets->value("application/makerLamedb5", true).toBool();
	e2db::PARSER_TUNERSETS = sets->value("application/parserTunerset", true).toBool();
	e2db::MAKER_TUNERSETS = sets->value("application/makerTunerset", true).toBool();
}

void e2db::error(string msg, string optk, string optv)
{
	debug("gui.error()");

	this->::e2se_e2db::e2db::error(msg, optk, optv);
	QMessageBox::critical(nullptr, NULL, QString::fromStdString(optv));
}

string e2db::addTransponder(transponder& tx)
{
	debug("addTransponder()", "txid", tx.txid);

	this->::e2se_e2db::e2db::add_transponder(tx);
	tx = db.transponders[tx.txid];
	entries.transponders[tx.txid] = entryTransponder(tx);
	return tx.txid;
}

string e2db::editTransponder(string txid, transponder& tx)
{
	debug("editTransponder()", "txid", txid);

	this->::e2se_e2db::e2db::edit_transponder(txid, tx);
	tx = db.transponders[tx.txid];
	entries.transponders[tx.txid] = entryTransponder(tx);
	return tx.txid;
}

void e2db::removeTransponder(string txid)
{
	debug("removTransponder()", "txid", txid);

	this->::e2se_e2db::e2db::remove_transponder(txid);
	entries.transponders.erase(txid);
}

string e2db::addService(service& ch)
{
	debug("addService()", "chid", ch.chid);

	this->::e2se_e2db::e2db::add_service(ch);
	ch = db.services[ch.chid];
	entries.services[ch.chid] = entryService(ch);
	return ch.chid;
}

string e2db::editService(string chid, service& ch)
{
	debug("editService()", "chid", chid);

	this->::e2se_e2db::e2db::edit_service(chid, ch);
	ch = db.services[ch.chid];
	entries.services[ch.chid] = entryService(ch);
	return ch.chid;
}

void e2db::removeService(string chid)
{
	debug("removeService()", "chid", chid);

	this->::e2se_e2db::e2db::remove_service(chid);
	entries.services.erase(chid);
}

string e2db::addUserbouquet(userbouquet& ub)
{
	debug("addUserbouquet()");

	this->::e2se_e2db::e2db::add_userbouquet(ub);
	return ub.bname;
}

string e2db::editUserbouquet(userbouquet& ub)
{
	debug("editUserbouquet()");

	this->::e2se_e2db::e2db::edit_userbouquet(ub);
	return ub.bname;
}

void e2db::removeUserbouquet(string bname)
{
	debug("removeUserbouquet()", "bname", bname);

	this->::e2se_e2db::e2db::remove_userbouquet(bname);
}

void e2db::plain()
{
	debug("plain()");

	// empty services list - touch index["chs"]
	if (! index.count("chs"))
		index["chs"];

	e2db::bouquet bs;

	bs = e2db::bouquet();
	bs.bname = "bouquets.tv";
	bs.name = "User - bouquet (TV)";
	bs.btype = bs.index = 1;
	bs.nname = "TV";
	this->::e2se_e2db::e2db::add_bouquet(bs);

	bs = e2db::bouquet();
	bs.bname = "bouquets.radio";
	bs.name = "User - bouquet (Radio)";
	bs.btype = bs.index = 2;
	bs.nname = "Radio";
	this->::e2se_e2db::e2db::add_bouquet(bs);

	this->gindex = index;
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
		entries.transponders[txdata.first] = entryTransponder(txdata.second);
	}
	for (auto & chdata : db.services)
	{
		entries.services[chdata.first] = entryService(chdata.second);
	}
	return true;
}

bool e2db::write(string localdir, bool overwrite)
{
	debug("write()");

	this->gindex = index;

	if (! this->::e2se_e2db::e2db::write(localdir, overwrite))
		return false;

	return true;
}

//TODO FIX duplicate bouquets
void e2db::merge(unordered_map<string, e2se_e2db::e2db_file> files)
{
	debug("merge()");

	bool merge = this->get_input().size() != 0 ? true : false;
	e2db* nw_dbih = new e2db(this->log->log);
	nw_dbih->parse_e2db(files);
	this->::e2se_e2db::e2db::merge(nw_dbih);
	delete nw_dbih;

	if (merge)
	{
		entries.transponders.clear();
		entries.services.clear();
	}

	this->gindex = index;

	for (auto & txdata : db.transponders)
	{
		entries.transponders[txdata.first] = entryTransponder(txdata.second);
	}
	for (auto & chdata : db.services)
	{
		entries.services[chdata.first] = entryService(chdata.second);
	}
}

void e2db::importFile(vector<string> paths)
{
	debug("importFile()");

	bool merge = this->get_input().size() != 0 ? true : false;
	import_file(paths);

	if (merge)
	{
		entries.transponders.clear();
		entries.services.clear();
	}

	this->gindex = index;

	for (auto & txdata : db.transponders)
	{
		entries.transponders[txdata.first] = entryTransponder(txdata.second);
	}
	for (auto & chdata : db.services)
	{
		entries.services[chdata.first] = entryService(chdata.second);
	}
}

void e2db::exportFile(int flags, vector<string> paths)
{
	debug("exportFile()");

	if (flags != -1)
		export_file(FPORTS (flags), paths);
	else
		export_file(paths);
}

QStringList e2db::entryTransponder(transponder tx)
{
	QString freq = QString::fromStdString(to_string(tx.freq));
	QString pol = QString::fromStdString(tx.pol != -1 ? e2db::SAT_POL[tx.pol] : "");
	QString sr = QString::fromStdString(to_string(tx.sr));
	QString fec = QString::fromStdString(e2db::SAT_FEC[tx.fec]);
	string ppos;
	if (tx.ttype == 's')
	{
		if (tuners_pos.count(tx.pos))
		{
			string tnid = tuners_pos.at(tx.pos);
			e2db::tunersets_table tns = tuners[0].tables[tnid];
			ppos = tns.name;
		} else {
			char cposdeg[6];
			// %3d.%1d%C
			std::sprintf(cposdeg, "%.1f", float (std::abs (tx.pos)) / 10);
			ppos = (string (cposdeg) + (tx.pos > 0 ? 'E' : 'W'));
		}
	}
	QString pos = QString::fromStdString(ppos);
	string psys;
	switch (tx.ttype) {
		case 's':
			psys = tx.sys != -1 ? e2db::SAT_SYS[tx.sys] : "DVB-S";
		break;
		case 't':
			psys = "DVB-T";
		break;
		case 'c':
			psys = "DVB-C";
		break;
		case 'a':
			psys = "ATSC";
		break;
	}
	QString sys = QString::fromStdString(psys);

	return QStringList ({freq, pol, sr, fec, pos, sys});
}

QStringList e2db::entryService(service ch)
{
	// macos: unwanted chars [qt.qpa.fonts] Menlo notice
	QString chname;
	if (sets->value("preference/fixUnicodeChars").toBool())
		chname = QString::fromStdString(ch.chname).remove(QRegularExpression("[^\\p{L}\\p{M}\\p{N}\\p{P}\\p{S}\\s]+"));
	else
		chname = QString::fromStdString(ch.chname);
	QString chid = QString::fromStdString(ch.chid);
	QString txid = QString::fromStdString(ch.txid);
	QString stype = e2db::STYPES.count(ch.stype) ? QString::fromStdString(e2db::STYPES.at(ch.stype).second) : "Data";
	QString pname = QString::fromStdString(ch.data.count(e2db::SDATA::p) ? ch.data[e2db::SDATA::p][0] : "");

	QString scas;
	if (ch.data.count(e2db::SDATA::C))
	{
		unordered_set<string> _unique;
		QStringList cas;

		for (string & w : ch.data[e2db::SDATA::C])
		{
			string caidpx = w.substr(0, 2);
			if (e2db::SDATA_CAS.count(caidpx) && ! _unique.count(caidpx))
			{
				cas.append(QString::fromStdString(e2db::SDATA_CAS.at(caidpx)));
				_unique.insert(caidpx);
			}
		}
		scas.append(' ' + cas.join(", "));
	}

	QStringList entry = QStringList ({chname, chid, txid, stype, scas, pname});
	entry.append(entries.transponders[ch.txid]);
	return entry;
}

QStringList e2db::entryMarker(channel_reference chref)
{
	QString chid = QString::fromStdString(chref.chid);
	QString value = QString::fromStdString(chref.value);

	return QStringList({NULL, value, chid, NULL, "MARKER", NULL});
}

}
