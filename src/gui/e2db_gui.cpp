/*!
 * e2-sat-editor/src/gui/e2db_gui.cpp
 * 
 * @link https://github.com/ctlcltd/e2-sat-editor
 * @copyright e2 SAT Editor Team
 * @author Leonardo Laureti
 * @version 1.6.0
 * @license MIT License
 * @license GNU GPLv3 License
 */

#include <cstdio>
#include <ctime>
#include <clocale>
#include <cmath>
#include <string>
#include <unordered_set>
#include <stdexcept>

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
	primer();
}

e2db::~e2db()
{
	debug("~e2db");

	delete this->log;
}

void e2db::setup()
{
	debug("setup");

	QSettings settings;

	int datadb_type = settings.value("engine/dbTypeDefault", 0x1224).toInt();

	switch (datadb_type)
	{
		case 0x1224: e2db::LAMEDB_VER = 4; e2db::ZAPIT_VER = -1; db.type = 0; db.version = 0x1224; break;
		case 0x1225: e2db::LAMEDB_VER = 5; e2db::ZAPIT_VER = -1; db.type = 0; db.version = 0x1225; break;
		case 0x1223: e2db::LAMEDB_VER = 3; e2db::ZAPIT_VER = -1; db.type = 0; db.version = 0x1223; break;
		case 0x1222: e2db::LAMEDB_VER = 2; e2db::ZAPIT_VER = -1; db.type = 0; db.version = 0x1222; break;
		case 0x1014: e2db::LAMEDB_VER = -1; e2db::ZAPIT_VER = 4; db.type = 1; db.version = 0x1014; break;
		case 0x1013: e2db::LAMEDB_VER = -1; e2db::ZAPIT_VER = 3; db.type = 1; db.version = 0x1012; break;
		case 0x1012: e2db::LAMEDB_VER = -1; e2db::ZAPIT_VER = 2; db.type = 1; db.version = 0x1012; break;
		case 0x1011: e2db::LAMEDB_VER = -1; e2db::ZAPIT_VER = 1; db.type = 1; db.version = 0x1011; break;
	}

	e2db::OVERWRITE_FILE = true;
	e2db::PARSER_PRIOR_LAMEDB5 = settings.value("engine/parserPriorLamedb5", false).toBool();
	e2db::PARSER_TUNERSETS = settings.value("engine/parserTunerset", true).toBool();
	e2db::MAKER_TUNERSETS = settings.value("engine/makerTunerset", true).toBool();
	e2db::PARSER_PARENTALLOCK_LIST = settings.value("engine/parserParentalLock", true).toBool();
	e2db::MAKER_PARENTALLOCK_LIST = settings.value("engine/makerParentalLock", true).toBool();
	e2db::USERBOUQUET_FILENAME_SUFFIX = settings.value("engine/userbouquetFilenameSuffix", "dbe").toString().toStdString();

	e2db::MARKER_GLOBAL_INDEX = settings.value("engine/markerGlobalIndex", false).toBool();
	e2db::FAVOURITE_MATCH_SERVICE = settings.value("engine/favouriteMatchService", true).toBool();
	e2db::MERGE_SORT_ID = settings.value("engine/mergeSortId", false).toBool();

	int profile_sel = settings.value("profile/selected", -1).toInt();
	int profile_i = -1;

	int idx = 0;
	int size = settings.beginReadArray("profile");
	for (int i = 0; i < size; i++)
	{
		settings.setArrayIndex(i);
		idx = settings.group().section('/', 1).toInt();
		if (profile_sel == idx)
		{
			profile_i = 1;
			break;
		}
	}

	if (profile_i != -1)
	{
		settings.setArrayIndex(profile_i);
		e2db::MAKER_TPATH = settings.value("pathTransponders").toString().toStdString();
		e2db::MAKER_SPATH = settings.value("pathServices").toString().toStdString();
		e2db::MAKER_BPATH = settings.value("pathBouquets").toString().toStdString();
	}
	settings.endArray();

	e2db::CSV_HEADER = settings.value("engine/toolsCsvHeader", true).toBool();
	string csv_dlm = doubleToSingleEscaped(settings.value("engine/toolsCsvDelimiter", "\\n").toString()).toStdString();
	string csv_sep = doubleToSingleEscaped(settings.value("engine/toolsCsvSeparator", ",").toString()).toStdString();
	string csv_esp = doubleToSingleEscaped(settings.value("engine/toolsCsvEscape", "\"").toString()).toStdString();
	e2db::CSV_DELIMITER = csv_dlm.size() != 0 && csv_dlm.size() <= 2 ? csv_dlm : "\n";
	e2db::CSV_SEPARATOR = csv_sep.size() == 1 ? csv_sep[0] : ',';
	e2db::CSV_ESCAPE = csv_esp.size() == 1 ? csv_esp[0] : '"';
	e2db::CONVERTER_EXTENDED_FIELDS = settings.value("engine/toolsFieldsExtended", false).toBool();

	e2db::FIX_CRLF = settings.value("engine/fixCrlf", true).toBool();
	e2db::PARSER_FIX_CRLF = e2db::FIX_CRLF || settings.value("engine/parserFixCrlf", true).toBool();
	e2db::MAKER_FIX_CRLF = e2db::FIX_CRLF || settings.value("engine/makerFixCrlf", true).toBool();
	e2db::CONVERTER_IN_FIX_CRLF = e2db::FIX_CRLF || settings.value("engine/converterInFixCrlf", true).toBool();
	e2db::CONVERTER_OUT_CSV_FIX_CRLF = e2db::FIX_CRLF || settings.value("engine/converterOutCsvFixCrlf", false).toBool();
	e2db::CONVERTER_OUT_M3U_FIX_CRLF = e2db::FIX_CRLF || settings.value("engine/converterOutM3uFixCrlf", false).toBool();
	e2db::CONVERTER_OUT_HTML_FIX_CRLF = e2db::FIX_CRLF || settings.value("engine/converterOutHtmlFixCrlf", true).toBool();
}

