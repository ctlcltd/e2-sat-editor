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
#include <cstdio>

#include <QtGlobal>
#include <QGuiApplication>
#include <QTimer>
#include <QList>
#include <QStyle>
#include <QMessageBox>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QSplitter>
#include <QGroupBox>
#include <QToolBar>
#include <QMenu>
#include <QPushButton>
#include <QComboBox>
#include <QClipboard>
#include <QMimeData>

#include "theme.h"
#include "tab.h"
#include "gui.h"
#include "editBouquet.h"
#include "editService.h"
#include "channelBook.h"
#include "ftpcom_gui.h"

using std::to_string, std::sort;
using namespace e2se;

namespace e2se_gui
{

tab::tab(gui* gid, QWidget* wid)
{
	this->log = new logger("tab");
	debug("tab()");

	this->gid = gid;
	this->cwid = wid;
	this->widget = new QWidget;
	widget->setStyleSheet("QGroupBox { spacing: 0; padding: 20px 0 0 0; border: 0 } QGroupBox::title { margin: 0 12px }");

	QGridLayout* frm = new QGridLayout(widget);

	QHBoxLayout* top = new QHBoxLayout;
	QGridLayout* container = new QGridLayout;
	QHBoxLayout* bottom = new QHBoxLayout;

	//TODO bouquets_box and scrollbar in GTK+
	QSplitter* splitterc = new QSplitter;

	QVBoxLayout* side_box = new QVBoxLayout;
	QVBoxLayout* list_box = new QVBoxLayout;

	QWidget* side = new QWidget;
	QVBoxLayout* services_box = new QVBoxLayout;
	QVBoxLayout* bouquets_box = new QVBoxLayout;

	QGroupBox* services = new QGroupBox("Services");
	QGroupBox* bouquets = new QGroupBox("Bouquets");
	QGroupBox* channels = new QGroupBox("Channels");

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
	services_tree->setStyleSheet("QTreeWidget { background: transparent } ::item { padding: 6px auto }");
	bouquets_tree->setStyleSheet("QTreeWidget { background: transparent } ::item { padding: 6px auto }");
	list_tree->setStyleSheet("::item { padding: 6px auto }");

	services_tree->setHeaderHidden(true);
	services_tree->setUniformRowHeights(true);
	bouquets_tree->setHeaderHidden(true);
	bouquets_tree->setUniformRowHeights(true);
	list_tree->setUniformRowHeights(true);

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

	QTreeWidgetItem* lheader_item = new QTreeWidgetItem({"", "Index", "Name", "CHID", "TXID", "Type", "Provider", "Frequency", "Polarization", "Symbol Rate", "FEC", "SAT", "System"});

	int col = 0;
	list_tree->setHeaderItem(lheader_item);
	list_tree->setColumnHidden(col++, true);
	list_tree->setColumnWidth(col++, 75);		// Index
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

	if (gid->sets->value("application/debug", true).toBool())
	{
		QWidget* bottom_spacer = new QWidget;
		bottom_spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

		bottom_toolbar->addAction("§ Load seeds", [=]() { this->loadSeeds(); });
		bottom_toolbar->addAction("§ Reset", [=]() { this->newFile(); tabChangeName(); });
		bottom_toolbar->addWidget(bottom_spacer);
	}

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
	this->action.list_dnd->setDisabled(true);
	this->action.list_dnd->connect(this->action.list_dnd, &QPushButton::pressed, [=]() { this->reharmDnD(); });
	QWidget* list_ats_spacer = new QWidget;
	list_ats_spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

	bouquets_ats->addAction(theme::icon("add"), "New Bouquet", [=]() { this->addUserbouquet(); });
	this->action.list_addch = list_ats->addAction(theme::icon("add"), "Add Channel", [=]() { this->addChannel(); });
	this->action.list_newch = list_ats->addAction(theme::icon("add"), "New Service", [=]() { this->addService(); });
	list_ats->addWidget(list_ats_spacer);
	list_ats->addWidget(this->action.list_dnd);
	this->action.list_addch->setDisabled(true);

	//TODO reindex userbouquets before saving
	this->bouquets_evth = new BouquetsEventHandler;
	this->list_evth = new ListEventHandler;
	this->list_evto = new ListEventObserver;
	services_tree->connect(services_tree, &QTreeWidget::itemPressed, [=](QTreeWidgetItem* item) { this->treeSwitched(services_tree, item); });
	services_tree->connect(services_tree, &QTreeWidget::currentItemChanged, [=](QTreeWidgetItem* current) { this->servicesItemChanged(current); });
	bouquets_evth->setEventCallback([=]() { list_tree->scrollToBottom(); this->visualReindexList(); });
	bouquets_tree->viewport()->installEventFilter(bouquets_evth);
	bouquets_tree->connect(bouquets_tree, &QTreeWidget::itemPressed, [=](QTreeWidgetItem* item) { this->treeSwitched(bouquets_tree, item); });
	bouquets_tree->connect(bouquets_tree, &QTreeWidget::currentItemChanged, [=](QTreeWidgetItem* current) { this->bouquetsItemChanged(current); });
	list_tree->installEventFilter(list_evto);
	list_tree->viewport()->installEventFilter(list_evth);
	list_tree->connect(list_tree, &QTreeWidget::currentItemChanged, [=]() { this->listItemChanged(); });
	list_tree->connect(list_tree, &QTreeWidget::itemDoubleClicked, [=]() { this->editService(); });

	top->addWidget(top_toolbar);
	bottom->addWidget(bottom_toolbar);

	services_box->addWidget(services_tree);
	services->setLayout(services_box);
	//TODO FIX
	services_tree->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Minimum);

