/*!
 * e2-sat-editor/src/gui/tab.cpp
 *
 * @link https://github.com/ctlcltd/e2-sat-editor
 * @copyright e2 SAT Editor Team
 * @author Leonardo Laureti
 * @version 0.1
 * @license MIT License
 * @license GNU GPLv3 License
 */

#include <algorithm>
#include <filesystem>
#include <cstdio>

#include <QtGlobal>
#include <QGuiApplication>
#include <QTimer>
#include <QList>
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
#include "tab.h"
#include "theme.h"
#include "gui.h"
#include "editBouquet.h"
#include "editService.h"
#include "channelBook.h"
#include "ftpcom_gui.h"

using std::to_string, std::sort;
using namespace e2se;

namespace e2se_gui
{

tab::tab(gui* gid, QWidget* wid, e2se::logger::session* log)
{
	this->log = new logger(log, "tab");
	debug("tab()");

	this->gid = gid;
	this->cwid = wid;
	this->widget = new QWidget;
	widget->setStyleSheet("QGroupBox { spacing: 0; padding: 20px 0 0 0; border: 0 } QGroupBox::title { margin: 0 12px }");

	QGridLayout* frm = new QGridLayout(widget);

	QHBoxLayout* top = new QHBoxLayout;
	QGridLayout* container = new QGridLayout;
	QHBoxLayout* bottom = new QHBoxLayout;

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
	this->bouquets_tree = new QTreeWidget;
	this->list_tree = new QTreeWidget;
	services_tree->setStyleSheet("QTreeWidget { background: transparent } ::item { padding: 9px auto }");
	bouquets_tree->setStyleSheet("QTreeWidget { background: transparent } ::item { margin: 1px 0 0; padding: 8px auto }");
	list_tree->setStyleSheet("::item { padding: 6px auto }");

	services_tree->setHeaderHidden(true);
	services_tree->setUniformRowHeights(true);
	bouquets_tree->setHeaderHidden(true);
	bouquets_tree->setUniformRowHeights(true);
	list_tree->setUniformRowHeights(true);
	
	services_tree->setRootIsDecorated(false);
	services_tree->setItemsExpandable(false);
	services_tree->setExpandsOnDoubleClick(false);

	bouquets_tree->setSelectionBehavior(QAbstractItemView::SelectRows);
	bouquets_tree->setDropIndicatorShown(true);
	bouquets_tree->setDragDropMode(QAbstractItemView::DragDrop);
	bouquets_tree->setEditTriggers(QAbstractItemView::NoEditTriggers);

	list_tree->setRootIsDecorated(false);
	list_tree->setSelectionBehavior(QAbstractItemView::SelectRows);
	list_tree->setSelectionMode(QAbstractItemView::ExtendedSelection);
	list_tree->setItemsExpandable(false);
	list_tree->setExpandsOnDoubleClick(false);
	list_tree->setDropIndicatorShown(true);
	list_tree->setDragDropMode(QAbstractItemView::InternalMove);
	list_tree->setEditTriggers(QAbstractItemView::NoEditTriggers);
	
	TreeStyledItemDelegate* bouquets_delegate = new TreeStyledItemDelegate(bouquets_tree);
	bouquets_delegate->setIndentation(bouquets_tree->indentation());
	bouquets_tree->setItemDelegateForColumn(0, bouquets_delegate);

	QTreeWidgetItem* lheader_item = new QTreeWidgetItem({NULL, "Index", "Name", "CHID", "TXID", "Type", "CAS", "Provider", "Frequency", "Polarization", "Symbol Rate", "FEC", "SAT", "System"});

	int col = 0;
	list_tree->setHeaderItem(lheader_item);
	list_tree->setColumnHidden(col++, true);
	list_tree->setColumnWidth(col++, 65);		// Index
	list_tree->setColumnWidth(col++, 200);		// Name
	if (gid->sets->value("application/debug", true).toBool()) {
		list_tree->setColumnWidth(col++, 175);	// CHID
		list_tree->setColumnWidth(col++, 150);	// TXID
	}
	else
	{
		list_tree->setColumnHidden(col++, true);
		list_tree->setColumnHidden(col++, true);
	}
	list_tree->setColumnWidth(col++, 85);		// Type
	list_tree->setColumnWidth(col++, 45);		// CAS
	list_tree->setColumnWidth(col++, 150);		// Provider
	list_tree->setColumnWidth(col++, 95);		// Frequency
	list_tree->setColumnWidth(col++, 85);		// Polarization
	list_tree->setColumnWidth(col++, 95);		// Symbol Rate
	list_tree->setColumnWidth(col++, 50);		// FEC
	list_tree->setColumnWidth(col++, 125);		// SAT
	list_tree->setColumnWidth(col++, 75);		// System

	this->lheaderv = list_tree->header();
	lheaderv->connect(lheaderv, &QHeaderView::sectionClicked, [=](int column) { this->trickySortByColumn(column); });

	bouquets_tree->setContextMenuPolicy(Qt::CustomContextMenu);
	bouquets_tree->connect(bouquets_tree, &QTreeWidget::customContextMenuRequested, [=](QPoint pos) { this->showBouquetEditContextMenu(pos); });
	list_tree->setContextMenuPolicy(Qt::CustomContextMenu);
	list_tree->connect(list_tree, &QTreeWidget::customContextMenuRequested, [=](QPoint pos) { this->showListEditContextMenu(pos); });

	this->bouquets_search = new QWidget;
	this->list_search = new QWidget;
	this->list_reference = new QWidget;
	bouquets_search->setHidden(true);
	list_search->setHidden(true);
	list_reference->setHidden(true);
	bouquets_search->setBackgroundRole(QPalette::Mid);
	list_search->setBackgroundRole(QPalette::Mid);
	list_reference->setBackgroundRole(QPalette::Mid);
	bouquets_search->setAutoFillBackground(true);
	list_search->setAutoFillBackground(true);
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

	QGridLayout* bsr_box = new QGridLayout(bouquets_search);
	bsr_box->setContentsMargins(4, 3, 3, 6);
	bsr_box->setSpacing(0);
	this->bsr_search.input = new QLineEdit;
	this->bsr_search.input->connect(this->bsr_search.input, &QLineEdit::textChanged, [=](const QString& text) { this->bouquetsFindPerform(text); });
	this->bsr_search.next = new QPushButton("Find");
	this->bsr_search.next->setStyleSheet("QPushButton, QPushButton:pressed { margin: 0 2px; padding: 3px 2ex; border: 1px solid transparent; border-radius: 3px; background: palette(button) } QPushButton:pressed { background: palette(light) }");
	this->bsr_search.next->connect(this->bsr_search.next, &QPushButton::pressed, [=]() { this->bouquetsFindPerform(); });
	this->bsr_search.close = new QPushButton;
	this->bsr_search.close->setIconSize(QSize(10, 10));
	this->bsr_search.close->setIcon(theme::icon("close"));
	this->bsr_search.close->setFlat(true);
	this->bsr_search.close->setMaximumWidth(22);
	this->bsr_search.close->connect(this->bsr_search.close, &QPushButton::pressed, [=]() { this->bouquetsSearchClose(); });
	bsr_box->addItem(new QSpacerItem(5, 0), 0, 0);
	bsr_box->addWidget(this->bsr_search.input, 0, 1);
	bsr_box->addItem(new QSpacerItem(2, 0), 0, 2);
	bsr_box->addWidget(this->bsr_search.next, 0, 3);
	bsr_box->addItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Preferred), 0, 4);
	bsr_box->addWidget(this->bsr_search.close, 0, 5);

	QGridLayout* lsr_box = new QGridLayout(list_search);
	lsr_box->setContentsMargins(4, 3, 3, 6);
	lsr_box->setSpacing(0);
	this->lsr_search.filter = new QComboBox;
	this->lsr_search.filter->addItem("Name", 2);
	this->lsr_search.filter->addItem("Type", 5);
	this->lsr_search.filter->addItem("CAS", 6);
	this->lsr_search.filter->addItem("Provider", 7);
	this->lsr_search.filter->addItem("SAT", 12);
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

	QToolBar* top_toolbar = new QToolBar;
	top_toolbar->setIconSize(QSize(32, 32));
	top_toolbar->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
	top_toolbar->setStyleSheet("QToolBar { padding: 0 12px } QToolButton { font: 18px }");

	QToolBar* bottom_toolbar = new QToolBar;
	bottom_toolbar->setStyleSheet("QToolBar { padding: 8px 12px } QToolButton { font: bold 16px }");

	QWidget* top_toolbar_spacer = new QWidget;
	top_toolbar_spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	QComboBox* profile_combo = new QComboBox;
	int profile_sel = gid->sets->value("profile/selected").toInt();
	int size = gid->sets->beginReadArray("profile");
	for (int i = 0; i < size; i++)
	{
		gid->sets->setArrayIndex(i);
		if (! gid->sets->contains("profileName"))
			continue;
		profile_combo->addItem(gid->sets->value("profileName").toString(), i + 1); //TODO
	}
	gid->sets->endArray();
	profile_combo->setCurrentIndex(profile_sel);
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
	profile_combo->connect(profile_combo, &QComboBox::currentIndexChanged, [=](int index) { this->profileComboChanged(index); });
