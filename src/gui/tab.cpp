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
#include <filesystem>
#include <cstdio>

#include <QGuiApplication>
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
#include <QTimer>

#include <QList>

#include "../commons.h"
#include "tab.h"
#include "gui.h"
#include "DropEventHandler.h"
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
	bouquets_tree->setDragDropMode(QAbstractItemView::DragDrop);
	bouquets_tree->setEditTriggers(QAbstractItemView::NoEditTriggers);

	list_tree->setRootIsDecorated(false);
	list_tree->setSelectionBehavior(QAbstractItemView::SelectRows);
	list_tree->setSelectionMode(QAbstractItemView::ExtendedSelection);
	list_tree->setItemsExpandable(false);
	list_tree->setExpandsOnDoubleClick(false);
	list_tree->setDropIndicatorShown(true);
	list_tree->setDragDropMode(QAbstractItemView::InternalMove);
	list_tree->setEditTriggers(QAbstractItemView::NoEditTriggers);

	QTreeWidgetItem* lheader_item = new QTreeWidgetItem({"", "Index", "Name", "CHID", "TXID", "Type", "Provider", "Frequency", "Polarization", "Symbol Rate", "FEC", "SAT", "System"});

	int col = 0;
	list_tree->setHeaderItem(lheader_item);
	list_tree->setColumnHidden(col++, true);
	list_tree->setColumnWidth(col++, 75);		// Index
	list_tree->setColumnWidth(col++, 200);		// Name
	if (DEBUG) {
		list_tree->setColumnWidth(col++, 175);	// CHID
		list_tree->setColumnWidth(col++, 150);	// TXID
	}
	else
	{
		list_tree->setColumnHidden(col++, true);
		list_tree->setColumnHidden(col++, true);
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

	top_toolbar->addAction(QIcon(gid->icopx + "file-open.png"), "Open", [=]() { this->openFile(); });
	top_toolbar->addAction(QIcon(gid->icopx + "save.png"), "Save", [=]() { this->saveFile(false); });
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

	DropEventHandler* bouquets_evt = new DropEventHandler;
	this->list_evt = new TreeEventObserver;
	bouquets_tree->viewport()->installEventFilter(bouquets_evt);
	bouquets_tree->connect(bouquets_tree, &QTreeWidget::currentItemChanged, [=]() { this->updateListIndex(); this->populate(); });
	list_tree->installEventFilter(list_evt);
	list_tree->connect(list_tree, &QTreeWidget::currentItemChanged, [=]() { this->listItemChanged(); });

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
		readFile(filename);
}

void tab::newFile()
{
	debug("tab", "newFile()");

	initialize();
}

void tab::openFile()
{
	debug("tab", "openFile()");

	string dirname = gid->openFileDialog();

	if (! dirname.empty())
	{
		readFile(dirname);
		gid->tabChangeName(ttid, dirname);
	}
}