	bouquets_box->addWidget(bouquets_tree);
	bouquets_box->addWidget(bouquets_ats);
	bouquets->setLayout(bouquets_box);

	side_box->addWidget(services);
	side_box->addWidget(bouquets);
	side_box->setContentsMargins(0, 0, 0, 0);
	side->setLayout(side_box);

	list_layout->addWidget(list_tree);
	list_layout->setContentsMargins(3, 3, 3, 3);
	list_wrap->setLayout(list_layout);

	list_box->addWidget(list_wrap);
	list_box->addWidget(list_ats);
	channels->setLayout(list_box);

	side->setMinimumWidth(240);
	channels->setMinimumWidth(520);

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

	initialize();

	if (this->dbih != nullptr)
		delete this->dbih;

	this->dbih = new e2db;

	load();
}

void tab::openFile()
{
	debug("openFile()");

	string dirname = gid->openFileDialog();

	if (! dirname.empty())
		readFile(dirname);
}

void tab::saveFile(bool saveas)
{
	debug("saveFile()", "saveas", to_string(saveas));

	QMessageBox dial = QMessageBox();
	string dirname;
	bool overwrite = ! saveas && (! this->state.nwwr || this->state.ovwr);

	if (overwrite)
	{
		this->updateListIndex();
		dirname = this->filename;
		dial.setText("Files will be overwritten.");
		dial.exec();
	}
	else
	{
		dirname = gid->saveFileDialog(this->filename);
	}

	if (! dirname.empty())
	{
		debug("saveFile()", "overwrite", to_string(overwrite));
		debug("saveFile()", "filename", filename);

		QGuiApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
		bool wr = dbih->write(dirname, overwrite);
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

void tab::importFile()
{
	debug("importFile()");

	vector<string> filenames;

	filenames = gid->importFileDialog();
}

void tab::exportFile()
{
	debug("exportFile()");

	string filename;
	string dirname = gid->exportFileDialog(filename);
}

void tab::addUserbouquet()
{
	debug("addUserbouquet()");

	string bname;
	e2se_gui::editBouquet* add = new e2se_gui::editBouquet(dbih, this->state.ti);
	add->display(cwid);
	bname = add->getEditID();
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

	dbih->updateUserbouquetIndexes();
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

	debug("editService()", "bname", bname);

	e2se_gui::editBouquet* edit = new e2se_gui::editBouquet(dbih, this->state.ti);
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

	dbih->updateUserbouquetIndexes();
}

void tab::addChannel()
{
	debug("addChannel()");

	e2se_gui::channelBook* cb = new e2se_gui::channelBook(dbih);
	string curr_chlist = this->state.curr;
	QDialog* dial = new QDialog(cwid);
	dial->setMinimumSize(760, 420);
	dial->setWindowTitle("Add Channel");
	//TODO FIX SEGFAULT
	// dial->connect(dial, &QDialog::finished, [=]() { delete dial; delete cb; });

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

	e2se_gui::editService* add = new e2se_gui::editService(dbih);
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
		e2se_gui::editService* edit = new e2se_gui::editService(dbih);
		edit->setEditID(chid);
		edit->display(cwid);
		nw_chid = edit->getEditID(); //TODO returned after dial.exec()
		edit->destroy();

		debug("editService()", "nw_chid", nw_chid);

		QStringList entry = dbih->entries.services[nw_chid];
		entry.prepend(item->text(1));
		entry.prepend(item->text(0));
		for (int i = 0; i < entry.count(); i++)
			item->setText(i, entry[i]);
		item->setData(2, Qt::UserRole, QString::fromStdString(nw_chid)); // data: chid

		dbih->updateUserbouquetIndexes(chid, nw_chid);
	}
}

bool tab::readFile(string filename)
{
	debug("readFile()", "filename", filename);

	if (filename.empty())
		return false;

	initialize();

	if (this->dbih != nullptr)
		delete this->dbih;

	this->dbih = new e2db;

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

	sort(dbih->gindex["bss"].begin(), dbih->gindex["bss"].end());
	unordered_map<string, QTreeWidgetItem*> bgroups;

	for (auto & bsi : dbih->gindex["bss"])
	{
		debug("load()", "bouquet", bsi.second);
		e2db::bouquet gboq = dbih->bouquets[bsi.second];
		QString bname = QString::fromStdString(bsi.second);
		QString name = QString::fromStdString(gboq.nname.empty() ? gboq.name : gboq.nname);

		QTreeWidgetItem* pgroup = new QTreeWidgetItem();
		pgroup->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
		QMap<QString, QVariant> tdata; //TODO
		tdata["id"] = bname;
		pgroup->setData(0, Qt::UserRole, QVariant (tdata));
		pgroup->setText(0, name);
		bouquets_tree->addTopLevelItem(pgroup);
		bouquets_tree->expandItem(pgroup);

		for (string & ubname : gboq.userbouquets)
			bgroups[ubname] = pgroup;
	}
	for (auto & ubi : dbih->gindex["ubs"])
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
	setCounters();
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
		return;
	if (selected != NULL)
	{
		QVariantMap tdata = selected->data(0, Qt::UserRole).toMap();
		QString qcurr_bouquet = tdata["id"].toString();
		curr_chlist = qcurr_bouquet.toStdString();
		this->state.curr = curr_chlist;
	}

	debug("populate()", "curr_chlist", curr_chlist);

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
		for (auto & ch : dbih->gindex[curr_chlist])
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
				e2db::channel_reference chref = dbih->userbouquets[curr_chlist].channels[ch.second];

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
					entry = QStringList({x, "", "", chid, "", "ERROR"});
					// idx = 0; //Qt5
					error("populate()", "chid", ch.second);
					//TEST
				}
			}

			QTreeWidgetItem* item = new QTreeWidgetItem(entry);
			item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsDragEnabled | Qt::ItemNeverHasChildren);
			item->setData(0, Qt::UserRole, idx);    // data: Index
			item->setData(1, Qt::UserRole, marker); // data: marker flag
			item->setData(2, Qt::UserRole, chid);   // data: chid
			cache[curr_chlist].append(item);
		}
	}

	list_tree->addTopLevelItems(cache[curr_chlist]);

	// sorting
	if (this->state.sort.first != -1)
	{
		list_tree->sortByColumn(this->state.sort.first, this->state.sort.second);

		// sorting column 0|desc
		if (this->state.sort.first == 0 && this->state.sort.second == Qt::DescendingOrder)
			lheaderv->setSortIndicator(1, this->state.sort.second);
	}
	// non-sorting
	else if (this->state.dnd)
	{
		list_tree->setDragEnabled(true);
		list_tree->setAcceptDrops(true);
	}
	lheaderv->setSectionsClickable(true);

	setCounters(true);
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

		list_tree->clearSelection();
		list_tree->scrollToTop();
	}

	updateListIndex();
	populate(services_tree);
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
		}

		list_tree->clearSelection();
		list_tree->scrollToTop();
	}

	updateListIndex();
	populate(bouquets_tree);
}