#else
	profile_combo->connect(profile_combo, QOverload<int>::of(&QComboBox::currentIndexChanged), [=](int index) { this->profileComboChanged(index); });
#endif

	top_toolbar->addAction(theme::icon("file-open"), "Open", [=]() { this->openFile(); });
	top_toolbar->addAction(theme::icon("save"), "Save", [=]() { this->saveFile(false); });
	top_toolbar->addSeparator();
	top_toolbar->addAction(theme::icon("import"), "Import", [=]() { this->importFile(); });
	top_toolbar->addAction(theme::icon("export"), "Export", [=]() { this->exportFile(); });
	top_toolbar->addSeparator();
	top_toolbar->addAction(theme::icon("settings"), "Settings", [=]() { gid->settings(); });
	top_toolbar->addWidget(top_toolbar_spacer);
	top_toolbar->addWidget(profile_combo);
	top_toolbar->addAction("Connect", [=]() { this->ftpConnect(); });
	top_toolbar->addSeparator();
	top_toolbar->addAction("Upload", [=]() { this->ftpUpload(); });
	top_toolbar->addAction("Download", [=]() { this->ftpDownload(); });

	QWidget* bottom_spacer = new QWidget;
	bottom_spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

	QPushButton* tools_close_edit = new QPushButton;
	tools_close_edit->setIconSize(QSize(16, 16));
	tools_close_edit->setText("Close Editor");
	tools_close_edit->setIcon(theme::icon("close"));
	tools_close_edit->setStyleSheet("QPushButton, QPushButton:pressed { padding: 5px 2ex; border: 1px solid transparent; border-radius: 3px; font: bold 14px; background: palette(button) } QPushButton:pressed { background: palette(light) }");
	tools_close_edit->connect(tools_close_edit, &QPushButton::pressed, [=]() { this->closeTunersets(); });

	this->action.tools_close_edit = bottom_toolbar->addWidget(tools_close_edit);
	this->action.tools_close_edit->setDisabled(true);
	this->action.tools_close_edit->setVisible(false);
	if (gid->sets->value("application/debug", true).toBool())
	{
		bottom_toolbar->addSeparator();
		bottom_toolbar->addAction("?? Load seeds", [=]() { this->loadSeeds(); });
		bottom_toolbar->addAction("?? Reset", [=]() { this->newFile(); tabChangeName(); });
	}
	bottom_toolbar->addWidget(bottom_spacer);

	QToolBar* bouquets_ats = new QToolBar;
	bouquets_ats->setIconSize(QSize(12, 12));
	bouquets_ats->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
	bouquets_ats->setStyleSheet("QToolButton { font: bold 14px }");
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

	this->action.bouquets_search = new QPushButton;
	this->action.bouquets_search->setText("Find???");
	this->action.bouquets_search->setIcon(theme::icon("search"));
	this->action.bouquets_search->connect(this->action.bouquets_search, &QPushButton::pressed, [=]() { this->bouquetsSearchToggle(); });
	this->action.bouquets_search->setDisabled(true);

	this->action.list_search = new QPushButton;
	this->action.list_search->setText("Find???");
	this->action.list_search->setIcon(theme::icon("search"));
	this->action.list_search->connect(this->action.list_search, &QPushButton::pressed, [=]() { this->listSearchToggle(); });

	QWidget* bouquets_ats_spacer = new QWidget;
	bouquets_ats_spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	QWidget* list_ats_spacer = new QWidget;
	list_ats_spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

	bouquets_ats->addAction(theme::icon("add"), "New Bouquet", [=]() { this->addUserbouquet(); });
	bouquets_ats->addWidget(bouquets_ats_spacer);
	bouquets_ats->addWidget(this->action.bouquets_search);
	this->action.list_addch = list_ats->addAction(theme::icon("add"), "Add Channel", [=]() { this->addChannel(); });
	this->action.list_addch->setDisabled(true);
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
	bouquets_tree->viewport()->installEventFilter(bouquets_evth);
	bouquets_tree->connect(bouquets_tree, &QTreeWidget::itemPressed, [=](QTreeWidgetItem* item) { this->treeSwitched(bouquets_tree, item); });
	bouquets_tree->connect(bouquets_tree, &QTreeWidget::currentItemChanged, [=](QTreeWidgetItem* current) { this->bouquetsItemChanged(current); });
	list_tree->installEventFilter(list_evto);
	list_tree->viewport()->installEventFilter(list_evth);
	list_tree->connect(list_tree, &QTreeWidget::currentItemChanged, [=]() { this->listItemChanged(); });
	list_tree->connect(list_tree, &QTreeWidget::itemSelectionChanged, [=]() { this->listItemSelectionChanged(); });
	list_tree->connect(list_tree, &QTreeWidget::itemDoubleClicked, [=]() { this->editService(); });

	top->addWidget(top_toolbar);
	bottom->addWidget(bottom_toolbar);

	services_box->addWidget(services_tree);
	services->setLayout(services_box);

	bouquets_box->addWidget(bouquets_tree);
	bouquets_box->addWidget(bouquets_search);
	bouquets_box->addWidget(bouquets_ats);
	bouquets->setLayout(bouquets_box);

	side_box->addWidget(services);
	side_box->addItem(new QSpacerItem(0, 8, QSizePolicy::Preferred, QSizePolicy::Fixed));
	side_box->addWidget(bouquets, 1);
	side_box->setContentsMargins(0, 0, 0, 0);
	side->setLayout(side_box);

	list_layout->addWidget(list_tree);
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

	container->addWidget(splitterc, 0, 0, 1, 1);
	container->setContentsMargins(8, 8, 8, 8);

	frm->setContentsMargins(0, 0, 0, 0);
	frm->addLayout(top, 0, 0);
	frm->addLayout(container, 1, 0);
	frm->addLayout(bottom, 2, 0);

	this->root = container;
	this->tools = new e2se_gui_tools::tools(root, this->log->log);

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
		QVariantMap tdata; //TODO
		tdata["id"] = item.first;
		titem->setData(0, Qt::UserRole, QVariant (tdata));
		titem->setText(0, item.second);
		titem->setIcon(0, theme::spacer(2));
		services_tree->addTopLevelItem(titem);
	}

	newFile();
}

tab::~tab()
{
	delete this->dbih;
}

void tab::newFile()
{
	debug("newFile()");

	closeTunersets();
	gid->update(gui::init);
	initialize();

	if (this->dbih != nullptr)
		delete this->dbih;
	if (this->tools != nullptr)
		this->tools->destroy();

	this->dbih = new e2db(this->log->log);

	load();
}

void tab::openFile()
{
	debug("openFile()");

	string path = gid->openFileDialog();

	if (! path.empty())
		readFile(path);
}

