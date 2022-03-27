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

#include <QtGlobal>
#include <QGuiApplication>
#include <QTimer>
#include <QList>
#include <QStyle>
#include <QMessageBox>
#include <QErrorMessage>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QSplitter>
#include <QGroupBox>
#include <QToolBar>
#include <QMenu>
#include <QPushButton>
#include <QComboBox>
#include <QClipboard>
#include <QMimeData>

#include "tab.h"
#include "gui.h"
#include "DropEventHandler.h"
#include "todo.h"
#include "editService.h"
#include "channelBook.h"
#include "../ftpcom.h"

using std::to_string;
using namespace e2se;

namespace e2se_gui
{

tab::tab(gui* gid, QWidget* wid, string filename = "")
{
	this->log = new logger("tab");
	debug("tab()");

	this->gid = gid;
	this->cwid = wid;
	this->widget = new QWidget;
	widget->setStyleSheet("QGroupBox { spacing: 0; padding: 20px 0 0 0; border: 0 } QGroupBox::title { margin: 0 12px }");

	QGridLayout* frm = new QGridLayout(widget);

	QHBoxLayout* top = new QHBoxLayout;
	QGridLayout* container = new QGridLayout;
	QHBoxLayout* bottom = new QHBoxLayout;

	//TODO bouquets_box and scrollbar in GTK+3
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
	if (gid->sets->value("debug", true).toBool()) {
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
	lheaderv->connect(lheaderv, &QHeaderView::sectionClicked, [=](int column) { this->trickySortByColumn(column); });

	list_tree->setContextMenuPolicy(Qt::CustomContextMenu);
	list_tree->connect(list_tree, &QTreeWidget::customContextMenuRequested, [=](QPoint pos) { this->showListEditContextMenu(pos); });

	QToolBar* top_toolbar = new QToolBar;
	top_toolbar->setStyleSheet("QToolBar { padding: 0 12px } QToolButton { font: 20px }");

	QToolBar* bottom_toolbar = new QToolBar;
	bottom_toolbar->setStyleSheet("QToolBar { padding: 8px 12px } QToolButton { font: bold 16px }");

	QWidget* top_toolbar_spacer = new QWidget;
	top_toolbar_spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	QComboBox* profile_combo = new QComboBox;
	int profile_sel = gid->sets->value("profile/selected").toInt();
	int size = gid->sets->beginReadArray("profile");
	for (int i = 0; i < size; i++)
	{
		gid->sets->setArrayIndex(i);
		if (! gid->sets->contains("profileName"))
			continue;
		profile_combo->addItem(gid->sets->value("profileName").toString(), i + 1); //TODO
	}
	gid->sets->endArray();
	profile_combo->setCurrentIndex(profile_sel);
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
	profile_combo->connect(profile_combo, &QComboBox::currentIndexChanged, [=](int index) { this->profileComboChanged(index); });
#else
	profile_combo->connect(profile_combo, QOverload<int>::of(&QComboBox::currentIndexChanged), [=](int index) { this->profileComboChanged(index); });
#endif

	top_toolbar->addAction(QIcon(gid->icopx + "file-open.png"), "Open", [=]() { this->openFile(); });
	top_toolbar->addAction(QIcon(gid->icopx + "save.png"), "Save", [=]() { this->saveFile(false); });
	top_toolbar->addSeparator();
	top_toolbar->addAction(QIcon(gid->icopx + "import.png"), "Import", todo);
	top_toolbar->addAction(QIcon(gid->icopx + "export.png"), "Export", todo);
	top_toolbar->addSeparator();
	top_toolbar->addAction(QIcon(gid->icopx + "settings.png"), "Settings", [=]() { gid->settings(); });
	top_toolbar->addWidget(top_toolbar_spacer);
	top_toolbar->addWidget(profile_combo);
	top_toolbar->addAction("Connect", [=]() { this->ftpConnect(); });

	if (gid->sets->value("debug", true).toBool())
	{
		QWidget* bottom_spacer = new QWidget;
		bottom_spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

		bottom_toolbar->addAction("§ Load seeds", [=]() { this->loadSeeds(); });
		bottom_toolbar->addAction("§ Reset", [=]() { this->newFile(); gid->tabChangeName(ttid, ""); });
		bottom_toolbar->addWidget(bottom_spacer);
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
	ats->list_newch = list_ats->addAction("+ New Service", [=]() { this->addService(); });
	list_ats->addWidget(list_ats_spacer);
	list_ats->addWidget(ats->list_dnd);
	ats->list_addch->setDisabled(true);

	DropEventHandler* bouquets_evt = new DropEventHandler;
	this->list_evt = new TreeEventObserver;
	bouquets_tree->viewport()->installEventFilter(bouquets_evt);
	bouquets_tree->connect(bouquets_tree, &QTreeWidget::currentItemChanged, [=](QTreeWidgetItem* current) { this->bouquetsItemChanged(current); });
	list_tree->installEventFilter(list_evt);
	list_tree->connect(list_tree, &QTreeWidget::currentItemChanged, [=]() { this->listItemChanged(); });
	list_tree->connect(list_tree, &QTreeWidget::itemDoubleClicked, [=]() { this->editService(); });

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

	if (filename.empty())
		newFile();
	else
		readFile(filename);
}

tab::~tab()
{
	delete dbih;
	//TODO FIX deleting widget removes next tab
	// delete widget;
}

void tab::newFile()
{
	debug("newFile()");

	initialize();
}

void tab::openFile()
{
	debug("openFile()");

	string dirname = gid->openFileDialog();

	if (! dirname.empty())
	{
		readFile(dirname);
		gid->tabChangeName(ttid, dirname);
	}
}

void tab::saveFile(bool saveas)
{
	debug("saveFile()", "saveas", to_string(saveas));

	QMessageBox dial = QMessageBox();
	string filename;
	bool overwrite = ! saveas && (! this->_state_nwwr || this->_state_ovwr);

	if (overwrite)
	{
		this->updateListIndex();
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
		debug("saveFile()", "overwrite", to_string(overwrite));
		debug("saveFile()", "filename", filename);

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
	debug("addChannel()");

	e2se_gui::channelBook* cb = new e2se_gui::channelBook(dbih);
	string curr_chlist = this->_state_curr;
	QDialog* dial = new QDialog(cwid);
	dial->setMinimumSize(760, 420);
	dial->setWindowTitle("Add Channel");
	//TODO FIX SEGFAULT
	// dial->connect(dial, &QDialog::finished, [=]() { delete dial; delete cb; });

	QGridLayout* layout = new QGridLayout;
	QToolBar* bottom_toolbar = new QToolBar;
	bottom_toolbar->setStyleSheet("QToolBar { padding: 0 8px } QToolButton { font: 16px }");
	QWidget* bottom_spacer = new QWidget;
	bottom_spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	bottom_toolbar->addWidget(bottom_spacer);
	bottom_toolbar->addAction("Add", [=]() { auto selected = cb->getSelected(); this->putChannels(selected, curr_chlist); });

	layout->addWidget(cb->widget);
	layout->addWidget(bottom_toolbar);
	layout->setContentsMargins(0, 0, 0, 0);
	dial->setLayout(layout);
	dial->exec();
}

void tab::addService()
{
	debug("addService()");

	e2se_gui::editService* add = new e2se_gui::editService(dbih);
	add->display(cwid);
}

void tab::editService()
{
	debug("editService()");

	QList<QTreeWidgetItem*> selected = list_tree->selectedItems();
	
	if (selected.empty() || selected.count() > 1)
		return;

	QTreeWidgetItem* item = selected.first();
	string chid = item->data(2, Qt::UserRole).toString().toStdString();
	string nw_chid;
	bool mrkr = item->data(1, Qt::UserRole).toBool();

	debug("editService()", "chid", chid);

	if (! mrkr && dbih->db.services.count(chid))
	{
		e2se_gui::editService* add = new e2se_gui::editService(dbih);
		add->setEditID(chid);
		add->display(cwid);
		nw_chid = add->getEditID(); //TODO returned after dial.exec()

		debug("editService()", "nw_chid", nw_chid);

		QStringList entry = dbih->entries.services[nw_chid];
		entry.prepend(item->text(1));
		entry.prepend(item->text(0));
		for (int i = 0; i < entry.count(); i++)
			item->setText(i, entry[i]);
		item->setData(2, Qt::UserRole, QString::fromStdString(nw_chid)); // data: chid

		dbih->updateUserbouquetIndexes(chid, nw_chid);
	}
}

bool tab::readFile(string filename)
{
	debug("readFile()", "filename", filename);

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

	load();

	return true;
}

void tab::load()
{
	debug("load()");

	sort(dbih->gindex["bss"].begin(), dbih->gindex["bss"].end());
	unordered_map<string, QTreeWidgetItem*> bgroups;

	for (auto & bsi : dbih->gindex["bss"])
	{
		debug("load()", "bouquet", bsi.second);
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
	for (auto & ubi : dbih->gindex["ubs"])
	{
		debug("load()", "userbouquet", ubi.second);
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

	debug("populate()", "curr_chlist", curr_chlist);

	lheaderv->setSortIndicatorShown(true);
	lheaderv->setSectionsClickable(false);
	list_tree->setDragEnabled(false);
	list_tree->setAcceptDrops(false);
	list_tree->scrollToItem(list_tree->topLevelItem(0)); //TODO FIX
	list_tree->clear();
	if (precached)
	{
		cache[prev_chlist].swap(cachep);
	}

	int i = 0;

	if (cache[curr_chlist].isEmpty())
	{
	// QList<QTreeWidgetItem*> cache;
		for (auto & ch : dbih->gindex[curr_chlist])
		{
			char ci[7];
			sprintf(ci, "%06d", i++);
			bool mrkr = false;
			QString chid = QString::fromStdString(ch.second);
			QString x = QString::fromStdString(ci);
			QString idx;
			QStringList entry;

			if (dbih->db.services.count(ch.second))
			{
				entry = dbih->entries.services[ch.second];
				idx = QString::fromStdString(to_string(ch.first));
				entry.prepend(idx);
				entry.prepend(x);
			}
			else
			{
				e2db::reference cref = dbih->userbouquets[curr_chlist].channels[ch.second];

				if (cref.refmrker)
				{
					mrkr = true;
					entry = dbih->entry_marker(cref);
					idx = entry[1];
					entry.prepend(x);
				}
				else
				{
					//TEST
					entry = QStringList({x, "", "", chid, "", "ERROR"});
					// idx = 0; //Qt5
					error("populate()", "chid", ch.second);
					//TEST
				}
			}

			QTreeWidgetItem* item = new QTreeWidgetItem(entry);
			item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsDragEnabled | Qt::ItemNeverHasChildren);
			item->setData(0, Qt::UserRole, idx);  // data: Index
			item->setData(1, Qt::UserRole, mrkr); // data: marker flag
			item->setData(2, Qt::UserRole, chid); // data: chid
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

	setCounters(true);
}

void tab::bouquetsItemChanged(QTreeWidgetItem* current)
{
	debug("bouquetsItemChanged()");

	if (current != NULL)
	{
		int ti = bouquets_tree->indexOfTopLevelItem(current);

		// all | tv | radio
		if (ti != -1)
		{
			ats->list_addch->setDisabled(true);
			ats->list_newch->setEnabled(true);
		}
		// userbouquets
		else
		{
			ats->list_addch->setEnabled(true);
			ats->list_newch->setDisabled(true);
		}
		if (ti > 0)
		{
			disallowDnD();
		}
		else if (ti < 1)
		{
			allowDnD();
		}
	}

	updateListIndex();
	populate();
}

void tab::listItemChanged()
{
	if (! list_evt->isChanged()) return;

	debug("listItemChanged()");

	QTimer::singleShot(0, [=]() { this->visualReindexList(); });
	this->_state_changed = true;
}

//TODO improve by positions QAbstractItemView::indexAt(x, y) min|max
void tab::visualReindexList()
{
	int i = 0, y = 0, idx = 0;
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
			y++;
			idx = y;
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
	debug("trickySortByColumn()", "column", to_string(column));

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
	debug("allowDnd()");

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
	debug("disallowDnD()");

	if (! this->_state_dnd) return;

	list_tree->setDragEnabled(false);
	list_tree->setAcceptDrops(false);
	this->_state_dnd = false;
	list_wrap->setStyleSheet("#channels_wrap { background: rgba(255, 192, 0, 20%) }");
	ats->list_dnd->setText("• Drag&Drop deactivated");
}

void tab::listItemCut()
{
	debug("listItemCut()");

	listItemCopy(true);
}

void tab::listItemCopy(bool cut)
{
	debug("listItemCopy()");

	QList<QTreeWidgetItem*> selected = list_tree->selectedItems();
	
	if (selected.empty())
		return;

	QClipboard* clipboard = QGuiApplication::clipboard();
	QStringList text;
	for (auto & item : selected)
	{
		QStringList data;
		// skip column 0 = x index
		for (int i = 1; i < list_tree->columnCount(); i++)
			data.append(item->data(i, Qt::DisplayRole).toString());
		text.append(data.join(",")); // CSV
	}
	clipboard->setText(text.join("\n")); // CSV

	if (cut)
		listItemDelete();
}

//TODO FIX chlist ERROR
void tab::listItemPaste()
{
	debug("listItemPaste()");

	QTreeWidgetItem* selected = list_tree->currentItem();

	if (selected == NULL)
		selected = list_tree->topLevelItem(list_tree->topLevelItemCount());
	if (selected != NULL)
	{
		QClipboard* clipboard = QGuiApplication::clipboard();
		const QMimeData* mimeData = clipboard->mimeData();
		vector<QString> items;
		string curr_chlist = this->_state_curr;

		if (mimeData->hasText())
		{
			QStringList list = clipboard->text().split("\n");
			for (QString & data : list)
			{
				//TODO validate
				items.emplace_back(data.split(",")[2]);
			}
		}
		if (! items.empty())
			putChannels(items, curr_chlist);
	}
}

void tab::listItemDelete()
{
	debug("listItemDelete()");

	QList<QTreeWidgetItem*> selected = list_tree->selectedItems();
	
	if (selected.empty())
		return;

	lheaderv->setSectionsClickable(false);
	list_tree->setDragEnabled(false);
	list_tree->setAcceptDrops(false);

	for (auto & item : selected)
	{
		int i = list_tree->indexOfTopLevelItem(item);
		list_tree->takeTopLevelItem(i);
	}

	lheaderv->setSectionsClickable(true);
	list_tree->setDragEnabled(true);
	list_tree->setAcceptDrops(true);
	this->_state_changed = true;
	updateListIndex();
	visualReindexList();
	setCounters();
}

//TODO focus in
void tab::listItemSelectAll()
{
	debug("listItemSelectAll()");

	list_tree->selectAll();
}

void tab::listItemAction(int action)
{
	debug("listItemAction()", "action", to_string(action));

	switch (action)
	{
		case LIST_EDIT_ATS::Cut:
			listItemCut();
		break;
		case LIST_EDIT_ATS::Copy:
			listItemCopy();
		break;
		case LIST_EDIT_ATS::Paste:
			listItemPaste();
		break;
		case LIST_EDIT_ATS::Delete:
			listItemDelete();
		break;
		case LIST_EDIT_ATS::SelectAll:
			listItemSelectAll();
		break;
	}
}

//TODO allow duplicates
//TODO put in selected place
//TODO FIX chlist from Paste
void tab::putChannels(vector<QString> channels, string chlist)
{
	debug("putChannels()");

	lheaderv->setSectionsClickable(false);
	list_tree->setDragEnabled(false);
	list_tree->setAcceptDrops(false);
	QList<QTreeWidgetItem*> clist;
	int i = list_tree->topLevelItemCount() + 1;

	for (QString & qchid : channels)
	{
		string chid = qchid.toStdString();
		char ci[7];
		sprintf(ci, "%06d", i);
		QString x = QString::fromStdString(ci);
		QString idx = QString::fromStdString(to_string(i));
		QStringList entry;
		bool mrkr = false;
		if (dbih->db.services.count(chid))
		{
			entry = dbih->entries.services[chid];
			entry.prepend(idx);
			entry.prepend(x);
		}
		else
		{
			e2db::reference cref = dbih->userbouquets[chlist].channels[chid];

			if (cref.refmrker)
			{
				mrkr = true;
				entry = dbih->entry_marker(cref);
				idx = entry[1];
				entry.prepend(x);
			}
			else
			{
				//TEST
				entry = QStringList({x, "", "", qchid, "", "ERROR"});
				// idx = 0; //Qt5
				error("putChannels()", "chid", chid);
				//TEST
			}
		}
		QTreeWidgetItem* item = new QTreeWidgetItem(entry);
		item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsDragEnabled | Qt::ItemNeverHasChildren);
		item->setData(1, Qt::UserRole, mrkr); // data: marker flag
		clist.append(item);
		i++;
	}
	list_tree->addTopLevelItems(clist);

	lheaderv->setSectionsClickable(true);
	list_tree->setDragEnabled(true);
	list_tree->setAcceptDrops(true);
	this->_state_changed = true;
	updateListIndex();
	visualReindexList();
	setCounters();
}

void tab::updateListIndex()
{
	if (! this->_state_changed) return;

	int i = 0, y = 0, idx = 0;
	int count = list_tree->topLevelItemCount();
	string curr_chlist = this->_state_curr;
	dbih->gindex[curr_chlist].clear();

	debug("updateListIndex()", "curr_chlist", curr_chlist);

	do
	{
		QTreeWidgetItem* item = list_tree->topLevelItem(i);
		string chid = item->data(2, Qt::UserRole).toString().toStdString();
		bool mrkr = item->data(1, Qt::UserRole).toBool();
		if (mrkr)
		{
			idx = 0;
		}
		else
		{
			y++;
			idx = y;
		}
		dbih->gindex[curr_chlist].emplace_back(pair (idx, chid)); //C++ 17
		i++;
	}
	while (i != count);

	this->_state_changed = false;
}

void tab::showListEditContextMenu(QPoint &pos)
{
	debug("showListEditContextMenu()");

	QMenu* list_edit = new QMenu;
	list_edit->addAction("Edit Service", [=]() { this->editService(); });
	list_edit->addSeparator();
	list_edit->addAction("Cut", [=]() { this->listItemCut(); });
	list_edit->addAction("Copy", [=]() { this->listItemCopy(); });
	list_edit->addAction("Paste", [=]() { this->listItemPaste(); });
	list_edit->addSeparator();
	list_edit->addAction("Delete", [=]() { this->listItemDelete(); });

	list_edit->exec(list_tree->mapToGlobal(pos));
}

void tab::setCounters(bool channels)
{
	debug("setCounters()");

	int counters[5] = {0, 0, 0, 0, 0};

	if (channels)
	{
		string curr_chlist = this->_state_curr;
		counters[4] = dbih->gindex[curr_chlist].size();
	}
	else
	{
		counters[0] = dbih->gindex["chs:0"].size(); // data
		counters[1] = dbih->gindex["chs:1"].size(); // tv
		counters[2] = dbih->gindex["chs:2"].size(); // radio
		counters[3] = dbih->gindex["chs"].size();   // all
	}

	gid->loaded(counters);
}

void tab::setTabId(int ttid)
{
	debug("setTabId()", "ttid", to_string(ttid));

	this->ttid = ttid;
}

void tab::initialize()
{
	debug("initialize()");

	if (this->dbih != nullptr)
	  	delete this->dbih;

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
	this->_state_dnd = true;
	list_tree->scrollToItem(list_tree->topLevelItem(0));
	list_tree->clear();
	cache.clear();

	gid->reset();

	QTreeWidgetItem* titem = new QTreeWidgetItem();
	titem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
	QVariantMap tdata; //TODO
	tdata["id"] = "chs";
	titem->setData(0, Qt::UserRole, QVariant (tdata));
	titem->setText(0, "All channels");

	bouquets_tree->addTopLevelItem(titem);
}

void tab::destroy()
{
	debug("destroy()");

	delete this;
}

void tab::profileComboChanged(int index)
{
	debug("profileComboChanged()", "index", to_string(index));

	gid->sets->setValue("profile/selected", index);
}

//TEST
void tab::ftpConnect()
{
	using e2se_ftpcom::ftpcom;

	debug("ftpConnect()");

	int profile_sel = gid->sets->value("profile/selected").toInt();
	gid->sets->beginReadArray("profile");
	gid->sets->setArrayIndex(profile_sel);
	ftpcom::ftp_params params;
	params.host = gid->sets->value("ipAddress").toString().toStdString();
	params.port = gid->sets->value("ftpPort").toInt();
	params.user = gid->sets->value("username").toString().toStdString();
	params.pass = gid->sets->value("password").toString().toStdString();
	params.tpath = gid->sets->value("pathTransponders").toString().toStdString();
	params.spath = gid->sets->value("pathServices").toString().toStdString();
	params.bpath = gid->sets->value("pathBouquets").toString().toStdString();
	gid->sets->endArray();

	ftpcom* ftp = new ftpcom(params);
	ftp->connect();
	ftp->listDir(ftpcom::path_param::services);
	ftp->uploadData(ftpcom::path_param::services, "testfile", "test\ntest\n\n");
	ftp->disconnect();
}
//TEST

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
		std::filesystem::path path = cwd + "/seeds./enigma_db";
		readFile(std::filesystem::absolute(path).u8string());
	}
}
//TEST

}