void tab::listItemChanged()
{
	if (! list_evto->isChanged()) return;

	debug("listItemChanged()");

	QTimer::singleShot(0, [=]() { this->visualReindexList(); });
	this->state.changed = true;
}

void tab::visualReindexList()
{
	debug("visualReindexList()");

	int i = 0, y = 0, idx = 0;
	int maxs = list_tree->topLevelItemCount();

	while (i != maxs)
	{
		char ci[7];
		std::sprintf(ci, "%06d", i + 1);
		QString x = QString::fromStdString(ci);
		QTreeWidgetItem* item = list_tree->topLevelItem(i);
		bool marker = item->data(1, Qt::UserRole).toBool();
		if (marker)
		{
			idx = 0;
		}
		else
		{
			y++;
			idx = y;
		}
		item->setText(0, x);
		if (! marker)
			item->setText(1, QString::fromStdString(to_string(idx)));
		i++;
	}
}

void tab::trickySortByColumn(int column)
{
	debug("trickySortByColumn()", "column", to_string(column));

	Qt::SortOrder order = lheaderv->sortIndicatorOrder();
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
		list_tree->sortByColumn(column, order);
		lheaderv->setSortIndicator(1, order);
		allowDnD();

		this->action.list_dnd->setDisabled(true);

		// default column 0|asc
		if (order == Qt::AscendingOrder)
			lheaderv->setSortIndicatorShown(false);
		else
			lheaderv->setSortIndicatorShown(true);

		if (this->state.changed)
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
	list_tree->sortByColumn(0, Qt::AscendingOrder);
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
		return;

	for (auto & item : selected)
	{
		QVariantMap tdata = item->data(0, Qt::UserRole).toMap();
		QString qbname = tdata["id"].toString();
		string bname = qbname.toStdString();
		dbih->removeUserbouquet(bname);
		QTreeWidgetItem* parent = item->parent();
		parent->removeChild(item);
	}

	this->state.changed = true;
	dbih->updateUserbouquetIndexes();
}