void tab::saveFile(bool saveas)
{
	debug("saveFile()", "saveas", saveas);

	QMessageBox dial = QMessageBox();
	string path;
	bool overwrite = ! saveas && (! this->state.nwwr || this->state.ovwr);

	if (overwrite)
	{
		this->updateListIndex();
		this->updateBouquetsIndex();
		path = this->filename;
		dial.setText("Files will be overwritten.");
		dial.exec();
	}
	else
	{
		path = gid->saveFileDialog(this->filename);
	}

	if (! path.empty())
	{
		debug("saveFile()", "overwrite", overwrite);
		debug("saveFile()", "filename", filename);

		QGuiApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
		bool wr = dbih->write(path, overwrite);
		QGuiApplication::restoreOverrideCursor();
		
		if (wr) {
			dial.setText("Saved!");
			dial.exec();
		}
		else
		{
			QMessageBox::critical(cwid, NULL, "Error writing files.");
		}
	}
}

//TODO tools: tunersets
void tab::importFile()
{
	debug("importFile()");

	vector<string> paths;

	paths = gid->importFileDialog();
	if (! paths.empty())
	{
		QGuiApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
		dbih->importFile(paths);
		QGuiApplication::restoreOverrideCursor();
		initialize();
		load();
	}
}

void tab::exportFile()
{
	debug("exportFile()");

	gui::GUI_DPORTS gde = gui::GUI_DPORTS::_default;
	vector<string> paths;
	string filename;
	int flags = -1;

	// tools: tunersets
	if (this->state.tunersets)
	{
		gde = gui::GUI_DPORTS::Tunersets;
		flags = e2db::FPORTS::singleTunersets;
		switch (this->state.ty)
		{
			case e2db::YTYPE::sat:
				filename = "satellites.xml";
			break;
			case e2db::YTYPE::terrestrial:
				filename = "terrestrial.xml";
			break;
			case e2db::YTYPE::cable:
				filename = "cables.xml";
			break;
			case e2db::YTYPE::atsc:
				filename = "atsc.xml";
			break;
		}
		paths.push_back(filename);
	}
	// services
	else if (this->state.tc == 0)
	{
		gde = gui::GUI_DPORTS::Services;
		flags = e2db::FPORTS::allServices;
		filename = "lamedb";
		paths.push_back(filename);
	}
	// bouquets
	else if (this->state.tc == 1)
	{
		int ti = -1;
		QList<QTreeWidgetItem*> selected = bouquets_tree->selectedItems();

		if (selected.empty())
		{
			return;
		}
		for (auto & item : selected)
		{
			ti = bouquets_tree->indexOfTopLevelItem(item);
			QVariantMap tdata = item->data(0, Qt::UserRole).toMap();
			QString qchlist = tdata["id"].toString();
			string filename = qchlist.toStdString();

			paths.push_back(filename);
		}
		if (paths.size() == 1)
		{
			filename = paths[0];
		}
		// bouquet | userbouquets
		if (ti != -1)
		{
			gde = gui::GUI_DPORTS::Bouquets;
			flags = e2db::FPORTS::singleBouquetAll;

			if (dbih->bouquets.count(filename))
			{
				for (string & w : dbih->bouquets[filename].userbouquets)
					paths.push_back(w);
			}
		}
		// userbouquet
		else
		{
			gde = gui::GUI_DPORTS::Userbouquets;
			flags = e2db::FPORTS::singleUserbouquet;
		}
	}
	if (paths.empty())
	{
		return;
	}

	string path = gid->exportFileDialog(gde, filename, flags);

	if (! path.empty())
	{
		debug("exportFile()", "flags", flags);

		if (gde == gui::GUI_DPORTS::Services || gde == gui::GUI_DPORTS::Tunersets)
		{
			paths[0] = path;
		}
		else
		{
			string basedir = std::filesystem::path(path).remove_filename(); //C++17
			//TODO rend trailing
			for (string & w : paths)
				w = basedir + w;
		}

		QMessageBox dial = QMessageBox();
		QGuiApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
		dbih->exportFile(flags, paths);
		QGuiApplication::restoreOverrideCursor();
		dial.setText("Saved!");
		dial.exec();
	}
}

void tab::exportFile(QTreeWidgetItem* item)
{
	debug("exportFile()");

	gui::GUI_DPORTS gde = gui::GUI_DPORTS::_default;
	vector<string> paths;
	string filename;
	int flags = -1;

	if (item == nullptr)
	{
		return;
	}
	// services
	else if (this->state.tc == 0)
	{
		gde = gui::GUI_DPORTS::Services;
		filename = "lamedb";
		paths.push_back(filename);
	}
	// bouquets
	else if (this->state.tc == 1)
	{
		int ti = bouquets_tree->indexOfTopLevelItem(item);
		QVariantMap tdata = item->data(0, Qt::UserRole).toMap();
		QString qchlist = tdata["id"].toString();
		filename = qchlist.toStdString();
		paths.push_back(filename);

		// bouquet | userbouquets
		if (ti != -1)
		{
			gde = gui::GUI_DPORTS::Bouquets;
			flags = e2db::FPORTS::singleBouquetAll;

			if (dbih->bouquets.count(filename))
			{
				for (string & w : dbih->bouquets[filename].userbouquets)
					paths.push_back(w);
			}
		}
		// userbouquet
		else
		{
			gde = gui::GUI_DPORTS::Userbouquets;
			flags = e2db::FPORTS::singleUserbouquet;
		}
	}
	if (paths.empty())
	{
		return;
	}

	string path = gid->exportFileDialog(gde, filename, flags);

	if (! path.empty())
	{
		debug("exportFile()", "filename", filename);

		QMessageBox dial = QMessageBox();
		QGuiApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
		dbih->exportFile(flags, paths);
		QGuiApplication::restoreOverrideCursor();
		dial.setText("Saved!");
		dial.exec();
	}
}

void tab::addUserbouquet()
{
	debug("addUserbouquet()");

	string bname;
	e2se_gui::editBouquet* add = new e2se_gui::editBouquet(dbih, this->state.ti, this->log->log);
	add->display(cwid);
	bname = add->getEditID(); // returned after dial.exec()
	add->destroy();

	if (bname.empty())
		return;

	e2db::userbouquet uboq = dbih->userbouquets[bname];
	e2db::bouquet gboq = dbih->bouquets[uboq.pname];
	int pidx = gboq.btype == 1 ? 0 : 1;
	QTreeWidgetItem* pgroup = bouquets_tree->topLevelItem(pidx);
	// macos: unwanted chars [qt.qpa.fonts] Menlo notice
	QString name;
	if (gid->sets->value("preference/fixUnicodeChars").toBool())
		name = QString::fromStdString(uboq.name).remove(QRegularExpression("[^\\p{L}\\p{N}\\p{Sm}\\p{M}\\p{P}\\s]+"));
	else
		name = QString::fromStdString(uboq.name);

	QTreeWidgetItem* bitem = new QTreeWidgetItem(pgroup);
	bitem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled | Qt::ItemNeverHasChildren);
	QMap<QString, QVariant> tdata; //TODO
	tdata["id"] = QString::fromStdString(uboq.bname);
	bitem->setData(0, Qt::UserRole, QVariant (tdata));
	bitem->setText(0, name);
	bouquets_tree->addTopLevelItem(bitem);

	updateBouquetsIndex();
}

void tab::editUserbouquet()
{
	debug("editUserbouquet()");

	QList<QTreeWidgetItem*> selected = bouquets_tree->selectedItems();
	
	if (selected.empty() || selected.count() > 1)
		return;

	QTreeWidgetItem* item = selected.first();
	QVariantMap tdata = item->data(0, Qt::UserRole).toMap();
	QString qbname = tdata["id"].toString();
	string bname = qbname.toStdString();

	debug("editUserbouquet()", "bname", bname);

	e2se_gui::editBouquet* edit = new e2se_gui::editBouquet(dbih, this->state.ti, this->log->log);
	edit->setEditID(bname);
	edit->display(cwid);
	edit->destroy();

	e2db::userbouquet uboq = dbih->userbouquets[bname];
	QString name;
	if (gid->sets->value("preference/fixUnicodeChars").toBool())
		name = QString::fromStdString(uboq.name).remove(QRegularExpression("[^\\p{L}\\p{N}\\p{Sm}\\p{M}\\p{P}\\s]+"));
	else
		name = QString::fromStdString(uboq.name);
	selected[0]->setText(0, name);

	updateBouquetsIndex();
}

