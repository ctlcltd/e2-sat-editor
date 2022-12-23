/*!
 * e2-sat-editor/src/gui/tunersetsView.cpp
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

#include <QTimer>
#include <QGridLayout>
#include <QSplitter>
#include <QGroupBox>
#include <QHeaderView>
#include <QToolBar>
#include <QMenu>
#include <QScrollArea>
#include <QClipboard>
#include <QMimeData>

#include "tunersetsView.h"
#include "theme.h"
#include "tab.h"
#include "gui.h"
#include "editTunersets.h"
#include "editTunersetsTable.h"
#include "editTunersetsTransponder.h"

using std::to_string;
using namespace e2se;

namespace e2se_gui
{

tunersetsView::tunersetsView(tab* tid, QWidget* cwid, dataHandler* data, int ytype, e2se::logger::session* log)
{
	this->log = new logger(log, "tunersetsView");
	debug("tunersetsView()");

	this->tid = tid;
	this->cwid = cwid;
	this->data = data;
	this->yx = ytype;
	this->sets = new QSettings;
	this->widget = new QWidget;

	layout();
}

void tunersetsView::layout()
{
	debug("layout()");

	QGridLayout* frm = new QGridLayout(widget);

	QSplitter* swid = new QSplitter;
	QGroupBox* tfrm = new QGroupBox;
	QGroupBox* lfrm = new QGroupBox;
	this->tbox = new QVBoxLayout;
	this->lbox = new QVBoxLayout;
	tbox->setSpacing(0);
	lbox->setSpacing(0);
	tfrm->setFlat(true);
	lfrm->setFlat(true);

	switch (yx)
	{
		case e2db::YTYPE::sat:
			tfrm->setTitle("Satellites");
		break;
		case e2db::YTYPE::terrestrial:
		case e2db::YTYPE::cable:
		case e2db::YTYPE::atsc:
			tfrm->setTitle("Positions");
		break;
		default:
			error("tunersets()", "Error", "Not supported yet.");
	}
	lfrm->setTitle("Transponders");

	tfrm->setLayout(tbox);
	lfrm->setLayout(lbox);

	QStringList ths, lhs;
	switch (yx)
	{
		case e2db::YTYPE::sat:
			ths = QStringList ({NULL, "Name", "Position"});
			lhs = QStringList ({NULL, "TRID", "Freq/Pol/SR", "Frequency", "Polarization", "Symbol Rate", "FEC", "System", "Modulation", "Inversion", "Pilot", "Roll offset"});
		break;
		case e2db::YTYPE::terrestrial:
			ths = QStringList ({NULL, "Name", "Country"});
			lhs = QStringList ({NULL, "TRID", "Freq/Const/Band", "Frequency", "Constellation", "Bandwidth", "System", "Tx Mode", "HP FEC", "LP FEC", "Inversion", "Guard", "Hierarchy"});
		break;
		case e2db::YTYPE::cable:
			ths = QStringList ({NULL, "Name", "Country"});
			lhs = QStringList ({NULL, "TRID", "Freq/Mod/SR", "Frequency", "Modulation", "Symbol Rate", "FEC", "Inversion", "System"});
		break;
		case e2db::YTYPE::atsc:
			ths = QStringList ({NULL, "Name"});
			lhs = QStringList ({NULL, "TRID", NULL, "Frequency", "Modulation", "System"});
		break;
	}

	this->tree = new QTreeWidget;
	tree->setMinimumWidth(240);
	tree->setUniformRowHeights(true);
	tree->setRootIsDecorated(false);
	tree->setSelectionBehavior(QAbstractItemView::SelectRows);
	tree->setSelectionMode(QAbstractItemView::ExtendedSelection);
	tree->setItemsExpandable(false);
	tree->setExpandsOnDoubleClick(false);
	tree->setDropIndicatorShown(true);
	// tree->setDragDropMode(QAbstractItemView::DragDrop);
	tree->setDragDropMode(QAbstractItemView::InternalMove);
	tree->setEditTriggers(QAbstractItemView::NoEditTriggers);
	tree->setStyleSheet("::item { padding: 6px auto }");

	QTreeWidgetItem* tree_thead = new QTreeWidgetItem(ths);
	tree->setHeaderItem(tree_thead);
	tree->setColumnHidden(TREE_ROW_ROLE::tnid, true); // hidden tnid
	tree->setColumnWidth(TREE_ROW_ROLE::trow1, 200);  // Name
	tree->setColumnWidth(TREE_ROW_ROLE::trow2, 75);   // Position | Country

	this->list = new QTreeWidget;
	list->setUniformRowHeights(true);
	list->setRootIsDecorated(false);
	list->setSelectionBehavior(QAbstractItemView::SelectRows);
	list->setSelectionMode(QAbstractItemView::ExtendedSelection);
	list->setItemsExpandable(false);
	list->setExpandsOnDoubleClick(false);
	list->setDropIndicatorShown(true);
	list->setDragDropMode(QAbstractItemView::InternalMove);
	list->setEditTriggers(QAbstractItemView::NoEditTriggers);
	list->setStyleSheet("::item { padding: 6px auto }");

	QTreeWidgetItem* list_thead = new QTreeWidgetItem(lhs);
	list->setHeaderItem(list_thead);
	list->setColumnHidden(ITEM_ROW_ROLE::x, true);		// hidden index
	if (sets->value("application/debug", true).toBool()) {
		list->setColumnWidth(ITEM_ROW_ROLE::debug_trid, 175);
	}
	else
	{
		list->setColumnHidden(ITEM_ROW_ROLE::debug_trid, true);
	}
	list->setColumnWidth(ITEM_ROW_ROLE::combo, 175);	// combo (s: freq|pol|sr, t: freq|tmod|band, c: freq|cmod|sr)
	list->setColumnHidden(ITEM_ROW_ROLE::combo, true);
	list->setColumnWidth(ITEM_ROW_ROLE::row3, 85);		// Frequency
	list->setColumnWidth(ITEM_ROW_ROLE::row4, 85);		// Polarization | Constellation | Modulation
	list->setColumnWidth(ITEM_ROW_ROLE::row5, 85);		// Symbol Rate | Bandwidth
	list->setColumnWidth(ITEM_ROW_ROLE::row6, 65);		// FEC | System
	list->setColumnWidth(ITEM_ROW_ROLE::row7, 65);		// System | Tx Mode
	list->setColumnWidth(ITEM_ROW_ROLE::row8, 75);		// Modulation | HP FEC | System
	list->setColumnWidth(ITEM_ROW_ROLE::row9, 75);		// Inversion | LP FEC
	list->setColumnWidth(ITEM_ROW_ROLE::rowA, 75);		// Pilot | Inversion
	list->setColumnWidth(ITEM_ROW_ROLE::rowB, 70);		// Roll offset | Guard
	list->setColumnWidth(ITEM_ROW_ROLE::rowC, 70);		// Hierarchy

	tree->connect(tree, &QTreeWidget::currentItemChanged, [=]() { this->treeItemChanged(); });
	list->header()->connect(list->header(), &QHeaderView::sectionClicked, [=](int column) { this->sortByColumn(column); });

	tree->setContextMenuPolicy(Qt::CustomContextMenu);
	tree->connect(tree, &QTreeWidget::customContextMenuRequested, [=](QPoint pos) { this->showTreeEditContextMenu(pos); });
	list->setContextMenuPolicy(Qt::CustomContextMenu);
	list->connect(list, &QTreeWidget::customContextMenuRequested, [=](QPoint pos) { this->showListEditContextMenu(pos); });

	searchLayout();

	QToolBar* tree_ats = new QToolBar;
	tree_ats->setIconSize(QSize(12, 12));
	tree_ats->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
	tree_ats->setStyleSheet("QToolButton { font: bold 14px }");
	QToolBar* list_ats = new QToolBar;
	list_ats->setIconSize(QSize(12, 12));
	list_ats->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
	list_ats->setStyleSheet("QToolButton { font: bold 14px }");

	this->action.tree_search = new QPushButton;
	this->action.tree_search->setText("Find…");
	this->action.tree_search->setIcon(theme::icon("search"));
	this->action.tree_search->connect(this->action.tree_search, &QPushButton::pressed, [=]() { this->treeSearchToggle(); });

	this->action.list_search = new QPushButton;
	this->action.list_search->setText("&Find…");
	this->action.list_search->setIcon(theme::icon("search"));
	this->action.list_search->connect(this->action.list_search, &QPushButton::pressed, [=]() { this->listSearchToggle(); });

	QWidget* tree_ats_spacer = new QWidget;
	tree_ats_spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	QWidget* list_ats_spacer = new QWidget;
	list_ats_spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

	tree_ats->addAction(theme::icon("add"), "New Position", [=]() { this->addPosition(); });
	tree_ats->addWidget(tree_ats_spacer);
	tree_ats->addWidget(this->action.tree_search);
	this->action.list_newtr = list_ats->addAction(theme::icon("add"), "New Transponder", [=]() { this->addTransponder(); });
	list_ats->addWidget(list_ats_spacer);
	list_ats->addWidget(this->action.list_search);

	list->connect(list, &QTreeWidget::itemDoubleClicked, [=]() { this->listItemDoubleClicked(); });

	tbox->addWidget(tree);
	tbox->addWidget(tree_search);
	tbox->addWidget(tree_ats);
	lbox->addWidget(list);
	lbox->addWidget(list_search);
	lbox->addWidget(list_ats);

	swid->addWidget(tfrm);
	swid->addWidget(lfrm);

	swid->setStretchFactor(0, 1);
	swid->setStretchFactor(1, 5);

	frm->addWidget(swid, 0, 0);
	frm->setSpacing(0);
	frm->setContentsMargins(0, 0, 0, 0);
}

void tunersetsView::searchLayout()
{
	this->viewAbstract::searchLayout();

	switch (yx)
	{
		case e2db::YTYPE::sat:
			this->lsr_search.filter->addItem("Freq/Pol/SR", ITEM_ROW_ROLE::combo);
			this->lsr_search.filter->addItem("Frequency", ITEM_ROW_ROLE::s_freq);
			this->lsr_search.filter->addItem("Polarization", ITEM_ROW_ROLE::s_pol);
			this->lsr_search.filter->addItem("Symbol Rate", ITEM_ROW_ROLE::s_sr);
			this->lsr_search.filter->addItem("FEC", ITEM_ROW_ROLE::s_fec);
			this->lsr_search.filter->addItem("System", ITEM_ROW_ROLE::s_sys);
		break;
		case e2db::YTYPE::terrestrial:
			this->lsr_search.filter->addItem("Freq/Const/Band", ITEM_ROW_ROLE::combo);
			this->lsr_search.filter->addItem("Frequency", ITEM_ROW_ROLE::t_freq);
			this->lsr_search.filter->addItem("Constellation", ITEM_ROW_ROLE::t_tmod);
			this->lsr_search.filter->addItem("Bandwidth", ITEM_ROW_ROLE::t_band);
			this->lsr_search.filter->addItem("HP FEC", ITEM_ROW_ROLE::t_hpfec);
			this->lsr_search.filter->addItem("LP FEC", ITEM_ROW_ROLE::t_lpfec);
			this->lsr_search.filter->addItem("System", ITEM_ROW_ROLE::t_sys);
		break;
		case e2db::YTYPE::cable:
			this->lsr_search.filter->addItem("Freq/Mod/SR", ITEM_ROW_ROLE::combo);
			this->lsr_search.filter->addItem("Frequency", ITEM_ROW_ROLE::c_freq);
			this->lsr_search.filter->addItem("Modulation", ITEM_ROW_ROLE::c_cmod);
			this->lsr_search.filter->addItem("Symbol Rate", ITEM_ROW_ROLE::c_sr);
			this->lsr_search.filter->addItem("FEC", ITEM_ROW_ROLE::c_cfec);
			this->lsr_search.filter->addItem("System", ITEM_ROW_ROLE::c_sys);
		break;
		case e2db::YTYPE::atsc:
			this->lsr_search.filter->addItem("Frequency", ITEM_ROW_ROLE::a_freq);
			this->lsr_search.filter->addItem("Modulation", ITEM_ROW_ROLE::a_amod);
			this->lsr_search.filter->addItem("System", ITEM_ROW_ROLE::a_sys);
		break;
	}
}

void tunersetsView::load()
{
	debug("load()");

	tabUpdateFlags(gui::init);

	this->dbih = this->data->dbih;

	string iname = "tns:";
	switch (yx)
	{
		case e2db::YTYPE::sat: iname += 's'; break;
		case e2db::YTYPE::terrestrial: iname += 't'; break;
		case e2db::YTYPE::cable: iname += 'c'; break;
		case e2db::YTYPE::atsc: iname += 'a'; break;
	}

	for (auto & x : dbih->index[iname])
	{
		e2db::tunersets_table tns = dbih->tuners[yx].tables[x.second];
		QString idx = QString::fromStdString(tns.tnid);
		QStringList entry = dbih->entryTunersetsTable(tns);

		QTreeWidgetItem* item = new QTreeWidgetItem(entry);
		item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsDragEnabled | Qt::ItemNeverHasChildren);
		item->setData(0, Qt::UserRole, idx);
		tree->addTopLevelItem(item);
	}

	tree->setDragEnabled(true);
	tree->setAcceptDrops(true);

	list->setDragEnabled(true);
	list->setAcceptDrops(true);

	updateStatus();
}

void tunersetsView::reset()
{
	debug("reset()");

	this->state.changed = false;
	this->state.ti = 0;
	this->state.curr = "";
	this->state.sort = pair (-1, Qt::AscendingOrder); //C++17
	this->state.ty = -1;

	tree->clear();
	tree->setDragEnabled(false);
	tree->setAcceptDrops(false);
	tree->reset();

	list->clear();
	list->setDragEnabled(false);
	list->setAcceptDrops(false);
	list->reset();
	list->header()->setSortIndicatorShown(false);
	list->header()->setSectionsClickable(false);
	list->header()->setSortIndicator(0, Qt::AscendingOrder);

	this->lsr_find.curr = -1;
	this->lsr_find.match.clear();

	this->action.list_newtr->setEnabled(true);

	tabResetStatus();

	this->dbih = nullptr;
}

void tunersetsView::populate()
{
	string curr;
	QTreeWidgetItem* selected;
	selected = tree->currentItem();

	if (selected == NULL)
		selected = tree->topLevelItem(0);
	if (selected != NULL)
	{
		selected->setExpanded(true);
		curr = selected->data(0, Qt::UserRole).toString().toStdString();
		this->state.curr = curr;
	}

	debug("populate()", "curr", curr);

	e2db::tunersets tvs = dbih->tuners[yx];

	if (! tvs.tables.count(curr))
		error("populate()", "curr", curr);

	e2db::tunersets_table tns = tvs.tables[curr];

	list->header()->setSortIndicatorShown(false);
	list->header()->setSectionsClickable(false);
	list->clear();

	int i = 0;

	for (auto & tp : dbih->index[curr])
	{
		e2db::tunersets_transponder txp = tns.transponders[tp.second];
		char ci[7];
		std::sprintf(ci, "%06d", i++);
		QString x = QString::fromStdString(ci);

		QString idx = QString::fromStdString(to_string(tp.first));
		QString trid = QString::fromStdString(txp.trid);
		QStringList entry = dbih->entryTunersetsTransponder(txp, tns);
		entry.prepend(x);
		
		QTreeWidgetItem* item = new QTreeWidgetItem(entry);
		item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsDragEnabled | Qt::ItemNeverHasChildren);
		item->setData(ITEM_DATA_ROLE::idx, Qt::UserRole, idx);
		item->setData(ITEM_DATA_ROLE::trid, Qt::UserRole, trid);

		list->addTopLevelItem(item);
	}

	list->setDragEnabled(true);
	list->setAcceptDrops(true);
	list->header()->setSectionsClickable(true);

	// sorting default column 0|asc
	list->sortItems(0, Qt::AscendingOrder);
	list->header()->setSortIndicator(1, Qt::AscendingOrder);
}

void tunersetsView::treeItemChanged()
{
	debug("treeItemChanged()");

	list->clearSelection();
	list->scrollToTop();
	list->clear();
	list->setDragEnabled(false);
	list->setAcceptDrops(false);

	populate();
	updateStatus(true);
}

void tunersetsView::listItemDoubleClicked()
{
	debug("listItemDoubleClicked()");

	QList<QTreeWidgetItem*> selected = list->selectedItems();
	
	if (selected.empty() || selected.count() > 1)
		return;

	editTransponder();
}

void tunersetsView::addSettings()
{
	debug("addSettings()");

	int tvid = yx;
	e2db::tunersets tvs;
	tvs.ytype = tvid;
	dbih->addTunersets(tvs);
}

void tunersetsView::editSettings()
{
	debug("editSettings()");

	int tvid = yx;

	if (dbih->tuners.count(tvid))
		debug("editSettings()", "tvid", tvid);
	else
		return error("editSettings()", "tvid", tvid);

	e2se_gui::editTunersets* edit = new e2se_gui::editTunersets(this->data, this->yx, this->log->log);
	edit->setEditId(tvid);
	edit->display(cwid);
	edit->getEditId(); // returned after dial.exec()
	edit->destroy();

	this->state.changed = true;
}

void tunersetsView::addPosition()
{
	debug("addPosition()");

	int tvid = yx;
	string tnid;

	if (! dbih->tuners.count(tvid))
		addSettings();

	e2se_gui::editTunersetsTable* add = new e2se_gui::editTunersetsTable(this->data, this->yx, this->log->log);
	add->setAddId(tvid);
	add->display(cwid);
	tnid = add->getAddId(); // returned after dial.exec()
	add->destroy();

	if (dbih->tuners[yx].tables.count(tnid))
		debug("addPosition()", "tnid", tnid);
	else
		return error("addPosition()", "tnid", tnid);

	tree->header()->setSectionsClickable(false);
	tree->setDragEnabled(false);
	tree->setAcceptDrops(false);

	int i = 0, y;
	QTreeWidgetItem* current = tree->currentItem();
	QTreeWidgetItem* parent = tree->invisibleRootItem();
	i = current != nullptr ? parent->indexOfChild(current) : tree->topLevelItemCount();
	y = i + 1;

	e2db::tunersets_table tns = dbih->tuners[yx].tables[tnid];

	QString idx = QString::fromStdString(tns.tnid);
	QStringList entry = dbih->entryTunersetsTable(tns);

	QTreeWidgetItem* item = new QTreeWidgetItem(entry);
	item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsDragEnabled | Qt::ItemNeverHasChildren);
	item->setData(0, Qt::UserRole, idx);

	if (current == nullptr)
		tree->addTopLevelItem(item);
	else
		tree->insertTopLevelItem(y, item);

	tree->header()->setSectionsClickable(true);
	tree->setDragEnabled(true);
	tree->setAcceptDrops(true);

	this->state.changed = true;

	updateStatus();
}

void tunersetsView::editPosition()
{
	debug("editPosition()");

	QList<QTreeWidgetItem*> selected = tree->selectedItems();
	
	if (selected.empty() || selected.count() > 1)
		return;

	QTreeWidgetItem* item = selected.first();
	int tvid = this->yx;
	string tnid = item->data(0, Qt::UserRole).toString().toStdString();
	string nw_tnid;

	if (! dbih->tuners.count(tvid))
		return error("addTransponder()", "tvid", tvid);
	if (dbih->tuners[yx].tables.count(tnid))
		debug("editPosition()", "tnid", tnid);
	else
		return error("editPosition()", "tnid", tnid);

	e2se_gui::editTunersetsTable* edit = new e2se_gui::editTunersetsTable(this->data, this->yx, this->log->log);
	edit->setEditId(tnid, tvid);
	edit->display(cwid);
	nw_tnid = edit->getEditId(); // returned after dial.exec()
	edit->destroy();

	if (dbih->tuners[yx].tables.count(nw_tnid))
		debug("editPosition()", "new tnid", nw_tnid);
	else
		return error("editPosition()", "new tnid", nw_tnid);

	e2db::tunersets_table tns = dbih->tuners[yx].tables[nw_tnid];

	QString idx = QString::fromStdString(tns.tnid);
	QStringList entry = dbih->entryTunersetsTable(tns);
	for (int i = 0; i < entry.count(); i++)
		item->setText(i, entry[i]);
	item->setData(0, Qt::UserRole, idx);

	this->state.changed = true;
}

void tunersetsView::addTransponder()
{
	debug("addTransponder()");

	int tvid = this->yx;
	string tnid = this->state.curr;

	//TODO set disabled
	if (! dbih->tuners.count(tvid))
		return error("addTransponder()", "tvid", tvid);
	if (! dbih->tuners[yx].tables.count(tnid))
		return error("addTransponder()", "tnid", tnid);

	string trid;
	e2se_gui::editTunersetsTransponder* add = new e2se_gui::editTunersetsTransponder(this->data, this->yx, this->log->log);
	add->setAddId(tnid, tvid);
	add->display(cwid);
	trid = add->getAddId(); // returned after dial.exec()
	add->destroy();

	if (dbih->tuners[yx].tables[tnid].transponders.count(trid))
		debug("addTransponder()", "trid", trid);
	else
		return error("addTransponder()", "trid", trid);

	list->header()->setSectionsClickable(false);
	list->setDragEnabled(false);
	list->setAcceptDrops(false);

	int i = 0, y;
	QTreeWidgetItem* current = list->currentItem();
	QTreeWidgetItem* parent = list->invisibleRootItem();
	i = current != nullptr ? parent->indexOfChild(current) : list->topLevelItemCount();
	y = i + 1;

	e2db::tunersets_table tns = dbih->tuners[yx].tables[tnid];
	e2db::tunersets_transponder txp = tns.transponders[trid];

	char ci[7];
	std::sprintf(ci, "%06d", i++);
	QString x = QString::fromStdString(ci);
	QString idx = "";
	QStringList entry = dbih->entryTunersetsTransponder(txp, tns);
	entry.prepend(x);

	QTreeWidgetItem* item = new QTreeWidgetItem(entry);
	item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsDragEnabled | Qt::ItemNeverHasChildren);
	item->setData(ITEM_DATA_ROLE::idx, Qt::UserRole, idx);
	item->setData(ITEM_DATA_ROLE::trid, Qt::UserRole, QString::fromStdString(trid));

	if (current == nullptr)
		list->addTopLevelItem(item);
	else
		list->insertTopLevelItem(y, item);

	list->header()->setSectionsClickable(true);
	list->setDragEnabled(true);
	list->setAcceptDrops(true);

	this->state.changed = true;

	updateStatus();
}

void tunersetsView::editTransponder()
{
	debug("editTransponder()");

	QList<QTreeWidgetItem*> selected = list->selectedItems();
	
	if (selected.empty() || selected.count() > 1)
		return;

	QTreeWidgetItem* item = selected.first();
	int tvid = this->yx;
	string trid = item->data(ITEM_DATA_ROLE::trid, Qt::UserRole).toString().toStdString();
	string nw_trid;
	string tnid = this->state.curr;

	if (! dbih->tuners.count(tvid))
		return error("editTransponder()", "tvid", tvid);
	if (! dbih->tuners[yx].tables.count(tnid))
		return error("editTransponder()", "tnid", tnid);

	if (dbih->tuners[yx].tables[tnid].transponders.count(trid))
		debug("editTransponder()", "trid", trid);
	else
		return error("editTransponder()", "trid", trid);

	e2se_gui::editTunersetsTransponder* edit = new e2se_gui::editTunersetsTransponder(this->data, this->yx, this->log->log);
	edit->setEditId(trid, tnid, tvid);
	edit->display(cwid);
	nw_trid = edit->getEditId(); // returned after dial.exec()
	edit->destroy();

	if (dbih->tuners[yx].tables[tnid].transponders.count(nw_trid))
		debug("editTransponder()", "new trid", nw_trid);
	else
		return error("editTransponder()", "new trid", nw_trid);

	e2db::tunersets_table tns = dbih->tuners[yx].tables[tnid];
	e2db::tunersets_transponder txp = tns.transponders[nw_trid];

	QStringList entry = dbih->entryTunersetsTransponder(txp, tns);
	entry.prepend(item->text(ITEM_ROW_ROLE::x));
	for (int i = 0; i < entry.count(); i++)
		item->setText(i, entry[i]);
	item->setData(ITEM_DATA_ROLE::trid, Qt::UserRole, QString::fromStdString(nw_trid));

	this->state.changed = true;
}

void tunersetsView::treeItemDelete()
{
	debug("treeItemDelete()");

	QList<QTreeWidgetItem*> selected = tree->selectedItems();
	
	if (selected.empty())
	{
		return;
	}

	e2db::tunersets tvs = dbih->tuners[yx];
	
	for (auto & item : selected)
	{
		int i = tree->indexOfTopLevelItem(item);
		string tnid = item->data(0, Qt::UserRole).toString().toStdString();
		tree->takeTopLevelItem(i);

		dbih->removeTunersetsTable(tnid, tvs);
	}

	this->state.changed = true;

	updateStatus();
}

void tunersetsView::listItemCut()
{
	debug("listItemCut()");

	listItemCopy(true);
}

void tunersetsView::listItemCopy(bool cut)
{
	debug("listItemCopy()");

	QList<QTreeWidgetItem*> selected = list->selectedItems();
	
	if (selected.empty())
		return;

	if (cut)
		listItemDelete();
}

void tunersetsView::listItemPaste()
{
	debug("listItemPaste()");

	QClipboard* clipboard = QGuiApplication::clipboard();
	const QMimeData* mimeData = clipboard->mimeData();
	vector<QString> items;
}

void tunersetsView::listItemDelete()
{
	debug("listItemDelete()");

	QList<QTreeWidgetItem*> selected = list->selectedItems();
	
	if (selected.empty())
		return;

	list->header()->setSectionsClickable(false);
	list->setDragEnabled(false);
	list->setAcceptDrops(false);

	string tnid = this->state.curr;
	e2db::tunersets_table tns = dbih->tuners[yx].tables[tnid];

	for (auto & item : selected)
	{
		int i = list->indexOfTopLevelItem(item);
		string trid = item->data(ITEM_DATA_ROLE::trid, Qt::UserRole).toString().toStdString();
		list->takeTopLevelItem(i);

		dbih->removeTunersetsTransponder(trid, tns);
	}

	list->header()->setSectionsClickable(true);
	list->setDragEnabled(true);
	list->setAcceptDrops(true);

	this->state.changed = true;
}

void tunersetsView::listItemSelectAll()
{
	debug("listItemSelectAll()");

	list->selectAll();
}

void tunersetsView::putListItems(vector<QString> items)
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

	list->header()->setSectionsClickable(true);
	list->setDragEnabled(true);
	list->setAcceptDrops(true);

	updateFlags();
	updateStatus();
}

void tunersetsView::updateStatus(bool current)
{
	debug("updateStatus()");

	gui::STATUS status;
	status.current = current;

	//TODO FIX
	//
	// clang mach size = 133295584
	//  g++ linux size = 32674
	//  g++ mingw size = ?
	// 
	// clang mach size = 411
	//  g++ linux size = 411
	//  g++ mingw size = 4509

	if (current && ! this->state.curr.empty())
	{
		string curr = this->state.curr;
		if (yx == e2db::YTYPE::sat)
		{
			e2db::tunersets_table tns = dbih->tuners[yx].tables[curr];
			status.position = dbih->value_transponder_position(tns);
		}
		status.counters[gui::COUNTER::position] = dbih->index[curr].size();
	}
	else
	{
		string iname = "tns:";
		switch (yx)
		{
			case e2db::YTYPE::sat: iname += 's'; break;
			case e2db::YTYPE::terrestrial: iname += 't'; break;
			case e2db::YTYPE::cable: iname += 'c'; break;
			case e2db::YTYPE::atsc: iname += 'a'; break;
		}
		for (auto & x : dbih->index[iname])
		{
			status.counters[gui::COUNTER::transponders] += dbih->index[x.second].size();
		}
	}

	tabSetStatus(status);
}

void tunersetsView::showTreeEditContextMenu(QPoint &pos)
{
	debug("showTreeEditContextMenu()");

	QMenu* tree_edit = new QMenu;

	tree_edit->addAction("Edit Settings", [=]() { this->editSettings(); });
	tree_edit->addSeparator();
	tree_edit->addAction("Edit Position", [=]() { this->editPosition(); });
	tree_edit->addSeparator();
	tree_edit->addAction("Delete", [=]() { this->treeItemDelete(); });

	tree_edit->exec(tree->mapToGlobal(pos));
}

void tunersetsView::showListEditContextMenu(QPoint &pos)
{
	debug("showListEditContextMenu()");

	QMenu* list_edit = new QMenu;
	list_edit->addAction("Edit Transponder", [=]() { this->editTransponder(); });
	list_edit->addSeparator();
	list_edit->addAction("Delete", [=]() { this->listItemDelete(); });

	list_edit->exec(list->mapToGlobal(pos));
}

void tunersetsView::updateFlags()
{
	debug("updateFlags()");

	if (tree->topLevelItemCount())
	{
		//TODO connect to QScrollArea Event
		/*if (tree->verticalScrollBar()->isVisible())
		{*/
			tabSetFlag(gui::TabTreeFind, true);
			this->action.tree_search->setEnabled(true);
		/*}
		else
		{
			tabSetFlag(gui::TabTreeFind, false);
			this->action.bouquets_search->setEnabled(false);
		}*/
	}
	else
	{
		tabSetFlag(gui::TabTreeFind, false);
		this->action.tree_search->setDisabled(true);
	}

	if (list->topLevelItemCount())
	{
		tabSetFlag(gui::TabListSelectAll, true);
		tabSetFlag(gui::TabListFind, true);
		this->action.list_search->setEnabled(true);
	}
	else
	{
		tabSetFlag(gui::TabListSelectAll, false);
		tabSetFlag(gui::TabListFind, false);
		this->action.list_search->setDisabled(true);
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

}