void tab::listItemCut()
{
	debug("listItemCut()");

	listItemCopy(true);
}

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
			data.append(item->data(i, Qt::DisplayRole).toString());
		text.append(data.join(",")); // CSV
	}
	clipboard->setText(text.join("\n")); // CSV

	if (cut)
		listItemDelete();
}

void tab::listItemPaste()
{
	debug("listItemPaste()");

	QClipboard* clipboard = QGuiApplication::clipboard();
	const QMimeData* mimeData = clipboard->mimeData();
	vector<QString> items;
	string curr_chlist = this->state.curr;

	if (mimeData->hasText())
	{
		QStringList list = clipboard->text().split("\n");
		for (QString & data : list)
		{
			//TODO validate
			items.emplace_back(data.split(",")[2]);
		}
	}
	if (! items.empty())
		putChannels(items);
}

void tab::listItemDelete()
{
	debug("listItemDelete()");

	QList<QTreeWidgetItem*> selected = list_tree->selectedItems();
	
	if (selected.empty())
		return;

	lheaderv->setSectionsClickable(false);
	list_tree->setDragEnabled(false);
	list_tree->setAcceptDrops(false);

	for (auto & item : selected)
	{
		int i = list_tree->indexOfTopLevelItem(item);
		list_tree->takeTopLevelItem(i);
	}

	lheaderv->setSectionsClickable(true);
	list_tree->setDragEnabled(true);
	list_tree->setAcceptDrops(true);
	this->state.changed = true;

	// non-sorting
	if (this->state.dnd)
	{
		updateListIndex();
		visualReindexList();
	}

	setCounters();
}

void tab::listItemSelectAll()
{
	debug("listItemSelectAll()");

	list_tree->selectAll();
}

void tab::listItemAction(int action)
{
	debug("listItemAction()", "action", to_string(action));

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
	int i = 0;
	int idx;
	QTreeWidgetItem* current = list_tree->currentItem();
	QTreeWidgetItem* parent = list_tree->invisibleRootItem();
	i = current != nullptr ? parent->indexOfChild(current) : list_tree->topLevelItemCount();
	idx = i + 1;

	for (QString & w : channels)
	{
		string chid = w.toStdString();
		char ci[7];
		std::sprintf(ci, "%06d", i++);
		QString x = QString::fromStdString(ci);
		QString idx = QString::fromStdString(to_string(i));
		QStringList entry;
		bool marker = false;
		if (dbih->db.services.count(chid))
		{
			entry = dbih->entries.services[chid];
			entry.prepend(idx);
			entry.prepend(x);
		}
		else
		{
			string chlist;

			for (auto & q : dbih->gindex["mks"])
			{
				if (q.second == chid)
				{
					for (auto & u : dbih->gindex["ubs"])
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
				e2db::channel_reference chref = dbih->userbouquets[chlist].channels[chid];
				marker = true;
				entry = dbih->entryMarker(chref);
				idx = entry[1];
				entry.prepend(x);
			}
		}
		QTreeWidgetItem* item = new QTreeWidgetItem(entry);
		item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsDragEnabled | Qt::ItemNeverHasChildren);
		item->setData(0, Qt::UserRole, idx);    // data: Index
		item->setData(1, Qt::UserRole, marker); // data: marker flag
		item->setData(2, Qt::UserRole, w);      // data: chid
		clist.append(item);
	}

	if (current == nullptr)
		list_tree->addTopLevelItems(clist);
	else
		list_tree->insertTopLevelItems(idx, clist);

	lheaderv->setSectionsClickable(true);
	list_tree->setDragEnabled(true);
	list_tree->setAcceptDrops(true);
	this->state.changed = true;

	// non-sorting
	if (this->state.dnd)
	{
		updateListIndex();
		visualReindexList();
	}

	setCounters();
}