void tab::addChannel()
{
	debug("addChannel()");

	e2se_gui::channelBook* cb = new e2se_gui::channelBook(dbih, this->log->log);
	string curr_chlist = this->state.curr;
	QDialog* dial = new QDialog(cwid);
	dial->setMinimumSize(760, 420);
	dial->setWindowTitle("Add Channel");
	dial->connect(dial, &QDialog::finished, [=]() { delete cb; delete dial; });

	QGridLayout* layout = new QGridLayout;
	QToolBar* bottom_toolbar = new QToolBar;
	bottom_toolbar->setIconSize(QSize(16, 16));
	bottom_toolbar->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
	bottom_toolbar->setStyleSheet("QToolBar { padding: 0 8px } QToolButton { font: 16px }");
	QWidget* bottom_spacer = new QWidget;
	bottom_spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	bottom_toolbar->addWidget(bottom_spacer);
	bottom_toolbar->addAction(theme::icon("add"), "Add", [=]() { auto selected = cb->getSelected(); this->putChannels(selected); });

	layout->addWidget(cb->widget);
	layout->addWidget(bottom_toolbar);
	layout->setContentsMargins(0, 0, 0, 0);
	dial->setLayout(layout);
	dial->exec();
}

void tab::addService()
{
	debug("addService()");

	e2se_gui::editService* add = new e2se_gui::editService(dbih, this->log->log);
	add->display(cwid);
	add->destroy();
}

void tab::editService()
{
	debug("editService()");

	QList<QTreeWidgetItem*> selected = list_tree->selectedItems();
	
	if (selected.empty() || selected.count() > 1)
		return;

	QTreeWidgetItem* item = selected.first();
	string chid = item->data(2, Qt::UserRole).toString().toStdString();
	string nw_chid;
	bool marker = item->data(1, Qt::UserRole).toBool();

	debug("editService()", "chid", chid);

	if (! marker && dbih->db.services.count(chid))
	{
		e2se_gui::editService* edit = new e2se_gui::editService(dbih, this->log->log);
		edit->setEditID(chid);
		edit->display(cwid);
		nw_chid = edit->getEditID(); // returned after dial.exec()
		edit->destroy();

		cache.clear();

		debug("editService()", "nw_chid", nw_chid);

		QStringList entry = dbih->entries.services[nw_chid];
		entry.prepend(item->text(1));
		entry.prepend(item->text(0));
		for (int i = 0; i < entry.count(); i++)
			item->setText(i, entry[i]);
		item->setData(ITEM_DATA_ROLE::chid, Qt::UserRole, QString::fromStdString(nw_chid));
	}
}

bool tab::readFile(string filename)
{
	debug("readFile()", "filename", filename);

	if (filename.empty())
		return false;

	closeTunersets();
	initialize();

	if (this->dbih != nullptr)
		delete this->dbih;

	this->dbih = new e2db(this->log->log);

	QGuiApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
	bool rr = dbih->prepare(filename);
	QGuiApplication::restoreOverrideCursor();

	if (rr)
	{
		tabChangeName(filename);
	}
	else
	{
		tabChangeName();
		QMessageBox::critical(cwid, NULL, "Error opening files.");
		return false;
	}

	this->state.nwwr = false;
	this->filename = filename;

	load();

	return true;
}

void tab::load()
{
	debug("load()");

	sort(dbih->index["bss"].begin(), dbih->index["bss"].end());
	unordered_map<string, QTreeWidgetItem*> bgroups;

	for (auto & bsi : dbih->index["bss"])
	{
		debug("load()", "bouquet", bsi.second);
		e2db::bouquet gboq = dbih->bouquets[bsi.second];
		QString bname = QString::fromStdString(bsi.second);
		QString name = QString::fromStdString(gboq.nname.empty() ? gboq.name : gboq.nname);

		QTreeWidgetItem* bitem = new QTreeWidgetItem();
		bitem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
		QMap<QString, QVariant> tdata; //TODO
		tdata["id"] = bname;
		bitem->setData(0, Qt::UserRole, QVariant (tdata));
		bitem->setText(0, name);
		bouquets_tree->addTopLevelItem(bitem);
		bouquets_tree->expandItem(bitem);
	
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
		if (gid->sets->value("preference/fixUnicodeChars").toBool())
			name = QString::fromStdString(uboq.name).remove(QRegularExpression("[^\\p{L}\\p{N}\\p{Sm}\\p{M}\\p{P}\\s]+"));
		else
			name = QString::fromStdString(uboq.name);

		QTreeWidgetItem* bitem = new QTreeWidgetItem(pgroup);
		bitem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled | Qt::ItemNeverHasChildren);
		QMap<QString, QVariant> tdata; //TODO
		tdata["id"] = bname;
		bitem->setData(0, Qt::UserRole, QVariant (tdata));
		bitem->setText(0, name);
		bouquets_tree->addTopLevelItem(bitem);
	}

	bouquets_tree->setDragEnabled(true);
	bouquets_tree->setAcceptDrops(true);
	services_tree->setCurrentItem(services_tree->topLevelItem(0));
	populate(services_tree);
	updateConnectors();
	updateCounters();
}

void tab::populate(QTreeWidget* side_tree)
{
	string curr_chlist;
	string prev_chlist;
	bool precached = false;
	QList<QTreeWidgetItem*> cachep;
	
	if (! cache.empty())
	{
		for (int i = 0; i < list_tree->topLevelItemCount(); i++)
		{
			QTreeWidgetItem* item = list_tree->topLevelItem(i);
			cachep.append(item->clone());
		}
		precached = true;
		prev_chlist = string (this->state.curr);
	}

	QTreeWidgetItem* selected = side_tree->currentItem();
	if (selected == NULL)
	{
		return;
	}
	if (selected != NULL)
	{
		QVariantMap tdata = selected->data(0, Qt::UserRole).toMap();
		QString qcurr_bouquet = tdata["id"].toString();
		curr_chlist = qcurr_bouquet.toStdString();
		this->state.curr = curr_chlist;
	}

	debug("populate()", "curr_chlist", curr_chlist);

	if (! dbih->index.count(curr_chlist))
		error("populate()", "curr_chlist", curr_chlist);

	lheaderv->setSortIndicatorShown(true);
	lheaderv->setSectionsClickable(false);
	list_tree->clear();
	if (precached)
	{
		cache[prev_chlist].swap(cachep);
	}

	int i = 0;

	if (cache[curr_chlist].isEmpty())
	{
		for (auto & ch : dbih->index[curr_chlist])
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
				if (dbih->userbouquets.count(curr_chlist))
					chref = dbih->userbouquets[curr_chlist].channels[ch.second];

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
			item->setIcon(1, theme::spacer(4));
			if (marker)
			{
				item->setFont(2, QFont(theme::fontFamily(), theme::calcFontSize(-1), QFont::Weight::Bold));
				item->setFont(5, QFont(theme::fontFamily(), theme::calcFontSize(-1), QFont::Weight::Bold));
			}
			item->setFont(6, QFont(theme::fontFamily(), theme::calcFontSize(-1)));
			/*if (! entry.at(6).isEmpty())
			{
				item->setIcon(6, theme::icon("crypted"));
			}*/
			cache[curr_chlist].append(item);
		}
	}

	list_tree->addTopLevelItems(cache[curr_chlist]);

	// sorting by
	if (this->state.sort.first != -1)
	{
		list_tree->sortItems(this->state.sort.first, this->state.sort.second);

		// sorting column 0|desc
		if (this->state.sort.first == 0 && this->state.sort.second == Qt::DescendingOrder)
			lheaderv->setSortIndicator(1, this->state.sort.second);
	}
	// sorting default
	else if (this->state.dnd)
	{
		list_tree->setDragEnabled(true);
		list_tree->setAcceptDrops(true);
	}
	lheaderv->setSectionsClickable(true);
}

