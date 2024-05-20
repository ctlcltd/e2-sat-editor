/*!
 * e2-sat-editor/src/gui/mainView.cpp
 *
 * @link https://github.com/ctlcltd/e2-sat-editor
 * @copyright e2 SAT Editor Team
 * @author Leonardo Laureti
 * @version 1.5.0
 * @license MIT License
 * @license GNU GPLv3 License
 */

#include <QtGlobal>
#include <QGuiApplication>
#include <QWindow>
#include <QSettings>
#include <QTimer>
#include <QList>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QSplitter>
#include <QGroupBox>
#include <QScrollArea>
#include <QMenu>
#include <QWidgetAction>
#include <QClipboard>
#include <QMimeData>
#include <QHeaderView>
#include <QMouseEvent>
#ifdef Q_OS_WIN
#include <QStyleFactory>
#include <QScrollBar>
#endif

#include "platforms/platform.h"

#include "toolkit/TreeProxyStyle.h"
#include "mainView.h"
#include "theme.h"
#include "tab.h"
#include "editBouquet.h"
#include "editUserbouquet.h"
#include "editService.h"
#include "editFavourite.h"
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

	if (this->dialchbook != nullptr)
	{
		this->dialchbook->destroy();
		delete this->dialchbook;
	}

	delete this->widget;
	delete this->theme;
	delete this->log;
}

