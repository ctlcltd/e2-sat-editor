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

#include <cstdio>
#include <cmath>
#include <algorithm>
#include <filesystem>

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
#include "dataHandler.h"
#include "ftpcom_gui.h"
#include "tunersetsView.h"
#include "channelBookView.h"
#include "editBouquet.h"
#include "editService.h"
#include "editMarker.h"
#include "printable.h"

using std::to_string, std::sort;
using namespace e2se;

namespace e2se_gui
{

tab::tab(gui* gid, QWidget* cwid, e2se::logger::session* log)
{
	this->log = new logger(log, "tab");
	debug("tab()");

	this->gid = gid;
	this->cwid = cwid;
	this->widget = new QWidget;
}

tab::~tab()
{
	if (! this->child)
	{
		delete this->data;
	}
}

bool tab::isChild()
{
	return this->child;
}

bool tab::hasChildren()
{
	return ! this->childs.empty();
}

vector<tab*> tab::children()
{
	return this->childs;
}

void tab::addChild(tab* child)
{
	debug("addChild()");

	this->childs.emplace_back(child);
}

void tab::removeChild(tab* child)
{
	debug("removeChild()");

	vector<tab*>::iterator pos;
	for (auto it = childs.begin(); it != childs.end(); it++)
	{
		if (*it == child)
		{
			pos = it;
			break;
		}
	}
	if (pos != childs.end())
	{
		childs.erase(pos);
	}
}

int tab::getTabId()
{
	return this->ttid;
}

void tab::setTabId(int ttid)
{
	this->ttid = ttid;
}

string tab::getTabName()
{
	return this->ttname;
}

void tab::setTabName(string ttname)
{
	this->ttname = ttname;
}

gui::TAB_VIEW tab::getTabView()
{
	return this->ttv;
}

void tab::tabSwitched()
{
	debug("tabSwitched()");

	retrieveFlags();
	view->updateStatus();
	view->updateStatus(true);
}

void tab::tabChangeName(string filename)
{
	debug("tabChangeName()");

	if (! filename.empty())
		filename = std::filesystem::path(filename).filename().u8string(); //C++17

	gid->tabChangeName(ttid, filename);

	for (auto & child : childs)
	{
		int ttid = child->getTabId();
		gid->tabChangeName(ttid, filename);
	}
}

void tab::setFlag(gui::GUI_CXE bit, bool flag)
{
	gid->update(bit, flag);
}

void tab::setFlag(gui::GUI_CXE bit)
{
	gid->update(bit);
}

bool tab::getFlag(gui::GUI_CXE bit)
{
	return gid->getFlag(bit);
}

void tab::storeFlags()
{
	this->gxe = gid->getFlags();
}

void tab::retrieveFlags()
{
	gid->setFlags(this->gxe);
}

void tab::setStatus(gui::STATUS status)
{
	status.view = this->ttv;
	gid->setStatus(status);
}

void tab::resetStatus()
{
	gid->resetStatus();
}

void tab::viewMain()
{
	debug("viewMain()");

	this->data = new dataHandler(this->log->log);
	this->tools = new e2se_gui::tools(this, this->gid, this->cwid, this->data, this->log->log);
	this->main = new mainView(this, this->cwid, this->data, this->log->log);
	this->view = this->main;

	this->ttv = gui::TAB_VIEW::main;

	layout();
	
	this->root->addWidget(main->widget, 0, 0, 1, 1);

	newFile();
}

void tab::viewTunersets(tab* parent, int ytype)
{
	debug("viewTunersets()");

	parent->addChild(this);

	this->parent = parent;
	this->child = true;

	this->data = parent->data;
	this->tools = parent->tools;
	this->main = parent->main;
	this->view = new tunersetsView(this, this->cwid, this->data, ytype, this->log->log);

	this->ttv = gui::TAB_VIEW::tunersets;
	this->ty = ytype;

	layout();
	
	this->root->addWidget(view->widget, 0, 0, 1, 1);

	load();
}

void tab::viewChannelBook(tab* parent)
{
	debug("viewChannelBook()");

	parent->addChild(this);

	this->parent = parent;
	this->child = true;

	this->data = parent->data;
	this->tools = parent->tools;
	this->main = parent->main;
	this->view = new channelBookView(this, this->cwid, this->data, this->log->log);

	this->ttv = gui::TAB_VIEW::channelBook;

	layout();
	
	this->root->addWidget(view->widget, 0, 0, 1, 1);

	load();
}

void tab::load()
{
	debug("load()");

	view->load();

	//TODO FIX tab::removeChild
	for (auto & child : childs)
		child->view->load();
}

void tab::reset()
{
	debug("reset()");

	view->reset();

	//TODO FIX tab::removeChild
	for (auto & child : childs)
		child->view->reset();
}

void tab::layout()
{
	debug("layout()");

	widget->setStyleSheet("QGroupBox { spacing: 0; padding: 20px 0 0 0; border: 0 } QGroupBox::title { margin: 0 12px }");

	QGridLayout* frm = new QGridLayout(widget);

	QHBoxLayout* top = new QHBoxLayout;
	QGridLayout* container = new QGridLayout;
	QHBoxLayout* bottom = new QHBoxLayout;

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
		//TODO profile/selected and array index differs trouble
		profile_combo->addItem(gid->sets->value("profileName").toString(), i + 1);
	}
	gid->sets->endArray();
	profile_combo->setCurrentIndex(profile_sel);
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
	profile_combo->connect(profile_combo, &QComboBox::currentIndexChanged, [=](int index) { this->profileComboChanged(index); });
#else
	profile_combo->connect(profile_combo, QOverload<int>::of(&QComboBox::currentIndexChanged), [=](int index) { this->profileComboChanged(index); });
#endif

