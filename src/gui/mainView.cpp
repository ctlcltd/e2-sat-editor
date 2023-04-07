/*!
 * e2-sat-editor/src/gui/mainView.cpp
 *
 * @link https://github.com/ctlcltd/e2-sat-editor
 * @copyright e2 SAT Editor Team
 * @author Leonardo Laureti
 * @version 0.6
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

#include "toolkit/TreeProxyStyle.h"
#include "mainView.h"
#include "theme.h"
#include "tab.h"
#include "editBouquet.h"
#include "editUserbouquet.h"
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

mainView::~mainView()
{
	debug("~mainView");

	delete this->widget;
	delete this->theme;
	delete this->log;
}

void mainView::layout()
{
	debug("layout");

	widget->setStyleSheet("QGroupBox { spacing: 0; border: 0; padding: 0; padding-top: 32px; font-weight: bold } QGroupBox::title { margin: 8px 4px; padding: 0 1px 1px }");

	QGridLayout* frm = new QGridLayout(widget);

	//TODO vertical scrollbar 1px gap [Linux] [Windows]
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

	QGroupBox* sfrm = new QGroupBox(tr("Services"));
	QGroupBox* tfrm = new QGroupBox(tr("Bouquets"));
	QGroupBox* lfrm = new QGroupBox(tr("Channels"));

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

	TreeProxyStyle* side_style = new TreeProxyStyle;
	side->setStyle(side_style);
	TreeProxyStyle* tree_style = new TreeProxyStyle;
	tree->setStyle(tree_style);
	TreeProxyStyle* list_style = new TreeProxyStyle;
	list->setStyle(list_style);

	side->setStyleSheet("QTreeWidget { background: transparent } QTreeWidget::item { padding: 9px 0 }");
	tree->setStyleSheet("QTreeWidget { background: transparent } QTreeWidget::item { margin: 1px 0 0; padding: 8px 0 }");
	list->setStyleSheet("QTreeWidget::item { height: 32px }");

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
	side->setItemsExpandable(false);
	side->setExpandsOnDoubleClick(false);
	side->setRootIsDecorated(false);
	side->setIndentation(false);
	side_style->setIndentation(10, true);

	tree->setHeaderHidden(true);
	tree->setUniformRowHeights(true);
	tree->setSelectionBehavior(QTreeWidget::SelectRows);
	tree->setDragDropMode(QTreeWidget::DragDrop);
	tree->setDefaultDropAction(Qt::MoveAction);
	tree->setDropIndicatorShown(true);
	tree->setEditTriggers(QTreeWidget::NoEditTriggers);
	tree->setIndentation(1);
	tree_style->setIndentation(10);

	list->setUniformRowHeights(true);
	list->setSelectionBehavior(QTreeWidget::SelectRows);
	list->setSelectionMode(QTreeWidget::ExtendedSelection);
	list->setItemsExpandable(false);
	list->setExpandsOnDoubleClick(false);
	list->setDragDropMode(QTreeWidget::InternalMove);
	if (QSettings().value("preference/treeDropMove", false).toBool())
		list->setDefaultDropAction(Qt::MoveAction);
	else
		list->setDefaultDropAction(Qt::CopyAction);
	list->setDropIndicatorShown(true);
	list->setEditTriggers(QTreeWidget::NoEditTriggers);
	list->setRootIsDecorated(false);
	list->setIndentation(false);
	list_style->setIndentation(12, true);
	list_style->setFirstColumnIndent(1);

	QTreeWidgetItem* lheader_item = new QTreeWidgetItem({NULL, tr("Index"), tr("Name"), tr("Parental"), "CHID", "TXID", tr("Service ID"), tr("Transport ID"), tr("Type"), tr("CAS"), tr("Provider"), tr("System"), tr("Position"), tr("Tuner"), tr("Frequency"), tr("Polarization"), tr("Symbol Rate"), tr("FEC")});

	list->setHeaderItem(lheader_item);
	list->setColumnHidden(ITEM_ROW_ROLE::x, true);		// hidden index
	list->setColumnWidth(ITEM_ROW_ROLE::chnum, 65);		// (Channel Number) Index
	list->setColumnWidth(ITEM_ROW_ROLE::chname, 200);	// (Channel) Name
	list->setColumnWidth(ITEM_ROW_ROLE::chlock, 70);	// Parental lock
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

	// numeric items
	QTreeWidgetItem* tree_head = list->headerItem();
	tree_head->setData(ITEM_ROW_ROLE::chssid, Qt::UserRole, true);
	tree_head->setData(ITEM_ROW_ROLE::chtsid, Qt::UserRole, true);
	tree_head->setData(ITEM_ROW_ROLE::chfreq, Qt::UserRole, true);
	tree_head->setData(ITEM_ROW_ROLE::chsr, Qt::UserRole, true);

	list->header()->connect(list->header(), &QHeaderView::sectionClicked, [=](int column) { this->sortByColumn(column); });

	tree->setContextMenuPolicy(Qt::CustomContextMenu);
	tree->connect(tree, &QTreeWidget::customContextMenuRequested, [=](QPoint pos) { this->showTreeEditContextMenu(pos); });
	list->setContextMenuPolicy(Qt::CustomContextMenu);
	list->connect(list, &QTreeWidget::customContextMenuRequested, [=](QPoint pos) { this->showListEditContextMenu(pos); });

	searchLayout();
	referenceBoxLayout();

	QToolBar* tree_ats = toolBar();
	QToolBar* list_ats = toolBar();

	this->action.tree_newbq = toolBarAction(tree_ats, tr("New Bouquet", "toolbar"), theme->dynamicIcon("add"), [=]() { this->addUserbouquet(); });
	toolBarSpacer(tree_ats);
	this->action.tree_search = toolBarButton(tree_ats, tr("Find…", "toolbar"), theme->dynamicIcon("search"), [=]() { this->treeSearchToggle(); });

	this->action.tree_search->setDisabled(true);

	this->action.list_addch = toolBarAction(list_ats, tr("Add Channel", "toolbar"), theme->dynamicIcon("add"), [=]() { this->addChannel(); });
	this->action.list_addmk = toolBarAction(list_ats, tr("Add Marker", "toolbar"), theme->dynamicIcon("add"), [=]() { this->addMarker(); });
	this->action.list_newch = toolBarAction(list_ats, tr("New Service", "toolbar"), theme->dynamicIcon("add"), [=]() { this->addService(); });
	toolBarSeparator(list_ats);
	//: Encoding: take care about double ampersand
	this->action.list_dnd = toolBarButton(list_ats, tr("Drag&&Drop", "toolbar"), [=]() { this->reharmDnD(); });
	this->action.list_ref = toolBarButton(list_ats, tr("Reference", "toolbar"), [=]() { this->listReferenceToggle(); });
	toolBarSpacer(list_ats);
	this->action.list_search = toolBarButton(list_ats, tr("&Find…", "toolbar"), theme->dynamicIcon("search"), [=]() { this->listSearchToggle(); });

	this->action.list_dnd->setCheckable(true);
	this->action.list_ref->setCheckable(true);

	this->action.list_addmk->setDisabled(true);
	this->action.list_newch->setDisabled(true);
	this->action.list_dnd->setDisabled(true);
	this->action.list_search->setDisabled(true);

	this->tree_evth = new TreeEventHandler;
	this->list_evth = new TreeDragDropEventHandler;
	this->list_evto = new TreeItemChangedEventObserver;
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
	sfrm->setMaximumHeight(192);

	atbox->addWidget(tree);
	atbox->addWidget(tree_search);
	atbox->addWidget(tree_ats);
	tfrm->setLayout(atbox);

	tbox->addWidget(sfrm);
	tbox->addItem(new QSpacerItem(0, 4, QSizePolicy::Preferred, QSizePolicy::Fixed));
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
		{"chs", tr("All services")},
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
		side->addTopLevelItem(item);
	}
}

void mainView::searchLayout()
{
	this->viewAbstract::searchLayout();

	this->lsr_search.filter->addItem(tr("Name"), ITEM_ROW_ROLE::chname);
	this->lsr_search.filter->addItem(tr("Type"), ITEM_ROW_ROLE::chtype);
	this->lsr_search.filter->addItem(tr("CAS"), ITEM_ROW_ROLE::chcas);
	this->lsr_search.filter->addItem(tr("Provider"), ITEM_ROW_ROLE::chpname);
	this->lsr_search.filter->addItem(tr("Position"), ITEM_ROW_ROLE::chpos);
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
	QLabel* ref0lr = new QLabel(tr("Reference ID", "reference-box"));
	QLabel* ref0tr = new QLabel("< >");
	ref0lr->setFont(QFont(theme::fontFamily(), theme::calcFontSize(-2)));
	ref0tr->setAlignment(ref0tr->layoutDirection() == Qt::LeftToRight ? Qt::AlignLeft : Qt::AlignRight);
	ref_fields[LIST_REF::ReferenceID] = ref0tr;
	ref_box->addWidget(ref0lr, 0, 0, Qt::AlignTop);
	ref_box->addWidget(ref0tr, 0, 1, Qt::AlignTop);

	QLabel* ref1ls = new QLabel(tr("Service ID", "reference-box"));
	QLabel* ref1ts = new QLabel("< >");
	ref1ls->setFont(QFont(theme::fontFamily(), theme::calcFontSize(-2)));
	ref1ts->setAlignment(ref1ts->layoutDirection() == Qt::LeftToRight ? Qt::AlignLeft : Qt::AlignRight);
	ref_fields[LIST_REF::ServiceID] = ref1ts;
	ref_box->addWidget(ref1ls, 0, 2, Qt::AlignTop);
	ref_box->addWidget(ref1ts, 0, 3, Qt::AlignLeading | Qt::AlignTop);

	QLabel* ref2lt = new QLabel(tr("Transponder", "reference-box"));
	QLabel* ref2tt = new QLabel("< >");
	ref2lt->setFont(QFont(theme::fontFamily(), theme::calcFontSize(-2)));
	ref2tt->setAlignment(ref2tt->layoutDirection() == Qt::LeftToRight ? Qt::AlignLeft : Qt::AlignRight);
	ref_fields[LIST_REF::Transponder] = ref2tt;
	ref_box->addWidget(ref2lt, 0, 4, Qt::AlignTop);
	ref_box->addWidget(ref2tt, 0, 5, Qt::AlignLeading | Qt::AlignTop);
	ref_box->addItem(new QSpacerItem(0, 12), 1, 0);

	QLabel* ref3lu = new QLabel(tr("Userbouquets", "reference-box"));
	QLabel* ref3tu = new QLabel("< >");
	ref3lu->setFont(QFont(theme::fontFamily(), theme::calcFontSize(-2)));
	ref3tu->setAlignment(ref3tu->layoutDirection() == Qt::LeftToRight ? Qt::AlignLeft : Qt::AlignRight);
	ref_fields[LIST_REF::Userbouquets] = ref3tu;
	ref_box->addWidget(ref3lu, 2, 0, Qt::AlignTop);
	ref_box->addWidget(ref3tu, 2, 1, Qt::AlignLeading | Qt::AlignTop);

	QLabel* ref4lb = new QLabel(tr("Bouquets", "reference-box"));
	QLabel* ref4tb = new QLabel("< >");
	ref4lb->setFont(QFont(theme::fontFamily(), theme::calcFontSize(-2)));
	ref4tb->setAlignment(ref4tb->layoutDirection() == Qt::LeftToRight ? Qt::AlignLeft : Qt::AlignRight);
	ref_fields[LIST_REF::Bouquets] = ref4tb;
	ref_box->addWidget(ref4lb, 2, 2, Qt::AlignTop);
	ref_box->addWidget(ref4tb, 2, 3, Qt::AlignLeading | Qt::AlignTop);

	QLabel* ref5ln = new QLabel(tr("Tuner", "reference-box"));
	QLabel* ref5tn = new QLabel("< >");
	ref5ln->setFont(QFont(theme::fontFamily(), theme::calcFontSize(-2)));
	ref5tn->setAlignment(ref5tn->layoutDirection() == Qt::LeftToRight ? Qt::AlignLeft : Qt::AlignRight);
	ref_fields[LIST_REF::Tuner] = ref5tn;
	ref_box->addWidget(ref5ln, 2, 4, Qt::AlignTop);
	ref_box->addWidget(ref5tn, 2, 5, Qt::AlignLeading | Qt::AlignTop);

	ref_box->addItem(new QSpacerItem(0, 0), 0, 6, 3, 1);

	ref_box->setRowStretch(2, 1);
	ref_box->setColumnStretch(6, 1);
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

	for (auto & x : dbih->index["bss"])
	{
		debug("load", "bouquet", x.second);

		e2db::bouquet gboq = dbih->bouquets[x.second];
		QString qbs = QString::fromStdString(x.second);
		QString name = gboq.nname.empty() ? e2db::fixUnicodeChars(gboq.name) : QString::fromStdString(gboq.nname);

		QTreeWidgetItem* bitem = new QTreeWidgetItem();
		bitem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
		bitem->setData(0, Qt::UserRole, qbs);
		bitem->setText(0, name);
		tree->addTopLevelItem(bitem);
		tree->expandItem(bitem);

		for (string & ubname : gboq.userbouquets)
			bgroups[ubname] = bitem;
	}
	for (auto & x : dbih->index["ubs"])
	{
		debug("load", "userbouquet", x.second);

		e2db::userbouquet uboq = dbih->userbouquets[x.second];
		QString qub = QString::fromStdString(x.second);
		QTreeWidgetItem* pgroup = bgroups[x.second];
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
	this->state.si = 0;
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

	for (auto & q : cache)
		q.second.clear();
	cache.clear();

	this->lsr_find.curr = -1;
	this->lsr_find.match.clear();

	this->action.list_addch->setDisabled(true);
	this->action.list_addmk->setDisabled(true);
	this->action.list_newch->setEnabled(true);
	this->action.list_dnd->setDisabled(true);

	updateReferenceBox();

	resetStatusBar();
}

void mainView::didChange()
{
	debug("didChange()");

	visualReloadList();
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
		prev = this->state.curr;
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
		error("populate", tr("Error", "error").toStdString(), tr("Missing index key \"%1\".", "error").arg(bname.data()).toStdString());

	list->header()->setSortIndicatorShown(true);
	list->header()->setSectionsClickable(false);
	list->clear();

	if (precached)
	{
		cache[prev].swap(items_prev);
	}

	if (cache[bname].isEmpty())
	{
		bool ub_locked = false;

		if (dbih->userbouquets.count(bname))
		{
			e2db::userbouquet uboq = dbih->userbouquets[bname];
			ub_locked = uboq.locked;
		}

		QString parentalicon = QSettings().value("preference/parentalLockInvert", false).toBool() || dbih->db.parental ? "service-whitelist" : "service-blacklist";

		int i = 0;
		size_t pad_width = std::to_string(int (dbih->index[bname].size())).size() + 1;

		for (auto & ch : dbih->index[bname])
		{
			bool marker = false;
			bool locked = false;
			QString x = QString::number(i++).rightJustified(pad_width, '0');
			QString idx;
			QString chid = QString::fromStdString(ch.second);
			QStringList entry;

			if (dbih->db.services.count(ch.second))
			{
				entry = dbih->entries.services[ch.second];
				//TODO TEST idx changed after edit
				idx = QString::number(ch.first);
				locked = entry[1].size() || ub_locked;
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
					entry = QStringList({x, NULL, NULL, NULL, chid, NULL, NULL, NULL, "ERROR", NULL});
					error("populate", tr("Error", "error").toStdString(), tr("Channel reference mismatch \"%1\".", "error").arg(ch.second.data()).toStdString());
				}
			}

			QTreeWidgetItem* item = new QTreeWidgetItem(entry);
			item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsDragEnabled | Qt::ItemNeverHasChildren);
			item->setData(ITEM_DATA_ROLE::idx, Qt::UserRole, idx);
			item->setData(ITEM_DATA_ROLE::marker, Qt::UserRole, marker);
			item->setData(ITEM_DATA_ROLE::chid, Qt::UserRole, chid);
			item->setData(ITEM_DATA_ROLE::locked, Qt::UserRole, locked);
			if (locked)
			{
				item->setIcon(ITEM_ROW_ROLE::chlock, theme::icon(parentalicon));
			}
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
		treeSortItems(list, this->state.sort.first, this->state.sort.second);
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
		int si = side->indexOfTopLevelItem(current);
		this->state.si = si;

		this->action.list_addch->setDisabled(true);
		this->action.list_addmk->setDisabled(true);
		this->action.list_newch->setEnabled(true);

		// tv | radio | data
		if (si)
		{
			disallowDnD();

			// sorting by
			if (this->state.sort.first > 0)
				this->action.list_dnd->setDisabled(true);
		}
		// all
		else
		{
			// sorting by
			if (this->state.sort.first > 0)
				this->action.list_dnd->setEnabled(true);
			// sorting default
			else
				allowDnD();
		}

		tabSetFlag(gui::TabListDelete, false);

		if (QGuiApplication::clipboard()->text().isEmpty())
			tabSetFlag(gui::TabListPaste, false);
		else
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
		}

		list->clearSelection();
		list->scrollToTop();
	}

	// services tree || userbouquet
	if (! this->state.tc || this->state.ti == -1)
	{
		if (QGuiApplication::clipboard()->text().isEmpty())
			tabSetFlag(gui::TabListPaste, false);
		else
			tabSetFlag(gui::TabListPaste, true);
	}
	else
	{
		tabSetFlag(gui::TabListPaste, false);
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

	// bouquet: tv | radio
	// ExpandsOnDoubleClick
	if (this->state.ti != -1)
		return;
	// userbouquet
	else
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
		tabSetFlag(gui::TabListDelete, false);
	}
	else
	{
		tabSetFlag(gui::TabListCopy, true);

		// services tree || userbouquet
		if (! this->state.tc || this->state.ti == -1)
		{
			tabSetFlag(gui::TabListCut, true);
			tabSetFlag(gui::TabListDelete, true);
		}
		else
		{
			tabSetFlag(gui::TabListCut, false);
			tabSetFlag(gui::TabListDelete, false);
		}
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

	// services tree || userbouquet
	if (! this->state.tc || this->state.ti == -1)
	{
		if (QGuiApplication::clipboard()->text().isEmpty())
			tabSetFlag(gui::TabListPaste, false);
		else
			tabSetFlag(gui::TabListPaste, true);
	}

	if (this->state.refbox)
		updateReferenceBox();

	tabUpdateFlags();
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
	size_t pad_width = std::to_string(j).size() + 1;

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
		item->setText(ITEM_ROW_ROLE::x, QString::number(idx++).rightJustified(pad_width, '0'));
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

	bool ub_locked = false;

	// userbouquet
	if (this->state.ti == -1)
	{
		string bname = this->state.curr;
		if (dbih->userbouquets.count(bname))
		{
			e2db::userbouquet uboq = dbih->userbouquets[bname];
			ub_locked = uboq.locked;
		}
	}

	QString parentalicon = QSettings().value("preference/parentalLockInvert", false).toBool() || dbih->db.parental ? "service-whitelist" : "service-blacklist";

	int i = 0;
	int j = list->topLevelItemCount();

	while (i != j)
	{
		QTreeWidgetItem* item = list->topLevelItem(i);
		string chid = item->data(ITEM_DATA_ROLE::chid, Qt::UserRole).toString().toStdString();
		bool marker = item->data(ITEM_DATA_ROLE::marker, Qt::UserRole).toBool();

		if (marker || ! dbih->entries.services.count(chid))
		{
			i++;
			continue;
		}

		QStringList entry = dbih->entries.services[chid];
		bool locked = entry[1].size() || ub_locked;
		entry.prepend(item->text(ITEM_ROW_ROLE::chnum));
		entry.prepend(item->text(ITEM_ROW_ROLE::x));
		for (int i = 0; i < entry.count(); i++)
			item->setText(i, entry[i]);
		item->setIcon(ITEM_ROW_ROLE::chlock, locked ? theme::icon(parentalicon) : QIcon());
		item->setFont(ITEM_ROW_ROLE::chcas, QFont(theme::fontFamily(), theme::calcFontSize(-1)));
		item->setIcon(ITEM_ROW_ROLE::chcas, ! item->text(ITEM_ROW_ROLE::chcas).isEmpty() ? theme::icon("crypted") : QIcon());
		i++;
	}
}

void mainView::clipboardDataChanged()
{
	// services tree || userbouquet
	if (! this->state.tc || this->state.ti == -1)
	{
		return this->viewAbstract::clipboardDataChanged();
	}

	debug("clipboardDataChanged");
}

void mainView::sortByColumn(int column)
{
	debug("sortByColumn", "column", column);

	Qt::SortOrder order = this->state.sort.first == -1 ? Qt::DescendingOrder : list->header()->sortIndicatorOrder();
	column = column == 1 ? 0 : column;

	// sorting by
	if (column)
	{
		disallowDnD();

		treeSortItems(list, column, order);

		// services: all | userbouquet
		if (this->state.si == 0 || this->state.ti == -1)
			this->action.list_dnd->setEnabled(true);
	}
	// sorting default
	else
	{
		allowDnD();

		treeSortItems(list, column, order);

		this->action.list_dnd->setDisabled(true);

		if (this->state.vlx_pending)
			this->visualReindexList();
	}

	this->state.sort = pair (column, order);
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
	this->state.sort = pair (0, Qt::AscendingOrder);
	this->action.list_dnd->setDisabled(true);
}

void mainView::addBouquet()
{
	debug("addBouquet");

	string bname;
	e2se_gui::editBouquet* add = new e2se_gui::editBouquet(this->data);
	add->display(cwid);
	bname = add->getAddId();
	add->destroy();

	auto* dbih = this->data->dbih;

	if (dbih->bouquets.count(bname))
		debug("addBouquet", "bname", bname);
	else
		return error("addBouquet", tr("Error", "error").toStdString(), tr("Missing bouquet key \"%1\".", "error").arg(bname.data()).toStdString());

	tree->setDragEnabled(false);
	tree->setAcceptDrops(false);

	int i = 0, y;
	QTreeWidgetItem* current = tree->currentItem();
	bool isTopLevel = current != nullptr && tree->indexOfTopLevelItem(current) != -1;
	QTreeWidgetItem* parent = current != nullptr && isTopLevel ? current->parent() : tree->invisibleRootItem();
	i = current != nullptr && isTopLevel ? parent->indexOfChild(current) : tree->topLevelItemCount() - 1;
	y = i + 1;

	e2db::bouquet gboq = dbih->bouquets[bname];

	QTreeWidgetItem* item = new QTreeWidgetItem;
	item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
	item->setData(0, Qt::UserRole, QString::fromStdString(gboq.bname));
	item->setText(0, gboq.nname.empty() ? e2db::fixUnicodeChars(gboq.name) : QString::fromStdString(gboq.nname));

	tree->insertTopLevelItem(y, item);

	tree->setDragEnabled(true);
	tree->setAcceptDrops(true);

	updateTreeIndex();

	this->data->setChanged(true);
}

void mainView::editBouquet()
{
	debug("editBouquet");

	QList<QTreeWidgetItem*> selected = tree->selectedItems();

	if (selected.empty() || selected.count() > 1)
		return;

	QTreeWidgetItem* item = selected.first();
	QString qbs = item->data(0, Qt::UserRole).toString();
	string bname = qbs.toStdString();
	string nw_bname;

	auto* dbih = this->data->dbih;

	if (dbih->bouquets.count(bname))
		debug("editBouquet", "bname", bname);
	else
		return error("editBouquet", tr("Error", "error").toStdString(), tr("Bouquet \"%1\" not exists.", "error").arg(bname.data()).toStdString());

	e2se_gui::editBouquet* edit = new e2se_gui::editBouquet(this->data);
	edit->setEditId(bname);
	edit->display(cwid);
	nw_bname = edit->getEditId();
	edit->destroy();

	if (dbih->bouquets.count(nw_bname))
		debug("ediBouquet", "new bname", nw_bname);
	else
		return error("editBouquet", tr("Error", "error").toStdString(), tr("Missing bouquet key \"%1\".", "error").arg(nw_bname.data()).toStdString());

	e2db::bouquet gboq = dbih->bouquets[nw_bname];
	item->setText(0, gboq.nname.empty() ? e2db::fixUnicodeChars(gboq.name) : QString::fromStdString(gboq.nname));

	updateTreeIndex();

	this->data->setChanged(true);
}

void mainView::addUserbouquet()
{
	debug("addUserbouquet");

	string bname;
	e2se_gui::editUserbouquet* add = new e2se_gui::editUserbouquet(this->data, this->state.ti);
	add->display(cwid);
	bname = add->getAddId();
	add->destroy();

	auto* dbih = this->data->dbih;

	if (dbih->userbouquets.count(bname))
		debug("addUserbouquet", "bname", bname);
	else
		return error("addUserbouquet", tr("Error", "error").toStdString(), tr("Missing userbouquet key \"%1\".", "error").arg(bname.data()).toStdString());

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
	string nw_bname;

	auto* dbih = this->data->dbih;

	if (dbih->userbouquets.count(bname))
		debug("editUserbouquet", "bname", bname);
	else
		return error("editUserbouquet", tr("Error", "error").toStdString(), tr("Userbouquet \"%1\" not exists.", "error").arg(bname.data()).toStdString());

	e2se_gui::editUserbouquet* edit = new e2se_gui::editUserbouquet(this->data, this->state.ti);
	edit->setEditId(bname);
	edit->display(cwid);
	nw_bname = edit->getEditId();
	edit->destroy();

	if (dbih->userbouquets.count(nw_bname))
		debug("editUserbouquet", "new bname", nw_bname);
	else
		return error("editUserbouquet", tr("Error", "error").toStdString(), tr("Missing userbouquet key \"%1\".", "error").arg(nw_bname.data()).toStdString());

	e2db::userbouquet uboq = dbih->userbouquets[nw_bname];
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

			e2db::bouquet gboq = dbih->bouquets[bname];
			stype = gboq.btype;
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
		return error("addService", tr("Error", "error").toStdString(), tr("Missing service key \"%1\".", "error").arg(chid.data()).toStdString());

	for (auto & q : cache)
		q.second.clear();
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

	bool ub_locked = false;

	// userbouquet
	if (this->state.ti == -1)
	{
		string bname = this->state.curr;
		if (dbih->userbouquets.count(bname))
		{
			e2db::userbouquet uboq = dbih->userbouquets[bname];
			ub_locked = uboq.locked;
		}
	}

	QString parentalicon = QSettings().value("preference/parentalLockInvert", false).toBool() || dbih->db.parental ? "service-whitelist" : "service-blacklist";

	size_t pad_width = std::to_string(int (dbih->index["chs"].size())).size() + 1;
	QString x = QString::number(i++).rightJustified(pad_width, '0');
	QString idx = QString::number(i);
	QStringList entry = dbih->entries.services[chid];
	bool locked = entry[1].size() || ub_locked;
	entry.prepend(idx);
	entry.prepend(x);

	QTreeWidgetItem* item = new QTreeWidgetItem(entry);
	item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsDragEnabled | Qt::ItemNeverHasChildren);
	item->setData(ITEM_DATA_ROLE::idx, Qt::UserRole, idx);
	item->setData(ITEM_DATA_ROLE::marker, Qt::UserRole, false);
	item->setData(ITEM_DATA_ROLE::chid, Qt::UserRole, QString::fromStdString(chid));
	item->setData(ITEM_DATA_ROLE::locked, Qt::UserRole, locked);
	item->setIcon(ITEM_ROW_ROLE::chlock, locked ? theme::icon(parentalicon) : QIcon());
	item->setFont(ITEM_ROW_ROLE::chcas, QFont(theme::fontFamily(), theme::calcFontSize(-1)));
	item->setIcon(ITEM_ROW_ROLE::chcas, ! item->text(ITEM_ROW_ROLE::chcas).isEmpty() ? theme::icon("crypted") : QIcon());

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

	if (! marker && dbih->db.services.count(chid))
		debug("editService", "chid", chid);
	else
		return error("editService", tr("Error", "error").toStdString(), tr("Service \"%1\" not exists or is a channel reference.", "error").arg(chid.data()).toStdString());

	e2se_gui::editService* edit = new e2se_gui::editService(this->data);
	edit->setEditId(chid);
	edit->display(cwid);
	nw_chid = edit->getEditId();
	reload = ! (edit->getTransponderId()).empty();
	edit->destroy();

	for (auto & q : cache)
		q.second.clear();
	cache.clear();

	if (reload)
		dbih->cache(true);

	if (dbih->db.services.count(nw_chid))
		debug("editService", "new chid", nw_chid);
	else
		return error("editService", tr("Error", "error").toStdString(), tr("Missing service key \"%1\".", "error").arg(nw_chid.data()).toStdString());

	bool ub_locked = false;

	// userbouquet
	if (this->state.ti == -1)
	{
		string bname = this->state.curr;
		if (dbih->userbouquets.count(bname))
		{
			e2db::userbouquet uboq = dbih->userbouquets[bname];
			ub_locked = uboq.locked;
		}
	}

	QString parentalicon = QSettings().value("preference/parentalLockInvert", false).toBool() || dbih->db.parental ? "service-whitelist" : "service-blacklist";

	QStringList entry = dbih->entries.services[nw_chid];
	bool locked = entry[1].size() || ub_locked;
	entry.prepend(item->text(ITEM_ROW_ROLE::chnum));
	entry.prepend(item->text(ITEM_ROW_ROLE::x));
	for (int i = 0; i < entry.count(); i++)
		item->setText(i, entry[i]);
	item->setData(ITEM_DATA_ROLE::marker, Qt::UserRole, false);
	item->setData(ITEM_DATA_ROLE::chid, Qt::UserRole, QString::fromStdString(nw_chid));
	item->setData(ITEM_DATA_ROLE::locked, Qt::UserRole, locked);
	item->setIcon(ITEM_ROW_ROLE::chlock, locked ? theme::icon(parentalicon) : QIcon());
	item->setFont(ITEM_ROW_ROLE::chcas, QFont(theme::fontFamily(), theme::calcFontSize(-1)));
	item->setIcon(ITEM_ROW_ROLE::chcas, ! item->text(ITEM_ROW_ROLE::chcas).isEmpty() ? theme::icon("crypted") : QIcon());

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

	if (! (dbih->userbouquets.count(bname) && dbih->userbouquets[bname].channels.count(chid)))
		return error("addMarker", tr("Error", "error").toStdString(), tr("Missing channel reference key \"%1\".", "error").arg(chid.data()).toStdString());

	e2db::channel_reference chref = dbih->userbouquets[bname].channels[chid];

	if (chref.marker)
		debug("addMarker", "chid", chid);
	else
		return error("addMarker", tr("Error", "error").toStdString(), tr("Channel reference \"%1\" is not a valid marker.", "error").arg(chid.data()).toStdString());

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
	size_t pad_width = std::to_string(int (dbih->index[bname].size())).size() + 1;
	QString x = QString::number(i++).rightJustified(pad_width, '0');
	QString idx = QString::number(i);
	QStringList entry = dbih->entryMarker(chref);
	entry.prepend(x);

	QTreeWidgetItem* item = new QTreeWidgetItem(entry);
	item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsDragEnabled | Qt::ItemNeverHasChildren);
	item->setData(ITEM_DATA_ROLE::idx, Qt::UserRole, idx);
	item->setData(ITEM_DATA_ROLE::marker, Qt::UserRole, marker);
	item->setData(ITEM_DATA_ROLE::chid, Qt::UserRole, QString::fromStdString(chid));
	item->setData(ITEM_DATA_ROLE::locked, Qt::UserRole, false);
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

	auto* dbih = this->data->dbih;

	if (! (dbih->userbouquets.count(bname) && dbih->userbouquets[bname].channels.count(chid)))
		return error("editMarker", tr("Error", "error").toStdString(), tr("Channel reference\"%1\" not exists.", "error").arg(chid.data()).toStdString());

	e2db::channel_reference chref = dbih->userbouquets[bname].channels[chid];

	if (marker && chref.marker)
		debug("editMarker", "chid", chid);
	else
		return error("editMarker", tr("Error", "error").toStdString(), tr("Channel reference \"%1\" is not a valid marker.", "error").arg(chid.data()).toStdString());

	e2se_gui::editMarker* edit = new e2se_gui::editMarker(this->data);
	edit->setEditId(chid, bname);
	edit->display(cwid);
	nw_chid = edit->getEditId();
	edit->destroy();

	if (! dbih->userbouquets[bname].channels.count(nw_chid))
		return error("editMarker", tr("Error", "error").toStdString(), tr("Missing channel reference key \"%1\".", "error").arg(nw_chid.data()).toStdString());

	if (chref.marker)
		debug("editMarker", "new chid", nw_chid);
	else
		return error("editMarker", tr("Error", "error").toStdString(), tr("Channel reference \"%1\" is not a valid marker.", "error").arg(nw_chid.data()).toStdString());

	chref = dbih->userbouquets[bname].channels[nw_chid];

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

	bool remove = tabRemoveQuestion(tr("Confirm deletetion", "message"), tr("Do you want to delete items?", "message"));
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

void mainView::setServiceParentalLock()
{
	debug("setServiceParentalLock");

	QList<QTreeWidgetItem*> selected = list->selectedItems();

	if (selected.empty() || selected.count() > 1)
		return;

	QTreeWidgetItem* item = selected.first();
	string chid = item->data(ITEM_DATA_ROLE::chid, Qt::UserRole).toString().toStdString();
	bool marker = item->data(ITEM_DATA_ROLE::marker, Qt::UserRole).toBool();

	auto* dbih = this->data->dbih;

	if (! marker && dbih->db.services.count(chid))
		debug("setServiceParentalLock", "chid", chid);
	else
		return error("setServiceParentalLock", tr("Error", "error").toStdString(), tr("Service \"%1\" not exists or is a channel reference.", "error").arg(chid.data()).toStdString());

	for (auto & q : cache)
		q.second.clear();
	cache.clear();

	dbih->setServiceParentalLock(chid);

	QString parentalicon = QSettings().value("preference/parentalLockInvert", false).toBool() || dbih->db.parental ? "service-whitelist" : "service-blacklist";

	QStringList entry = dbih->entries.services[chid];
	entry.prepend(item->text(ITEM_ROW_ROLE::chnum));
	entry.prepend(item->text(ITEM_ROW_ROLE::x));
	for (int i = 0; i < entry.count(); i++)
		item->setText(i, entry[i]);
	item->setIcon(ITEM_ROW_ROLE::chlock, theme::icon(parentalicon));
	item->setData(ITEM_DATA_ROLE::locked, Qt::UserRole, true);

	this->data->setChanged(true);
}

void mainView::unsetServiceParentalLock()
{
	debug("unsetServiceParentalLock");

	QList<QTreeWidgetItem*> selected = list->selectedItems();

	if (selected.empty() || selected.count() > 1)
		return;

	QTreeWidgetItem* item = selected.first();
	string chid = item->data(ITEM_DATA_ROLE::chid, Qt::UserRole).toString().toStdString();
	bool marker = item->data(ITEM_DATA_ROLE::marker, Qt::UserRole).toBool();

	auto* dbih = this->data->dbih;

	if (! marker && dbih->db.services.count(chid))
		debug("unsetServiceParentalLock", "chid", chid);
	else
		return error("unsetServiceParentalLock", tr("Error", "error").toStdString(), tr("Service \"%1\" not exists or is a channel reference.", "error").arg(chid.data()).toStdString());

	for (auto & q : cache)
		q.second.clear();
	cache.clear();

	dbih->unsetServiceParentalLock(chid);

	QStringList entry = dbih->entries.services[chid];
	entry.prepend(item->text(ITEM_ROW_ROLE::chnum));
	entry.prepend(item->text(ITEM_ROW_ROLE::x));
	for (int i = 0; i < entry.count(); i++)
		item->setText(i, entry[i]);
	item->setIcon(ITEM_ROW_ROLE::chlock, QIcon());
	item->setData(ITEM_DATA_ROLE::locked, Qt::UserRole, false);

	this->data->setChanged(true);
}

void mainView::toggleServiceParentalLock()
{
	debug("toggleServiceParentalLock");

	QList<QTreeWidgetItem*> selected = list->selectedItems();

	if (selected.empty() || selected.count() > 1)
		return;

	QTreeWidgetItem* item = selected.first();
	bool locked = item->data(ITEM_DATA_ROLE::locked, Qt::UserRole).toBool();

	if (locked)
		unsetServiceParentalLock();
	else
		setServiceParentalLock();
}

void mainView::setUserbouquetParentalLock()
{
	debug("setUserbouquetParentalLock");

	QList<QTreeWidgetItem*> selected = tree->selectedItems();

	if (selected.empty() || selected.count() > 1)
		return;

	QTreeWidgetItem* item = selected.first();
	QString qub = item->data(0, Qt::UserRole).toString();
	string bname = qub.toStdString();

	auto* dbih = this->data->dbih;

	if (dbih->userbouquets.count(bname))
		debug("setUserbouquetParentalLock", "bname", bname);
	else
		return error("setUserbouquetParentalLock", tr("Error", "error").toStdString(), tr("Userbouquet \"%1\" not exists.", "error").arg(bname.data()).toStdString());

	dbih->setUserbouquetParentalLock(bname);

	cache[bname].clear();

	visualReloadList();

	dbih->cache(true);

	this->data->setChanged(true);
}

void mainView::unsetUserbouquetParentalLock()
{
	debug("unsetUserbouquetParentalLock");

	QList<QTreeWidgetItem*> selected = tree->selectedItems();

	if (selected.empty() || selected.count() > 1)
		return;

	QTreeWidgetItem* item = selected.first();
	QString qub = item->data(0, Qt::UserRole).toString();
	string bname = qub.toStdString();

	auto* dbih = this->data->dbih;

	if (dbih->userbouquets.count(bname))
		debug("unsetUserbouquetParentalLock", "bname", bname);
	else
		return error("unsetUserbouquetParentalLock", tr("Error", "error").toStdString(), tr("Userbouquet \"%1\" not exists.", "error").arg(bname.data()).toStdString());

	dbih->unsetUserbouquetParentalLock(bname);

	cache[bname].clear();

	visualReloadList();

	dbih->cache(true);

	this->data->setChanged(true);
}

void mainView::toggleUserbouquetParentalLock()
{
	debug("toggleUserbouquetParentalLock");

	QList<QTreeWidgetItem*> selected = tree->selectedItems();

	if (selected.empty() || selected.count() > 1)
		return;

	QTreeWidgetItem* item = selected.first();
	QString qub = item->data(0, Qt::UserRole).toString();
	string bname = qub.toStdString();

	auto* dbih = this->data->dbih;

	if (dbih->userbouquets.count(bname))
		debug("toggleUserbouquetParentalLock", "bname", bname);
	else
		return error("toggleUserbouquetParentalLock", tr("Error", "error").toStdString(), tr("Userbouquet \"%1\" not exists.", "error").arg(bname.data()).toStdString());

	e2db::userbouquet uboq = dbih->userbouquets[bname];

	if (uboq.locked)
		dbih->unsetUserbouquetParentalLock(bname);
	else
		dbih->setUserbouquetParentalLock(bname);

	cache[bname].clear();

	visualReloadList();

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

void mainView::listItemCopy(bool cut)
{
	debug("listItemCopy");

	QList<QTreeWidgetItem*> selected = list->selectedItems();

	if (selected.empty())
		return;

	auto* dbih = this->data->dbih;

	QClipboard* clipboard = QGuiApplication::clipboard();
	QStringList content;

	for (auto & item : selected)
	{
		QString qchid = item->data(ITEM_DATA_ROLE::chid, Qt::UserRole).toString();
		bool marker = item->data(ITEM_DATA_ROLE::marker, Qt::UserRole).toBool();
		bool locked = item->data(ITEM_DATA_ROLE::locked, Qt::UserRole).toBool();
		string chid = qchid.toStdString();

		QStringList data;
		data.reserve(TSV_TABS + 1);

		// start from chnum column [1]
		for (int i = ITEM_ROW_ROLE::chnum; i < list->columnCount(); i++)
		{
			QString qstr = item->data(i, Qt::DisplayRole).toString();
			// parental lock
			if (i == ITEM_ROW_ROLE::chlock)
				qstr = locked ? "1" : "0";
			// debug_chid
			else if (i == ITEM_ROW_ROLE::debug_chid)
				continue;
			// debug_txid
			else if (i == ITEM_ROW_ROLE::debug_txid)
				continue;
			// chcas
			else if (i == ITEM_ROW_ROLE::chcas && ! marker)
				qstr.prepend(qstr.isEmpty() ? "" : "$");
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

		content.append(data.join("\t")); // TSV
	}
	clipboard->setText(content.join("\n"));

	if (cut)
		listItemDelete();
}

void mainView::listItemPaste()
{
	debug("listItemPaste", "entered", ! (this->state.tc && this->state.ti != -1));

	// bouquets tree && bouquet: tv | radio
	if (this->state.tc && this->state.ti != -1)
		return;

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
			else if (str.count('\t') == channelBookView::TSV_TABS) // TSV
				items.emplace_back(str);
			else
				return;
		}
	}

	if (! items.empty())
	{
		auto* dbih = this->data->dbih;

		putListItems(items);

		if (list->currentItem() == nullptr)
			list->scrollToBottom();

		// bouquets tree
		if (this->state.tc)
		{
			string bname = this->state.curr;
			if (dbih->userbouquets.count(bname))
			{
				e2db::userbouquet uboq = dbih->userbouquets[bname];
				string pname = uboq.pname;

				cache[pname].clear();
			}
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

	// bouquets tree && bouquet: tv | radio
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
		if (dbih->userbouquets.count(bname))
		{
			e2db::userbouquet uboq = dbih->userbouquets[bname];
			pname = uboq.pname;
		}
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

//TODO handle duplicates
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

	int ub_idx = -1;
	bool ub_locked = false;
	int anum_count = 0;
	string bname = this->state.curr;
	if (dbih->userbouquets.count(bname))
	{
		e2db::userbouquet uboq = dbih->userbouquets[bname];
		ub_idx = uboq.index;
		ub_locked = uboq.locked;
	}
	anum_count = int (dbih->index["mks"].size());

	QString parentalicon = QSettings().value("preference/parentalLockInvert", false).toBool() || dbih->db.parental ? "service-whitelist" : "service-blacklist";

	for (QString & q : items)
	{
		size_t pad_width = std::to_string(int (dbih->index[bname].size())).size() + 1;
		QString x = QString::number(i++).rightJustified(pad_width, '0');
		QString idx = QString::number(i);

		string refid;
		string value;
		bool locked = false;
		e2db::channel_reference chref;
		e2db::service_reference ref;

		QStringList qs;

		if (q.count('\t') == TSV_TABS || q.count('\t') == channelBookView::TSV_TABS) // TSV
		{
			qs = q.split('\t');
			refid = qs[2].toStdString();
			value = qs[1].toStdString();
		}
		else if (q.count(':') == 9) // refid
		{
			refid = q.toStdString();
		}
		else
		{
			error("putListItems", tr("Error", "error").toStdString(), tr("Not a valid data format.", "error").toStdString());

			break;
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
			locked = entry[1].size() || ub_locked;
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
			else if (qs.size() == TSV_TABS)
			{
				e2db::service ch;
				e2db::transponder tx;
				e2db::fec fec;
				locked = (qs[3] == "0" ? false : true) || ub_locked;

				ch.ssid = ref.ssid;
				ch.tsid = tx.tsid = ref.tsid;
				ch.dvbns = tx.dvbns = ref.dvbns;
				ch.onid = tx.onid = ref.onid;
				ch.chname = value;
				if (! qs[4].isEmpty())
					ch.ssid = ref.ssid = qs[4].toInt();
				if (! qs[5].isEmpty())
					ch.tsid = tx.tsid = ref.tsid = qs[5].toInt();
				ch.stype = dbih->value_service_type(qs[6].toStdString());
				//TODO
				// ch.data[e2db::SDATA::C]; qs[7]
				ch.data[e2db::SDATA::p] = dbih->value_channel_provider(qs[8].toStdString());
				ch.locked = locked;
				tx.tsid = ch.tsid;
				tx.dvbns = ch.dvbns;
				tx.sys = dbih->value_transponder_system(qs[9].toStdString());
				tx.ytype = dbih->value_transponder_type(qs[9].toStdString());
				tx.pos = dbih->value_transponder_position(qs[10].toStdString());
				tx.freq = qs[12].toInt();
				tx.pol = dbih->value_transponder_polarization(qs[13].toStdString());
				tx.sr = qs[14].toInt();
				dbih->value_transponder_fec(qs[15].toStdString(), tx.ytype, fec);
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
				error("putListItems", tr("Error", "error").toStdString(), tr("Channel reference mismatch.", "error").toStdString());

				continue;
			}
		}

		QTreeWidgetItem* item = new QTreeWidgetItem(entry);
		item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsDragEnabled | Qt::ItemNeverHasChildren);
		item->setData(ITEM_DATA_ROLE::idx, Qt::UserRole, idx);
		item->setData(ITEM_DATA_ROLE::marker, Qt::UserRole, chref.marker);
		item->setData(ITEM_DATA_ROLE::chid, Qt::UserRole, QString::fromStdString(chref.chid));
		item->setData(ITEM_DATA_ROLE::locked, Qt::UserRole, locked);
		if (locked)
		{
			item->setIcon(ITEM_ROW_ROLE::chlock, theme::icon(parentalicon));
		}
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

	if (! clist.empty())
	{
		if (current == nullptr)
			list->addTopLevelItems(clist);
		else
			list->insertTopLevelItems(y, clist);

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

	list->header()->setSectionsClickable(true);
	list->setDragEnabled(true);
	list->setAcceptDrops(true);
}

void mainView::showTreeEditContextMenu(QPoint& pos)
{
	debug("showTreeEditContextMenu");

	QList<QTreeWidgetItem*> selected = tree->selectedItems();

	if (selected.empty())
		return;

	QMenu* tree_edit = contextMenu();

	// bouquet: tv | radio
	if (this->state.ti != -1)
	{
		contextMenuAction(tree_edit, tr("Edit Bouquet", "context-menu"), [=]() { this->editBouquet(); }, tabGetFlag(gui::TabTreeEdit));
	}
	// userbouquet
	else
	{
		auto* dbih = this->data->dbih;

		bool ub_locked = false;
		string bname = this->state.curr;
		if (dbih->userbouquets.count(bname))
		{
			e2db::userbouquet uboq = dbih->userbouquets[bname];
			ub_locked = uboq.locked;
		}

		contextMenuAction(tree_edit, tr("Edit Userbouquet", "context-menu"), [=]() { this->editUserbouquet(); }, tabGetFlag(gui::TabTreeEdit));
		contextMenuSeparator(tree_edit);
		contextMenuAction(tree_edit, ! ub_locked ? tr("Set Parental lock", "context-menu") : tr("Unset Parental lock", "context-menu"), [=]() { this->toggleUserbouquetParentalLock(); });
		contextMenuSeparator(tree_edit);
		contextMenuAction(tree_edit, tr("&Delete", "context-menu"), [=]() { this->treeItemDelete(); }, tabGetFlag(gui::TabTreeDelete));
	}
	contextMenuSeparator(tree_edit);
	contextMenuAction(tree_edit, tr("Export", "context-menu"), [=]() { this->tabExportFile(); });

	platform::osContextMenuPopup(tree_edit, tree, pos);
}

void mainView::showListEditContextMenu(QPoint& pos)
{
	debug("showListEditContextMenu");

	QList<QTreeWidgetItem*> selected = list->selectedItems();

	if (selected.empty() && list->topLevelItemCount() != 0)
		return;

	bool marker = false;
	bool locked = false;
	bool ub_locked = false;
	bool editable = false;

	// userbouquet
	if (this->state.ti == -1)
	{
		auto* dbih = this->data->dbih;

		string bname = this->state.curr;
		if (dbih->userbouquets.count(bname))
		{
			e2db::userbouquet uboq = dbih->userbouquets[bname];
			ub_locked = uboq.locked;
		}
	}

	if (selected.count() == 1)
	{
		QTreeWidgetItem* item = selected.first();
		marker = item->data(ITEM_DATA_ROLE::marker, Qt::UserRole).toBool();
		locked = item->data(ITEM_DATA_ROLE::locked, Qt::UserRole).toBool();
		editable = true;
	}

	QMenu* list_edit = contextMenu();

	if (marker)
		contextMenuAction(list_edit, tr("Edit Marker", "context-menu"), [=]() { this->editMarker(); }, editable && tabGetFlag(gui::TabListEditMarker));
	else
		contextMenuAction(list_edit, tr("Edit Service", "context-menu"), [=]() { this->editService(); }, editable && tabGetFlag(gui::TabListEditService));
	contextMenuSeparator(list_edit);
	if (ub_locked)
		contextMenuAction(list_edit, tr("Parental lock (Userbouquet)", "context-menu"), [=]() {}, false);
	else if (selected.count() > 1)
		contextMenuAction(list_edit, tr("Parental lock", "context-menu"), [=]() {}, false);
	else
		contextMenuAction(list_edit, ! locked ? tr("Set Parental lock", "context-menu") : tr("Unset Parental lock", "context-menu"), [=]() { this->toggleServiceParentalLock(); }, editable && ! marker);
	contextMenuSeparator(list_edit);
	contextMenuAction(list_edit, tr("Cu&t", "context-menu"), [=]() { this->listItemCut(); }, tabGetFlag(gui::TabListCut), QKeySequence::Cut);
	contextMenuAction(list_edit, tr("&Copy", "context-menu"), [=]() { this->listItemCopy(); }, tabGetFlag(gui::TabListCopy), QKeySequence::Copy);
	contextMenuAction(list_edit, tr("&Paste", "context-menu"), [=]() { this->listItemPaste(); }, tabGetFlag(gui::TabListPaste), QKeySequence::Paste);
	contextMenuSeparator(list_edit);
	contextMenuAction(list_edit, tr("&Delete", "context-menu"), [=]() { this->listItemDelete(); }, tabGetFlag(gui::TabListDelete), QKeySequence::Delete);

	platform::osContextMenuPopup(list_edit, list, pos);
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
		{
			string filename;

			// bouquet: tv | radio
			if (this->state.ti != -1)
			{
				e2db::bouquet gboq = dbih->bouquets[bname];
				filename = gboq.rname.empty() ? gboq.bname : gboq.rname;
			}
			// userbouquet
			else
			{
				e2db::userbouquet uboq = dbih->userbouquets[bname];
				filename = uboq.rname.empty() ? uboq.bname : uboq.rname;
			}

			msg.curr = filename;
		}
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
			if (dbih->userbouquets.count(bname))
			{
				e2db::channel_reference chref = dbih->userbouquets[bname].channels[chid];
				string crefid = dbih->get_reference_id(chref);
				refid = QString::fromStdString(crefid);
			}
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

				// i18n rtl combo (LRM)
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

	int i = 0, y = 0, idx = 0;
	int count = tree->topLevelItemCount();
	vector<pair<int, string>> bss;
	vector<pair<int, string>> ubs;
	unordered_map<string, vector<string>> ubouquets;

	while (i != count)
	{
		QTreeWidgetItem* parent = tree->topLevelItem(i);
		QString qbs = parent->data(0, Qt::UserRole).toString();
		string pname = qbs.toStdString();
		idx = i + 1;
		bss.emplace_back(pair (idx, pname));
		y = 0;

		if (parent->childCount())
		{
			int childs = parent->childCount();
			while (y != childs)
			{
				QTreeWidgetItem* item = parent->child(y);
				QString qub = item->data(0, Qt::UserRole).toString();
				string bname = qub.toStdString();
				idx = y + 1;
				ubs.emplace_back(pair (idx, bname));
				ubouquets[pname].emplace_back(bname);
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
			x.second.userbouquets.swap(ubouquets[x.first]);
	}
}

//TODO improve give tree model persistentIndexList and remove sort
void mainView::updateListIndex()
{
	if (! this->state.chx_pending)
		return;

	string bname = this->state.curr;

	debug("updateListIndex", "current", bname);

	int i = 0, idx = 0;
	int count = list->topLevelItemCount();

	auto* dbih = this->data->dbih;

	dbih->index[bname].clear();

	int sort_column = list->sortColumn();
	Qt::SortOrder sort_order = list->header()->sortIndicatorOrder();
	bool sorted = sort_column != 0 && sort_order != Qt::AscendingOrder;
	sort_column = sort_column == 1 ? 0 : sort_column;

	if (sorted)
		list->sortItems(0, Qt::AscendingOrder);

	while (i != count)
	{
		QTreeWidgetItem* item = list->topLevelItem(i);
		string chid = item->data(mainView::ITEM_DATA_ROLE::chid, Qt::UserRole).toString().toStdString();
		bool marker = item->data(mainView::ITEM_DATA_ROLE::marker, Qt::UserRole).toBool();
		idx = marker ? 0 : i + 1;
		dbih->index[bname].emplace_back(pair (idx, chid));
		i++;
	}

	if (sorted)
		treeSortItems(list, sort_column, sort_order);

	this->state.chx_pending = false;
}

void mainView::updateListReferences(QTreeWidgetItem* current, QList<QTreeWidgetItem*> items)
{
	auto* dbih = this->data->dbih;

	QString qub = current->data(0, Qt::UserRole).toString();
	string bname = qub.toStdString();
	if (! dbih->userbouquets.count(bname))
		return;

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

			if (marker)
			{
				string value = item->text(mainView::ITEM_ROW_ROLE::chname).toStdString();
				int atype = 0;
				int anum = 0;
				int ub_idx = -1;

				std::sscanf(chid.c_str(), "%d:%x:%d", &atype, &anum, &ub_idx);

				chref.atype = atype;
				chref.anum = anum;
				chref.value = value;
			}
			else
			{
				chref.chid = chid;
				chref.index = idx;
			}

			dbih->addChannelReference(chref, bname);

			QString chid = QString::fromStdString(chref.chid);
			item->setData(mainView::ITEM_DATA_ROLE::chid, Qt::UserRole, chid);
			item->setText(mainView::ITEM_ROW_ROLE::debug_chid, chid);
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
