/*!
 * e2-sat-editor/src/gui/e2db_gui.cpp
 * 
 * @link https://github.com/ctlcltd/e2-sat-editor
 * @copyright e2 SAT Editor Team
 * @author Leonardo Laureti
 * @version 0.5
 * @license MIT License
 * @license GNU GPLv3 License
 */

#include <cstdio>
#include <ctime>
#include <clocale>
#include <cmath>
#include <string>
#include <unordered_set>

#include <QRegularExpression>
#include <QSettings>
#include <QMessageBox>

#include "e2db_gui.h"

using std::string, std::unordered_set, std::to_string;

namespace e2se_gui
{

e2db::e2db()
{
	std::setlocale(LC_NUMERIC, "C");

	this->log = new e2se::logger("gui", "e2db");

	setup();
	createBouquets();
}

void e2db::setup()
{
	debug("setup");

	QSettings settings;

	e2db::OVERWRITE_FILE = true;
	e2db::PARSER_LAMEDB5_PRIOR = settings.value("application/parserLamedb5", false).toBool();
	e2db::MAKER_LAMEDB5 = settings.value("application/makerLamedb5", true).toBool();
	e2db::PARSER_TUNERSETS = settings.value("application/parserTunerset", true).toBool();
	e2db::MAKER_TUNERSETS = settings.value("application/makerTunerset", true).toBool();
	e2db::PARSER_PARENTALLOCK_LIST = settings.value("application/parserParentalLock", true).toBool();
	e2db::MAKER_PARENTALLOCK_LIST = settings.value("application/makerParentalLock", true).toBool();

	int profile_sel = settings.value("profile/selected").toInt();
	settings.beginReadArray("profile");
	settings.setArrayIndex(profile_sel);
	e2db::MAKER_TPATH = settings.value("pathTransponders").toString().toStdString();
	e2db::MAKER_SPATH = settings.value("pathServices").toString().toStdString();
	e2db::MAKER_BPATH = settings.value("pathBouquets").toString().toStdString();
	settings.endArray();

	e2db::CSV_HEADER = settings.value("preference/toolsCsvHeader", true).toBool();
	string csv_dlm = settings.value("preference/toolsCsvDelimiter", "\n").toString().toStdString();
	string csv_sep = settings.value("preference/toolsCsvSeparator", ",").toString().toStdString();
	string csv_esp = settings.value("preference/toolsCsvEscape", "\"").toString().toStdString();
	e2db::CSV_DELIMITER = csv_dlm.find("\n") != string::npos ? '\n' : '\n';
	e2db::CSV_SEPARATOR = csv_sep[0];
	e2db::CSV_ESCAPE = csv_esp[0];
	e2db::CONVERTER_EXTENDED_FIELDS = settings.value("preference/toolsFieldsExtended", false).toBool();
}

//TODO FIX
void e2db::error(string msg, string optk, string optv)
{
	this->::e2se_e2db::e2db::error(msg, optk, optv);
	QMessageBox::critical(nullptr, QString::fromStdString(optk), QString::fromStdString(optv));
}

void e2db::didChange()
{
	debug("didChange");

	setup();
}

void e2db::createBouquets()
{
	debug("createBouquets");

	// empty services list - touch index["chs"]
	if (! index.count("chs"))
		index["chs"];

	e2db::bouquet bs;

	bs = e2db::bouquet();
	bs.bname = "bouquets.tv";
	bs.name = "User - bouquet (TV)";
	bs.btype = e2db::STYPE::tv;
	bs.nname = "TV";
	this->::e2se_e2db::e2db::add_bouquet(bs);

	bs = e2db::bouquet();
	bs.bname = "bouquets.radio";
	bs.name = "User - bouquet (Radio)";
	bs.btype = e2db::STYPE::radio;
	bs.nname = "Radio";
	this->::e2se_e2db::e2db::add_bouquet(bs);
}

void e2db::fixBouquets()
{
	debug("fixBouquets");

	if (index["bss"].size() > 2)
	{
		vector<string> del;

		for (auto & x : bouquets)
		{
			bouquet bs = x.second;

			if (bs.userbouquets.size() == 0)
			{
				string bname = bs.bname;

				for (auto it = index["bss"].begin(); it != index["bss"].end(); it++)
				{
					if (it->second == bname)
						index["bss"].erase(it);
				}
			}
		}
		for (string & w : del)
		{
			bouquets.erase(w);
		}
		del.clear();
	}
}

void e2db::cache(bool clear)
{
	debug("cache");

	if (clear)
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

string e2db::addTransponder(transponder& tx)
{
	debug("addTransponder", "txid", tx.txid);

	this->::e2se_e2db::e2db::add_transponder(tx);
	entries.transponders[tx.txid] = entryTransponder(tx);
	return tx.txid;
}

string e2db::editTransponder(string txid, transponder& tx)
{
	debug("editTransponder", "txid", txid);

	this->::e2se_e2db::e2db::edit_transponder(txid, tx);
	entries.transponders[tx.txid] = entryTransponder(tx);
	return tx.txid;
}

void e2db::removeTransponder(string txid)
{
	debug("removTransponder", "txid", txid);

	this->::e2se_e2db::e2db::remove_transponder(txid);
	entries.transponders.erase(txid);
}

string e2db::addService(service& ch)
{
	debug("addService", "chid", ch.chid);

	this->::e2se_e2db::e2db::add_service(ch);
	entries.services[ch.chid] = entryService(ch);
	return ch.chid;
}

string e2db::editService(string chid, service& ch)
{
	debug("editService", "chid", chid);

	this->::e2se_e2db::e2db::edit_service(chid, ch);
	entries.services[ch.chid] = entryService(ch);
	return ch.chid;
}

void e2db::removeService(string chid)
{
	debug("removeService", "chid", chid);

	this->::e2se_e2db::e2db::remove_service(chid);
	entries.services.erase(chid);
}

string e2db::addUserbouquet(userbouquet& ub)
{
	debug("addUserbouquet");

	this->::e2se_e2db::e2db::add_userbouquet(ub);
	return ub.bname;
}

string e2db::editUserbouquet(userbouquet& ub)
{
	debug("editUserbouquet");

	this->::e2se_e2db::e2db::edit_userbouquet(ub);
	return ub.bname;
}

void e2db::removeUserbouquet(string bname)
{
	debug("removeUserbouquet", "bname", bname);

	this->::e2se_e2db::e2db::remove_userbouquet(bname);
}

string e2db::addChannelReference(channel_reference& chref, string bname)
{
	debug("addChannelReference", "chid", chref.chid);

	this->::e2se_e2db::e2db::add_channel_reference(chref, bname);
	return chref.chid;
}

string e2db::editChannelReference(string chid, channel_reference& chref, string bname)
{
	debug("editChannelReference", "chid", chid);

	this->::e2se_e2db::e2db::edit_channel_reference(chid, chref, bname);
	return chref.chid;
}

void e2db::removeChannelReference(channel_reference chref, string bname)
{
	debug("removeChannelReference", "chid", chref.chid);

	this->::e2se_e2db::e2db::remove_channel_reference(chref, bname);
}

void e2db::removeChannelReference(string chid, string bname)
{
	debug("removeChannelReference", "chid", chid);

	this->::e2se_e2db::e2db::remove_channel_reference(chid, bname);
}

int e2db::addTunersets(tunersets& tv)
{
	debug("addTunersets");

	this->::e2se_e2db::e2db::add_tunersets(tv);
	return tv.ytype;
}

int e2db::editTunersets(int tvid, tunersets& tv)
{
	debug("editTunersets", "tvid", tvid);

	this->::e2se_e2db::e2db::edit_tunersets(tvid, tv);
	return tv.ytype;
}

void e2db::removeTunersets(int tvid)
{
	debug("removeTunersets", "tvid", tvid);

	this->::e2se_e2db::e2db::remove_tunersets(tvid);
}

string e2db::addTunersetsTable(tunersets_table& tn, tunersets tv)
{
	debug("addTunersetsTable");

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
	debug("removeTunersetsTable", "tnid", tnid);

	this->::e2se_e2db::e2db::remove_tunersets_table(tnid, tv);
}

string e2db::addTunersetsTransponder(tunersets_transponder& tntxp, tunersets_table tn)
{
	debug("addTunersetsTransponder");

	this->::e2se_e2db::e2db::add_tunersets_transponder(tntxp, tn);
	return tntxp.trid;
}

string e2db::editTunersetsTransponder(string trid, tunersets_transponder& tntxp, tunersets_table tn)
{
	debug("editTunersetsTransponder", "trid", trid);

	this->::e2se_e2db::e2db::edit_tunersets_transponder(trid, tntxp, tn);
	return tntxp.trid;
}

void e2db::removeTunersetsTransponder(string trid, tunersets_table tn)
{
	debug("removeTunersetsTransponder", "trid", trid);

	this->::e2se_e2db::e2db::remove_tunersets_transponder(trid, tn);
}

void e2db::setServiceParentalLock(string chid)
{
	debug("setServiceParentalLock", "chid", chid);

	this->::e2se_e2db::e2db::set_service_parentallock(chid);
	entries.services[chid][1] = " ";
}

void e2db::unsetServiceParentalLock(string chid)
{
	debug("unsetServiceParentalLock", "chid", chid);

	this->::e2se_e2db::e2db::unset_service_parentallock(chid);
	entries.services[chid][1] = "";
}

void e2db::setUserbouquetParentalLock(string bname)
{
	debug("setUserbouquetParentalLock", "bname", bname);

	this->::e2se_e2db::e2db::set_userbouquet_parentallock(bname);
}

void e2db::unsetUserbouquetParentalLock(string bname)
{
	debug("unsetUserbouquetParentalLock", "bname", bname);

	this->::e2se_e2db::e2db::unset_userbouquet_parentallock(bname);
}

bool e2db::prepare(string filename)
{
	debug("prepare");

	if (! this->read(filename))
		return false;

	cache();
	fixBouquets();

	return true;
}

bool e2db::write(string path)
{
	debug("write");

	if (! this->::e2se_e2db::e2db::write(path))
		return false;

	return true;
}

void e2db::importFile(vector<string> paths)
{
	debug("importFile");

	bool merge = this->get_input().size() != 0 ? true : false;
	import_file(paths);

	cache(merge);
	fixBouquets();
}

void e2db::exportFile(int bit, vector<string> paths)
{
	debug("exportFile");

	if (bit != -1)
		export_file(FPORTS (bit), paths);
	else
		export_file(paths);
}

void e2db::importBlob(unordered_map<string, e2db_file> files)
{
	debug("importBlob");

	bool merge = this->get_input().size() != 0 ? true : false;

	if (merge)
	{
		auto* dst = newptr();
		dst->parse_e2db(files);
		this->::e2se_e2db::e2db::merge(dst);
		delete dst;
	}
	else
	{
		this->parse_e2db(files);
	}

	cache(merge);
	fixBouquets();
}

QStringList e2db::entryTransponder(transponder tx)
{
	QString sys = QString::fromStdString(value_transponder_system(tx));
	QString pos = QString::fromStdString(value_transponder_position(tx));
	QString tname = QString::fromStdString(get_tuner_name(tx));
	QString freq = QString::number(tx.freq);
	QString pol = QString::fromStdString(value_transponder_polarization(tx.pol));
	QString sr = QString::fromStdString(value_transponder_sr(tx.sr));
	QString fec;
	switch (tx.ytype)
	{
		case e2db::YTYPE::satellite:
		case e2db::YTYPE::cable:
			fec = QString::fromStdString(value_transponder_fec(tx.fec, tx.ytype));
		break;
		case e2db::YTYPE::terrestrial:
			fec.append(QString::fromStdString(value_transponder_fec(tx.hpfec, tx.ytype)));
			fec.append(" | ");
			fec.append(QString::fromStdString(value_transponder_fec(tx.lpfec, tx.ytype)));
		break;
	}

	return QStringList ({sys, pos, tname, freq, pol, sr, fec});
}

QStringList e2db::entryTransponder(transponder tx, bool extended)
{
	QStringList entry = entries.transponders[tx.txid];

	QString txid = QString::fromStdString(tx.txid);
	QString combo = QString::fromStdString(value_transponder_combo(tx));
	QString tsid = QString::number(tx.tsid);
	QString dvbns = QString::fromStdString(value_transponder_dvbns(tx.dvbns));
	QString onid = QString::number(tx.onid);
	entry.insert(2, tsid);
	entry.insert(3, dvbns);
	entry.insert(4, onid);
	if (tx.ytype == YTYPE::satellite)
	{
		QString mod = QString::fromStdString(value_transponder_modulation(tx.mod, YTYPE::satellite));
		QString inv = QString::fromStdString(value_transponder_inversion(tx.inv, YTYPE::satellite));
		QString rol = QString::fromStdString(value_transponder_rollof(tx.rol));
		QString pil = QString::fromStdString(value_transponder_pilot(tx.pil));
		entry.append({mod, NULL, rol, pil, inv});
	}
	else if (tx.ytype == YTYPE::terrestrial)
	{
		QString mod = QString::fromStdString(value_transponder_modulation(tx.tmod, YTYPE::terrestrial));
		QString band = QString::fromStdString(value_transponder_bandwidth(tx.band));
		QString tmx = QString::fromStdString(value_transponder_tmx_mode(tx.tmx));
		QString inv = QString::fromStdString(value_transponder_inversion(tx.inv, YTYPE::terrestrial));
		QString guard = QString::fromStdString(value_transponder_guard(tx.guard));
		QString hier = QString::fromStdString(value_transponder_hier(tx.hier));
		entry.append({mod, band, NULL, NULL, inv, tmx, guard, hier});
	}
	else if (tx.ytype == YTYPE::cable)
	{
		QString mod = QString::fromStdString(value_transponder_modulation(tx.cmod, YTYPE::cable));
		QString inv = QString::fromStdString(value_transponder_inversion(tx.inv, YTYPE::cable));
		entry.append({mod, NULL, NULL, NULL, inv});
	}
	else if (tx.ytype == YTYPE::atsc)
	{
		QString mod = QString::number(tx.amod);
		entry.append({mod});
	}
	entry.prepend(combo);
	entry.prepend(txid);

	return entry;
}

QStringList e2db::entryService(service ch)
{
	QString chname = fixUnicodeChars(ch.chname);
	QString chid = QString::fromStdString(ch.chid);
	QString locked = ch.locked ? " " : "";
	QString txid = QString::fromStdString(ch.txid);
	QString ssid = QString::number(ch.ssid);
	QString tsid = QString::number(ch.tsid);
	QString stype = QString::fromStdString(value_service_type(ch.stype));
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
	QString pname = QString::fromStdString(value_channel_provider(ch));

	QStringList entry = QStringList ({chname, locked, chid, txid, ssid, tsid, stype, scas, pname});
	entry.append(entries.transponders[ch.txid]);
	return entry;
}

QStringList e2db::entryMarker(channel_reference chref)
{
	QString chid = QString::fromStdString(chref.chid);
	QString value = QString::fromStdString(chref.value);

	return QStringList ({NULL, value, NULL, chid, NULL, NULL, NULL, "MARKER", NULL});
}

QStringList e2db::entryTunersetsTable(tunersets_table tn)
{
	QStringList entry;
	QString tnid = QString::fromStdString(tn.tnid);
	QString name = QString::fromStdString(tn.name);

	if (tn.ytype == e2db::YTYPE::satellite)
	{
		QString pos = QString::fromStdString(value_transponder_position(tn));
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
	QString freq = QString::number(tntxp.freq);
	QString combo = QString::fromStdString(value_transponder_combo(tntxp, tn));

	if (tn.ytype == YTYPE::satellite)
	{
		QString pol = QString::fromStdString(value_transponder_polarization(tntxp.pol));
		QString sr = QString::fromStdString(value_transponder_sr(tntxp.sr));
		QString fec = QString::fromStdString(value_transponder_fec(tntxp.fec, YTYPE::satellite));
		QString sys = QString::fromStdString(value_transponder_system(tntxp.sys, YTYPE::satellite));
		QString mod = QString::fromStdString(value_transponder_modulation(tntxp.mod, YTYPE::satellite));
		QString inv = QString::fromStdString(value_transponder_inversion(tntxp.inv, YTYPE::satellite));
		QString rol = QString::fromStdString(value_transponder_rollof(tntxp.rol));
		QString pil = QString::fromStdString(value_transponder_pilot(tntxp.pil));
		entry = QStringList ({trid, combo, freq, pol, sr, fec, sys, mod, inv, rol, pil});
	}
	else if (tn.ytype == YTYPE::terrestrial)
	{
		QString tmod = QString::fromStdString(value_transponder_modulation(tntxp.tmod, YTYPE::terrestrial));
		QString band = QString::fromStdString(value_transponder_bandwidth(tntxp.band));
		QString sys = QString::fromStdString(value_transponder_system(tntxp.sys, YTYPE::terrestrial));
		QString tmx = QString::fromStdString(value_transponder_tmx_mode(tntxp.tmx));
		QString hpfec = QString::fromStdString(value_transponder_fec(tntxp.hpfec, YTYPE::terrestrial));
		QString lpfec = QString::fromStdString(value_transponder_fec(tntxp.lpfec, YTYPE::terrestrial));
		QString inv = QString::fromStdString(value_transponder_inversion(tntxp.inv, YTYPE::terrestrial));
		QString guard = QString::fromStdString(value_transponder_guard(tntxp.guard));
		QString hier = QString::fromStdString(value_transponder_hier(tntxp.hier));
		entry = QStringList ({trid, combo, freq, tmod, band, sys, tmx, hpfec, lpfec, inv, guard, hier});
	}
	else if (tn.ytype == YTYPE::cable)
	{
		QString cmod = QString::fromStdString(value_transponder_modulation(tntxp.cmod, YTYPE::cable));
		QString sr = QString::fromStdString(value_transponder_sr(tntxp.sr));
		QString cfec = QString::fromStdString(value_transponder_fec(tntxp.cfec, YTYPE::cable));
		QString inv = QString::fromStdString(value_transponder_inversion(tntxp.inv, YTYPE::cable));
		QString sys = QString::fromStdString(value_transponder_system(tntxp.sys, YTYPE::cable));
		entry = QStringList ({trid, combo, freq, cmod, sr, cfec, inv, sys});
	}
	else if (tn.ytype == YTYPE::atsc)
	{
		// combo = NULL;
		combo = ""; //Qt5
		QString amod = QString::number(tntxp.amod);
		QString sys = QString::fromStdString(value_transponder_system(tntxp.sys, YTYPE::atsc));
		entry = QStringList ({trid, combo, freq, amod, sys});
	}

	return entry;
}

// macos: unwanted chars [qt.qpa.fonts] Menlo notice
QString e2db::fixUnicodeChars(string str)
{
	if (QSettings().value("application/fixUnicodeChars").toBool())
		return QString::fromStdString(str).remove(QRegularExpression("[^\\p{L}\\p{M}\\p{N}\\p{P}\\p{S}\\s]+"));
	else
		return QString::fromStdString(str);
}

}