void mainView::layout()
{
	debug("layout");

	widget->setStyleSheet("QGroupBox { spacing: 0; border: 0; padding: 0; padding-top: 32px; font-weight: bold } QGroupBox::title { margin: 8px 4px; padding: 0 1px 1px }");

	QGridLayout* frm = new QGridLayout(widget);

	QSplitter* swid = new QSplitter;
	QWidget* afrm = new QWidget;
	QWidget* bfrm = new QWidget;

	QGridLayout* swrap = new QGridLayout;
	QGridLayout* twrap = new QGridLayout;
	QGridLayout* lwrap = new QGridLayout;
	QVBoxLayout* asbox = new QVBoxLayout;
	QVBoxLayout* atbox = new QVBoxLayout;
	QGridLayout* bbox = new QGridLayout;
	QVBoxLayout* tbox = new QVBoxLayout;
	QVBoxLayout* lbox = new QVBoxLayout;

	QGroupBox* sfrm = new QGroupBox(tr("Services"));
	QGroupBox* tfrm = new QGroupBox(tr("Bouquets"));
	QGroupBox* lfrm = new QGroupBox(tr("Channels"));

	frm->setContentsMargins(0, 0, 0, 0);
	swrap->setContentsMargins(0, 0, 0, 0);
	twrap->setContentsMargins(0, 0, 0, 0);
	lwrap->setContentsMargins(0, 0, 0, 0);
	asbox->setContentsMargins(0, 0, 0, 0);
	atbox->setContentsMargins(0, 0, 0, 0);
	bbox->setContentsMargins(0, 0, 0, 0);
	tbox->setContentsMargins(0, 0, 0, 0);
	lbox->setContentsMargins(0, 0, 0, 0);

	frm->setSpacing(0);
	swrap->setSpacing(0);
	twrap->setSpacing(0);
	lwrap->setSpacing(0);
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
	TreeProxyStyle* tree_style = new TreeProxyStyle;
	TreeProxyStyle* list_style = new TreeProxyStyle;

#ifdef Q_OS_WIN
	if (! theme::isOverridden() && theme::isFluentWin())
	{
		QStyle* style = QStyleFactory::create("fusion");
		side_style->setBaseStyle(style);
		tree_style->setBaseStyle(style);
		list_style->setBaseStyle(style);
	}
#endif

	side->setStyle(side_style);
	tree->setStyle(tree_style);
	list->setStyle(list_style);

#ifdef Q_OS_WIN
	if (! theme::isOverridden() && (theme::isFluentWin() || theme::absLuma() || ! theme::isDefault()))
	{
		QStyle* style;

		if (theme::isFluentWin())
			style = QStyleFactory::create("windows11");
		else
			style = QStyleFactory::create("fusion");

		side->verticalScrollBar()->setStyle(style);
		side->horizontalScrollBar()->setStyle(style);
		tree->verticalScrollBar()->setStyle(style);
		tree->horizontalScrollBar()->setStyle(style);
		list->verticalScrollBar()->setStyle(style);
		list->horizontalScrollBar()->setStyle(style);
		list->header()->setStyle(style);
	}
#endif

	side->setStyleSheet("QTreeWidget { background: transparent; border-style: none } QTreeWidget::item { padding: 9px 0 }");
	tree->setStyleSheet("QTreeWidget { background: transparent; border-style: none } QTreeWidget::item { margin: 1px 0 0; padding: 8px 0 }");
	list->setStyleSheet("QTreeWidget { border-style: none } QTreeWidget::item { height: 32px }");

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

	side->setHeaderHidden(true);
	side->setUniformRowHeights(true);
	side->setItemsExpandable(false);
	side->setExpandsOnDoubleClick(false);
	side->setRootIsDecorated(false);
	side->setIndentation(0);
	side_style->setIndentation(10, true);

	tree->setHeaderHidden(true);
	tree->setUniformRowHeights(true);
	tree->setSelectionBehavior(QTreeWidget::SelectRows);
	tree->setDragDropMode(QTreeWidget::DragDrop);
	tree->setDefaultDropAction(Qt::MoveAction);
	tree->setDropIndicatorShown(true);
	tree->setEditTriggers(QTreeWidget::NoEditTriggers);
	tree->setRootIsDecorated(false);
	tree->setIndentation(0);
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
	list->setIndentation(0);
	list_style->setIndentation(12, true);
	list_style->setFirstColumnIndent(1);

	QTreeWidgetItem* lheader_item = new QTreeWidgetItem({NULL, tr("Index"), tr("Name"), tr("Parental"), "CHID", "TXID", tr("Service ID"), tr("Transport ID"), tr("Type"), tr("CAS"), tr("Provider"), tr("System"), tr("Position"), tr("Tuner"), tr("Frequency"), tr("Polarization"), tr("Symbol Rate"), tr("FEC")});

	list->setHeaderItem(lheader_item);
#if QT_VERSION >= QT_VERSION_CHECK(6, 6, 0)
	list->header()->setDefaultSectionSize(0);
#endif
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
	list->setColumnWidth(ITEM_ROW_ROLE::chsys, 85);		// System
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

	QWidget* scrn = new QWidget;
	scrn->setStyleSheet("position: absolute; top: 0");
	QHBoxLayout* scrn_box = new QHBoxLayout;
	scrn_box->setContentsMargins(0, 0, 0, 0);
	scrn_box->setSizeConstraint(QLayout::SetMinimumSize);

	QWidget* lcrn = new QWidget;
	lcrn->setStyleSheet("position: absolute; top: 0");
	QHBoxLayout* lcrn_box = new QHBoxLayout;
	lcrn_box->setContentsMargins(0, 0, 0, 0);
	lcrn_box->setSizeConstraint(QLayout::SetMinimumSize);

	this->action.scrn_sets = new QPushButton;
#ifdef Q_OS_MAC
	this->action.scrn_sets->setFlat(true);
#endif
	this->action.scrn_sets->setIcon(theme->dynamicIcon("services-sets-menu", this->action.scrn_sets));
	this->action.scrn_sets->setWhatsThis(tr("Settings Convert", "corner"));
	this->action.scrn_sets->setToolTip(tr("Settings Convert", "corner"));
	this->action.scrn_sets->connect(this->action.scrn_sets, &QPushButton::pressed, [=]() {
		QMenu* menu = this->servicesSetsCornerMenu();
		// menu->popup(this->action.scrn_sets->mapToGlobal(this->action.scrn_sets->pos()));
		platform::osMenuPopup(menu, this->action.scrn_sets, this->action.scrn_sets->pos());

		QMouseEvent mouseRelease(QEvent::MouseButtonRelease, this->action.scrn_sets->pos(), this->action.scrn_sets->mapToGlobal(QPoint(0, 0)), Qt::LeftButton, Qt::MouseButtons(Qt::LeftButton), {});
		QCoreApplication::sendEvent(this->action.scrn_sets, &mouseRelease);
	});

	this->action.lcrn_prefs = new QPushButton;
#ifdef Q_OS_MAC
	this->action.lcrn_prefs->setFlat(true);
#endif
	this->action.lcrn_prefs->setIcon(theme->dynamicIcon("list-prefs-menu", this->action.lcrn_prefs));
	//: Encoding: note double ampersand, not an accelerator
	this->action.lcrn_prefs->setWhatsThis(tr("Drag&&Drop Preferences", "corner"));
	this->action.lcrn_prefs->setToolTip(tr("Drag&&Drop Preferences", "corner").replace("&&", "&"));
	this->action.lcrn_prefs->connect(this->action.lcrn_prefs, &QPushButton::pressed, [=]() {
		QMenu* menu = this->listPrefsCornerMenu();
		// menu->popup(this->action.lcrn_prefs->mapToGlobal(this->action.lcrn_prefs->pos()));
		platform::osMenuPopup(menu, this->action.lcrn_prefs, this->action.lcrn_prefs->pos());

		QMouseEvent mouseRelease(QEvent::MouseButtonRelease, this->action.lcrn_prefs->pos(), this->action.lcrn_prefs->mapToGlobal(QPoint(0, 0)), Qt::LeftButton, Qt::MouseButtons(Qt::LeftButton), {});
		QCoreApplication::sendEvent(this->action.lcrn_prefs, &mouseRelease);
	});

	scrn_box->addWidget(this->action.scrn_sets, 0, Qt::AlignTrailing);
	scrn->setLayout(scrn_box);

	lcrn_box->addWidget(this->action.lcrn_prefs, 0, Qt::AlignTrailing);
	lcrn->setLayout(lcrn_box);

	QToolBar* tree_ats = toolBar();
	QToolBar* list_ats = toolBar();

	this->action.tree_newbq = toolBarAction(tree_ats, tr("Bouquet", "toolbar"), theme->dynamicIcon("add"), [=]() { this->addUserbouquet(); });
	toolBarSpacer(tree_ats);
	this->action.tree_search = toolBarButton(tree_ats, tr("Find…", "toolbar"), theme->dynamicIcon("search"), [=]() { this->treeSearchToggle(); });

	this->action.tree_newbq->setWhatsThis(tr("New Bouquet", "toolbar"));
	this->action.tree_newbq->setToolTip(tr("New Bouquet", "toolbar"));

	this->action.tree_search->setDisabled(true);

	this->action.list_addch = toolBarAction(list_ats, tr("Channel", "toolbar"), theme->dynamicIcon("add"), [=]() { this->addChannel(); });
	this->action.list_addfh = toolBarAction(list_ats, tr("Stream", "toolbar"), theme->dynamicIcon("add"), [=]() { this->addFavourite(); });
	this->action.list_addmk = toolBarAction(list_ats, tr("Marker", "toolbar"), theme->dynamicIcon("add"), [=]() { this->addMarker(); });
	this->action.list_newch = toolBarAction(list_ats, tr("Service", "toolbar"), theme->dynamicIcon("add"), [=]() { this->addService(); });

	this->action.list_ref = toolBarButton(list_ats, tr("Reference", "toolbar"), [=]() { this->listReferenceToggle(); });
	toolBarSeparator(list_ats);
	//: Encoding: note double ampersand, not an accelerator
	this->action.list_dnd = toolBarButton(list_ats, tr("Drag&&Drop", "toolbar"), [=]() { this->reharmDnD(); });
	toolBarSpacer(list_ats);
	this->action.list_search = toolBarButton(list_ats, tr("&Find…", "toolbar"), theme->dynamicIcon("search"), [=]() { this->listSearchToggle(); });

	this->action.list_addch->setWhatsThis(tr("Add Channel", "toolbar"));
	this->action.list_addch->setToolTip(tr("Add Channel", "toolbar"));
	this->action.list_addfh->setWhatsThis(tr("Add Stream", "toolbar"));
	this->action.list_addfh->setToolTip(tr("Add Stream", "toolbar"));
	this->action.list_addmk->setWhatsThis(tr("Add Marker", "toolbar"));
	this->action.list_addmk->setToolTip(tr("Add Marker", "toolbar"));
	this->action.list_newch->setWhatsThis(tr("New Service", "toolbar"));
	this->action.list_newch->setToolTip(tr("New Service", "toolbar"));

	this->action.list_dnd->setCheckable(true);
	this->action.list_ref->setCheckable(true);

	this->action.list_addfh->setDisabled(true);
	this->action.list_addmk->setDisabled(true);
	this->action.list_newch->setDisabled(true);
	this->action.list_dnd->setDisabled(true);
	this->action.list_dnd->actions().first()->setDisabled(true);
	this->action.list_search->setDisabled(true);
	this->action.list_search->actions().first()->setDisabled(true);

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
	tree->connect(tree, &QTreeWidget::itemSelectionChanged, [=]() { this->treeItemSelectionChanged(); });
	tree->connect(tree, &QTreeWidget::itemDoubleClicked, [=]() { this->treeItemDoubleClicked(); });
	list->installEventFilter(list_evto);
	list->viewport()->installEventFilter(list_evth);
	list->connect(list, &QTreeWidget::currentItemChanged, [=]() { this->listItemChanged(); });
	list->connect(list, &QTreeWidget::itemSelectionChanged, [=]() { this->listItemSelectionChanged(); });
	list->connect(list, &QTreeWidget::itemDoubleClicked, [=]() { this->listItemDoubleClicked(); });

	asbox->addWidget(side);
	sfrm->setLayout(asbox);
	sfrm->setMaximumHeight(192);
	swrap->addWidget(sfrm, 0, 0);
	swrap->addWidget(scrn, 0, 0, Qt::AlignTop | Qt::AlignTrailing);

	atbox->addWidget(tree);
	atbox->addWidget(tree_search);
	atbox->addWidget(tree_ats);
	tfrm->setLayout(atbox);
	twrap->addWidget(tfrm, 0, 0);

	tbox->addItem(swrap);
	tbox->addItem(new QSpacerItem(0, 4, QSizePolicy::Preferred, QSizePolicy::Fixed));
	tbox->addWidget(tfrm, 1);
	afrm->setLayout(tbox);

	lbox->addWidget(list);
	lbox->addWidget(list_search);
	lbox->addWidget(list_reference);
	lbox->addWidget(list_ats);
	lfrm->setLayout(lbox);
	lwrap->addWidget(lfrm, 0, 0);
	lwrap->addWidget(lcrn, 0, 0, Qt::AlignTop | Qt::AlignTrailing);

	afrm->setMinimumWidth(250);
	lfrm->setMinimumWidth(510);

	bbox->addItem(lwrap, 0, 0);
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

//TODO TEST viewAbstract::themeChange()
//TODO copyable viewport text
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

#ifdef Q_OS_WIN
	if (! theme::isOverridden() && (theme::isFluentWin() || theme::absLuma() || ! theme::isDefault()))
	{
		QStyle* style;

		if (theme::isFluentWin())
			style = QStyleFactory::create("windows11");
		else
			style = QStyleFactory::create("fusion");

		ref_area->verticalScrollBar()->setStyle(style);
		ref_area->horizontalScrollBar()->setStyle(style);
	}
#endif

	QGridLayout* ref_box = new QGridLayout;
	QLabel* ref0lr = new QLabel(tr("Reference ID", "reference-box"));
	QLabel* ref0tr = new QLabel("< >");
	ref0tr->setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::TextSelectableByKeyboard);
	platform::osLabel(ref0tr);
	ref0lr->setFont(QFont(theme::fontFamily(), theme::calcFontSize(-2)));
	ref0tr->setAlignment(ref0tr->layoutDirection() == Qt::LeftToRight ? Qt::AlignLeft : Qt::AlignRight);
	ref_fields[LIST_REF::ReferenceID] = ref0tr;
	ref_box->addWidget(ref0lr, 0, 0, Qt::AlignTop);
	ref_box->addWidget(ref0tr, 0, 1, Qt::AlignTop);

	QLabel* ref1ls = new QLabel(tr("Service ID", "reference-box"));
	QLabel* ref1ts = new QLabel("< >");
	ref1ts->setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::TextSelectableByKeyboard);
	platform::osLabel(ref1ts);
	ref1ls->setFont(QFont(theme::fontFamily(), theme::calcFontSize(-2)));
	ref1ts->setAlignment(ref1ts->layoutDirection() == Qt::LeftToRight ? Qt::AlignLeft : Qt::AlignRight);
	ref_fields[LIST_REF::ServiceID] = ref1ts;
	ref_box->addWidget(ref1ls, 0, 2, Qt::AlignTop);
	ref_box->addWidget(ref1ts, 0, 3, Qt::AlignLeading | Qt::AlignTop);

	QLabel* ref2lt = new QLabel(tr("Transponder", "reference-box"));
	QLabel* ref2tt = new QLabel("< >");
	ref2tt->setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::TextSelectableByKeyboard);
	platform::osLabel(ref2tt);
	ref2lt->setFont(QFont(theme::fontFamily(), theme::calcFontSize(-2)));
	ref2tt->setAlignment(ref2tt->layoutDirection() == Qt::LeftToRight ? Qt::AlignLeft : Qt::AlignRight);
	ref_fields[LIST_REF::Transponder] = ref2tt;
	ref_box->addWidget(ref2lt, 0, 4, Qt::AlignTop);
	ref_box->addWidget(ref2tt, 0, 5, Qt::AlignLeading | Qt::AlignTop);
	ref_box->addItem(new QSpacerItem(0, 12), 1, 0);

	QLabel* ref3lu = new QLabel(tr("Userbouquets", "reference-box"));
	QLabel* ref3tu = new QLabel("< >");
	ref3tu->setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::TextSelectableByKeyboard);
	platform::osLabel(ref3tu);
	ref3lu->setFont(QFont(theme::fontFamily(), theme::calcFontSize(-2)));
	ref3tu->setAlignment(ref3tu->layoutDirection() == Qt::LeftToRight ? Qt::AlignLeft : Qt::AlignRight);
	ref_fields[LIST_REF::Userbouquets] = ref3tu;
	ref_box->addWidget(ref3lu, 2, 0, Qt::AlignTop);
	ref_box->addWidget(ref3tu, 2, 1, Qt::AlignLeading | Qt::AlignTop);

	QLabel* ref4lb = new QLabel(tr("Bouquets", "reference-box"));
	QLabel* ref4tb = new QLabel("< >");
	ref4tb->setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::TextSelectableByKeyboard);
	platform::osLabel(ref4tb);
	ref4lb->setFont(QFont(theme::fontFamily(), theme::calcFontSize(-2)));
	ref4tb->setAlignment(ref4tb->layoutDirection() == Qt::LeftToRight ? Qt::AlignLeft : Qt::AlignRight);
	ref_fields[LIST_REF::Bouquets] = ref4tb;
	ref_box->addWidget(ref4lb, 2, 2, Qt::AlignTop);
	ref_box->addWidget(ref4tb, 2, 3, Qt::AlignLeading | Qt::AlignTop);

	QLabel* ref5ln = new QLabel(tr("Tuner", "reference-box"));
	QLabel* ref5tn = new QLabel("< >");
	ref5tn->setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::TextSelectableByKeyboard | Qt::LinksAccessibleByMouse | Qt::LinksAccessibleByKeyboard);
	ref5tn->setOpenExternalLinks(false);
	platform::osLabel(ref5tn);
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

	if (this->dialchbook != nullptr)
		this->dialchbook->load();
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
	this->state.chx_pending = false;
	this->state.tab_pending = false;
	this->state.q_parentalLockInvert = QSettings().value("engine/parentalLockInvert", false).toBool();
	this->state.q_markerGlobalIndex = QSettings().value("engine/markerGlobalIndex", false).toBool();
	this->state.q_favouriteMatchService = QSettings().value("engine/favouriteMatchService", true).toBool();

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

	listFindClear();

	this->action.list_addch->setDisabled(true);
	this->action.list_addfh->setDisabled(true);
	this->action.list_addmk->setDisabled(true);
	this->action.list_newch->setEnabled(true);
	this->action.list_dnd->setDisabled(true);
	this->action.list_dnd->actions().first()->setDisabled(true);

	updateReferenceBox();

	resetStatusBar();

	if (this->dialchbook != nullptr)
		this->dialchbook->reset();
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

		QString parentalicon = QSettings().value("engine/parentalLockInvert", false).toBool() || dbih->db.parental ? "service-whitelist" : "service-blacklist";

		int i = 0;
		size_t pad_width = std::to_string(int (dbih->index[bname].size())).size() + 1;

		for (auto & chi : dbih->index[bname])
		{
			int reftype = REF_TYPE::service;
			bool numbered = true;
			bool marker = false;
			bool locked = false;
			QString x = QString::number(i++).rightJustified(pad_width, '0');
			QString idx;
			QString chid = QString::fromStdString(chi.second);
			QString refid;
			QString uri;
			QStringList entry;
			bool ref_error = false;

			if (dbih->db.services.count(chi.second))
			{
				entry = dbih->entries.services[chi.second];
				idx = QString::number(chi.first);
				locked = entry[1].size() || ub_locked;
				entry.prepend(idx);
				entry.prepend(x);
			}
			else if (dbih->userbouquets.count(bname) && dbih->userbouquets[bname].channels.count(chi.second))
			{
				e2db::channel_reference chref = dbih->userbouquets[bname].channels[chi.second];

				if (chref.marker)
				{
					reftype = REF_TYPE::marker;
					marker = true;
					entry = dbih->entryMarker(chref);

					if (chref.atype == e2db::ATYPE::marker_numbered)
						entry[0] = QString::number(chi.first);
					else
						numbered = false;

					idx = entry[0];

					entry.prepend(x);
				}
				else if (chref.stream)
				{
					reftype = REF_TYPE::stream;
					entry = dbih->entryFavourite(chref);
					idx = QString::number(chi.first);
					locked = entry[1].size() || ub_locked;
					refid = QString::fromStdString(dbih->get_reference_id(chref));
					uri = entry[11];
					entry.prepend(idx);
					entry.prepend(x);
				}
				else
				{
					ref_error = true;
					reftype = REF_TYPE::favourite;
					idx = QString::number(chi.first);
					refid = QString::fromStdString(dbih->get_reference_id(chref));
					entry = dbih->entryFavourite(chref);
					locked = entry[1].size() || ub_locked;
					uri = entry[11];
					entry.prepend(idx);
					entry.prepend(x);
				}
			}
			else
			{
				ref_error = true;
				reftype = REF_TYPE::favourite;
				idx = QString::number(chi.first);
				entry = QStringList({x, idx, NULL, NULL, chid, NULL, NULL, NULL, "ERROR", NULL});
			}

			if (ref_error)
			{
				error("populate", tr("Error", "error").toStdString(), tr("Channel reference mismatch \"%1\".", "error").arg(chid).toStdString());
			}

			QTreeWidgetItem* item = new QTreeWidgetItem(entry);
			item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsDragEnabled | Qt::ItemNeverHasChildren);
			item->setData(ITEM_DATA_ROLE::idx, Qt::UserRole, idx);
			item->setData(ITEM_DATA_ROLE::numbered, Qt::UserRole, numbered);
			item->setData(ITEM_DATA_ROLE::chid, Qt::UserRole, chid);
			item->setData(ITEM_DATA_ROLE::locked, Qt::UserRole, locked);
			item->setData(ITEM_DATA_ROLE::reftype, Qt::UserRole, reftype);
			item->setData(ITEM_DATA_ROLE::refid, Qt::UserRole, refid);
			item->setData(ITEM_DATA_ROLE::uri, Qt::UserRole, uri);
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
		this->action.list_addfh->setDisabled(true);
		this->action.list_addmk->setDisabled(true);
		this->action.list_newch->setEnabled(true);

		// tv | radio | data
		if (si)
		{
			disallowDnD();

			// sorting by
			if (this->state.sort.first > 0)
			{
				this->action.list_dnd->setDisabled(true);
				this->action.list_dnd->actions().first()->setDisabled(true);
			}
		}
		// all
		else
		{
			// sorting by
			if (this->state.sort.first > 0)
			{
				this->action.list_dnd->setEnabled(true);
				this->action.list_dnd->actions().first()->setEnabled(true);
			}
			// sorting default
			else
			{
				allowDnD();
			}
		}

		tabSetFlag(gui::TabTreeEditBouquet, false);
		tabSetFlag(gui::TabListEditService, false);
		tabSetFlag(gui::TabListEditMarker, false);
		tabSetFlag(gui::TabListEditFavourite, false);

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

	this->lsr_find.timer.invalidate();

	updateFlags();
	updateStatusBar(true);
}

