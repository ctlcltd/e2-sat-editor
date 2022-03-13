/*!
 * e2-sat-editor/src/gui/tab.cpp
 *
 * @link https://github.com/ctlcltd/e2-sat-editor
 * @copyright e2 SAT Editor Team
 * @author Leonardo Laureti
 * @version 0.1
 * @license MIT License
 * @license GNU GPLv3 License
 */

#include <algorithm>
#include <map>
#include <filesystem>
#include <cstdio>

#include <QGridLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QSplitter>
#include <QGroupBox>
#include <QToolBar>
#include <QPushButton>
#include <QMessageBox>
#include <QErrorMessage>
#include <QStyle>

#include "../commons.h"
#include "tab.h"
#include "gui.h"
#include "todo.h"
#include "channelBook.h"

using namespace std;
using namespace e2se;

namespace e2se_gui
{

tab::tab(gui* gid, QWidget* wid, string filename = "")
{
	debug("tab");

	this->gid = gid;
	this->cwid = wid;
	QWidget* widget = new QWidget;
	widget->setStyleSheet("QGroupBox { spacing: 0; padding: 20px 0 0 0; border: 0 } QGroupBox::title { margin: 0 12px }");

	QGridLayout* frm = new QGridLayout(widget);

	QHBoxLayout* top = new QHBoxLayout;
	QGridLayout* container = new QGridLayout;
	QHBoxLayout* bottom = new QHBoxLayout;

	QSplitter* splitterc = new QSplitter;

	QVBoxLayout* bouquets_box = new QVBoxLayout;
	QVBoxLayout* list_box = new QVBoxLayout;

	QGroupBox* bouquets = new QGroupBox("Bouquets");
	QGroupBox* channels = new QGroupBox("Channels");

	QGridLayout* list_layout = new QGridLayout;
	this->list_wrap = new QWidget;
	list_wrap->setObjectName("channels_wrap");
	list_wrap->setStyleSheet("#channels_wrap { background: transparent }");

	this->bouquets_tree = new QTreeWidget;
	this->list_tree = new QTreeWidget;
	bouquets_tree->setStyleSheet("QTreeWidget { background: transparent } ::item { padding: 6px auto }");
	list_tree->setStyleSheet("::item { padding: 6px auto }");

	bouquets_tree->setHeaderHidden(true);
	bouquets_tree->setUniformRowHeights(true);
	list_tree->setUniformRowHeights(true);

	bouquets_tree->setSelectionBehavior(QAbstractItemView::SelectRows);
	bouquets_tree->setDragDropMode(QAbstractItemView::InternalMove);
	bouquets_tree->setEditTriggers(QAbstractItemView::NoEditTriggers);
	
	list_tree->setRootIsDecorated(false);
	list_tree->setSelectionBehavior(QAbstractItemView::SelectRows);
	list_tree->setSelectionMode(QAbstractItemView::ContiguousSelection);
	list_tree->setItemsExpandable(false);
	list_tree->setExpandsOnDoubleClick(false);
	list_tree->setDropIndicatorShown(true);
	list_tree->setDragDropMode(QAbstractItemView::InternalMove);
	list_tree->setEditTriggers(QAbstractItemView::NoEditTriggers);

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
	list_tree->setColumnWidth(col++, 125);		// SAT
	list_tree->setColumnWidth(col++, 75);		// System

	this->lheaderv = list_tree->header();
	lheaderv->connect(lheaderv, &::QHeaderView::sectionClicked, [=](int column) { this->trickySortByColumn(column); });
	
	QToolBar* top_toolbar = new QToolBar;
	top_toolbar->setStyleSheet("QToolBar { padding: 0 12px } QToolButton { font: 20px }");

	QToolBar* bottom_toolbar = new QToolBar;
	bottom_toolbar->setStyleSheet("QToolBar { padding: 8px 12px } QToolButton { font: bold 16px }");

	top_toolbar->addAction(QIcon(gid->icopx + "file-open.png"), "Open", [=]() { this->open(); });
	top_toolbar->addAction(QIcon(gid->icopx + "save.png"), "Save", [=]() { this->save(false); });
	top_toolbar->addSeparator();
	top_toolbar->addAction(QIcon(gid->icopx + "import.png"), "Import", todo);
	top_toolbar->addAction(QIcon(gid->icopx + "export.png"), "Export", todo);
	top_toolbar->addSeparator();
	top_toolbar->addAction(QIcon(gid->icopx + "settings.png"), "Settings", [=]() { gid->settings(); });

	if (DEBUG_TOOLBAR)
	{
		QWidget* bottom_toolbar_spacer = new QWidget;
		bottom_toolbar_spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
		bottom_toolbar->addAction("§ Load seeds", [=]() { this->loadSeeds(); });
		bottom_toolbar->addAction("§ Reset", [=]() { this->newFile(); gid->tabChangeName(ttid, ""); });
		bottom_toolbar->addWidget(bottom_toolbar_spacer);
		bottom_toolbar->addAction("FTP Test §", [=]() { gid->ftpConnect(); });
	}

	QToolBar* bouquets_ats = new QToolBar;
	QToolBar* list_ats = new QToolBar;
	this->ats = new actions;

	ats->list_dnd = new QLabel; //TODO QPushButton
	ats->list_dnd->setText("• Drag&Drop actived");
	QWidget* list_ats_spacer = new QWidget;
	list_ats_spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	
	bouquets_ats->addAction("+ New Bouquet", todo);
	ats->list_addch = list_ats->addAction("+ Add Channel", [=]() { this->addChannel(); });
	ats->list_newch = list_ats->addAction("+ New Service", todo);
	list_ats->addWidget(list_ats_spacer);
	list_ats->addWidget(ats->list_dnd);

	// bouquets_tree->connect(bouquets_tree, &QTreeWidget::itemSelectionChanged, [=]() { this->populate(); });
	bouquets_tree->connect(bouquets_tree, &QTreeWidget::currentItemChanged, [=]() { this->populate(); });

	top->addWidget(top_toolbar);
	bottom->addWidget(bottom_toolbar);

	bouquets_box->addWidget(bouquets_ats);
	bouquets_box->addWidget(bouquets_tree);
	bouquets->setLayout(bouquets_box);

	list_layout->addWidget(list_tree);
	list_layout->setContentsMargins(3, 3, 3, 3);
	list_wrap->setLayout(list_layout);

	list_box->addWidget(list_wrap);
	list_box->addWidget(list_ats);
	channels->setLayout(list_box);

	bouquets->setFlat(true);
	channels->setFlat(true);
	bouquets->setMinimumWidth(240);
	channels->setMinimumWidth(520);

	splitterc->addWidget(bouquets);
	splitterc->addWidget(channels);
	splitterc->setStretchFactor(0, 1);
	splitterc->setStretchFactor(1, 4);

	container->addWidget(splitterc, 0, 0, 1, 1);
	container->setContentsMargins(8, 8, 8, 8);

	frm->setContentsMargins(0, 0, 0, 0);
	frm->addLayout(top, 0, 0);
	frm->addLayout(container, 1, 0);
	frm->addLayout(bottom, 2, 0);

	this->widget = widget;

	if (filename.empty())
		newFile();
	else
		load(filename);
}

void tab::newFile()
{
	debug("tab", "newFile()");
	
	this->dbih = new e2db;
	this->_state_nwwr = true;

	bouquets_tree->setDragEnabled(false);
	bouquets_tree->viewport()->setAcceptDrops(false);
	this->_state_dnd = false;
	bouquets_tree->scrollToItem(bouquets_tree->topLevelItem(0));
	bouquets_tree->clear();
	lheaderv->setSortIndicatorShown(false);
	lheaderv->setSectionsClickable(false);
	list_tree->setDragEnabled(false);
	list_tree->viewport()->setAcceptDrops(false);
	list_tree->scrollToItem(list_tree->topLevelItem(0));
	list_tree->clear();
}

void tab::open()
{
	debug("tab", "open()");

	string dirname = gid->openFileDialog();

	if (! dirname.empty())
	{
		load(dirname);
		gid->tabChangeName(ttid, dirname);
	}
}

void tab::addChannel()
{
	QDialog* dial = new QDialog(cwid);
	dial->setMinimumSize(760, 420);
	dial->setWindowTitle("Add Channel");

	QGridLayout* layout = new QGridLayout;
	channelBook* cb = new channelBook(dbih);

	layout->addWidget(cb->widget);
	layout->setContentsMargins(0, 0, 0, 0);
	dial->setLayout(layout);
	dial->exec();
}

bool tab::load(string filename)
{
	debug("tab", "load()", "filename", filename);

	string dirname;

	if (! filename.empty())
		dirname = filename;

	if (dirname.empty())
	{
		return false;
	}
	else
	{
		newFile();
		if (dbih->read(dirname))
		{
			this->_state_nwwr = false;
			this->filename = dirname;
			if (DEBUG_E2DB) dbih->debugger();
		}
		else
		{
			QErrorMessage warn = QErrorMessage();
			warn.showMessage("Error reading files.");
			return false;
		}
	}

	QTreeWidgetItem* titem = new QTreeWidgetItem();
	QVariantMap tdata;
	tdata["bouquet_id"] = "chs";
	titem->setData(0, Qt::UserRole, QVariant (tdata));
	titem->setText(0, "All channels");

	bouquets_tree->addTopLevelItem(titem);

	sort(dbih->index["bss"].begin(), dbih->index["bss"].end());
	map<string, QTreeWidgetItem*> bgroups;

	for (auto & bsi : dbih->index["bss"])
	{
		debug("tab", "load()", "bouquet", bsi.second);
		e2db::bouquet gboq = dbih->bouquets[bsi.second];
		QString bgroup = QString::fromStdString(bsi.second);
		QString qbname = QString::fromStdString(gboq.nname.empty() ? gboq.name : gboq.nname);

		QTreeWidgetItem* pgroup = new QTreeWidgetItem();
		pgroup->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
		QMap<QString, QVariant> tdata;
		tdata["bouquet_id"] = bgroup;
		pgroup->setData(0, Qt::UserRole, QVariant (tdata));
		pgroup->setText(0, qbname);
		bouquets_tree->addTopLevelItem(pgroup);
		bouquets_tree->expandItem(pgroup);

		for (string & ubname : gboq.userbouquets)
			bgroups[ubname] = pgroup;
	}
	for (auto & ubi : dbih->index["ubs"])
	{
		debug("tab", "load()", "userbouquet", ubi.second);
		e2db::userbouquet uboq = dbih->userbouquets[ubi.second];
		QString bgroup = QString::fromStdString(ubi.second);
		QTreeWidgetItem* pgroup = bgroups[ubi.second];
		//macos: unwanted chars [qt.qpa.fonts] Menlo notice
		QString qbname;
		if (gid->unicode_fix)
			qbname = QString::fromStdString(uboq.name).remove(QRegularExpression("[^\\p{L}\\p{N}\\p{Pc}\\p{M}\\p{P}\\s]+"));
		else
			qbname = QString::fromStdString(uboq.name);

		QTreeWidgetItem* bitem = new QTreeWidgetItem(pgroup);
		bitem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsDragEnabled | Qt::ItemNeverHasChildren);
		QMap<QString, QVariant> tdata;
		tdata["bouquet_id"] = bgroup;
		bitem->setData(0, Qt::UserRole, QVariant (tdata));
		bitem->setText(0, qbname);
		bouquets_tree->addTopLevelItem(bitem);
	}

