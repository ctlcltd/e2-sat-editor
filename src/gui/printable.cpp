/*!
 * e2-sat-editor/src/gui/printable.cpp
 *
 * @link https://github.com/ctlcltd/e2-sat-editor
 * @copyright e2 SAT Editor Team
 * @author Leonardo Laureti
 * @version 0.3
 * @license MIT License
 * @license GNU GPLv3 License
 */

#include <cstdio>
#include <cmath>
#include <unordered_set>
#include <filesystem>

#include <QtGlobal>
#include <QPrinter>
#include <QTextDocument>
#include <QPrintDialog>

#include "printable.h"
#include "dataHandler.h"

using std::to_string, std::unordered_set;

using namespace e2se;

namespace e2se_gui
{

printable::printable(QWidget* cwid, dataHandler* data, e2se::logger::session* log)
{
	this->log = new logger(log, "printable");
	debug("printable()");

	this->cwid = cwid;
	this->data = data;
}

void printable::documentAll()
{
	debug("documentAll()");

	auto* dbih = this->data->dbih;

	documentIndex();
	documentServices();
	for (auto & x : dbih->index["bss"])
	{
		string bname = x.second;
		e2db::bouquet gboq = dbih->bouquets[bname];
		documentBouquet(bname);

		for (string & bname : gboq.userbouquets)
		{
			documentUserbouquet(bname);
		}
	}
	for (auto & x : dbih->tuners)
	{
		documentTunersets(x.first);
	}
}

void printable::documentIndex()
{
	debug("documentIndex()");

	auto* dbih = this->data->dbih;

	string filename = std::filesystem::path(dbih->get_filepath()).filename().u8string(); //C++17
	if (filename.empty())
	{
		filename = "Untitled";
	}

	html_page page;
	pageHeader(page, filename, DOC_VIEW::view_index);
	pageFooter(page, filename, DOC_VIEW::view_index);
	
	vector<string> paths;
	for (auto & x : dbih->get_input())
	{
		paths.emplace_back(x.first);
	}
	pageBodyIndexList(page, paths);

	pages.emplace_back(page);
}

void printable::documentServices()
{
	documentServices(-1);
}

void printable::documentServices(int stype)
{
	debug("documentServices()");

	auto* dbih = this->data->dbih;
	
	string filename = std::filesystem::path(dbih->get_services_filename()).filename().u8string(); //C++17
	string iname;
	string xname;
	string headname = filename;
	string footname = filename;
	switch (stype)
	{
		// Data
		case e2db::STYPE::data:
			iname = "chs:0";
			xname = "Data";
		break;
		// TV
		case e2db::STYPE::tv:
			iname = "chs:1";
			xname = "TV";
		break;
		// Radio
		case e2db::STYPE::radio:
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

	html_page page;
	pageHeader(page, headname, DOC_VIEW::view_services);
	pageFooter(page, footname, DOC_VIEW::view_services);

	pageBodyChannelList(page, iname, DOC_VIEW::view_services);

	pages.emplace_back(page);
}

void printable::documentBouquet(string bname)
{
	debug("documentBouquet()", "bname", bname);

	html_page page;
	pageHeader(page, bname, DOC_VIEW::view_bouquets);
	pageFooter(page, bname, DOC_VIEW::view_bouquets);

	pageBodyBouquetList(page, bname);

	pages.emplace_back(page);
}

void printable::documentUserbouquet(string bname)
{
	debug("documentUserbouquet()", "bname", bname);

	html_page page;
	pageHeader(page, bname, DOC_VIEW::view_userbouquets);
	pageFooter(page, bname, DOC_VIEW::view_userbouquets);

	pageBodyChannelList(page, bname, DOC_VIEW::view_userbouquets);

	pages.emplace_back(page);
}

void printable::documentTunersets(int ytype)
{
	debug("documentTunersets()", "ytype", ytype);

	string filename;
	switch (ytype)
	{
		case e2db::YTYPE::satellite:
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

	html_page page;
	pageHeader(page, filename, DOC_VIEW::view_tunersets);
	pageFooter(page, filename, DOC_VIEW::view_tunersets);

	pageBodyTunersetsList(page, ytype);

	pages.emplace_back(page);
}

void printable::pageHeader(html_page& page, string filename, DOC_VIEW view)
{
	auto* dbih = this->data->dbih;

	QString name;

	page.header += "<div class=\"header\">";
	if (view == DOC_VIEW::view_index)
	{
		name = "Settings";
	}
	else if (view == DOC_VIEW::view_tunersets)
	{
		name = "Tuner Settings";
	}
	else if (view == DOC_VIEW::view_bouquets)
	{
		if (dbih->bouquets.count(filename))
		{
			e2db::bouquet gboq = dbih->bouquets[filename];
			name = QString::fromStdString(gboq.name);
		}
	}
	else if (view == DOC_VIEW::view_userbouquets)
	{
		if (dbih->userbouquets.count(filename))
		{
			e2db::userbouquet uboq = dbih->userbouquets[filename];
			name = QString::fromStdString(uboq.name);
		}
	}
	else
	{
		name = "Services List";
	}
	page.header += "<h1>" + name + "</h1>";
	page.header += "<h3>" + QString::fromStdString(filename) + "</h3>";
	page.header += "</div>";
}

void printable::pageFooter(html_page& page, string filename, DOC_VIEW view)
{
	auto* dbih = this->data->dbih;

	QString editor = QString::fromStdString(dbih->editor_string(true));
	QString timestamp = QString::fromStdString(dbih->editor_timestamp());

	page.footer += "<div class=\"footer\">";
	page.footer += "File: <b>" + QString::fromStdString(filename) + "</b><br>";
	page.footer += "Editor: <b>" + editor + "</b><br>";
	page.footer += "Datetime: <b>" + timestamp + "</b>";
	page.footer += "</div>";
}

void printable::pageBodyIndexList(html_page& page, vector<string> paths)
{
	debug("pageBodyIndexList()");

	auto* dbih = this->data->dbih;

	page.body += "<div class=\"toc\">";
	page.body += "<h4>Table of Contents</h4>";
	page.body += "<table>";
	page.body += "<thead>";
	page.body += "<tr>";
	page.body += "<th>Content</th>";
	page.body += "<th>Type</th>";
	page.body += "</tr>";

	page.body += "<tbody>";
	for (auto & path : paths)
	{
		string filename = std::filesystem::path(path).filename().u8string(); //C++17
		QString fname = QString::fromStdString(filename);
		QString ftype;
		e2db::FPORTS fpi = dbih->filetype_detect(path);
		switch (fpi)
		{
			case e2db::FPORTS::single_tunersets:
				ftype = "tunersets";
			break;
			case e2db::FPORTS::single_bouquet:
				ftype = "bouquet";
			break;
			case e2db::FPORTS::single_userbouquet:
				ftype = "userbouquet";
			break;
			default:
				ftype = "services";
		}

		page.body += "<tr>";
		page.body += "<td class=\"trid\">" + fname + "</td>";
		page.body += "<td>" + ftype + "</td>";
		page.body += "</tr>";
	}
	page.body += "</tbody>";

	page.body += "</table>";
	page.body += "</div>";
}

void printable::pageBodyChannelList(html_page& page, string bname, DOC_VIEW view)
{
	auto* dbih = this->data->dbih;

	if (dbih->index.count(bname))
		debug("pageBodyChannelList()", "bname", bname);
	else
		error("pageBodyChannelList()", "bname", bname);
	debug("pageBodyChannelList()", "view", view);
	
	QString cssname = view == DOC_VIEW::view_bouquets ? "userbouquet" : "services";
	
	page.body += "<div class=\"" + cssname + "\">";
	page.body += "<table>";
	page.body += "<thead>";
	page.body += "<tr>";
	page.body += "<th>Index</th>";
	page.body += "<th>Name</th>";
	// page.body += "<th>Reference ID</th>";
	page.body += "<th>SSID</th>";
	page.body += "<th>TSID</th>";
	page.body += "<th>ONID</th>";
	page.body += "<th>Type</th>";
	page.body += "<th>CAS</th>";
	page.body += "<th>Provider</th>";
	page.body += "<th>Freq</th>";
	page.body += "<th>Pol</th>";
	page.body += "<th>SR</th>";
	page.body += "<th>FEC</th>";
	page.body += "<th>Sys</th>";
	page.body += "<th>Pos</th>";
	page.body += "<th>Tuner</th>";
	page.body += "</tr>";
	page.body += "</thead>";

	page.body += "<tbody>";
	for (auto & chdata : dbih->index[bname])
	{
		string chid = chdata.second;

		if (dbih->db.services.count(chdata.second))
		{
			e2db::service ch = dbih->db.services[chid];
			e2db::transponder tx;
			if (ch.tsid != 0)
				tx = dbih->db.transponders[ch.txid];

			QString idx = QString::fromStdString(to_string(chdata.first));
			QString chname = QString::fromStdString(ch.chname);
			QString refid;

			// services
			if (view == DOC_VIEW::view_services)
			{
				string crefid = dbih->get_reference_id(chid);
				refid = QString::fromStdString(crefid);
			}
			// bouquets
			else
			{
				e2db::channel_reference chref;
				if (dbih->userbouquets.count(bname))
					chref = dbih->userbouquets[bname].channels[chid];
				string crefid = dbih->get_reference_id(chref);
				refid = QString::fromStdString(crefid);
			}
			QString ssid = QString::fromStdString(to_string(ch.ssid));
			QString tsid = QString::fromStdString(to_string(ch.tsid));
			QString onid = QString::fromStdString(to_string(ch.onid));
			QString stype = QString::fromStdString(dbih->value_service_type(ch.stype));
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
			QString pname = QString::fromStdString(dbih->value_channel_provider(ch));
			QString freq = QString::fromStdString(to_string(tx.freq));
			QString pol = QString::fromStdString(dbih->value_transponder_polarization(tx.pol));
			QString sr = QString::fromStdString(to_string(tx.sr));
			QString fec = QString::fromStdString(dbih->value_transponder_fec(tx.fec, tx.ytype));
			QString pos = QString::fromStdString(dbih->value_transponder_position(tx));
			QString sys = QString::fromStdString(dbih->value_transponder_system(tx));
			QString tname = QString::fromStdString(dbih->get_tuner_name(tx));

			page.body += "<tr>";
			page.body += "<td class=\"trid\">" + idx + "</td>";
			page.body += "<td class=\"chname\">" + chname + "</td>";
			// page.body += "<td class=\"refid\"><span >" + refid + "</span></td>";
			page.body += "<td>" + ssid + "</td>";
			page.body += "<td>" + tsid + "</td>";
			page.body += "<td>" + onid + "</td>";
			page.body += "<td>" + stype + "</td>";
			page.body += "<td class=\"scas\">" + scas + "</span></td>";
			page.body += "<td class=\"pname\">" + pname + "</td>";
			page.body += "<td>" + freq + "</td>";
			page.body += "<td>" + pol + "</td>";
			page.body += "<td>" + sr + "</td>";
			page.body += "<td>" + fec + "</td>";
			page.body += "<td>" + sys + "</td>";
			page.body += "<td>" + pos + "</td>";
			page.body += "<td>" + tname + "</td>";
			page.body += "</tr>";
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

			page.body += "<tr class=\"marker\">";
			page.body += "<td class=\"trid\"></td>";
			page.body += "<td class=\"name\">" + value + "</td>";
			// page.body += "<td class=\"refid\">" + refid + "</td>";
			page.body += "<td></td>";
			page.body += "<td></td>";
			page.body += "<td class=\"atype\">" + atype + "</td>";
			page.body += "<td></td>";
			page.body += "<td></td>";
			page.body += "<td></td>";
			page.body += "<td></td>";
			page.body += "<td></td>";
			page.body += "<td></td>";
			page.body += "<td></td>";
			page.body += "<td></td>";
			page.body += "<td></td>";
			page.body += "</tr>";
		}
	}
	page.body += "</tbody>";

	page.body += "</table>";
	page.body += "</div>";
}

void printable::pageBodyBouquetList(html_page& page, string bname)
{
	auto* dbih = this->data->dbih;

	if (dbih->bouquets.count(bname))
		debug("pageBodyBouquetList()", "bname", bname);
	else
		error("pageBodyBouquetList()", "bname", bname);

	e2db::bouquet gboq = dbih->bouquets[bname];
	QString btype;
	if (gboq.btype == e2db::STYPE::tv)
		btype = "TV";
	else if (gboq.btype == e2db::STYPE::radio)
		btype = "Radio";

	page.body += "<div class=\"bouquet\">";
	page.body += "<table>";
	page.body += "<thead>";
	page.body += "<tr>";
	page.body += "<th>Index</th>";
	page.body += "<th>Bouquet</th>";
	page.body += "<th>Userbouquet</th>";
	page.body += "<th>Name</th>";
	page.body += "<th>Type</th>";
	page.body += "</tr>";

	page.body += "<tbody>";
	int i = 1;
	for (auto & bname : gboq.userbouquets)
	{
		e2db::userbouquet uboq = dbih->userbouquets[bname];
		page.body += "<tr>";
		page.body += "<td class=\"trid\">" + QString::fromStdString(to_string(i++)) + "</td>";
		page.body += "<td>" + QString::fromStdString(gboq.name) + "</td>";
		page.body += "<td>" + QString::fromStdString(bname) + "</td>";
		page.body += "<td>" + QString::fromStdString(uboq.name) + "</td>";
		page.body += "<td>" + btype + "</td>";
		page.body += "</tr>";
	}
	page.body += "</tbody>";

	page.body += "</table>";
	page.body += "</div>";
}

//TODO improve list
void printable::pageBodyTunersetsList(html_page& page, int ytype)
{
	debug("pageBodyTunersetsList()", "ytype", ytype);

	auto* dbih = this->data->dbih;

	page.body += "<div class=\"tunersets\">";

	e2db::tunersets tvoq = dbih->tuners[ytype];
	string iname = "tns:";
	char yname = dbih->value_transponder_type(ytype);
	iname += yname;
	
	for (auto & x : dbih->index[iname])
	{
		string tnid = x.second;
		e2db::tunersets_table tns = tvoq.tables[tnid];
		QString tnname = QString::fromStdString(tns.name);
		string ppos;
		if (ytype == e2db::YTYPE::satellite)
		{
			ppos = dbih->value_transponder_position(tns);
		}
		QString pos = QString::fromStdString(ppos);

		page.body += "<div class=\"transponder\">";
		page.body += "<h4>Transponders</h4>";
		page.body += "<h2>" + tnname + "</h2>";
		if (ytype == e2db::YTYPE::satellite)
			page.body += "<p>Position: <b>" + pos + "</b></p>";
		page.body += "<table>";
		page.body += "<thead>";
		page.body += "<tr>";
		page.body += "<th>Index</th>";
		if (ytype == e2db::YTYPE::satellite)
		{
			page.body += "<th>Freq</th>";
			page.body += "<th>Pol</th>";
			page.body += "<th>SR</th>";
			page.body += "<th>FEC</th>";
			page.body += "<th>Sys</th>";
			page.body += "<th>Mod</th>";
			page.body += "<th>Inv</th>";
			page.body += "<th>Pilot</th>";
			page.body += "<th>Rollof</th>";
		}
		else if (ytype == e2db::YTYPE::terrestrial)
		{
			page.body += "<th>Freq</th>";
			page.body += "<th>Const</th>";
			page.body += "<th>Band</th>";
			page.body += "<th>Sys</th>";
			page.body += "<th>Tmx Mode</th>";
			page.body += "<th>HP FEC</th>";
			page.body += "<th>LP FEC</th>";
			page.body += "<th>Inv</th>";
			page.body += "<th>Guard</th>";
			page.body += "<th>Hier</th>";
		}
		else if (ytype == e2db::YTYPE::cable)
		{
			page.body += "<th>Freq</th>";
			page.body += "<th>Mod</th>";
			page.body += "<th>SR</th>";
			page.body += "<th>FEC</th>";
			page.body += "<th>Inv</th>";
			page.body += "<th>Sys</th>";
		}
		else if (ytype == e2db::YTYPE::atsc)
		{
			page.body += "<th>Freq</th>";
			page.body += "<th>Mod</th>";
			page.body += "<th>Sys</th>";
		}
		page.body += "</tr>";

		page.body += "<tbody>";
		int i = 1;
		for (auto & x : dbih->index[tns.tnid])
		{
			string trid = x.second;
			e2db::tunersets_transponder tntxp = tns.transponders[trid];

			page.body += "<tr>";
			page.body += "<td class=\"trid\">" + QString::fromStdString(to_string(i++)) + "</td>";
			if (ytype == e2db::YTYPE::satellite)
			{
				QString freq = QString::fromStdString(to_string(tntxp.freq));
				QString pol = QString::fromStdString(dbih->value_transponder_polarization(tntxp.pol));
				QString sr = QString::fromStdString(to_string(tntxp.sr));
				QString fec = QString::fromStdString(dbih->value_transponder_fec(tntxp.fec, e2db::YTYPE::satellite));
				QString sys = QString::fromStdString(dbih->value_transponder_system(tntxp.sys, e2db::YTYPE::satellite));
				QString mod = QString::fromStdString(dbih->value_transponder_modulation(tntxp.mod, e2db::YTYPE::satellite));
				QString inv = QString::fromStdString(dbih->value_transponder_inversion(tntxp.inv, e2db::YTYPE::satellite));
				QString pil = QString::fromStdString(dbih->value_transponder_pilot(tntxp.pil));
				QString rol = QString::fromStdString(dbih->value_transponder_rollof(tntxp.rol));

				page.body += "<td>" + freq + "</td>";
				page.body += "<td>" + pol + "</td>";
				page.body += "<td>" + sr + "</td>";
				page.body += "<td>" + fec + "</td>";
				page.body += "<td>" + sys + "</td>";
				page.body += "<td>" + mod + "</td>";
				page.body += "<td>" + inv + "</td>";
				page.body += "<td>" + pil + "</td>";
				page.body += "<td>" + rol + "</td>";
			}
			else if (ytype == e2db::YTYPE::terrestrial)
			{
				QString freq = QString::fromStdString(to_string(tntxp.freq));
				QString tmod = QString::fromStdString(dbih->value_transponder_modulation(tntxp.tmod, e2db::YTYPE::terrestrial));
				QString band = QString::fromStdString(dbih->value_transponder_bandwidth(tntxp.band));
				QString sys = QString::fromStdString(dbih->value_transponder_system(tntxp.sys, e2db::YTYPE::terrestrial));
				QString tmx = QString::fromStdString(dbih->value_transponder_tmx_mode(tntxp.tmx));
				QString hpfec = QString::fromStdString(dbih->value_transponder_fec(tntxp.hpfec, e2db::YTYPE::terrestrial));
				QString lpfec = QString::fromStdString(dbih->value_transponder_fec(tntxp.lpfec, e2db::YTYPE::terrestrial));
				QString inv = QString::fromStdString(dbih->value_transponder_inversion(tntxp.inv, e2db::YTYPE::terrestrial));
				QString guard = QString::fromStdString(dbih->value_transponder_guard(tntxp.guard));
				QString hier = QString::fromStdString(dbih->value_transponder_hier(tntxp.hier));

				page.body += "<td>" + freq + "</td>";
				page.body += "<td>" + tmod + "</td>";
				page.body += "<td>" + band + "</td>";
				page.body += "<td>" + sys + "</td>";
				page.body += "<td>" + tmx + "</td>";
				page.body += "<td>" + hpfec + "</td>";
				page.body += "<td>" + lpfec + "</td>";
				page.body += "<td>" + inv + "</td>";
				page.body += "<td>" + guard + "</td>";
				page.body += "<td>" + hier + "</td>";
			}
			else if (ytype == e2db::YTYPE::cable)
			{
				QString freq = QString::fromStdString(to_string(tntxp.freq));
				QString cmod = QString::fromStdString(dbih->value_transponder_modulation(tntxp.cmod, e2db::YTYPE::cable));
				QString sr = QString::fromStdString(to_string(tntxp.sr));
				QString cfec = QString::fromStdString(dbih->value_transponder_fec(tntxp.cfec, e2db::YTYPE::cable));
				QString inv = QString::fromStdString(dbih->value_transponder_inversion(tntxp.inv, e2db::YTYPE::cable));
				QString sys = QString::fromStdString(dbih->value_transponder_system(tntxp.sys, e2db::YTYPE::cable));

				page.body += "<td>" + freq + "</td>";
				page.body += "<td>" + cmod + "</td>";
				page.body += "<td>" + sr + "</td>";
				page.body += "<td>" + cfec + "</td>";
				page.body += "<td>" + inv + "</td>";
				page.body += "<td>" + sys + "</td>";
			}
			else if (ytype == e2db::YTYPE::atsc)
			{
				QString freq = QString::fromStdString(to_string(tntxp.freq));
				QString amod = QString::fromStdString(to_string(tntxp.amod));
				QString sys = QString::fromStdString(dbih->value_transponder_system(tntxp.sys, e2db::YTYPE::atsc));

				page.body += "<td>" + freq + "</td>";
				page.body += "<td>" + amod + "</td>";
				page.body += "<td>" + sys + "</td>";
			}
			page.body += "</tr>";
		}
		page.body += "</tbody>";

		page.body += "</table>";
		page.body += "</div>";
	}

	page.body += "</div>";
}

void printable::print()
{
	debug("print()");

	QPrinter* printer = new QPrinter;
	QTextDocument* doc = new QTextDocument;

#if QT_VERSION >= QT_VERSION_CHECK(5, 30, 0)
	printer->setPageSize(QPageSize::A4);
#else
	printer->setPageSize(QPagedPaintDevice::PageSize::A4);
#endif
	printer->setPageOrientation(QPageLayout::Landscape);

	QStringList html;
	html.append(docHtmlHead());
	html.append("<body>");

	for (auto & page : pages)
	{
		html.append(page.header);
		html.append(page.body);
		html.append(page.footer);
	}

	html.append("</body>");
	html.append(docHtmlFoot());

	doc->setHtml(html.join(""));

	QPrintDialog pdial = QPrintDialog(printer, cwid);
	if (pdial.exec() == QDialog::Accepted) {
		doc->print(printer);
	}
}

//TODO FIX Qt5 borderSize
QString printable::docHtmlHead()
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

QString printable::docHtmlFoot()
{
	return "</html>";
}

}