void e2db::primer()
{
	debug("primer");

	index["chs"]; // touch index["chs"]
	index["txs"]; // touch index["txs"]

	// set parental lock type
	int parental_invert = QSettings().value("engine/parentalLockInvert", false).toInt();
	e2db::PARENTALLOCK ltype = static_cast<e2db::PARENTALLOCK>(parental_invert);
	db.parental = ltype;

	// create bouquets
	e2db::bouquet bs;

	bs = e2db::bouquet();
	bs.bname = "bouquets.tv";
	bs.name = "User - bouquet (TV)";
	bs.btype = e2db::STYPE::tv;
	bs.nname = "TV";
	this->::e2se_e2db::e2db::add_bouquet(bs);
	index["bouquets.tv"]; // touch index["bouquets.tv"]

	bs = e2db::bouquet();
	bs.bname = "bouquets.radio";
	bs.name = "User - bouquet (Radio)";
	bs.btype = e2db::STYPE::radio;
	bs.nname = "Radio";
	this->::e2se_e2db::e2db::add_bouquet(bs);
	index["bouquets.radio"]; // touch index["bouquets.radio"]
}

void e2db::didChange()
{
	debug("didChange");

	setup();
}

void e2db::initStorage()
{
	debug("initStorage");

	for (auto & tx : db.transponders)
	{
		entries.transponders[tx.first] = entryTransponder(tx.second);
	}
	for (auto & ch : db.services)
	{
		entries.services[ch.first] = entryService(ch.second);
	}
}

void e2db::clearStorage()
{
	debug("clearStorage");

	entries.transponders.clear();
	entries.services.clear();

	for (auto & tx : db.transponders)
	{
		entries.transponders[tx.first] = entryTransponder(tx.second);
	}
	for (auto & ch : db.services)
	{
		entries.services[ch.first] = entryService(ch.second);
	}
}

void e2db::clearStorage(bool merge)
{
	debug("clearStorage", "merge", merge);

	if (merge)
	{
		entries.transponders.clear();
		entries.services.clear();
	}

	for (auto & tx : db.transponders)
	{
		entries.transponders[tx.first] = entryTransponder(tx.second);
	}
	for (auto & ch : db.services)
	{
		entries.services[ch.first] = entryService(ch.second);
	}
}

string e2db::addTransponder(transponder& tx)
{
	// debug("addTransponder", "txid", tx.txid);

	this->::e2se_e2db::e2db::add_transponder(tx);
	entries.transponders[tx.txid] = entryTransponder(tx);
	return tx.txid;
}

string e2db::editTransponder(string txid, transponder& tx)
{
	// debug("editTransponder", "txid", txid);

	this->::e2se_e2db::e2db::edit_transponder(txid, tx);
	entries.transponders[tx.txid] = entryTransponder(tx);
	return tx.txid;
}

void e2db::removeTransponder(string txid)
{
	// debug("removeTransponder", "txid", txid);

	this->::e2se_e2db::e2db::remove_transponder(txid);
	entries.transponders.erase(txid);
}

