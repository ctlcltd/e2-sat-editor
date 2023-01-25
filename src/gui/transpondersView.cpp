/*!
 * e2-sat-editor/src/gui/transpondersView.cpp
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

#include <QTimer>
#include <QSettings>
#include <QGridLayout>
#include <QSplitter>
#include <QGroupBox>
#include <QHeaderView>
#include <QToolBar>
#include <QMenu>
#include <QScrollArea>
#include <QClipboard>
#include <QMimeData>

#include "platforms/platform.h"

#include "transpondersView.h"
#include "theme.h"
#include "tab.h"
#include "gui.h"
#include "editTransponder.h"

using std::to_string;

using namespace e2se;

namespace e2se_gui
{

transpondersView::transpondersView(tab* tid, QWidget* cwid, dataHandler* data, e2se::logger::session* log)
{
	this->log = new logger(log, "transpondersView");
	debug("transpondersView()");

	this->tid = tid;
	this->cwid = cwid;
	this->data = data;
	this->theme = new e2se_gui::theme;
	this->widget = new QWidget;

	layout();
}

void transpondersView::layout()
{
	debug("layout()");

	widget->setStyleSheet("QGroupBox { spacing: 0; border: 0; padding: 0; padding-top: 32px; font-weight: bold } QGroupBox::title { margin: 8px 4px; padding: 0 1px 1px }");

	QGridLayout* frm = new QGridLayout(widget);

	QVBoxLayout* abox = new QVBoxLayout;

	QGroupBox* afrm = new QGroupBox("Transponders");

	frm->setContentsMargins(0, 0, 0, 0);
	abox->setContentsMargins(0, 0, 0, 0);

	frm->setSpacing(0);
	abox->setSpacing(0);

	afrm->setFlat(true);

	this->list = new QTreeWidget;

	list->setStyleSheet("QTreeWidget::item { padding: 6px 0 }");

#ifdef Q_OS_MAC
	QColor itembackground;
	QString itembackground_hexArgb;

	itembackground = QColor(Qt::black);
	itembackground.setAlphaF(0.08);
	itembackground_hexArgb = itembackground.name(QColor::HexArgb);

	theme->dynamicStyleSheet(widget, "QTreeWidget::item:selected:!active { selection-background-color: " + itembackground_hexArgb + " }", theme::light);

	itembackground = QPalette().color(QPalette::Dark);
	itembackground.setAlphaF(0.15);
	itembackground_hexArgb = itembackground.name(QColor::HexArgb);

	theme->dynamicStyleSheet(widget, "QTreeWidget::item:selected:!active { selection-background-color: " + itembackground_hexArgb + " }", theme::dark);
#endif

	list->setUniformRowHeights(true);
	list->setRootIsDecorated(false);
	list->setSelectionBehavior(QAbstractItemView::SelectRows);
	list->setSelectionMode(QAbstractItemView::ExtendedSelection);
	list->setItemsExpandable(false);
	list->setExpandsOnDoubleClick(false);
	list->setDropIndicatorShown(true);
	list->setDragDropMode(QAbstractItemView::InternalMove);
	list->setEditTriggers(QAbstractItemView::NoEditTriggers);

	QTreeWidgetItem* list_thead = new QTreeWidgetItem({NULL, "TXID", "Combo", "System", "Position", "Frequency", "Polarization", "Symbol Rate", "FEC", "Modulation", "Bandwidth", "Pilot", "Roll offset", "Inversion", "Tmx Mode", "Guard", "Hierarchy"});
	list->setHeaderItem(list_thead);
	list->setColumnHidden(ITEM_ROW_ROLE::x, true);		// hidden index
	if (QSettings().value("application/debug", true).toBool()) {
		list->setColumnWidth(ITEM_ROW_ROLE::debug_txid, 175);
	}
	else
	{
		list->setColumnHidden(ITEM_ROW_ROLE::debug_txid, true);
	}
	list->setColumnWidth(ITEM_ROW_ROLE::combo, 175);	// combo (s: freq|pol|sr, t: freq|tmod|band, c: freq|cmod|sr)
	list->setColumnWidth(ITEM_ROW_ROLE::sys, 65);		// System
	list->setColumnWidth(ITEM_ROW_ROLE::pos, 75);		// Position
	list->setColumnWidth(ITEM_ROW_ROLE::freq, 85);		// Frequency
	list->setColumnWidth(ITEM_ROW_ROLE::pol, 85);		// Polarization
	list->setColumnWidth(ITEM_ROW_ROLE::sr, 85);		// Symbol Rate
	list->setColumnWidth(ITEM_ROW_ROLE::fec, 65);		// FEC
	list->setColumnWidth(ITEM_ROW_ROLE::mod, 75);		// Modulation | Constellation
	list->setColumnWidth(ITEM_ROW_ROLE::band, 85);		// Bandwidth
	list->setColumnWidth(ITEM_ROW_ROLE::pil, 75);		// Pilot
	list->setColumnWidth(ITEM_ROW_ROLE::rol, 70);		// Roll offset
	list->setColumnWidth(ITEM_ROW_ROLE::inv, 75);		// Inversion
	list->setColumnWidth(ITEM_ROW_ROLE::tmx, 65);		// Tmx Mode
	list->setColumnWidth(ITEM_ROW_ROLE::guard, 70);		// Guard
	list->setColumnWidth(ITEM_ROW_ROLE::hier, 70);		// Hierarchy

	list->header()->connect(list->header(), &QHeaderView::sectionClicked, [=](int column) { this->sortByColumn(column); });

	list->setContextMenuPolicy(Qt::CustomContextMenu);
	list->connect(list, &QTreeWidget::customContextMenuRequested, [=](QPoint pos) { this->showListEditContextMenu(pos); });

	searchLayout();

	QToolBar* list_ats = toolBar();

	this->action.list_newtx = toolBarAction(list_ats, "New Transponder", theme->dynamicIcon("add"), [=]() { this->addTransponder(); });
	toolBarSpacer(list_ats);
	this->action.list_search = toolBarButton(list_ats, "Find…", theme->dynamicIcon("search"), [=]() { this->listSearchToggle(); });

	this->action.list_search->setDisabled(true);

	this->list_evto = new ListEventObserver;
	list->installEventFilter(list_evto);
	list->connect(list, &QTreeWidget::currentItemChanged, [=]() { this->listItemChanged(); });
	list->connect(list, &QTreeWidget::itemSelectionChanged, [=]() { this->listItemSelectionChanged(); });
	list->connect(list, &QTreeWidget::itemDoubleClicked, [=]() { this->listItemDoubleClicked(); });

	abox->addWidget(list);
	abox->addWidget(list_search);
	abox->addWidget(list_ats);
	afrm->setLayout(abox);

	frm->addWidget(afrm);

	toolBarStyleSheet();
}

void transpondersView::searchLayout()
{
	this->viewAbstract::searchLayout();

	this->lsr_search.filter->addItem("Combo", ITEM_ROW_ROLE::combo);
	this->lsr_search.filter->addItem("Frequency", ITEM_ROW_ROLE::freq);
	this->lsr_search.filter->addItem("Polarization", ITEM_ROW_ROLE::pol);
	this->lsr_search.filter->addItem("Symbol Rate", ITEM_ROW_ROLE::sr);
	this->lsr_search.filter->addItem("Modulation", ITEM_ROW_ROLE::mod);
	this->lsr_search.filter->addItem("FEC", ITEM_ROW_ROLE::fec);
	this->lsr_search.filter->addItem("Position", ITEM_ROW_ROLE::pos);
	this->lsr_search.filter->addItem("System", ITEM_ROW_ROLE::sys);
}

void transpondersView::load()
{
	debug("load()");

	tabUpdateFlags(gui::init);

	populate();

	updateFlags();
	updateStatusBar();
}

void transpondersView::reset()
{
	debug("reset()");

	this->state.sort = pair (-1, Qt::AscendingOrder); //C++17

	list->reset();
	list->setDragEnabled(false);
	list->setAcceptDrops(false);
	list->clear();
	list->header()->setSortIndicatorShown(false);
	list->header()->setSectionsClickable(false);
	list->header()->setSortIndicator(0, Qt::AscendingOrder);

	this->lsr_find.curr = -1;
	this->lsr_find.match.clear();

	resetStatusBar();
}

void transpondersView::populate()
{
	auto* dbih = this->data->dbih;

	list->header()->setSortIndicatorShown(false);
	list->header()->setSectionsClickable(false);
	list->clear();

	QList<QTreeWidgetItem*> items;
	int i = 0;

	for (auto & tp : dbih->index["txs"])
	{
		e2db::transponder txp = dbih->db.transponders[tp.second];
		char ci[7];
		std::sprintf(ci, "%06d", i++);
		QString x = QString::fromStdString(ci);

		QString idx = QString::fromStdString(to_string(tp.first));
		QString txid = QString::fromStdString(txp.txid);
		QStringList entry = dbih->entryTransponder(txp, true);
		entry.prepend(x);
		entry.removeAt(5);

		QTreeWidgetItem* item = new QTreeWidgetItem(entry);
		item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsDragEnabled | Qt::ItemNeverHasChildren);
		item->setData(ITEM_DATA_ROLE::idx, Qt::UserRole, idx);
		item->setData(ITEM_DATA_ROLE::txid, Qt::UserRole, txid);

		items.append(item);
	}

	list->addTopLevelItems(items);

	list->setDragEnabled(true);
	list->setAcceptDrops(true);
	list->header()->setSectionsClickable(true);

	// sorting default column 0|asc
	list->sortItems(0, Qt::AscendingOrder);
	list->header()->setSortIndicator(1, Qt::AscendingOrder);
}

void transpondersView::listItemChanged()
{
	// debug("listItemChanged()");

	if (list_evto->isChanged())
		listUpdate();
}

void transpondersView::listItemSelectionChanged()
{
	// debug("listItemSelectionChanged()");

	QList<QTreeWidgetItem*> selected = list->selectedItems();

	if (selected.empty())
	{
		tabSetFlag(gui::TabListCut, false);
		tabSetFlag(gui::TabListCopy, false);
		tabSetFlag(gui::TabListDelete, false);
	}
	else
	{
		tabSetFlag(gui::TabListCut, true);
		tabSetFlag(gui::TabListCopy, true);
		tabSetFlag(gui::TabListDelete, true);
	}
	if (selected.count() == 1)
	{
		tabSetFlag(gui::TabListEditService, true);
	}
	else
	{
		tabSetFlag(gui::TabListEditService, false);
	}
}

void transpondersView::listItemDoubleClicked()
{
	debug("listItemDoubleClicked()");

	QList<QTreeWidgetItem*> selected = list->selectedItems();

	if (selected.empty() || selected.count() > 1)
		return;

	editTransponder();
}

void transpondersView::listUpdate()
{
	debug("listUpdate()");

	updateListIndex();

	this->data->setChanged(true);
}

void transpondersView::addTransponder()
{
	debug("addTransponder()");

	auto* dbih = this->data->dbih;

	string txid;
	e2se_gui::editTransponder* add = new e2se_gui::editTransponder(this->data, this->log->log);
	add->display(cwid);
	txid = add->getAddId(); // returned after dial.exec()
	add->destroy();

	if (dbih->db.transponders.count(txid))
		debug("addTransponder()", "txid", txid);
	else
		return error("addTransponder()", "txid", txid);

	list->header()->setSectionsClickable(false);
	list->setDragEnabled(false);
	list->setAcceptDrops(false);

	int i = 0, y;
	QTreeWidgetItem* current = list->currentItem();
	QTreeWidgetItem* parent = list->invisibleRootItem();
	i = current != nullptr ? parent->indexOfChild(current) : list->topLevelItemCount();
	y = i + 1;

	e2db::transponder txp = dbih->db.transponders[txid];

	char ci[7];
	std::sprintf(ci, "%06d", i++);
	QString x = QString::fromStdString(ci);
	QString idx = QString::fromStdString(to_string(txp.index));
	QStringList entry = dbih->entryTransponder(txp, true);
	entry.prepend(x);
	entry.removeAt(5);

	QTreeWidgetItem* item = new QTreeWidgetItem(entry);
	item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsDragEnabled | Qt::ItemNeverHasChildren);
	item->setData(ITEM_DATA_ROLE::idx, Qt::UserRole, idx);
	item->setData(ITEM_DATA_ROLE::txid, Qt::UserRole, QString::fromStdString(txid));

	if (current == nullptr)
		list->addTopLevelItem(item);
	else
		list->insertTopLevelItem(y, item);

	list->header()->setSectionsClickable(true);
	list->setDragEnabled(true);
	list->setAcceptDrops(true);

	updateListIndex();

	updateFlags();
	updateStatusBar();

	this->data->setChanged(true);
}

void transpondersView::editTransponder()
{
	debug("editTransponder()");

	QList<QTreeWidgetItem*> selected = list->selectedItems();

	if (selected.empty() || selected.count() > 1)
		return;

	QTreeWidgetItem* item = selected.first();
	string txid = item->data(ITEM_DATA_ROLE::txid, Qt::UserRole).toString().toStdString();
	string nw_txid;

	auto* dbih = this->data->dbih;

	if (dbih->db.transponders.count(txid))
		debug("editTransponder()", "txid", txid);
	else
		return error("editTransponder()", "txid", txid);

	e2se_gui::editTransponder* edit = new e2se_gui::editTransponder(this->data, this->log->log);
	edit->setEditId(txid);
	edit->display(cwid);
	nw_txid = edit->getEditId(); // returned after dial.exec()
	edit->destroy();

	if (dbih->db.transponders.count(nw_txid))
		debug("editTransponder()", "new txid", nw_txid);
	else
		return error("editTransponder()", "new txid", nw_txid);

	e2db::transponder txp = dbih->db.transponders[nw_txid];

	QStringList entry = dbih->entryTransponder(txp, true);
	entry.prepend(item->text(ITEM_ROW_ROLE::x));
	entry.removeAt(5);
	for (int i = 0; i < entry.count(); i++)
		item->setText(i, entry[i]);
	item->setData(ITEM_DATA_ROLE::txid, Qt::UserRole, QString::fromStdString(nw_txid));

	updateListIndex();

	this->data->setChanged(true);
}

void transpondersView::listItemCopy(bool cut)
{
	debug("listItemCopy()");

	QList<QTreeWidgetItem*> selected = list->selectedItems();

	if (selected.empty())
		return;

	QClipboard* clipboard = QGuiApplication::clipboard();
	QStringList text;
	for (auto & item : selected)
	{
		QString txid = item->data(ITEM_DATA_ROLE::txid, Qt::UserRole).toString();

		QStringList data;
		// start from sys column [2]
		for (int i = ITEM_ROW_ROLE::sys; i < list->columnCount(); i++)
		{
			QString qstr = item->data(i, Qt::DisplayRole).toString();
			data.append(qstr);
		}

		data.prepend(txid); // insert txid column [0]
		text.append(data.join(",")); // CSV
	}
	clipboard->setText(text.join("\n")); // CSV

	if (cut)
		listItemDelete();
}

void transpondersView::listItemPaste()
{
	debug("listItemPaste()");

	QClipboard* clipboard = QGuiApplication::clipboard();
	const QMimeData* mimeData = clipboard->mimeData();
	vector<QString> items;

	if (mimeData->hasText())
	{
		QStringList list = clipboard->text().split("\n");

		for (QString & data : list)
		{
			items.emplace_back(data);
		}
	}
	if (! items.empty())
	{
		putListItems(items);

		if (list->currentItem() == nullptr)
			list->scrollToBottom();
	}

	this->data->setChanged(true);
}

void transpondersView::listItemDelete()
{
	debug("listItemDelete()");

	QList<QTreeWidgetItem*> selected = list->selectedItems();

	if (selected.empty())
		return;

	bool remove = tabRemoveQuestion("Confirm deletetion", "Do you want to delete items?");
	if (! remove)
		return;

	list->header()->setSectionsClickable(false);
	list->setDragEnabled(false);
	list->setAcceptDrops(false);

	auto* dbih = this->data->dbih;

	for (auto & item : selected)
	{
		int i = list->indexOfTopLevelItem(item);
		string txid = item->data(ITEM_DATA_ROLE::txid, Qt::UserRole).toString().toStdString();
		list->takeTopLevelItem(i);

		dbih->removeTransponder(txid);
	}

	list->header()->setSectionsClickable(true);
	list->setDragEnabled(true);
	list->setAcceptDrops(true);

	updateListIndex();

	updateFlags();
	updateStatusBar();

	this->data->setChanged(true);
}

//TODO improve
//TODO duplicates
void transpondersView::putListItems(vector<QString> items)
{
	debug("putListItems()");

	list->header()->setSectionsClickable(false);
	list->setDragEnabled(false);
	list->setAcceptDrops(false);

	QList<QTreeWidgetItem*> clist;
	int i = 0, y;
	QTreeWidgetItem* current = list->currentItem();
	QTreeWidgetItem* parent = list->invisibleRootItem();
	i = current != nullptr ? parent->indexOfChild(current) : list->topLevelItemCount();
	y = i + 1;

	auto* dbih = this->data->dbih;

	for (QString & q : items)
	{
		char ci[7];
		std::sprintf(ci, "%06d", i++);
		QString x = QString::fromStdString(ci);
		QString idx = QString::fromStdString(to_string(i));

		string txid;
		e2db::transponder txp;

		if (q.contains(','))
		{
			auto qs = q.split(',');
			txid = txp.txid = qs[0].toStdString();
			txp.freq = qs[1].toInt();

			/*switch (this->state.yx)
			{
				case e2db::YTYPE::satellite:
					txp.pol = dbih->value_transponder_polarization(qs[2].toStdString());
					txp.sr = qs[3].toInt();
					txp.fec = dbih->value_transponder_fec(qs[4].toStdString(), e2db::YTYPE::satellite);
					txp.sys = dbih->value_transponder_system(qs[5].toStdString());
					txp.mod = dbih->value_transponder_modulation(qs[6].toStdString(), e2db::YTYPE::satellite);
					txp.inv = dbih->value_transponder_inversion(qs[7].toStdString(), e2db::YTYPE::satellite);
					txp.rol = dbih->value_transponder_rollof(qs[8].toStdString());
					txp.pil = dbih->value_transponder_pilot(qs[9].toStdString());
				break;
				case e2db::YTYPE::terrestrial:
					txp.tmod = dbih->value_transponder_modulation(qs[2].toStdString(), e2db::YTYPE::terrestrial);
					txp.band = dbih->value_transponder_bandwidth(qs[3].toStdString());
					txp.sys = dbih->value_transponder_system(qs[4].toStdString());
					txp.tmx = dbih->value_transponder_tmx_mode(qs[5].toStdString());
					txp.hpfec = dbih->value_transponder_fec(qs[6].toStdString(), e2db::YTYPE::terrestrial);
					txp.lpfec = dbih->value_transponder_fec(qs[7].toStdString(), e2db::YTYPE::terrestrial);
					txp.inv = dbih->value_transponder_inversion(qs[8].toStdString(), e2db::YTYPE::terrestrial);
					txp.guard = dbih->value_transponder_guard(qs[9].toStdString());
					txp.hier = dbih->value_transponder_hier(qs[10].toStdString());
				break;
				case e2db::YTYPE::cable:
					txp.cmod = dbih->value_transponder_modulation(qs[2].toStdString(), e2db::YTYPE::cable);
					txp.sr = qs[3].toInt();
					txp.cfec = dbih->value_transponder_fec(qs[4].toStdString(), e2db::YTYPE::cable);
					txp.inv = dbih->value_transponder_inversion(qs[5].toStdString(), e2db::YTYPE::cable);
					txp.sys = dbih->value_transponder_system(qs[6].toStdString());
				break;
				case e2db::YTYPE::atsc:
					txp.amod = dbih->value_transponder_modulation(qs[2].toStdString(), e2db::YTYPE::atsc);
					txp.sys = dbih->value_transponder_system(qs[3].toStdString());
				break;
			}*/
		}
		else
		{
			txid = q.toStdString();
		}

		QStringList entry;

		if (dbih->db.transponders.count(txid))
		{
			txp = dbih->db.transponders[txid];
		}
		entry = dbih->entryTransponder(txp);
		entry.prepend(x);

		QTreeWidgetItem* item = new QTreeWidgetItem(entry);
		item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsDragEnabled | Qt::ItemNeverHasChildren);
		item->setData(ITEM_DATA_ROLE::idx, Qt::UserRole, idx);
		item->setData(ITEM_DATA_ROLE::txid, Qt::UserRole, QString::fromStdString(txid));
		clist.append(item);

		dbih->addTransponder(txp);
	}

	if (current == nullptr)
		list->addTopLevelItems(clist);
	else
		list->insertTopLevelItems(y, clist);

	list->header()->setSectionsClickable(true);
	list->setDragEnabled(true);
	list->setAcceptDrops(true);

	updateListIndex();

	updateFlags();
	updateStatusBar();

	this->data->setChanged(true);
}

