/*!
 * e2-sat-editor/src/gui/tab.cpp
 *
 * @author Leonardo Laureti
 * @version 0.1
 * @license MIT License
 */

#include <iostream>
#include <filesystem>
#include <algorithm>
#include <string>
#include <vector>
#include <map>
#include <Qt>
#include <QWidget>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QSplitter>
#include <QGroupBox>
#include <QHeaderView>
#include <QTreeWidget>
#include <QToolBar>
#include <QPushButton>
#include <QLabel>
#include <QString>
#include <cstdio>
#include <stdlib.h>
#include "../commons.h"
#include "../e2db.h"
#include "tab.h"
#include "gui.h"
#include "todo.h"

using namespace std;

namespace e2se_gui
{

tab::tab(gui* gid, QWidget* wid, string filename = "")
{
	debug("tab()");

	this->gid = gid;
	this->cwid = wid;
	QWidget* widget = new QWidget;

	QGridLayout* frm = new QGridLayout(widget);

	QHBoxLayout* top = new QHBoxLayout;
	QGridLayout* container = new QGridLayout;
	QHBoxLayout* bottom = new QHBoxLayout;

	QSplitter* splitterc = new QSplitter;

	QVBoxLayout* bouquets_box = new QVBoxLayout;
	QVBoxLayout* list_box = new QVBoxLayout;

	QGroupBox* bouquets = new QGroupBox("Bouquets");
	QGroupBox* channels = new QGroupBox("Channels");

	this->bouquets_tree = new QTreeWidget;
	this->list_tree = new QTreeWidget;
	bouquets_tree->setStyleSheet("QTreeWidget { background: transparent } ::item { padding: 6px auto }");
	list_tree->setStyleSheet("QTreeWidget { border: 3px solid red } ::item { padding: 6px auto }");

	bouquets_tree->setHeaderHidden(true);
	bouquets_tree->setUniformRowHeights(true);
	list_tree->setUniformRowHeights(true);

	QTreeWidgetItem* lheader_item; // Qt5
	if (DEBUG) lheader_item = new QTreeWidgetItem({"", "Index", "Name", "CHID", "TXID", "Type", "Provider", "Frequency", "Polarization", "Symbol Rate", "FEC", "SAT", "System"});
	else lheader_item = new QTreeWidgetItem({"", "Index", "Name", "Type", "Provider", "Frequency", "Polarization", "Symbol Rate", "FEC", "SAT", "System"});

	list_tree->setHeaderItem(lheader_item);
	list_tree->setColumnHidden(0, true);
	int col = 1;
	list_tree->setColumnWidth(col++, 75);        // Index
	list_tree->setColumnWidth(col++, 200);        // Name
	if (DEBUG) {
		list_tree->setColumnWidth(col++, 175);    // CHID
		list_tree->setColumnWidth(col++, 150);    // TXID
	}
	else
	{
		col -= 2;
	}
	list_tree->setColumnWidth(col++, 85);        // Type
	list_tree->setColumnWidth(col++, 150);        // Provider
	list_tree->setColumnWidth(col++, 95);        // Frequency
	list_tree->setColumnWidth(col++, 85);        // Polarization
	list_tree->setColumnWidth(col++, 95);        // Symbol Rate
	list_tree->setColumnWidth(col++, 50);        // FEC
	list_tree->setColumnWidth(col++, 85);        // SAT
	list_tree->setColumnWidth(col++, 75);        // System

	this->lheaderv = list_tree->header();
	lheaderv->connect(lheaderv, &::QHeaderView::sectionClicked, [=](int column) { this->trickySortByColumn(column); });
	
	QToolBar* top_toolbar = new QToolBar;
	top_toolbar->setStyleSheet("QToolBar { padding: 0 12px } QToolButton { font: 20px }");

	QToolBar* bottom_toolbar = new QToolBar;
	bottom_toolbar->setStyleSheet("QToolBar { padding: 8px 12px } QToolButton { font: bold 16px }");

	top_toolbar->addAction("Open", [=]() { this->open(); });
	top_toolbar->addAction("Save", todo);
	top_toolbar->addAction("Import", todo);
	top_toolbar->addAction("Export", todo);
	top_toolbar->addAction("Settings", [=]() { gid->settings(); });

	if (DEBUG_TOOLBAR)
	{
		bottom_toolbar->addAction("§ Load seeds", [=]() { this->loadSeeds(); });
		bottom_toolbar->addAction("§ Reset", [=]() { this->newFile(); gid->tabChangeName(tid, ""); });
	}

	QToolBar* bouquets_ats = new QToolBar;
	QToolBar* list_ats = new QToolBar;

	QLabel* list_ats_dndstatus = new QLabel;
	list_ats_dndstatus->setText("• Drag&Drop actived");
	QWidget* list_ats_spacer = new QWidget;
	list_ats_spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	
	bouquets_ats->addAction("+ New Bouquet", todo);
	list_ats->addAction("+ Add Channel", todo);
	list_ats->addWidget(list_ats_spacer);
	list_ats->addWidget(list_ats_dndstatus);

	bouquets_tree->connect(bouquets_tree, &QTreeWidget::itemSelectionChanged, [=]() { this->populate(); });

	top->addWidget(top_toolbar);
	bottom->addWidget(bottom_toolbar);

	bouquets_box->addWidget(bouquets_ats);
	bouquets_box->addWidget(bouquets_tree);
	bouquets->setLayout(bouquets_box);

	list_box->addWidget(list_tree);
	list_box->addWidget(list_ats);
	channels->setLayout(list_box);

	bouquets->setFlat(true);
	channels->setFlat(true);

	splitterc->addWidget(bouquets);
	splitterc->addWidget(channels);
	splitterc->setStretchFactor(0, 1.5);
	splitterc->setStretchFactor(1, 4.5);

	container->addWidget(splitterc, 0, 0, 1, 1);
	container->setContentsMargins(8, 8, 8, 8);

	frm->setContentsMargins(0, 0, 0, 0);
	frm->addLayout(top, 0, 0);
	frm->addLayout(container, 1, 0);
	frm->addLayout(bottom, 2, 0);

	this->widget = widget;
}

void tab::newFile()
{
	debug("tab", "newFile()");
	
	this->temp_parser = new e2db_parser;

	bouquets_tree->scrollToItem(bouquets_tree->topLevelItem(0));
	bouquets_tree->clear();
	lheaderv->setSortIndicatorShown(false);
	lheaderv->setSectionsClickable(false);
	list_tree->scrollToItem(list_tree->topLevelItem(0));
	list_tree->clear();
}

void tab::open()
{
	debug("tab", "open()");

	string dirname = gid->openFileDialog();

	if (dirname != "")
	{
		load(dirname);
		gid->tabChangeName(tid, dirname);
	}
}

//TODO remove filename from args
bool tab::load(string filename)
{
	debug("tab", "load()", "filename", filename);

	string dirname;

	if (filename != "")
		dirname = filename;

	if (dirname != "")
	{
		newFile();
		if (temp_parser->read(dirname))
		{
			if (DEBUG_E2DB)
				temp_parser->debugger();
			temp_transponders = temp_parser->get_transponders();
			temp_channels = temp_parser->get_channels();
			temp_bouquets = temp_parser->get_bouquets();
			temp_index = temp_parser->index;
		}
		else
		{
			return false;
		}
	}
	else
	{
		return false;
	}

	QTreeWidgetItem* titem = new QTreeWidgetItem();
	QVariantMap tdata;
	tdata["bouquet_id"] = "all";
	titem->setData(0, Qt::UserRole, QVariant (tdata));
	titem->setText(0, "All channels");

	bouquets_tree->addTopLevelItem(titem);

	map<string, QTreeWidgetItem*> bgroups;

	//TODO order A-Z & parent
	for (auto & gboq : temp_bouquets.first)
	{
		debug("tab", "load()", "bouquet", gboq.first);

		QString bgroup = QString::fromStdString(gboq.first);
		QString bcname = QString::fromStdString(gboq.second.nname.size() ? gboq.second.nname : gboq.second.name);

		QTreeWidgetItem* pgroup = new QTreeWidgetItem();
		QMap<QString, QVariant> tdata;
		tdata["bouquet_id"] = bgroup;
		pgroup->setData(0, Qt::UserRole, QVariant (tdata));
		pgroup->setText(0, bcname);
		bouquets_tree->addTopLevelItem(pgroup);
		bouquets_tree->expandItem(pgroup);

		for (auto & ubname : gboq.second.userbouquets)
			bgroups[ubname] = pgroup;
	}
	for (auto & uboq : temp_bouquets.second)
	{
		debug("tab", "load()", "userbouquet", uboq.first);

		QString bgroup = QString::fromStdString(uboq.first);
		QTreeWidgetItem* pgroup = bgroups[uboq.first];

		QTreeWidgetItem* bitem = new QTreeWidgetItem(pgroup);
		QMap<QString, QVariant> tdata;
		tdata["bouquet_id"] = bgroup;
		bitem->setData(0, Qt::UserRole, QVariant (tdata));
		bitem->setText(0, QString::fromStdString(uboq.second.name));
		bouquets_tree->addTopLevelItem(bitem);
	}

	populate();
	return true;
}

void tab::populate()
{
	QTreeWidgetItem* selected = bouquets_tree->currentItem();
	string cur_bouquet = "";

	if (selected != NULL)
	{
		QVariantMap tdata = selected->data(0, Qt::UserRole).toMap();
		QString qcur_bouquet = tdata["bouquet_id"].toString();
		cur_bouquet = qcur_bouquet.toStdString();
	}

	debug("tab", "populate()", "cur_bouquet", cur_bouquet);

	lheaderv->setSortIndicatorShown(true);
	lheaderv->setSectionsClickable(false);
	list_tree->scrollToItem(list_tree->topLevelItem(0));
	list_tree->clear();

	string cur_chlist = "all";
	vector<pair<int, string>> cur_chdata;
	int i = 0;

	if (cur_bouquet != "" && cur_bouquet != "all")
		cur_chlist = cur_bouquet;
	cur_chdata = temp_index[cur_chlist]; //TODO reference

	for (auto & ch : cur_chdata)
	{
		char ci[6];
		sprintf(ci, "%05d", i++);
		QString x = QString::fromStdString(ci);

		//TODO ? transponder.ttype
		if (temp_channels.count(ch.second))
		{
			e2db_parser::service cdata = temp_channels[ch.second];
			auto txdata = temp_transponders[cdata.txid];

			QString idx = QString::fromStdString(to_string(ch.first));
			QString chname = QString::fromStdString(cdata.chname);
			QString chid = QString::fromStdString(ch.second);
			QString txid = QString::fromStdString(cdata.txid);
			QString stype = STYPES.count(cdata.stype) ? QString::fromStdString(STYPES.at(cdata.stype)) : "Data";
			QString pname = QString::fromStdString(cdata.data.at('p')[0]);
			QString freq = QString::fromStdString(txdata.freq);
			QString pol = QString::fromStdString(SAT_POL[txdata.pol]);
			QString sr = QString::fromStdString(txdata.sr);
			QString fec = QString::fromStdString(SAT_FEC[txdata.fec]);
			QString pos = QString::fromStdString(to_string(txdata.pos));
			QString sys = QString::fromStdString(SAT_SYS[txdata.sys]);

			QTreeWidgetItem* item; // Qt5
			if (DEBUG) item = new QTreeWidgetItem({x, idx, chname, chid, txid, stype, pname, freq, pol, sr, fec, pos, sys});
			else item = new QTreeWidgetItem({x, idx, chname, stype, pname, freq, pol, sr, fec, pos, sys});

			list_tree->addTopLevelItem(item);
		}
		//TODO marker QWidget ?
		else
		{
			e2db_parser::reference cref = temp_bouquets.second[cur_bouquet].channels[ch.second];

			QString chid = QString::fromStdString(cref.chid);
			QString refval = QString::fromStdString(cref.refval);

			QTreeWidgetItem* item = new QTreeWidgetItem({x, "", refval, chid, "", "MARKER"});
			list_tree->addTopLevelItem(item);
		}
	}

	if (_state_sort.first)
	{
		list_tree->sortByColumn(_state_sort.first, _state_sort.second);
		if (_state_sort.first == 0) lheaderv->setSortIndicator(1, _state_sort.second); //TODO FIX Index sort
	}
	lheaderv->setSectionsClickable(true);
}

void tab::trickySortByColumn(int column)
{
	debug("tab", "trickySortByColumn()", "column", to_string(column));

	Qt::SortOrder order = lheaderv->sortIndicatorOrder();
	column = column == 1 ? 0 : column;

	if (column)
	{
		list_tree->sortItems(column, order);
	}
	else
	{
		list_tree->sortByColumn(column, order);
		lheaderv->setSortIndicator(1, order);
	}
	_state_sort = pair (column, order);
}

void tab::setIndex(int index)
{
	debug("tab", "setIndex()", "index", to_string(index));

	tid = index;
}

//TEST
void tab::loadSeeds()
{
	string cwd;

	// *ux
	char* ccwd = getenv("PWD");
	if (ccwd != NULL) cwd = string (ccwd);

	// xcodeproj
	if (cwd.empty())
	{
		char* ccwd = getenv("DYLD_FRAMEWORK_PATH");
		if (ccwd != NULL) cwd = string (ccwd);
		if (cwd.find("Debug") != string::npos) cwd = cwd.substr(0, cwd.length() - 6); // rstrip /Debug
	}

	cwd = cwd.substr(0, cwd.length() - 4); // rstrip /src

	if (cwd != "")
	{
		filesystem::path path = cwd + "/seeds./enigma_db";
		load(filesystem::absolute(path));
	}
}
//TEST

}
