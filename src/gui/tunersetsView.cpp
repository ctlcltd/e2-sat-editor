/*!
 * e2-sat-editor/src/gui/tunersetsView.cpp
 *
 * @link https://github.com/ctlcltd/e2-sat-editor
 * @copyright e2 SAT Editor Team
 * @author Leonardo Laureti
 * @version 0.6
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

#include "toolkit/TreeProxyStyle.h"
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

tunersetsView::tunersetsView(tab* tid, QWidget* cwid, dataHandler* data, int ytype)
{
	this->log = new logger("gui", "tunersetsView");

	this->tid = tid;
	this->cwid = cwid;
	this->data = data;
	this->theme = new e2se_gui::theme;
	this->widget = new QWidget;

	this->state.yx = ytype;

	layout();
}

tunersetsView::~tunersetsView()
{
	debug("~tunersetsView");

	delete this->widget;
	delete this->theme;
	delete this->log;
}

void tunersetsView::layout()
{
	debug("layout");

	widget->setStyleSheet("QGroupBox { spacing: 0; border: 0; padding: 0; padding-top: 32px; font-weight: bold } QGroupBox::title { margin: 8px 4px; padding: 0 1px 1px }");

	QGridLayout* frm = new QGridLayout(widget);

	QSplitter* swid = new QSplitter;

	QVBoxLayout* tbox = new QVBoxLayout;
	QVBoxLayout* lbox = new QVBoxLayout;

	QGroupBox* tfrm = new QGroupBox;
	QGroupBox* lfrm = new QGroupBox;

	frm->setContentsMargins(0, 0, 0, 0);
	tbox->setContentsMargins(0, 0, 0, 0);
	lbox->setContentsMargins(0, 0, 0, 0);

	frm->setSpacing(0);
	tbox->setSpacing(0);
	lbox->setSpacing(0);

	tfrm->setFlat(true);
	lfrm->setFlat(true);

	switch (this->state.yx)
	{
		case e2db::YTYPE::satellite:
			tfrm->setTitle(tr("Satellites"));
		break;
		case e2db::YTYPE::terrestrial:
		case e2db::YTYPE::cable:
		case e2db::YTYPE::atsc:
			tfrm->setTitle(tr("Positions"));
		break;
		default:
			error("layout", tr("Error", "error").toStdString(), tr("Unknown tuner settings type.", "error").toStdString());
	}
	lfrm->setTitle(tr("Transponders"));

	QStringList ths, lhs;
	switch (this->state.yx)
	{
		case e2db::YTYPE::satellite:
			ths = QStringList ({NULL, tr("Name"), tr("Position")});
			lhs = QStringList ({NULL, "TRID", tr("Freq/Pol/SR"), tr("Frequency"), tr("Polarization"), tr("Symbol Rate"), tr("System"), tr("FEC"), tr("Modulation"), tr("Inversion"),  tr("Roll offset"), tr("Pilot")});
		break;
		case e2db::YTYPE::terrestrial:
			ths = QStringList ({NULL, tr("Name"), tr("Country")});
			lhs = QStringList ({NULL, "TRID", tr("Freq/Const/Band"), tr("Frequency"), tr("Constellation"), NULL, tr("System"), tr("Bandwidth"), tr("Tmx Mode"), tr("HP FEC"), tr("LP FEC"), tr("Inversion"), tr("Guard"), tr("Hierarchy")});
		break;
		case e2db::YTYPE::cable:
			ths = QStringList ({NULL, tr("Name"), tr("Country")});
			lhs = QStringList ({NULL, "TRID", tr("Freq/Mod/SR"), tr("Frequency"), tr("Modulation"), tr("Symbol Rate"), tr("System"), tr("FEC"), tr("Inversion")});
		break;
		case e2db::YTYPE::atsc:
			ths = QStringList ({NULL, tr("Name")});
			lhs = QStringList ({NULL, "TRID", NULL, tr("Frequency"), tr("Modulation"), NULL, tr("System")});
		break;
	}

	this->tree = new QTreeWidget;
	this->list = new QTreeWidget;

	if (widget->layoutDirection() == Qt::LeftToRight)
	{
		tree->setContentsMargins(0, 0, 10, 0);
		list->setContentsMargins(10, 0, 0, 0);
	}
	else
	{
		tree->setContentsMargins(0, 0, 10, 0);
		list->setContentsMargins(10, 0, 0, 0);
	}

	TreeProxyStyle* tree_style = new TreeProxyStyle;
	tree->setStyle(tree_style);
	TreeProxyStyle* list_style = new TreeProxyStyle;
	list->setStyle(list_style);

	if (theme::isDefault())
	{
		tree->setStyleSheet("QTreeWidget::item { padding: 6px 0 }");
		list->setStyleSheet("QTreeWidget::item { padding: 6px 0 }");
	}
	else
	{
		tree->setStyleSheet("QTreeWidget { border: 0 } QTreeWidget::item { padding: 6px 0 }");
		list->setStyleSheet("QTreeWidget { border: 0 } QTreeWidget::item { padding: 6px 0 }");
	}

#ifdef Q_OS_MAC
	if (theme::isDefault())
	{
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
	}
#endif

	tree->setMinimumWidth(240);
	tree->setUniformRowHeights(true);
	tree->setSelectionBehavior(QTreeWidget::SelectRows);
	tree->setSelectionMode(QTreeWidget::SingleSelection);
	tree->setItemsExpandable(false);
	tree->setExpandsOnDoubleClick(false);
	tree->setDropIndicatorShown(false);
	tree->setDragDropMode(QTreeWidget::InternalMove);
	tree->setEditTriggers(QTreeWidget::NoEditTriggers);
	tree->setRootIsDecorated(false);
	tree->setIndentation(false);
	tree_style->setIndentation(6, true);
	tree_style->setFirstColumnIndent(1);

	QTreeWidgetItem* tree_thead = new QTreeWidgetItem(ths);
	tree->setHeaderItem(tree_thead);
	tree->setColumnHidden(TREE_ROW_ROLE::tnid, true); // hidden tnid
	tree->setColumnWidth(TREE_ROW_ROLE::trow1, 200);  // Name
	tree->setColumnWidth(TREE_ROW_ROLE::trow2, 75);   // Position | Country

	list->setUniformRowHeights(true);
	list->setSelectionBehavior(QTreeWidget::SelectRows);
	list->setSelectionMode(QTreeWidget::ExtendedSelection);
	list->setItemsExpandable(false);
	list->setExpandsOnDoubleClick(false);
	list->setDropIndicatorShown(false);
	list->setDragDropMode(QTreeWidget::InternalMove);
	list->setEditTriggers(QTreeWidget::NoEditTriggers);
	list->setRootIsDecorated(false);
	list->setIndentation(false);
	list_style->setIndentation(8, true);
	list_style->setFirstColumnIndent(1);

	QTreeWidgetItem* list_thead = new QTreeWidgetItem(lhs);
	list->setHeaderItem(list_thead);
	list->setColumnHidden(ITEM_ROW_ROLE::x, true);		// hidden index
	if (QSettings().value("application/debug", true).toBool()) {
		list->setColumnWidth(ITEM_ROW_ROLE::debug_trid, 175);
	}
	else
	{
		list->setColumnHidden(ITEM_ROW_ROLE::debug_trid, true);
	}
	list->setColumnWidth(ITEM_ROW_ROLE::combo, 175);	// combo (s: freq|pol|sr, t: freq|tmod|band, c: freq|cmod|sr, a: freq)
	if (lhs.at(ITEM_ROW_ROLE::combo).isEmpty())
		list->setColumnHidden(ITEM_ROW_ROLE::combo, true);
	list->setColumnWidth(ITEM_ROW_ROLE::row3, 85);		// Frequency
	list->setColumnWidth(ITEM_ROW_ROLE::row4, 85);		// Polarization | Constellation | Modulation
	list->setColumnWidth(ITEM_ROW_ROLE::row5, 85);		// Symbol Rate
	if (lhs.at(ITEM_ROW_ROLE::row5).isEmpty())
		list->setColumnHidden(ITEM_ROW_ROLE::row5, true);
	list->setColumnWidth(ITEM_ROW_ROLE::row6, 75);		// System
	list->setColumnWidth(ITEM_ROW_ROLE::row7, 75);		// FEC | Bandwidth
	list->setColumnWidth(ITEM_ROW_ROLE::row8, 75);		// Modulation | Tmx Mod | Inversion
	list->setColumnWidth(ITEM_ROW_ROLE::row9, 75);		// Inversion | HP FEC
	list->setColumnWidth(ITEM_ROW_ROLE::rowA, 75);		// Roll offset | LP FEC
	list->setColumnWidth(ITEM_ROW_ROLE::rowB, 75);		// Pilot | Inversion
	list->setColumnWidth(ITEM_ROW_ROLE::rowC, 70);		// Guard
	list->setColumnWidth(ITEM_ROW_ROLE::rowD, 70);		// Hierarchy

	// numeric items
	QTreeWidgetItem* tree_head = list->headerItem();
	tree_head->setData(ITEM_ROW_ROLE::row3, Qt::UserRole, true);
	tree_head->setData(ITEM_ROW_ROLE::row5, Qt::UserRole, true);

	list->header()->connect(list->header(), &QHeaderView::sectionClicked, [=](int column) { this->sortByColumn(column); });

	tree->setContextMenuPolicy(Qt::CustomContextMenu);
	tree->connect(tree, &QTreeWidget::customContextMenuRequested, [=](QPoint pos) { this->showTreeEditContextMenu(pos); });
	list->setContextMenuPolicy(Qt::CustomContextMenu);
	list->connect(list, &QTreeWidget::customContextMenuRequested, [=](QPoint pos) { this->showListEditContextMenu(pos); });

	searchLayout();

	QToolBar* tree_ats = toolBar();
	QToolBar* list_ats = toolBar();

	this->action.tree_newtn = toolBarAction(tree_ats, tr("New Position", "toolbar"), theme->dynamicIcon("add"), [=]() { this->addPosition(); });
	toolBarSpacer(tree_ats);
	this->action.tree_search = toolBarButton(tree_ats, tr("Find…", "toolbar"), theme->dynamicIcon("search"), [=]() { this->treeSearchToggle(); });

	this->action.tree_search->setDisabled(true);

	this->action.list_newtr = toolBarAction(list_ats, tr("New Transponder", "toolbar"), theme->dynamicIcon("add"), [=]() { this->addTransponder(); });
	toolBarSpacer(list_ats);
	this->action.list_search = toolBarButton(list_ats, tr("Find…", "toolbar"), theme->dynamicIcon("search"), [=]() { this->listSearchToggle(); });

	this->action.list_newtr->setDisabled(true);
	this->action.list_search->setDisabled(true);

	this->tree_evth = new TreeDropIndicatorEventPainter;
	this->list_evth = new TreeDropIndicatorEventPainter;
	this->tree_evto = new TreeItemChangedEventObserver;
	this->list_evto = new TreeItemChangedEventObserver;
	tree->installEventFilter(tree_evto);
	tree->viewport()->installEventFilter(tree_evth);
	tree->connect(tree, &QTreeWidget::currentItemChanged, [=](QTreeWidgetItem* current) { this->treeItemChanged(current); });
	tree->connect(tree, &QTreeWidget::itemDoubleClicked, [=]() { this->treeItemDoubleClicked(); });
	list->installEventFilter(list_evto);
	list->viewport()->installEventFilter(list_evth);
	list->connect(list, &QTreeWidget::currentItemChanged, [=]() { this->listItemChanged(); });
	list->connect(list, &QTreeWidget::itemSelectionChanged, [=]() { this->listItemSelectionChanged(); });
	list->connect(list, &QTreeWidget::itemDoubleClicked, [=]() { this->listItemDoubleClicked(); });

	tbox->addWidget(tree);
	tbox->addWidget(tree_search);
	tbox->addWidget(tree_ats);
	tfrm->setLayout(tbox);

	lbox->addWidget(list);
	lbox->addWidget(list_search);
	lbox->addWidget(list_ats);
	lfrm->setLayout(lbox);

	platform::osWidgetOpaque(swid);

	swid->addWidget(tfrm);
	swid->addWidget(lfrm);

	swid->setStretchFactor(0, 1);
	swid->setStretchFactor(1, 5);

	frm->addWidget(swid);

	toolBarStyleSheet();
}

void tunersetsView::searchLayout()
{
	this->viewAbstract::searchLayout();

	switch (this->state.yx)
	{
		case e2db::YTYPE::satellite:
			this->lsr_search.filter->addItem(tr("Freq/Pol/SR"), ITEM_ROW_ROLE::combo);
			this->lsr_search.filter->addItem(tr("Frequency"), ITEM_ROW_ROLE::s_freq);
			this->lsr_search.filter->addItem(tr("Polarization"), ITEM_ROW_ROLE::s_pol);
			this->lsr_search.filter->addItem(tr("Symbol Rate"), ITEM_ROW_ROLE::s_sr);
			this->lsr_search.filter->addItem(tr("FEC"), ITEM_ROW_ROLE::s_fec);
			this->lsr_search.filter->addItem(tr("System"), ITEM_ROW_ROLE::s_sys);
		break;
		case e2db::YTYPE::terrestrial:
			this->lsr_search.filter->addItem(tr("Freq/Const/Band"), ITEM_ROW_ROLE::combo);
			this->lsr_search.filter->addItem(tr("Frequency"), ITEM_ROW_ROLE::t_freq);
			this->lsr_search.filter->addItem(tr("Constellation"), ITEM_ROW_ROLE::t_tmod);
			this->lsr_search.filter->addItem(tr("Bandwidth"), ITEM_ROW_ROLE::t_band);
			this->lsr_search.filter->addItem(tr("HP FEC"), ITEM_ROW_ROLE::t_hpfec);
			this->lsr_search.filter->addItem(tr("LP FEC"), ITEM_ROW_ROLE::t_lpfec);
			this->lsr_search.filter->addItem(tr("System"), ITEM_ROW_ROLE::t_sys);
		break;
		case e2db::YTYPE::cable:
			this->lsr_search.filter->addItem(tr("Freq/Mod/SR"), ITEM_ROW_ROLE::combo);
			this->lsr_search.filter->addItem(tr("Frequency"), ITEM_ROW_ROLE::c_freq);
			this->lsr_search.filter->addItem(tr("Modulation"), ITEM_ROW_ROLE::c_cmod);
			this->lsr_search.filter->addItem(tr("Symbol Rate"), ITEM_ROW_ROLE::c_sr);
			this->lsr_search.filter->addItem(tr("FEC"), ITEM_ROW_ROLE::c_cfec);
			this->lsr_search.filter->addItem(tr("System"), ITEM_ROW_ROLE::c_sys);
		break;
		case e2db::YTYPE::atsc:
			this->lsr_search.filter->addItem(tr("Frequency"), ITEM_ROW_ROLE::a_freq);
			this->lsr_search.filter->addItem(tr("Modulation"), ITEM_ROW_ROLE::a_amod);
			this->lsr_search.filter->addItem(tr("System"), ITEM_ROW_ROLE::a_sys);
		break;
	}
}

void tunersetsView::load()
{
	debug("load");

	tabUpdateFlags(gui::init);

	int tvid = this->state.yx;

	auto* dbih = this->data->dbih;

	string iname = "tns:";
	char yname = dbih->value_transponder_type(tvid);
	iname += yname;

	for (auto & x : dbih->index[iname])
	{
		e2db::tunersets_table tns = dbih->tuners[tvid].tables[x.second];
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

	updateFlags();
	updateStatusBar();
}

void tunersetsView::reset()
{
	debug("reset");

	unsetPendingUpdateListIndex();

	this->state.curr = "";
	this->state.sort = pair (-1, Qt::AscendingOrder);

	tree->reset();
	tree->setDragEnabled(false);
	tree->setAcceptDrops(false);
	tree->clear();

	list->reset();
	list->setDragEnabled(false);
	list->setAcceptDrops(false);
	list->clear();
	list->header()->setSortIndicatorShown(false);
	list->header()->setSectionsClickable(false);
	list->header()->setSortIndicator(0, Qt::AscendingOrder);

	this->lsr_find.curr = -1;
	this->lsr_find.match.clear();

	this->action.list_newtr->setDisabled(true);

	resetStatusBar();
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

	int tvid = this->state.yx;
	string tnid = this->state.curr;

	auto* dbih = this->data->dbih;

	e2db::tunersets tvs = dbih->tuners[tvid];

	if (tvs.tables.count(tnid))
		debug("populate", "current", tnid);
	else
		error("populate", tr("Error", "error").toStdString(), tr("Missing tuner settings table key \"%1\".", "error").arg(tnid.data()).toStdString());

	e2db::tunersets_table tns = tvs.tables[tnid];

	list->header()->setSortIndicatorShown(false);
	list->header()->setSectionsClickable(false);
	list->clear();

	QList<QTreeWidgetItem*> items;
	int i = 0;
	size_t pad_width = 4;

	for (auto & tp : dbih->index[tnid])
	{
		e2db::tunersets_transponder tntxp = tns.transponders[tp.second];

		QString x = QString::number(i++).rightJustified(pad_width, '0');
		QString idx = QString::number(tp.first);
		QString trid = QString::fromStdString(tntxp.trid);
		QStringList entry = dbih->entryTunersetsTransponder(tntxp, tns);
		entry.prepend(x);

		QTreeWidgetItem* item = new QTreeWidgetItem(entry);
		item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsDragEnabled | Qt::ItemNeverHasChildren);
		item->setData(ITEM_DATA_ROLE::idx, Qt::UserRole, idx);
		item->setData(ITEM_DATA_ROLE::trid, Qt::UserRole, trid);

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

void tunersetsView::treeItemChanged(QTreeWidgetItem* current)
{
	debug("treeItemChanged");

	if (current != NULL)
	{
		list->clearSelection();
		list->scrollToTop();

		this->action.list_newtr->setEnabled(true);

		if (QGuiApplication::clipboard()->text().isEmpty())
			tabSetFlag(gui::TabListPaste, false);
		else
			tabSetFlag(gui::TabListPaste, true);
	}

	updateListIndex();

	populate();

	updateFlags();
	updateStatusBar(true);
}

void tunersetsView::treeItemDoubleClicked()
{
	debug("treeItemDoubleClicked");

	QList<QTreeWidgetItem*> selected = tree->selectedItems();

	if (selected.empty() || selected.count() > 1)
		return;

	editPosition();
}

void tunersetsView::listItemChanged()
{
	// debug("listItemChanged");

	if (list_evto->isChanged())
		listPendingUpdate();
}

void tunersetsView::listItemSelectionChanged()
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

void tunersetsView::listItemDoubleClicked()
{
	debug("listItemDoubleClicked");

	QList<QTreeWidgetItem*> selected = list->selectedItems();

	if (selected.empty() || selected.count() > 1)
		return;

	editTransponder();
}

void tunersetsView::listPendingUpdate()
{
	debug("listPendingUpdate");

	setPendingUpdateListIndex();

	this->data->setChanged(true);
}

void tunersetsView::addSettings()
{
	debug("addSettings");

	int tvid = this->state.yx;
	e2db::tunersets tvs;
	tvs.ytype = tvid;

	auto* dbih = this->data->dbih;

	dbih->addTunersets(tvs);

	this->data->setChanged(true);
}

void tunersetsView::editSettings()
{
	debug("editSettings");

	int tvid = this->state.yx;

	auto* dbih = this->data->dbih;

	if (dbih->tuners.count(tvid))
		debug("editSettings", "tvid", tvid);
	else
		addSettings();

	e2se_gui::editTunersets* edit = new e2se_gui::editTunersets(this->data, tvid);
	edit->setEditId(tvid);
	edit->display(cwid);
	edit->getEditId();
	edit->destroy();

	this->data->setChanged(true);
}

void tunersetsView::addPosition()
{
	debug("addPosition");

	int tvid = this->state.yx;
	string tnid;

	auto* dbih = this->data->dbih;

	if (! dbih->tuners.count(tvid))
		addSettings();

	e2se_gui::editTunersetsTable* add = new e2se_gui::editTunersetsTable(this->data, tvid);
	add->setAddId(tvid);
	add->display(cwid);
	tnid = add->getAddId();
	add->destroy();

	if (dbih->tuners[tvid].tables.count(tnid))
		debug("addPosition", "tnid", tnid);
	else
		return error("addPosition", tr("Error", "error").toStdString(), tr("Missing tuner settings table key \"%1\".", "error").arg(tnid.data()).toStdString());

	tree->header()->setSectionsClickable(false);
	tree->setDragEnabled(false);
	tree->setAcceptDrops(false);

	int i = 0, y;
	QTreeWidgetItem* current = tree->currentItem();
	QTreeWidgetItem* parent = tree->invisibleRootItem();
	i = current != nullptr ? parent->indexOfChild(current) : tree->topLevelItemCount();
	y = i + 1;

	e2db::tunersets_table tns = dbih->tuners[tvid].tables[tnid];

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

	updateTreeIndex();

	updateFlags();
	updateStatusBar();

	this->data->setChanged(true);
}

void tunersetsView::editPosition()
{
	debug("editPosition");

	QList<QTreeWidgetItem*> selected = tree->selectedItems();

	if (selected.empty() || selected.count() > 1)
		return;

	QTreeWidgetItem* item = selected.first();
	int tvid = this->state.yx;
	string tnid = item->data(0, Qt::UserRole).toString().toStdString();
	string nw_tnid;

	auto* dbih = this->data->dbih;

	if (! dbih->tuners.count(tvid))
		return error("editPosition", tr("Error", "error").toStdString(), tr("Missing tuner settings key \"%1\".", "error").arg(tvid).toStdString());

	if (dbih->tuners[tvid].tables.count(tnid))
		debug("editPosition", "tnid", tnid);
	else
		return error("editPosition", tr("Error", "error").toStdString(), tr("Tuner settings table \"%1\" not exists.", "error").arg(tnid.data()).toStdString());

	e2se_gui::editTunersetsTable* edit = new e2se_gui::editTunersetsTable(this->data, tvid);
	edit->setEditId(tnid, tvid);
	edit->display(cwid);
	nw_tnid = edit->getEditId();
	edit->destroy();

	if (dbih->tuners[tvid].tables.count(nw_tnid))
		debug("editPosition", "new tnid", nw_tnid);
	else
		return error("editPosition", tr("Error", "error").toStdString(), tr("Missing tuner settings table key \"%1\".", "error").arg(nw_tnid.data()).toStdString());

	e2db::tunersets_table tns = dbih->tuners[tvid].tables[nw_tnid];

	QString idx = QString::fromStdString(tns.tnid);
	QStringList entry = dbih->entryTunersetsTable(tns);
	for (int i = 0; i < entry.count(); i++)
		item->setText(i, entry[i]);
	item->setData(0, Qt::UserRole, idx);

	this->data->setChanged(true);
}

void tunersetsView::addTransponder()
{
	debug("addTransponder");

	int tvid = this->state.yx;
	string tnid = this->state.curr;

	auto* dbih = this->data->dbih;

	if (! dbih->tuners.count(tvid))
		return error("addTransponder", tr("Error", "error").toStdString(), tr("Missing tuner settings key \"%1\".", "error").arg(tvid).toStdString());

	if (! dbih->tuners[tvid].tables.count(tnid))
		return error("addTransponder", tr("Error", "error").toStdString(), tr("Missing tuner settings table key \"%1\".", "error").arg(tnid.data()).toStdString());

	string trid;
	e2se_gui::editTunersetsTransponder* add = new e2se_gui::editTunersetsTransponder(this->data, tvid);
	add->setAddId(tnid, tvid);
	add->display(cwid);
	trid = add->getAddId();
	add->destroy();

	if (dbih->tuners[tvid].tables[tnid].transponders.count(trid))
		debug("addTransponder", "trid", trid);
	else
		return error("addTransponder", tr("Error", "error").toStdString(), tr("Missing tuner settings transponder key \"%1\".", "error").arg(trid.data()).toStdString());

	list->header()->setSectionsClickable(false);
	list->setDragEnabled(false);
	list->setAcceptDrops(false);

	int i = 0, y;
	QTreeWidgetItem* current = list->currentItem();
	QTreeWidgetItem* parent = list->invisibleRootItem();
	i = current != nullptr ? parent->indexOfChild(current) : list->topLevelItemCount();
	y = i + 1;

	e2db::tunersets_table tns = dbih->tuners[tvid].tables[tnid];
	e2db::tunersets_transponder tntxp = tns.transponders[trid];

	size_t pad_width = 4;
	QString x = QString::number(i++).rightJustified(pad_width, '0');
	QString idx = QString::number(i);
	QStringList entry = dbih->entryTunersetsTransponder(tntxp, tns);
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

	setPendingUpdateListIndex();

	updateFlags();
	updateStatusBar();

	this->data->setChanged(true);
}

void tunersetsView::editTransponder()
{
	debug("editTransponder");

	QList<QTreeWidgetItem*> selected = list->selectedItems();

	if (selected.empty() || selected.count() > 1)
		return;

	QTreeWidgetItem* item = selected.first();
	int tvid = this->state.yx;
	string trid = item->data(ITEM_DATA_ROLE::trid, Qt::UserRole).toString().toStdString();
	string nw_trid;
	string tnid = this->state.curr;

	auto* dbih = this->data->dbih;

	if (! dbih->tuners.count(tvid))
		return error("editTransponder", tr("Error", "error").toStdString(), tr("Missing tuner settings key \"%1\".", "error").arg(tvid).toStdString());

	if (! dbih->tuners[tvid].tables.count(tnid))
		return error("editTransponder", tr("Error", "error").toStdString(), tr("Missing tuner settings table key \"%1\".", "error").arg(tnid.data()).toStdString());

	if (dbih->tuners[tvid].tables[tnid].transponders.count(trid))
		debug("editTransponder", "trid", trid);
	else
		return error("editTransponder", tr("Error", "error").toStdString(), tr("Tuner settings transponder \"%1\" not exists.", "error").arg(trid.data()).toStdString());

	e2se_gui::editTunersetsTransponder* edit = new e2se_gui::editTunersetsTransponder(this->data, tvid);
	edit->setEditId(trid, tnid, tvid);
	edit->display(cwid);
	nw_trid = edit->getEditId();
	edit->destroy();

	if (dbih->tuners[tvid].tables[tnid].transponders.count(nw_trid))
		debug("editTransponder", "new trid", nw_trid);
	else
		return error("editTransponder", tr("Error", "error").toStdString(), tr("Missing tuner settings transponder key \"%1\".", "error").arg(nw_trid.data()).toStdString());

	e2db::tunersets_table tns = dbih->tuners[tvid].tables[tnid];
	e2db::tunersets_transponder tntxp = tns.transponders[nw_trid];

	QStringList entry = dbih->entryTunersetsTransponder(tntxp, tns);
	entry.prepend(item->text(ITEM_ROW_ROLE::x));
	for (int i = 0; i < entry.count(); i++)
		item->setText(i, entry[i]);
	item->setData(ITEM_DATA_ROLE::trid, Qt::UserRole, QString::fromStdString(nw_trid));

	setPendingUpdateListIndex();

	this->data->setChanged(true);
}

void tunersetsView::treeItemDelete()
{
	debug("treeItemDelete");

	QList<QTreeWidgetItem*> selected = tree->selectedItems();

	if (selected.empty())
	{
		return;
	}

	bool remove = tabRemoveQuestion(tr("Confirm deletetion", "message"), tr("Do you want to delete items?", "message"));
	if (! remove)
		return;

	int tvid = this->state.yx;

	auto* dbih = this->data->dbih;

	e2db::tunersets tvs = dbih->tuners[tvid];

	for (auto & item : selected)
	{
		int i = tree->indexOfTopLevelItem(item);
		string tnid = item->data(0, Qt::UserRole).toString().toStdString();
		tree->takeTopLevelItem(i);

		dbih->removeTunersetsTable(tnid, tvs);
	}

	setPendingUpdateListIndex();
	updateTreeIndex();

	updateStatusBar();

	this->data->setChanged(true);
}

void tunersetsView::listItemCopy(bool cut)
{
	debug("listItemCopy");

	QList<QTreeWidgetItem*> selected = list->selectedItems();

	if (selected.empty())
		return;

	QClipboard* clipboard = QGuiApplication::clipboard();
	QStringList text;

	for (auto & item : selected)
	{
		QString trid = item->data(ITEM_DATA_ROLE::trid, Qt::UserRole).toString();
		QChar ty = trid.isEmpty() ? '\0' : QChar (trid[0]); //Qt5

		QStringList data;
		data.reserve(TSV_TABS + 1);

		// start from freq column [3]
		for (int i = ITEM_ROW_ROLE::row3; i < list->columnCount(); i++)
		{
			QString qstr = item->data(i, Qt::DisplayRole).toString();
			data.append(qstr);
		}

		data.prepend(ty); // insert ty column [0]
		switch (this->state.yx)
		{
			case e2db::YTYPE::terrestrial:
			case e2db::YTYPE::atsc:
				//Qt5
				data.removeAt(3); // remove sr column [5]
			break;
		}

		// fill with empty columns
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
		data.resize(TSV_TABS + 1);
#else
		//TODO TEST
		for (int i = data.size(); i != TSV_TABS + 1; i++)
			data.append("");
#endif

		text.append(data.join("\t")); // TSV
	}
	clipboard->setText(text.join("\n"));

	if (cut)
		listItemDelete();
}

void tunersetsView::listItemPaste()
{
	debug("listItemPaste");

	QClipboard* clipboard = QGuiApplication::clipboard();
	const QMimeData* mimeData = clipboard->mimeData();
	vector<QString> items;

	if (mimeData->hasText())
	{
		QString text = clipboard->text();
		QString separator;

		if (text.endsWith("\r\n"))
			separator = "\r\n";
		else if (text.endsWith("\r"))
			separator = "\r";
		else
			separator = "\n";

		if (text.endsWith(separator))
			text.remove(text.size() - separator.size(), separator.size());

		QStringList data = text.split(separator);

		for (QString & str : data)
		{
			if (str.count('\t') == TSV_TABS) // TSV
				items.emplace_back(str);
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
}

void tunersetsView::listItemDelete()
{
	debug("listItemDelete");

	QList<QTreeWidgetItem*> selected = list->selectedItems();

	if (selected.empty())
		return;

	bool remove = tabRemoveQuestion(tr("Confirm deletetion", "message"), tr("Do you want to delete items?", "message"));
	if (! remove)
		return;

	list->header()->setSectionsClickable(false);
	list->setDragEnabled(false);
	list->setAcceptDrops(false);

	int tvid = this->state.yx;
	string tnid = this->state.curr;

	auto* dbih = this->data->dbih;

	e2db::tunersets_table tns = dbih->tuners[tvid].tables[tnid];

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

	setPendingUpdateListIndex();

	updateFlags();
	updateStatusBar();

	this->data->setChanged(true);
}

//TODO handle duplicates
void tunersetsView::putListItems(vector<QString> items)
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

	int tvid = this->state.yx;
	string tnid = this->state.curr;

	auto* dbih = this->data->dbih;

	e2db::tunersets tvs = dbih->tuners[tvid];
	e2db::tunersets_table tns = tvs.tables[tnid];

	size_t pad_width = 4;

	for (QString & q : items)
	{
		QString x = QString::number(i++).rightJustified(pad_width, '0');
		QString idx = QString::number(i);

		string trid;
		e2db::tunersets_transponder tntxp;

		QStringList qs;

		if (q.count('\t') == TSV_TABS) // TSV
		{
			qs = q.split('\t');
			char ty = qs[0].isEmpty() ? '\0' : qs[0].toStdString()[0];
			int ytype = dbih->value_transponder_type(ty);

			if (this->state.yx != ytype)
			{
				error("putListItems", tr("Error", "error").toStdString(), tr("Tuner settings transponder type mismatch.", "error").toStdString());

				continue;
			}

			tntxp.freq = qs[1].toInt();

			switch (this->state.yx)
			{
				case e2db::YTYPE::satellite:
					tntxp.pol = dbih->value_transponder_polarization(qs[2].toStdString());
					tntxp.sr = qs[3].toInt();
					tntxp.sys = dbih->value_transponder_system(qs[4].toStdString());
					tntxp.fec = dbih->value_transponder_fec(qs[5].toStdString(), e2db::YTYPE::satellite);
					tntxp.mod = dbih->value_transponder_modulation(qs[6].toStdString(), e2db::YTYPE::satellite);
					tntxp.inv = dbih->value_transponder_inversion(qs[7].toStdString(), e2db::YTYPE::satellite);
					tntxp.rol = dbih->value_transponder_rollof(qs[8].toStdString());
					tntxp.pil = dbih->value_transponder_pilot(qs[9].toStdString());
				break;
				case e2db::YTYPE::terrestrial:
					tntxp.tmod = dbih->value_transponder_modulation(qs[2].toStdString(), e2db::YTYPE::terrestrial);
					tntxp.sys = dbih->value_transponder_system(qs[3].toStdString());
					tntxp.band = dbih->value_transponder_bandwidth(qs[4].toStdString());
					tntxp.tmx = dbih->value_transponder_tmx_mode(qs[5].toStdString());
					tntxp.hpfec = dbih->value_transponder_fec(qs[6].toStdString(), e2db::YTYPE::terrestrial);
					tntxp.lpfec = dbih->value_transponder_fec(qs[7].toStdString(), e2db::YTYPE::terrestrial);
					tntxp.inv = dbih->value_transponder_inversion(qs[8].toStdString(), e2db::YTYPE::terrestrial);
					tntxp.guard = dbih->value_transponder_guard(qs[9].toStdString());
					tntxp.hier = dbih->value_transponder_hier(qs[10].toStdString());
				break;
				case e2db::YTYPE::cable:
					tntxp.cmod = dbih->value_transponder_modulation(qs[2].toStdString(), e2db::YTYPE::cable);
					tntxp.sr = qs[3].toInt();
					tntxp.sys = dbih->value_transponder_system(qs[4].toStdString());
					tntxp.cfec = dbih->value_transponder_fec(qs[5].toStdString(), e2db::YTYPE::cable);
					tntxp.inv = dbih->value_transponder_inversion(qs[6].toStdString(), e2db::YTYPE::cable);
				break;
				case e2db::YTYPE::atsc:
					tntxp.amod = dbih->value_transponder_modulation(qs[2].toStdString(), e2db::YTYPE::atsc);
					tntxp.sys = dbih->value_transponder_system(qs[3].toStdString());
				break;
			}

			char yname = dbih->value_transponder_type(tns.ytype);

			char trid[25];
			std::snprintf(trid, 25, "%c:%04x:%04x", yname, tntxp.freq, tntxp.sr);
			tntxp.trid = trid;
		}
		else if (q.count(':') == 3) // trid
		{
			trid = q.toStdString();
		}
		else
		{
			error("putListItems", tr("Error", "error").toStdString(), tr("Not a valid data format.", "error").toStdString());

			break;
		}

		QStringList entry;

		if (tns.transponders.count(trid))
		{
			tntxp = tns.transponders[trid];
		}
		entry = dbih->entryTunersetsTransponder(tntxp, tns);
		entry.prepend(x);

		QTreeWidgetItem* item = new QTreeWidgetItem(entry);
		item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsDragEnabled | Qt::ItemNeverHasChildren);
		item->setData(ITEM_DATA_ROLE::idx, Qt::UserRole, idx);
		item->setData(ITEM_DATA_ROLE::trid, Qt::UserRole, QString::fromStdString(trid));
		clist.append(item);

		dbih->addTunersetsTransponder(tntxp, tns);
	}

	if (! clist.empty())
	{
		if (current == nullptr)
			list->addTopLevelItems(clist);
		else
			list->insertTopLevelItems(y, clist);

		setPendingUpdateListIndex();

		updateFlags();
		updateStatusBar();

		this->data->setChanged(true);
	}

	list->header()->setSectionsClickable(true);
	list->setDragEnabled(true);
	list->setAcceptDrops(true);
}

void tunersetsView::updateStatusBar(bool current)
{
	debug("updateStatusBar");

	int tvid = this->state.yx;

	gui::status msg;
	msg.update = current;

	auto* dbih = this->data->dbih;

	if (current && ! this->state.curr.empty())
	{
		string tnid = this->state.curr;
		if (tvid == e2db::YTYPE::satellite)
		{
			e2db::tunersets_table tns = dbih->tuners[tvid].tables[tnid];
			msg.curr = dbih->value_transponder_position(tns);
		}

		msg.counters[gui::COUNTER::n_position] = int (dbih->index[tnid].size());
	}
	else
	{
		string iname = "tns:";
		char yname = dbih->value_transponder_type(tvid);
		iname += yname;

		msg.counters[gui::COUNTER::n_transponders] = 0;

		for (auto & x : dbih->index[iname])
		{
			string tnid = x.second;
			msg.counters[gui::COUNTER::n_transponders] += int (dbih->index[tnid].size());
		}
	}

	tabSetStatusBar(msg);
}

void tunersetsView::showTreeEditContextMenu(QPoint &pos)
{
	debug("showTreeEditContextMenu");

	QList<QTreeWidgetItem*> selected = tree->selectedItems();

	if (selected.empty() && tree->topLevelItemCount() != 0)
		return;

	QMenu* tree_edit = contextMenu();

	contextMenuAction(tree_edit, tr("Edit Position", "context-menu"), [=]() { this->editPosition(); }, tabGetFlag(gui::TabTreeEdit));
	contextMenuSeparator(tree_edit);
	contextMenuAction(tree_edit, tr("&Delete", "context-menu"), [=]() { this->treeItemDelete(); }, tabGetFlag(gui::TabTreeDelete));
	contextMenuSeparator(tree_edit);
	contextMenuAction(tree_edit, tr("Edit Settings", "context-menu"), [=]() { this->editSettings(); });

	platform::osContextMenuPopup(tree_edit, tree, pos);
}

void tunersetsView::showListEditContextMenu(QPoint &pos)
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

	contextMenuAction(list_edit, tr("Edit Transponder", "context-menu"), [=]() { this->editTransponder(); }, editable && tabGetFlag(gui::TabListEditTransponder));
	contextMenuSeparator(list_edit);
	contextMenuAction(list_edit, tr("Cu&t", "context-menu"), [=]() { this->listItemCut(); }, tabGetFlag(gui::TabListCut), QKeySequence::Cut);
	contextMenuAction(list_edit, tr("&Copy", "context-menu"), [=]() { this->listItemCopy(); }, tabGetFlag(gui::TabListCopy), QKeySequence::Copy);
	contextMenuAction(list_edit, tr("&Paste", "context-menu"), [=]() { this->listItemPaste(); }, tabGetFlag(gui::TabListPaste), QKeySequence::Paste);
	contextMenuSeparator(list_edit);
	contextMenuAction(list_edit, tr("&Delete", "context-menu"), [=]() { this->listItemDelete(); }, tabGetFlag(gui::TabListDelete), QKeySequence::Delete);

	platform::osContextMenuPopup(list_edit, list, pos);
}

void tunersetsView::updateFlags()
{
	debug("updateFlags");

	if (tree->topLevelItemCount())
	{
		tabSetFlag(gui::TabTreeEdit, true);
		tabSetFlag(gui::TabTreeDelete, true);
		tabSetFlag(gui::TabTreeFind, true);
		this->action.tree_search->setEnabled(true);
	}
	else
	{
		tabSetFlag(gui::TabTreeEdit, false);
		tabSetFlag(gui::TabTreeDelete, false);
		tabSetFlag(gui::TabTreeFind, false);
		this->action.tree_search->setDisabled(true);
	}

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

void tunersetsView::updateTreeIndex()
{
	debug("updateTreeIndex");

	int tvid = this->state.yx;

	auto* dbih = this->data->dbih;

	string iname = "tns:";
	char yname = dbih->value_transponder_type(tvid);
	iname += yname;

	int i = 0, idx = 0;
	int count = tree->topLevelItemCount();
	vector<pair<int, string>> tns;
	unordered_map<string, vector<string>> index;

	while (i != count)
	{
		QTreeWidgetItem* item = tree->topLevelItem(i);
		string tnid = item->data(0, Qt::UserRole).toString().toStdString();
		idx = i + 1;
		tns.emplace_back(pair (idx, tnid));
		i++;
	}
	if (tns != dbih->index[iname])
	{
		dbih->index[iname].swap(tns);
	}
}

void tunersetsView::updateListIndex()
{
	if (! this->state.tvx_pending)
		return;

	string tnid = this->state.curr;

	debug("updateListIndex", "current", tnid);

	int i = 0, idx = 0;
	int count = list->topLevelItemCount();

	auto* dbih = this->data->dbih;

	dbih->index[tnid].clear();

	int sort_column = list->sortColumn();
	Qt::SortOrder sort_order = list->header()->sortIndicatorOrder();
	bool sorted = sort_column != 0 && sort_order != Qt::AscendingOrder;
	sort_column = sort_column == 1 ? 0 : sort_column;

	if (sorted)
		list->sortItems(0, Qt::AscendingOrder);

	while (i != count)
	{
		QTreeWidgetItem* item = list->topLevelItem(i);
		string trid = item->data(ITEM_DATA_ROLE::trid, Qt::UserRole).toString().toStdString();
		idx = i + 1;
		dbih->index[tnid].emplace_back(pair (idx, trid));
		i++;
	}

	if (sorted)
		treeSortItems(list, sort_column, sort_order);

	this->state.tvx_pending = false;
}

void tunersetsView::setPendingUpdateListIndex()
{
	this->state.tvx_pending = true;
}

void tunersetsView::unsetPendingUpdateListIndex()
{
	this->state.tvx_pending = false;
}

void tunersetsView::updateIndex()
{
	updateTreeIndex();
	this->state.tvx_pending = true;
	updateListIndex();
	this->state.tvx_pending = false;
}

}