//TODO tree item position
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
			this->action.list_addfh->setDisabled(true);
			this->action.list_addmk->setDisabled(true);
			this->action.list_newch->setEnabled(true);

			disallowDnD();

			// sorting by
			if (this->state.sort.first > 0)
			{
				this->action.list_dnd->setDisabled(true);
				this->action.list_dnd->actions().first()->setDisabled(true);
			}
		}
		// userbouquet
		else
		{
			this->action.list_addch->setEnabled(true);
			this->action.list_addfh->setEnabled(true);
			this->action.list_addmk->setEnabled(true);
			this->action.list_newch->setDisabled(true);

			// sorting by
			if (this->state.sort.first > 0)
			{
				this->action.list_dnd->setEnabled(true);
				this->action.list_dnd->actions().first()->setEnabled(true);
			}
			// sorting default
			else
			{
				allowDnD();
			}
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

	listFindClear();

	updateFlags();
	updateStatusBar(true);
}

void mainView::treeItemSelectionChanged()
{
	// debug("treeItemSelectionChanged");

	QList<QTreeWidgetItem*> selected = tree->selectedItems();

	if (selected.count() == 1)
	{
		tabSetFlag(gui::TabTreeEditBouquet, true);
		tabSetFlag(gui::TabTreeDelete, true);
	}
	else
	{
		tabSetFlag(gui::TabTreeEditBouquet, false);
		tabSetFlag(gui::TabTreeDelete, false);
	}

	tabSetFlag(gui::TabListEditService, false);
	tabSetFlag(gui::TabListEditMarker, false);
	tabSetFlag(gui::TabListEditFavourite, false);

	tabUpdateFlags();
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
		bool favourite = item->data(ITEM_DATA_ROLE::reftype, Qt::UserRole).toInt() == REF_TYPE::favourite;
		bool marker = item->data(ITEM_DATA_ROLE::reftype, Qt::UserRole).toInt() == REF_TYPE::marker;
		bool stream = item->data(ITEM_DATA_ROLE::reftype, Qt::UserRole).toInt() == REF_TYPE::stream;

		if (marker)
		{
			tabSetFlag(gui::TabListEditService, false);
			tabSetFlag(gui::TabListEditMarker, true);
			tabSetFlag(gui::TabListEditFavourite, false);
		}
		else if (stream || favourite)
		{
			tabSetFlag(gui::TabListEditService, true);
			tabSetFlag(gui::TabListEditMarker, false);
			tabSetFlag(gui::TabListEditFavourite, true);
		}
		else
		{
			tabSetFlag(gui::TabListEditService, true);
			tabSetFlag(gui::TabListEditMarker, false);
			tabSetFlag(gui::TabListEditFavourite, false);
		}
	}
	else
	{
		tabSetFlag(gui::TabListEditService, false);
		tabSetFlag(gui::TabListEditMarker, false);
		tabSetFlag(gui::TabListEditFavourite, false);
	}

	tabSetFlag(gui::TabTreeEditBouquet, false);

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
	bool favourite = item->data(ITEM_DATA_ROLE::reftype, Qt::UserRole).toInt() == REF_TYPE::favourite;
	bool marker = item->data(ITEM_DATA_ROLE::reftype, Qt::UserRole).toInt() == REF_TYPE::marker;
	bool stream = item->data(ITEM_DATA_ROLE::reftype, Qt::UserRole).toInt() == REF_TYPE::stream;

	if (marker)
		editMarker();
	else if (stream || favourite)
		editFavourite();
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
			bool numbered = item->data(ITEM_DATA_ROLE::numbered, Qt::UserRole).toBool();
			if (numbered)
				y++;
			i++;
		}
		i = 0;
		j = list->topLevelItemCount();
	}
	while (reverse ? j-- : i != j)
	{
		QTreeWidgetItem* item = list->topLevelItem(i);
		bool numbered = item->data(ITEM_DATA_ROLE::numbered, Qt::UserRole).toBool();
		idx = reverse ? j : i;
		item->setText(ITEM_ROW_ROLE::x, QString::number(idx++).rightJustified(pad_width, '0'));
		if (numbered)
			item->setText(ITEM_ROW_ROLE::chnum, QString::number(idx - y));
		i++;
		y = ! numbered ? reverse ? y - 1 : y + 1 : y;
	}

	this->state.vlx_pending = false;
}