void tab::treeSwitched(QTreeWidget* tree, QTreeWidgetItem* item)
{
	debug("treeSwitched()");

	int tc = -1;

	if (tree == this->services_tree)
		tc = 0;
	else if (tree == this->bouquets_tree)
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

void tab::servicesItemChanged(QTreeWidgetItem* current)
{
	debug("servicesItemChanged()");

	this->state.tc = 0;

	if (current != NULL)
	{
		int ti = services_tree->indexOfTopLevelItem(current);

		this->action.list_addch->setDisabled(true);
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

		gid->update(gui::TabListDelete, true);
		gid->update(gui::TabListPaste, true);

		list_tree->clearSelection();
		list_tree->scrollToTop();
	}

	updateListIndex();
	populate(services_tree);
	updateConnectors();
	updateCounters(true);
}

void tab::bouquetsItemChanged(QTreeWidgetItem* current)
{
	debug("bouquetsItemChanged()");

	this->state.tc = 1;

	if (current != NULL)
	{
		int ti = bouquets_tree->indexOfTopLevelItem(current);
		this->state.ti = ti;

		// bouquet: tv | radio
		if (ti != -1)
		{
			this->action.list_addch->setDisabled(true);
			this->action.list_newch->setEnabled(true);

			disallowDnD();

			// sorting by
			if (this->state.sort.first > 0)
				this->action.list_dnd->setDisabled(true);

			gid->update(gui::TabListDelete, false);
			gid->update(gui::TabListPaste, false);
		}
		// userbouquet
		else
		{
			this->action.list_addch->setEnabled(true);
			this->action.list_newch->setDisabled(true);

			// sorting by
			if (this->state.sort.first > 0)
				this->action.list_dnd->setEnabled(true);
			// sorting default
			else
				allowDnD();

			gid->update(gui::TabListDelete, true);
			gid->update(gui::TabListPaste, true);
		}

		list_tree->clearSelection();
		list_tree->scrollToTop();
	}

	updateListIndex();
	populate(bouquets_tree);
	updateConnectors();
	updateCounters(true);
}

void tab::listItemChanged()
{
	// debug("listItemChanged()");

	if (list_evto->isChanged())
		listPendingUpdate();
}

void tab::listItemSelectionChanged()
{
	// debug("listItemSelectionChanged()");
	
	QList<QTreeWidgetItem*> selected = list_tree->selectedItems();

	if (selected.empty())
	{
		gid->update(gui::TabListCut, false);
		gid->update(gui::TabListCopy, false);

		// userbouquet
		if (this->state.ti == -1)
			gid->update(gui::TabListDelete, false);
	}
	else
	{
		gid->update(gui::TabListCut, true);
		gid->update(gui::TabListCopy, true);

		// userbouquet
		if (this->state.ti == -1)
			gid->update(gui::TabListDelete, true);
	}
	if (selected.count() > 1)
	{
		gid->update(gui::TabListEditService, false);
		gid->update(gui::TabListEditMarker, false);
	}
	else
	{
		gid->update(gui::TabListEditService, true);
		gid->update(gui::TabListEditMarker, true);
	}

	if (this->state.refbox)
		updateRefBox();
}

void tab::listPendingUpdate()
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

void tab::visualReindexList()
{
	debug("visualReindexList()");

	// sorting column 0|desc || column 0|asc
	bool reverse = (this->state.sort.first < 1 && this->state.sort.second == Qt::DescendingOrder) ? true : false;

	int i = 0, y = 0, idx = 0;
	int j = list_tree->topLevelItemCount();

	if (reverse)
	{
		while (j--)
		{
			QTreeWidgetItem* item = list_tree->topLevelItem(i);
			bool marker = item->data(1, Qt::UserRole).toBool();
			if (marker)
				y++;
			i++;
		}
		i = 0;
		j = list_tree->topLevelItemCount();
	}
	while (reverse ? j-- : i != j)
	{
		QTreeWidgetItem* item = list_tree->topLevelItem(i);
		bool marker = item->data(1, Qt::UserRole).toBool();
		idx = reverse ? j : i;
		char ci[7];
		std::sprintf(ci, "%06d", idx++);
		item->setText(0, QString::fromStdString(ci));
		if (! marker)
			item->setText(1, QString::fromStdString(to_string(idx - y)));
		i++;
		y = marker ? reverse ? y - 1 : y + 1 : y;
	}

	this->state.reindex = false;
}

void tab::trickySortByColumn(int column)
{
	debug("trickySortByColumn()", "column", column);

	Qt::SortOrder order = this->state.sort.first == -1 ? Qt::DescendingOrder : lheaderv->sortIndicatorOrder();
	column = column == 1 ? 0 : column;

	// sorting by
	if (column)
	{
		list_tree->sortItems(column, order);
		disallowDnD();

		// userbouquet
		if (this->state.ti == -1)
			this->action.list_dnd->setEnabled(true);

		lheaderv->setSortIndicatorShown(true);
	}
	// sorting default
	else
	{
		list_tree->sortItems(column, order);
		lheaderv->setSortIndicator(1, order);
		allowDnD();

		this->action.list_dnd->setDisabled(true);

		// default column 0|asc
		if (order == Qt::AscendingOrder)
			lheaderv->setSortIndicatorShown(false);
		else
			lheaderv->setSortIndicatorShown(true);

		if (this->state.reindex)
			this->visualReindexList();
	}
	this->state.sort = pair (column, order); //C++17
}

void tab::allowDnD()
{
	debug("allowDnd()");

	this->list_evth->allowInternalMove();
	this->state.dnd = true;
	// list_wrap->setStyleSheet("#channels_wrap { background: transparent }");
}

void tab::disallowDnD()
{
	debug("disallowDnD()");

	this->list_evth->disallowInternalMove();
	this->state.dnd = false;
	// list_wrap->setStyleSheet("#channels_wrap { background: rgba(255, 192, 0, 20%) }");
}

void tab::reharmDnD()
{
	debug("reharmDnD()");

	// sorting default 0|asc
	list_tree->sortItems(0, Qt::AscendingOrder);
	list_tree->setDragEnabled(true);
	list_tree->setAcceptDrops(true);
	this->state.sort = pair (0, Qt::AscendingOrder); //C++17
	this->action.list_dnd->setDisabled(true);
}

void tab::bouquetItemDelete()
{
	debug("bouquetItemDelete()");

	QList<QTreeWidgetItem*> selected = bouquets_tree->selectedItems();
	
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
	updateBouquetsIndex();
}

void tab::actionCall(int action)
{
	debug("actionCall()", "action", action);

	switch (action)
	{
		case LIST_EDIT_ATS::Cut:
			listItemCut();
		break;
		case LIST_EDIT_ATS::Copy:
			listItemCopy();
		break;
		case LIST_EDIT_ATS::Paste:
			listItemPaste();
		break;
		case LIST_EDIT_ATS::Delete:
			listItemDelete();
		break;
		case LIST_EDIT_ATS::SelectAll:
			listItemSelectAll();
		break;

		case gui::TAB_ATS::BouquetsFind:
			bouquetsSearchShow();
		break;
		case gui::TAB_ATS::BouquetsFindNext:
			bouquetsFindPerform();
		break;
		case gui::TAB_ATS::ListFind:
			listSearchShow();
		break;
		case gui::TAB_ATS::ListFindNext:
			listFindPerform(LIST_FIND::next);
		break;
		case gui::TAB_ATS::ListFindPrev:
			listFindPerform(LIST_FIND::prev);
		break;
		case gui::TAB_ATS::ListFindAll:
			listFindPerform(LIST_FIND::all);
		break;

		case gui::TAB_ATS::EditTunerSat:
			editTunersets(e2db::YTYPE::sat);
		break;
		case gui::TAB_ATS::EditTunerTerrestrial:
			editTunersets(e2db::YTYPE::terrestrial);
		break;
		case gui::TAB_ATS::EditTunerCable:
			editTunersets(e2db::YTYPE::cable);
		break;
		case gui::TAB_ATS::EditTunerAtsc:
			editTunersets(e2db::YTYPE::atsc);
		break;
		case gui::TAB_ATS::Inspector:
			tools->inspector();
		break;
	}
}

void tab::bouquetsSearchShow()
{
	debug("bouquetsSearchShow()");

	bouquets_search->show();
}

void tab::bouquetsSearchHide()
{
	debug("bouquetsSearchHide()");

	bouquets_search->hide();
}

void tab::bouquetsSearchToggle()
{
	// debug("bouquetsSearchToggle()");

	if (bouquets_search->isHidden())
	{
		bouquetsSearchShow();
	}
	else
	{
		bouquetsSearchHide();
	}
}

void tab::bouquetsSearchClose()
{
	// debug("bouquetsSearchClose()");

	QTimer::singleShot(100, [=]() {
		bouquetsSearchHide();
	});
}

void tab::listSearchShow()
{
	debug("listSearchShow()");

	list_search->show();
	this->lsr_find.timer.start();
}

void tab::listSearchHide()
{
	debug("listSearchHide()");

	list_search->hide();
	if (! this->lsr_find.highlight)
		listFindClear();
}

void tab::listSearchToggle()
{
	// debug("listSearchToggle()");

	if (list_search->isHidden())
		listSearchShow();
	else
		listSearchHide();
}

void tab::listSearchClose()
{
	// debug("listSearchClose()");

	QTimer::singleShot(100, [=]() {
		listSearchHide();
	});
}

void tab::listReferenceToggle()
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

void tab::bouquetsFindPerform()
{
	if (this->bsr_search.input->text().isEmpty())
		return;

	bouquetsFindPerform(this->bsr_search.input->text());
}

void tab::bouquetsFindPerform(const QString& value)
{
	bouquets_tree->keyboardSearch(value);

	gid->update(gui::TabBouquetsFind, true);
	if (bouquets_tree->currentIndex().isValid())
		gid->update(gui::TabBouquetsFindNext, true);
	else
		gid->update(gui::TabBouquetsFindNext, false);
}

void tab::listFindPerform(LIST_FIND flag)
{
	if (this->lsr_search.input->text().isEmpty())
		return;

	listFindPerform(this->lsr_search.input->text(), flag);
}

//TODO FIX
// multiple selection with shortcut FindNext when search_box is closed
void tab::listFindPerform(const QString& value, LIST_FIND flag)
{
	// QTreeWidgetItem* item list_tree->currentItem() || list_tree->topLevelItem(0)
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

	if (this->lsr_find.filter != column || this->lsr_find.input != text)
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
		QModelIndex start = list_tree->model()->index(0, column);
		int limit = -1;
		match = list_tree->model()->match(start, Qt::DisplayRole, text, limit, Qt::MatchFlag::MatchContains);

		if (this->lsr_find.flag == LIST_FIND::all)
			listFindClear();

		this->lsr_find.curr = -1;
	}
	else
	{
		match = this->lsr_find.match;

		//TODO
		/*if (flag == LIST_FIND::fast && this->lsr_find.flag == LIST_FIND::all)
		{
			if (list_tree->currentIndex().isValid())
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
			list_tree->setCurrentIndex(match.at(i));
		}
		else if (type == LIST_FIND::prev)
		{
			i = int (this->lsr_find.curr);
			i = i <= 0 ? j - 1 : i - 1;
			list_tree->setCurrentIndex(match.at(i));
		}
		else if (type == LIST_FIND::all)
		{

			listFindClear(false);
			while (i != j)
			{
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
				QTreeWidgetItem* item = list_tree->itemFromIndex(match.at(i));
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
				list_tree->scrollTo(match.at(0));
			i = -1;
		}
		// int i = list_tree->indexOfTopLevelItem(QTreeWidgetItem* item);
		this->lsr_find.curr = i;

		gid->update(gui::TabListFindNext, true);
		gid->update(gui::TabListFindPrev, true);
		gid->update(gui::TabListFindAll, true);
	}
	else
	{
		gid->update(gui::TabListFindNext, false);
		gid->update(gui::TabListFindPrev, false);
		gid->update(gui::TabListFindAll, false);
	}

	this->lsr_find.flag = flag;
	this->lsr_find.filter = column;
	this->lsr_find.input = text;
	this->lsr_find.match = match;
}

void tab::listFindHighlightToggle()
{
	listFindReset();
	this->lsr_find.highlight = ! this->lsr_find.highlight;
}

void tab::listFindClear(bool hidden)
{
	list_tree->clearSelection();

	if (! this->lsr_find.highlight)
	{
		int j = list_tree->topLevelItemCount();
		while (j--)
		{
			QTreeWidgetItem* item = list_tree->topLevelItem(j);
			item->setHidden(! hidden);
		}
	}
}

void tab::listFindReset()
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

void tab::listItemCut()
{
	debug("listItemCut()");

	listItemCopy(true);
}

//TODO Reference ID
void tab::listItemCopy(bool cut)
{
	debug("listItemCopy()");

	QList<QTreeWidgetItem*> selected = list_tree->selectedItems();
	
	if (selected.empty())
		return;

	QClipboard* clipboard = QGuiApplication::clipboard();
	QStringList text;
	for (auto & item : selected)
	{
		QStringList data;
		// skip column 0 = x index
		for (int i = 1; i < list_tree->columnCount(); i++)
		{
			QString qstr = item->data(i, Qt::DisplayRole).toString();
			// CAS
			if (i == 6 && ! qstr.isEmpty())
				qstr.replace(", ", "|").replace(" ", "");

			data.append(qstr);
		}
		text.append(data.join(",")); // CSV
	}
	clipboard->setText(text.join("\n")); // CSV

	if (cut)
		listItemDelete();
}

//TODO validate
void tab::listItemPaste()
{
	// bouquet: tv | radio
	if (this->state.ti != -1)
		return;

	debug("listItemPaste()");

	QClipboard* clipboard = QGuiApplication::clipboard();
	const QMimeData* mimeData = clipboard->mimeData();
	vector<QString> items;

	if (mimeData->hasText())
	{
		QStringList list = clipboard->text().split('\n');

		for (QString & data : list)
		{
			if (data.contains(','))
				items.emplace_back(data.split(',')[2]);
		}
	}
	if (! items.empty())
	{
		putChannels(items);

		if (list_tree->currentItem() == nullptr)
			list_tree->scrollToBottom();

		// bouquets tree
		if (this->state.tc)
		{
			string curr_chlist = this->state.curr;
			e2db::userbouquet uboq = dbih->userbouquets[curr_chlist];
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

void tab::listItemDelete()
{
	// bouquet: tv | radio
	if (this->state.ti != -1)
		return;

	debug("listItemDelete()");

	QList<QTreeWidgetItem*> selected = list_tree->selectedItems();
	
	if (selected.empty())
		return;

	lheaderv->setSectionsClickable(false);
	list_tree->setDragEnabled(false);
	list_tree->setAcceptDrops(false);

	string curr_chlist = this->state.curr;
	string pname;

	// bouquets tree
	if (this->state.tc)
	{
		e2db::userbouquet uboq = dbih->userbouquets[curr_chlist];
		pname = uboq.pname;
	}
	for (auto & item : selected)
	{
		int i = list_tree->indexOfTopLevelItem(item);
		string chid = item->data(ITEM_DATA_ROLE::chid, Qt::UserRole).toString().toStdString();
		list_tree->takeTopLevelItem(i);

		// bouquets tree
		if (this->state.tc)
		{
			dbih->remove_channel_reference(chid, curr_chlist);
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

	lheaderv->setSectionsClickable(true);
	list_tree->setDragEnabled(true);
	list_tree->setAcceptDrops(true);

	// sorting default
	if (this->state.dnd)
		visualReindexList();
	else
		this->state.reindex = true;
	this->state.changed = true;

	updateConnectors();
	updateCounters();
}

void tab::listItemSelectAll()
{
	debug("listItemSelectAll()");

	list_tree->selectAll();
}

void tab::editTunersets(int ytype)
{
	debug("editTunersets()", "ytype", ytype);

	if (! this->state.tunersets)
	{
		root->itemAt(0)->widget()->hide();
		tools->editTunersets(dbih, ytype);
		this->state.tunersets = true;
		this->state.ty = ytype;

		QTimer::singleShot(100, [=]() {
			this->action.tools_close_edit->setDisabled(false);
			this->action.tools_close_edit->setVisible(true);
		});
	}
}

void tab::closeTunersets()
{
	debug("closeTunersets()");

	if (this->state.tunersets)
	{
		tools->closeTunersets();
		root->itemAt(0)->widget()->show();
		this->state.tunersets = false;
		this->state.ty = -1;

		QTimer::singleShot(300, [=]() {
			this->action.tools_close_edit->setVisible(false);
			this->action.tools_close_edit->setDisabled(true);
		});
	}
}

//TODO duplicates and new
void tab::putChannels(vector<QString> channels)
{
	debug("putChannels()");

	lheaderv->setSectionsClickable(false);
	list_tree->setDragEnabled(false);
	list_tree->setAcceptDrops(false);
	QList<QTreeWidgetItem*> clist;
	string curr_chlist = this->state.curr;
	int i = 0, y;
	QTreeWidgetItem* current = list_tree->currentItem();
	QTreeWidgetItem* parent = list_tree->invisibleRootItem();
	i = current != nullptr ? parent->indexOfChild(current) : list_tree->topLevelItemCount();
	y = i + 1;

	for (QString & q : channels)
	{
		string chid = q.toStdString();
		char ci[7];
		std::sprintf(ci, "%06d", i++);
		QString x = QString::fromStdString(ci);
		QString idx = QString::fromStdString(to_string(i));
		QStringList entry;
		bool marker = false;
		e2db::channel_reference chref;
		e2db::service ch;

		if (dbih->db.services.count(chid))
		{
			ch = dbih->db.services[chid];
			entry = dbih->entries.services[chid];
			entry.prepend(idx);
			entry.prepend(x);

			chref.marker = false;
			chref.chid = chid;
			chref.type = ch.stype;
			chref.index = idx.toInt();
		}
		else
		{
			string chlist;

			for (auto & q : dbih->index["mks"])
			{
				if (q.second == chid)
				{
					for (auto & u : dbih->index["ubs"])
					{
						if (u.first == q.first)
							chlist = u.second;
					}
				}
			}

			if (chlist.empty())
			{
				error("putChannels()", "chid", chid);
				continue;
			}
			else
			{
				if (dbih->userbouquets.count(chlist))
				{
					chref = dbih->userbouquets[chlist].channels[chid];
					chref.index = idx.toInt();
				}
				marker = true;
				entry = dbih->entryMarker(chref);
				idx = entry[1];
				entry.prepend(x);
			}
		}
		QTreeWidgetItem* item = new QTreeWidgetItem(entry);
		item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsDragEnabled | Qt::ItemNeverHasChildren);
		item->setData(ITEM_DATA_ROLE::idx, Qt::UserRole, idx);
		item->setData(ITEM_DATA_ROLE::marker, Qt::UserRole, marker);
		item->setData(ITEM_DATA_ROLE::chid, Qt::UserRole, q);
		item->setIcon(1, theme::spacer(4));
		if (marker)
		{
			item->setFont(2, QFont(theme::fontFamily(), theme::calcFontSize(-1), QFont::Weight::Bold));
			item->setFont(5, QFont(theme::fontFamily(), theme::calcFontSize(-1), QFont::Weight::Bold));
		}
		item->setFont(6, QFont(theme::fontFamily(), theme::calcFontSize(-1)));
		if (! entry.at(6).isEmpty())
		{
			item->setIcon(6, theme::icon("crypted"));
		}
		clist.append(item);

		// bouquets tree
		if (this->state.tc)
			dbih->add_channel_reference(chref, curr_chlist);
		// services tree
		else
			dbih->addService(ch);
	}

	if (current == nullptr)
		list_tree->addTopLevelItems(clist);
	else
		list_tree->insertTopLevelItems(y, clist);

	lheaderv->setSectionsClickable(true);
	list_tree->setDragEnabled(true);
	list_tree->setAcceptDrops(true);

	// sorting default
	if (this->state.dnd)
		visualReindexList();
	else
		this->state.reindex = true;
	this->state.changed = true;

	updateConnectors();
	updateCounters();
}

void tab::updateBouquetsIndex()
{
	debug("updateBouquetsIndex()");

	int i = 0, y;
	int count = bouquets_tree->topLevelItemCount();
	vector<pair<int, string>> bss;
	vector<pair<int, string>> ubs;
	unordered_map<string, vector<string>> index;

	while (i != count)
	{
		QTreeWidgetItem* parent = bouquets_tree->topLevelItem(i);
		QVariantMap tdata = parent->data(0, Qt::UserRole).toMap();
		string pname = tdata["id"].toString().toStdString();
		bss.emplace_back(pair (i, pname)); //C++17
		y = 0;

		if (parent->childCount())
		{
			int childs = parent->childCount();
			while (y != childs)
			{
				QTreeWidgetItem* item = parent->child(y);
				QVariantMap tdata = item->data(0, Qt::UserRole).toMap();
				string bname = tdata["id"].toString().toStdString();
				ubs.emplace_back(pair (i, bname)); //C++17
				index[pname].emplace_back(bname);
				y++;
			}
		}
		i++;
	}
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

void tab::updateListIndex()
{
	if (! this->state.changed)
		return;

	int i = 0, idx = 0;
	int count = list_tree->topLevelItemCount();
	string curr_chlist = this->state.curr;
	dbih->index[curr_chlist].clear();

	debug("updateListIndex()", "curr_chlist", curr_chlist);

	int sort_col = list_tree->sortColumn();
	list_tree->sortItems(0, Qt::AscendingOrder);

	while (i != count)
	{
		QTreeWidgetItem* item = list_tree->topLevelItem(i);
		string chid = item->data(ITEM_DATA_ROLE::chid, Qt::UserRole).toString().toStdString();
		bool marker = item->data(ITEM_DATA_ROLE::marker, Qt::UserRole).toBool();
		idx = marker ? 0 : i + 1;
		dbih->index[curr_chlist].emplace_back(pair (idx, chid)); //C++17
		i++;
	}

	list_tree->sortItems(this->state.sort.first, this->state.sort.second);
	lheaderv->setSortIndicator(sort_col, this->state.sort.second);

	this->state.changed = false;
}

void tab::updateConnectors()
{
	debug("updateConnectors()");

	if (bouquets_tree->topLevelItemCount())
	{
		//TODO connect to QScrollArea Event
		/*if (bouquets_tree->verticalScrollBar()->isVisible())
		{*/
			gid->update(gui::TabBouquetsFind, true);
			this->action.bouquets_search->setEnabled(true);
		/*}
		else
		{
			gid->update(gui::TabBouquetsFind, false);
			this->action.bouquets_search->setEnabled(false);
		}*/
	}
	else
	{
		gid->update(gui::TabBouquetsFind, false);
		this->action.bouquets_search->setDisabled(true);
	}

	if (list_tree->topLevelItemCount())
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

	gid->update(gui::TabBouquetsFindNext, false);	
	gid->update(gui::TabListFindNext, false);
	gid->update(gui::TabListFindPrev, false);
	gid->update(gui::TabListFindAll, false);

	if (dbih->tuners.count(e2db::YTYPE::sat))
		gid->update(gui::ToolsTunersetsSat, true);
	if (dbih->tuners.count(e2db::YTYPE::terrestrial))
		gid->update(gui::ToolsTunersetsTerrestrial, true);
	if (dbih->tuners.count(e2db::YTYPE::cable))
		gid->update(gui::ToolsTunersetsCable, true);
	if (dbih->tuners.count(e2db::YTYPE::atsc))
		gid->update(gui::ToolsTunersetsAtsc, true);

	this->state.gxe = gid->getActionFlags();
}