string e2db::addService(service& ch)
{
	// debug("addService", "chid", ch.chid);

	this->::e2se_e2db::e2db::add_service(ch);
	entries.services[ch.chid] = entryService(ch);
	return ch.chid;
}

string e2db::editService(string chid, service& ch)
{
	// debug("editService", "chid", chid);

	this->::e2se_e2db::e2db::edit_service(chid, ch);
	entries.services[ch.chid] = entryService(ch);
	return ch.chid;
}

void e2db::removeService(string chid)
{
	// debug("removeService", "chid", chid);

	this->::e2se_e2db::e2db::remove_service(chid);
	entries.services.erase(chid);
}

string e2db::addBouquet(bouquet& bs)
{
	// debug("addBouquet");

	this->::e2se_e2db::e2db::add_bouquet(bs);
	index[bs.bname]; // touch index[bs.bname]
	return bs.bname;
}

string e2db::editBouquet(bouquet& bs)
{
	// debug("editBouquet");

	this->::e2se_e2db::e2db::edit_bouquet(bs);
	return bs.bname;
}

void e2db::removeBouquet(string bname)
{
	// debug("removeBouquet", "bname", bname);

	this->::e2se_e2db::e2db::remove_bouquet(bname);
}

string e2db::addUserbouquet(userbouquet& ub)
{
	// debug("addUserbouquet");

	this->::e2se_e2db::e2db::add_userbouquet(ub);
	index[ub.bname]; // touch index[ub.bname]
	return ub.bname;
}

string e2db::editUserbouquet(userbouquet& ub)
{
	// debug("editUserbouquet");

	this->::e2se_e2db::e2db::edit_userbouquet(ub);
	return ub.bname;
}

void e2db::removeUserbouquet(string bname)
{
	// debug("removeUserbouquet", "bname", bname);

	this->::e2se_e2db::e2db::remove_userbouquet(bname);
}

string e2db::addChannelReference(channel_reference& chref, string bname)
{
	// debug("addChannelReference", "chid", chref.chid);

	this->::e2se_e2db::e2db::add_channel_reference(chref, bname);
	return chref.chid;
}

string e2db::editChannelReference(string chid, channel_reference& chref, string bname)
{
	// debug("editChannelReference", "chid", chid);

	this->::e2se_e2db::e2db::edit_channel_reference(chid, chref, bname);
	return chref.chid;
}

void e2db::removeChannelReference(channel_reference chref, string bname)
{
	// debug("removeChannelReference", "chid", chref.chid);

	this->::e2se_e2db::e2db::remove_channel_reference(chref, bname);
}

void e2db::removeChannelReference(string chid, string bname)
{
	// debug("removeChannelReference", "chid", chid);

	this->::e2se_e2db::e2db::remove_channel_reference(chid, bname);
}

int e2db::addTunersets(tunersets& tv)
{
	// debug("addTunersets");

	this->::e2se_e2db::e2db::add_tunersets(tv);
	return tv.ytype;
}

int e2db::editTunersets(int tvid, tunersets& tv)
{
	// debug("editTunersets", "tvid", tvid);

	this->::e2se_e2db::e2db::edit_tunersets(tvid, tv);
	return tv.ytype;
}

void e2db::removeTunersets(int tvid)
{
	// debug("removeTunersets", "tvid", tvid);

	this->::e2se_e2db::e2db::remove_tunersets(tvid);
}

string e2db::addTunersetsTable(tunersets_table& tn, tunersets tv)
{
	// debug("addTunersetsTable");

	this->::e2se_e2db::e2db::add_tunersets_table(tn, tv);
	return tn.tnid;
}

string e2db::editTunersetsTable(string tnid, tunersets_table& tn, tunersets tv)
{
	// debug("editTunersetsTable", "tnid", tnid);

	this->::e2se_e2db::e2db::edit_tunersets_table(tnid, tn, tv);
	return tn.tnid;
}

void e2db::removeTunersetsTable(string tnid, tunersets tv)
{
	// debug("removeTunersetsTable", "tnid", tnid);

	this->::e2se_e2db::e2db::remove_tunersets_table(tnid, tv);
}

string e2db::addTunersetsTransponder(tunersets_transponder& tntxp, tunersets_table tn)
{
	// debug("addTunersetsTransponder");

	this->::e2se_e2db::e2db::add_tunersets_transponder(tntxp, tn);
	return tntxp.trid;
}