	bouquets_tree->setDragEnabled(true);
	bouquets_tree->viewport()->setAcceptDrops(true);
	populate();

	int counters[4];
	counters[0] = dbih->index["chs:0"].size();
	counters[1] = dbih->index["chs:1"].size();
	counters[2] = dbih->index["chs:2"].size();
	counters[3] = dbih->index["chs"].size();

	gid->loaded(counters);

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
	list_tree->setDragEnabled(false);
	this->_state_dnd = false;
	list_tree->viewport()->setAcceptDrops(false);
	list_tree->scrollToItem(list_tree->topLevelItem(0));
	list_tree->clear();

	string cur_chlist = "chs";
	int i = 0;

	if (! cur_bouquet.empty() && cur_bouquet != "chs")
		cur_chlist = cur_bouquet;

	//TODO [API] cannot add handler to n from ns - dropping

	for (auto & ch : dbih->index[cur_chlist])
	{
		char ci[7];
		sprintf(ci, "%06d", i++);
		QString x = QString::fromStdString(ci);

		if (dbih->db.services.count(ch.second))
		{
			e2db::service chdata = dbih->db.services[ch.second];
			e2db::transponder txdata = dbih->db.transponders[chdata.txid];

			QString idx = QString::fromStdString(to_string(ch.first));
			//macos: unwanted chars [qt.qpa.fonts] Menlo notice
			QString chname;
			if (gid->unicode_fix)
				chname = QString::fromStdString(chdata.chname).remove(QRegularExpression("[^\\p{L}\\p{N}\\p{Pc}\\p{M}\\p{P}\\s]+"));
			else
				chname = QString::fromStdString(chdata.chname);
			QString chid = QString::fromStdString(ch.second);
			QString txid = QString::fromStdString(chdata.txid);
			QString stype = e2db::STYPES.count(chdata.stype) ? QString::fromStdString(e2db::STYPES.at(chdata.stype).second) : "Data";
			QString pname = QString::fromStdString(chdata.data.count(e2db::PVDR_DATA.at('p')) ? chdata.data[e2db::PVDR_DATA.at('p')][0] : "");

			QString freq = QString::fromStdString(txdata.freq);
			QString pol = QString::fromStdString(txdata.pol != -1 ? e2db::SAT_POL[txdata.pol] : "");
			QString sr = QString::fromStdString(txdata.sr);
			QString fec = QString::fromStdString(e2db::SAT_FEC[txdata.fec]);
			string ppos;
			if (txdata.ttype == 's')
			{
				if (dbih->tuners.count(txdata.pos))
				{
					ppos = dbih->tuners.at(txdata.pos).name;
				} else {
					char cposdeg[5];
					sprintf(cposdeg, "%.1f", float(txdata.pos / 10));
					ppos = (string (cposdeg) + (txdata.pos ? 'E' : 'W'));
				}
			}
			QString pos = QString::fromStdString(ppos);
			string psys;
			if (txdata.ttype == 's')
				psys = txdata.sys != -1 ? e2db::SAT_SYS[txdata.sys] : "DVB-S";
			else if (txdata.ttype == 't')
				psys = "DVB-T"; //TODO terrestrial.xml
			else if (txdata.ttype == 'c')
				psys = "DVB-C";
			QString sys = QString::fromStdString(psys);

			QTreeWidgetItem* item;
			if (DEBUG) item = new QTreeWidgetItem({x, idx, chname, chid, txid, stype, pname, freq, pol, sr, fec, pos, sys});
			else item = new QTreeWidgetItem({x, idx, chname, stype, pname, freq, pol, sr, fec, pos, sys});
			item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsDragEnabled | Qt::ItemNeverHasChildren);

			list_tree->addTopLevelItem(item);
		}
		else
		{
			e2db::reference cref = dbih->userbouquets[cur_bouquet].channels[ch.second];

			//TODO marker QWidget ?
			if (cref.refmrker)
			{
				QString chid = QString::fromStdString(cref.chid);
				QString refval = QString::fromStdString(cref.refval);

				QTreeWidgetItem* item = new QTreeWidgetItem({x, "", refval, chid, "", "MARKER"});
				item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsDragEnabled | Qt::ItemNeverHasChildren);
				list_tree->addTopLevelItem(item);
			}
			else
			{
				error("tab", "populate()", "chid", ch.second, "\t");
			}
		}
	}

	if (this->_state_sort.first)
	{
		list_tree->sortByColumn(this->_state_sort.first, this->_state_sort.second);
		if (this->_state_sort.first == 0) lheaderv->setSortIndicator(1, this->_state_sort.second); //TODO FIX Index sort
	}
	lheaderv->setSectionsClickable(true);
	list_tree->setDragEnabled(true);
	list_tree->viewport()->setAcceptDrops(true);
	this->_state_dnd = true;
}