void mainView::visualReloadList()
{
	debug("visualReloadList");

	auto* dbih = this->data->dbih;

	bool chs_changed = dbih->changes.size();

	if (chs_changed)
		debug("visualReloadList", "chs changed", true);

	string bname = this->state.curr;
	bool ub_locked = false;

	// userbouquet
	if (this->state.tc && this->state.ti == -1)
	{
		if (dbih->userbouquets.count(bname))
		{
			e2db::userbouquet uboq = dbih->userbouquets[bname];
			ub_locked = uboq.locked;
		}
	}

	QString parentalicon = QSettings().value("engine/parentalLockInvert", false).toBool() || dbih->db.parental ? "service-whitelist" : "service-blacklist";

	int i = 0;
	int j = list->topLevelItemCount();

	while (i != j)
	{
		QTreeWidgetItem* item = list->topLevelItem(i);
		string chid = item->data(ITEM_DATA_ROLE::chid, Qt::UserRole).toString().toStdString();
		string nw_chid = chid;
		bool marker = item->data(ITEM_DATA_ROLE::reftype, Qt::UserRole).toInt() == REF_TYPE::marker;
		bool stream = item->data(ITEM_DATA_ROLE::reftype, Qt::UserRole).toInt() == REF_TYPE::stream;
		bool ref_error = false;

		if (chs_changed && dbih->changes.count(chid))
			nw_chid = dbih->changes[chid];

		if (dbih->entries.services.count(nw_chid))
		{
			QStringList entry = dbih->entries.services[nw_chid];
			bool locked = entry[1].size() || ub_locked;
			entry.prepend(item->text(ITEM_ROW_ROLE::chnum));
			entry.prepend(item->text(ITEM_ROW_ROLE::x));
			for (int i = 0; i < entry.count(); i++)
				item->setText(i, entry[i]);
			item->setIcon(ITEM_ROW_ROLE::chlock, locked ? theme::icon(parentalicon) : QIcon());
			item->setFont(ITEM_ROW_ROLE::chcas, QFont(theme::fontFamily(), theme::calcFontSize(-1)));
			item->setIcon(ITEM_ROW_ROLE::chcas, ! item->text(ITEM_ROW_ROLE::chcas).isEmpty() ? theme::icon("crypted") : QIcon());
			if (chs_changed)
				item->setData(ITEM_DATA_ROLE::chid, Qt::UserRole, QString::fromStdString(nw_chid));
		}
		// userbouquet
		else if (this->state.tc && this->state.ti == -1)
		{
			if (dbih->userbouquets.count(bname) && dbih->userbouquets[bname].channels.count(nw_chid))
			{
				e2db::channel_reference chref = dbih->userbouquets[bname].channels[nw_chid];

				if (marker)
				{
					QStringList entry = dbih->entryMarker(chref);
					entry.prepend(item->text(ITEM_ROW_ROLE::x));
					for (int i = 0; i < entry.count(); i++)
						item->setText(i, entry[i]);
					item->setData(ITEM_DATA_ROLE::chid, Qt::UserRole, QString::fromStdString(nw_chid));
				}
				else if (stream)
				{
					QStringList entry = dbih->entryFavourite(chref);
					bool locked = entry[1].size() || ub_locked;
					entry.prepend(item->text(ITEM_ROW_ROLE::chnum));
					entry.prepend(item->text(ITEM_ROW_ROLE::x));
					for (int i = 0; i < entry.count(); i++)
						item->setText(i, entry[i]);
					item->setData(ITEM_DATA_ROLE::chid, Qt::UserRole, QString::fromStdString(nw_chid));
					item->setData(ITEM_DATA_ROLE::locked, Qt::UserRole, locked);
					item->setIcon(ITEM_ROW_ROLE::chlock, locked ? theme::icon(parentalicon) : QIcon());
					item->setFont(ITEM_ROW_ROLE::chcas, QFont(theme::fontFamily(), theme::calcFontSize(-1)));
					item->setIcon(ITEM_ROW_ROLE::chcas, ! item->text(ITEM_ROW_ROLE::chcas).isEmpty() ? theme::icon("crypted") : QIcon());
				}
				else
				{
					ref_error = true;
					QStringList entry = dbih->entryFavourite(chref);
					bool locked = entry[1].size() || ub_locked;
					entry.prepend(item->text(ITEM_ROW_ROLE::chnum));
					entry.prepend(item->text(ITEM_ROW_ROLE::x));
					for (int i = 0; i < entry.count(); i++)
						item->setText(i, entry[i]);
					item->setData(ITEM_DATA_ROLE::chid, Qt::UserRole, QString::fromStdString(nw_chid));
					item->setData(ITEM_DATA_ROLE::locked, Qt::UserRole, locked);
					item->setIcon(ITEM_ROW_ROLE::chlock, locked ? theme::icon(parentalicon) : QIcon());
					item->setFont(ITEM_ROW_ROLE::chcas, QFont(theme::fontFamily(), theme::calcFontSize(-1)));
					item->setIcon(ITEM_ROW_ROLE::chcas, ! item->text(ITEM_ROW_ROLE::chcas).isEmpty() ? theme::icon("crypted") : QIcon());
				}
			}
			else
			{
				ref_error = true;
				QString chid = QString::fromStdString(nw_chid);
				QString x = item->text(ITEM_ROW_ROLE::x);
				QStringList entry = QStringList({x, NULL, NULL, NULL, chid, NULL, NULL, NULL, "ERROR", NULL});
				for (int i = 0; i < entry.count(); i++)
					item->setText(i, entry[i]);
				item->setData(ITEM_DATA_ROLE::reftype, Qt::UserRole, REF_TYPE::favourite);
			}
		}
		else
		{
			ref_error = true;
			QString chid = QString::fromStdString(nw_chid);
			QString x = item->text(ITEM_ROW_ROLE::x);
			QStringList entry = QStringList({x, NULL, NULL, NULL, chid, NULL, NULL, NULL, "ERROR", NULL});
			for (int i = 0; i < entry.count(); i++)
				item->setText(i, entry[i]);
			item->setData(ITEM_DATA_ROLE::reftype, Qt::UserRole, REF_TYPE::favourite);
		}

		if (ref_error)
		{
			QString chid = QString::fromStdString(nw_chid);

			error("visualReloadList", tr("Error", "error").toStdString(), tr("Channel reference mismatch \"%1\".", "error").arg(chid).toStdString());
		}

		i++;
	}

	if (chs_changed)
	{
		dbih->changes.clear();
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
		{
			this->action.list_dnd->setEnabled(true);
			this->action.list_dnd->actions().first()->setEnabled(true);
		}
	}
	// sorting default
	else
	{
		allowDnD();

		treeSortItems(list, column, order);

		this->action.list_dnd->setDisabled(true);
		this->action.list_dnd->actions().first()->setDisabled(true);

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
	this->action.list_dnd->actions().first()->setDisabled(true);
}

QMenu* mainView::servicesSetsCornerMenu()
{
	if (this->action.scrn_sets_menu == nullptr)
	{
		QMenu* menu = this->action.scrn_sets_menu = new QMenu;
		menu->setStyleSheet("font-size: small");

		QMenu* submenu = new QMenu(tr("Convert", "corner"));

		QList<QPair<int, QString>> opts = {
			{0x1224, tr("Lamedb 2.4 [Enigma 2]")},
			{0x1225, tr("Lamedb 2.5 [Enigma 2]")},
			{0x1223, tr("Lamedb 2.3 [Enigma 1]")},
			{0x1222, tr("Lamedb 2.2 [Enigma 1]")},
			{0x1014, tr("Zapit api-v4 [Neutrino]")},
			{0x1013, tr("Zapit api-v3 [Neutrino]")},
			{0x1012, tr("Zapit api-v2 [Neutrino]")},
			{0x1011, tr("Zapit api-v1 [Neutrino]")},
		};
		for (auto & opt : opts)
		{
			QAction* action = new QAction;
			action->setText(opt.second);
			action->setCheckable(true);
			action->connect(action, &QAction::triggered, [=]() { this->convert(opt.first); });
			submenu->addAction(action);
		}

		menu->addMenu(submenu);
	}

	auto* dbih = this->data->dbih;

	QMenu* menu = this->action.scrn_sets_menu;

#if QT_VERSION >= QT_VERSION_CHECK(6, 4, 0)
	auto actions = menu->menuInAction(menu->actions().first())->actions();
#else
	auto actions = menu->actions().first()->menu()->actions();
#endif

	int lamedb_ver = dbih->get_lamedb_version();
	int zapit_ver = dbih->get_zapit_version();

	for (auto & act : actions)
		act->setChecked(false);

	if (lamedb_ver == 4)
		actions.at(0)->setChecked(true);
	else if (lamedb_ver == 5)
		actions.at(1)->setChecked(true);
	else if (lamedb_ver == 3)
		actions.at(2)->setChecked(true);
	else if (lamedb_ver == 2)
		actions.at(3)->setChecked(true);

	if (zapit_ver == 4)
		actions.at(4)->setChecked(true);
	else if (zapit_ver == 3)
		actions.at(5)->setChecked(true);
	else if (zapit_ver == 2)
		actions.at(6)->setChecked(true);
	else if (zapit_ver == 1)
		actions.at(7)->setChecked(true);

	return menu;
}

QMenu* mainView::listPrefsCornerMenu()
{
	if (this->action.lcrn_prefs_menu == nullptr)
	{
		QMenu* menu = this->action.lcrn_prefs_menu = new QMenu;

		{
			QWidgetAction* action = new QWidgetAction(nullptr);
			QLabel* label = new QLabel(tr("Drag and Drop"));
#ifndef Q_OS_MAC
			label->setStyleSheet("QLabel { margin: 5px 10px }");
#else
			label->setStyleSheet("QLabel { margin: 5px 10px; font-weight: bold }");
#endif
			action->setDefaultWidget(label);
			menu->addAction(action);
		}
		{
			QAction* action = new QAction;
			action->setText(tr("Switch to bouquet after drop"));
			action->setCheckable(true);
			action->connect(action, &QAction::triggered, [=](bool checked)
			{
				QSettings settings;
				settings.setValue("preference/treeCurrentAfterDrop", checked);
				settings.sync();
				action->setChecked(checked);
			});
			menu->addAction(action);
		}
		menu->addSeparator();
		{
			QWidgetAction* action = new QWidgetAction(nullptr);
			QLabel* label = new QLabel(tr("Channel operations"));
#ifndef Q_OS_MAC
			label->setStyleSheet("QLabel { margin: 5px 10px }");
#else
			label->setStyleSheet("QLabel { margin: 5px 10px; font-weight: bold }");
#endif
			action->setDefaultWidget(label);
			menu->addAction(action);
		}
		{
			QAction* action = new QAction;
			action->setText(tr("Copy channels (preserving)"));
			action->setCheckable(true);
			action->connect(action, &QAction::triggered, [=](bool checked)
			{
				QSettings settings;
				settings.setValue("preference/treeDropCopy", checked);
				settings.setValue("preference/treeDropMove", ! checked);
				settings.sync();
				action->setChecked(checked);
			});
			menu->addAction(action);
		}
		{
			QAction* action = new QAction;
			action->setText(tr("Move channels (deleting)"));
			action->setCheckable(true);
			action->connect(action, &QAction::triggered, [=](bool checked)
			{
				QSettings settings;
				settings.setValue("preference/treeDropCopy", ! checked);
				settings.setValue("preference/treeDropMove", checked);
				settings.sync();
				action->setChecked(checked);
			});
			menu->addAction(action);
		}
	}

	QMenu* menu = this->action.lcrn_prefs_menu;

	auto actions = menu->actions();
	QSettings settings;

	for (auto & act : actions)
		act->setChecked(false);

	actions.at(1)->setChecked(settings.value("preference/treeCurrentAfterDrop", true).toBool());
	actions.at(4)->setChecked(settings.value("preference/treeDropCopy", true).toBool());
	actions.at(5)->setChecked(settings.value("preference/treeDropMove", false).toBool());

	return menu;
}

void mainView::convert(int bit)
{
	debug("convert", "bit", bit);

	auto* dbih = this->data->dbih;

	e2db::FPORTS fpx = static_cast<e2db::FPORTS>(bit);

	switch (fpx)
	{
		case e2db::FPORTS::all_services__2_4:
			dbih->set_e2db_services_type(0);
			dbih->set_lamedb_version(4);
			dbih->set_zapit_version(-1);
		break;
		case e2db::FPORTS::all_services__2_5:
			dbih->set_e2db_services_type(0);
			dbih->set_lamedb_version(5);
			dbih->set_zapit_version(-1);
		break;
		case e2db::FPORTS::all_services__2_3:
			dbih->set_e2db_services_type(0);
			dbih->set_lamedb_version(3);
			dbih->set_zapit_version(-1);
		break;
		case e2db::FPORTS::all_services__2_2:
			dbih->set_e2db_services_type(0);
			dbih->set_lamedb_version(2);
			dbih->set_zapit_version(-1);
		break;
		case e2db::FPORTS::all_services_xml__4:
			dbih->set_e2db_services_type(1);
			dbih->set_lamedb_version(-1);
			dbih->set_zapit_version(4);
		break;
		case e2db::FPORTS::all_services_xml__3:
			dbih->set_e2db_services_type(1);
			dbih->set_lamedb_version(-1);
			dbih->set_zapit_version(3);
		break;
		case e2db::FPORTS::all_services_xml__2:
			dbih->set_e2db_services_type(1);
			dbih->set_lamedb_version(-1);
			dbih->set_zapit_version(2);
		break;
		case e2db::FPORTS::all_services_xml__1:
			dbih->set_e2db_services_type(1);
			dbih->set_lamedb_version(-1);
			dbih->set_zapit_version(1);
		break;
		default:
			return;
	}

	updateStatusBar();
}

void mainView::addBouquet()
{
	debug("addBouquet");

	e2se_gui::editBouquet* dialog = new e2se_gui::editBouquet(this->data);
	dialog->display(cwid);
	string bname = dialog->getEditId();
	if (dialog->destroy()) return;

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

	tabPropagateChanges();
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

	auto* dbih = this->data->dbih;

	if (dbih->bouquets.count(bname))
		debug("editBouquet", "bname", bname);
	else
		return error("editBouquet", tr("Error", "error").toStdString(), tr("Bouquet \"%1\" not exists.", "error").arg(bname.data()).toStdString());

	e2se_gui::editBouquet* dialog = new e2se_gui::editBouquet(this->data);
	dialog->setEditId(bname);
	dialog->display(cwid);
	string nw_bname = dialog->getEditId();
	if (dialog->destroy()) return;

	if (dbih->bouquets.count(nw_bname))
		debug("ediBouquet", "new bname", nw_bname);
	else
		return error("editBouquet", tr("Error", "error").toStdString(), tr("Missing bouquet key \"%1\".", "error").arg(nw_bname.data()).toStdString());

	e2db::bouquet gboq = dbih->bouquets[nw_bname];
	item->setText(0, gboq.nname.empty() ? e2db::fixUnicodeChars(gboq.name) : QString::fromStdString(gboq.nname));

	treeItemSelectionChanged();

	updateTreeIndex();

	this->data->setChanged(true);

	tabPropagateChanges();
}

void mainView::addUserbouquet()
{
	debug("addUserbouquet");

	e2se_gui::editUserbouquet* dialog = new e2se_gui::editUserbouquet(this->data, this->state.ti);
	dialog->display(cwid);
	string bname = dialog->getEditId();
	if (dialog->destroy()) return;

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

	tabPropagateChanges();
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

	auto* dbih = this->data->dbih;

	if (dbih->userbouquets.count(bname))
		debug("editUserbouquet", "bname", bname);
	else
		return error("editUserbouquet", tr("Error", "error").toStdString(), tr("Userbouquet \"%1\" not exists.", "error").arg(bname.data()).toStdString());

	e2se_gui::editUserbouquet* dialog = new e2se_gui::editUserbouquet(this->data, this->state.ti);
	dialog->setEditId(bname);
	dialog->display(cwid);
	string nw_bname = dialog->getEditId();
	if (dialog->destroy()) return;

	if (dbih->userbouquets.count(nw_bname))
		debug("editUserbouquet", "new bname", nw_bname);
	else
		return error("editUserbouquet", tr("Error", "error").toStdString(), tr("Missing userbouquet key \"%1\".", "error").arg(nw_bname.data()).toStdString());

	e2db::userbouquet uboq = dbih->userbouquets[nw_bname];
	item->setText(0, e2db::fixUnicodeChars(uboq.name));

	treeItemSelectionChanged();

	updateTreeIndex();

	this->data->setChanged(true);

	tabPropagateChanges();
}

//TODO improve stype on curr change
//TODO improve gui <-> dialChannelBook <-> tab(s)
void mainView::addChannel()
{
	debug("addChannel");

	if (this->dialchbook != nullptr)
	{
		bool found = false;

		for (auto & q : QGuiApplication::allWindows())
		{
			if (q->isWindowType() && q->objectName() == "dialchbookWindow")
			{
				debug("addChannel", "raise", 1);

				q->requestActivate();
				q->raise();

				found = true;
				break;
			}
		}

		if (found)
		{
			return;
		}
		else
		{
			delete this->dialchbook;
			this->dialchbook = nullptr;
		}
	}

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

	this->dialchbook = new e2se_gui::dialChannelBook(this->data, stype);
	dialchbook->setEventCallback([=](vector<QString> items) { this->putListItems(items); });
	dialchbook->display(cwid);
}

void mainView::addService()
{
	debug("addService");

	bool is_fav = false;
	bool stream = false;
	bool favourite = false;
	string bname;
	int reftype = REF_TYPE::service;

	auto* dbih = this->data->dbih;

	// userbouquet
	if (this->state.tc && this->state.ti == -1)
	{
		bname = this->state.curr;

		if (dbih->userbouquets.count(bname))
			is_fav = true;
		else
			return;
	}

	e2se_gui::editService* dialog = new e2se_gui::editService(this->data);
	dialog->setAddId(bname);
	dialog->display(cwid);
	string chid = dialog->getEditId();
	bool reload = dialog->getReload();
	if (dialog->destroy()) return;

	e2db::channel_reference chref;

	if (dbih->db.services.count(chid))
	{
		debug("addService", "chid", chid);
	}
	else
	{
		if (is_fav && dbih->userbouquets[bname].channels.count(chid))
		{
			chref = dbih->userbouquets[bname].channels[chid];

			stream = chref.stream;
			favourite = ! stream;

			if (chref.stream)
				reftype = REF_TYPE::stream;
			else if (chref.marker)
				reftype = REF_TYPE::marker;
			else
				reftype = REF_TYPE::favourite;
		}
		else
		{
			return error("addService", tr("Error", "error").toStdString(), tr("Missing service key \"%1\".", "error").arg(chid.data()).toStdString());
		}
	}

	for (auto & q : cache)
		q.second.clear();
	cache.clear();

	if (reload)
		dbih->clearStorage();

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
	if (this->state.tc && this->state.ti == -1)
	{
		string bname = this->state.curr;
		if (dbih->userbouquets.count(bname))
		{
			e2db::userbouquet uboq = dbih->userbouquets[bname];
			ub_locked = uboq.locked;
		}
	}

	QString parentalicon = QSettings().value("engine/parentalLockInvert", false).toBool() || dbih->db.parental ? "service-whitelist" : "service-blacklist";

	size_t pad_width = std::to_string(int (dbih->index["chs"].size())).size() + 1;
	QString x = QString::number(i++).rightJustified(pad_width, '0');
	QString idx = QString::number(i);

	QTreeWidgetItem* item;

	if (favourite || stream)
	{
		QString refid = QString::fromStdString(dbih->get_reference_id(chref));
		QStringList entry = dbih->entryFavourite(chref);
		bool locked = entry[1].size() || ub_locked;
		QString uri = entry[11];
		entry.prepend(idx);
		entry.prepend(x);

		item = new QTreeWidgetItem(entry);
		item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsDragEnabled | Qt::ItemNeverHasChildren);
		item->setData(ITEM_DATA_ROLE::idx, Qt::UserRole, idx);
		item->setData(ITEM_DATA_ROLE::numbered, Qt::UserRole, true);
		item->setData(ITEM_DATA_ROLE::chid, Qt::UserRole, QString::fromStdString(chid));
		item->setData(ITEM_DATA_ROLE::locked, Qt::UserRole, locked);
		item->setData(ITEM_DATA_ROLE::reftype, Qt::UserRole, reftype);
		item->setData(ITEM_DATA_ROLE::refid, Qt::UserRole, refid);
		item->setData(ITEM_DATA_ROLE::uri, Qt::UserRole, uri);
		item->setIcon(ITEM_ROW_ROLE::chlock, locked ? theme::icon(parentalicon) : QIcon());
		item->setFont(ITEM_ROW_ROLE::chcas, QFont(theme::fontFamily(), theme::calcFontSize(-1)));
		item->setIcon(ITEM_ROW_ROLE::chcas, ! item->text(ITEM_ROW_ROLE::chcas).isEmpty() ? theme::icon("crypted") : QIcon());
	}
	else
	{
		QStringList entry = dbih->entries.services[chid];
		bool locked = entry[1].size() || ub_locked;
		entry.prepend(idx);
		entry.prepend(x);

		item = new QTreeWidgetItem(entry);
		item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsDragEnabled | Qt::ItemNeverHasChildren);
		item->setData(ITEM_DATA_ROLE::idx, Qt::UserRole, idx);
		item->setData(ITEM_DATA_ROLE::numbered, Qt::UserRole, true);
		item->setData(ITEM_DATA_ROLE::chid, Qt::UserRole, QString::fromStdString(chid));
		item->setData(ITEM_DATA_ROLE::locked, Qt::UserRole, locked);
		item->setData(ITEM_DATA_ROLE::reftype, Qt::UserRole, reftype);
		item->setIcon(ITEM_ROW_ROLE::chlock, locked ? theme::icon(parentalicon) : QIcon());
		item->setFont(ITEM_ROW_ROLE::chcas, QFont(theme::fontFamily(), theme::calcFontSize(-1)));
		item->setIcon(ITEM_ROW_ROLE::chcas, ! item->text(ITEM_ROW_ROLE::chcas).isEmpty() ? theme::icon("crypted") : QIcon());
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

	tabPropagateChanges();
}

