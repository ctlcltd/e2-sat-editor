/*!
 * e2-sat-editor/src/gui_qt6.cpp
 * 
 * @author Leonardo Laureti
 * @version 0.1
 * @license MIT License
 */

#include <iostream>
#include <algorithm>
#include <string>
#include <vector>
#include <map>
#include <Qt>
#include <QApplication>
#include <QWidget>
#include <QScreen>
#include <QGridLayout>
#include <QGroupBox>
#include <QHeaderView>
#include <QTreeWidget>
#include <QToolBar>
#include <QMessageBox>
#include <QFileDialog>
#include <QString>
#include <cstdio>
#include <stdlib.h>
#include "commons.h"
#include "e2db.h"

using namespace std;


void todo()
{
	cout << "app TODO" << endl;
	QMessageBox dial = QMessageBox();
	dial.setText("app TODO");
	dial.exec();
}


class gui
{
	public:
		void root(int argc, char* argv[]);
		void main(QWidget& mwid);
		void tab(QWidget& ttab);
		void newFile();
		bool load(string filename = "");
		void populate();
		void trickySortByColumn(int column);
		void loadSeeds();
	private:
		e2db_parser* temp_parser;
		map<string, e2db_parser::transponder> temp_transponders;
		map<string, e2db_parser::service> temp_channels;
		pair<map<string, e2db_parser::bouquet>, map<string, e2db_parser::userbouquet>> temp_bouquets;
		map<string, vector<pair<int, string>>> temp_index;
		QTreeWidget* bouquets_tree;
		QTreeWidget* list_tree;
		QHeaderView* lheaderv;
		pair<int, Qt::SortOrder> _state_sort;
};

void gui::root(int argc, char* argv[])
{
	debug("gui", "root() qt6");

	QApplication mroot = QApplication(argc, argv);

	QScreen* screen = mroot.primaryScreen();
	QSize wsize = screen->availableSize();

	QWidget mwid = QWidget();
	mwid.setWindowTitle("enigma2 channel editor");
	mwid.resize(wsize);

	mroot.setStyleSheet("QGroupBox { spacing: 0; padding: 20px 0 0 0; border: 0 } QGroupBox::title { margin: 0 12px }");

	main(mwid);

	mwid.show();

	debug("\t", "mroot.exec()");

	mroot.exec();
}

void gui::main(QWidget& mwid)
{
	debug("gui", "main()");

	tab(mwid);
}