void transpondersView::updateStatusBar(bool current)
{
	debug("updateStatusBar()");

	gui::status msg;
	msg.update = current;

	auto* dbih = this->data->dbih;

	msg.counters[gui::COUNTER::n_transponders] += dbih->index["txs"].size();

	tabSetStatusBar(msg);
}

void transpondersView::showListEditContextMenu(QPoint &pos)
{
	debug("showListEditContextMenu()");

	QList<QTreeWidgetItem*> selected = list->selectedItems();

	if (selected.empty())
		return;

	bool editable = false;

	if (selected.count() == 1)
	{
		editable = true;
	}

	QMenu* list_edit = contextMenu();

	contextMenuAction(list_edit, "Edit Transponder", [=]() { this->editTransponder(); }, editable && tabGetFlag(gui::TabListEditTransponder));
	contextMenuSeparator(list_edit);
	contextMenuAction(list_edit, "Cu&t", [=]() { this->listItemCut(); }, tabGetFlag(gui::TabListCut), QKeySequence::Cut);
	contextMenuAction(list_edit, "&Copy", [=]() { this->listItemCopy(); }, tabGetFlag(gui::TabListCopy), QKeySequence::Copy);
	contextMenuAction(list_edit, "&Paste", [=]() { this->listItemPaste(); }, tabGetFlag(gui::TabListPaste), QKeySequence::Paste);
	contextMenuSeparator(list_edit);
	contextMenuAction(list_edit, "&Delete", [=]() { this->listItemDelete(); }, tabGetFlag(gui::TabListDelete), QKeySequence::Delete);

	platform::osContextMenuPopup(list_edit, list, pos);
	// list_edit->exec(list->mapToGlobal(pos));
}