void tab::saveFile(bool saveas)
{
	debug("tab", "saveFile()", "saveas", to_string(saveas));

	QMessageBox dial = QMessageBox();
	string filename;
	bool overwrite = ! saveas && (! this->_state_nwwr || this->_state_ovwr);

	if (overwrite)
	{
		//TEST
		this->updateListIndex();
		dbih->set_index(index);
		//TEST
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
		debug("tab", "saveFile()", "overwrite", to_string(overwrite));
		debug("tab", "saveFile()", "filename", filename);

		QGuiApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
		bool wr = dbih->write(filename, overwrite);
		QGuiApplication::restoreOverrideCursor();
		
		if (wr) {
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

bool tab::readFile(string filename)
{
	debug("tab", "readFile()", "filename", filename);

	if (filename.empty())
		return false;

	initialize();

	QGuiApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
	bool rr = dbih->prepare(filename);
	QGuiApplication::restoreOverrideCursor();

	if (! rr)
	{
		QErrorMessage warn = QErrorMessage();
		warn.showMessage("Error opening files.");
		return false;
	}

	this->_state_nwwr = false;
	this->filename = filename;
	this->index = dbih->index;

	load();

	return true;
}

void tab::load()
{
	debug("tab", "load()");

	sort(index["bss"].begin(), index["bss"].end());
	unordered_map<string, QTreeWidgetItem*> bgroups;

	for (auto & bsi : index["bss"])
	{
		debug("tab", "load()", "bouquet", bsi.second);
		e2db::bouquet gboq = dbih->bouquets[bsi.second];
		QString bgroup = QString::fromStdString(bsi.second);
		QString qbname = QString::fromStdString(gboq.nname.empty() ? gboq.name : gboq.nname);

		QTreeWidgetItem* pgroup = new QTreeWidgetItem();
		pgroup->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
		QMap<QString, QVariant> tdata; //TODO
		tdata["id"] = bgroup;
		pgroup->setData(0, Qt::UserRole, QVariant (tdata));
		pgroup->setText(0, qbname);
		bouquets_tree->addTopLevelItem(pgroup);
		bouquets_tree->expandItem(pgroup);

		for (string & ubname : gboq.userbouquets)
			bgroups[ubname] = pgroup;
	}
	for (auto & ubi : index["ubs"])
	{
		debug("tab", "load()", "userbouquet", ubi.second);
		e2db::userbouquet uboq = dbih->userbouquets[ubi.second];
		QString bgroup = QString::fromStdString(ubi.second);
		QTreeWidgetItem* pgroup = bgroups[ubi.second];
		// macos: unwanted chars [qt.qpa.fonts] Menlo notice
		QString qbname;
		if (gid->sets->value("fixUnicodeChars").toBool())
			qbname = QString::fromStdString(uboq.name).remove(QRegularExpression("[^\\p{L}\\p{N}\\p{Pc}\\p{M}\\p{P}\\s]+"));
		else
			qbname = QString::fromStdString(uboq.name);

		QTreeWidgetItem* bitem = new QTreeWidgetItem(pgroup);
		bitem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled | Qt::ItemNeverHasChildren);
		QMap<QString, QVariant> tdata; //TODO
		tdata["id"] = bgroup;
		bitem->setData(0, Qt::UserRole, QVariant (tdata));
		bitem->setText(0, qbname);
		bouquets_tree->addTopLevelItem(bitem);
	}

	bouquets_tree->setDragEnabled(true);
	bouquets_tree->setAcceptDrops(true);
	populate();
	setCounters();
}

void tab::populate()
{
	string curr_chlist;
	string prev_chlist;
	bool precached = false;
	QList<QTreeWidgetItem*> cachep;
	
	if (! cache.empty())
	{
		for (int i = 0; i < list_tree->topLevelItemCount(); i++)
		{
			QTreeWidgetItem* item = list_tree->topLevelItem(i);
			cachep.append(item->clone());
		}
		precached = true;
		prev_chlist = string (this->_state_curr);
	}

	QTreeWidgetItem* selected = bouquets_tree->currentItem();
	if (selected == NULL)
		selected = bouquets_tree->topLevelItem(0);
	if (selected != NULL)
	{
		QVariantMap tdata = selected->data(0, Qt::UserRole).toMap();
		QString qcurr_bouquet = tdata["id"].toString();
		curr_chlist = qcurr_bouquet.toStdString();
		this->_state_curr = curr_chlist;
	}

	debug("tab", "populate()", "curr_chlist", curr_chlist);

	lheaderv->setSortIndicatorShown(true);
	lheaderv->setSectionsClickable(false);
	list_tree->setDragEnabled(false);
	list_tree->setAcceptDrops(false);
	this->_state_dnd = false;
	list_tree->scrollToItem(list_tree->topLevelItem(0)); //TODO FIX
	list_tree->clear();
	if (precached)
	{
		cache[prev_chlist].swap(cachep);
	}

	//TODO [API] cannot add handler to n from ns - dropping

	int i = 0;

	if (cache[curr_chlist].isEmpty())
	{
	// QList<QTreeWidgetItem*> cache;
		for (auto & ch : index[curr_chlist])
		{
			char ci[7];
			sprintf(ci, "%06d", i++);
			QString x = QString::fromStdString(ci);
			QString idx;
			QStringList qitem;

			if (dbih->db.services.count(ch.second))
			{
				qitem = dbih->entries.services[ch.second];
				idx = QString::fromStdString(to_string(ch.first));
				qitem.prepend(idx);
				qitem.prepend(x);
			}
			else
			{
				e2db::reference cref = dbih->userbouquets[curr_chlist].channels[ch.second];

				if (cref.refmrker)
				{
					qitem = dbih->entry_marker(cref);
					idx = qitem[1];
					qitem.prepend(x);
				}
				else
				{
					//TEST
					qitem = QStringList({x, "", "", QString::fromStdString(ch.second), "", "ERROR"});
					idx = 0;
					error("tab", "populate()", "chid", ch.second, "\t");
					//TEST
				}
			}

			QTreeWidgetItem* item = new QTreeWidgetItem(qitem);
			item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsDragEnabled | Qt::ItemNeverHasChildren);
			item->setData(0, Qt::UserRole, idx);   // data: Index
			item->setData(1, Qt::UserRole, false); // data: marker flag
			cache[curr_chlist].append(item);
			// cache.append(item);
		}
	}

	list_tree->addTopLevelItems(cache[curr_chlist]);
	// list_tree->addTopLevelItems(cache);

	if (this->_state_sort.first)
	{
		list_tree->sortByColumn(this->_state_sort.first, this->_state_sort.second);
		if (this->_state_sort.first == 0) lheaderv->setSortIndicator(1, this->_state_sort.second); //TODO FIX Index sort
	}
	lheaderv->setSectionsClickable(true);
	list_tree->setDragEnabled(true);
	list_tree->setAcceptDrops(true);
	this->_state_dnd = true;
}

void tab::listItemChanged()
{
	if (! list_evt->isChanged()) return;

	debug("tab", "listItemChanged()");

	QTimer::singleShot(0, [=]() { this->visualReindexList(); });
	this->_state_changed = true;
}

//TODO improve by positions QAbstractItemView::indexAt(x, y) min|max
void tab::visualReindexList()
{
	int i = 0, j = 0, idx = 0;
	int maxs = list_tree->topLevelItemCount() - 1;

	do
	{
		char ci[7];
		sprintf(ci, "%06d", i + 1);
		QString x = QString::fromStdString(ci);
		QTreeWidgetItem* item = list_tree->topLevelItem(i);
		bool mrkr = item->data(1, Qt::UserRole).toBool();
		if (mrkr)
		{
			idx = 0;
		}
		else
		{
			j += 1;
			idx = j;
		}
		item->setText(0, x);
		if (! mrkr)
			item->setText(1, QString::fromStdString(to_string(idx)));
		i++;
	}
	while (i != maxs);
}

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
	list_tree->setAcceptDrops(true);
	this->_state_dnd = true;
	list_wrap->setStyleSheet("#channels_wrap { background: transparent }");
	ats->list_dnd->setText("• Drag&Drop activated");
}