void gui::tab(QWidget& ttab)
{
	debug("gui", "tab()");

	QGridLayout* frm = new QGridLayout(&ttab);

	QHBoxLayout* top = new QHBoxLayout;
	QGridLayout* container = new QGridLayout;
	QHBoxLayout* bottom = new QHBoxLayout;

	QGroupBox* bouquets = new QGroupBox("Bouquets");
	QGroupBox* channels = new QGroupBox("Channels");

	QVBoxLayout* bouquets_box = new QVBoxLayout;
	QVBoxLayout* list_box = new QVBoxLayout;

	container->addWidget(bouquets, 1, 0);
	container->addWidget(channels, 1, 1);

	this->bouquets_tree = new QTreeWidget;
	this->list_tree = new QTreeWidget;
	bouquets_tree->setStyleSheet("QTreeWidget { background: transparent } ::item { padding: 6px auto }");
	list_tree->setStyleSheet("::item { padding: 6px auto }");

	bouquets_tree->setHeaderHidden(true);
	bouquets_tree->setUniformRowHeights(true);
	list_tree->setUniformRowHeights(true);
//	QTreeWidgetItem* bheader_item = bouquets_tree->headerItem();
//	bheader_item->setText(0, "Bouquets");
//	bheader_item->setSizeHint(0, QSize(0, 0));

	QTreeWidgetItem* lheader_item; // Qt5
	if (DEBUG) lheader_item = new QTreeWidgetItem({"", "Index", "Name", "CHID", "TXID", "Type", "Provider", "Frequency", "Polarization", "Symbol Rate", "FEC", "SAT", "System"});
	else lheader_item = new QTreeWidgetItem({"", "Index", "Name", "Type", "Provider", "Frequency", "Polarization", "Symbol Rate", "FEC", "SAT", "System"});

	list_tree->setHeaderItem(lheader_item);
	list_tree->setColumnHidden(0, true);
	int col = 1;
	list_tree->setColumnWidth(col++, 75);		// Index
	list_tree->setColumnWidth(col++, 200);		// Name
	if (DEBUG) {
		list_tree->setColumnWidth(col++, 175);	// CHID
		list_tree->setColumnWidth(col++, 150);	// TXID
	}
	else
	{
		col -= 2;
	}
	list_tree->setColumnWidth(col++, 85);		// Type
	list_tree->setColumnWidth(col++, 150);		// Provider
	list_tree->setColumnWidth(col++, 95);		// Frequency
	list_tree->setColumnWidth(col++, 85);		// Polarization
	list_tree->setColumnWidth(col++, 95);		// Symbol Rate
	list_tree->setColumnWidth(col++, 50);		// FEC
	list_tree->setColumnWidth(col++, 85);		// SAT
	list_tree->setColumnWidth(col++, 75);		// System

	this->lheaderv = list_tree->header();
	lheaderv->connect(lheaderv, &::QHeaderView::sectionClicked, [=](int column) { this->trickySortByColumn(column); });
	
	QToolBar* top_toolbar = new QToolBar();
	top_toolbar->setStyleSheet("QToolButton { font: 20px }");

	QToolBar* bottom_toolbar = new QToolBar;
	bottom_toolbar->setStyleSheet("QToolButton { font: bold 16px }");

	top_toolbar->addAction("New", [=]() { this->newFile(); });
	top_toolbar->addAction("Open", [=]() { this->load(); });
	top_toolbar->addAction("Save", todo);
	top_toolbar->addAction("Import", todo);
	top_toolbar->addAction("Export", todo);

	if (DEBUG_TOOLBAR)
		bottom_toolbar->addAction("§ Load seeds", [=]() { this->loadSeeds(); });

	bouquets_tree->connect(bouquets_tree, &QTreeWidget::itemSelectionChanged, [=]() { this->populate(); });

	top->addWidget(top_toolbar);
	bottom->addWidget(bottom_toolbar);

	bouquets_box->addWidget(bouquets_tree);
	bouquets->setLayout(bouquets_box);

	list_box->addWidget(list_tree);
	channels->setLayout(list_box);

	bouquets->setFlat(true);
	channels->setFlat(true);

	container->setColumnStretch(0, 0);
	container->setColumnStretch(1, 1);

	frm->addLayout(top, 0, 0);
	frm->addLayout(container, 1, 0);
	frm->addLayout(bottom, 2, 0);
}

void gui::newFile()
{
	debug("gui", "newFile()");
	
	this->temp_parser = new e2db_parser;

	bouquets_tree->scrollToItem(bouquets_tree->topLevelItem(0));
	bouquets_tree->clear();
	lheaderv->setSortIndicatorShown(false);
	lheaderv->setSectionsClickable(false);
	list_tree->scrollToItem(list_tree->topLevelItem(0));
	list_tree->clear();
}

//TODO remove filename from args
bool gui::load(string filename)
{
	debug("gui", "load()", "filename", filename);

	string dirname;

	if (filename != "")
	{
		dirname = filename;
	}
	else
	{
		//TODO ~ $HOME
		QString qdirname = QFileDialog::getExistingDirectory(nullptr, "Select enigma2 db folder", "~", QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
		dirname = qdirname.toStdString();
	}

	if (dirname != "")
	{
		newFile();
		if (temp_parser->load(dirname))
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
		debug("gui", "load()", "bouquet", gboq.first);

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
		debug("gui", "load()", "userbouquet", uboq.first);

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

void gui::populate()
{
	QTreeWidgetItem* selected = bouquets_tree->currentItem();
	string cur_bouquet = "";

	if (selected != NULL)
	{
		QVariantMap tdata = selected->data(0, Qt::UserRole).toMap();
		QString qcur_bouquet = tdata["bouquet_id"].toString();
		cur_bouquet = qcur_bouquet.toStdString();
	}

	debug("gui", "populate()", "cur_bouquet", cur_bouquet);

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

void gui::trickySortByColumn(int column)
{
	debug("gui", "trickySortByColumn()", "column", to_string(column));

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

//TEST
void gui::loadSeeds()
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