	top_toolbar->addAction(theme::icon("file-open"), "&Open", QKeySequence::Open, [=]() { this->openFile(); });
	top_toolbar->addAction(theme::icon("save"), "&Save", QKeySequence::Save, [=]() { this->saveFile(false); });
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

	if (gid->sets->value("application/debug", true).toBool())
	{
		bottom_toolbar->addSeparator();
		bottom_toolbar->addAction("§ Load seeds", [=]() { this->loadSeeds(); });
		bottom_toolbar->addAction("§ Reset", [=]() { this->newFile(); tabChangeName(); });

		//TODO
		/*if (this->child)
		{
			for (auto & action : bottom_toolbar->actions())
				action->setDisabled(true);
		}*/
	}
	bottom_toolbar->addWidget(bottom_spacer);

	top->addWidget(top_toolbar);
	bottom->addWidget(bottom_toolbar);
	container->setContentsMargins(8, 8, 8, 8);

	frm->setContentsMargins(0, 0, 0, 0);
	frm->addLayout(top, 0, 0);
	frm->addLayout(container, 1, 0);
	frm->addLayout(bottom, 2, 0);

	this->root = container;

	widget->setLayout(frm);
}

void tab::newFile()
{
	debug("newFile()");

	reset();

	this->data->newFile();

	for (auto & child : childs)
		child->reset();

	load();

	if (this->child)
	{
		parent->reset();
		parent->load();
	}
}

void tab::openFile()
{
	debug("openFile()");

	string path = gid->openFileDialog();

	if (! path.empty())
		readFile(path);
}

bool tab::readFile(string filename)
{
	debug("readFile()", "filename", filename);

	if (filename.empty())
		return false;

	reset();

	QGuiApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
	bool rr = this->data->readFile(filename);
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

	load();

	if (this->child)
	{
		parent->reset();
		parent->load();
	}

	return true;
}

