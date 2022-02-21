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
#include <QProxyStyle>
#include <QScreen>
#include <QStatusBar>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGroupBox>
#include <QHeaderView>
#include <QTreeWidget>
#include <QToolBar>
#include <QPushButton>
#include <QLabel>
#include <QMessageBox>
#include <QFileDialog>
#include <QString>
#include <cstdio>
#include <stdlib.h>
#include "commons.h"
#include "e2db.h"

using namespace std;

namespace e2se_gui
{

void todo()
{
	cout << "app TODO" << endl;
	QMessageBox dial = QMessageBox();
	dial.setText("app TODO");
	dial.exec();
}


class tab
{
	public:
		tab(QWidget* wid);
		void newFile();
		bool load(string filename = "");
		void populate();
		void trickySortByColumn(int column);
		void loadSeeds();
		QWidget* widget;
	private:
		QWidget* cwid;
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

tab::tab(QWidget* wid)
{
	debug("tab()");

	this->cwid = wid;
	QWidget* widget = new QWidget;

	QGridLayout* frm = new QGridLayout(widget);

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
	top_toolbar->setStyleSheet("QToolBar { padding: 0 12px } QToolButton { font: 20px }");

	QToolBar* bottom_toolbar = new QToolBar;
	bottom_toolbar->setStyleSheet("QToolBar { padding: 8px 12px } QToolButton { font: bold 16px }");

	top_toolbar->addAction("Open", [=]() { this->load(); });
	top_toolbar->addAction("Save", todo);

	if (DEBUG_TOOLBAR)
	{
		bottom_toolbar->addAction("§ Load seeds", [=]() { this->loadSeeds(); });
		bottom_toolbar->addAction("§ Reset", [=]() { this->newFile(); });
	}

	bouquets_tree->connect(bouquets_tree, &QTreeWidget::itemSelectionChanged, [=]() { this->populate(); });

	top->addWidget(top_toolbar);
	bottom->addWidget(bottom_toolbar);

	bouquets_box->addWidget(bouquets_tree);
	bouquets->setLayout(bouquets_box);

	list_box->addWidget(list_tree);
	channels->setLayout(list_box);

	bouquets->setFlat(true);
	channels->setFlat(true);

	container->setContentsMargins(8, 8, 8, 8);
	container->setColumnStretch(0, 0);
	container->setColumnStretch(1, 1);

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

//TODO remove filename from args
bool tab::load(string filename)
{
	debug("tab", "load()", "filename", filename);

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

	// qmake in Qt5 wants std::filesystem instead of filesystem
	if (cwd != "")
	{
		std::filesystem::path path = cwd + "/seeds./enigma_db";
		load(std::filesystem::absolute(path));
	}
}
//TEST



class guiProxyStyle : public QProxyStyle
{
  public:
	int styleHint(StyleHint hint, const QStyleOption *option = 0, const QWidget *widget = 0, QStyleHintReturn *returnData = 0) const override
	{
		if (hint == QStyle::SH_TabBar_CloseButtonPosition)
			return QTabBar::RightSide;
		return QProxyStyle::styleHint(hint, option, widget, returnData);
	}
};



class gui
{
	public:
		gui(int argc, char* argv[]);
		void root();
		void tabCtl();
		void statusCtl();
		void newTab();
		void closeTab(int index);
		void tabChanged(int index);
	private:
		QApplication* mroot;
		QWidget* mwid;
		QGridLayout* mfrm;
		QHBoxLayout* mcnt;
		QHBoxLayout* mstatusb;
		QStatusBar* sbwid;
		QTabWidget* twid;
};

gui::gui(int argc, char* argv[])
{
	debug("gui", "qt6");

	this->mroot = new QApplication(argc, argv);
	mroot->setStyle(new guiProxyStyle);

	QScreen* screen = mroot->primaryScreen();
	QSize wsize = screen->availableSize();

	this->mwid = new QWidget;
	mwid->setWindowTitle("enigma2 channel editor");
	mwid->resize(wsize);

	mroot->setStyleSheet("QGroupBox { spacing: 0; padding: 20px 0 0 0; border: 0 } QGroupBox::title { margin: 0 12px }");

	root();

	mwid->show();

	mroot->exec();
}

void gui::root()
{
	debug("gui", "root()");

	this->mfrm = new QGridLayout(mwid);

	this->mcnt = new QHBoxLayout;
	this->mstatusb = new QHBoxLayout;

	mfrm->setContentsMargins(0, 0, 0, 0);
	mfrm->setSpacing(0);

	mfrm->addLayout(mcnt, 0, 0);
	mfrm->addLayout(mstatusb, 1, 0);

	statusCtl();
	tabCtl();
}

//TODO FIX EXC_BAD_ACCESS
void gui::tabCtl()
{
	debug("gui", "tabCtl()");

	this->twid = new QTabWidget(mwid);
	twid->setTabsClosable(true);
	twid->setMovable(true);
	twid->setStyleSheet("QTabWidget::tab-bar { left: 0px } QTabWidget::pane { border: 0; border-radius: 0 } QTabBar::tab { height: 32px; padding: 5px; background: palette(mid); border: 1px solid transparent; border-radius: 0 } QTabBar::tab:selected { background: palette(highlight) } QTabWidget::tab QLabel { margin-left: 5px }");
	twid->connect(twid, &QTabWidget::currentChanged, [=](int index) { this->tabChanged(index); });
	twid->connect(twid, &QTabWidget::tabCloseRequested, [=] (int index) { this->closeTab(index); });

	QPushButton* ttbnew = new QPushButton();
	ttbnew->setText("+ New Tab");
//	ttbnew->setFlat(true);
	ttbnew->setStyleSheet("width: 8ex; height: 32px"); //TODO FIX height & ::left-corner padding
	ttbnew->setMinimumHeight(32);
	ttbnew->connect(ttbnew, &QPushButton::pressed, [=]() { this->newTab(); });
	twid->setCornerWidget(ttbnew, Qt::TopLeftCorner);

	newTab();

	mcnt->addWidget(twid);
}

void gui::statusCtl()
{
	debug("gui", "statusCtl()");

	this->sbwid = new QStatusBar(mwid);

	mstatusb->addWidget(sbwid);
}

void gui::newTab()
{
	tab* ttab = new tab(mwid);
	int ttcount = twid->count();
	QString ttname = QString::fromStdString("Untitled" + (ttcount ? " " + to_string(ttcount++) : ""));
	int index = twid->addTab(ttab->widget, ttname);
	QTabBar* ttabbar = twid->tabBar();
	QLabel* ttlabel = new QLabel;
	ttlabel->setText(ttname);
	ttabbar->setTabButton(index, QTabBar::LeftSide, ttlabel);
	ttabbar->setTabText(index, "");
	twid->setCurrentIndex(index);

	debug("gui", "newTab()", "index", to_string(index));
}

void gui::closeTab(int index)
{
	debug("gui", "closeTab()", "index", to_string(index));

	//TODO destruct
	twid->removeTab(index);

	if (twid->count() == 0) newTab();
}

void gui::tabChanged(int index)
{
	debug("gui", "tabChanged()", "index", to_string(index));

	QString msg = QString::fromStdString("Current tab index: " + to_string(index));
	sbwid->showMessage(msg);
}

}