string e2db::editTunersetsTransponder(string trid, tunersets_transponder& tntxp, tunersets_table tn)
{
	// debug("editTunersetsTransponder", "trid", trid);

	this->::e2se_e2db::e2db::edit_tunersets_transponder(trid, tntxp, tn);
	return tntxp.trid;
}

void e2db::removeTunersetsTransponder(string trid, tunersets_table tn)
{
	// debug("removeTunersetsTransponder", "trid", trid);

	this->::e2se_e2db::e2db::remove_tunersets_transponder(trid, tn);
}

void e2db::setServiceParentalLock(string chid)
{
	// debug("setServiceParentalLock", "chid", chid);

	this->::e2se_e2db::e2db::set_service_parentallock(chid);
	entries.services[chid][1] = " ";
}

void e2db::unsetServiceParentalLock(string chid)
{
	// debug("unsetServiceParentalLock", "chid", chid);

	this->::e2se_e2db::e2db::unset_service_parentallock(chid);
	entries.services[chid][1] = "";
}

void e2db::setUserbouquetParentalLock(string bname)
{
	// debug("setUserbouquetParentalLock", "bname", bname);

	this->::e2se_e2db::e2db::set_userbouquet_parentallock(bname);
}

void e2db::unsetUserbouquetParentalLock(string bname)
{
	// debug("unsetUserbouquetParentalLock", "bname", bname);

	this->::e2se_e2db::e2db::unset_userbouquet_parentallock(bname);
}

//TODO fix bouquets
void e2db::removeBouquets()
{
	// debug("removeBouquets");

	this->::e2se_e2db::e2db_utils::remove_bouquets();
	this->primer();
}

void e2db::removeUserbouquets()
{
	// debug("removeUserbouquets");

	this->::e2se_e2db::e2db_utils::remove_userbouquets();
}

bool e2db::prepare(string filename) noexcept
{
	debug("prepare");

	try
	{
		if (! this->read(filename))
			return false;
	}
	catch (...)
	{
		return false;
	}

	initStorage();

	return true;
}

bool e2db::write(string path) noexcept
{
	debug("write");

	try
	{
		return this->::e2se_e2db::e2db::write(path);
	}
	catch (...)
	{
		return false;
	}
}

//TODO FIX merge import and input empty map
void e2db::importFile(vector<string> paths) noexcept
{
	debug("importFile");

	bool merge = this->get_input().size() != 0 ? true : false;

	try
	{
		import_file(paths);
	}
	catch (...)
	{
		return;
	}

	clearStorage(merge);
}

void e2db::exportFile(int bit, vector<string> paths, string filename) noexcept
{
	debug("exportFile");

	if (bit != -1)
	{
		if (paths.size() == 1)
			export_file(FPORTS (bit), paths[0], filename);
		else
			export_file(FPORTS (bit), paths);
	}
	else
	{
		export_file(paths);
	}
}

void e2db::importBlob(unordered_map<string, e2db_file> files)
{
	debug("importBlob");

	bool merge = this->get_input().size() != 0 ? true : false;

	try
	{
		import_blob(files);
	}
	catch (...)
	{
		// note: forward exceptions
		throw;
	}

	clearStorage(merge);
}

bool e2db::haveErrors()
{
	return e2se::logger::OBJECT->trace.size() ? true : false;
}

vector<string> e2db::getErrors()
{
	return e2se::logger::OBJECT->trace;
}

void e2db::clearErrors()
{
	e2se::logger::OBJECT->trace.clear();
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

	if (QApplication::layoutDirection() == Qt::RightToLeft)
	{
		tname.append(QChar(0x200e)); // LRM
	}

	return QStringList ({sys, pos, tname, freq, pol, sr, fec});
}

QStringList e2db::entryTransponder(transponder tx, bool extended)
{
	QStringList entry;

	if (entries.transponders.count(tx.txid))
		entry = entries.transponders[tx.txid];
	else
		entry = entryTransponder(tx);

	if (! extended)
		return entry;

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
		QString mod = QString::fromStdString(value_transponder_modulation(tx.amod, YTYPE::atsc));
		entry.append({mod});
	}

	if (QApplication::layoutDirection() == Qt::RightToLeft)
	{
		combo.prepend(QChar(0x200e)); // LRM
	}

	entry.prepend(combo);
	entry.prepend(txid);

	return entry;
}