//TODO FIX
void tab::trickySortByColumn(int column)
{
	debug("tab", "trickySortByColumn()", "column", to_string(column));

	Qt::SortOrder order = lheaderv->sortIndicatorOrder();
	column = column == 1 ? 0 : column;

	if (column)
	{
		list_tree->sortItems(column, order);
		disallowDnD();
	}
	else
	{
		list_tree->sortByColumn(column, order);
		lheaderv->setSortIndicator(1, order);
		allowDnD();
	}
	this->_state_sort = pair (column, order); //C++ 17
}

void tab::allowDnD()
{
	debug("tab", "allowDnd()");

	if (this->_state_dnd) return;

	list_tree->setDragEnabled(true);
	list_tree->viewport()->setAcceptDrops(true);
	list_wrap->setStyleSheet("#channels_wrap { background: transparent }");
	ats->list_dnd->setText("• Drag&Drop activated");
	this->_state_dnd = true;
}

//TODO FIX unexpect behav switchs to QAbstractItemView::MultiSelection
void tab::disallowDnD()
{
	debug("tab", "disallowDnD()");

	if (! this->_state_dnd) return;

	list_tree->setDragEnabled(false);
	list_tree->viewport()->setAcceptDrops(false);
	list_wrap->setStyleSheet("#channels_wrap { background: rgba(255, 192, 0, 20%) }");
	ats->list_dnd->setText("• Drag&Drop deactivated");
	this->_state_dnd = false;
}

void tab::setTabId(int ttid)
{
	debug("tab", "setTabId()", "ttid", to_string(ttid));

	this->ttid = ttid;
}

void tab::save(bool saveas)
{
	debug("tab", "save()", "saveas", to_string(saveas));

	QMessageBox dial = QMessageBox();
	string filename;
	bool overwrite = ! saveas && (! this->_state_nwwr || this->_state_ovwr);

	if (overwrite)
	{
		filename = this->filename;
		dial.setText("Files will be overwritten.");
		dial.exec();
	}
	else
	{
		filename = gid->saveFileDialog(this->filename);
	}

	if (! filename.empty())
	{
		debug("tab", "save()", "overwrite", to_string(overwrite));
		debug("tab", "save()", "filename", filename);

		if (dbih->write(filename, overwrite))
		{
			dial.setText("Saved!");
			dial.exec();
		}
		else
		{
			QErrorMessage warn = QErrorMessage();
			warn.showMessage("Error writing files.");
		}
	}
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
