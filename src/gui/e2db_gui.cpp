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

#include "e2db_gui.h"

using namespace std;

namespace e2se_gui
{
e2db::e2db()
{
	e2se::debug("e2db");

	options();
}

void e2db::options()
{
	e2se::debug("e2db", "options()");

	e2db::DEBUG = e2se::DEBUG;
	e2db::PARSER_TUNERSETS = e2se::PARSER_TUNERSETS;
	e2db::PARSER_LAMEDB5_PRIOR = e2se::PARSER_LAMEDB5;
	e2db::MAKER_LAMEDB5 = e2se::MAKER_LAMEDB5;
}

bool e2db::prepare(string localdir)
{
	if (! this->read(localdir))
		return false;

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
			sprintf(cposdeg, "%.1f", float(tx.pos / 10));
			ppos = (string (cposdeg) + (tx.pos ? 'E' : 'W'));
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
	QString sys = QString::fromStdString(psys);

	return QStringList ({freq, pol, sr, fec, pos, sys});
}

QStringList e2db::entry_service(service ch)
{
	// macos: unwanted chars [qt.qpa.fonts] Menlo notice
	QString chname;
	if (1)
		chname = QString::fromStdString(ch.chname).remove(QRegularExpression("[^\\p{L}\\p{N}\\p{Pc}\\p{M}\\p{P}\\s]+"));
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