void tab::saveFile(bool saveas)
{
	debug("saveFile()", "saveas", saveas);

	QMessageBox dial = QMessageBox();
	string path;
	bool overwrite = ! saveas && (! this->data->newfile || this->data->overwrite);

	if (overwrite)
	{
		this->updateChannelsIndex();
		this->updateBouquetsIndex();
		path = this->data->filename;
		dial.setText("Files will be overwritten.");
		dial.exec();
	}
	else
	{
		path = gid->saveFileDialog(this->data->filename);
	}

	if (! path.empty())
	{
		debug("saveFile()", "overwrite", overwrite);
		debug("saveFile()", "filename", this->data->filename);

		QGuiApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
		bool wr = this->data->writeFile(path, overwrite);
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

	gui::GUI_DPORTS gde;
	vector<string> paths;

	paths = gid->importFileDialog(gde);
	if (! paths.empty())
	{
		QGuiApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
		dbih->importFile(paths);
		QGuiApplication::restoreOverrideCursor();
		view->reset();
		view->load();
	}
}

void tab::exportFile()
{
	debug("exportFile()");

	gui::GUI_DPORTS gde;
	vector<string> paths;
	string filename;
	int flags = -1;

	// tunersets
	if (this->ttv == gui::TAB_VIEW::tunersets)
	{
		gde = gui::GUI_DPORTS::Tunersets;
		flags = e2db::FPORTS::single_tunersets;
		switch (this->ty)
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
	else if (main->state.tc == 0)
	{
		gde = gui::GUI_DPORTS::Services;
		flags = e2db::FPORTS::all_services;
		filename = "lamedb";
		paths.push_back(filename);
	}
	// bouquets
	else if (main->state.tc == 1)
	{
		int ti = -1;
		QList<QTreeWidgetItem*> selected = main->tree->selectedItems();

		if (selected.empty())
		{
			return;
		}
		for (auto & item : selected)
		{
			ti = main->tree->indexOfTopLevelItem(item);
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
			flags = e2db::FPORTS::single_bouquet_all;

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
			flags = e2db::FPORTS::single_userbouquet;
		}
	}
	if (paths.empty())
	{
		return;
	}

	bool overwrite = ! this->data->newfile || this->data->overwrite;

	if (overwrite)
	{
		this->updateChannelsIndex();
		this->updateBouquetsIndex();
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
			//TODO right-end trailing
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

	gui::GUI_DPORTS gde;
	vector<string> paths;
	string filename;
	int bit = -1;

	if (item == nullptr)
	{
		return;
	}
	// services
	else if (main->state.tc == 0)
	{
		gde = gui::GUI_DPORTS::Services;
		filename = "lamedb";
		paths.push_back(filename);
	}
	// bouquets
	else if (main->state.tc == 1)
	{
		int ti = main->tree->indexOfTopLevelItem(item);
		QVariantMap tdata = item->data(0, Qt::UserRole).toMap();
		QString qchlist = tdata["id"].toString();
		filename = qchlist.toStdString();
		paths.push_back(filename);

		// bouquet | userbouquets
		if (ti != -1)
		{
			gde = gui::GUI_DPORTS::Bouquets;
			bit = e2db::FPORTS::single_bouquet_all;

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
			bit = e2db::FPORTS::single_userbouquet;
		}
	}
	if (paths.empty())
	{
		return;
	}

	bool overwrite = ! this->data->newfile || this->data->overwrite;

	if (overwrite)
	{
		this->updateChannelsIndex();
		this->updateBouquetsIndex();
	}

	string path = gid->exportFileDialog(gde, filename, bit);

	if (! path.empty())
	{
		QMessageBox dial = QMessageBox();
		QGuiApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
		dbih->exportFile(bit, paths);
		QGuiApplication::restoreOverrideCursor();
		dial.setText("Saved!");
		dial.exec();
	}
}

void tab::printFile(bool all)
{
	debug("printFile()");

	printable* printer = new printable(dbih, this->log->log);

	// print all
	if (all)
	{
		printer->document_all();
	}
	// tunersets
	else if (this->ttv == gui::TAB_VIEW::tunersets)
	{
		printer->document_tunersets(this->ty);
	}
	// services
	else if (main->state.tc == 0)
	{
		int ti = main->services_tree->indexOfTopLevelItem(main->services_tree->currentItem());
		int stype;
		switch (ti)
		{
			// TV
			case 1:
				stype = e2db::STYPE::tv;
			break;
			// Radio
			case 2:
				stype = e2db::STYPE::radio;
			break;
			// Data
			case 3:
				stype = e2db::STYPE::data;
			break;
			// All Services
			default:
				stype = -1;
		}
		printer->document_services(stype);
	}
	// bouquets
	else if (main->state.tc == 1)
	{
		int ti = -1;
		QList<QTreeWidgetItem*> selected = main->tree->selectedItems();

		if (selected.empty())
		{
			printer->destroy();
			return;
		}
		for (auto & item : selected)
		{
			ti = main->tree->indexOfTopLevelItem(item);
			QVariantMap tdata = item->data(0, Qt::UserRole).toMap();
			QString qchlist = tdata["id"].toString();
			string bname = qchlist.toStdString();

			// bouquet | userbouquets
			if (ti != -1)
				printer->document_bouquet(bname);
			// userbouquet
			else
				printer->document_userbouquet(bname);
		}
	}

	printer->print();
	printer->destroy();
}

void tab::toolsInspector()
{
	tools->inspector();
}

void tab::toolsImportFromFile(TOOLS_FILE ftype, e2db::FCONVS fci)
{
	e2db::fcopts opts;
	opts.fc = fci;

	switch (ftype)
	{
		case TOOLS_FILE::tools_csv:
			this->tools->importFileCSV(fci, opts);
		break;
		case TOOLS_FILE::tools_html:
			return;
	}
}

void tab::toolsExportToFile(TOOLS_FILE ftype, e2db::FCONVS fco)
{
	e2db::fcopts opts;
	opts.fc = fco;

	if (fco == e2db::FCONVS::convert_current)
	{
		string filename;

		// tunersets
		if (this->ttv == gui::TAB_VIEW::tunersets)
		{
			switch (this->ty)
			{
				case e2db::YTYPE::sat:
					filename = "satellites";
				break;
				case e2db::YTYPE::terrestrial:
					filename = "terrestrial";
				break;
				case e2db::YTYPE::cable:
					filename = "cables";
				break;
				case e2db::YTYPE::atsc:
					filename = "atsc";
				break;
			}
			opts.ytype = this->ty;
			fco = e2db::FCONVS::convert_tunersets;
		}
		// services
		else if (main->state.tc == 0)
		{
			int ti = main->services_tree->indexOfTopLevelItem(main->services_tree->currentItem());
			int stype;
			switch (ti)
			{
				// TV
				case 1:
					stype = e2db::STYPE::tv;
					filename = "services-tv";
				break;
				// Radio
				case 2:
					stype = e2db::STYPE::radio;
					filename = "services-radio";
				break;
				// Data
				case 3:
					stype = e2db::STYPE::data;
					filename = "services-data";
				break;
				// All Services
				default:
					stype = -1;
					filename = "services";
			}
			opts.stype = stype;
			fco = e2db::FCONVS::convert_services;
		}
		// bouquets
		else if (main->state.tc == 1)
		{
			int ti = -1;
			QList<QTreeWidgetItem*> selected = main->tree->selectedItems();

			if (selected.empty())
			{
				return;
			}
			for (auto & item : selected)
			{
				ti = main->tree->indexOfTopLevelItem(item);
				QVariantMap tdata = item->data(0, Qt::UserRole).toMap();
				QString qchlist = tdata["id"].toString();
				string bname = qchlist.toStdString();

				// bouquet | userbouquets
				if (ti != -1)
				{
					opts.bname = bname;
					fco = e2db::FCONVS::convert_bouquets;
				}
				// userbouquet
				else
				{
					opts.bname = bname;
					fco = e2db::FCONVS::convert_userbouquets;
				}

				filename = bname;
			}
		}
		opts.filename = filename;
	}

	switch (ftype)
	{
		case TOOLS_FILE::tools_csv:
			opts.filename += ".csv";
			this->tools->exportFileCSV(fco, opts);
		break;
		case TOOLS_FILE::tools_html:
			opts.filename += ".html";
			this->tools->exportFileHTML(fco, opts);
		break;
	}
}

void tab::actionCall(int action)
{
	debug("actionCall()", "action", action);

	switch (action)
	{
		case gui::TAB_ATS::ListCut:
			view->listItemCut();
		break;
		case gui::TAB_ATS::ListCopy:
			view->listItemCopy();
		break;
		case gui::TAB_ATS::ListPaste:
			view->listItemPaste();
		break;
		case gui::TAB_ATS::ListDelete:
			view->listItemDelete();
		break;
		case gui::TAB_ATS::ListSelectAll:
			view->listItemSelectAll();
		break;

		case gui::TAB_ATS::TreeFind:
			view->treeSearchShow();
		break;
		case gui::TAB_ATS::TreeFindNext:
			view->treeFindPerform();
		break;
		case gui::TAB_ATS::ListFind:
			view->listSearchShow();
		break;
		case gui::TAB_ATS::ListFindNext:
			view->listFindPerform(viewAbstract::LIST_FIND::next);
		break;
		case gui::TAB_ATS::ListFindPrev:
			view->listFindPerform(viewAbstract::LIST_FIND::prev);
		break;
		case gui::TAB_ATS::ListFindAll:
			view->listFindPerform(viewAbstract::LIST_FIND::all);
		break;

		case gui::TAB_ATS::ShowChannelBook:
			gid->openTab(gui::TAB_VIEW::channelBook);
		break;
		case gui::TAB_ATS::EditTunersetsSat:
			gid->openTab(gui::TAB_VIEW::tunersets, e2db::YTYPE::sat);
		break;
		case gui::TAB_ATS::EditTunersetsTerrestrial:
			gid->openTab(gui::TAB_VIEW::tunersets, e2db::YTYPE::terrestrial);
		break;
		case gui::TAB_ATS::EditTunersetsCable:
			gid->openTab(gui::TAB_VIEW::tunersets, e2db::YTYPE::cable);
		break;
		case gui::TAB_ATS::EditTunersetsAtsc:
			gid->openTab(gui::TAB_VIEW::tunersets, e2db::YTYPE::atsc);
		break;

		case gui::TAB_ATS::ImportCSV_services:
			toolsImportFromFile(TOOLS_FILE::tools_csv, e2db::FCONVS::convert_services);
		break;
		case gui::TAB_ATS::ImportCSV_bouquet:
			toolsImportFromFile(TOOLS_FILE::tools_csv, e2db::FCONVS::convert_bouquets);
		break;
		case gui::TAB_ATS::ImportCSV_userbouquet:
			toolsImportFromFile(TOOLS_FILE::tools_csv, e2db::FCONVS::convert_userbouquets);
		break;
		case gui::TAB_ATS::ImportCSV_tunersets:
			toolsImportFromFile(TOOLS_FILE::tools_csv, e2db::FCONVS::convert_tunersets);
		break;
		case gui::TAB_ATS::ExportCSV_current:
			toolsExportToFile(TOOLS_FILE::tools_csv, e2db::FCONVS::convert_current);
		break;
		case gui::TAB_ATS::ExportCSV_all:
			toolsExportToFile(TOOLS_FILE::tools_csv, e2db::FCONVS::convert_all);
		break;
		case gui::TAB_ATS::ExportCSV_services:
			toolsExportToFile(TOOLS_FILE::tools_csv, e2db::FCONVS::convert_services);
		break;
		case gui::TAB_ATS::ExportCSV_bouquets:
			toolsExportToFile(TOOLS_FILE::tools_csv, e2db::FCONVS::convert_bouquets);
		break;
		case gui::TAB_ATS::ExportCSV_userbouquets:
			toolsExportToFile(TOOLS_FILE::tools_csv, e2db::FCONVS::convert_userbouquets);
		break;
		case gui::TAB_ATS::ExportCSV_tunersets:
			toolsExportToFile(TOOLS_FILE::tools_csv, e2db::FCONVS::convert_tunersets);
		break;
		case gui::TAB_ATS::ExportHTML_current:
			toolsExportToFile(TOOLS_FILE::tools_html, e2db::FCONVS::convert_current);
		break;
		case gui::TAB_ATS::ExportHTML_index:
			toolsExportToFile(TOOLS_FILE::tools_html, e2db::FCONVS::convert_index);
		break;
		case gui::TAB_ATS::ExportHTML_all:
			toolsExportToFile(TOOLS_FILE::tools_html, e2db::FCONVS::convert_all);
		break;
		case gui::TAB_ATS::ExportHTML_services:
			toolsExportToFile(TOOLS_FILE::tools_html, e2db::FCONVS::convert_services);
		break;
		case gui::TAB_ATS::ExportHTML_bouquets:
			toolsExportToFile(TOOLS_FILE::tools_html, e2db::FCONVS::convert_bouquets);
		break;
		case gui::TAB_ATS::ExportHTML_userbouquets:
			toolsExportToFile(TOOLS_FILE::tools_html, e2db::FCONVS::convert_tunersets);
		break;
		case gui::TAB_ATS::ExportHTML_tunersets:
			toolsExportToFile(TOOLS_FILE::tools_html, e2db::FCONVS::convert_tunersets);
		break;

		case gui::TAB_ATS::Inspector:
			tools->inspector();
		break;
	}
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

		unordered_map<string, e2se_ftpcom::ftpcom_file> tfiles;

		int profile_sel = gid->sets->value("profile/selected").toInt();
		gid->sets->beginReadArray("profile");
		gid->sets->setArrayIndex(profile_sel);
		for (auto & x : files)
		{
			string base;

			if (x.first.find(".tv") != string::npos || x.first.find(".radio") != string::npos)
			{
				base = gid->sets->value("pathBouquets").toString().toStdString();
			}
			else if (x.first == "satellites.xml" || x.first == "terrestrial.xml" || x.first == "cables.xml" || x.first == "atsc.xml")
			{
				base = gid->sets->value("pathTransponders").toString().toStdString();
			}
			//TODO upload services, other data ... (eg. picons)
			else
			{
				base = gid->sets->value("pathServices").toString().toStdString();
			}
			tfiles.emplace(base + '/' + x.first, x.second);

			debug("ftpUpload()", "file", base + '/' + x.first + " | " + to_string(x.second.size()));
		}
		gid->sets->endArray();
		files.clear();

		ftph->put_files(tfiles);
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

		this->updateChannelsIndex();
		this->updateBouquetsIndex();
		dbih->merge(files);
		view->reset();
		view->load();
	}
}

void tab::updateBouquetsIndex()
{
	debug("updateBouquetsIndex()");

	int i = 0, y;
	int count = main->tree->topLevelItemCount();
	vector<pair<int, string>> bss;
	vector<pair<int, string>> ubs;
	unordered_map<string, vector<string>> index;

	while (i != count)
	{
		QTreeWidgetItem* parent = main->tree->topLevelItem(i);
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

void tab::updateChannelsIndex()
{
	if (! main->state.changed)
		return;

	int i = 0, idx = 0;
	int count = main->list->topLevelItemCount();
	string curr_chlist = main->state.curr;
	dbih->index[curr_chlist].clear();

	debug("updateChannelsIndex()", "curr_chlist", curr_chlist);

	int sort_col = main->list->sortColumn();
	main->list->sortItems(0, Qt::AscendingOrder);

	while (i != count)
	{
		QTreeWidgetItem* item = main->list->topLevelItem(i);
		string chid = item->data(mainView::ITEM_DATA_ROLE::chid, Qt::UserRole).toString().toStdString();
		bool marker = item->data(mainView::ITEM_DATA_ROLE::marker, Qt::UserRole).toBool();
		idx = marker ? 0 : i + 1;
		dbih->index[curr_chlist].emplace_back(pair (idx, chid)); //C++17
		i++;
	}

	main->list->sortItems(main->state.sort.first, main->state.sort.second);
	main->list->header()->setSortIndicator(sort_col, main->state.sort.second);

	main->state.changed = false;
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
		QMessageBox::information(cwid, NULL, "For debugging purpose, set application.seeds absolute path under Settings > Advanced tab, then restart the software.");
	}
}

}