void transpondersView::updateFlags()
{
	debug("updateFlags()");

	tabSetFlag(gui::TabTreeEdit, false);
	tabSetFlag(gui::TabTreeDelete, false);
	tabSetFlag(gui::TabTreeFind, false);

	if (list->topLevelItemCount())
	{
		tabSetFlag(gui::TabListEditTransponder, true);
		tabSetFlag(gui::TabListSelectAll, true);
		tabSetFlag(gui::TabListFind, true);
		this->action.list_search->setEnabled(true);
	}
	else
	{
		tabSetFlag(gui::TabListEditTransponder, false);
		tabSetFlag(gui::TabListSelectAll, false);
		tabSetFlag(gui::TabListFind, false);
		this->action.list_search->setDisabled(true);
	}

	auto* dbih = this->data->dbih;

	if (dbih->index.count("chs"))
	{
		tabSetFlag(gui::OpenChannelBook, true);
	}
	else
	{
		tabSetFlag(gui::OpenChannelBook, false);
	}

	tabSetFlag(gui::TabTreeFindNext, false);
	tabSetFlag(gui::TabListFindNext, false);
	tabSetFlag(gui::TabListFindPrev, false);
	tabSetFlag(gui::TabListFindAll, false);

	tabSetFlag(gui::TunersetsSat, true);
	tabSetFlag(gui::TunersetsTerrestrial, true);
	tabSetFlag(gui::TunersetsCable, true);
	tabSetFlag(gui::TunersetsAtsc, true);

	tabUpdateFlags();
}

void transpondersView::updateListIndex()
{
	int i = 0, idx = 0;
	int count = list->topLevelItemCount();

	auto* dbih = this->data->dbih;

	dbih->index["txs"].clear();

	debug("updateListIndex()");

	int sort_col = list->sortColumn();
	list->sortItems(0, Qt::AscendingOrder);

	while (i != count)
	{
		QTreeWidgetItem* item = list->topLevelItem(i);
		string txid = item->data(ITEM_DATA_ROLE::txid, Qt::UserRole).toString().toStdString();
		idx = i + 1;
		dbih->index["txs"].emplace_back(pair (idx, txid)); //C++17
		i++;
	}

	list->sortItems(this->state.sort.first, this->state.sort.second);
	list->header()->setSortIndicator(sort_col, this->state.sort.second);

	this->state.txx_pending = false;
}

void transpondersView::updateIndex()
{
	updateListIndex();
}

}
