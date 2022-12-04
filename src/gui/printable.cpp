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

	string filename = dbih->get_localdir();

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
	debug("document_lamedb()");
	
	string filename = dbih->get_filename();

	page p;
	page_header(p, "lamedb", view::services);
	page_footer(p, "lamedb", view::services);

	page_body_channel_list(p, "chs", view::services);

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
	p.footer += "Editor: <b>" + QString::fromStdString(dbih->get_editor_string()) + "</b> &lt;https://github.com/ctlcltd/e2-sat-editor&gt;<br>";
	p.footer += "Datetime: <b>" + QString::fromStdString(dbih->get_timestamp()) + "</b>";
	p.footer += "</div>";
}

void printable::page_body_index_list(page& p, vector<string> paths)
{
	debug("page_body_index_list()");

	p.body += "<div class=\"toc\">";
	p.body += "<h4>Table of content</h4>";
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
	p.body += "<th>Type</th>";
	p.body += "<th>CAS</th>";
	p.body += "<th>Provider</th>";
	p.body += "<th>Frequency</th>";
	p.body += "<th>Polarization</th>";
	p.body += "<th>Symbol Rate</th>";
	p.body += "<th>FEC</th>";
	p.body += "<th>SAT</th>";
	p.body += "<th>System</th>";
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
			QString stype = e2db::STYPES.count(ch.stype) ? QString::fromStdString(e2db::STYPES.at(ch.stype).second) : "Data";
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
			string ppos;
			if (tx.ttype == 's')
			{
				if (dbih->tuners_pos.count(tx.pos))
				{
					string tnid = dbih->tuners_pos.at(tx.pos);
					e2db::tunersets_table tns = dbih->tuners[0].tables[tnid];
					ppos = tns.name;
				}
				else
				{
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

			p.body += "<tr>";
			p.body += "<td class=\"trid\">" + idx + "</td>";
			p.body += "<td class=\"chname\">" + chname + "</td>";
			// p.body += "<td class=\"refid\"><span >" + refid + "</span></td>";
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
			char cposdeg[6];
			// %3d.%1d%C
			std::sprintf(cposdeg, "%.1f", float (std::abs (tns.pos)) / 10);
			ppos = (string (cposdeg) + (tns.pos > 0 ? 'E' : 'W'));
		}
		QString pos = QString::fromStdString(ppos);

		p.body += "<div class=\"transponder\">";
		p.body += "<h4>Transponders</h4>";
		p.body += "<h2>" + tnname + "</h2>";
		p.body += "<p>Position: <b>" + pos + "</b></p>";
		p.body += "<table>";
		p.body += "<thead>";
		p.body += "<tr>";
		p.body += "<th>Index</th>";
		if (ytype == e2db::YTYPE::sat)
		{
			p.body += "<th>Frequency</th>";
			p.body += "<th>Polarization</th>";
			p.body += "<th>Symbol Rate</th>";
			p.body += "<th>FEC</th>";
			p.body += "<th>System</th>";
		}
		else if (ytype == e2db::YTYPE::terrestrial)
		{
			p.body += "<th>Frequency</th>";
			p.body += "<th>Constellation</th>";
			p.body += "<th>Bandwidth</th>";
			p.body += "<th>System</th>";
		}
		else if (ytype == e2db::YTYPE::cable)
		{
			p.body += "<th>Frequency</th>";
			p.body += "<th>Modulation</th>";
			p.body += "<th>Symbol Rate</th>";
			p.body += "<th>FEC</th>";
			p.body += "<th>System</th>";
		}
		else if (ytype == e2db::YTYPE::atsc)
		{
			p.body += "<th>Frequency</th>";
			p.body += "<th>Modulation</th>";
			p.body += "<th>System</th>";
		}
		p.body += "</tr>";

		int i = 1;
		p.body += "<tbody>";
		for (auto & x : dbih->index[tns.tnid])
		{
			string trid = x.second;
			e2db::tunersets_transponder tntxp = tns.transponders[trid];

			p.body += "<tr>";
			p.body += "<td class=\"trid\">" + QString::fromStdString(to_string(i++)) + "</td>";
			if (ytype == e2db::YTYPE::sat)
			{
				QString freq = QString::fromStdString(tntxp.freq != -1 ? to_string(tntxp.freq) : "");
				QString pol = QString::fromStdString(tntxp.pol != -1 ? e2db::SAT_POL[tntxp.pol] : "");
				QString sr = QString::fromStdString(tntxp.freq != -1 ? to_string(tntxp.sr) : "");
				QString fec = QString::fromStdString(tntxp.fec != -1 ? e2db::SAT_FEC[tntxp.fec] : "");
				QString sys = QString::fromStdString(tntxp.sys != -1 ? e2db::SAT_SYS[tntxp.sys] : "DVB-S");

				p.body += "<td>" + freq + "</td>";
				p.body += "<td>" + pol + "</td>";
				p.body += "<td>" + sr + "</td>";
				p.body += "<td>" + fec + "</td>";
				p.body += "<td>" + sys + "</td>";
			}
			else if (ytype == e2db::YTYPE::terrestrial)
			{
				QString freq = QString::fromStdString(tntxp.freq != -1 ? to_string(tntxp.freq) : "");
				QString tmod = QString::fromStdString(tntxp.tmod != -1 ? e2db::TER_MOD[tntxp.tmod] : "");
				QString band = QString::fromStdString(tntxp.band != -1 ? e2db::TER_BAND[tntxp.band] : "");
				QString sys = "DVB-T";

				p.body += "<td>" + freq + "</td>";
				p.body += "<td>" + tmod + "</td>";
				p.body += "<td>" + band + "</td>";
				p.body += "<td>" + sys + "</td>";
			}
			else if (ytype == e2db::YTYPE::cable)
			{
				QString freq = QString::fromStdString(tntxp.freq != -1 ? to_string(tntxp.freq) : "");
				QString cmod = QString::fromStdString(tntxp.cmod != -1 ? e2db::CAB_MOD[tntxp.cmod] : "");
				QString sr = QString::fromStdString(tntxp.freq != -1 ? to_string(tntxp.sr) : "");
				QString cfec = QString::fromStdString(tntxp.cfec != -1 ? e2db::CAB_IFEC[tntxp.cfec] : "");
				QString sys = "DVB-C";

				p.body += "<td>" + freq + "</td>";
				p.body += "<td>" + cmod + "</td>";
				p.body += "<td>" + sr + "</td>";
				p.body += "<td>" + cfec + "</td>";
				p.body += "<td>" + sys + "</td>";
			}
			else if (ytype == e2db::YTYPE::atsc)
			{
				QString freq = QString::fromStdString(tntxp.freq != -1 ? to_string(tntxp.freq * 1e3) : "");
				QString amod = QString::fromStdString(tntxp.tmod != -1 ? to_string(tntxp.amod) : "");
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