void mainView::editService()
{
	debug("editService");

	QList<QTreeWidgetItem*> selected = list->selectedItems();

	if (selected.empty() || selected.count() > 1)
		return;

	QTreeWidgetItem* item = selected.first();
	string chid = item->data(ITEM_DATA_ROLE::chid, Qt::UserRole).toString().toStdString();

	bool is_fav = false;
	int reftype = REF_TYPE::service;
	bool favourite = item->data(ITEM_DATA_ROLE::reftype, Qt::UserRole).toInt() == REF_TYPE::favourite;
	bool marker = item->data(ITEM_DATA_ROLE::reftype, Qt::UserRole).toInt() == REF_TYPE::marker;
	bool stream = item->data(ITEM_DATA_ROLE::reftype, Qt::UserRole).toInt() == REF_TYPE::stream;
	string bname;

	auto* dbih = this->data->dbih;

	if (! marker && (stream || favourite || dbih->db.services.count(chid)))
		debug("editService", "chid", chid);
	else
		return error("editService", tr("Error", "error").toStdString(), tr("Service \"%1\" not exists or is a channel reference.", "error").arg(chid.data()).toStdString());

	// userbouquet
	if (this->state.tc && this->state.ti == -1)
	{
		bname = this->state.curr;

		if (dbih->userbouquets.count(bname) && dbih->userbouquets[bname].channels.count(chid))
			is_fav = true;
		else
			return error("editService", tr("Error", "error").toStdString(), tr("Missing channel reference key \"%1\".", "error").arg(chid.data()).toStdString());
	}

	e2se_gui::editService* dialog = new e2se_gui::editService(this->data);
	dialog->setEditId(chid, bname, stream);
	dialog->display(cwid);
	string nw_chid = dialog->getEditId();
	bool reload = dialog->getReload();
	if (dialog->destroy()) return;

	for (auto & q : cache)
		q.second.clear();
	cache.clear();

	if (reload)
		dbih->clearStorage();

	e2db::channel_reference chref;

	if (dbih->db.services.count(nw_chid))
	{
		debug("editService", "new chid", nw_chid);
	}
	else
	{
		if (is_fav && dbih->userbouquets[bname].channels.count(nw_chid))
		{
			chref = dbih->userbouquets[bname].channels[nw_chid];

			stream = chref.stream;
			favourite = ! stream;

			if (chref.stream)
				reftype = REF_TYPE::stream;
			else if (chref.marker)
				reftype = REF_TYPE::marker;
			else
				reftype = REF_TYPE::favourite;
		}
		else
		{
			return error("editService", tr("Error", "error").toStdString(), tr("Missing service key \"%1\".", "error").arg(nw_chid.data()).toStdString());
		}
	}

	bool ub_locked = false;

	// userbouquet
	if (this->state.tc && this->state.ti == -1)
	{
		string bname = this->state.curr;
		if (dbih->userbouquets.count(bname))
		{
			e2db::userbouquet uboq = dbih->userbouquets[bname];
			ub_locked = uboq.locked;
		}
	}

	QString parentalicon = QSettings().value("engine/parentalLockInvert", false).toBool() || dbih->db.parental ? "service-whitelist" : "service-blacklist";

	if (favourite || stream)
	{
		QString refid = QString::fromStdString(dbih->get_reference_id(chref));
		QStringList entry = dbih->entryFavourite(chref);
		bool locked = entry[1].size() || ub_locked;
		QString uri = entry[11];
		entry.prepend(item->text(ITEM_ROW_ROLE::chnum));
		entry.prepend(item->text(ITEM_ROW_ROLE::x));
		for (int i = 0; i < entry.count(); i++)
			item->setText(i, entry[i]);
		item->setData(ITEM_DATA_ROLE::numbered, Qt::UserRole, true);
		item->setData(ITEM_DATA_ROLE::chid, Qt::UserRole, QString::fromStdString(nw_chid));
		item->setData(ITEM_DATA_ROLE::locked, Qt::UserRole, locked);
		item->setData(ITEM_DATA_ROLE::reftype, Qt::UserRole, reftype);
		item->setData(ITEM_DATA_ROLE::refid, Qt::UserRole, refid);
		item->setData(ITEM_DATA_ROLE::uri, Qt::UserRole, uri);
		item->setIcon(ITEM_ROW_ROLE::chlock, locked ? theme::icon(parentalicon) : QIcon());
		item->setFont(ITEM_ROW_ROLE::chcas, QFont(theme::fontFamily(), theme::calcFontSize(-1)));
		item->setIcon(ITEM_ROW_ROLE::chcas, ! item->text(ITEM_ROW_ROLE::chcas).isEmpty() ? theme::icon("crypted") : QIcon());
	}
	else
	{
		QStringList entry = dbih->entries.services[nw_chid];
		bool locked = entry[1].size() || ub_locked;
		entry.prepend(item->text(ITEM_ROW_ROLE::chnum));
		entry.prepend(item->text(ITEM_ROW_ROLE::x));
		for (int i = 0; i < entry.count(); i++)
			item->setText(i, entry[i]);
		item->setData(ITEM_DATA_ROLE::numbered, Qt::UserRole, true);
		item->setData(ITEM_DATA_ROLE::chid, Qt::UserRole, QString::fromStdString(nw_chid));
		item->setData(ITEM_DATA_ROLE::locked, Qt::UserRole, locked);
		item->setData(ITEM_DATA_ROLE::reftype, Qt::UserRole, reftype);
		item->setIcon(ITEM_ROW_ROLE::chlock, locked ? theme::icon(parentalicon) : QIcon());
		item->setFont(ITEM_ROW_ROLE::chcas, QFont(theme::fontFamily(), theme::calcFontSize(-1)));
		item->setIcon(ITEM_ROW_ROLE::chcas, ! item->text(ITEM_ROW_ROLE::chcas).isEmpty() ? theme::icon("crypted") : QIcon());
	}

	if (reload)
		visualReloadList();

	listItemSelectionChanged();

	this->data->setChanged(true);

	tabPropagateChanges();
}

void mainView::addFavourite()
{
	debug("addFavourite");

	string bname = this->state.curr;
	int reftype = REF_TYPE::favourite;

	e2se_gui::editFavourite* dialog = new e2se_gui::editFavourite(this->data);
	dialog->setAddId(bname);
	dialog->display(cwid);
	string chid = dialog->getEditId();
	if (dialog->destroy()) return;

	auto* dbih = this->data->dbih;

	e2db::channel_reference chref = dbih->userbouquets[bname].channels[chid];

	cache[bname].clear();

	list->header()->setSectionsClickable(false);
	list->setDragEnabled(false);
	list->setAcceptDrops(false);

	int i = 0, y;
	QTreeWidgetItem* current = list->currentItem();
	QTreeWidgetItem* parent = list->invisibleRootItem();
	i = current != nullptr ? parent->indexOfChild(current) : list->topLevelItemCount();
	y = i + 1;

	bool numbered = true;
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

	if (chref.stream)
	{
		reftype = REF_TYPE::stream;
	}
	else if (chref.marker)
	{
		reftype = REF_TYPE::marker;
		numbered = chref.atype == e2db::ATYPE::marker_numbered;
	}

	QString parentalicon = QSettings().value("engine/parentalLockInvert", false).toBool() || dbih->db.parental ? "service-whitelist" : "service-blacklist";

	size_t pad_width = std::to_string(int (dbih->index["chs"].size())).size() + 1;
	QString x = QString::number(i++).rightJustified(pad_width, '0');
	QString idx = QString::number(i);
	QString refid = QString::fromStdString(dbih->get_reference_id(chref));
	QStringList entry = dbih->entryFavourite(chref);
	bool locked = entry[1].size() || ub_locked;
	QString uri = entry[11];
	entry.prepend(idx);
	entry.prepend(x);

	QTreeWidgetItem* item = new QTreeWidgetItem(entry);
	item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsDragEnabled | Qt::ItemNeverHasChildren);
	item->setData(ITEM_DATA_ROLE::idx, Qt::UserRole, idx);
	item->setData(ITEM_DATA_ROLE::numbered, Qt::UserRole, numbered);
	item->setData(ITEM_DATA_ROLE::chid, Qt::UserRole, QString::fromStdString(chid));
	item->setData(ITEM_DATA_ROLE::locked, Qt::UserRole, locked);
	item->setData(ITEM_DATA_ROLE::reftype, Qt::UserRole, reftype);
	item->setData(ITEM_DATA_ROLE::refid, Qt::UserRole, refid);
	item->setData(ITEM_DATA_ROLE::uri, Qt::UserRole, uri);
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

