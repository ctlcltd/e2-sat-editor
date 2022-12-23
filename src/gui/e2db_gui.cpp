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

//TODO options are constants
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
	entries.transponders[tx.txid] = entryTransponder(tx);
	return tx.txid;
}

string e2db::editTransponder(string txid, transponder& tx)
{
	debug("editTransponder()", "txid", txid);

	this->::e2se_e2db::e2db::edit_transponder(txid, tx);
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
	entries.services[ch.chid] = entryService(ch);
	return ch.chid;
}

string e2db::editService(string chid, service& ch)
{
	debug("editService()", "chid", chid);

	this->::e2se_e2db::e2db::edit_service(chid, ch);
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

string e2db::addChannelReference(channel_reference& chref, string bname)
{
	debug("addChannelReference()", "chid", chref.chid);

	this->::e2se_e2db::e2db::add_channel_reference(chref, bname);
	return chref.chid;
}

string e2db::editChannelReference(string chid, channel_reference& chref, string bname)
{
	debug("editChannelReference()", "chid", chid);

	this->::e2se_e2db::e2db::edit_channel_reference(chid, chref, bname);
	return chref.chid;
}

void e2db::removeChannelReference(channel_reference chref, string bname)
{
	debug("removeChannelReference()", "chid", chref.chid);

	this->::e2se_e2db::e2db::remove_channel_reference(chref, bname);
}

void e2db::removeChannelReference(string chid, string bname)
{
	debug("removeChannelReference()", "chid", chid);

	this->::e2se_e2db::e2db::remove_channel_reference(chid, bname);
}

int e2db::addTunersets(tunersets& tv)
{
	debug("addTunersets()");

	this->::e2se_e2db::e2db::add_tunersets(tv);
	return tv.ytype;
}

int e2db::editTunersets(int tvid, tunersets& tv)
{
	debug("editTunersets()", "tvid", tvid);

	this->::e2se_e2db::e2db::edit_tunersets(tvid, tv);
	return tv.ytype;
}

void e2db::removeTunersets(int tvid)
{
	debug("removeTunersets()", "tvid", tvid);

	this->::e2se_e2db::e2db::remove_tunersets(tvid);
}

string e2db::addTunersetsTable(tunersets_table& tn, tunersets tv)
{
	debug("addTunersetsTable()");

	this->::e2se_e2db::e2db::add_tunersets_table(tn, tv);
	return tn.tnid;
}

string e2db::editTunersetsTable(string tnid, tunersets_table& tn, tunersets tv)
{
	this->::e2se_e2db::e2db::edit_tunersets_table(tnid, tn, tv);
	return tn.tnid;
}

void e2db::removeTunersetsTable(string tnid, tunersets tv)
{
	debug("removeTunersetsTable()", "tnid", tnid);

	this->::e2se_e2db::e2db::remove_tunersets_table(tnid, tv);
}

string e2db::addTunersetsTransponder(tunersets_transponder& tntxp, tunersets_table tn)
{
	debug("addTunersetsTransponder()");

	this->::e2se_e2db::e2db::add_tunersets_transponder(tntxp, tn);
	return tntxp.trid;
}

string e2db::editTunersetsTransponder(string trid, tunersets_transponder& tntxp, tunersets_table tn)
{
	debug("editTunersetsTransponder()", "trid", trid);

	this->::e2se_e2db::e2db::edit_tunersets_transponder(trid, tntxp, tn);
	return tntxp.trid;
}

void e2db::removeTunersetsTransponder(string trid, tunersets_table tn)
{
	debug("removeTunersetsTransponder()", "trid", trid);

	this->::e2se_e2db::e2db::remove_tunersets_transponder(trid, tn);
}

void e2db::plain()
{
	debug("plain()");

	// empty services list - touch index["chs"]
	/*if (! index.count("chs"))
		index["chs"];*/

	e2db::bouquet bs;

	bs = e2db::bouquet();
	bs.bname = "bouquets.tv";
	bs.name = "User - bouquet (TV)";
	bs.btype = bs.index = e2db::STYPE::tv;
	bs.nname = "TV";
	this->::e2se_e2db::e2db::add_bouquet(bs);

	bs = e2db::bouquet();
	bs.bname = "bouquets.radio";
	bs.name = "User - bouquet (Radio)";
	bs.btype = bs.index = e2db::STYPE::radio;
	bs.nname = "Radio";
	this->::e2se_e2db::e2db::add_bouquet(bs);
}

bool e2db::prepare(string localdir)
{
	debug("prepare()");

	if (! this->read(localdir))
		return false;

	if (sets->value("application/parserDebugger", false).toBool())
		this->debugger();

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

	if (! this->::e2se_e2db::e2db::write(localdir, overwrite))
		return false;

	return true;
}

//TODO FIX duplicate bouquets
void e2db::merge(unordered_map<string, e2se_e2db::e2db_file> files)
{
	debug("merge()");

	bool merge = this->get_input().size() != 0 ? true : false;
	auto* dst = new e2db(this->log->log);
	dst->parse_e2db(files);
	this->::e2se_e2db::e2db::merge(dst);
	delete dst;

	if (merge)
	{
		entries.transponders.clear();
		entries.services.clear();
	}

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

	for (auto & txdata : db.transponders)
	{
		entries.transponders[txdata.first] = entryTransponder(txdata.second);
	}
	for (auto & chdata : db.services)
	{
		entries.services[chdata.first] = entryService(chdata.second);
	}
}

void e2db::exportFile(int bit, vector<string> paths)
{
	debug("exportFile()");

	if (bit != -1)
		export_file(FPORTS (bit), paths);
	else
		export_file(paths);
}

QStringList e2db::entryTransponder(transponder tx)
{
	QString freq = QString::fromStdString(to_string(tx.freq));
	QString pol = QString::fromStdString(tx.pol != -1 ? e2db::SAT_POL[tx.pol] : "");
	QString sr = QString::fromStdString(to_string(tx.sr));
	QString fec = QString::fromStdString(e2db::SAT_FEC[tx.fec]);
	string ppos = value_transponder_position(tx);
	QString pos = QString::fromStdString(ppos);
	string psys = value_transponder_system(tx);
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
	QString ssid = QString::fromStdString(to_string(ch.ssid));
	QString tsid = QString::fromStdString(to_string(ch.tsid));
	QString stype = QString::fromStdString(e2db::STYPE_EXT_LABEL.count(ch.stype) ? e2db::STYPE_EXT_LABEL.at(ch.stype) : e2db::STYPE_EXT_LABEL.at(e2db::STYPE::data));
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

	QStringList entry = QStringList ({chname, chid, txid, ssid, tsid, stype, scas, pname});
	entry.append(entries.transponders[ch.txid]);
	return entry;
}

QStringList e2db::entryMarker(channel_reference chref)
{
	QString chid = QString::fromStdString(chref.chid);
	QString value = QString::fromStdString(chref.value);

	return QStringList ({NULL, value, chid, NULL, NULL, NULL, "MARKER", NULL});
}

QStringList e2db::entryTunersetsTable(tunersets_table tn)
{
	QStringList entry;
	QString tnid = QString::fromStdString(tn.tnid);
	QString name = QString::fromStdString(tn.name);

	if (tn.ytype == e2db::YTYPE::sat)
	{
		string ppos = value_transponder_position(tn);
		QString pos = QString::fromStdString(ppos);
		entry = QStringList ({tnid, name, pos});
	}
	else if (tn.ytype == e2db::YTYPE::terrestrial || tn.ytype == e2db::YTYPE::cable)
	{
		QString country = QString::fromStdString(tn.country);
		entry = QStringList ({tnid, name, country});
	}
	else if (tn.ytype == e2db::YTYPE::atsc)
	{
		entry = QStringList ({tnid, name});
	}

	return entry;
}

QStringList e2db::entryTunersetsTransponder(tunersets_transponder tntxp, tunersets_table tn)
{
	QStringList entry;
	QString trid = QString::fromStdString(tntxp.trid);
	QString freq = QString::fromStdString(to_string(tntxp.freq));
	string ptxp = value_transponder_combo(tntxp, tn);
	QString combo = QString::fromStdString(ptxp);

	if (tn.ytype == YTYPE::sat)
	{
		QString pol = QString::fromStdString(e2db::SAT_POL[tntxp.pol]);
		QString sr = QString::fromStdString(to_string(tntxp.sr));
		QString fec = QString::fromStdString(e2db::SAT_FEC[tntxp.fec]);
		QString sys = QString::fromStdString(e2db::SAT_SYS[tntxp.sys]);
		QString mod = QString::fromStdString(e2db::SAT_MOD[tntxp.mod]);
		QString inv = QString::fromStdString(e2db::SAT_INV[tntxp.inv]);
		QString pil = QString::fromStdString(e2db::SAT_PIL[tntxp.pil]);
		QString rol = QString::fromStdString(e2db::SAT_ROL[tntxp.rol]);
		entry = QStringList ({trid, combo, freq, pol, sr, fec, sys, mod, inv, pil, rol});
	}
	else if (tn.ytype == YTYPE::terrestrial)
	{
		QString tmod = QString::fromStdString(e2db::TER_MOD[tntxp.tmod]);
		QString band = QString::fromStdString(e2db::TER_BAND[tntxp.band]);
		QString sys = "DVB-T";
		QString tmx = QString::fromStdString(e2db::TER_TRXMODE[tntxp.tmx]);
		QString hpfec = QString::fromStdString(e2db::TER_HPFEC[tntxp.hpfec]);
		QString lpfec = QString::fromStdString(e2db::TER_LPFEC[tntxp.lpfec]);
		QString inv = QString::fromStdString(e2db::TER_INV[tntxp.inv]);
		QString guard = QString::fromStdString(e2db::TER_GUARD[tntxp.guard]);
		QString hier = QString::fromStdString(e2db::TER_HIER[tntxp.hier]);
		entry = QStringList ({trid, combo, freq, tmod, band, sys, tmx, hpfec, lpfec, inv, guard, hier});
	}
	else if (tn.ytype == YTYPE::cable)
	{
		QString cmod = QString::fromStdString(e2db::CAB_MOD[tntxp.cmod]);
		QString sr = QString::fromStdString(to_string(tntxp.sr));
		QString cfec = QString::fromStdString(e2db::CAB_IFEC[tntxp.cfec]);
		QString inv = QString::fromStdString(e2db::CAB_INV[tntxp.inv]);
		QString sys = "DVB-C";
		entry = QStringList ({trid, combo, freq, cmod, sr, cfec, inv, sys});
	}
	else if (tn.ytype == YTYPE::atsc)
	{
		// combo = NULL;
		combo = ""; //Qt5
		QString amod = QString::fromStdString(to_string(tntxp.amod));
		QString sys = "ATSC";
		entry = QStringList ({trid, combo, freq, amod, sys});
	}

	return entry;
}

}