QStringList e2db::entryService(service ch)
{
	QString chname = fixUnicodeChars(ch.chname);
	QString chid = QString::fromStdString(ch.chid);
	QString parental = ch.parental ? " " : "";
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

	if (QApplication::layoutDirection() == Qt::RightToLeft)
	{
		chname.append(QChar(0x200e)); // LRM
		pname.append(QChar(0x200e)); // LRM
	}

	QStringList entry = QStringList ({chname, parental, chid, txid, ssid, tsid, stype, scas, pname});

	if (entries.transponders.count(ch.txid))
		entry.append(entries.transponders[ch.txid]);

	return entry;
}

QStringList e2db::entryFavourite(channel_reference chref)
{
	QString chid = QString::fromStdString(chref.chid);
	QString value = QString::fromStdString(chref.value);
	QString ssid = QString::number(chref.ref.ssid);
	QString tsid = QString::number(chref.ref.tsid);
	QString chtype;
	QString sys;
	QString uri = QString::fromStdString(chref.uri);

	switch (chref.etype)
	{
		case e2db::ETYPE::ecast: sys = "[broadcast]"; break;
		case e2db::ETYPE::efile: sys = "[file]"; break;
		case e2db::ETYPE::evod: sys = "[vod]"; break;
		case e2db::ETYPE::eraw: sys = "[raw]"; break;
		case e2db::ETYPE::egstplayer: sys = "[gstplayer]"; break;
		case e2db::ETYPE::eexteplayer3: sys = "[exteplayer3]"; break;
		case e2db::ETYPE::eservice: sys = "[eservice]"; break;
		case e2db::ETYPE::eyoutube: sys = "[youtube]"; break;
		case e2db::ETYPE::eservice2: sys = "[eservice dreamos]"; break;
		default: sys = e2db::ETYPE_EXT_LABEL.count(chref.etype) ? QString::fromStdString(e2db::ETYPE_EXT_LABEL.at(chref.etype)) : QString::number(chref.etype);
	}

	if (chref.stream)
		chtype = "STREAM";
	else if (chref.marker)
		chtype = "MARKER";
	else if (entries.services.count(chref.chid))
		chtype = entries.services[chref.chid][6];
	else
		chtype = "ERROR";

	if (FAVOURITE_MATCH_SERVICE && chref.stream)
	{
		string ref_txid;
		string ref_chid;
		int dvbns = chref.ref.dvbns;

		for (auto & x : db.transponders)
		{
			transponder& tx = x.second;

			//TODO TEST (dvbns or onid reverse to pos)
			if (tx.tsid == chref.ref.tsid && (tx.dvbns == chref.ref.dvbns || tx.onid == chref.ref.onid))
			{
				ref_txid = tx.txid;
				dvbns = tx.dvbns;
				break;
			}
		}

		if (dvbns == chref.ref.dvbns)
		{
			ref_chid = chref.chid;
		}
		else
		{
			char chid[25];

			// %4x:%4x:%8x
			std::snprintf(chid, 25, "%x:%x:%x", chref.ref.ssid, chref.ref.tsid, dvbns);

			ref_chid = chid;
		}

		if (entries.services.count(ref_chid))
		{
			QStringList entry = QStringList (entries.services[ref_chid]);
			entry[0] = value;
			entry[1] = ""; //Qt5
			entry[2] = chid;
			entry[4] = ssid;
			entry[5] = tsid;
			entry[6] = "STREAM";
			entry[9] = sys;
			entry[11] = uri;
			return entry;
		}
		else if (entries.transponders.count(ref_txid))
		{
			QString txid = QString::fromStdString(ref_txid);

			QStringList entry = QStringList ({value, NULL, chid, txid, ssid, tsid, "STREAM", NULL, NULL});
			entry.append(entries.transponders[ref_txid]);
			entry[9] = sys;
			entry[11] = uri;
			return entry;
		}
	}

	return QStringList ({value, NULL, chid, NULL, ssid, tsid, chtype, NULL, NULL, sys, NULL, uri});
}

QStringList e2db::entryMarker(channel_reference chref)
{
	QString chid = QString::fromStdString(chref.chid);
	QString value = QString::fromStdString(chref.value);
	QString sys;

	switch (chref.atype)
	{
		case e2db::ATYPE::marker_hidden_512: sys = "[hidden]"; break;
		case e2db::ATYPE::marker_hidden_832: sys = "[hidden]"; break;
		case e2db::ATYPE::marker_numbered: sys = "[numbered]"; break;
	}

	return QStringList ({NULL, value, NULL, chid, NULL, NULL, NULL, "MARKER", NULL, NULL, sys});
}

