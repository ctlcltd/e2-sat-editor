/*!
 * e2-sat-editor/src/gui/printable.cpp
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
#include <unordered_set>

#include <QPrinter>
#include <QTextDocument>
#include <QPrintDialog>

#include "printable.h"

using std::to_string, std::unordered_set;
using namespace e2se;

namespace e2se_gui
{

printable::printable(e2db* dbih, e2se::logger::session* log)
{
	this->log = new logger(log, "printable");
	debug("printable()");

	this->dbih = dbih;
	this->sets = new QSettings;
}

void printable::document_all()
{
	debug("document_all()");

	document_index();
	document_lamedb();
	for (auto & x : dbih->index["bss"])
	{
		string bname = x.second;
		e2db::bouquet gboq = dbih->bouquets[bname];
		document_bouquet(bname);

		for (string & bname : gboq.userbouquets)
		{
			document_userbouquet(bname);
		}
	}
	for (auto & x : dbih->tuners)
	{
		document_tunersets(x.first);
	}
}

void printable::document_index()
{
	debug("document_index()");

	string filename = std::filesystem::path(dbih->get_localdir()).filename().u8string(); //C++17
	if (filename.empty())
	{
		filename = "Untitled";
	}

	page p;
	page_header(p, filename, view::index);
	page_footer(p, filename, view::index);
	
	vector<string> paths;
	for (auto & x : dbih->get_input())
	{
		paths.emplace_back(x.first);
	}
	page_body_index_list(p, paths);

	pages.emplace_back(p);
}

void printable::document_lamedb()
{
	document_lamedb(-1);
}

void printable::document_lamedb(int stype)
{
	debug("document_lamedb()");
	
	string filename = std::filesystem::path(dbih->get_filename()).filename().u8string(); //C++17
	string iname;
	string xname;
	string headname = filename;
	string footname = filename;
	switch (stype)
	{
		// Data
		case 0:
			iname = "chs:0";
			xname = "Data";
		break;
		// TV
		case 1:
			iname = "chs:1";
			xname = "TV";
		break;
		// Radio
		case 2:
			iname = "chs:2";
			xname = "Radio";
		break;
		// All Services
		default:
			iname = "chs";
	}
	if (! xname.empty())
	{
		headname += " <i>" + xname + "</i>";
		footname += " (extract)";
	}

	page p;
	page_header(p, headname, view::services);
	page_footer(p, footname, view::services);

	page_body_channel_list(p, iname, view::services);

	pages.emplace_back(p);
}

void printable::document_bouquet(string bname)
{
	debug("document_bouquet()", "bname", bname);

	page p;
	page_header(p, bname, view::bouquets);
	page_footer(p, bname, view::bouquets);

	page_body_bouquet_list(p, bname);

	pages.emplace_back(p);
}

void printable::document_userbouquet(string bname)
{
	debug("document_userbouquet()", "bname", bname);

	page p;
	page_header(p, bname, view::userbouquets);
	page_footer(p, bname, view::userbouquets);

	page_body_channel_list(p, bname, view::userbouquets);

	pages.emplace_back(p);
}

void printable::document_tunersets(int ytype)
{
	debug("document_bouquet()", "ytype", ytype);

	string filename;
	switch (ytype)
	{
		case e2db::YTYPE::sat:
			filename = "satellites.xml";
		break;
		case e2db::YTYPE::terrestrial:
			filename = "terrestrial.xml";
		break;
		case e2db::YTYPE::cable:
			filename = "cables.xml";
		break;
		case e2db::YTYPE::atsc:
			filename = "atsc.xml";
		break;
	}

	page p;
	page_header(p, filename, view::tunersets);
	page_footer(p, filename, view::tunersets);

	page_body_tunersets_list(p, ytype);

	pages.emplace_back(p);
}

void printable::page_header(page& p, string filename, view v)
{
	QString name;

	p.header += "<div class=\"header\">";
	if (v == view::index)
	{
		name = "Settings";
	}
	else if (v == view::tunersets)
	{
		name = "Tuner Settings";
	}
	else if (v == view::bouquets)
	{
		if (dbih->bouquets.count(filename))
		{
			e2db::bouquet gboq = dbih->bouquets[filename];
			name = QString::fromStdString(gboq.name);
		}
	}
	else if (v == view::userbouquets)
	{
		if (dbih->userbouquets.count(filename))
		{
			e2db::userbouquet uboq = dbih->userbouquets[filename];
			name = QString::fromStdString(uboq.name);
		}
	}
	else
	{
		name = "Service List";
	}
	p.header += "<h1>" + name + "</h1>";
	p.header += "<h3>" + QString::fromStdString(filename) + "</h3>";
	p.header += "</div>";
}

void printable::page_footer(page& p, string filename, view v)
{
	p.footer += "<div class=\"footer\">";
	p.footer += "File: <b>" + QString::fromStdString(filename) + "</b><br>";
	p.footer += "Editor: <b>" + QString::fromStdString(dbih->get_editor_string()).toHtmlEscaped() + "</b><br>";
	p.footer += "Datetime: <b>" + QString::fromStdString(dbih->get_timestamp()) + "</b>";
	p.footer += "</div>";
}

void printable::page_body_index_list(page& p, vector<string> paths)
{
	debug("page_body_index_list()");

	p.body += "<div class=\"toc\">";
	p.body += "<h4>Table of Contents</h4>";
	p.body += "<table>";
	p.body += "<thead>";
	p.body += "<tr>";
	p.body += "<th>Content</th>";
	p.body += "<th>Type</th>";
	p.body += "</tr>";

	p.body += "<tbody>";
	for (auto & path : paths)
	{
		string filename = std::filesystem::path(path).filename().u8string(); //C++17
		QString fname = QString::fromStdString(filename);
		QString ftype;
		e2db::FPORTS fpi = dbih->filetype_detect(path);
		switch (fpi)
		{
			case e2db::FPORTS::singleTunersets:
				ftype = "tunersets";
			break;
			case e2db::FPORTS::singleBouquet:
				ftype = "bouquet";
			break;
			case e2db::FPORTS::singleUserbouquet:
				ftype = "userbouquet";
			break;
			default:
				ftype = "services";
		}

		p.body += "<tr>";
		p.body += "<td class=\"trid\">" + fname + "</td>";
		p.body += "<td>" + ftype + "</td>";
		p.body += "</tr>";
	}
	p.body += "</tbody>";

	p.body += "</table>";
	p.body += "</div>";
}

void printable::page_body_channel_list(page& p, string bname, view v)
{
	if (dbih->index.count(bname))
		debug("page_body_channel_list()", "bname", bname);
	else
		error("page_body_channel_list()", "bname", bname);
	debug("page_body_channel_list()", "view", v);
	
	QString cssname = v == view::bouquets ? "userbouquet" : "services";
	
	p.body += "<div class=\"" + cssname + "\">";
	p.body += "<table>";
	p.body += "<thead>";
	p.body += "<tr>";
	p.body += "<th>Index</th>";
	p.body += "<th>Name</th>";
	// p.body += "<th>Reference ID</th>";
	p.body += "<th>SSID</th>";
	p.body += "<th>TSID</th>";
	p.body += "<th>Type</th>";
	p.body += "<th>CAS</th>";
	p.body += "<th>Provider</th>";
	p.body += "<th>Freq</th>";
	p.body += "<th>Pol</th>";
	p.body += "<th>SR</th>";
	p.body += "<th>FEC</th>";
	p.body += "<th>Pos</th>";
	p.body += "<th>Sys</th>";
	p.body += "</tr>";
	p.body += "</thead>";

	p.body += "<tbody>";
	for (auto & chdata : dbih->index[bname])
	{
		string chid = chdata.second;

		if (dbih->db.services.count(chdata.second))
		{
			e2db::service ch = dbih->db.services[chid];
			e2db::transponder tx = dbih->db.transponders[ch.txid];

			QString idx = QString::fromStdString(to_string(chdata.first));
			QString chname = QString::fromStdString(ch.chname);
			QString refid;
			// bouquets
			if (v)
			{
				e2db::channel_reference chref;
				if (dbih->userbouquets.count(bname))
					chref = dbih->userbouquets[bname].channels[chid];
				string crefid = dbih->get_reference_id(chref);
				refid = QString::fromStdString(crefid);
			}
			// services
			else
			{
				string crefid = dbih->get_reference_id(chid);
				refid = QString::fromStdString(crefid);
			}
			QString ssid = QString::fromStdString(to_string(ch.ssid));
			QString tsid = QString::fromStdString(to_string(ch.tsid));
			QString stype = QString::fromStdString(e2db::STYPE_EXT_LABEL.count(ch.stype) ? e2db::STYPE_EXT_LABEL.at(ch.stype) : e2db::STYPE_EXT_LABEL.at(e2db::STYPE::data));
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
				scas.append("<b>$</b>");
				scas.append(" ");
				scas.append("<span class=\"cas\">");
				scas.append(cas.join(", "));
				scas.append("</span>");
			}
			QString pname = QString::fromStdString(ch.data.count(e2db::SDATA::p) ? ch.data[e2db::SDATA::p][0] : "");
			QString freq = QString::fromStdString(to_string(tx.freq));
			QString pol = QString::fromStdString(tx.pol != -1 ? e2db::SAT_POL[tx.pol] : "");
			QString sr = QString::fromStdString(to_string(tx.sr));
			QString fec = QString::fromStdString(e2db::SAT_FEC[tx.fec]);
			string ppos = dbih->get_transponder_position_text(tx);
			QString pos = QString::fromStdString(ppos);
			string psys = dbih->get_transponder_system_text(tx);
			QString sys = QString::fromStdString(psys);

			p.body += "<tr>";
			p.body += "<td class=\"trid\">" + idx + "</td>";
			p.body += "<td class=\"chname\">" + chname + "</td>";
			// p.body += "<td class=\"refid\"><span >" + refid + "</span></td>";
			p.body += "<td>" + ssid + "</td>";
			p.body += "<td>" + tsid + "</td>";
			p.body += "<td>" + stype + "</td>";
			p.body += "<td class=\"scas\">" + scas + "</span></td>";
			p.body += "<td class=\"pname\">" + pname + "</td>";
			p.body += "<td>" + freq + "</td>";
			p.body += "<td>" + pol + "</td>";
			p.body += "<td>" + sr + "</td>";
			p.body += "<td>" + fec + "</td>";
			p.body += "<td>" + pos + "</td>";
			p.body += "<td>" + sys + "</td>";
			p.body += "</tr>";
		}
		else
		{
			e2db::channel_reference chref;
			if (dbih->userbouquets.count(bname))
				chref = dbih->userbouquets[bname].channels[chid];

			if (! chref.marker)
				continue;

			QString qchid = QString::fromStdString(chid);
			QString value = QString::fromStdString(chref.value);
			//TODO marker index and compatibility
			QString refid = "1:" + qchid.toUpper() + ":0:0:0:0:0:0";
			QString atype = "MARKER";

			p.body += "<tr class=\"marker\">";
			p.body += "<td class=\"trid\"></td>";
			p.body += "<td class=\"name\">" + value + "</td>";
			// p.body += "<td class=\"refid\">" + refid + "</td>";
			p.body += "<td></td>";
			p.body += "<td></td>";
			p.body += "<td class=\"atype\">" + atype + "</td>";
			p.body += "<td></td>";
			p.body += "<td></td>";
			p.body += "<td></td>";
			p.body += "<td></td>";
			p.body += "<td></td>";
			p.body += "<td></td>";
			p.body += "<td></td>";
			p.body += "<td></td>";
			p.body += "</tr>";
		}
	}
	p.body += "</tbody>";

	p.body += "</table>";
	p.body += "</div>";
}

void printable::page_body_bouquet_list(page& p, string bname)
{
	if (dbih->bouquets.count(bname))
		debug("page_body_bouquet_list()", "bname", bname);
	else
		error("page_body_bouquet_list()", "bname", bname);

	e2db::bouquet gboq = dbih->bouquets[bname];

	p.body += "<div class=\"bouquet\">";
	p.body += "<table>";
	p.body += "<thead>";
	p.body += "<tr>";
	p.body += "<th>Index</th>";
	p.body += "<th>Userbouquet</th>";
	p.body += "<th>Name</th>";
	p.body += "<th>Type</th>";
	p.body += "</tr>";

	p.body += "<tbody>";
	int i = 1;
	for (auto & bname : gboq.userbouquets)
	{
		e2db::userbouquet uboq = dbih->userbouquets[bname];
		QString btype;
		if (gboq.btype == 1)
			btype = "TV";
		else if (gboq.btype == 2)
			btype = "Radio";
		p.body += "<tr>";
		p.body += "<td class=\"trid\">" + QString::fromStdString(to_string(i++)) + "</td>";
		p.body += "<td>" + QString::fromStdString(bname) + "</td>";
		p.body += "<td>" + QString::fromStdString(uboq.name) + "</td>";
		p.body += "<td>" + btype + "</td>";
		p.body += "</tr>";
	}
	p.body += "</tbody>";

	p.body += "</table>";
	p.body += "</div>";
}

//TODO improve list
void printable::page_body_tunersets_list(page& p, int ytype)
{
	debug("page_body_tunersets_list()", "ytype", ytype);

	p.body += "<div class=\"tunersets\">";

	e2db::tunersets tvoq = dbih->tuners[ytype];
	string iname = "tns:";
	switch (ytype)
	{
		case e2db::YTYPE::sat:
			iname += 's';
		break;
		case e2db::YTYPE::terrestrial:
			iname += 't';
		break;
		case e2db::YTYPE::cable:
			iname += 'c';
		break;
		case e2db::YTYPE::atsc:
			iname += 'a';
		break;
	}
	
	for (auto & x : dbih->index[iname])
	{
		string tnid = x.second;
		e2db::tunersets_table tns = tvoq.tables[tnid];
		QString tnname = QString::fromStdString(tns.name);
		string ppos;
		if (ytype == e2db::YTYPE::sat)
		{
			ppos = dbih->get_transponder_position_text(tns);
		}
		QString pos = QString::fromStdString(ppos);

		p.body += "<div class=\"transponder\">";
		p.body += "<h4>Transponders</h4>";
		p.body += "<h2>" + tnname + "</h2>";
		if (ytype == e2db::YTYPE::sat)
			p.body += "<p>Position: <b>" + pos + "</b></p>";
		p.body += "<table>";
		p.body += "<thead>";
		p.body += "<tr>";
		p.body += "<th>Index</th>";
		if (ytype == e2db::YTYPE::sat)
		{
			p.body += "<th>Freq</th>";
			p.body += "<th>Pol</th>";
			p.body += "<th>SR</th>";
			p.body += "<th>FEC</th>";
			p.body += "<th>Sys</th>";
			p.body += "<th>Mod</th>";
			p.body += "<th>Inv</th>";
			p.body += "<th>Pilot</th>";
			p.body += "<th>Rollof</th>";
		}
		else if (ytype == e2db::YTYPE::terrestrial)
		{
			p.body += "<th>Freq</th>";
			p.body += "<th>Const</th>";
			p.body += "<th>Band</th>";
			p.body += "<th>Sys</th>";
			p.body += "<th>Tx Mode</th>";
			p.body += "<th>HP FEC</th>";
			p.body += "<th>LP FEC</th>";
			p.body += "<th>Inv</th>";
			p.body += "<th>Guard</th>";
			p.body += "<th>Hier</th>";
		}
		else if (ytype == e2db::YTYPE::cable)
		{
			p.body += "<th>Freq</th>";
			p.body += "<th>Mod</th>";
			p.body += "<th>SR</th>";
			p.body += "<th>FEC</th>";
			p.body += "<th>Inv</th>";
			p.body += "<th>Sys</th>";
		}
		else if (ytype == e2db::YTYPE::atsc)
		{
			p.body += "<th>Freq</th>";
			p.body += "<th>Mod</th>";
			p.body += "<th>Sys</th>";
		}
		p.body += "</tr>";

		int i = 1;
		p.body += "<tbody>";
		for (auto & x : dbih->index[tns.tnid])
		{
			string trid = x.second;
			e2db::tunersets_transponder txp = tns.transponders[trid];

			p.body += "<tr>";
			p.body += "<td class=\"trid\">" + QString::fromStdString(to_string(i++)) + "</td>";
			if (ytype == e2db::YTYPE::sat)
			{
				QString freq = QString::fromStdString(to_string(txp.freq));
				QString pol = QString::fromStdString(e2db::SAT_POL[txp.pol]);
				QString sr = QString::fromStdString(to_string(txp.sr));
				QString fec = QString::fromStdString(e2db::SAT_FEC[txp.fec]);
				QString sys = QString::fromStdString(e2db::SAT_SYS[txp.sys]);
				QString mod = QString::fromStdString(e2db::SAT_MOD[txp.mod]);
				QString inv = QString::fromStdString(e2db::SAT_INV[txp.inv]);
				QString pil = QString::fromStdString(e2db::SAT_PIL[txp.pil]);
				QString rol = QString::fromStdString(e2db::SAT_ROL[txp.rol]);

				p.body += "<td>" + freq + "</td>";
				p.body += "<td>" + pol + "</td>";
				p.body += "<td>" + sr + "</td>";
				p.body += "<td>" + fec + "</td>";
				p.body += "<td>" + sys + "</td>";
				p.body += "<td>" + mod + "</td>";
				p.body += "<td>" + inv + "</td>";
				p.body += "<td>" + pil + "</td>";
				p.body += "<td>" + rol + "</td>";
			}
			else if (ytype == e2db::YTYPE::terrestrial)
			{
				QString freq = QString::fromStdString(to_string(txp.freq));
				QString tmod = QString::fromStdString(e2db::TER_MOD[txp.tmod]);
				QString band = QString::fromStdString(e2db::TER_BAND[txp.band]);
				QString sys = "DVB-T";
				QString tmx = QString::fromStdString(e2db::TER_TRXMODE[txp.tmx]);
				QString hpfec = QString::fromStdString(e2db::TER_HPFEC[txp.hpfec]);
				QString lpfec = QString::fromStdString(e2db::TER_LPFEC[txp.lpfec]);
				QString inv = QString::fromStdString(e2db::TER_INV[txp.inv]);
				QString guard = QString::fromStdString(e2db::TER_GUARD[txp.guard]);
				QString hier = QString::fromStdString(e2db::TER_HIER[txp.hier]);

				p.body += "<td>" + freq + "</td>";
				p.body += "<td>" + tmod + "</td>";
				p.body += "<td>" + band + "</td>";
				p.body += "<td>" + sys + "</td>";
				p.body += "<td>" + tmx + "</td>";
				p.body += "<td>" + hpfec + "</td>";
				p.body += "<td>" + lpfec + "</td>";
				p.body += "<td>" + inv + "</td>";
				p.body += "<td>" + guard + "</td>";
				p.body += "<td>" + hier + "</td>";
			}
			else if (ytype == e2db::YTYPE::cable)
			{
				QString freq = QString::fromStdString(to_string(txp.freq));
				QString cmod = QString::fromStdString(e2db::CAB_MOD[txp.cmod]);
				QString sr = QString::fromStdString(to_string(txp.sr));
				QString cfec = QString::fromStdString(e2db::CAB_IFEC[txp.cfec]);
				QString inv = QString::fromStdString(e2db::CAB_INV[txp.inv]);
				QString sys = "DVB-C";

				p.body += "<td>" + freq + "</td>";
				p.body += "<td>" + cmod + "</td>";
				p.body += "<td>" + sr + "</td>";
				p.body += "<td>" + cfec + "</td>";
				p.body += "<td>" + inv + "</td>";
				p.body += "<td>" + sys + "</td>";
			}
			else if (ytype == e2db::YTYPE::atsc)
			{
				QString freq = QString::fromStdString(to_string(txp.freq));
				QString amod = QString::fromStdString(to_string(txp.amod));
				QString sys = "ATSC";

				p.body += "<td>" + freq + "</td>";
				p.body += "<td>" + amod + "</td>";
				p.body += "<td>" + sys + "</td>";
			}
			p.body += "</tr>";
		}
		p.body += "</tbody>";

		p.body += "</table>";
		p.body += "</div>";
	}

	p.body += "</div>";
}

void printable::print()
{
	debug("print()");

	QPrinter* printer = new QPrinter;
	QTextDocument* doc = new QTextDocument;

	printer->setPageSize(QPageSize::A4);
	printer->setPageOrientation(QPageLayout::Landscape);

	QStringList html;
	html.append(doc_head());
	html.append("<body>");

	for (auto & p : pages)
	{
		html.append(p.header);
		html.append(p.body);
		html.append(p.footer);
	}

	html.append("</body>");
	html.append(doc_foot());

	doc->setHtml(html.join(""));

	//TODO move to gui | tab
	QPrintDialog pdial = QPrintDialog(printer, nullptr);
	if (pdial.exec() == QDialog::Accepted) {
		doc->print(printer);
	}
}

QString printable::doc_head()
{
	return "<html lang=\"en\">\
<head>\
<meta charset=\"utf-8\">\
<style type=\"text/css\">\
body { font-size: 10px; font-weight: normal }\
h1 { font-size: 18px }\
h2 { font-size: 14px }\
h3 { font-size: 12px }\
h4 { font-size: 10px; text-transform: uppercase }\
div { margin: 3em 0 5em }\
div.footer { margin: 3em 1em; line-height: 1.5 }\
table { margin: 2em 0; border-collapse: collapse }\
td, th { padding: .4em .5em }\
table, td, th { border: 1px solid }\
th, td.chname, td.name, td.pname, td.refid { white-space: nowrap }\
tr.marker td, td.refid { font-weight: bold }\
tr.marker td, span.cas { font-size: 9px }\
td.trid { padding: .4em .8em .4em 1.4em }\
td.refid { font-size: 8px }\
span.cas { margin: 0 .3em 0 0 } \
</style>\
</head>";
}

QString printable::doc_foot()
{
	return "</html>";
}

}