void mainView::editFavourite()
{
	debug("editFavourite");

	QList<QTreeWidgetItem*> selected = list->selectedItems();

	if (selected.empty() || selected.count() > 1)
		return;

	QTreeWidgetItem* item = selected.first();
	string chid = item->data(ITEM_DATA_ROLE::chid, Qt::UserRole).toString().toStdString();
	string bname = this->state.curr;
	bool numbered = item->data(ITEM_DATA_ROLE::numbered, Qt::UserRole).toBool();
	bool reindex = false;
	int reftype = REF_TYPE::favourite;

	auto* dbih = this->data->dbih;

	e2db::channel_reference chref;

	if (dbih->userbouquets.count(bname) && dbih->userbouquets[bname].channels.count(chid))
		chref = dbih->userbouquets[bname].channels[chid];
	else
		return error("editFavourite", tr("Error", "error").toStdString(), tr("Channel reference \"%1\" not exists.", "error").arg(chid.data()).toStdString());

	e2se_gui::editFavourite* dialog = new e2se_gui::editFavourite(this->data);
	dialog->setEditId(chid, bname);
	dialog->display(cwid);
	string nw_chid = dialog->getEditId();
	if (dialog->destroy()) return;

	if (! dbih->userbouquets[bname].channels.count(nw_chid))
		return error("editFavourite", tr("Error", "error").toStdString(), tr("Missing channel reference key \"%1\".", "error").arg(nw_chid.data()).toStdString());

	chref = dbih->userbouquets[bname].channels[nw_chid];
	reindex = (numbered != (chref.atype == e2db::ATYPE::marker_numbered));

	cache[bname].clear();

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

	if (chref.stream)
	{
		reftype = REF_TYPE::stream;
	}
	else if (chref.marker)
	{
		reftype = REF_TYPE::marker;
		numbered = chref.atype == e2db::ATYPE::marker_numbered;
	}

	QString parentalicon = QSettings().value("engine/parentalLockInvert", false).toBool() || dbih->db.parental ? "service-whitelist" : "service-blacklist";

	QString refid = QString::fromStdString(dbih->get_reference_id(chref));
	QStringList entry = dbih->entryFavourite(chref);
	bool locked = entry[1].size() || ub_locked;
	QString uri = entry[11];
	entry.prepend(item->text(ITEM_ROW_ROLE::chnum));
	entry.prepend(item->text(ITEM_ROW_ROLE::x));
	for (int i = 0; i < entry.count(); i++)
		item->setText(i, entry[i]);
	item->setData(ITEM_DATA_ROLE::numbered, Qt::UserRole, numbered);
	item->setData(ITEM_DATA_ROLE::chid, Qt::UserRole, QString::fromStdString(nw_chid));
	item->setData(ITEM_DATA_ROLE::locked, Qt::UserRole, locked);
	item->setData(ITEM_DATA_ROLE::reftype, Qt::UserRole, reftype);
	item->setData(ITEM_DATA_ROLE::refid, Qt::UserRole, refid);
	item->setData(ITEM_DATA_ROLE::uri, Qt::UserRole, uri);
	item->setIcon(ITEM_ROW_ROLE::chlock, locked ? theme::icon(parentalicon) : QIcon());
	item->setFont(ITEM_ROW_ROLE::chcas, QFont(theme::fontFamily(), theme::calcFontSize(-1)));
	item->setIcon(ITEM_ROW_ROLE::chcas, ! item->text(ITEM_ROW_ROLE::chcas).isEmpty() ? theme::icon("crypted") : QIcon());

	if (reindex)
	{
		// sorting default
		if (this->state.dnd)
			visualReindexList();
		else
			this->state.vlx_pending = true;

		setPendingUpdateListIndex();
	}

	listItemSelectionChanged();

	this->data->setChanged(true);
}

void mainView::addMarker()
{
	debug("addMarker");

	string bname = this->state.curr;

	e2se_gui::editMarker* dialog = new e2se_gui::editMarker(this->data);
	dialog->setAddId(bname);
	dialog->display(cwid);
	string chid = dialog->getEditId();
	if (dialog->destroy()) return;

	auto* dbih = this->data->dbih;

	e2db::channel_reference chref;

	if (dbih->userbouquets.count(bname) && dbih->userbouquets[bname].channels.count(chid))
		chref = dbih->userbouquets[bname].channels[chid];
	else
		return error("addMarker", tr("Error", "error").toStdString(), tr("Missing channel reference key \"%1\".", "error").arg(chid.data()).toStdString());

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

	bool numbered = chref.atype == e2db::ATYPE::marker_numbered;
	bool marker = true;
	size_t pad_width = std::to_string(int (dbih->index[bname].size())).size() + 1;
	QString x = QString::number(i++).rightJustified(pad_width, '0');
	QString idx = QString::number(i);
	QStringList entry = dbih->entryMarker(chref);
	entry.prepend(x);

	QTreeWidgetItem* item = new QTreeWidgetItem(entry);
	item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsDragEnabled | Qt::ItemNeverHasChildren);
	item->setData(ITEM_DATA_ROLE::idx, Qt::UserRole, idx);
	item->setData(ITEM_DATA_ROLE::numbered, Qt::UserRole, numbered);
	item->setData(ITEM_DATA_ROLE::chid, Qt::UserRole, QString::fromStdString(chid));
	item->setData(ITEM_DATA_ROLE::locked, Qt::UserRole, false);
	item->setData(ITEM_DATA_ROLE::reftype, Qt::UserRole, REF_TYPE::marker);
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
	string bname = this->state.curr;

	bool numbered = item->data(ITEM_DATA_ROLE::numbered, Qt::UserRole).toBool();
	bool marker = item->data(ITEM_DATA_ROLE::reftype, Qt::UserRole).toInt() == REF_TYPE::marker;
	bool reindex = false;

	auto* dbih = this->data->dbih;

	e2db::channel_reference chref;

	if (dbih->userbouquets.count(bname) && dbih->userbouquets[bname].channels.count(chid))
		chref = dbih->userbouquets[bname].channels[chid];
	else
		return error("editMarker", tr("Error", "error").toStdString(), tr("Channel reference \"%1\" not exists.", "error").arg(chid.data()).toStdString());

	if (marker && chref.marker)
		debug("editMarker", "chid", chid);
	else
		return error("editMarker", tr("Error", "error").toStdString(), tr("Channel reference \"%1\" is not a valid marker.", "error").arg(chid.data()).toStdString());

	e2se_gui::editMarker* dialog = new e2se_gui::editMarker(this->data);
	dialog->setEditId(chid, bname);
	dialog->display(cwid);
	string nw_chid = dialog->getEditId();
	if (dialog->destroy()) return;

	if (! dbih->userbouquets[bname].channels.count(nw_chid))
		return error("editMarker", tr("Error", "error").toStdString(), tr("Missing channel reference key \"%1\".", "error").arg(nw_chid.data()).toStdString());

	if (chref.marker)
		debug("editMarker", "new chid", nw_chid);
	else
		return error("editMarker", tr("Error", "error").toStdString(), tr("Channel reference \"%1\" is not a valid marker.", "error").arg(nw_chid.data()).toStdString());

	chref = dbih->userbouquets[bname].channels[nw_chid];
	reindex = (numbered != (chref.atype == e2db::ATYPE::marker_numbered));
	numbered = chref.atype == e2db::ATYPE::marker_numbered;

	cache[bname].clear();

	QStringList entry = dbih->entryMarker(chref);
	entry.prepend(item->text(ITEM_ROW_ROLE::x));
	for (int i = 0; i < entry.count(); i++)
		item->setText(i, entry[i]);
	item->setData(ITEM_DATA_ROLE::numbered, Qt::UserRole, numbered);
	item->setData(ITEM_DATA_ROLE::chid, Qt::UserRole, QString::fromStdString(nw_chid));

	if (reindex)
	{
		// sorting default
		if (this->state.dnd)
			visualReindexList();
		else
			this->state.vlx_pending = true;

		setPendingUpdateListIndex();
	}

	listItemSelectionChanged();

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

	bool removing = tabRemoveQuestion();
	if (! removing)
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

	tabPropagateChanges();
}