QStringList e2db::entryTunersetsTable(tunersets_table tn)
{
	QStringList entry;
	QString tnid = QString::fromStdString(tn.tnid);
	QString name = QString::fromStdString(tn.name);

	if (QApplication::layoutDirection() == Qt::RightToLeft)
	{
		name.append(QChar(0x200e)); // LRM
	}

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

	if (QApplication::layoutDirection() == Qt::RightToLeft)
	{
		combo.prepend(QChar(0x200e)); // LRM
	}

	if (tn.ytype == YTYPE::satellite)
	{
		QString pol = QString::fromStdString(value_transponder_polarization(tntxp.pol));
		QString sr = QString::fromStdString(value_transponder_sr(tntxp.sr));
		QString sys = QString::fromStdString(value_transponder_system(tntxp.sys, YTYPE::satellite));
		QString fec = QString::fromStdString(value_transponder_fec(tntxp.fec, YTYPE::satellite));
		QString mod = QString::fromStdString(value_transponder_modulation(tntxp.mod, YTYPE::satellite));
		QString inv = QString::fromStdString(value_transponder_inversion(tntxp.inv, YTYPE::satellite));
		QString rol = QString::fromStdString(value_transponder_rollof(tntxp.rol));
		QString pil = QString::fromStdString(value_transponder_pilot(tntxp.pil));
		entry = QStringList ({trid, combo, freq, pol, sr, sys, fec, mod, inv, rol, pil});
	}
	else if (tn.ytype == YTYPE::terrestrial)
	{
		QString tmod = QString::fromStdString(value_transponder_modulation(tntxp.tmod, YTYPE::terrestrial));
		QString sys = QString::fromStdString(value_transponder_system(tntxp.sys, YTYPE::terrestrial));
		QString band = QString::fromStdString(value_transponder_bandwidth(tntxp.band));
		QString tmx = QString::fromStdString(value_transponder_tmx_mode(tntxp.tmx));
		QString hpfec = QString::fromStdString(value_transponder_fec(tntxp.hpfec, YTYPE::terrestrial));
		QString lpfec = QString::fromStdString(value_transponder_fec(tntxp.lpfec, YTYPE::terrestrial));
		QString inv = QString::fromStdString(value_transponder_inversion(tntxp.inv, YTYPE::terrestrial));
		QString guard = QString::fromStdString(value_transponder_guard(tntxp.guard));
		QString hier = QString::fromStdString(value_transponder_hier(tntxp.hier));
		entry = QStringList ({trid, combo, freq, tmod, NULL, sys, band, tmx, hpfec, lpfec, inv, guard, hier});
	}
	else if (tn.ytype == YTYPE::cable)
	{
		QString cmod = QString::fromStdString(value_transponder_modulation(tntxp.cmod, YTYPE::cable));
		QString sr = QString::fromStdString(value_transponder_sr(tntxp.sr));
		QString sys = QString::fromStdString(value_transponder_system(tntxp.sys, YTYPE::cable));
		QString cfec = QString::fromStdString(value_transponder_fec(tntxp.cfec, YTYPE::cable));
		QString inv = QString::fromStdString(value_transponder_inversion(tntxp.inv, YTYPE::cable));
		entry = QStringList ({trid, combo, freq, cmod, sr, sys, cfec, inv});
	}
	else if (tn.ytype == YTYPE::atsc)
	{
		QString amod = QString::fromStdString(value_transponder_modulation(tntxp.amod, YTYPE::atsc));
		QString sys = QString::fromStdString(value_transponder_system(tntxp.sys, YTYPE::atsc));
		entry = QStringList ({trid, combo, freq, amod, NULL, sys});
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

QString e2db::doubleToSingleEscaped(QString text)
{
	return text.replace("\\n", "\n").replace("\\r", "\r").replace("\\t", "\t").replace("\\s", " ");
}

string e2db::msg(string str, string param)
{
	string trstr = tr(str.data()).toStdString();
	size_t tsize = trstr.size() + param.size();
	char tstr[tsize];
	std::snprintf(tstr, tsize, trstr.c_str(), param.c_str());

	return string (tstr);
}

void e2db::error(string fn, string optk, string optv)
{
	this->::e2se_e2db::e2db::error(fn, tr(optk.data(), "error").toStdString(), tr(optv.data(), "error").toStdString());

	trace(tr(optk.data(), "error").toStdString() + '\t' + tr(optv.data(), "error").toStdString() + '\t' + fn);
}

void e2db::trace(string error)
{
	e2se::logger::OBJECT->trace.emplace_back(error);
}

}
