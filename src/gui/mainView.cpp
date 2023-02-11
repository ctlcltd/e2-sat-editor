/*!
 * e2-sat-editor/src/gui/mainView.cpp
 *
 * @link https://github.com/ctlcltd/e2-sat-editor
 * @copyright e2 SAT Editor Team
 * @author Leonardo Laureti
 * @version 0.3
 * @license MIT License
 * @license GNU GPLv3 License
 */

#include <QtGlobal>
#include <QSettings>
#include <QTimer>
#include <QList>
#include <QGridLayout>
#include <QSplitter>
#include <QGroupBox>
#include <QHeaderView>
#include <QStyle>
#include <QMessageBox>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QSplitter>
#include <QGroupBox>
#include <QMenu>
#include <QScrollArea>
#include <QClipboard>
#include <QMimeData>

#include "platforms/platform.h"

#include "toolkit/TreeStyledItemDelegate.h"
#include "mainView.h"
#include "theme.h"
#include "tab.h"
#include "editBouquet.h"
#include "editService.h"
#include "editMarker.h"
#include "dialChannelBook.h"

using namespace e2se;

namespace e2se_gui
{

mainView::mainView(tab* tid, QWidget* cwid, dataHandler* data)
{
	this->log = new logger("gui", "mainView");

	this->tid = tid;
	this->cwid = cwid;
	this->data = data;
	this->theme = new e2se_gui::theme;
	this->widget = new QWidget;

	layout();
}

void mainView::layout()
{
	debug("layout");

	widget->setStyleSheet("QGroupBox { spacing: 0; border: 0; padding: 0; padding-top: 32px; font-weight: bold } QGroupBox::title { margin: 8px 4px; padding: 0 1px 1px }");

	QGridLayout* frm = new QGridLayout(widget);

	//TODO twrap and vertical scrollbar hSize in GTK+
	QSplitter* swid = new QSplitter;
#ifdef Q_OS_MAC
	swid->setStyleSheet("QSplitter::handle {}");
#endif

	QWidget* afrm = new QWidget;
	QWidget* bfrm = new QWidget;

	QVBoxLayout* asbox = new QVBoxLayout;
	QVBoxLayout* atbox = new QVBoxLayout;
	QGridLayout* bbox = new QGridLayout;
	QVBoxLayout* tbox = new QVBoxLayout;
	QVBoxLayout* lbox = new QVBoxLayout;

	QGroupBox* sfrm = new QGroupBox("Services");
	QGroupBox* tfrm = new QGroupBox("Bouquets");
	QGroupBox* lfrm = new QGroupBox("Channels");

	frm->setContentsMargins(0, 0, 0, 0);
	asbox->setContentsMargins(0, 0, 0, 0);
	atbox->setContentsMargins(0, 0, 0, 0);
	bbox->setContentsMargins(0, 0, 0, 0);
	tbox->setContentsMargins(0, 0, 0, 0);
	lbox->setContentsMargins(0, 0, 0, 0);

	frm->setSpacing(0);
	asbox->setSpacing(0);
	atbox->setSpacing(0);
	bbox->setSpacing(0);
	tbox->setSpacing(4);
	lbox->setSpacing(0);

	sfrm->setFlat(true);
	tfrm->setFlat(true);
	lfrm->setFlat(true);

	this->side = new QTreeWidget;
	this->tree = new QTreeWidget;
	this->list = new QTreeWidget;

	side->setStyleSheet("QTreeWidget { background: transparent } QTreeWidget::item { padding: 9px 0 }");
	tree->setStyleSheet("QTreeWidget { background: transparent } QTreeWidget::item { margin: 1px 0 0; padding: 8px 0 }");
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

	side->setHeaderHidden(true);
	side->setUniformRowHeights(true);
	tree->setHeaderHidden(true);
	tree->setUniformRowHeights(true);
	list->setUniformRowHeights(true);

	side->setRootIsDecorated(false);
	side->setItemsExpandable(false);
	side->setExpandsOnDoubleClick(false);

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

	QTreeWidgetItem* lheader_item = new QTreeWidgetItem({NULL, "Index", "Name", "CHID", "TXID", "Service ID", "Transport ID", "Type", "CAS", "Provider", "System", "Position", "Tuner", "Frequency", "Polarization", "Symbol Rate", "FEC"});

	list->setHeaderItem(lheader_item);
	list->setColumnHidden(ITEM_ROW_ROLE::x, true);		// hidden index
	list->setColumnWidth(ITEM_ROW_ROLE::chnum, 65);		// (Channel Number) Index
	list->setColumnWidth(ITEM_ROW_ROLE::chname, 200);	// (Channel) Name
	if (QSettings().value("application/debug", true).toBool()) {
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
	list->setColumnWidth(ITEM_ROW_ROLE::chcas, 45);		// CAS
	list->setColumnWidth(ITEM_ROW_ROLE::chpname, 150);	// Provider
	list->setColumnWidth(ITEM_ROW_ROLE::chsys, 75);		// System
	list->setColumnWidth(ITEM_ROW_ROLE::chpos, 80);		// Position
	list->setColumnWidth(ITEM_ROW_ROLE::chtname, 125);	// Tuner Name
	list->setColumnWidth(ITEM_ROW_ROLE::chfreq, 95);	// Frequency
	list->setColumnWidth(ITEM_ROW_ROLE::chpol, 85);		// Polarization
	list->setColumnWidth(ITEM_ROW_ROLE::chsr, 95);		// Symbol Rate
	list->setColumnWidth(ITEM_ROW_ROLE::chfec, 50);		// FEC

	list->header()->connect(list->header(), &QHeaderView::sectionClicked, [=](int column) { this->sortByColumn(column); });

	tree->setContextMenuPolicy(Qt::CustomContextMenu);
	tree->connect(tree, &QTreeWidget::customContextMenuRequested, [=](QPoint pos) { this->showTreeEditContextMenu(pos); });
	list->setContextMenuPolicy(Qt::CustomContextMenu);
	list->connect(list, &QTreeWidget::customContextMenuRequested, [=](QPoint pos) { this->showListEditContextMenu(pos); });

	searchLayout();
	referenceBoxLayout();

	QToolBar* tree_ats = toolBar();
	QToolBar* list_ats = toolBar();

	this->action.tree_newbq = toolBarAction(tree_ats, "New Bouquet", theme->dynamicIcon("add"), [=]() { this->addUserbouquet(); });
	toolBarSpacer(tree_ats);
	this->action.tree_search = toolBarButton(tree_ats, "Find…", theme->dynamicIcon("search"), [=]() { this->treeSearchToggle(); });

	this->action.tree_search->setDisabled(true);

	this->action.list_addch = toolBarAction(list_ats, "Add Channel", theme->dynamicIcon("add"), [=]() { this->addChannel(); });
	this->action.list_addmk = toolBarAction(list_ats, "Add Marker", theme->dynamicIcon("add"), [=]() { this->addMarker(); });
	this->action.list_newch = toolBarAction(list_ats, "New Service", theme->dynamicIcon("add"), [=]() { this->addService(); });
	toolBarSeparator(list_ats);
	this->action.list_dnd = toolBarButton(list_ats, "Drag&&Drop", [=]() { this->reharmDnD(); });
	this->action.list_ref = toolBarButton(list_ats, "Reference", [=]() { this->listReferenceToggle(); });
	toolBarSpacer(list_ats);
	this->action.list_search = toolBarButton(list_ats, "&Find…", theme->dynamicIcon("search"), [=]() { this->listSearchToggle(); });

	this->action.list_dnd->setCheckable(true);
	this->action.list_ref->setCheckable(true);

	this->action.list_addmk->setDisabled(true);
	this->action.list_newch->setDisabled(true);
	this->action.list_dnd->setDisabled(true);
	this->action.list_search->setDisabled(true);

	this->tree_evth = new TreeEventHandler;
	this->list_evth = new ListEventHandler;
	this->list_evto = new ListEventObserver;
	tree_evth->setEventCallback([=](QTreeWidget* tw, QTreeWidgetItem* current) { this->treeAfterDrop(tw, current); });
	list_evth->setEventCallback([=](QTreeWidget* tw) { this->listAfterDrop(tw); });
	side->connect(side, &QTreeWidget::itemPressed, [=](QTreeWidgetItem* item) { this->treeSwitched(side, item); });
	side->connect(side, &QTreeWidget::currentItemChanged, [=](QTreeWidgetItem* current) { this->servicesItemChanged(current); });
	tree->viewport()->installEventFilter(tree_evth);
	tree->connect(tree, &QTreeWidget::itemPressed, [=](QTreeWidgetItem* item) { this->treeSwitched(tree, item); });
	tree->connect(tree, &QTreeWidget::currentItemChanged, [=](QTreeWidgetItem* current) { this->treeItemChanged(current); });
	tree->connect(tree, &QTreeWidget::itemDoubleClicked, [=]() { this->treeItemDoubleClicked(); });
	list->installEventFilter(list_evto);
	list->viewport()->installEventFilter(list_evth);
	list->connect(list, &QTreeWidget::currentItemChanged, [=]() { this->listItemChanged(); });
	list->connect(list, &QTreeWidget::itemSelectionChanged, [=]() { this->listItemSelectionChanged(); });
	list->connect(list, &QTreeWidget::itemDoubleClicked, [=]() { this->listItemDoubleClicked(); });

	asbox->addWidget(side);
	sfrm->setLayout(asbox);

	atbox->addWidget(tree);
	atbox->addWidget(tree_search);
	atbox->addWidget(tree_ats);
	tfrm->setLayout(atbox);

	tbox->addWidget(sfrm);
	tbox->addItem(new QSpacerItem(0, 8, QSizePolicy::Preferred, QSizePolicy::Fixed));
	tbox->addWidget(tfrm, 1);
	afrm->setLayout(tbox);

	lbox->addWidget(list);
	lbox->addWidget(list_search);
	lbox->addWidget(list_reference);
	lbox->addWidget(list_ats);
	lfrm->setLayout(lbox);

	afrm->setMinimumWidth(250);
	lfrm->setMinimumWidth(510);

	bbox->addWidget(lfrm);
	bfrm->setLayout(bbox);

	platform::osWidgetBlend(afrm);
	platform::osWidgetOpaque(bfrm);

	swid->addWidget(afrm);
	swid->addWidget(bfrm);

	swid->setStretchFactor(0, 1);
	swid->setStretchFactor(1, 4);

	// platform::osWidgetOpaque(swid->handle(1));

	frm->addWidget(swid);

	toolBarStyleSheet();

	vector<pair<QString, QString>> tree = {
		{"chs", "All services"},
		{"chs:1", "TV"},
		{"chs:2", "Radio"},
		{"chs:0", "Data"}
	};

	for (auto & x : tree)
	{
		QTreeWidgetItem* item = new QTreeWidgetItem();
		item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
		item->setData(0, Qt::UserRole, x.first);
		item->setText(0, x.second);
		item->setIcon(0, theme::spacer(2));
		side->addTopLevelItem(item);
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

//TODO improve viewAbstract::themeChange()
void mainView::referenceBoxLayout()
{
	this->list_reference = new QWidget;

	list_reference->setObjectName("list_reference");
	list_reference->setHidden(true);

#ifndef Q_OS_MAC
	QColor referencebackground;
	QColor referencecolor;
	QString referencebackground_hexArgb;
	QString referencecolor_hexArgb;

	referencebackground = QPalette().color(QPalette::Mid).lighter();
	referencecolor = QColor(Qt::black);
	referencebackground_hexArgb = referencebackground.name(QColor::HexArgb);
	referencecolor_hexArgb = referencecolor.name(QColor::HexArgb);

	theme->dynamicStyleSheet(list_reference, "#list_reference { color: " + referencecolor_hexArgb + "; background: " + referencebackground_hexArgb + " }", theme::light);

	referencebackground = QPalette().color(QPalette::Mid).darker();
	referencecolor = QColor(Qt::white);
	referencebackground_hexArgb = referencebackground.name(QColor::HexArgb);
	referencecolor_hexArgb = referencecolor.name(QColor::HexArgb);

	theme->dynamicStyleSheet(list_reference, "#list_reference { color: " + referencecolor_hexArgb + "; background: " + referencebackground_hexArgb + " }", theme::dark);
#else
	theme->dynamicStyleSheet(list_reference, "#list_reference { border-top: 1px solid }");

	QColor referencebackground;
	QColor referenceshade;
	QString referencebackground_hexArgb;
	QString referenceshade_hexArgb;

	referencebackground = QPalette().color(QPalette::Mid).lighter();
	referencebackground_hexArgb = referencebackground.name(QColor::HexArgb);
	referenceshade = QColor(Qt::black);
	referenceshade.setAlphaF(0.06);
	referenceshade_hexArgb = referenceshade.name(QColor::HexArgb);

	theme->dynamicStyleSheet(list_reference, "#list_reference { border-color: " + referenceshade_hexArgb + "; background: " + referencebackground_hexArgb + " }", theme::light);

	referencebackground = QPalette().color(QPalette::Mid).darker();
	referencebackground_hexArgb = referencebackground.name(QColor::HexArgb);
	referenceshade = QPalette().color(QPalette::Dark).darker();
	referenceshade.setAlphaF(0.20);
	referenceshade_hexArgb = referenceshade.name(QColor::HexArgb);

	theme->dynamicStyleSheet(list_reference, "#list_reference { border-color: " + referenceshade_hexArgb + "; background: " + referencebackground_hexArgb + " }", theme::dark);
#endif
	theme->dynamicStyleSheet(list_reference, "#list_reference_area, #list_reference_wrap { background: transparent }");

	QGridLayout* ref_frm = new QGridLayout(list_reference);
	QScrollArea* ref_area = new QScrollArea;
	ref_area->setObjectName("list_reference_area");
	QWidget* ref_wrap = new QWidget;
	ref_wrap->setObjectName("list_reference_wrap");

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
	debug("load");

	tabUpdateFlags(gui::init);

	auto* dbih = this->data->dbih;

	unordered_map<string, QTreeWidgetItem*> bgroups;

	for (auto & bsi : dbih->index["bss"])
	{
		debug("load", "bouquet", bsi.second);
		e2db::bouquet gboq = dbih->bouquets[bsi.second];
		QString qbs = QString::fromStdString(bsi.second);
		QString name = QString::fromStdString(gboq.nname.empty() ? gboq.name : gboq.nname);

		QTreeWidgetItem* bitem = new QTreeWidgetItem();
		bitem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
		bitem->setData(0, Qt::UserRole, qbs);
		bitem->setText(0, name);
		tree->addTopLevelItem(bitem);
		tree->expandItem(bitem);

		for (string & ubname : gboq.userbouquets)
			bgroups[ubname] = bitem;
	}
	for (auto & ubi : dbih->index["ubs"])
	{
		debug("load", "userbouquet", ubi.second);
		e2db::userbouquet uboq = dbih->userbouquets[ubi.second];
		QString qub = QString::fromStdString(ubi.second);
		QTreeWidgetItem* pgroup = bgroups[ubi.second];
		QTreeWidgetItem* bitem = new QTreeWidgetItem(pgroup);
		bitem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled | Qt::ItemNeverHasChildren);
		bitem->setData(0, Qt::UserRole, qub);
		bitem->setText(0, e2db::fixUnicodeChars(uboq.name));
		tree->addTopLevelItem(bitem);
	}

	tree->setDragEnabled(true);
	tree->setAcceptDrops(true);

	side->setCurrentItem(side->topLevelItem(0));

	populate(side);

	updateFlags();
	updateStatusBar();
}

void mainView::reset()
{
	debug("reset");

	unsetPendingUpdateListIndex();

	this->state.dnd = true;
	this->state.vlx_pending = false;
	this->state.refbox = list_reference->isVisible();
	this->state.tc = 0;
	this->state.ti = 0;
	this->state.curr = "";
	this->state.sort = pair (-1, Qt::AscendingOrder); //C++17

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

	cache.clear();

	this->lsr_find.curr = -1;
	this->lsr_find.match.clear();

	this->action.list_addch->setDisabled(true);
	this->action.list_addmk->setDisabled(true);
	this->action.list_newch->setEnabled(true);
	this->action.list_dnd->setDisabled(true);

	resetStatusBar();
}

void mainView::populate(QTreeWidget* tw)
{
	string curr;
	string prev;

	bool precached = false;
	QList<QTreeWidgetItem*> items_prev;

	if (! cache.empty())
	{
		for (int i = 0; i < list->topLevelItemCount(); i++)
		{
			QTreeWidgetItem* item = list->topLevelItem(i);
			items_prev.append(item->clone());
		}
		precached = true;
		prev = string (this->state.curr);
	}

	QTreeWidgetItem* selected = tw->currentItem();
	if (selected == NULL)
	{
		return;
	}
	if (selected != NULL)
	{
		QString qub = selected->data(0, Qt::UserRole).toString();
		curr = qub.toStdString();
		this->state.curr = curr;
	}

	string bname = this->state.curr;

	auto* dbih = this->data->dbih;

	if (dbih->index.count(bname))
		debug("populate", "current", bname);
	else
		error("populate", "current", bname);

	list->header()->setSortIndicatorShown(true);
	list->header()->setSectionsClickable(false);
	list->clear();

	if (precached)
	{
		cache[prev].swap(items_prev);
	}

	int i = 0;

	if (cache[bname].isEmpty())
	{
		for (auto & ch : dbih->index[bname])
		{
			char ci[7];
			std::snprintf(ci, 7, "%06d", i++);
			bool marker = false;
			QString chid = QString::fromStdString(ch.second);
			QString x = QString::fromStdString(ci);
			QString idx;
			QStringList entry;

			if (dbih->db.services.count(ch.second))
			{
				entry = dbih->entries.services[ch.second];
				//TODO TEST idx changed after edit
				idx = QString::number(ch.first);
				entry.prepend(idx);
				entry.prepend(x);
			}
			else
			{
				e2db::channel_reference chref;
				if (dbih->userbouquets.count(bname))
					chref = dbih->userbouquets[bname].channels[ch.second];

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
					entry = QStringList({x, NULL, NULL, chid, NULL, NULL, NULL, "ERROR", NULL});
					// idx = 0; //Qt5
					error("populate", "chid", ch.second);
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

void mainView::treeSwitched(QTreeWidget* tw, QTreeWidgetItem* item)
{
	debug("treeSwitched");

	int tc = -1;

	if (tw == side)
		tc = 0;
	else if (tw == tree)
		tc = 1;

	if (tc != this->state.tc)
	{
		switch (tc)
		{
			case 0: return this->servicesItemChanged(item);
			case 1: return this->treeItemChanged(item);
		}
	}
}

void mainView::servicesItemChanged(QTreeWidgetItem* current)
{
	debug("servicesItemChanged");

	this->state.tc = 0;

	if (current != NULL)
	{
		int ti = side->indexOfTopLevelItem(current);

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

	updateListIndex();

	populate(side);

	updateFlags();
	updateStatusBar(true);
}

void mainView::treeItemChanged(QTreeWidgetItem* current)
{
	debug("treeItemChanged");

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

	updateListIndex();

	populate(tree);

	updateFlags();
	updateStatusBar(true);
}

void mainView::treeItemDoubleClicked()
{
	debug("treeItemDoubleClicked");

	QList<QTreeWidgetItem*> selected = tree->selectedItems();

	if (selected.empty() || selected.count() > 1)
		return;

	// userbouquet
	if (this->state.ti == -1)
		editUserbouquet();
}

void mainView::listItemChanged()
{
	// debug("listItemChanged");

	if (list_evto->isChanged())
		listPendingUpdate();
}

void mainView::listItemSelectionChanged()
{
	// debug("listItemSelectionChanged");

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
	else
	{
		tabSetFlag(gui::TabListEditService, false);
		tabSetFlag(gui::TabListEditMarker, false);

	}

	if (this->state.refbox)
		updateReferenceBox();
}

void mainView::listItemDoubleClicked()
{
	debug("listItemDoubleClicked");

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
	debug("listPendingUpdate");

	// sorting default
	if (this->state.dnd)
	{
		QTimer::singleShot(0, [=]() {
			this->visualReindexList();
		});
	}
	else
	{
		this->state.vlx_pending = true;
	}

	setPendingUpdateListIndex();
}

void mainView::visualReindexList()
{
	debug("visualReindexList");

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
		std::snprintf(ci, 7, "%06d", idx++);
		item->setText(ITEM_ROW_ROLE::x, QString::fromStdString(ci));
		if (! marker)
			item->setText(ITEM_ROW_ROLE::chnum, QString::number(idx - y));
		i++;
		y = marker ? reverse ? y - 1 : y + 1 : y;
	}

	this->state.vlx_pending = false;
}

void mainView::visualReloadList()
{
	debug("visualReloadList");

	auto* dbih = this->data->dbih;

	int i = 0;
	int j = list->topLevelItemCount();

	while (i != j)
	{
		QTreeWidgetItem* item = list->topLevelItem(i);
		string chid = item->data(ITEM_DATA_ROLE::chid, Qt::UserRole).toString().toStdString();

		QStringList entry = dbih->entries.services[chid];
		entry.prepend(item->text(ITEM_ROW_ROLE::chnum));
		entry.prepend(item->text(ITEM_ROW_ROLE::x));
		for (int i = 0; i < entry.count(); i++)
			item->setText(i, entry[i]);
		i++;
	}
}

void mainView::sortByColumn(int column)
{
	debug("sortByColumn", "column", column);

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

		if (this->state.vlx_pending)
			this->visualReindexList();
	}
	this->state.sort = pair (column, order); //C++17
}

void mainView::allowDnD()
{
	debug("allowDnd");

	this->list_evth->allowInternalMove();
	this->state.dnd = true;
}

void mainView::disallowDnD()
{
	debug("disallowDnD");

	this->list_evth->disallowInternalMove();
	this->state.dnd = false;
}

void mainView::reharmDnD()
{
	debug("reharmDnD");

	// sorting default 0|asc
	list->sortItems(0, Qt::AscendingOrder);
	list->setDragEnabled(true);
	list->setAcceptDrops(true);
	this->state.sort = pair (0, Qt::AscendingOrder); //C++17
	this->action.list_dnd->setDisabled(true);
}

void mainView::addUserbouquet()
{
	debug("addUserbouquet");

	string bname;
	e2se_gui::editBouquet* add = new e2se_gui::editBouquet(this->data, this->state.ti);
	add->display(cwid);
	bname = add->getAddId();
	add->destroy();

	auto* dbih = this->data->dbih;

	if (dbih->userbouquets.count(bname))
		debug("addUserbouquet", "bname", bname);
	else
		return error("addUserbouquet", "bname", bname);

	tree->setDragEnabled(false);
	tree->setAcceptDrops(false);

	e2db::userbouquet uboq = dbih->userbouquets[bname];
	e2db::bouquet gboq = dbih->bouquets[uboq.pname];
	int idx = gboq.btype == 1 ? 0 : 1;
	int i = 0, y;
	QTreeWidgetItem* current = tree->currentItem();
	bool isTopLevel = current != nullptr && tree->indexOfTopLevelItem(current) != -1;
	QTreeWidgetItem* parent = current != nullptr && ! isTopLevel ? current->parent() : tree->topLevelItem(idx);
	i = current != nullptr && ! isTopLevel ? parent->indexOfChild(current) : -1;
	y = i + 1;

	QTreeWidgetItem* item = new QTreeWidgetItem;
	item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled | Qt::ItemNeverHasChildren);
	item->setData(0, Qt::UserRole, QString::fromStdString(uboq.bname));
	item->setText(0, e2db::fixUnicodeChars(uboq.name));

	parent->insertChild(y, item);

	tree->setDragEnabled(true);
	tree->setAcceptDrops(true);

	updateTreeIndex();

	this->data->setChanged(true);
}

void mainView::editUserbouquet()
{
	debug("editUserbouquet");

	QList<QTreeWidgetItem*> selected = tree->selectedItems();

	if (selected.empty() || selected.count() > 1)
		return;

	QTreeWidgetItem* item = selected.first();
	QString qub = item->data(0, Qt::UserRole).toString();
	string bname = qub.toStdString();

	e2se_gui::editBouquet* edit = new e2se_gui::editBouquet(this->data, this->state.ti);
	edit->setEditId(bname);
	edit->display(cwid);
	edit->destroy();

	auto* dbih = this->data->dbih;

	if (dbih->userbouquets.count(bname))
		debug("editUserbouquet", "bname", bname);
	else
		return error("editUserbouquet", "bname", bname);

	e2db::userbouquet uboq = dbih->userbouquets[bname];
	item->setText(0, e2db::fixUnicodeChars(uboq.name));

	updateTreeIndex();

	this->data->setChanged(true);
}

void mainView::addChannel()
{
	debug("addChannel");

	int stype = -1;
	QTreeWidgetItem* selected = tree->currentItem();

	auto* dbih = this->data->dbih;

	if (selected == NULL)
	{
		return;
	}
	if (selected != NULL)
	{
		QTreeWidgetItem* parent = selected->parent();
		if (parent == NULL)
		{
			return;
		}
		else
		{
			QString qub = parent->data(0, Qt::UserRole).toString();
			string bname = qub.toStdString();
			e2db::bouquet bsoq = dbih->bouquets[bname];
			stype = bsoq.btype;
		}
	}

	e2se_gui::dialChannelBook* book = new e2se_gui::dialChannelBook(this->data, stype);
	book->setEventCallback([=](vector<QString> items) { this->putListItems(items); });
	book->display(cwid);
}

void mainView::addService()
{
	debug("addService");

	string chid;
	bool reload = false;
	e2se_gui::editService* add = new e2se_gui::editService(this->data);
	add->display(cwid);
	chid = add->getAddId();
	reload = ! (add->getTransponderId()).empty();
	add->destroy();

	auto* dbih = this->data->dbih;

	if (dbih->db.services.count(chid))
		debug("addService", "chid", chid);
	else
		return error("addService", "chid", chid);

	cache.clear();

	if (reload)
		dbih->cache(true);

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
	std::snprintf(ci, 7, "%06d", i++);
	QString x = QString::fromStdString(ci);
	QString idx = QString::number(i);
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

	if (reload)
		visualReloadList();
	// sorting default
	if (this->state.dnd)
		visualReindexList();
	else
		this->state.vlx_pending = true;

	setPendingUpdateListIndex();

	updateFlags();
	updateStatusBar();

	this->data->setChanged(true);
}

void mainView::editService()
{
	debug("editService");

	QList<QTreeWidgetItem*> selected = list->selectedItems();

	if (selected.empty() || selected.count() > 1)
		return;

	QTreeWidgetItem* item = selected.first();
	string chid = item->data(ITEM_DATA_ROLE::chid, Qt::UserRole).toString().toStdString();
	string nw_chid;
	bool reload = false;
	bool marker = item->data(ITEM_DATA_ROLE::marker, Qt::UserRole).toBool();

	auto* dbih = this->data->dbih;

	debug("editService", "chid", chid);

	if (! marker && dbih->db.services.count(chid))
		debug("editService", "chid", chid);
	else
		return error("editService", "chid", chid);

	e2se_gui::editService* edit = new e2se_gui::editService(this->data);
	edit->setEditId(chid);
	edit->display(cwid);
	nw_chid = edit->getEditId();
	reload = ! (edit->getTransponderId()).empty();
	edit->destroy();

	cache.clear();

	if (reload)
		dbih->cache(true);

	if (dbih->db.services.count(nw_chid))
		debug("editService", "new chid", nw_chid);
	else
		return error("editService", "new chid", nw_chid);

	QStringList entry = dbih->entries.services[nw_chid];
	entry.prepend(item->text(ITEM_ROW_ROLE::chnum));
	entry.prepend(item->text(ITEM_ROW_ROLE::x));
	for (int i = 0; i < entry.count(); i++)
		item->setText(i, entry[i]);
	item->setData(ITEM_DATA_ROLE::chid, Qt::UserRole, QString::fromStdString(nw_chid));

	if (reload)
		visualReloadList();

	this->data->setChanged(true);
}

void mainView::addMarker()
{
	debug("addMarker");

	string chid;
	string bname = this->state.curr;
	e2se_gui::editMarker* add = new e2se_gui::editMarker(this->data);
	add->setAddId(bname);
	add->display(cwid);
	chid = add->getAddId();
	add->destroy();

	auto* dbih = this->data->dbih;

	e2db::channel_reference chref;
	if (dbih->userbouquets.count(bname))
		chref = dbih->userbouquets[bname].channels[chid];

	if (chref.marker)
		debug("addMarker", "chid", chid);
	else
		return error("addMarker", "chid", chid);

	cache[bname].clear();

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
	std::snprintf(ci, 7, "%06d", i++);
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
		this->state.vlx_pending = true;

	setPendingUpdateListIndex();

	updateFlags();
	updateStatusBar();

	this->data->setChanged(true);
}

void mainView::editMarker()
{
	debug("editMarker");

	QList<QTreeWidgetItem*> selected = list->selectedItems();

	if (selected.empty() || selected.count() > 1)
		return;

	QTreeWidgetItem* item = selected.first();
	string chid = item->data(ITEM_DATA_ROLE::chid, Qt::UserRole).toString().toStdString();
	string nw_chid;
	string bname = this->state.curr;
	bool marker = item->data(ITEM_DATA_ROLE::marker, Qt::UserRole).toBool();

	if (marker)
		debug("editMarker", "chid", chid);
	else
		return error("editMarker", "chid", chid);

	e2se_gui::editMarker* edit = new e2se_gui::editMarker(this->data);
	edit->setEditId(chid, bname);
	edit->display(cwid);
	nw_chid = edit->getEditId();
	edit->destroy();

	auto* dbih = this->data->dbih;

	e2db::channel_reference chref;
	if (dbih->userbouquets.count(bname))
		chref = dbih->userbouquets[bname].channels[chid];

	if (chref.marker)
		debug("editMarker", "new chid", nw_chid);
	else
		return error("editMarker", "new chid", nw_chid);

	cache[bname].clear();

	QStringList entry = dbih->entryMarker(chref);
	entry.prepend(item->text(ITEM_ROW_ROLE::x));
	for (int i = 0; i < entry.count(); i++)
		item->setText(i, entry[i]);
	item->setData(ITEM_DATA_ROLE::chid, Qt::UserRole, QString::fromStdString(nw_chid));

	this->data->setChanged(true);
}

void mainView::treeItemDelete()
{
	debug("treeItemDelete");

	QList<QTreeWidgetItem*> selected = tree->selectedItems();

	if (selected.empty())
	{
		return;
	}

	bool remove = tabRemoveQuestion("Confirm deletetion", "Do you want to delete items?");
	if (! remove)
		return;

	auto* dbih = this->data->dbih;

	for (auto & item : selected)
	{
		QString qub = item->data(0, Qt::UserRole).toString();
		string bname = qub.toStdString();
		e2db::userbouquet uboq = dbih->userbouquets[bname];
		string pname = uboq.pname;
		dbih->removeUserbouquet(bname);

		cache[bname].clear();
		cache[pname].clear();
		cache.erase(bname);

		QTreeWidgetItem* parent = item->parent();
		parent->removeChild(item);
	}

	setPendingUpdateListIndex();
	updateTreeIndex();

	updateStatusBar();

	this->data->setChanged(true);
}

void mainView::listReferenceToggle()
{
	debug("listReferenceToggle");

	if (list_reference->isHidden())
	{
		list_reference->show();
		this->state.refbox = true;
		updateReferenceBox();
	}
	else
	{
		list_reference->hide();
		this->state.refbox = false;
	}
}

//TODO CSV Tools compatibility
void mainView::listItemCopy(bool cut)
{
	debug("listItemCopy");

	QList<QTreeWidgetItem*> selected = list->selectedItems();

	if (selected.empty())
		return;

	auto* dbih = this->data->dbih;

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
			// chtname
			else if (i == ITEM_ROW_ROLE::chtname && ! marker)
				qstr.prepend("\"").append("\"");
			data.append(qstr);
		}

		// Reference ID
		QString refid;
		// bouquets tree
		if (this->state.tc)
		{
			string bname = this->state.curr;
			e2db::channel_reference chref;
			if (dbih->userbouquets.count(bname))
				chref = dbih->userbouquets[bname].channels[chid];
			refid = QString::fromStdString(dbih->get_reference_id(chref));
		}
		// services tree
		else
		{
			refid = QString::fromStdString(dbih->get_reference_id(chid));
		}
		data.insert(2, refid); // insert refid column [2]
		text.append(data.join(",")); // CSV
	}
	clipboard->setText(text.join("\n")); // CSV

	if (cut)
		listItemDelete();
}

//TODO validate
void mainView::listItemPaste()
{
	debug("listItemPaste", "entered", ! (this->state.tc && this->state.ti != -1));

	// services tree && bouquet: tv | radio
	if (this->state.tc && this->state.ti != -1)
		return;

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

	auto* dbih = this->data->dbih;

	if (! items.empty())
	{
		putListItems(items);

		if (list->currentItem() == nullptr)
			list->scrollToBottom();

		// bouquets tree
		if (this->state.tc)
		{
			string bname = this->state.curr;
			e2db::userbouquet uboq = dbih->userbouquets[bname];
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
	debug("listItemDelete", "entered", ! (this->state.tc && this->state.ti != -1));

	// services tree && bouquet: tv | radio
	if (this->state.tc && this->state.ti != -1)
		return;

	QList<QTreeWidgetItem*> selected = list->selectedItems();

	if (selected.empty())
		return;

	bool remove = tabRemoveQuestion("Confirm deletetion", "Do you want to delete items?");
	if (! remove)
		return;

	list->header()->setSectionsClickable(false);
	list->setDragEnabled(false);
	list->setAcceptDrops(false);

	string pname;
	string bname = this->state.curr;

	auto* dbih = this->data->dbih;

	// bouquets tree
	if (this->state.tc)
	{
		e2db::userbouquet uboq = dbih->userbouquets[bname];
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
			dbih->removeChannelReference(chid, bname);

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
		this->state.vlx_pending = true;

	setPendingUpdateListIndex();

	updateFlags();
	updateStatusBar();

	this->data->setChanged(true);
}

//TODO CSV Tools compatibility
void mainView::putListItems(vector<QString> items)
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

	string bname = this->state.curr;
	e2db::userbouquet uboq = dbih->userbouquets[bname];
	int ub_idx = uboq.index;
	int anum_count = int (dbih->index["mks"].size());

	for (QString & q : items)
	{
		char ci[7];
		std::snprintf(ci, 7, "%06d", i++);
		QString x = QString::fromStdString(ci);
		QString idx = QString::number(i);

		string refid;
		string value;
		e2db::channel_reference chref;
		e2db::service_reference ref;

		QStringList qs;
		if (q.contains(','))
		{
			qs = q.split(',');
			refid = qs[2].toStdString();
			value = qs[1].replace("\"", "").toStdString();
		}
		else
		{
			refid = q.toStdString();
		}

		QStringList entry;

		dbih->parse_channel_reference(refid, chref, ref);

		char chid[25];

		if (chref.marker)
		{
			anum_count++;

			// %4d:%2x:%d
			std::snprintf(chid, 25, "%d:%x:%d", chref.atype, anum_count, ub_idx);
		}
		else
		{
			// %4x:%4x:%8x
			std::snprintf(chid, 25, "%x:%x:%x", ref.ssid, ref.tsid, ref.dvbns);
		}

		if (dbih->db.services.count(chid))
		{
			entry = dbih->entries.services[chid];
			entry.prepend(idx);
			entry.prepend(x);

			chref.marker = false;
			chref.chid = chid;
			chref.atype = 0;
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

				entry = dbih->entryMarker(chref);
				entry.prepend(x);
			}
			else if (! qs.isEmpty())
			{
				e2db::service ch;
				e2db::transponder tx;
				e2db::fec fec;

				ch.ssid = ref.ssid;
				ch.tsid = tx.tsid = ref.tsid;
				ch.dvbns = tx.dvbns = ref.dvbns;
				ch.onid = tx.onid = ref.onid;
				ch.chname = value;
				if (! qs[3].isEmpty())
					ch.ssid = ref.ssid = qs[3].toInt();
				if (! qs[4].isEmpty())
					ch.tsid = tx.tsid = ref.tsid = qs[4].toInt();
				ch.stype = dbih->value_service_type(qs[5].toStdString());
				//TODO
				// ch.data[e2db::SDATA::C]; qs[6]
				ch.data[e2db::SDATA::p] = dbih->value_channel_provider(qs[7].replace("\"", "").toStdString());
				tx.tsid = ch.tsid;
				tx.dvbns = ch.dvbns;
				tx.sys = dbih->value_transponder_system(qs[8].toStdString());
				tx.ytype = dbih->value_transponder_type(qs[8].toStdString());
				tx.pos = dbih->value_transponder_position(qs[9].toStdString());
				tx.freq = qs[11].toInt();
				tx.pol = dbih->value_transponder_polarization(qs[12].toStdString());
				tx.sr = qs[13].toInt();
				dbih->value_transponder_fec(qs[14].toStdString(), tx.ytype, fec);
				if (tx.ytype == e2db::YTYPE::satellite)
				{
					tx.fec = fec.inner_fec;
				}
				else if (tx.ytype == e2db::YTYPE::terrestrial)
				{
					tx.hpfec = fec.hp_fec;
					tx.lpfec = fec.lp_fec;
				}
				else if (tx.ytype == e2db::YTYPE::cable)
				{
					tx.fec = fec.inner_fec;
				}

				char txid[25];
				// %4x:%8x
				std::snprintf(txid, 25, "%x:%x", tx.tsid, tx.dvbns);
				tx.txid = ch.txid = txid;

				char chid[25];
				std::snprintf(chid, 25, "%x:%x:%x", ref.ssid, ref.tsid, ref.dvbns);
				ch.chid = chid;

				chref.chid = ch.chid;
				chref.index = idx.toInt();

				if (! dbih->db.transponders.count(tx.txid))
					dbih->addTransponder(tx);
				if (! dbih->db.services.count(ch.chid))
					dbih->addService(ch);

				entry = dbih->entries.services[chid];
				entry.prepend(idx);
				entry.prepend(x);
			}
			else
			{
				error("putListItems", "refid", refid);
				continue;
			}
		}
		QTreeWidgetItem* item = new QTreeWidgetItem(entry);
		item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsDragEnabled | Qt::ItemNeverHasChildren);
		item->setData(ITEM_DATA_ROLE::idx, Qt::UserRole, idx);
		item->setData(ITEM_DATA_ROLE::marker, Qt::UserRole, chref.marker);
		item->setData(ITEM_DATA_ROLE::chid, Qt::UserRole, QString::fromStdString(chref.chid));
		item->setIcon(ITEM_ROW_ROLE::chnum, theme::spacer(4));
		if (chref.marker)
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
			dbih->addChannelReference(chref, bname);
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
		this->state.vlx_pending = true;

	setPendingUpdateListIndex();

	updateFlags();
	updateStatusBar();

	this->data->setChanged(true);
}

void mainView::showTreeEditContextMenu(QPoint& pos)
{
	debug("showTreeEditContextMenu");

	QList<QTreeWidgetItem*> selected = tree->selectedItems();

	if (selected.empty())
		return;

	QMenu* tree_edit = contextMenu();

	// userbouquet
	if (this->state.ti == -1)
	{
		contextMenuAction(tree_edit, "Edit Userbouquet", [=]() { this->editUserbouquet(); }, tabGetFlag(gui::TabTreeEdit));
		contextMenuSeparator(tree_edit);
		contextMenuAction(tree_edit, "Delete", [=]() { this->treeItemDelete(); }, tabGetFlag(gui::TabTreeDelete));
		contextMenuSeparator(tree_edit);
	}
	contextMenuAction(tree_edit, "Export", [=]() { tabExportFile(); });

	platform::osContextMenuPopup(tree_edit, tree, pos);
	// tree_edit->exec(tree->mapToGlobal(pos));
}

void mainView::showListEditContextMenu(QPoint& pos)
{
	debug("showListEditContextMenu");

	QList<QTreeWidgetItem*> selected = list->selectedItems();

	if (selected.empty())
		return;

	bool marker = false;
	bool editable = false;

	if (selected.count() == 1)
	{
		QTreeWidgetItem* item = selected.first();
		marker = item->data(ITEM_DATA_ROLE::marker, Qt::UserRole).toBool();
		editable = true;
	}

	QMenu* list_edit = contextMenu();

	if (marker)
		contextMenuAction(list_edit, "Edit Marker", [=]() { this->editMarker(); }, editable && tabGetFlag(gui::TabListEditMarker));
	else
		contextMenuAction(list_edit, "Edit Service", [=]() { this->editService(); }, editable && tabGetFlag(gui::TabListEditService));
	contextMenuSeparator(list_edit);
	contextMenuAction(list_edit, "Cu&t", [=]() { this->listItemCut(); }, tabGetFlag(gui::TabListCut), QKeySequence::Cut);
	contextMenuAction(list_edit, "&Copy", [=]() { this->listItemCopy(); }, tabGetFlag(gui::TabListCopy), QKeySequence::Copy);
	contextMenuAction(list_edit, "&Paste", [=]() { this->listItemPaste(); }, tabGetFlag(gui::TabListPaste), QKeySequence::Paste);
	contextMenuSeparator(list_edit);
	contextMenuAction(list_edit, "&Delete", [=]() { this->listItemDelete(); }, tabGetFlag(gui::TabListDelete), QKeySequence::Delete);

	platform::osContextMenuPopup(list_edit, list, pos);
	// list_edit->exec(list->mapToGlobal(pos));
}

void mainView::treeAfterDrop(QTreeWidget* tw, QTreeWidgetItem* current)
{
	debug("treeAfterDrop");

	if (tw == list)
		treeDropFromList(current);
	else
		treeDropFromTree(current);

	this->data->setChanged(true);
}

void mainView::listAfterDrop(QTreeWidget* tw)
{
	debug("listAfterDrop");

	this->data->setChanged(true);
}

void mainView::treeDropFromTree(QTreeWidgetItem* current)
{
	debug("treeDropFromTree");

	QTreeWidgetItem* parent = current->parent();
	QList<QTreeWidgetItem*> selected = tree->selectedItems();

	if (parent == nullptr || selected.empty())
		return;

	QList<QTreeWidgetItem*> items;

	for (auto & item : selected)
	{
		if (item->parent() != parent)
			return;

		items.append(item->clone());
	}
	int index = parent->indexOfChild(current);
	for (auto & item : selected)
		parent->removeChild(item);

	parent->insertChildren(index, items);

	updateTreeIndex();

	tree->setCurrentItem(items.first());
}

void mainView::treeDropFromList(QTreeWidgetItem* current)
{
	debug("treeDropFromList");

	if (tree->currentItem() == current)
		return;

	QList<QTreeWidgetItem*> selected = list->selectedItems();

	if (selected.empty())
		return;

	QList<QTreeWidgetItem*> items;

	for (auto & item : selected)
		items.append(item->clone());

	if (QSettings().value("preference/treeDropMove", false).toBool())
	{
		for (auto & item : selected)
			delete item;
	}

	if (QSettings().value("preference/treeCurrentAfterDrop", true).toBool())
	{
		tree->setCurrentItem(current);

		list->addTopLevelItems(items);
		list->scrollToBottom();
	}

	updateListReferences(current, items);
}

void mainView::updateStatusBar(bool current)
{
	debug("updateStatusBar");

	gui::status msg;
	msg.update = current;

	auto* dbih = this->data->dbih;

	if (current && ! this->state.curr.empty())
	{
		string bname = this->state.curr;
		msg.counters[gui::COUNTER::n_bouquet] = int (dbih->index[bname].size());

		// bouquets tree
		if (this->state.tc)
			msg.curr = bname;
	}
	else
	{
		msg.counters[gui::COUNTER::n_data] = int (dbih->index["chs:0"].size());
		msg.counters[gui::COUNTER::n_tv] = int (dbih->index["chs:1"].size());
		msg.counters[gui::COUNTER::n_radio] = int (dbih->index["chs:2"].size());
		msg.counters[gui::COUNTER::n_services] = int (dbih->index["chs"].size());
	}

	tabSetStatusBar(msg);
}

void mainView::updateReferenceBox()
{
	debug("updateReferenceBox");

	QList<QTreeWidgetItem*> selected = list->selectedItems();

	auto* dbih = this->data->dbih;

	if (selected.empty() || selected.count() > 1)
	{
		for (auto & field : ref_fields)
			field.second->setText(selected.empty() ? "< >" : "< … >");
	}
	else
	{
		QTreeWidgetItem* item = selected[0];
		string chid = item->data(ITEM_DATA_ROLE::chid, Qt::UserRole).toString().toStdString();
		QString ssid, refid, txp, tns, bsls, ubls;

		// debug("updateReferenceBox", "chid", chid);

		// bouquets tree
		if (this->state.tc)
		{
			string bname = this->state.curr;
			e2db::channel_reference chref = dbih->userbouquets[bname].channels[chid];
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

			ssid = QString::number(ch.ssid);

			if (ch.tsid != 0)
			{
				e2db::transponder tx = dbih->db.transponders[ch.txid];

				string ptxp = dbih->value_transponder_combo(tx);
				string sys = dbih->value_transponder_system(tx);
				string pos = dbih->value_transponder_position(tx);
				string tname = dbih->get_tuner_name(tx);

				txp = QString::fromStdString(ptxp);

				string ppos;
				if (tname.empty())
					ppos = pos;
				else
					ppos = tname + ' ' + '(' + pos + ')';

				tns = "<p style=\"line-height: 125%\">" + QString::fromStdString(sys + "<br>" + ppos) + "</p>";
			}
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

void mainView::updateFlags()
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

void mainView::updateTreeIndex()
{
	debug("updateTreeIndex");

	int i = 0, y;
	int count = tree->topLevelItemCount();
	vector<pair<int, string>> bss;
	vector<pair<int, string>> ubs;
	unordered_map<string, vector<string>> index;

	while (i != count)
	{
		QTreeWidgetItem* parent = tree->topLevelItem(i);
		QString qbs = parent->data(0, Qt::UserRole).toString();
		string pname = qbs.toStdString();
		bss.emplace_back(pair (i, pname)); //C++17
		y = 0;

		if (parent->childCount())
		{
			int childs = parent->childCount();
			while (y != childs)
			{
				QTreeWidgetItem* item = parent->child(y);
				QString qub = item->data(0, Qt::UserRole).toString();
				string bname = qub.toStdString();
				ubs.emplace_back(pair (i, bname)); //C++17
				index[pname].emplace_back(bname);
				y++;
			}
		}
		i++;
	}

	auto* dbih = this->data->dbih;

	if (bss != dbih->index["bss"])
	{
		dbih->index["bss"].swap(bss);
	}
	if (ubs != dbih->index["ubs"])
	{
		dbih->index["ubs"].swap(ubs);

		for (auto & x : dbih->bouquets)
			x.second.userbouquets.swap(index[x.first]);
	}
}

void mainView::updateListIndex()
{
	if (! this->state.chx_pending)
		return;

	int i = 0, idx = 0;
	int count = list->topLevelItemCount();
	string bname = this->state.curr;

	auto* dbih = this->data->dbih;

	dbih->index[bname].clear();

	debug("updateListIndex", "current", bname);

	int sort_col = list->sortColumn();
	list->sortItems(0, Qt::AscendingOrder);

	while (i != count)
	{
		QTreeWidgetItem* item = list->topLevelItem(i);
		string chid = item->data(mainView::ITEM_DATA_ROLE::chid, Qt::UserRole).toString().toStdString();
		bool marker = item->data(mainView::ITEM_DATA_ROLE::marker, Qt::UserRole).toBool();
		idx = marker ? 0 : i + 1;
		dbih->index[bname].emplace_back(pair (idx, chid)); //C++17
		i++;
	}

	list->sortItems(this->state.sort.first, this->state.sort.second);
	list->header()->setSortIndicator(sort_col, this->state.sort.second);

	this->state.chx_pending = false;
}

void mainView::updateListReferences(QTreeWidgetItem* current, QList<QTreeWidgetItem*> items)
{
	QString qub = current->data(0, Qt::UserRole).toString();
	string bname = qub.toStdString();

	auto* dbih = this->data->dbih;

	debug("updateListReferences", "current", bname);

	for (auto & item : items)
	{
		string chid = item->data(mainView::ITEM_DATA_ROLE::chid, Qt::UserRole).toString().toStdString();

		if (! dbih->userbouquets[bname].channels.count(chid))
		{
			e2db::channel_reference chref;

			int idx = item->data(mainView::ITEM_DATA_ROLE::idx, Qt::UserRole).toInt();
			bool marker = item->data(mainView::ITEM_DATA_ROLE::marker, Qt::UserRole).toBool();

			chref.marker = marker;
			chref.chid = chid;
			chref.index = idx;

			if (marker)
			{
				string chname = item->text(mainView::ITEM_ROW_ROLE::chname).toStdString();
				int atype = 0;
				int anum = 0;
				int ub_idx = -1;

				std::sscanf(chid.c_str(), "%d:%x:%d", &atype, &anum, &ub_idx);

				chref.atype = atype;
				chref.anum = anum;
				chref.value = chname;
			}

			dbih->addChannelReference(chref, bname);
		}
	}
}

void mainView::setPendingUpdateListIndex()
{
	this->state.chx_pending = true;
}

void mainView::unsetPendingUpdateListIndex()
{
	this->state.chx_pending = false;
}

void mainView::updateIndex()
{
	updateTreeIndex();
	this->state.chx_pending = true;
	updateListIndex();
	this->state.chx_pending = false;
}

}
