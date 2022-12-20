/*!
 * e2-sat-editor/src/gui/mainView.cpp
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
#include <algorithm>
#include <filesystem>

#include <QtGlobal>
#include <QTimer>
#include <QList>
#include <QGridLayout>
#include <QSplitter>
#include <QGroupBox>
#include <QHeaderView>
#include <QToolBar>
#include <QStyle>
#include <QMessageBox>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QSplitter>
#include <QGroupBox>
#include <QToolBar>
#include <QMenu>
#include <QScrollArea>
#include <QClipboard>
#include <QMimeData>

#include "toolkit/TreeStyledItemDelegate.h"
#include "mainView.h"
#include "theme.h"
#include "tab.h"
#include "gui.h"
#include "editBouquet.h"
#include "editService.h"
#include "editMarker.h"
#include "dialChannelBook.h"
// #include "channelBookView.h"

using std::to_string, std::sort;
using namespace e2se;

namespace e2se_gui
{

mainView::mainView(tab* twid, QWidget* wid, dataHandler* data, e2se::logger::session* log)
{
	this->log = new logger(log, "mainView");
	debug("mainView()");

	this->twid = twid;
	this->cwid = wid;
	this->data = data;
	this->sets = new QSettings;
	this->widget = new QWidget;

	layout();
}

void mainView::layout()
{
	debug("layout()");

	widget->setStyleSheet("QGroupBox { spacing: 0; padding: 20px 0 0 0; border: 0 } QGroupBox::title { margin: 0 12px }");

	QGridLayout* frm = new QGridLayout(widget);

	//TODO bouquets_box and vertical scrollbar hSize in GTK+
	QSplitter* splitterc = new QSplitter;

	QVBoxLayout* side_box = new QVBoxLayout;
	QVBoxLayout* list_box = new QVBoxLayout;

	QWidget* side = new QWidget;
	QVBoxLayout* services_box = new QVBoxLayout;
	QVBoxLayout* bouquets_box = new QVBoxLayout;

	QGroupBox* services = new QGroupBox("Services");
	QGroupBox* bouquets = new QGroupBox("Bouquets");
	QGroupBox* channels = new QGroupBox("Channels");

	services_box->setContentsMargins(12, 12, 12, 0);
	bouquets_box->setContentsMargins(12, 12, 12, 12);
	side_box->setSpacing(4);
	list_box->setSpacing(0);
	services_box->setSpacing(0);
	bouquets_box->setSpacing(0);
	services->setFlat(true);
	bouquets->setFlat(true);
	channels->setFlat(true);

	QGridLayout* list_layout = new QGridLayout;
	this->list_wrap = new QWidget;
	list_wrap->setObjectName("channels_wrap");
	list_wrap->setStyleSheet("#channels_wrap { background: transparent }");

	this->services_tree = new QTreeWidget;
	this->tree = new QTreeWidget;
	this->list = new QTreeWidget;
	services_tree->setStyleSheet("QTreeWidget { background: transparent } ::item { padding: 9px auto }");
	tree->setStyleSheet("QTreeWidget { background: transparent } ::item { margin: 1px 0 0; padding: 8px auto }");
	list->setStyleSheet("::item { padding: 6px auto }");

	services_tree->setHeaderHidden(true);
	services_tree->setUniformRowHeights(true);
	tree->setHeaderHidden(true);
	tree->setUniformRowHeights(true);
	list->setUniformRowHeights(true);
	
	services_tree->setRootIsDecorated(false);
	services_tree->setItemsExpandable(false);
	services_tree->setExpandsOnDoubleClick(false);

	tree->setSelectionBehavior(QAbstractItemView::SelectRows);
	tree->setDropIndicatorShown(true);
	tree->setDragDropMode(QAbstractItemView::DragDrop);
	tree->setEditTriggers(QAbstractItemView::NoEditTriggers);

	list->setRootIsDecorated(false);
	list->setSelectionBehavior(QAbstractItemView::SelectRows);
	list->setSelectionMode(QAbstractItemView::ExtendedSelection);
	list->setItemsExpandable(false);
	list->setExpandsOnDoubleClick(false);
	list->setDropIndicatorShown(true);
	list->setDragDropMode(QAbstractItemView::InternalMove);
	list->setEditTriggers(QAbstractItemView::NoEditTriggers);
	
	TreeStyledItemDelegate* bouquets_delegate = new TreeStyledItemDelegate(tree);
	bouquets_delegate->setIndentation(tree->indentation());
	tree->setItemDelegateForColumn(0, bouquets_delegate);

	QTreeWidgetItem* lheader_item = new QTreeWidgetItem({NULL, "Index", "Name", "CHID", "TXID", "Service ID", "Transport ID", "Type", "CAS", "Provider", "Frequency", "Polarization", "Symbol Rate", "FEC", "Position", "System"});

	list->setHeaderItem(lheader_item);
	list->setColumnHidden(ITEM_ROW_ROLE::x, true);		// hidden index
	list->setColumnWidth(ITEM_ROW_ROLE::chnum, 65);	// (Channel Number) Index
	list->setColumnWidth(ITEM_ROW_ROLE::chname, 200);	// (Channel) Name
	if (sets->value("application/debug", true).toBool()) {
		list->setColumnWidth(ITEM_ROW_ROLE::debug_chid, 175);
		list->setColumnWidth(ITEM_ROW_ROLE::debug_txid, 150);
	}
	else
	{
		list->setColumnHidden(ITEM_ROW_ROLE::debug_chid, true);
		list->setColumnHidden(ITEM_ROW_ROLE::debug_txid, true);
	}
	list->setColumnWidth(ITEM_ROW_ROLE::chssid, 80);	// Service ID
	list->setColumnWidth(ITEM_ROW_ROLE::chtsid, 80);	// Transport ID
	list->setColumnWidth(ITEM_ROW_ROLE::chtype, 85);	// (Channel) Type
	list->setColumnWidth(ITEM_ROW_ROLE::chcas, 45);	// CAS
	list->setColumnWidth(ITEM_ROW_ROLE::chpname, 150);	// Provider
	list->setColumnWidth(ITEM_ROW_ROLE::chfreq, 95);	// Frequency
	list->setColumnWidth(ITEM_ROW_ROLE::chpol, 85);	// Polarization
	list->setColumnWidth(ITEM_ROW_ROLE::chsr, 95);		// Symbol Rate
	list->setColumnWidth(ITEM_ROW_ROLE::chfec, 50);	// FEC
	list->setColumnWidth(ITEM_ROW_ROLE::chpos, 125);	// Position
	list->setColumnWidth(ITEM_ROW_ROLE::chsys, 75);	// System

	list->header()->connect(list->header(), &QHeaderView::sectionClicked, [=](int column) { this->sortByColumn(column); });

	tree->setContextMenuPolicy(Qt::CustomContextMenu);
	tree->connect(tree, &QTreeWidget::customContextMenuRequested, [=](QPoint pos) { this->showTreeEditContextMenu(pos); });
	list->setContextMenuPolicy(Qt::CustomContextMenu);
	list->connect(list, &QTreeWidget::customContextMenuRequested, [=](QPoint pos) { this->showListEditContextMenu(pos); });

	searchLayout();
	refboxLayout();

	QToolBar* tree_ats = new QToolBar;
	tree_ats->setIconSize(QSize(12, 12));
	tree_ats->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
	tree_ats->setStyleSheet("QToolButton { font: bold 14px }");
	QToolBar* list_ats = new QToolBar;
	list_ats->setIconSize(QSize(12, 12));
	list_ats->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
	list_ats->setStyleSheet("QToolButton { font: bold 14px }");

	this->action.list_dnd = new QPushButton;
	this->action.list_dnd->setText("Drag&&Drop");
	this->action.list_dnd->setCheckable(true);
	this->action.list_dnd->setDisabled(true);
	this->action.list_dnd->connect(this->action.list_dnd, &QPushButton::pressed, [=]() { this->reharmDnD(); });

	this->action.list_ref = new QPushButton;
	this->action.list_ref->setText("Reference");
	this->action.list_ref->setCheckable(true);
	this->action.list_ref->connect(this->action.list_ref, &QPushButton::pressed, [=]() { this->listReferenceToggle(); });

	this->action.tree_search = new QPushButton;
	this->action.tree_search->setText("Find…");
	this->action.tree_search->setIcon(theme::icon("search"));
	this->action.tree_search->connect(this->action.tree_search, &QPushButton::pressed, [=]() { this->treeSearchToggle(); });
	this->action.tree_search->setDisabled(true);

	this->action.list_search = new QPushButton;
	this->action.list_search->setText("&Find…");
	this->action.list_search->setIcon(theme::icon("search"));
	this->action.list_search->connect(this->action.list_search, &QPushButton::pressed, [=]() { this->listSearchToggle(); });

	QWidget* tree_ats_spacer = new QWidget;
	tree_ats_spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	QWidget* list_ats_spacer = new QWidget;
	list_ats_spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

	tree_ats->addAction(theme::icon("add"), "New Bouquet", [=]() { this->addUserbouquet(); });
	tree_ats->addWidget(tree_ats_spacer);
	tree_ats->addWidget(this->action.tree_search);
	this->action.list_addch = list_ats->addAction(theme::icon("add"), "Add Channel", [=]() { this->addChannel(); });
	this->action.list_addch->setDisabled(true);
	this->action.list_addmk = list_ats->addAction(theme::icon("add"), "Add Marker", [=]() { this->addMarker(); });
	this->action.list_addmk->setDisabled(true);
	this->action.list_newch = list_ats->addAction(theme::icon("add"), "New Service", [=]() { this->addService(); });
	list_ats->addSeparator();
	list_ats->addWidget(this->action.list_ref);
	list_ats->addWidget(this->action.list_dnd);
	list_ats->addWidget(list_ats_spacer);
	list_ats->addWidget(this->action.list_search);

	this->bouquets_evth = new BouquetsEventHandler;
	this->list_evth = new ListEventHandler;
	this->list_evto = new ListEventObserver;
	services_tree->connect(services_tree, &QTreeWidget::itemPressed, [=](QTreeWidgetItem* item) { this->treeSwitched(services_tree, item); });
	services_tree->connect(services_tree, &QTreeWidget::currentItemChanged, [=](QTreeWidgetItem* current) { this->servicesItemChanged(current); });
	tree->viewport()->installEventFilter(bouquets_evth);
	tree->connect(tree, &QTreeWidget::itemPressed, [=](QTreeWidgetItem* item) { this->treeSwitched(tree, item); });
	tree->connect(tree, &QTreeWidget::currentItemChanged, [=](QTreeWidgetItem* current) { this->bouquetsItemChanged(current); });
	list->installEventFilter(list_evto);
	list->viewport()->installEventFilter(list_evth);
	list->connect(list, &QTreeWidget::currentItemChanged, [=]() { this->listItemChanged(); });
	list->connect(list, &QTreeWidget::itemSelectionChanged, [=]() { this->listItemSelectionChanged(); });
	list->connect(list, &QTreeWidget::itemDoubleClicked, [=]() { this->listItemDoubleClicked(); });

	services_box->addWidget(services_tree);
	services->setLayout(services_box);

	bouquets_box->addWidget(tree);
	bouquets_box->addWidget(tree_search);
	bouquets_box->addWidget(tree_ats);
	bouquets->setLayout(bouquets_box);

	side_box->addWidget(services);
	side_box->addItem(new QSpacerItem(0, 8, QSizePolicy::Preferred, QSizePolicy::Fixed));
	side_box->addWidget(bouquets, 1);
	side_box->setContentsMargins(0, 0, 0, 0);
	side->setLayout(side_box);

	list_layout->addWidget(list);
	list_layout->setContentsMargins(0, 0, 0, 0);
	// list_layout->setContentsMargins(3, 3, 3, 3); // #channels_wrap
	list_wrap->setLayout(list_layout);

	list_box->addWidget(list_wrap);
	list_box->addWidget(list_search);
	list_box->addWidget(list_reference);
	list_box->addWidget(list_ats);
	channels->setLayout(list_box);

	side->setMinimumWidth(250);
	channels->setMinimumWidth(510);

	splitterc->addWidget(side);
	splitterc->addWidget(channels);
	splitterc->setStretchFactor(0, 1);
	splitterc->setStretchFactor(1, 4);

	frm->addWidget(splitterc);
	frm->setContentsMargins(0, 0, 0, 0);

	vector<pair<QString, QString>> tree = {
		{"chs", "All services"},
		{"chs:1", "TV"},
		{"chs:2", "Radio"},
		{"chs:0", "Data"}
	};

	for (auto & item : tree)
	{
		QTreeWidgetItem* titem = new QTreeWidgetItem();
		titem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
		QVariantMap tdata; // singular data
		tdata["id"] = item.first;
		titem->setData(0, Qt::UserRole, QVariant (tdata));
		titem->setText(0, item.second);
		titem->setIcon(0, theme::spacer(2));
		services_tree->addTopLevelItem(titem);
	}
}

void mainView::searchLayout()
{
	this->viewAbstract::searchLayout();

	this->lsr_search.filter->addItem("Name", ITEM_ROW_ROLE::chname);
	this->lsr_search.filter->addItem("Type", ITEM_ROW_ROLE::chtype);
	this->lsr_search.filter->addItem("CAS", ITEM_ROW_ROLE::chcas);
	this->lsr_search.filter->addItem("Provider", ITEM_ROW_ROLE::chpname);
	this->lsr_search.filter->addItem("Position", ITEM_ROW_ROLE::chpos);
}

void mainView::refboxLayout()
{

	this->list_reference = new QWidget;
	list_reference->setHidden(true);
	list_reference->setBackgroundRole(QPalette::Mid);
	list_reference->setAutoFillBackground(true);

	QGridLayout* ref_frm = new QGridLayout(list_reference);
	QScrollArea* ref_area = new QScrollArea;
	QWidget* ref_wrap = new QWidget;
	QGridLayout* ref_box = new QGridLayout;
	QLabel* ref0lr = new QLabel("Reference ID");
	QLabel* ref0tr = new QLabel("< >");
	ref0lr->setFont(QFont(theme::fontFamily(), theme::calcFontSize(-2)));
	ref_fields[LIST_REF::ReferenceID] = ref0tr;
	ref_box->addWidget(ref0lr, 0, 0, Qt::AlignTop);
	ref_box->addWidget(ref0tr, 0, 1, Qt::AlignTop);
	QLabel* ref1ls = new QLabel("Service ID");
	QLabel* ref1ts = new QLabel("< >");
	ref1ls->setFont(QFont(theme::fontFamily(), theme::calcFontSize(-2)));
	ref_fields[LIST_REF::ServiceID] = ref1ts;
	ref_box->addWidget(ref1ls, 0, 2, Qt::AlignTop);
	ref_box->addWidget(ref1ts, 0, 3, Qt::AlignTop);
	QLabel* ref2lt = new QLabel("Transponder");
	QLabel* ref2tt = new QLabel("< >");
	ref2lt->setFont(QFont(theme::fontFamily(), theme::calcFontSize(-2)));
	ref_fields[LIST_REF::Transponder] = ref2tt;
	ref_box->addWidget(ref2lt, 0, 4, Qt::AlignTop);
	ref_box->addWidget(ref2tt, 0, 5, Qt::AlignTop);
	ref_box->addItem(new QSpacerItem(0, 12), 1, 0);
	QLabel* ref3lu = new QLabel("Userbouquets");
	QLabel* ref3tu = new QLabel("< >");
	ref3lu->setFont(QFont(theme::fontFamily(), theme::calcFontSize(-2)));
	ref_fields[LIST_REF::Userbouquets] = ref3tu;
	ref_box->addWidget(ref3lu, 2, 0, Qt::AlignTop);
	ref_box->addWidget(ref3tu, 2, 1, Qt::AlignTop);
	QLabel* ref4lb = new QLabel("Bouquets");
	QLabel* ref4tb = new QLabel("< >");
	ref4lb->setFont(QFont(theme::fontFamily(), theme::calcFontSize(-2)));
	ref_fields[LIST_REF::Bouquets] = ref4tb;
	ref_box->addWidget(ref4lb, 2, 2, Qt::AlignTop);
	ref_box->addWidget(ref4tb, 2, 3, Qt::AlignTop);
	QLabel* ref5ln = new QLabel("Tuner");
	QLabel* ref5tn = new QLabel("< >");
	ref5ln->setFont(QFont(theme::fontFamily(), theme::calcFontSize(-2)));
	ref_fields[LIST_REF::Tuner] = ref5tn;
	ref_box->addWidget(ref5ln, 2, 4, Qt::AlignTop);
	ref_box->addWidget(ref5tn, 2, 5, Qt::AlignTop);
	ref_box->setRowStretch(2, 1);
	ref_box->setColumnStretch(5, 1);
	ref_box->setColumnMinimumWidth(1, 300);
	ref_box->setColumnMinimumWidth(3, 200);
	ref_box->setColumnMinimumWidth(5, 200);
	ref_area->setWidget(ref_wrap);
	ref_area->setWidgetResizable(true);
	ref_wrap->setLayout(ref_box);
	ref_frm->addWidget(ref_area);
	ref_frm->setContentsMargins(0, 0, 0, 0);
	list_reference->setFixedHeight(100);
}

void mainView::load()
{
	debug("load()");

	tabUpdateFlags(gui::init);

	this->dbih = this->data->dbih;

	unordered_map<string, QTreeWidgetItem*> bgroups;

	for (auto & bsi : dbih->index["bss"])
	{
		debug("load()", "bouquet", bsi.second);
		e2db::bouquet gboq = dbih->bouquets[bsi.second];
		QString bname = QString::fromStdString(bsi.second);
		QString name = QString::fromStdString(gboq.nname.empty() ? gboq.name : gboq.nname);

		QTreeWidgetItem* bitem = new QTreeWidgetItem();
		bitem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
		QMap<QString, QVariant> tdata; // singular data
		tdata["id"] = bname;
		bitem->setData(0, Qt::UserRole, QVariant (tdata));
		bitem->setText(0, name);
		tree->addTopLevelItem(bitem);
		tree->expandItem(bitem);
	
		for (string & ubname : gboq.userbouquets)
			bgroups[ubname] = bitem;
	}
	for (auto & ubi : dbih->index["ubs"])
	{
		debug("load()", "userbouquet", ubi.second);
		e2db::userbouquet uboq = dbih->userbouquets[ubi.second];
		QString bname = QString::fromStdString(ubi.second);
		QTreeWidgetItem* pgroup = bgroups[ubi.second];
		// macos: unwanted chars [qt.qpa.fonts] Menlo notice
		QString name;
		if (sets->value("preference/fixUnicodeChars").toBool())
			name = QString::fromStdString(uboq.name).remove(QRegularExpression("[^\\p{L}\\p{M}\\p{N}\\p{P}\\p{S}\\s]+"));
		else
			name = QString::fromStdString(uboq.name);

		QTreeWidgetItem* bitem = new QTreeWidgetItem(pgroup);
		bitem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled | Qt::ItemNeverHasChildren);
		QMap<QString, QVariant> tdata; // singular data
		tdata["id"] = bname;
		bitem->setData(0, Qt::UserRole, QVariant (tdata));
		bitem->setText(0, name);
		tree->addTopLevelItem(bitem);
	}

	tree->setDragEnabled(true);
	tree->setAcceptDrops(true);
	services_tree->setCurrentItem(services_tree->topLevelItem(0));
	populate(services_tree);
	updateFlags();
	updateStatus();
}

void mainView::reset()
{
	debug("reset()");

	this->state.dnd = true;
	this->state.changed = false;
	this->state.reindex = false;
	this->state.refbox = list_reference->isVisible();
	this->state.tc = 0;
	this->state.ti = 0;
	this->state.curr = "";
	this->state.sort = pair (-1, Qt::AscendingOrder); //C++17

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
	cache.clear();

	this->lsr_find.curr = -1;
	this->lsr_find.match.clear();

	this->action.list_addch->setDisabled(true);
	this->action.list_addmk->setDisabled(true);
	this->action.list_newch->setEnabled(true);
	this->action.list_dnd->setDisabled(true);

	tabResetStatus();

	this->dbih = nullptr;
}

void mainView::populate(QTreeWidget* side_tree)
{
	string curr;
	string prev;
	bool precached = false;
	QList<QTreeWidgetItem*> cachep;
	
	if (! cache.empty())
	{
		for (int i = 0; i < list->topLevelItemCount(); i++)
		{
			QTreeWidgetItem* item = list->topLevelItem(i);
			cachep.append(item->clone());
		}
		precached = true;
		prev = string (this->state.curr);
	}

	QTreeWidgetItem* selected = side_tree->currentItem();
	if (selected == NULL)
	{
		return;
	}
	if (selected != NULL)
	{
		QVariantMap tdata = selected->data(0, Qt::UserRole).toMap();
		QString curr_bouquet = tdata["id"].toString();
		curr = curr_bouquet.toStdString();
		this->state.curr = curr;
	}

	if (dbih->index.count(curr))
		debug("populate()", "curr", curr);
	else
		error("populate()", "curr", curr);

	list->header()->setSortIndicatorShown(true);
	list->header()->setSectionsClickable(false);
	list->clear();
	if (precached)
	{
		cache[prev].swap(cachep);
	}

	int i = 0;

	if (cache[curr].isEmpty())
	{
		for (auto & ch : dbih->index[curr])
		{
			char ci[7];
			std::sprintf(ci, "%06d", i++);
			bool marker = false;
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
				e2db::channel_reference chref;
				if (dbih->userbouquets.count(curr))
					chref = dbih->userbouquets[curr].channels[ch.second];

				if (chref.marker)
				{
					marker = true;
					entry = dbih->entryMarker(chref);
					idx = entry[1];
					entry.prepend(x);
				}
				else
				{
					//TEST
					entry = QStringList({x, NULL, NULL, chid, NULL, "ERROR", NULL});
					// idx = 0; //Qt5
					error("populate()", "chid", ch.second);
					//TEST
				}
			}

			QTreeWidgetItem* item = new QTreeWidgetItem(entry);
			item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsDragEnabled | Qt::ItemNeverHasChildren);
			item->setData(ITEM_DATA_ROLE::idx, Qt::UserRole, idx);
			item->setData(ITEM_DATA_ROLE::marker, Qt::UserRole, marker);
			item->setData(ITEM_DATA_ROLE::chid, Qt::UserRole, chid);
			item->setIcon(ITEM_ROW_ROLE::chnum, theme::spacer(4));
			if (marker)
			{
				item->setFont(ITEM_ROW_ROLE::chname, QFont(theme::fontFamily(), theme::calcFontSize(-1), QFont::Weight::Bold));
				item->setFont(ITEM_ROW_ROLE::chtype, QFont(theme::fontFamily(), theme::calcFontSize(-1), QFont::Weight::Bold));
			}
			item->setFont(ITEM_ROW_ROLE::chcas, QFont(theme::fontFamily(), theme::calcFontSize(-1)));
			if (! item->text(ITEM_ROW_ROLE::chcas).isEmpty())
			{
				item->setIcon(ITEM_ROW_ROLE::chcas, theme::icon("crypted"));
			}
			cache[curr].append(item);
		}
	}

	list->addTopLevelItems(cache[curr]);

	// sorting by
	if (this->state.sort.first != -1)
	{
		list->sortItems(this->state.sort.first, this->state.sort.second);

		// sorting column 0|desc
		if (this->state.sort.first == 0 && this->state.sort.second == Qt::DescendingOrder)
			list->header()->setSortIndicator(1, this->state.sort.second);
	}
	// sorting default
	else if (this->state.dnd)
	{
		list->setDragEnabled(true);
		list->setAcceptDrops(true);
	}
	list->header()->setSectionsClickable(true);
}

void mainView::treeSwitched(QTreeWidget* tree, QTreeWidgetItem* item)
{
	debug("treeSwitched()");

	int tc = -1;

	if (tree == this->services_tree)
		tc = 0;
	else if (tree == this->tree)
		tc = 1;

	if (tc != this->state.tc)
	{
		switch (tc)
		{
			case 0: return this->servicesItemChanged(item);
			case 1: return this->bouquetsItemChanged(item);
		}
	}
}

void mainView::servicesItemChanged(QTreeWidgetItem* current)
{
	debug("servicesItemChanged()");

	this->state.tc = 0;

	if (current != NULL)
	{
		int ti = services_tree->indexOfTopLevelItem(current);

		this->action.list_addch->setDisabled(true);
		this->action.list_addmk->setDisabled(true);
		this->action.list_newch->setEnabled(true);

		// tv | radio | data
		if (ti)
		{
			disallowDnD();
		}
		// all
		else
		{
			// sorting default
			if (this->state.sort.first == 0)
				allowDnD();
		}

		tabSetFlag(gui::TabListDelete, true);
		tabSetFlag(gui::TabListPaste, true);

		list->clearSelection();
		list->scrollToTop();
	}

	twid->updateChannelsIndex();
	populate(services_tree);
	updateFlags();
	updateStatus(true);
}

void mainView::bouquetsItemChanged(QTreeWidgetItem* current)
{
	debug("bouquetsItemChanged()");

	this->state.tc = 1;

	if (current != NULL)
	{
		int ti = tree->indexOfTopLevelItem(current);
		this->state.ti = ti;

		// bouquet: tv | radio
		if (ti != -1)
		{
			this->action.list_addch->setDisabled(true);
			this->action.list_addmk->setDisabled(true);
			this->action.list_newch->setEnabled(true);

			disallowDnD();

			// sorting by
			if (this->state.sort.first > 0)
				this->action.list_dnd->setDisabled(true);

			tabSetFlag(gui::TabListDelete, false);
			tabSetFlag(gui::TabListPaste, false);
		}
		// userbouquet
		else
		{
			this->action.list_addch->setEnabled(true);
			this->action.list_addmk->setEnabled(true);
			this->action.list_newch->setDisabled(true);

			// sorting by
			if (this->state.sort.first > 0)
				this->action.list_dnd->setEnabled(true);
			// sorting default
			else
				allowDnD();

			tabSetFlag(gui::TabListDelete, true);
			tabSetFlag(gui::TabListPaste, true);
		}

		list->clearSelection();
		list->scrollToTop();
	}

	twid->updateChannelsIndex();
	populate(tree);
	updateFlags();
	updateStatus(true);
}

void mainView::listItemChanged()
{
	// debug("listItemChanged()");

	if (list_evto->isChanged())
		listPendingUpdate();
}

void mainView::listItemSelectionChanged()
{
	// debug("listItemSelectionChanged()");
	
	QList<QTreeWidgetItem*> selected = list->selectedItems();

	if (selected.empty())
	{
		tabSetFlag(gui::TabListCut, false);
		tabSetFlag(gui::TabListCopy, false);

		// userbouquet
		if (this->state.ti == -1)
			tabSetFlag(gui::TabListDelete, false);
	}
	else
	{
		tabSetFlag(gui::TabListCut, true);
		tabSetFlag(gui::TabListCopy, true);

		// userbouquet
		if (this->state.ti == -1)
			tabSetFlag(gui::TabListDelete, true);
	}
	if (selected.count() == 1)
	{
		QTreeWidgetItem* item = selected.first();
		bool marker = item->data(ITEM_DATA_ROLE::marker, Qt::UserRole).toBool();

		if (marker)
		{
			tabSetFlag(gui::TabListEditService, false);
			tabSetFlag(gui::TabListEditMarker, true);
		}
		else
		{
			tabSetFlag(gui::TabListEditService, true);
			tabSetFlag(gui::TabListEditMarker, false);
		}
	}
	else if (selected.count() > 1)
	{
		tabSetFlag(gui::TabListEditService, false);
		tabSetFlag(gui::TabListEditMarker, false);

	}

	if (this->state.refbox)
		updateRefBox();
}

void mainView::listItemDoubleClicked()
{
	debug("listItemDoubleClicked()");

	QList<QTreeWidgetItem*> selected = list->selectedItems();
	
	if (selected.empty() || selected.count() > 1)
		return;

	QTreeWidgetItem* item = selected.first();
	bool marker = item->data(ITEM_DATA_ROLE::marker, Qt::UserRole).toBool();

	if (marker)
		editMarker();
	else
		editService();
}

void mainView::listPendingUpdate()
{
	debug("listPendingUpdate()");

	// sorting default
	if (this->state.dnd)
	{
		QTimer::singleShot(0, [=]() {
			this->visualReindexList();
		});
	}
	else
	{
		this->state.reindex = true;
	}
	this->state.changed = true;
}

void mainView::visualReindexList()
{
	debug("visualReindexList()");

	// sorting column 0|desc || column 0|asc
	bool reverse = (this->state.sort.first < 1 && this->state.sort.second == Qt::DescendingOrder) ? true : false;

	int i = 0, y = 0, idx = 0;
	int j = list->topLevelItemCount();

	if (reverse)
	{
		while (j--)
		{
			QTreeWidgetItem* item = list->topLevelItem(i);
			bool marker = item->data(ITEM_DATA_ROLE::marker, Qt::UserRole).toBool();
			if (marker)
				y++;
			i++;
		}
		i = 0;
		j = list->topLevelItemCount();
	}
	while (reverse ? j-- : i != j)
	{
		QTreeWidgetItem* item = list->topLevelItem(i);
		bool marker = item->data(ITEM_DATA_ROLE::marker, Qt::UserRole).toBool();
		idx = reverse ? j : i;
		char ci[7];
		std::sprintf(ci, "%06d", idx++);
		item->setText(ITEM_ROW_ROLE::x, QString::fromStdString(ci));
		if (! marker)
			item->setText(ITEM_ROW_ROLE::chnum, QString::fromStdString(to_string(idx - y)));
		i++;
		y = marker ? reverse ? y - 1 : y + 1 : y;
	}

	this->state.reindex = false;
}

void mainView::sortByColumn(int column)
{
	debug("sortByColumn()", "column", column);

	Qt::SortOrder order = this->state.sort.first == -1 ? Qt::DescendingOrder : list->header()->sortIndicatorOrder();
	column = column == 1 ? 0 : column;

	// sorting by
	if (column)
	{
		list->sortItems(column, order);
		disallowDnD();

		// userbouquet
		if (this->state.ti == -1)
			this->action.list_dnd->setEnabled(true);

		list->header()->setSortIndicatorShown(true);
	}
	// sorting default
	else
	{
		list->sortItems(column, order);
		list->header()->setSortIndicator(1, order);
		allowDnD();

		this->action.list_dnd->setDisabled(true);

		// default column 0|asc
		if (order == Qt::AscendingOrder)
			list->header()->setSortIndicatorShown(false);
		else
			list->header()->setSortIndicatorShown(true);

		if (this->state.reindex)
			this->visualReindexList();
	}
	this->state.sort = pair (column, order); //C++17
}

void mainView::allowDnD()
{
	debug("allowDnd()");

	this->list_evth->allowInternalMove();
	this->state.dnd = true;
	// list_wrap->setStyleSheet("#channels_wrap { background: transparent }");
}

void mainView::disallowDnD()
{
	debug("disallowDnD()");

	this->list_evth->disallowInternalMove();
	this->state.dnd = false;
	// list_wrap->setStyleSheet("#channels_wrap { background: rgba(255, 192, 0, 20%) }");
}

void mainView::reharmDnD()
{
	debug("reharmDnD()");

	// sorting default 0|asc
	list->sortItems(0, Qt::AscendingOrder);
	list->setDragEnabled(true);
	list->setAcceptDrops(true);
	this->state.sort = pair (0, Qt::AscendingOrder); //C++17
	this->action.list_dnd->setDisabled(true);
}

void mainView::addUserbouquet()
{
	debug("addUserbouquet()");

	string bname;
	e2se_gui::editBouquet* add = new e2se_gui::editBouquet(this->data, this->state.ti, this->log->log);
	add->display(cwid);
	bname = add->getAddId(); // returned after dial.exec()
	add->destroy();

	if (dbih->userbouquets.count(bname))
		debug("addUserbouquet()", "bname", bname);
	else
		return error("addUserbouquet()", "bname", bname);

	tree->setDragEnabled(false);
	tree->setAcceptDrops(false);

	int i = 0, y;
	QTreeWidgetItem* current = tree->currentItem();
	QTreeWidgetItem* parent = tree->invisibleRootItem();
	i = current != nullptr ? parent->indexOfChild(current) : tree->topLevelItemCount();
	y = i + 1;

	e2db::userbouquet uboq = dbih->userbouquets[bname];
	e2db::bouquet gboq = dbih->bouquets[uboq.pname];
	int pidx = gboq.btype == 1 ? 0 : 1;
	QTreeWidgetItem* pgroup = tree->topLevelItem(pidx);
	// macos: unwanted chars [qt.qpa.fonts] Menlo notice
	QString name;
	if (sets->value("preference/fixUnicodeChars").toBool())
		name = QString::fromStdString(uboq.name).remove(QRegularExpression("[^\\p{L}\\p{M}\\p{N}\\p{P}\\p{S}\\s]+"));
	else
		name = QString::fromStdString(uboq.name);

	QTreeWidgetItem* item = new QTreeWidgetItem(pgroup);
	item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled | Qt::ItemNeverHasChildren);
	QMap<QString, QVariant> tdata; // singular data
	tdata["id"] = QString::fromStdString(uboq.bname);
	item->setData(0, Qt::UserRole, QVariant (tdata));
	item->setText(0, name);

	if (current == nullptr)
		tree->addTopLevelItem(item);
	else
		tree->insertTopLevelItem(y, item);

	tree->setDragEnabled(true);
	tree->setAcceptDrops(true);

	twid->updateBouquetsIndex();
}

void mainView::editUserbouquet()
{
	debug("editUserbouquet()");

	QList<QTreeWidgetItem*> selected = tree->selectedItems();
	
	if (selected.empty() || selected.count() > 1)
		return;

	QTreeWidgetItem* item = selected.first();
	QVariantMap tdata = item->data(0, Qt::UserRole).toMap();
	QString qbname = tdata["id"].toString();
	string bname = qbname.toStdString();

	e2se_gui::editBouquet* edit = new e2se_gui::editBouquet(this->data, this->state.ti, this->log->log);
	edit->setEditId(bname);
	edit->display(cwid);
	edit->destroy();

	if (dbih->userbouquets.count(bname))
		debug("editUserbouquet()", "bname", bname);
	else
		return error("editUserbouquet()", "bname", bname);

	e2db::userbouquet uboq = dbih->userbouquets[bname];
	// macos: unwanted chars [qt.qpa.fonts] Menlo notice
	QString name;
	if (sets->value("preference/fixUnicodeChars").toBool())
		name = QString::fromStdString(uboq.name).remove(QRegularExpression("[^\\p{L}\\p{M}\\p{N}\\p{P}\\p{S}\\s]+"));
	else
		name = QString::fromStdString(uboq.name);
	item->setText(0, name);

	twid->updateBouquetsIndex();
}

void mainView::addChannel()
{
	debug("addChannel()");

	e2se_gui::dialChannelBook* book = new e2se_gui::dialChannelBook(this->data, this->log->log);
	book->setEventCallback([=](vector<QString> items) { this->putListItems(items); });
	book->display(cwid);
}

void mainView::addService()
{
	debug("addService()");

	string chid;
	string curr = this->state.curr;
	e2se_gui::editService* add = new e2se_gui::editService(this->data, this->log->log);
	add->display(cwid);
	chid = add->getAddId(); // returned after dial.exec()
	add->destroy();

	if (dbih->db.services.count(chid))
		debug("addService()", "chid", chid);
	else
		return error("addService()", "chid", chid);

	cache.clear();
	list->header()->setSectionsClickable(false);
	list->setDragEnabled(false);
	list->setAcceptDrops(false);

	int i = 0, y;
	QTreeWidgetItem* current = list->currentItem();
	QTreeWidgetItem* parent = list->invisibleRootItem();
	i = current != nullptr ? parent->indexOfChild(current) : list->topLevelItemCount();
	y = i + 1;

	bool marker = false;
	char ci[7];
	std::sprintf(ci, "%06d", i++);
	QString x = QString::fromStdString(ci);
	QString idx = QString::fromStdString(to_string(i));
	QStringList entry = dbih->entries.services[chid];
	entry.prepend(idx);
	entry.prepend(x);

	QTreeWidgetItem* item = new QTreeWidgetItem(entry);
	item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsDragEnabled | Qt::ItemNeverHasChildren);
	item->setData(ITEM_DATA_ROLE::idx, Qt::UserRole, idx);
	item->setData(ITEM_DATA_ROLE::marker, Qt::UserRole, marker);
	item->setData(ITEM_DATA_ROLE::chid, Qt::UserRole, QString::fromStdString(chid));
	item->setIcon(ITEM_ROW_ROLE::chnum, theme::spacer(4));
	if (marker)
	{
		item->setFont(ITEM_ROW_ROLE::chname, QFont(theme::fontFamily(), theme::calcFontSize(-1), QFont::Weight::Bold));
		item->setFont(ITEM_ROW_ROLE::chtype, QFont(theme::fontFamily(), theme::calcFontSize(-1), QFont::Weight::Bold));
	}
	item->setFont(ITEM_ROW_ROLE::chcas, QFont(theme::fontFamily(), theme::calcFontSize(-1)));
	if (! item->text(ITEM_ROW_ROLE::chcas).isEmpty())
	{
		item->setIcon(ITEM_ROW_ROLE::chcas, theme::icon("crypted"));
	}

	if (current == nullptr)
		list->addTopLevelItem(item);
	else
		list->insertTopLevelItem(y, item);

	list->header()->setSectionsClickable(true);
	list->setDragEnabled(true);
	list->setAcceptDrops(true);

	// sorting default
	if (this->state.dnd)
		visualReindexList();
	else
		this->state.reindex = true;
	this->state.changed = true;

	updateFlags();
	updateStatus();
}

void mainView::editService()
{
	debug("editService()");

	QList<QTreeWidgetItem*> selected = list->selectedItems();
	
	if (selected.empty() || selected.count() > 1)
		return;

	QTreeWidgetItem* item = selected.first();
	string chid = item->data(ITEM_DATA_ROLE::chid, Qt::UserRole).toString().toStdString();
	string nw_chid;
	bool marker = item->data(ITEM_DATA_ROLE::marker, Qt::UserRole).toBool();

	debug("editService()", "chid", chid);

	if (! marker && dbih->db.services.count(chid))
		debug("editService()", "chid", chid);
	else
		return error("editService()", "chid", chid);

	e2se_gui::editService* edit = new e2se_gui::editService(this->data, this->log->log);
	edit->setEditId(chid);
	edit->display(cwid);
	nw_chid = edit->getEditId(); // returned after dial.exec()
	edit->destroy();

	cache.clear();

	if (dbih->db.services.count(nw_chid))
		debug("editService()", "new chid", nw_chid);
	else
		return error("editService()", "new chid", nw_chid);

	QStringList entry = dbih->entries.services[nw_chid];
	entry.prepend(item->text(ITEM_ROW_ROLE::chnum));
	entry.prepend(item->text(ITEM_ROW_ROLE::x));
	for (int i = 0; i < entry.count(); i++)
		item->setText(i, entry[i]);
	item->setData(ITEM_DATA_ROLE::chid, Qt::UserRole, QString::fromStdString(nw_chid));
}

void mainView::addMarker()
{
	debug("addMarker()");

	string chid;
	string curr = this->state.curr;
	e2se_gui::editMarker* add = new e2se_gui::editMarker(this->data, this->log->log);
	add->setAddId(curr);
	add->display(cwid);
	chid = add->getAddId(); // returned after dial.exec()
	add->destroy();

	e2db::channel_reference chref;
	if (dbih->userbouquets.count(curr))
		chref = dbih->userbouquets[curr].channels[chid];

	if (chref.marker)
		debug("addMarker()", "chid", chid);
	else
		return error("addMarker()", "chid", chid);

	cache.clear();
	list->header()->setSectionsClickable(false);
	list->setDragEnabled(false);
	list->setAcceptDrops(false);

	int i = 0, y;
	QTreeWidgetItem* current = list->currentItem();
	QTreeWidgetItem* parent = list->invisibleRootItem();
	i = current != nullptr ? parent->indexOfChild(current) : list->topLevelItemCount();
	y = i + 1;

	bool marker = true;
	char ci[7];
	std::sprintf(ci, "%06d", i++);
	QString x = QString::fromStdString(ci);
	QString idx = "";
	QStringList entry = dbih->entryMarker(chref);
	entry.prepend(x);

	QTreeWidgetItem* item = new QTreeWidgetItem(entry);
	item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsDragEnabled | Qt::ItemNeverHasChildren);
	item->setData(ITEM_DATA_ROLE::idx, Qt::UserRole, idx);
	item->setData(ITEM_DATA_ROLE::marker, Qt::UserRole, marker);
	item->setData(ITEM_DATA_ROLE::chid, Qt::UserRole, QString::fromStdString(chid));
	item->setIcon(ITEM_ROW_ROLE::chnum, theme::spacer(4));
	if (marker)
	{
		item->setFont(ITEM_ROW_ROLE::chname, QFont(theme::fontFamily(), theme::calcFontSize(-1), QFont::Weight::Bold));
		item->setFont(ITEM_ROW_ROLE::chtype, QFont(theme::fontFamily(), theme::calcFontSize(-1), QFont::Weight::Bold));
	}
	item->setFont(ITEM_ROW_ROLE::chcas, QFont(theme::fontFamily(), theme::calcFontSize(-1)));
	if (! item->text(ITEM_ROW_ROLE::chcas).isEmpty())
	{
		item->setIcon(ITEM_ROW_ROLE::chcas, theme::icon("crypted"));
	}
	
	if (current == nullptr)
		list->addTopLevelItem(item);
	else
		list->insertTopLevelItem(y, item);

	list->header()->setSectionsClickable(true);
	list->setDragEnabled(true);
	list->setAcceptDrops(true);

	// sorting default
	if (this->state.dnd)
		visualReindexList();
	else
		this->state.reindex = true;
	this->state.changed = true;

	updateFlags();
	updateStatus();
}

void mainView::editMarker()
{
	debug("editMarker()");

	QList<QTreeWidgetItem*> selected = list->selectedItems();
	
	if (selected.empty() || selected.count() > 1)
		return;

	QTreeWidgetItem* item = selected.first();
	string chid = item->data(ITEM_DATA_ROLE::chid, Qt::UserRole).toString().toStdString();
	string nw_chid;
	string curr = this->state.curr;
	bool marker = item->data(ITEM_DATA_ROLE::marker, Qt::UserRole).toBool();

	if (marker)
		debug("editMarker()", "chid", chid);
	else
		return error("editMarker()", "chid", chid);

	e2se_gui::editMarker* edit = new e2se_gui::editMarker(this->data, this->log->log);
	edit->setEditId(chid, curr);
	edit->display(cwid);
	nw_chid = edit->getEditId(); // returned after dial.exec()
	edit->destroy();

	e2db::channel_reference chref;
	if (dbih->userbouquets.count(curr))
		chref = dbih->userbouquets[curr].channels[chid];

	if (chref.marker)
		debug("editMarker()", "new chid", nw_chid);
	else
		return error("editMarker()", "new chid", nw_chid);

	cache.clear();

	QStringList entry = dbih->entryMarker(chref);
	entry.prepend(item->text(ITEM_ROW_ROLE::x));
	for (int i = 0; i < entry.count(); i++)
		item->setText(i, entry[i]);
	item->setData(ITEM_DATA_ROLE::chid, Qt::UserRole, QString::fromStdString(nw_chid));
}

void mainView::bouquetItemDelete()
{
	debug("bouquetItemDelete()");

	QList<QTreeWidgetItem*> selected = tree->selectedItems();
	
	if (selected.empty())
	{
		return;
	}
	for (auto & item : selected)
	{
		QVariantMap tdata = item->data(0, Qt::UserRole).toMap();
		QString qbname = tdata["id"].toString();
		string bname = qbname.toStdString();
		e2db::userbouquet uboq = dbih->userbouquets[bname];
		string pname = uboq.pname;
		dbih->removeUserbouquet(bname);

		cache[bname].clear();
		cache[pname].clear();
		cache.erase(bname);

		QTreeWidgetItem* parent = item->parent();
		parent->removeChild(item);
	}

	this->state.changed = true;

	twid->updateBouquetsIndex();
	updateStatus();
}

void mainView::listReferenceToggle()
{
	debug("listReferenceToggle()");

	if (list_reference->isHidden())
	{
		list_reference->show();
		this->state.refbox = true;
		updateRefBox();
	}
	else
	{
		list_reference->hide();
		this->state.refbox = false;
	}
}

void mainView::listItemCut()
{
	debug("listItemCut()");

	listItemCopy(true);
}

void mainView::listItemCopy(bool cut)
{
	debug("listItemCopy()");

	QList<QTreeWidgetItem*> selected = list->selectedItems();
	
	if (selected.empty())
		return;

	QClipboard* clipboard = QGuiApplication::clipboard();
	QStringList text;
	for (auto & item : selected)
	{
		QString qchid = item->data(ITEM_DATA_ROLE::chid, Qt::UserRole).toString();
		bool marker = item->data(ITEM_DATA_ROLE::marker, Qt::UserRole).toBool();
		string chid = qchid.toStdString();

		QStringList data;
		// start from chnum column [1]
		for (int i = ITEM_ROW_ROLE::chnum; i < list->columnCount(); i++)
		{
			QString qstr = item->data(i, Qt::DisplayRole).toString();
			// chname
			if (i == ITEM_ROW_ROLE::chname)
				qstr.prepend("\"").append("\"");
			// debug_chid
			else if (i == ITEM_ROW_ROLE::debug_chid)
				continue;
			// debug_txid
			else if (i == ITEM_ROW_ROLE::debug_txid)
				continue;
			// chcas
			else if (i == ITEM_ROW_ROLE::chcas && ! marker)
				qstr.prepend(qstr.isEmpty() ? "" : "$").prepend("\"").append("\"");
			// chpname
			else if (i == ITEM_ROW_ROLE::chpname && ! marker)
				qstr.prepend("\"").append("\"");
			// chpos
			else if (i == ITEM_ROW_ROLE::chpos && ! marker)
				qstr.prepend("\"").append("\"");
			data.append(qstr);
		}

		// Reference ID
		QString refid;
		// bouquets tree
		if (this->state.tc)
		{
			string curr = this->state.curr;
			e2db::channel_reference chref;
			if (dbih->userbouquets.count(curr))
				chref = dbih->userbouquets[curr].channels[chid];
			string crefid = dbih->get_reference_id(chref);
			refid = QString::fromStdString(crefid);
		}
		// services tree
		else
		{
			string crefid = dbih->get_reference_id(chid);
			refid = QString::fromStdString(crefid);
		}
		//TODO marker index and compatibility
		if (marker)
		{
			refid = "1:" + qchid.toUpper() + ":0:0:0:0:0:0";
		}
		data.insert(2, refid); // insert refid column [2]
		text.append(data.join(",")); // CSV
	}
	clipboard->setText(text.join("\n")); // CSV

	//TODO global marker index
	if (cut)
		listItemDelete();
}

//TODO validate
void mainView::listItemPaste()
{
	debug("listItemPaste()", "entered", ! (this->state.tc && this->state.ti != -1));

	// services_tree && bouquet: tv | radio
	if (this->state.tc && this->state.ti != -1)
		return;

	QClipboard* clipboard = QGuiApplication::clipboard();
	const QMimeData* mimeData = clipboard->mimeData();
	vector<QString> items;

	if (mimeData->hasText())
	{
		QStringList list = clipboard->text().split('\n');

		for (QString & data : list)
		{
			if (data.contains(','))
			{
				auto line = data.split(',');
				// refid column [2] | chname column [1]
				items.emplace_back(line[2] + ',' + line[1]);
			}
			else
			{
				items.emplace_back(data);
			}
		}
	}
	if (! items.empty())
	{
		putListItems(items);

		if (list->currentItem() == nullptr)
			list->scrollToBottom();

		// bouquets tree
		if (this->state.tc)
		{
			string curr = this->state.curr;
			e2db::userbouquet uboq = dbih->userbouquets[curr];
			string pname = uboq.pname;
			cache[pname].clear();
		}
		// services tree
		else
		{
			cache["chs"].clear();
			cache["chs:0"].clear();
			cache["chs:1"].clear();
			cache["chs:2"].clear();
		}
	}
}

void mainView::listItemDelete()
{
	debug("listItemDelete()", "entered", ! (this->state.tc && this->state.ti != -1));

	// services_tree && bouquet: tv | radio
	if (this->state.tc && this->state.ti != -1)
		return;

	QList<QTreeWidgetItem*> selected = list->selectedItems();
	
	if (selected.empty())
		return;

	list->header()->setSectionsClickable(false);
	list->setDragEnabled(false);
	list->setAcceptDrops(false);

	string curr = this->state.curr;
	string pname;

	// bouquets tree
	if (this->state.tc)
	{
		e2db::userbouquet uboq = dbih->userbouquets[curr];
		pname = uboq.pname;
	}
	for (auto & item : selected)
	{
		int i = list->indexOfTopLevelItem(item);
		string chid = item->data(ITEM_DATA_ROLE::chid, Qt::UserRole).toString().toStdString();
		list->takeTopLevelItem(i);

		// bouquets tree
		if (this->state.tc)
		{
			dbih->removeChannelReference(chid, curr);
			cache[pname].clear();
		}
		// services tree
		else
		{
			dbih->removeService(chid);
			cache["chs"].clear();
			cache["chs:0"].clear();
			cache["chs:1"].clear();
			cache["chs:2"].clear();
		}
	}

	list->header()->setSectionsClickable(true);
	list->setDragEnabled(true);
	list->setAcceptDrops(true);

	// sorting default
	if (this->state.dnd)
		visualReindexList();
	else
		this->state.reindex = true;
	this->state.changed = true;

	updateFlags();
	updateStatus();
}

void mainView::listItemSelectAll()
{
	debug("listItemSelectAll()");

	list->selectAll();
}

//TODO duplicates
void mainView::putListItems(vector<QString> items)
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

	string curr = this->state.curr;
	e2db::userbouquet uboq = dbih->userbouquets[curr];
	int ub_idx = uboq.index;
	int anum_count = dbih->index["mks"].size();

	for (QString & q : items)
	{
		char ci[7];
		std::sprintf(ci, "%06d", i++);
		QString x = QString::fromStdString(ci);
		QString idx = QString::fromStdString(to_string(i));

		string refid;
		string value;
		if (q.contains(','))
		{
			// refid column [0] | chname column [1]
			auto data = q.split(',');
			refid = data[0].toStdString();
			value = data[1].replace("\"", "").toStdString();
		}
		else
		{
			refid = q.toStdString();
		}

		QStringList entry;
		bool marker = false;
		e2db::channel_reference chref;
		e2db::service_reference ref;
		e2db::service ch;

		dbih->parse_channel_reference(refid, chref, ref);

		char chid[25];

		if (chref.marker)
		{
			anum_count++;

			// %4d:%2x:%d
			std::sprintf(chid, "%d:%x:%d", chref.type, anum_count, ub_idx);
		}
		else
		{
			// %4x:%4x:%8x
			std::sprintf(chid, "%x:%x:%x", ref.ssid, ref.tsid, ref.dvbns);
		}

		if (dbih->db.services.count(chid))
		{
			ch = dbih->db.services[chid];
			entry = dbih->entries.services[chid];
			entry.prepend(idx);
			entry.prepend(x);

			chref.marker = false;
			chref.chid = chid;
			chref.type = 0;
			chref.anum = 0;
			chref.index = idx.toInt();
		}
		else
		{
			if (chref.marker)
			{
				chref.chid = chid;
				chref.anum = anum_count;
				chref.value = value;
				chref.index = -1;
				idx = "";

				marker = true;
				entry = dbih->entryMarker(chref);
				entry.prepend(x);
			}
			//TODO add new service/transponder
			else
			{
				error("putListItems()", "refid", refid);
				continue;
			}
		}
		QTreeWidgetItem* item = new QTreeWidgetItem(entry);
		item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsDragEnabled | Qt::ItemNeverHasChildren);
		item->setData(ITEM_DATA_ROLE::idx, Qt::UserRole, idx);
		item->setData(ITEM_DATA_ROLE::marker, Qt::UserRole, marker);
		item->setData(ITEM_DATA_ROLE::chid, Qt::UserRole, QString::fromStdString(chid));
		item->setIcon(ITEM_ROW_ROLE::chnum, theme::spacer(4));
		if (marker)
		{
			item->setFont(ITEM_ROW_ROLE::chname, QFont(theme::fontFamily(), theme::calcFontSize(-1), QFont::Weight::Bold));
			item->setFont(ITEM_ROW_ROLE::chtype, QFont(theme::fontFamily(), theme::calcFontSize(-1), QFont::Weight::Bold));
		}
		item->setFont(ITEM_ROW_ROLE::chcas, QFont(theme::fontFamily(), theme::calcFontSize(-1)));
		if (! item->text(ITEM_ROW_ROLE::chcas).isEmpty())
		{
			item->setIcon(ITEM_ROW_ROLE::chcas, theme::icon("crypted"));
		}
		clist.append(item);

		// bouquets tree
		if (this->state.tc)
			dbih->add_channel_reference(chref, curr);
		// services tree
		else
			dbih->addService(ch);
	}

	if (current == nullptr)
		list->addTopLevelItems(clist);
	else
		list->insertTopLevelItems(y, clist);

	list->header()->setSectionsClickable(true);
	list->setDragEnabled(true);
	list->setAcceptDrops(true);

	// sorting default
	if (this->state.dnd)
		visualReindexList();
	else
		this->state.reindex = true;
	this->state.changed = true;

	updateFlags();
	updateStatus();
}

void mainView::updateStatus(bool current)
{
	debug("updateStatus()");

	gui::STATUS status;
	status.current = current;

	if (current && ! this->state.curr.empty())
	{
		string curr = this->state.curr;
		status.counters[gui::COUNTER::bouquet] = dbih->index[curr].size();

		// bouquets tree
		if (this->state.tc)
			status.bname = curr;
	}
	else
	{
		status.counters[gui::COUNTER::data] = dbih->index["chs:0"].size();
		status.counters[gui::COUNTER::tv] = dbih->index["chs:1"].size();
		status.counters[gui::COUNTER::radio] = dbih->index["chs:2"].size();
		status.counters[gui::COUNTER::services] = dbih->index["chs"].size();
	}

	tabSetStatus(status);
}

void mainView::updateRefBox()
{
	debug("updateRefBox()");

	QList<QTreeWidgetItem*> selected = list->selectedItems();
	
	if (selected.empty() || selected.count() > 1)
	{
		for (auto & field : ref_fields)
			field.second->setText(selected.empty() ? "< >" : "< ... >");
	}
	else
	{
		QTreeWidgetItem* item = selected[0];
		string chid = item->data(ITEM_DATA_ROLE::chid, Qt::UserRole).toString().toStdString();
		QString ssid, refid, txp, tns, bsls, ubls;

		// debug("updateRefBox()", "chid", chid);

		// bouquets tree
		if (this->state.tc)
		{
			string curr = this->state.curr;
			e2db::channel_reference chref = dbih->userbouquets[curr].channels[chid];
			string crefid = dbih->get_reference_id(chref);
			refid = QString::fromStdString(crefid);
		}
		// services tree
		else
		{
			string crefid = dbih->get_reference_id(chid);
			refid = QString::fromStdString(crefid);
		}

		bsls = ubls = "< >";

		unordered_map<string, int> bss;
		QStringList ubl;
		for (auto & x : dbih->userbouquets)
		{
			if (x.second.channels.count(chid))
			{
				if (! x.second.name.empty())
					ubl.append(QString::fromStdString(x.second.name));
				bss[x.second.pname] = bss[x.second.pname]++;
			}
		}
		if (! ubl.empty())
		{
			ubls = "<p style=\"line-height: 150%\">" + ubl.join("<br>") + "</p>";
		}

		QStringList bsl;
		for (auto & x : bss)
		{
			if (dbih->bouquets.count(x.first))
				bsl.append(QString::fromStdString(dbih->bouquets[x.first].nname));
		}
		if (! bsl.empty())
		{
			bsls = "<p style=\"line-height: 150%\">" + bsl.join("<br>") + "</p>";
		}

		if (dbih->db.services.count(chid))
		{
			e2db::service ch = dbih->db.services[chid];
			e2db::transponder tx = dbih->db.transponders[ch.txid];

			ssid = QString::fromStdString(to_string(ch.ssid));

			string ptxp = dbih->get_transponder_combo_value(tx);
			txp = QString::fromStdString(ptxp);

			string psys = dbih->get_transponder_system_text(tx);
			string ppos = dbih->get_transponder_position_text(tx);

			tns = "<p style=\"line-height: 125%\">" + QString::fromStdString(psys + "<br>" + ppos) + "</p>";
		}
		else
		{
			ssid = txp = tns = "< >";
		}

		ref_fields[LIST_REF::ReferenceID]->setText(refid);
		ref_fields[LIST_REF::ServiceID]->setText(ssid);
		ref_fields[LIST_REF::Transponder]->setText(txp);
		ref_fields[LIST_REF::Userbouquets]->setText(ubls);
		ref_fields[LIST_REF::Bouquets]->setText(bsls);
		ref_fields[LIST_REF::Tuner]->setText(tns);
	}
}

void mainView::showTreeEditContextMenu(QPoint &pos)
{
	debug("showTreeEditContextMenu()");

	QMenu* tree_edit = new QMenu;
	QAction* bouquet_export = new QAction("Export");
	// bouquet: tv | radio
	if (this->state.ti != -1)
	{
		bouquet_export->connect(bouquet_export, &QAction::triggered, [=]() { twid->exportFile(); });
	}
	// userbouquet
	else
	{
		tree_edit->addAction("Edit Userbouquet", [=]() { this->editUserbouquet(); });
		tree_edit->addSeparator();
		tree_edit->addAction("Delete", [=]() { this->bouquetItemDelete(); });
		bouquet_export->connect(bouquet_export, &QAction::triggered, [=]() { twid->exportFile(); });
	}
	tree_edit->addSeparator();
	tree_edit->addAction(bouquet_export);

	tree_edit->exec(tree->mapToGlobal(pos));
}

void mainView::showListEditContextMenu(QPoint &pos)
{
	debug("showListEditContextMenu()");

	QMenu* list_edit = new QMenu;
	list_edit->addAction("Edit Service", [=]() { this->editService(); })->setEnabled(tabGetFlag(gui::TabListEditService));
	list_edit->addAction("Edit Marker", [=]() { this->editMarker(); })->setEnabled(tabGetFlag(gui::TabListEditMarker));
	list_edit->addSeparator();
	list_edit->addAction("Cu&t", [=]() { this->listItemCut(); }, QKeySequence::Cut)->setEnabled(tabGetFlag(gui::TabListCut));
	list_edit->addAction("&Copy", [=]() { this->listItemCopy(); }, QKeySequence::Copy)->setEnabled(tabGetFlag(gui::TabListCopy));
	list_edit->addAction("&Paste", [=]() { this->listItemPaste(); }, QKeySequence::Paste)->setEnabled(tabGetFlag(gui::TabListPaste));
	list_edit->addSeparator();
	list_edit->addAction("&Delete", [=]() { this->listItemDelete(); }, QKeySequence::Delete)->setEnabled(tabGetFlag(gui::TabListDelete));

	list_edit->exec(list->mapToGlobal(pos));
}

void mainView::updateFlags()
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
		tabSetFlag(gui::OpenChannelBook, true);
		tabSetFlag(gui::TabListSelectAll, true);
		tabSetFlag(gui::TabListFind, true);
		this->action.list_search->setEnabled(true);
	}
	else
	{
		tabSetFlag(gui::OpenChannelBook, false);
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