//TODO FIX unexpect behav switchs to QAbstractItemView::MultiSelection
void tab::disallowDnD()
{
	debug("tab", "disallowDnD()");

	if (! this->_state_dnd) return;

	list_tree->setDragEnabled(false);
	list_tree->setAcceptDrops(false);
	this->_state_dnd = false;
	list_wrap->setStyleSheet("#channels_wrap { background: rgba(255, 192, 0, 20%) }");
	ats->list_dnd->setText("• Drag&Drop deactivated");
}

void tab::updateListIndex()
{
	if (! this->_state_changed) return;

	int i = 0, j = 0, idx = 0;
	int count = list_tree->topLevelItemCount();
	string curr_chlist = this->_state_curr;
	index[curr_chlist].clear();

	debug("tab", "updateListIndex()", "curr_chlist", curr_chlist);

	do
	{
		QTreeWidgetItem* item = list_tree->topLevelItem(i);
		QString chid = item->data(0, Qt::UserRole).toString();
		bool mrkr = item->data(1, Qt::UserRole).toBool();
		if (mrkr)
		{
			idx = 0;
		}
		else
		{
			j += 1;
			idx = j;
		}
		index[curr_chlist].emplace_back(pair (idx, chid.toStdString())); //C++ 17
		i++;
	}
	while (i != count);

	this->_state_changed = false;
}

void tab::setCounters()
{
	debug("tab", "setCounters()");

	int counters[4];
	counters[0] = index["chs:0"].size();
	counters[1] = index["chs:1"].size();
	counters[2] = index["chs:2"].size();
	counters[3] = index["chs"].size();

	gid->loaded(counters);
}

void tab::setTabId(int ttid)
{
	debug("tab", "setTabId()", "ttid", to_string(ttid));

	this->ttid = ttid;
}

void tab::initialize()
{
	debug("tab", "initialize()");

	// if (this->dbih != nullptr)
	//  	delete this->dbih;

	this->dbih = new e2db;
	this->_state_nwwr = true;
	this->_state_ovwr = false;
	this->_state_changed = false;

	bouquets_tree->setDragEnabled(false);
	bouquets_tree->setAcceptDrops(false);
	bouquets_tree->scrollToItem(bouquets_tree->topLevelItem(0));
	bouquets_tree->clear();
	lheaderv->setSortIndicatorShown(false);
	lheaderv->setSectionsClickable(false);
	list_tree->setDragEnabled(false);
	list_tree->setAcceptDrops(false);
	this->_state_dnd = false;
	list_tree->scrollToItem(list_tree->topLevelItem(0));
	list_tree->clear();
	cache.clear();
	index.clear();

	QTreeWidgetItem* titem = new QTreeWidgetItem();
	titem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
	QVariantMap tdata; //TODO
	tdata["id"] = "chs";
	titem->setData(0, Qt::UserRole, QVariant (tdata));
	titem->setText(0, "All channels");

	bouquets_tree->addTopLevelItem(titem);

	setCounters();
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
		readFile(filesystem::absolute(path));
	}
}
//TEST

}