void mainView::setServiceParentalLock()
{
	debug("setServiceParentalLock");

	QList<QTreeWidgetItem*> selected = list->selectedItems();

	if (selected.empty() || selected.count() > 1)
		return;

	QTreeWidgetItem* item = selected.first();
	string chid = item->data(ITEM_DATA_ROLE::chid, Qt::UserRole).toString().toStdString();
	bool service = item->data(ITEM_DATA_ROLE::reftype, Qt::UserRole).toInt() == REF_TYPE::service;

	auto* dbih = this->data->dbih;

	if (service && dbih->db.services.count(chid))
		debug("setServiceParentalLock", "chid", chid);
	else
		return error("setServiceParentalLock", tr("Error", "error").toStdString(), tr("Service \"%1\" not exists or is a channel reference.", "error").arg(chid.data()).toStdString());

	for (auto & q : cache)
		q.second.clear();
	cache.clear();

	dbih->setServiceParentalLock(chid);

	QString parentalicon = QSettings().value("engine/parentalLockInvert", false).toBool() || dbih->db.parental ? "service-whitelist" : "service-blacklist";

	QStringList entry = dbih->entries.services[chid];
	entry.prepend(item->text(ITEM_ROW_ROLE::chnum));
	entry.prepend(item->text(ITEM_ROW_ROLE::x));
	for (int i = 0; i < entry.count(); i++)
		item->setText(i, entry[i]);
	item->setData(ITEM_DATA_ROLE::locked, Qt::UserRole, true);
	item->setIcon(ITEM_ROW_ROLE::chlock, theme::icon(parentalicon));

	listItemSelectionChanged();

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
	bool service = item->data(ITEM_DATA_ROLE::reftype, Qt::UserRole).toInt() == REF_TYPE::service;

	auto* dbih = this->data->dbih;

	if (service && dbih->db.services.count(chid))
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
	item->setData(ITEM_DATA_ROLE::locked, Qt::UserRole, false);
	item->setIcon(ITEM_ROW_ROLE::chlock, QIcon());

	listItemSelectionChanged();

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

	dbih->clearStorage();

	listItemSelectionChanged();

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

	dbih->clearStorage();

	listItemSelectionChanged();

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

	listItemSelectionChanged();

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

	string bname = this->state.curr;

	auto* dbih = this->data->dbih;

	QClipboard* clipboard = QGuiApplication::clipboard();
	QStringList content;

	for (auto & item : selected)
	{
		QString qchid = item->data(ITEM_DATA_ROLE::chid, Qt::UserRole).toString();
		bool stream = item->data(ITEM_DATA_ROLE::reftype, Qt::UserRole).toInt() == REF_TYPE::stream;
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
			else if (i == ITEM_ROW_ROLE::chcas)
			{
				if (! qstr.isEmpty())
				{
					qstr = "\t";

					string ref_chid = chid;

					// bouquets tree
					if (this->state.tc && stream && dbih->FAVOURITE_MATCH_SERVICE && dbih->userbouquets.count(bname) && dbih->userbouquets[bname].channels.count(chid))
					{
						e2db::channel_reference& chref = dbih->userbouquets[bname].channels[chid];

						int dvbns = chref.ref.dvbns;

						for (auto & x : dbih->db.transponders)
						{
							e2db::transponder& tx = x.second;

							//TODO TEST (dvbns or onid reverse to pos)
							if (tx.tsid == chref.ref.tsid && (tx.dvbns == chref.ref.dvbns || tx.onid == chref.ref.onid))
							{
								dvbns = tx.dvbns;
								break;
							}
						}

						if (dvbns == chref.ref.dvbns)
						{
							ref_chid = chref.chid;
						}
						else
						{
							char chid[25];

							// %4x:%4x:%8x
							std::snprintf(chid, 25, "%x:%x:%x", chref.ref.ssid, chref.ref.tsid, dvbns);

							ref_chid = chid;
						}
					}

					if (dbih->db.services.count(ref_chid))
					{
						e2db::service ch = dbih->db.services[ref_chid];

						vector<string> cas;
						for (string & w : ch.data[e2db::SDATA::C])
						{
							string caidpx = w.substr(0, 2);
							if (e2db::SDATA_CAS.count(caidpx))
								cas.emplace_back(e2db::SDATA_CAS.at(caidpx) + ':' + w);
							else
								cas.emplace_back(w);
						}

						QStringList scaid;

						for (size_t i = 0; i < cas.size(); i++)
						{
							scaid.append(cas[i].data());
						}

						qstr.prepend("$").append(scaid.join(", "));
					}
				}
				else
				{
					qstr = "\t";
				}
			}
			// uri
			else if (i == ITEM_ROW_ROLE::chtname && stream)
				qstr = item->data(ITEM_DATA_ROLE::uri, Qt::UserRole).toString();
			// fec
			else if (i == ITEM_ROW_ROLE::chfec)
				qstr.remove(" ").squeeze();

			data.append(qstr);
		}

		// Reference ID
		QString refid;
		// bouquets tree
		if (this->state.tc)
		{
			e2db::channel_reference chref;
			if (dbih->userbouquets.count(bname) && dbih->userbouquets[bname].channels.count(chid))
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
		listItemDelete(cut);
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

		//TODO tree item selection position

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

//TODO TEST random crash with put items
void mainView::listItemDelete(bool cut)
{
	debug("listItemDelete", "entered", ! (this->state.tc && this->state.ti != -1));

	// bouquets tree && bouquet: tv | radio
	if (this->state.tc && this->state.ti != -1)
		return;

	QList<QTreeWidgetItem*> selected = list->selectedItems();

	if (selected.empty())
		return;

	if (! cut)
	{
		bool removing = tabRemoveQuestion();
		if (! removing)
			return;
	}

	if (this->state.chx_pending)
	{
		updateListIndex();
	}

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
		int idx = item->data(ITEM_DATA_ROLE::idx, Qt::UserRole).toInt();
		string chid = item->data(ITEM_DATA_ROLE::chid, Qt::UserRole).toString().toStdString();
		list->takeTopLevelItem(i);

		bool found_duplicates = false;

		int count = 0;
		for (auto & x : dbih->index[bname])
		{
			if (x.second == chid)
			{
				count++;
			}
			if (count > 1)
			{
				found_duplicates = true;
				break;
			}
		}

		// bouquets tree
		if (this->state.tc)
		{
			if (! found_duplicates && dbih->userbouquets[bname].channels.count(chid))
			{
				e2db::channel_reference& chref = dbih->userbouquets[bname].channels[chid];

				if (chref.index == idx)
					dbih->removeChannelReference(chref, bname);
				else
					dbih->removeChannelReference(chid, bname);
			}

			cache[pname].clear();
		}
		// services tree
		else
		{
			if (! found_duplicates && dbih->db.services.count(chid))
				dbih->removeService(chid);

			for (auto & q : cache)
				q.second.clear();
			cache.clear();
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

	tabPropagateChanges();
}

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
	int marker_count = 0;
	int stream_count = 0;
	string bname = this->state.curr;
	if (dbih->userbouquets.count(bname))
	{
		e2db::userbouquet uboq = dbih->userbouquets[bname];
		ub_idx = uboq.index;
		ub_locked = uboq.locked;
	}
	marker_count = dbih->db.imarkers;
	stream_count = dbih->db.istreams;

	QString parentalicon = QSettings().value("engine/parentalLockInvert", false).toBool() || dbih->db.parental ? "service-whitelist" : "service-blacklist";

	for (QString & q : items)
	{
		size_t pad_width = std::to_string(int (dbih->index[bname].size())).size() + 1;
		QString x = QString::number(i++).rightJustified(pad_width, '0');
		QString idx = QString::number(i);
		QString ch_chid;
		QString ch_refid;
		QString ch_uri;

		bool numbered = true;
		int reftype = REF_TYPE::service;
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

			if (qs[6] == "MARKER")
				reftype = REF_TYPE::marker;
			else if (qs[6] == "STREAM" && ! qs[12].isEmpty())
				reftype = REF_TYPE::stream;
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

		if (reftype == REF_TYPE::marker)
		{
			chref.inum = marker_count + 1;

			// %4d:%4d:%8x:%d
			std::snprintf(chid, 25, "%d:%d:%x:%d", chref.etype, chref.atype, chref.inum, ub_idx);
		}
		else if (reftype == REF_TYPE::stream)
		{
			chref.inum = stream_count + 1;

			// %4d:%4d:%8x:%d
			std::snprintf(chid, 25, "%d:%d:%x:%d", chref.etype, chref.atype, chref.inum, ub_idx);
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
			chref.stream = false;
			chref.chid = chid;
			chref.atype = 0;
			chref.anum = 0;
			chref.index = idx.toInt();
		}
		else if (qs.size() == TSV_TABS + 1)
		{
			if (reftype == REF_TYPE::marker)
			{
				chref.marker = true;
				chref.chid = chid;
				chref.anum = marker_count + 1;
				chref.value = value;
				chref.ref = ref;

				reftype = REF_TYPE::marker;

				entry = dbih->entryMarker(chref);

				if (chref.atype == e2db::ATYPE::marker_numbered)
				{
					chref.index = idx.toInt();
					entry[0] = idx;
				}
				else
				{
					chref.index = -1;
					numbered = false;
					idx = "";
				}

				entry.prepend(x);

				marker_count++;
			}
			else if (reftype == REF_TYPE::stream)
			{
				chref.stream = true;
				chref.chid = chid;
				chref.value = value;
				chref.uri = qs[12].toStdString();
				chref.ref = ref;
				chref.index = idx.toInt();

				reftype = REF_TYPE::stream;
				locked = (qs[3] == "0" ? false : true) || ub_locked;
				ch_chid = QString::fromStdString(chid);
				ch_refid = QString::fromStdString(dbih->get_reference_id(chref));
				ch_uri = qs[12];

				entry = dbih->entryFavourite(chref);
				entry.prepend(idx);
				entry.prepend(x);

				stream_count++;
			}
			else
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
				if (! qs[7].isEmpty())
					ch.data[e2db::SDATA::C];
				ch.data[e2db::SDATA::C] = dbih->value_channel_caid(qs[8].toStdString(), ", ");
				ch.data[e2db::SDATA::p] = dbih->value_channel_provider(qs[9].toStdString());
				ch.locked = locked;
				tx.tsid = ch.tsid;
				tx.dvbns = ch.dvbns;
				tx.sys = dbih->value_transponder_system(qs[10].toStdString());
				tx.ytype = dbih->value_transponder_type(qs[10].toStdString());
				tx.pos = dbih->value_transponder_position(qs[11].toStdString());
				tx.freq = qs[13].toInt();
				tx.pol = dbih->value_transponder_polarization(qs[14].toStdString());
				tx.sr = qs[15].toInt();

				dbih->value_transponder_fec(qs[16].toStdString(), tx.ytype, fec);

				//TODO improve feparams reference
				if (tx.ytype == e2db::YTYPE::satellite)
				{
					tx.fec = fec.inner_fec;
					tx.mod = 0;
				}
				else if (tx.ytype == e2db::YTYPE::terrestrial)
				{
					tx.hpfec = fec.hp_fec;
					tx.lpfec = fec.lp_fec;
					tx.tmod = 3;
					tx.band = 3;
					tx.sr = -1;
				}
				else if (tx.ytype == e2db::YTYPE::cable)
				{
					tx.fec = fec.inner_fec;
					tx.cmod = 0;
				}
				else if (tx.ytype == e2db::YTYPE::atsc)
				{
					tx.amod = 0;
					tx.sr = -1;
				}

				if (ch.data.count(e2db::SDATA::C) && ch.data[e2db::SDATA::C].empty())
					ch.data.erase(e2db::SDATA::C);

				char txid[25];
				// %4x:%8x
				std::snprintf(txid, 25, "%x:%x", tx.tsid, tx.dvbns);
				tx.txid = ch.txid = txid;

				char chid[25];
				std::snprintf(chid, 25, "%x:%x:%x", ref.ssid, ref.tsid, ref.dvbns);
				ch.chid = chid;

				chref.chid = ch.chid;
				chref.ref = ref;
				chref.index = idx.toInt();

				if (! dbih->db.transponders.count(tx.txid))
					dbih->addTransponder(tx);
				if (! dbih->db.services.count(ch.chid))
					dbih->addService(ch);

				ch_chid = QString::fromStdString(chid);

				entry = dbih->entries.services[chid];
				entry.prepend(idx);
				entry.prepend(x);
			}
		}
		else
		{
			error("putListItems", tr("Error", "error").toStdString(), tr("Not a valid data format.", "error").toStdString());

			continue;
		}

		QTreeWidgetItem* item = new QTreeWidgetItem(entry);
		item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsDragEnabled | Qt::ItemNeverHasChildren);
		item->setData(ITEM_DATA_ROLE::idx, Qt::UserRole, idx);
		item->setData(ITEM_DATA_ROLE::numbered, Qt::UserRole, numbered);
		item->setData(ITEM_DATA_ROLE::locked, Qt::UserRole, locked);
		item->setData(ITEM_DATA_ROLE::reftype, Qt::UserRole, reftype);
		item->setData(ITEM_DATA_ROLE::refid, Qt::UserRole, ch_refid);
		item->setData(ITEM_DATA_ROLE::uri, Qt::UserRole, ch_uri);

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

		// bouquets tree
		if (this->state.tc)
		{
			string nw_chid = dbih->addChannelReference(chref, bname);

			ch_chid = QString::fromStdString(nw_chid);
		}

		item->setData(ITEM_DATA_ROLE::chid, Qt::UserRole, ch_chid);

		clist.append(item);
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

	tabPropagateChanges();
}

void mainView::showTreeEditContextMenu(QPoint& pos)
{
	// debug("showTreeEditContextMenu");

	QList<QTreeWidgetItem*> selected = tree->selectedItems();

	if (selected.empty())
		return;

	QMenu* tree_edit = contextMenu();

	// bouquet: tv | radio
	if (this->state.ti != -1)
	{
		contextMenuAction(tree_edit, tr("Edit Bouquet", "context-menu"), [=]() { this->editBouquet(); }, tabGetFlag(gui::TabTreeEditBouquet));
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

		contextMenuAction(tree_edit, tr("Edit Userbouquet", "context-menu"), [=]() { this->editUserbouquet(); }, tabGetFlag(gui::TabTreeEditBouquet));
		contextMenuSeparator(tree_edit);
		contextMenuAction(tree_edit, ! ub_locked ? tr("Set Parental lock", "context-menu") : tr("Unset Parental lock", "context-menu"), [=]() { this->toggleUserbouquetParentalLock(); });
		contextMenuSeparator(tree_edit);
		contextMenuAction(tree_edit, tr("&Delete", "context-menu"), [=]() { this->treeItemDelete(); }, tabGetFlag(gui::TabTreeDelete));
	}
	contextMenuSeparator(tree_edit);
	contextMenuAction(tree_edit, tr("Export", "context-menu"), [=]() { this->tabExportFile(); });

	platform::osMenuPopup(tree_edit, tree, pos);
}

void mainView::showListEditContextMenu(QPoint& pos)
{
	// debug("showListEditContextMenu");

	QList<QTreeWidgetItem*> selected = list->selectedItems();

	bool favourite = false;
	bool service = false;
	bool marker = false;
	bool stream = false;
	bool locked = false;
	bool ub_locked = false;
	bool editable = false;

	// userbouquet
	if (this->state.tc && this->state.ti == -1)
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
		favourite = item->data(ITEM_DATA_ROLE::reftype, Qt::UserRole).toInt() == REF_TYPE::favourite;
		service = item->data(ITEM_DATA_ROLE::reftype, Qt::UserRole).toInt() == REF_TYPE::service;
		marker = item->data(ITEM_DATA_ROLE::reftype, Qt::UserRole).toInt() == REF_TYPE::marker;
		stream = item->data(ITEM_DATA_ROLE::reftype, Qt::UserRole).toInt() == REF_TYPE::stream;
		locked = item->data(ITEM_DATA_ROLE::locked, Qt::UserRole).toBool();
		editable = true;
	}

	QMenu* list_edit = contextMenu();

	if (stream || favourite)
		contextMenuAction(list_edit, tr("Edit Favourite", "context-menu"), [=]() { this->editFavourite(); }, editable && tabGetFlag(gui::TabListEditFavourite));
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
		contextMenuAction(list_edit, ! locked ? tr("Set Parental lock", "context-menu") : tr("Unset Parental lock", "context-menu"), [=]() { this->toggleServiceParentalLock(); }, editable && service);
	contextMenuSeparator(list_edit);
	contextMenuAction(list_edit, tr("Cu&t", "context-menu"), [=]() { this->listItemCut(); }, tabGetFlag(gui::TabListCut), QKeySequence::Cut);
	contextMenuAction(list_edit, tr("&Copy", "context-menu"), [=]() { this->listItemCopy(); }, tabGetFlag(gui::TabListCopy), QKeySequence::Copy);
	contextMenuAction(list_edit, tr("&Paste", "context-menu"), [=]() { this->listItemPaste(); }, tabGetFlag(gui::TabListPaste), QKeySequence::Paste);
	contextMenuSeparator(list_edit);
	contextMenuAction(list_edit, tr("&Delete", "context-menu"), [=]() { this->listItemDelete(); }, tabGetFlag(gui::TabListDelete), QKeySequence::Delete);

	platform::osMenuPopup(list_edit, list, pos);
}

void mainView::actionCall(int bit)
{
	// debug("actionCall", "bit", bit);

	switch (bit)
	{
		case gui::TAB_ATS::TreeEditBouquet:
			// bouquet: tv | radio
			if (this->state.ti != -1)
			{
				editBouquet();
			}
			// userbouquet
			else
			{
				editUserbouquet();
			}
		break;
		case gui::TAB_ATS::ListEditService:
			editService();
		break;
		case gui::TAB_ATS::ListEditFavourite:
			editFavourite();
		break;
		case gui::TAB_ATS::ListEditMarker:
			editMarker();
		break;
	}
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

	tabPropagateChanges();
}

