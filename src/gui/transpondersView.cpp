/*!
 * e2-sat-editor/src/gui/transpondersView.cpp
 *
 * @link https://github.com/ctlcltd/e2-sat-editor
 * @copyright e2 SAT Editor Team
 * @author Leonardo Laureti
 * @version 0.5
 * @license MIT License
 * @license GNU GPLv3 License
 */

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

using namespace e2se;

namespace e2se_gui
{

transpondersView::transpondersView(tab* tid, QWidget* cwid, dataHandler* data)
{
	this->log = new logger("gui", "transpondersView");

	this->tid = tid;
	this->cwid = cwid;
	this->data = data;
	this->theme = new e2se_gui::theme;
	this->widget = new QWidget;

	layout();
}

transpondersView::~transpondersView()
{
	debug("~transpondersView");

	delete this->widget;
	delete this->theme;
	delete this->log;
}

void transpondersView::layout()
{
	debug("layout");

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

	QTreeWidgetItem* list_thead = new QTreeWidgetItem({NULL, "TXID", "Combo", "System", "Position", "Transport ID", "DVBNS", "ONID", "Frequency", "Polarization", "Symbol Rate", "FEC", "Modulation", "Bandwidth", "Pilot", "Roll offset", "Inversion", "Tmx Mode", "Guard", "Hierarchy"});
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
	list->setColumnWidth(ITEM_ROW_ROLE::sys, 75);		// System
	list->setColumnWidth(ITEM_ROW_ROLE::pos, 80);		// Position
	list->setColumnWidth(ITEM_ROW_ROLE::tsid, 80);		// Transport ID
	list->setColumnWidth(ITEM_ROW_ROLE::dvbns, 95);		// DVBNS
	list->setColumnWidth(ITEM_ROW_ROLE::onid, 85);		// ONID
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
	debug("load");

	tabUpdateFlags(gui::init);

	populate();

	updateFlags();
	updateStatusBar();
}

void transpondersView::reset()
{
	debug("reset");

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
	size_t pad_width = 4;

	if (! dbih->index.count("txs"))
		error("populate", "current", "Missing index key \"txs\".");

	for (auto & tp : dbih->index["txs"])
	{
		e2db::transponder txp = dbih->db.transponders[tp.second];

		QString x = QString::number(i++).rightJustified(pad_width, '0');
		QString idx = QString::number(tp.first);
		QString txid = QString::fromStdString(txp.txid);
		QStringList entry = dbih->entryTransponder(txp, true);
		entry.prepend(x);
		entry.removeAt(8);

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
	// debug("listItemChanged");

	if (list_evto->isChanged())
		listUpdate();
}

void transpondersView::listItemSelectionChanged()
{
	// debug("listItemSelectionChanged");

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

	if (QGuiApplication::clipboard()->text().isEmpty())
		tabSetFlag(gui::TabListPaste, false);
	else
		tabSetFlag(gui::TabListPaste, true);

	tabUpdateFlags();
}

void transpondersView::listItemDoubleClicked()
{
	debug("listItemDoubleClicked");

	QList<QTreeWidgetItem*> selected = list->selectedItems();

	if (selected.empty() || selected.count() > 1)
		return;

	editTransponder();
}

void transpondersView::listUpdate()
{
	debug("listUpdate");

	updateListIndex();

	this->data->setChanged(true);
}

void transpondersView::addTransponder()
{
	debug("addTransponder");

	auto* dbih = this->data->dbih;

	string txid;
	e2se_gui::editTransponder* add = new e2se_gui::editTransponder(this->data);
	add->display(cwid);
	txid = add->getAddId();
	add->destroy();

	if (dbih->db.transponders.count(txid))
		debug("addTransponder", "txid", txid);
	else
		return error("addTransponder", "Error", "Missing transponder key \"" + txid + "\".");

	list->header()->setSectionsClickable(false);
	list->setDragEnabled(false);
	list->setAcceptDrops(false);

	int i = 0, y;
	QTreeWidgetItem* current = list->currentItem();
	QTreeWidgetItem* parent = list->invisibleRootItem();
	i = current != nullptr ? parent->indexOfChild(current) : list->topLevelItemCount();
	y = i + 1;

	e2db::transponder txp = dbih->db.transponders[txid];

	size_t pad_width = 4;
	QString x = QString::number(i++).rightJustified(pad_width, '0');
	QString idx = QString::number(i);
	QStringList entry = dbih->entryTransponder(txp, true);
	entry.prepend(x);
	entry.removeAt(8);

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
	debug("editTransponder");

	QList<QTreeWidgetItem*> selected = list->selectedItems();

	if (selected.empty() || selected.count() > 1)
		return;

	QTreeWidgetItem* item = selected.first();
	string txid = item->data(ITEM_DATA_ROLE::txid, Qt::UserRole).toString().toStdString();
	string nw_txid;

	auto* dbih = this->data->dbih;

	if (dbih->db.transponders.count(txid))
		debug("editTransponder", "txid", txid);
	else
		return error("editTransponder", "txid", "Transponder \"" + txid + "\" not exists.");

	e2se_gui::editTransponder* edit = new e2se_gui::editTransponder(this->data);
	edit->setEditId(txid);
	edit->display(cwid);
	nw_txid = edit->getEditId();
	edit->destroy();

	if (dbih->db.transponders.count(nw_txid))
		debug("editTransponder", "new txid", nw_txid);
	else
		return error("editTransponder", "new txid", "Missing transponder key \"" + nw_txid + "\".");

	e2db::transponder txp = dbih->db.transponders[nw_txid];

	QStringList entry = dbih->entryTransponder(txp, true);
	entry.prepend(item->text(ITEM_ROW_ROLE::x));
	entry.removeAt(8);
	for (int i = 0; i < entry.count(); i++)
		item->setText(i, entry[i]);
	item->setData(ITEM_DATA_ROLE::txid, Qt::UserRole, QString::fromStdString(nw_txid));

	updateListIndex();

	this->data->setChanged(true);
}

void transpondersView::listItemCopy(bool cut)
{
	debug("listItemCopy");

	QList<QTreeWidgetItem*> selected = list->selectedItems();

	if (selected.empty())
		return;

	QClipboard* clipboard = QGuiApplication::clipboard();
	QStringList text;
	for (auto & item : selected)
	{
		QString txid = item->data(ITEM_DATA_ROLE::txid, Qt::UserRole).toString();

		QStringList data;
		// start from combo column [2]
		for (int i = ITEM_ROW_ROLE::combo; i < list->columnCount(); i++)
		{
			QString qstr = item->data(i, Qt::DisplayRole).toString();
			data.append(qstr);
		}

		text.append(data.join(",")); // CSV
	}
	clipboard->setText(text.join("\n")); // CSV

	if (cut)
		listItemDelete();

	tabSetFlag(gui::TabListPaste, true);
}

void transpondersView::listItemPaste()
{
	debug("listItemPaste");

	QClipboard* clipboard = QGuiApplication::clipboard();
	const QMimeData* mimeData = clipboard->mimeData();
	vector<QString> items;
	int commas = 17;

	if (mimeData->hasText())
	{
		QStringList list = clipboard->text().split("\n");

		for (QString & data : list)
		{
			if (data.count(',') == commas)
				items.emplace_back(data);
			else
				return;
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
	debug("listItemDelete");

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
	debug("putListItems");

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
	size_t pad_width = 4;

	for (QString & q : items)
	{
		QString x = QString::number(i++).rightJustified(pad_width, '0');
		QString idx = QString::number(i);

		e2db::transponder tx;

		if (q.contains(','))
		{
			auto qs = q.split(',');
			e2db::fec fec;
			tx.ytype = dbih->value_transponder_type(qs[1].toStdString());
			tx.pos = dbih->value_transponder_position(qs[2].toStdString());
			tx.tsid = qs[3].toInt();
			tx.dvbns = dbih->value_transponder_dvbns(qs[4].toStdString());
			tx.onid = qs[5].toInt();
			tx.freq = qs[6].toInt();
			dbih->value_transponder_fec(qs[9].toStdString(), tx.ytype, fec);

			switch (tx.ytype)
			{
				case e2db::YTYPE::satellite:
					tx.pol = dbih->value_transponder_polarization(qs[7].toStdString());
					tx.sr = qs[8].toInt();
					tx.fec = fec.inner_fec;
					tx.sys = dbih->value_transponder_system(qs[1].toStdString());
					tx.mod = dbih->value_transponder_modulation(qs[10].toStdString(), e2db::YTYPE::satellite);
					tx.inv = dbih->value_transponder_inversion(qs[14].toStdString(), e2db::YTYPE::satellite);
					tx.rol = dbih->value_transponder_rollof(qs[13].toStdString());
					tx.pil = dbih->value_transponder_pilot(qs[12].toStdString());
				break;
				case e2db::YTYPE::terrestrial:
					tx.tmod = dbih->value_transponder_modulation(qs[10].toStdString(), e2db::YTYPE::terrestrial);
					tx.band = dbih->value_transponder_bandwidth(qs[11].toStdString());
					tx.sys = dbih->value_transponder_system(qs[1].toStdString());
					tx.tmx = dbih->value_transponder_tmx_mode(qs[15].toStdString());
					tx.hpfec = fec.hp_fec;
					tx.lpfec = fec.lp_fec;
					tx.inv = dbih->value_transponder_inversion(qs[14].toStdString(), e2db::YTYPE::terrestrial);
					tx.guard = dbih->value_transponder_guard(qs[16].toStdString());
					tx.hier = dbih->value_transponder_hier(qs[17].toStdString());
				break;
				case e2db::YTYPE::cable:
					tx.cmod = dbih->value_transponder_modulation(qs[10].toStdString(), e2db::YTYPE::cable);
					tx.sr = qs[8].toInt();
					tx.fec = fec.inner_fec;
					tx.inv = dbih->value_transponder_inversion(qs[14].toStdString(), e2db::YTYPE::cable);
				break;
				case e2db::YTYPE::atsc:
					tx.amod = dbih->value_transponder_modulation(qs[2].toStdString(), e2db::YTYPE::atsc);
					tx.sys = dbih->value_transponder_system(qs[1].toStdString());
				break;
			}

			char txid[25];
			// %4x:%8x
			std::snprintf(txid, 25, "%x:%x", tx.tsid, tx.dvbns);
			tx.txid = txid;
		}
		else
		{
			tx.txid = q.toStdString();
		}

		QStringList entry;

		if (dbih->db.transponders.count(tx.txid))
		{
			tx = dbih->db.transponders[tx.txid];
		}
		entry = dbih->entryTransponder(tx);
		entry.prepend(x);

		QTreeWidgetItem* item = new QTreeWidgetItem(entry);
		item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsDragEnabled | Qt::ItemNeverHasChildren);
		item->setData(ITEM_DATA_ROLE::idx, Qt::UserRole, idx);
		item->setData(ITEM_DATA_ROLE::txid, Qt::UserRole, QString::fromStdString(tx.txid));
		clist.append(item);

		dbih->addTransponder(tx);
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
	debug("updateStatusBar");

	gui::status msg;
	msg.update = current;

	auto* dbih = this->data->dbih;

	msg.counters[gui::COUNTER::n_transponders] += dbih->index["txs"].size();

	tabSetStatusBar(msg);
}

void transpondersView::showListEditContextMenu(QPoint &pos)
{
	debug("showListEditContextMenu");

	QList<QTreeWidgetItem*> selected = list->selectedItems();

	if (selected.empty() && list->topLevelItemCount() != 0)
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
	debug("updateFlags");

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

	if (QGuiApplication::clipboard()->text().isEmpty())
		tabSetFlag(gui::TabListPaste, false);
	else
		tabSetFlag(gui::TabListPaste, true);

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

	debug("updateListIndex");

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