void tab::updateCounters(bool current)
{
	debug("updateCounters()");

	int counters[5] = {-1, -1, -1, -1, -1};

	if (current)
	{
		string curr_chlist = this->state.curr;
		counters[gui::COUNTER::current] = dbih->index[curr_chlist].size();
	}
	else
	{
		counters[gui::COUNTER::data] = dbih->index["chs:0"].size();
		counters[gui::COUNTER::tv] = dbih->index["chs:1"].size();
		counters[gui::COUNTER::radio] = dbih->index["chs:2"].size();
		counters[gui::COUNTER::all] = dbih->index["chs"].size();
	}

	gid->setStatus(counters);
}

void tab::updateRefBox()
{
	debug("updateRefBox()");

	QList<QTreeWidgetItem*> selected = list_tree->selectedItems();
	
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
			string curr_chlist = this->state.curr;
			e2db::channel_reference chref = dbih->userbouquets[curr_chlist].channels[chid];
			string crefid = dbih->get_reference_id(chref);
			refid = QString::fromStdString(crefid);

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
			ubls = "<p style=\"line-height: 150%\">" + ubl.join("<br>") + "</p>";

			QStringList bsl;
			for (auto & x : bss)
			{
				if (dbih->bouquets.count(x.first))
					bsl.append(QString::fromStdString(dbih->bouquets[x.first].nname));
			}
			bsls = "<p style=\"line-height: 150%\">" + bsl.join("<br>") + "</p>";
		}
		// services tree
		else
		{
			string crefid = dbih->get_reference_id(chid);
			refid = QString::fromStdString(crefid);
			bsls = ubls = "< >";
		}
		if (dbih->db.services.count(chid))
		{
			e2db::service ch = dbih->db.services[chid];
			e2db::transponder tx = dbih->db.transponders[ch.txid];
			string ptxp, psys, ppos;

			ssid = QString::fromStdString(to_string(ch.ssid));

			switch (tx.ttype)
			{
				case 's':
					ptxp = to_string(tx.freq) + '/' + e2db::SAT_POL[tx.pol] + '/' + to_string(tx.sr);
				break;
				case 't':
					ptxp = to_string(tx.freq) + '/' + e2db::TER_MOD[tx.tmod] + '/' + e2db::TER_BAND[tx.band];
				break;
				case 'c':
					ptxp = to_string(tx.freq) + '/' + e2db::CAB_MOD[tx.cmod] + '/' + to_string(tx.sr);
				break;
				case 'a':
					ptxp = to_string(tx.freq);
				break;
			}
			txp = QString::fromStdString(ptxp);

			switch (tx.ttype) {
				case 's':
					psys = tx.sys != -1 ? e2db::SAT_SYS[tx.sys] : "DVB-S";
				break;
				case 't':
					psys = "DVB-T";
				break;
				case 'c':
					psys = "DVB-C";
				break;
				case 'a':
					psys = "ATSC";
				break;
			}
			if (tx.ttype == 's')
			{
				if (dbih->tuners_pos.count(tx.pos))
				{
					string tnid = dbih->tuners_pos.at(tx.pos);
					e2db::tunersets_table tn = dbih->tuners[0].tables[tnid];
					ppos = tn.name;
				}

				char cposdeg[6];
				// %3d.%1d%C
				std::sprintf(cposdeg, "%.1f", float (std::abs (tx.pos)) / 10);
				ppos += ' ' + (string (cposdeg) + (tx.pos > 0 ? 'E' : 'W'));
			}

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

void tab::showBouquetEditContextMenu(QPoint &pos)
{
	debug("showBouquetEditContextMenu()");

	QMenu* bouquet_edit = new QMenu;
	QAction* bouquet_export = new QAction("Export");
	// bouquet: tv | radio
	if (this->state.ti != -1)
	{
		bouquet_export->connect(bouquet_export, &QAction::triggered, [=]() { this->exportFile(); });
	}
	// userbouquet
	else
	{
		bouquet_edit->addAction("Edit Userbouquet", [=]() { this->editUserbouquet(); });
		bouquet_edit->addSeparator();
		bouquet_edit->addAction("Delete", [=]() { this->bouquetItemDelete(); });
		bouquet_export->connect(bouquet_export, &QAction::triggered, [=]() { this->exportFile(); });
	}
	bouquet_edit->addSeparator();
	bouquet_edit->addAction(bouquet_export);

	bouquet_edit->exec(bouquets_tree->mapToGlobal(pos));
}

void tab::showListEditContextMenu(QPoint &pos)
{
	debug("showListEditContextMenu()");

	int gflags = gid->getActionFlags();

	QMenu* list_edit = new QMenu;
	list_edit->addAction("Edit Service", [=]() { this->editService(); })->setDisabled(gflags & gui::TabListEditService ? false : true);
	list_edit->addSeparator();
	list_edit->addAction("Cut", [=]() { this->listItemCut(); })->setDisabled(gflags & gui::TabListCut ? false : true);
	list_edit->addAction("Copy", [=]() { this->listItemCopy(); })->setDisabled(gflags & gui::TabListCopy ? false : true);
	list_edit->addAction("Paste", [=]() { this->listItemPaste(); })->setDisabled(gflags & gui::TabListPaste ? false : true);
	list_edit->addSeparator();
	list_edit->addAction("Delete", [=]() { this->listItemDelete(); })->setDisabled(gflags & gui::TabListDelete ? false : true);

	list_edit->exec(list_tree->mapToGlobal(pos));
}


void tab::setTabId(int ttid)
{
	debug("setTabId()", "ttid", ttid);

	this->ttid = ttid;
}

void tab::tabSwitched()
{
	gid->setActionFlags(this->state.gxe);
	updateCounters();
	updateCounters(true);
}

void tab::tabChangeName(string filename)
{
	debug("tabChangeName()");

	if (ttid != -1)
		gid->tabChangeName(ttid, filename);
}

void tab::initialize()
{
	debug("initialize()");

	this->state.nwwr = true;
	this->state.ovwr = false;
	this->state.dnd = true;
	this->state.changed = false;
	this->state.reindex = false;
	this->state.refbox = list_reference->isVisible();
	this->state.tc = 0;
	this->state.ti = 0;
	this->state.curr = "";
	this->state.sort = pair (-1, Qt::AscendingOrder); //C++17
	this->state.tunersets = false;
	this->state.ty = -1;

	bouquets_tree->clear();
	bouquets_tree->setDragEnabled(false);
	bouquets_tree->setAcceptDrops(false);
	bouquets_tree->reset();

	list_tree->clear();
	list_tree->setDragEnabled(false);
	list_tree->setAcceptDrops(false);
	list_tree->reset();
	lheaderv->setSortIndicatorShown(false);
	lheaderv->setSectionsClickable(false);
	lheaderv->setSortIndicator(0, Qt::AscendingOrder);
	cache.clear();

	this->action.list_addch->setDisabled(true);
	this->action.list_newch->setEnabled(true);
	this->action.list_dnd->setDisabled(true);

	gid->resetStatus();
}

void tab::profileComboChanged(int index)
{
	debug("profileComboChanged()", "index", index);

	gid->sets->setValue("profile/selected", index);
}

bool tab::ftpHandle()
{
	debug("ftpHandle()");

	if (ftph == nullptr)
		ftph = new ftpcom(this->log->log);

	if (ftph->connect())
		return true;
	else
		QMessageBox::critical(nullptr, NULL, "Cannot connect to FTP Server!");

	return false;
}

void tab::ftpConnect()
{
	debug("ftpConnect()");

	if (ftph != nullptr)
	{
		ftph->disconnect();
		delete ftph;
		ftph = nullptr;
	}
	if (ftpHandle())
		QMessageBox::information(nullptr, NULL, "Successfully connected!");
}

void tab::ftpUpload()
{
	debug("ftpUpload()");
	
	if (ftpHandle())
	{
		unordered_map<string, e2se_ftpcom::ftpcom_file> files = dbih->get_output();

		if (files.empty())
			return;
		for (auto & x : files)
			debug("ftpUpload()", "file", x.first + " | " + to_string(x.second.size()));

		ftph->put_files(files);
		QMessageBox::information(nullptr, NULL, "Uploaded");

		if (ftph->cmd_ifreload() || ftph->cmd_tnreload())
			QMessageBox::information(nullptr, NULL, "STB reloaded");
	}
}

void tab::ftpDownload()
{
	debug("ftpDownload()");

	if (ftpHandle())
	{
		unordered_map<string, e2se_ftpcom::ftpcom_file> files = ftph->get_files();

		if (files.empty())
			return;
		for (auto & x : files)
			debug("ftpDownload()", "file", x.first + " | " + to_string(x.second.size()));

		this->updateListIndex();
		this->updateBouquetsIndex();
		dbih->merge(files);
		initialize();
		load();
	}
}

void tab::loadSeeds()
{
	if (gid->sets->contains("application/seeds"))
	{
		readFile(gid->sets->value("application/seeds").toString().toStdString());
	}
	else
	{
		gid->sets->setValue("application/seeds", "");
		QMessageBox::information(cwid, NULL, "For debugging purpose, set application.seeds absolute path under Settings > Advanced tab, then restart software.");
	}
}

}