void tab::updateListIndex()
{
	if (! this->state.changed) return;

	int i = 0, y = 0, idx = 0;
	int count = list_tree->topLevelItemCount();
	string curr_chlist = this->state.curr;
	dbih->gindex[curr_chlist].clear();

	debug("updateListIndex()", "curr_chlist", curr_chlist);

	while (i != count)
	{
		QTreeWidgetItem* item = list_tree->topLevelItem(i);
		string chid = item->data(2, Qt::UserRole).toString().toStdString();
		bool marker = item->data(1, Qt::UserRole).toBool();
		if (marker)
		{
			idx = 0;
		}
		else
		{
			y++;
			idx = y;
		}
		dbih->gindex[curr_chlist].emplace_back(pair (idx, chid)); //C++17
		i++;
	}

	this->state.changed = false;
}

void tab::showBouquetEditContextMenu(QPoint &pos)
{
	debug("showBouquetEditContextMenu()");

	// bouquet: tv | radio
	if (this->state.ti != -1)
		return;

	QMenu* bouquet_edit = new QMenu;
	bouquet_edit->addAction("Edit Userbouquet", [=]() { this->editUserbouquet(); });
	bouquet_edit->addSeparator();
	bouquet_edit->addAction("Delete", [=]() { this->bouquetItemDelete(); });

	bouquet_edit->exec(bouquets_tree->mapToGlobal(pos));
}

void tab::showListEditContextMenu(QPoint &pos)
{
	debug("showListEditContextMenu()");

	QMenu* list_edit = new QMenu;
	list_edit->addAction("Edit Service", [=]() { this->editService(); });
	list_edit->addSeparator();
	list_edit->addAction("Cut", [=]() { this->listItemCut(); });
	list_edit->addAction("Copy", [=]() { this->listItemCopy(); });
	list_edit->addAction("Paste", [=]() { this->listItemPaste(); });
	list_edit->addSeparator();
	list_edit->addAction("Delete", [=]() { this->listItemDelete(); });

	list_edit->exec(list_tree->mapToGlobal(pos));
}

void tab::setCounters(bool channels)
{
	debug("setCounters()");

	int counters[5] = {-1, -1, -1, -1, -1};

	if (channels)
	{
		string curr_chlist = this->state.curr;
		counters[4] = dbih->gindex[curr_chlist].size();
	}
	else
	{
		counters[0] = dbih->gindex["chs:0"].size(); // data
		counters[1] = dbih->gindex["chs:1"].size(); // tv
		counters[2] = dbih->gindex["chs:2"].size(); // radio
		counters[3] = dbih->gindex["chs"].size();   // all
	}

	gid->setStatus(counters);
}

void tab::setTabId(int ttid)
{
	debug("setTabId()", "ttid", to_string(ttid));

	this->ttid = ttid;
}

void tab::tabSwitched()
{
	setCounters();
	setCounters(true);
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
	this->state.changed = false;
	this->state.dnd = true;
	this->state.sort = pair (-1, Qt::AscendingOrder); //C++17

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
	debug("profileComboChanged()", "index", to_string(index));

	gid->sets->setValue("profile/selected", index);
}

bool tab::ftpHandle()
{
	debug("ftpHandle()");

	if (ftph == nullptr)
		ftph = new ftpcom;

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

		if (ftph->cmd_ifreload())
			QMessageBox::information(nullptr, NULL, "STB reloaded");*/
		else if (ftph->cmd_tnreload())
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