void mainView::treeDropFromList(QTreeWidgetItem* current)
{
	debug("treeDropFromList");

	// all | tv | radio && dropping on itself
	if (this->state.ti != -1 && tree->currentItem() == current)
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
		tree->setCurrentItem(nullptr); // should reset QTreeView::currentIndex
		tree->setCurrentItem(current);

		list->addTopLevelItems(items);
		list->scrollToBottom();
	}

	updateListReferences(current, items);

	tabPropagateChanges();
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
				if (dbih->bouquets.count(bname))
				{
					e2db::bouquet gboq = dbih->bouquets[bname];
					filename = gboq.rname.empty() ? gboq.bname : gboq.rname;
				}
			}
			// userbouquet
			else
			{
				if (dbih->userbouquets.count(bname))
				{
					e2db::userbouquet uboq = dbih->userbouquets[bname];
					filename = uboq.rname.empty() ? uboq.bname : uboq.rname;
				}
			}

			msg.curr = filename;
		}
	}
	else
	{
		int srcver = dbih->db.version;
		int dstver = 0;
		int lamedb_ver = dbih->get_lamedb_version();
		int zapit_ver = dbih->get_zapit_version();

		if (lamedb_ver != -1)
			dstver = 0x1220 + lamedb_ver;
		else if (zapit_ver != -1)
			dstver = 0x1010 + zapit_ver;

		msg.version = srcver;
		if (srcver != dstver)
			msg.convert = dstver;

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

		string bname = this->state.curr;
		e2db::channel_reference chref;
		if (dbih->userbouquets.count(bname))
		{
			chref = dbih->userbouquets[bname].channels[chid];
		}

		// debug("updateReferenceBox", "chid", chid);

		// bouquets tree
		if (this->state.tc)
		{
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
			string txid = ch.txid;

			ssid = QString::number(ch.ssid);

			if (dbih->db.transponders.count(txid))
			{
				e2db::transponder tx = dbih->db.transponders[txid];

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
			else
			{
				txp = tns = "< >";
			}
		}
		else if (chref.stream)
		{
			ssid = QString::number(chref.ref.ssid);

			QString psys;
			QString uri = QString::fromStdString(chref.uri);

			QUrl url = QUrl (uri);
			QString puri = QString(uri);

			int chunk = 76;
			int a = chunk + 1;
			for (qsizetype i = 0; i <= puri.size(); i++)
			{
				if (! --a)
				{
					puri.insert(i, "<br>");
					a = chunk + 1;
				}
			}
			puri = QString("<a href=\"%1\">%2</a>").arg(url.toString()).arg(puri);

			switch (chref.etype)
			{
				case e2db::ETYPE::ecast: psys = "[broadcast]"; break;
				case e2db::ETYPE::efile: psys = "[file]"; break;
				case e2db::ETYPE::evod: psys = "[vod]"; break;
				case e2db::ETYPE::eraw: psys = "[raw]"; break;
				case e2db::ETYPE::egstplayer: psys = "[gstplayer]"; break;
				case e2db::ETYPE::eexteplayer3: psys = "[exteplayer3]"; break;
				case e2db::ETYPE::eservice: psys = "[eservice]"; break;
				case e2db::ETYPE::eyoutube: psys = "[youtube]"; break;
				case e2db::ETYPE::eservice2: psys = "[eservice dreamos]"; break;
				default: psys = e2db::ETYPE_EXT_LABEL.count(chref.etype) ? QString::fromStdString(e2db::ETYPE_EXT_LABEL.at(chref.etype)) : QString::number(chref.etype);
			}

			string ref_txid;

			if (chref.ref.tsid != 0 || chref.ref.onid != 0 || chref.ref.dvbns != 0)
			{
				if (dbih->FAVOURITE_MATCH_SERVICE)
				{
					for (auto & x : dbih->db.transponders)
					{
						e2db::transponder& tx = x.second;

						//TODO TEST (dvbns or onid reverse to pos)
						if (tx.tsid == chref.ref.tsid && (tx.dvbns == chref.ref.dvbns || tx.onid == chref.ref.onid))
						{
							ref_txid = tx.txid;
							break;
						}
					}
				}
				else
				{
					char txid[25];

					// %4x:%8x
					std::snprintf(txid, 25, "%x:%x", chref.ref.tsid, chref.ref.dvbns);

					ref_txid = txid;
				}
			}

			if (dbih->db.transponders.count(ref_txid))
			{
				e2db::transponder tx = dbih->db.transponders[ref_txid];

				// i18n rtl combo (LRM)
				string ptxp = dbih->value_transponder_combo(tx);
				txp = QString::fromStdString(ptxp);
			}
			else
			{
				txp = "< >";
			}

			tns = "<p style=\"line-height: 125%\">" + psys + "</p><p>" + puri + "</p>";
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
		tabSetFlag(gui::TabTreeFind, true);
		this->action.tree_search->setEnabled(true);
		this->action.tree_search->actions().first()->setEnabled(true);
	}
	else
	{
		tabSetFlag(gui::TabTreeFind, false);
		this->action.tree_search->setDisabled(true);
		this->action.tree_search->actions().first()->setDisabled(true);
	}

	if (list->topLevelItemCount())
	{
		tabSetFlag(gui::TabListSelectAll, true);
		tabSetFlag(gui::TabListFind, true);
		this->action.list_search->setEnabled(true);
		this->action.list_search->actions().first()->setEnabled(true);
	}
	else
	{
		tabSetFlag(gui::TabListSelectAll, false);
		tabSetFlag(gui::TabListFind, false);
		this->action.list_search->setDisabled(true);
		this->action.list_search->actions().first()->setDisabled(true);
	}

	auto* dbih = this->data->dbih;

	if (dbih->index.count("chs"))
	{
		tabSetFlag(gui::Picons, true);
		tabSetFlag(gui::OpenChannelBook, true);
	}
	else
	{
		tabSetFlag(gui::Picons, false);
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

//TODO improve remove sort tree model persistentIndexList
void mainView::updateListIndex()
{
	if (! this->state.chx_pending)
		return;

	string bname = this->state.curr;

	debug("updateListIndex", "current", bname);

	int i = 0, y = 0, idx = 0;
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
		bool numbered = item->data(mainView::ITEM_DATA_ROLE::numbered, Qt::UserRole).toBool();
		string chid = item->data(mainView::ITEM_DATA_ROLE::chid, Qt::UserRole).toString().toStdString();
		if (numbered)
		{
			y++;
		}
		idx = numbered ? y : 0;
		dbih->index[bname].emplace_back(pair (idx, chid));
		item->setData(ITEM_DATA_ROLE::idx, Qt::UserRole, idx);
		i++;
	}

	if (dbih->userbouquets.count(bname))
	{
		idx = 0;
		for (auto & x : dbih->userbouquets[bname].channels)
		{
			e2db::channel_reference& chref = x.second;

			if (! (chref.marker && chref.atype != e2db::ATYPE::marker_numbered))
			{
				idx += 1;
				chref.index = idx;
			}
		}
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

	cache[bname].clear();

	for (auto & item : items)
	{
		string chid = item->data(ITEM_DATA_ROLE::chid, Qt::UserRole).toString().toStdString();

		if (! dbih->userbouquets[bname].channels.count(chid))
		{
			e2db::channel_reference chref;

			int idx = item->data(ITEM_DATA_ROLE::idx, Qt::UserRole).toInt();
			bool marker = item->data(ITEM_DATA_ROLE::reftype, Qt::UserRole).toInt() == REF_TYPE::marker;
			bool stream = item->data(ITEM_DATA_ROLE::reftype, Qt::UserRole).toInt() == REF_TYPE::stream;
			bool favourite = item->data(ITEM_DATA_ROLE::reftype, Qt::UserRole).toInt() == REF_TYPE::favourite;

			chref.marker = marker;
			chref.stream = stream;

			if (marker)
			{
				string value = item->text(ITEM_ROW_ROLE::chname).toStdString();
				int etype = 0;
				int atype = 0;
				int inum = dbih->db.imarkers + 1;
				int anum = 0;

				std::sscanf(chid.c_str(), "%d:%d:%x", &etype, &atype, &anum);

				chref.etype = etype;
				chref.atype = atype;
				chref.inum = inum;
				chref.anum = anum;
				chref.value = value;
				chref.index = idx;
			}
			else if (stream)
			{
				e2db::service_reference ref;

				string refid = item->data(ITEM_DATA_ROLE::refid, Qt::UserRole).toString().toStdString();
				string value = item->text(ITEM_ROW_ROLE::chname).toStdString();
				string uri = item->data(ITEM_DATA_ROLE::uri, Qt::UserRole).toString().toStdString();
				int etype = 0;
				int atype = 0;
				int inum = dbih->db.istreams + 1;
				int anum = 0;
				int ssid = 0;
				int tsid = 0;
				int onid = 0;
				int dvbns = 0;

				std::sscanf(refid.c_str(), "%d:%d:%X:%X:%X:%X:%X", &etype, &atype, &anum, &ssid, &tsid, &onid, &dvbns);

				ref.ssid = ssid;
				ref.tsid = tsid;
				ref.onid = onid;
				ref.dvbns = dvbns;

				chref.etype = etype;
				chref.atype = atype;
				chref.inum = inum;
				chref.anum = anum;
				chref.value = value;
				chref.uri = uri;
				chref.ref = ref;
				chref.index = idx;
			}
			else if (favourite)
			{
				e2db::service_reference ref;

				string refid = item->data(ITEM_DATA_ROLE::refid, Qt::UserRole).toString().toStdString();
				string value = item->text(ITEM_ROW_ROLE::chname).toStdString();
				string uri = item->data(ITEM_DATA_ROLE::uri, Qt::UserRole).toString().toStdString();
				int etype = 0;
				int atype = 0;
				int anum = 0;
				int ssid = 0;
				int tsid = 0;
				int onid = 0;
				int dvbns = 0;

				std::sscanf(refid.c_str(), "%d:%d:%X:%X:%X:%X:%X", &etype, &atype, &anum, &ssid, &tsid, &onid, &dvbns);

				ref.ssid = ssid;
				ref.tsid = tsid;
				ref.onid = onid;
				ref.dvbns = dvbns;

				chref.etype = etype;
				chref.atype = atype;
				chref.inum = -1;
				chref.anum = anum;
				chref.value = value;
				chref.uri = uri;
				chref.ref = ref;
				chref.index = idx;
			}
			else
			{
				if (dbih->userbouquets["chs"].channels.count(chid))
					chref = dbih->userbouquets["chs"].channels[chid];
				else
					chref.etype = 1;

				chref.chid = chid;
				chref.index = idx;
			}

			dbih->addChannelReference(chref, bname);

			QString chid = QString::fromStdString(chref.chid);
			item->setData(ITEM_DATA_ROLE::chid, Qt::UserRole, chid);
			item->setText(ITEM_ROW_ROLE::debug_chid, chid);
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

void mainView::didChange()
{
	debug("didChange");

	bool reload = false;

	if (this->state.q_parentalLockInvert != QSettings().value("engine/parentalLockInvert", false).toBool())
	{
		this->state.q_parentalLockInvert = QSettings().value("engine/parentalLockInvert", false).toBool();
		reload = true;
	}
	if (this->state.q_markerGlobalIndex != QSettings().value("engine/markerGlobalIndex", false).toBool())
	{
		this->state.q_markerGlobalIndex = QSettings().value("engine/markerGlobalIndex", false).toBool();
		reload = true;
	}
	if (this->state.q_favouriteMatchService != QSettings().value("engine/favouriteMatchService", true).toBool())
	{
		this->state.q_favouriteMatchService = QSettings().value("engine/favouriteMatchService", true).toBool();
		reload = true;
	}

	if (reload)
	{
		visualReloadList();

		if (this->state.refbox)
			updateReferenceBox();
	}
}

void mainView::update()
{
	debug("update");

	if (this->state.tab_pending)
	{
		auto* dbih = this->data->dbih;

		for (auto & q : cache)
			q.second.clear();
		cache.clear();

		dbih->clearStorage();

		visualReloadList();

		listFindClear();

		if (this->state.refbox)
			updateReferenceBox();

		this->state.tab_pending = false;
	}
}

void mainView::updateIndex()
{
	updateTreeIndex();
	this->state.chx_pending = true;
	updateListIndex();
	this->state.chx_pending = false;
}

void mainView::updateFromTab()
{
	debug("updateFromTab");

	this->state.tab_pending = true;
}

}
