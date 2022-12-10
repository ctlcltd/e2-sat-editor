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
#include "gui.h"
#include "todo.h"

using std::to_string;
using namespace e2se;

namespace e2se_gui
{

tunersetsView::tunersetsView(gui* gid, tab* twid, QWidget* wid, int ytype, e2se::logger::session* log)
{
	this->log = new logger(log, "tunersetsView");
	debug("tunersetsView()");

	this->gid = gid;
	this->twid = twid;
	this->cwid = wid;
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
	if (this->sets->value("application/debug", true).toBool()) {
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

	tree_ats->addAction(theme::icon("add"), "New Position", todo);
	tree_ats->addWidget(tree_ats_spacer);
	tree_ats->addWidget(this->action.tree_search);
	this->action.list_newtr = list_ats->addAction(theme::icon("add"), "New Transponder", todo);
	list_ats->addWidget(list_ats_spacer);
	list_ats->addWidget(this->action.list_search);

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

	string iname = "tns:";
	switch (yx)
	{
		case e2db::YTYPE::sat: iname += 's'; break;
		case e2db::YTYPE::terrestrial: iname += 't'; break;
		case e2db::YTYPE::cable: iname += 'c'; break;
		case e2db::YTYPE::atsc: iname += 'a'; break;
	}

	e2db::tunersets tv = dbih->tuners[yx];

	for (auto & x : dbih->index[iname])
	{
		e2db::tunersets_table tn = tv.tables[x.second];
		QTreeWidgetItem* item;
		QString idx = QString::fromStdString(to_string(x.first));
		QString tnid = QString::fromStdString(x.second);
		QString name = QString::fromStdString(tn.name);

		if (yx == e2db::YTYPE::sat)
		{
			char cposdeg[6];
			// %3d.%1d%C
			std::sprintf(cposdeg, "%.1f", float (std::abs (tn.pos)) / 10);
			string ppos = (string (cposdeg) + (tn.pos > 0 ? 'E' : 'W'));

			QString pos = QString::fromStdString(ppos);
			item = new QTreeWidgetItem({idx, name, pos});
		}
		else if (yx == e2db::YTYPE::terrestrial || yx == e2db::YTYPE::cable)
		{
			QString country = QString::fromStdString(tn.country);
			item = new QTreeWidgetItem({idx, name, country});
		}
		else if (yx == e2db::YTYPE::atsc)
		{
			item = new QTreeWidgetItem({idx, name});
		}
		else
		{
			continue;
		}

		item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsDragEnabled | Qt::ItemNeverHasChildren);
		item->setData(0, Qt::UserRole, tnid);
		tree->addTopLevelItem(item);
	}

	tree->setDragEnabled(true);
	tree->setAcceptDrops(true);

	list->setDragEnabled(true);
	list->setAcceptDrops(true);
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

		QTreeWidgetItem* item;
		QString idx = QString::fromStdString(to_string(tp.first));
		QString trid = QString::fromStdString(tp.second);
		QString freq = QString::fromStdString(to_string(txp.freq));

		if (yx == e2db::YTYPE::sat)
		{
			QString combo = QString::fromStdString(to_string(txp.freq) + '/' + e2db::SAT_POL[txp.pol] + '/' + to_string(txp.sr));
			QString pol = QString::fromStdString(e2db::SAT_POL[txp.pol]);
			QString sr = QString::fromStdString(to_string(txp.sr));
			QString fec = QString::fromStdString(e2db::SAT_FEC[txp.fec]);
			QString sys = QString::fromStdString(e2db::SAT_SYS[txp.sys]);
			QString mod = QString::fromStdString(e2db::SAT_MOD[txp.mod]);
			QString inv = QString::fromStdString(e2db::SAT_INV[txp.inv]);
			QString pil = QString::fromStdString(e2db::SAT_PIL[txp.pil]);
			QString rol = QString::fromStdString(e2db::SAT_ROL[txp.rol]);
			item = new QTreeWidgetItem({x, trid, combo, freq, pol, sr, fec, sys, mod, inv, pil, rol});
		}
		else if (yx == e2db::YTYPE::terrestrial)
		{
			QString combo = QString::fromStdString(to_string(txp.freq) + '/' + e2db::TER_MOD[txp.tmod] + '/' + e2db::TER_BAND[txp.band]);
			QString tmod = QString::fromStdString(e2db::TER_MOD[txp.tmod]);
			QString band = QString::fromStdString(e2db::TER_BAND[txp.band]);
			QString sys = "DVB-T";
			QString tmx = QString::fromStdString(e2db::TER_TRXMODE[txp.tmx]);
			QString hpfec = QString::fromStdString(e2db::TER_HPFEC[txp.hpfec]);
			QString lpfec = QString::fromStdString(e2db::TER_LPFEC[txp.lpfec]);
			QString inv = QString::fromStdString(e2db::TER_INV[txp.inv]);
			QString guard = QString::fromStdString(e2db::TER_GUARD[txp.hier]);
			QString hier = QString::fromStdString(e2db::TER_HIER[txp.guard]);
			item = new QTreeWidgetItem({x, trid, combo, freq, tmod, band, sys, tmx, hpfec, lpfec, inv, guard, hier});
		}
		else if (yx == e2db::YTYPE::cable)
		{
			QString combo = QString::fromStdString(to_string(txp.freq) + '/' + e2db::CAB_MOD[txp.cmod] + '/' + to_string(txp.sr));
			QString cmod = QString::fromStdString(e2db::CAB_MOD[txp.cmod]);
			QString sr = QString::fromStdString(to_string(txp.sr));
			QString cfec = QString::fromStdString(e2db::CAB_IFEC[txp.cfec]);
			QString inv = QString::fromStdString(e2db::CAB_INV[txp.inv]);
			QString sys = "DVB-C";
			item = new QTreeWidgetItem({x, trid, combo, freq, cmod, sr, cfec, inv, sys});
		}
		else if (yx == e2db::YTYPE::atsc)
		{
			QString combo = NULL;
			QString amod = QString::fromStdString(to_string(txp.amod));
			QString sys = "ATSC";
			item = new QTreeWidgetItem({x, trid, combo, freq, amod, sys});
		}
		else
		{
			continue;
		}
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

void tunersetsView::preset()
{
	debug("preset()");
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

	list->header()->setSectionsClickable(true);
	list->setDragEnabled(true);
	list->setAcceptDrops(true);

	updateConnectors();
	updateCounters();
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

	updateConnectors();
	updateCounters();
}

void tunersetsView::updateCounters(bool current)
{
	debug("updateCounters()");
}

void tunersetsView::showTreeEditContextMenu(QPoint &pos)
{
	debug("showTreeEditContextMenu()");
}

void tunersetsView::showListEditContextMenu(QPoint &pos)
{
	debug("showListEditContextMenu()");
}

//TODO FIX EXC_BAD_ACCESS
void tunersetsView::updateConnectors()
{
	debug("updateConnectors()");

	if (tree->topLevelItemCount())
	{
		//TODO connect to QScrollArea Event
		/*if (tree->verticalScrollBar()->isVisible())
		{*/
			gid->update(gui::TabTreeFind, true);
			this->action.tree_search->setEnabled(true);
		/*}
		else
		{
			gid->update(gui::TabTreeFind, false);
			this->action.bouquets_search->setEnabled(false);
		}*/
	}
	else
	{
		gid->update(gui::TabTreeFind, false);
		this->action.tree_search->setDisabled(true);
	}

	if (list->topLevelItemCount())
	{
		gid->update(gui::TabListSelectAll, true);
		gid->update(gui::TabListFind, true);
		this->action.list_search->setEnabled(true);
	}
	else
	{
		gid->update(gui::TabListSelectAll, false);
		gid->update(gui::TabListFind, false);
		this->action.list_search->setDisabled(true);
	}

	gid->update(gui::TabTreeFindNext, false);
	gid->update(gui::TabListFindNext, false);
	gid->update(gui::TabListFindPrev, false);
	gid->update(gui::TabListFindAll, false);

	gid->update(gui::ToolsTunersetsSat, true);
	gid->update(gui::ToolsTunersetsTerrestrial, true);
	gid->update(gui::ToolsTunersetsCable, true);
	gid->update(gui::ToolsTunersetsAtsc, true);

	// this->gxe = gid->getActionFlags();
}

}
