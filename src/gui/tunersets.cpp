/*!
 * e2-sat-editor/src/gui/tunersets.cpp
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

#include "tunersets.h"
#include "theme.h"
#include "todo.h"

using std::to_string;
using namespace e2se;

namespace e2se_gui
{

tunersets::tunersets(e2db* dbih, int ytype, e2se::logger::session* log)
{
	this->log = new logger(log, "tunersets");
	debug("tunersets()");

	this->dbih = dbih;
	this->yx = ytype;
	this->sets = new QSettings;

	QGridLayout* afrm = new QGridLayout();

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

	layout();
	load();

	tfrm->setLayout(tbox);
	lfrm->setLayout(lbox);

	swid->addWidget(tfrm);
	swid->addWidget(lfrm);

	swid->setStretchFactor(0, 1);
	swid->setStretchFactor(1, 5);

	afrm->addWidget(swid, 0, 0);
	afrm->setSpacing(0);
	afrm->setContentsMargins(0, 0, 0, 0);

	this->widget = new QWidget;
	widget->setLayout(afrm);
}

void tunersets::layout()
{
	debug("layout()");

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
	list->header()->connect(list->header(), &QHeaderView::sectionClicked, [=](int column) { this->trickySortByColumn(column); });

	this->tree_search = new QWidget;
	this->list_search = new QWidget;
	tree_search->setHidden(true);
	list_search->setHidden(true);
	tree_search->setBackgroundRole(QPalette::Mid);
	list_search->setBackgroundRole(QPalette::Mid);
	tree_search->setAutoFillBackground(true);
	list_search->setAutoFillBackground(true);

	QGridLayout* tsr_box = new QGridLayout(tree_search);
	tsr_box->setContentsMargins(4, 3, 3, 6);
	tsr_box->setSpacing(0);
	this->tsr_search.input = new QLineEdit;
	this->tsr_search.input->connect(this->tsr_search.input, &QLineEdit::textChanged, [=](const QString& text) { this->treeFindPerform(text); });
	this->tsr_search.next = new QPushButton("Find");
	this->tsr_search.next->setStyleSheet("QPushButton, QPushButton:pressed { margin: 0 2px; padding: 3px 2ex; border: 1px solid transparent; border-radius: 3px; background: palette(button) } QPushButton:pressed { background: palette(light) }");
	this->tsr_search.next->connect(this->tsr_search.next, &QPushButton::pressed, [=]() { this->treeFindPerform(); });
	this->tsr_search.close = new QPushButton;
	this->tsr_search.close->setIconSize(QSize(10, 10));
	this->tsr_search.close->setIcon(theme::icon("close"));
	this->tsr_search.close->setFlat(true);
	this->tsr_search.close->setMaximumWidth(22);
	this->tsr_search.close->connect(this->tsr_search.close, &QPushButton::pressed, [=]() { this->treeSearchClose(); });
	tsr_box->addItem(new QSpacerItem(5, 0), 0, 0);
	tsr_box->addWidget(this->tsr_search.input, 0, 1);
	tsr_box->addItem(new QSpacerItem(2, 0), 0, 2);
	tsr_box->addWidget(this->tsr_search.next, 0, 3);
	tsr_box->addItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Preferred), 0, 4);
	tsr_box->addWidget(this->tsr_search.close, 0, 5);

	QGridLayout* lsr_box = new QGridLayout(list_search);
	lsr_box->setContentsMargins(4, 3, 3, 6);
	lsr_box->setSpacing(0);
	this->lsr_search.filter = new QComboBox;
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
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
	this->lsr_search.filter->connect(this->lsr_search.filter, &QComboBox::currentIndexChanged, [=]() { this->listFindReset(); });
#else
	this->lsr_search.filter->connect(this->lsr_search.filter, QOverload<int>::of(&QComboBox::currentIndexChanged), [=]() { this->listFindReset(); });
#endif
	this->lsr_search.input = new QLineEdit;
	this->lsr_search.input->setStyleSheet("padding: 2px 0");
	this->lsr_search.input->connect(this->lsr_search.input, &QLineEdit::textChanged, [=](const QString& text) { this->listFindPerform(text, LIST_FIND::fast); });
	this->lsr_search.highlight = new QPushButton;
	this->lsr_search.highlight->setText("Highlight");
	this->lsr_search.highlight->setCheckable(true);
	this->lsr_search.highlight->setChecked(true);
	this->lsr_search.highlight->setStyleSheet("QPushButton, QPushButton:checked { margin: 0 2px; padding: 2px 2ex; border: 1px solid palette(button); border-radius: 2px; background: palette(mid) } QPushButton:checked { background: rgb(9, 134, 211) }");
	this->lsr_search.highlight->connect(this->lsr_search.highlight, &QPushButton::pressed, [=]() { this->listFindHighlightToggle(); });
	this->lsr_search.next = new QPushButton("Find");
	this->lsr_search.next->setStyleSheet("QPushButton, QPushButton:pressed { margin: 0 2px; padding: 3px 2ex; border: 1px solid transparent; border-radius: 3px; background: palette(button) } QPushButton:pressed { background: palette(light) }");
	this->lsr_search.next->connect(this->lsr_search.next, &QPushButton::pressed, [=]() { this->listFindPerform(LIST_FIND::next); });
	this->lsr_search.prev = new QPushButton("Find Previous");
	this->lsr_search.prev->setStyleSheet("QPushButton, QPushButton:pressed { margin: 0 2px; padding: 3px 2ex; border: 1px solid transparent; border-radius: 3px; background: palette(button) } QPushButton:pressed { background: palette(light) }");
	this->lsr_search.prev->connect(this->lsr_search.prev, &QPushButton::pressed, [=]() { this->listFindPerform(LIST_FIND::prev); });
	this->lsr_search.all = new QPushButton("Find All");
	this->lsr_search.all->setStyleSheet("QPushButton, QPushButton:pressed { margin: 0 2px; padding: 3px 2ex; border: 1px solid transparent; border-radius: 3px; background: palette(button) } QPushButton:pressed { background: palette(light) }");
	this->lsr_search.all->connect(this->lsr_search.all, &QPushButton::pressed, [=]() { this->listFindPerform(LIST_FIND::all); });
	this->lsr_search.close = new QPushButton;
	this->lsr_search.close->setIconSize(QSize(10, 10));
	this->lsr_search.close->setIcon(theme::icon("close"));
	this->lsr_search.close->setFlat(true);
	this->lsr_search.close->setMaximumWidth(28);
	this->lsr_search.close->connect(this->lsr_search.close, &QPushButton::pressed, [=]() { this->listSearchClose(); });
	lsr_box->addWidget(this->lsr_search.filter, 0, 0);
	lsr_box->addWidget(this->lsr_search.input, 0, 1);
	lsr_box->addItem(new QSpacerItem(2, 0), 0, 2);
	lsr_box->addWidget(this->lsr_search.next, 0, 3);
	lsr_box->addWidget(this->lsr_search.prev, 0, 4);
	lsr_box->addWidget(this->lsr_search.all, 0, 5);
	lsr_box->addItem(new QSpacerItem(16, 0), 0, 6);
	lsr_box->addWidget(this->lsr_search.highlight, 0, 7);
	lsr_box->addItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Preferred), 0, 8);
	lsr_box->addWidget(this->lsr_search.close, 0, 9);

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
}

void tunersets::treeItemChanged()
{
	debug("treeItemChanged()");

	list->clearSelection();
	list->scrollToTop();
	list->clear();
	list->setDragEnabled(false);
	list->setAcceptDrops(false);

	populate();
}

void tunersets::load()
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

void tunersets::populate()
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

void tunersets::trickySortByColumn(int column)
{
	debug("trickySortByColumn()", "column", column);

	Qt::SortOrder order = list->header()->sortIndicatorOrder();
	column = column == 1 ? 0 : column;

	// sorting by
	if (column)
	{
		list->sortItems(column, order);
		list->header()->setSortIndicatorShown(true);
	}
	// sorting default
	else
	{
		list->sortItems(column, order);
		list->header()->setSortIndicator(1, order);

		// default column 0|asc
		if (order == Qt::AscendingOrder)
			list->header()->setSortIndicatorShown(false);
		else
			list->header()->setSortIndicatorShown(true);
	}
}

void tunersets::treeSearchShow()
{
	debug("treeSearchShow()");

	tree_search->show();
}

void tunersets::treeSearchHide()
{
	debug("treeSearchHide()");

	tree_search->hide();
}

void tunersets::treeSearchToggle()
{
	// debug("treeSearchToggle()");

	if (tree_search->isHidden())
	{
		treeSearchShow();
	}
	else
	{
		treeSearchHide();
	}
}

void tunersets::treeSearchClose()
{
	// debug("treeSearchClose()");

	QTimer::singleShot(100, [=]() {
		treeSearchHide();
	});
}

void tunersets::listSearchShow()
{
	debug("listSearchShow()");

	list_search->show();
	this->lsr_find.timer.start();
}

void tunersets::listSearchHide()
{
	debug("listSearchHide()");

	list_search->hide();
	if (! this->lsr_find.highlight)
		listFindClear();
}

void tunersets::listSearchToggle()
{
	// debug("listSearchToggle()");

	if (list_search->isHidden())
		listSearchShow();
	else
		listSearchHide();
}

void tunersets::listSearchClose()
{
	// debug("listSearchClose()");

	QTimer::singleShot(100, [=]() {
		listSearchHide();
	});
}

void tunersets::treeFindPerform()
{
	if (this->tsr_search.input->text().isEmpty())
		return;

	treeFindPerform(this->tsr_search.input->text());
}

void tunersets::treeFindPerform(const QString& value)
{
	tree->keyboardSearch(value);
}

void tunersets::listFindPerform(LIST_FIND flag)
{
	if (this->lsr_search.input->text().isEmpty())
		return;

	listFindPerform(this->lsr_search.input->text(), flag);
}

//TODO FIX multiple selection with shortcut FindNext when search_box is closed
void tunersets::listFindPerform(const QString& value, LIST_FIND flag)
{
	// QTreeWidgetItem* item list->currentItem() || list->topLevelItem(0)
	// int column || QTreeWidget::currentColumn()
	// void QTreeWidget::setCurrentItem(QTreeWidgetItem* item, int column)
	// void QTreeView::keyboardSearch(const QString& search)

	int column = this->lsr_search.filter->currentData().toInt();
	int delay = flag == LIST_FIND::fast ? QApplication::keyboardInputInterval() : 0;
	QString text;

	if (flag == LIST_FIND::fast)
	{
		bool keyboardTimeWasValid = this->lsr_find.timer.isValid();
		qint64 elapsed;
		if (keyboardTimeWasValid)
			elapsed = this->lsr_find.timer.restart();
		else
			this->lsr_find.timer.start();
		if (value.isEmpty() || ! keyboardTimeWasValid || elapsed > delay)
		{
			text = value;

			listFindClear();
		}
		else
		{
			text += value;
		}
	}
	else
	{
		if (value.isEmpty())
			return;
		else
			text = value;
	}

	QModelIndexList match;

	if (this->lsr_find.match.isEmpty() || this->lsr_find.filter != column || this->lsr_find.input != text)
	{
		// fast 0 --> start
		// fast i match(..., ..., ..., 1, ...)
		//
		// QModelIndex start;
		// if (list_tree->currentIndex().isValid())
		//	start = list_tree->currentIndex();
		// else
		// 	start = list_tree->model()->index(0, 0);
		//
		QModelIndex start = list->model()->index(0, column);
		int limit = -1;
		match = list->model()->match(start, Qt::DisplayRole, text, limit, Qt::MatchFlag::MatchContains);

		if (this->lsr_find.flag == LIST_FIND::all)
			listFindClear();

		this->lsr_find.curr = -1;
	}
	else
	{
		match = this->lsr_find.match;

		//TODO improve
		/*if (flag == LIST_FIND::fast && this->lsr_find.flag == LIST_FIND::all)
		{
			if (list->currentIndex().isValid())
				return;
			else
				this->lsr_find.curr = -1;
		}*/
	}

	if (match.count())
	{
		LIST_FIND type = flag == LIST_FIND::fast ? LIST_FIND::next : flag;
		int i = 0;
		int j = match.size();

		if (type == LIST_FIND::next)
		{
			i = int (this->lsr_find.curr);
			i = i == j - 1 ? 0 : i + 1;
			list->setCurrentIndex(match.at(i));
		}
		else if (type == LIST_FIND::prev)
		{
			i = int (this->lsr_find.curr);
			i = i <= 0 ? j - 1 : i - 1;
			list->setCurrentIndex(match.at(i));
		}
		else if (type == LIST_FIND::all)
		{

			listFindClear(false);
			while (i != j)
			{
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
				QTreeWidgetItem* item = list->itemFromIndex(match.at(i));
#else
				QTreeWidgetItem* item;
				QModelIndex index = match.at(i);
				if (index.isValid())
					item = static_cast<QTreeWidgetItem*>(index.internalPointer());
#endif

				if (this->lsr_find.highlight)
					item->setSelected(true);
				else
					item->setHidden(false);
				i++;
			}
			if (this->lsr_find.highlight)
				list->scrollTo(match.at(0));
			i = -1;
		}
		// int i = list_tree->indexOfTopLevelItem(QTreeWidgetItem* item);
		this->lsr_find.curr = i;
	}

	this->lsr_find.flag = flag;
	this->lsr_find.filter = column;
	this->lsr_find.input = text;
	this->lsr_find.match = match;
}

void tunersets::listFindHighlightToggle()
{
	listFindReset();
	this->lsr_find.highlight = ! this->lsr_find.highlight;
}

void tunersets::listFindClear(bool hidden)
{
	list->clearSelection();

	if (! this->lsr_find.highlight)
	{
		int j = list->topLevelItemCount();
		while (j--)
		{
			QTreeWidgetItem* item = list->topLevelItem(j);
			item->setHidden(! hidden);
		}
	}
}

void tunersets::listFindReset()
{
	listFindClear();

	this->lsr_find.flag = LIST_FIND::fast;
	this->lsr_find.filter = 0;
	this->lsr_find.highlight = true;
	this->lsr_find.curr = -1;
	this->lsr_find.input = "";
	this->lsr_find.match.clear();
	this->lsr_find.timer.invalidate();
}

}
