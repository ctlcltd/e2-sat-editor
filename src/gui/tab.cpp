/*!
 * e2-sat-editor/src/gui/tab.cpp
 *
 * @link https://github.com/ctlcltd/e2-sat-editor
 * @copyright e2 SAT Editor Team
 * @author Leonardo Laureti
 * @version 0.8
 * @license MIT License
 * @license GNU GPLv3 License
 */

#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <algorithm>
#include <filesystem>

#include <QtGlobal>
#include <QGuiApplication>
#include <QWindow>
#include <QList>
#include <QSettings>
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

#include "../e2se_defs.h"
#include "platforms/platform.h"

#ifdef Q_OS_WASM
#include <QFileDialog>
#endif
#ifndef Q_OS_WASM
#include <QThread>
#endif
#ifdef E2SE_DEMO
#include <QResource>
#endif

#include "tab.h"
#include "theme.h"
#include "gui.h"
#include "dataHandler.h"
#include "ftpcom_gui.h"
#include "transpondersView.h"
#include "tunersetsView.h"
#include "piconsView.h"
#include "channelBookView.h"
#include "editBouquet.h"
#include "editService.h"
#include "editMarker.h"
#include "printable.h"

using std::pair, std::to_string, std::sort;

using namespace e2se;

namespace e2se_gui
{

tab::tab(gui* gid, QWidget* cwid)
{
	this->log = new logger("gui", "tab");

	unsigned long uniqtt = reinterpret_cast<std::uintptr_t>(this);
	std::srand(int (uniqtt));
	uniqtt &= std::rand();
	this->ttid = int (uniqtt);

	this->gid = gid;
	this->cwid = cwid;
	this->theme = new e2se_gui::theme;
	this->widget = new QWidget;
}

tab::~tab()
{
	debug("~tab");

	if (! this->child)
	{	delete this->tools;
		delete this->data;
		delete this->ftph;
	}
	delete this->view;

	delete this->widget;
	delete this->theme;
	delete this->log;
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
	debug("addChild");

	this->childs.emplace_back(child);
}

void tab::removeChild(tab* child)
{
	debug("removeChild");

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
	debug("tabSwitched");

	retrieveFlags();
	view->updateStatusBar();
	view->updateStatusBar(true);
}

void tab::tabChangeName(string path)
{
	debug("tabChangeName");

	string filename;

	if (! path.empty())
		filename = std::filesystem::path(path).filename().u8string();

	gid->tabChangeName(ttid, filename);

	for (auto & child : childs)
	{
		int ttid = child->getTabId();
		gid->tabChangeName(ttid, filename);
	}
}

void tab::setFlag(gui::GUI_CXE bit, bool flag)
{
	gid->setFlag(bit, flag);
}

bool tab::getFlag(gui::GUI_CXE bit)
{
	return gid->getFlag(bit);
}

void tab::setFlags(int preset)
{
	gid->setFlags(preset);
}

void tab::storeFlags()
{
	this->gxe = gid->getFlags();
}

void tab::retrieveFlags()
{
	gid->setFlags(this->gxe);
}

void tab::updateToolBars()
{
	auto flags = gid->getFlags();

	for (auto & x : tbars)
	{
		if (flags[x.first])
			x.second->setEnabled(true);
		else
			x.second->setDisabled(true);
	}
}

bool tab::statusBarIsVisible()
{
	return gid->statusBarIsVisible();
}

bool tab::statusBarIsHidden()
{
	return gid->statusBarIsHidden();
}

void tab::setStatusBar(gui::status msg)
{
	msg.view = this->ttv;
	gid->setStatusBar(msg);
}

//TODO tab::statusBarMessage this (gui*) is 0x0 with deleted tab* QTimer [Qt5]
void tab::resetStatusBar()
{
	//TODO TEST gui::closeAllTabs
	if (gid != nullptr)
		gid->resetStatusBar();
}

void tab::viewMain()
{
	debug("viewMain");

	this->data = new dataHandler;
	this->ftph = new ftpHandler;
	this->tools = new e2se_gui::tools(this, this->gid, this->cwid, this->data);
	this->view = new mainView(this, this->cwid, this->data);

	this->ttv = gui::TAB_VIEW::main;

	layout();

	this->root->addWidget(view->widget, 0, 0, 1, 1);

	newFile();
}

void tab::viewTransponders(tab* parent)
{
	debug("viewTransponders");

	parent->addChild(this);

	this->parent = parent;
	this->child = true;

	this->data = parent->data;
	this->ftph = parent->ftph;
	this->tools = parent->tools;
	this->view = new transpondersView(this, this->cwid, this->data);

	this->ttv = gui::TAB_VIEW::transponders;

	layout();

	this->root->addWidget(view->widget, 0, 0, 1, 1);

	load();
}

void tab::viewTunersets(tab* parent, int ytype)
{
	debug("viewTunersets");

	parent->addChild(this);

	this->parent = parent;
	this->child = true;

	this->data = parent->data;
	this->ftph = parent->ftph;
	this->tools = parent->tools;
	this->view = new tunersetsView(this, this->cwid, this->data, ytype);

	this->ttv = gui::TAB_VIEW::tunersets;

	layout();

	this->root->addWidget(view->widget, 0, 0, 1, 1);

	load();
}

void tab::viewPicons(tab* parent)
{
	debug("viewPicons");

	parent->addChild(this);

	this->parent = parent;
	this->child = true;

	this->data = parent->data;
	this->ftph = parent->ftph;
	this->tools = parent->tools;
	this->view = new piconsView(this, this->cwid, this->data);

	this->ttv = gui::TAB_VIEW::picons;

	layout();

	this->root->addWidget(view->widget, 0, 0, 1, 1);

	load();
}

void tab::viewChannelBook(tab* parent)
{
	debug("viewChannelBook");

	parent->addChild(this);

	this->parent = parent;
	this->child = true;

	this->data = parent->data;
	this->ftph = parent->ftph;
	this->tools = parent->tools;
	this->view = new channelBookView(this, this->cwid, this->data);

	this->ttv = gui::TAB_VIEW::channelBook;

	layout();

	this->root->addWidget(view->widget, 0, 0, 1, 1);

	load();
}

void tab::load()
{
	debug("load");

	view->load();

	for (auto & child : childs)
		child->view->load();
}

void tab::reset()
{
	debug("reset");

	view->reset();

	for (auto & child : childs)
		child->view->reset();
}

void tab::layout()
{
	debug("layout");

	widget->setStyleSheet("QGroupBox { spacing: 0; padding: 20px 0 0 0; border: 0 } QGroupBox::title { margin: 0 12px }");

	QGridLayout* frm = new QGridLayout(widget);

	QHBoxLayout* top = new QHBoxLayout;
	QGridLayout* container = new QGridLayout;
	QHBoxLayout* bottom = new QHBoxLayout;

	this->top_toolbar = toolBar(1);
	this->bottom_toolbar = toolBar(0);

	this->ftp_combo = new QComboBox;
	ftpComboItems();
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
	ftp_combo->connect(ftp_combo, &QComboBox::currentIndexChanged, [=](int index) { this->ftpComboChanged(index); });
#else
	ftp_combo->connect(ftp_combo, QOverload<int>::of(&QComboBox::currentIndexChanged), [=](int index) { this->ftpComboChanged(index); });
#endif
	platform::osComboBox(ftp_combo);

	tbars[gui::FileOpen] = toolBarAction(top_toolbar, tr("&Open", "toolbar"), theme->dynamicIcon("file-open"), [=]() { this->openFile(); }, QKeySequence::Open);
	tbars[gui::FileSave] = toolBarAction(top_toolbar, tr("&Save", "toolbar"), theme->dynamicIcon("save"), [=]() { this->saveFile(false); }, QKeySequence::Save);
	toolBarSeparator(top_toolbar);
	tbars[gui::FileImport] = toolBarAction(top_toolbar, tr("Import", "toolbar"), theme->dynamicIcon("import"), [=]() { this->importFile(); });
	tbars[gui::FileExport] = toolBarAction(top_toolbar, tr("Export", "toolbar"), theme->dynamicIcon("export"), [=]() { this->exportFile(); });
	toolBarSeparator(top_toolbar);
	toolBarAction(top_toolbar, tr("Settings", "toolbar"), theme->dynamicIcon("settings"), [=]() { this->settingsDialog(); });
	toolBarSpacer(top_toolbar);
	toolBarWidget(top_toolbar, tr("Select profile", "toolbar"), ftp_combo, [=]() { ftp_combo->showPopup(); });
	toolBarAction(top_toolbar, tr("Connect", "toolbar"), [=]() { this->ftpConnect(); });
	toolBarAction(top_toolbar, tr("Disconnect", "toolbar"), [=]() { this->ftpDisconnect(); });
	toolBarSeparator(top_toolbar);
	toolBarAction(top_toolbar, tr("Upload", "toolbar"), [=]() { this->ftpUpload(); });
	toolBarAction(top_toolbar, tr("Download", "toolbar"), [=]() { this->ftpDownload(); });

	bool DEBUG = false;
#if E2SE_BUILD == E2SE_TARGET_DEBUG
	DEBUG = true;
#endif
	if (QSettings().value("application/debug", DEBUG).toBool())
	{
		toolBarSeparator(bottom_toolbar);
		toolBarAction(bottom_toolbar, "§ Load seeds", [=]() { this->loadSeeds(); });
		toolBarAction(bottom_toolbar, "§ Reset", [=]() { this->newFile(); this->tabChangeName(); });
	}
	toolBarSpacer(bottom_toolbar);

	top->addWidget(top_toolbar);
	bottom->addWidget(bottom_toolbar);
	container->setContentsMargins(0, 0, 0, 0);

	frm->setSpacing(0);
	frm->setContentsMargins(0, 0, 0, 0);
	frm->addLayout(top, 0, 0);
	frm->addLayout(container, 1, 0);
	frm->addLayout(bottom, 2, 0);

	this->root = container;

	widget->setLayout(frm);

	toolBarStyleSheet();
}

void tab::settingsChanged()
{
	debug("settingsChanged");

	this->ftph->settingsChanged();
	this->data->settingsChanged();

	ftpComboItems();

	view->didChange();

	for (auto & child : childs)
		child->view->didChange();
}

void tab::themeChanged()
{
	debug("themeChanged");

	theme->changed();

	view->themeChanged();

	for (auto & child : childs)
		child->view->themeChanged();
}

void tab::clipboardDataChanged()
{
	debug("clipboardDataChanged");

	view->clipboardDataChanged();

	for (auto & child : childs)
		child->view->clipboardDataChanged();
}

void tab::newFile()
{
	debug("newFile");

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
	debug("openFile");

#ifdef E2SE_DEMO
	return this->loadSeeds();
#endif

#ifndef Q_OS_WASM
	string path = gid->openFileDialog();

	if (path.empty())
	{
		return;
	}

	readFile(path);
#else
	gid->blobs.clear();

	auto fileContentReady = [=](const QString& path, const QByteArray& filedata)
	{
		if (! path.isEmpty())
		{
			gui::gui_file file;
			file.data = filedata.toStdString();
			file.filename = path.toStdString();
			file.size = filedata.size();

			gid->blobs.emplace_back(file);

			readFile(path.toStdString());
		}
	};

	QFileDialog::getOpenFileContent("All Files (*)", fileContentReady);
#endif
}

bool tab::readFile(string path)
{
	debug("readFile", "path", path);

	if (path.empty())
		return false;

	reset();

	QTimer* timer = nullptr;

	if (statusBarIsVisible())
		timer = statusBarMessage(tr("Reading from %1 …", "message").arg(path.data()));

#ifndef Q_OS_WASM
	theme::setWaitCursor();
	bool readen = this->data->readFile(path);
	theme::unsetWaitCursor();
#else
	unordered_map<string, e2db::e2db_file> files;

	for (auto & q : gid->blobs)
	{
		e2db::e2db_file file;
		file.filename = q.filename;
		file.mime = q.mime;
		file.data = q.data;
		file.size = q.size;

		debug("readFile", "file.filename", q.filename);
		debug("readFile", "file.size", to_string(q.size));

		files.emplace(file.filename, file);
	}

	bool readen = this->data->readBlob(path, files);
#endif

	if (readen)
	{
		tabChangeName(path);
	}
	else
	{
		tabChangeName();

		error("readFile", tr("File Error", "error").toStdString(), tr("Error reading file \"%1\".", "error").arg(path.data()).toStdString());

		errorMessage(tr("File Error", "error"), tr("Error opening files.", "error"));

		return false;
	}

	load();

	if (this->child)
	{
		parent->reset();
		parent->load();
	}

	if (statusBarIsVisible())
		statusBarMessage(timer);

	return true;
}

void tab::saveFile(bool saveas)
{
	debug("saveFile", "saveas", saveas);

#ifdef E2SE_DEMO
	return this->demoMessage();
#endif

	string path = this->data->getPath();
	string nw_path;

	if (this->data->hasChanged())
	{
		updateIndex();
	}
	if (saveas || this->data->isNewfile())
	{
		nw_path = gid->saveFileDialog(path);

		if (nw_path.empty())
		{
			return;
		}
		else
		{
			path = nw_path;

			int dirsize = 0;
			string basedir;
			if (std::filesystem::is_directory(path))
				basedir = path;
			else
				basedir = std::filesystem::path(path).parent_path().u8string();
			std::filesystem::directory_iterator dirlist (basedir);
			for (const auto & entry : dirlist)
			{
				if (std::filesystem::is_regular_file(entry))
					dirsize++;
			}
			if (dirsize != 0)
			{
				bool overwrite = saveQuestion(tr("The destination contains files that will be overwritten.", "message"), tr("Do you want to overwrite them?", "message"));
				if (! overwrite)
					return;
			}
		}
	}
	else if (this->data->hasChanged())
	{
		bool overwrite = saveQuestion(tr("The file has been modified", "message"), tr("Do you want to save your changes?", "message"));
		if (! overwrite)
			return;
	}

	if (path.empty())
	{
		error("saveFile", tr("File Error", "error").toStdString(), tr("Empty path.", "error").toStdString());

		return;
	}

	debug("saveFile", "path", path);

	theme::setWaitCursor();
	bool written = this->data->writeFile(path);
	theme::unsetWaitCursor();

	if (written)
	{
		if (saveas)
			tabChangeName(path);

		if (statusBarIsVisible())
			statusBarMessage(tr("Saved to %1", "message").arg(path.data()));
		else
			infoMessage(tr("Saved!", "message"));
	}
	else
	{
		error("saveFile", tr("File Error", "error").toStdString(), tr("Error writing file \"%1\".", "error").arg(path.data()).toStdString());

		errorMessage(tr("File Error", "error"), tr("Error writing files.", "error"));
	}
}

void tab::importFile()
{
	debug("importFile");

#ifdef E2SE_DEMO
	return this->demoMessage();
#endif

	auto* dbih = this->data->dbih;

	gui::TAB_VIEW current = getTabView();
	gui::GUI_DPORTS gde = gui::GUI_DPORTS::AllFiles;
	vector<string> paths;

	// other views
	if (current == gui::TAB_VIEW::picons || current == gui::TAB_VIEW::channelBook)
	{
		// picons view
		if (current == gui::TAB_VIEW::picons)
			infoMessage(tr("Nothing to import", "message"), tr("You are in Edit Picons.", "message"));
		// channelBook view
		else if (current == gui::TAB_VIEW::channelBook)
			infoMessage(tr("Nothing to import", "message"), tr("You are in Channel Book.", "message"));

		return;
	}

	paths = gid->importFileDialog(gde);

	if (paths.empty())
		return;

	if (statusBarIsVisible())
	{
		string fname;
		if (paths.size() > 0)
			fname = std::filesystem::path(paths[0]).parent_path().u8string();
		else
			fname = paths[0];

		statusBarMessage(tr("Importing from %1 …", "message").arg(fname.data()));
	}

	theme::setWaitCursor();
	dbih->importFile(paths);
	theme::unsetWaitCursor();

	view->reset();
	view->load();

	this->data->setChanged(true);
}

void tab::exportFile()
{
	debug("exportFile");

#ifdef E2SE_DEMO
	return this->demoMessage();
#endif

	auto* dbih = this->data->dbih;

	gui::TAB_VIEW current = getTabView();
	gui::GUI_DPORTS gde = gui::GUI_DPORTS::AllFiles;
	vector<string> paths;
	string filename;
	int bit = -1;
	int dbtype = dbih->get_e2db_services_type();

	// tunersets view
	if (current == gui::TAB_VIEW::tunersets)
	{
		tunersetsView* view = reinterpret_cast<tunersetsView*>(this->view);
		auto state = view->currentState();

		gde = gui::GUI_DPORTS::Tunersets;
		bit = e2db::FPORTS::single_tunersets;
		switch (state.yx)
		{
			case e2db::YTYPE::satellite:
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
	// transponders view
	else if (current == gui::TAB_VIEW::transponders)
	{
		gde = gui::GUI_DPORTS::Services;
		if (dbtype == 0)
		{
			int ver = dbih->get_zapit_version();

			bit = e2db::FPORTS::all_services;
			filename = ver > 4 ? "lamedb5" : "lamedb";
		}
		else
		{
			bit = e2db::FPORTS::all_services_xml;
			filename = "services.xml";
		}

		paths.push_back(filename);
	}
	// main view
	else if (current == gui::TAB_VIEW::main)
	{
		mainView* view = reinterpret_cast<mainView*>(this->view);
		auto state = view->currentState();

		// services
		if (state.tc == 0)
		{
			gde = gui::GUI_DPORTS::Services;
			if (dbtype == 0)
			{
				int ver = dbih->get_zapit_version();

				bit = e2db::FPORTS::all_services;
				filename = ver > 4 ? "lamedb5" : "lamedb";
			}
			else
			{
				bit = e2db::FPORTS::all_services_xml;
				filename = "services.xml";
			}

			paths.push_back(filename);
		}
		// bouquets
		else if (state.tc == 1)
		{
			int ti = -1;
			QList<QTreeWidgetItem*> selected = view->tree->selectedItems();

			if (selected.empty())
			{
				return;
			}
			for (auto & item : selected)
			{
				ti = view->tree->indexOfTopLevelItem(item);
				string bname = item->data(0, Qt::UserRole).toString().toStdString();

				paths.push_back(bname);
			}
			if (paths.size() == 1)
			{
				filename = paths[0];
			}
			// bouquet | userbouquets
			if (ti != -1 || dbtype == 1)
			{
				gde = gui::GUI_DPORTS::Bouquets;
				if (dbtype == 0)
				{
					int ver = dbih->get_zapit_version();

					if (ver > 3)
						bit = e2db::FPORTS::single_bouquet_all;
					else
						bit = e2db::FPORTS::single_bouquet_all_epl;

					if (dbih->bouquets.count(filename))
					{
						for (string & fname : dbih->bouquets[filename].userbouquets)
							paths.push_back(fname);
					}
				}
				else
				{
					int ver = dbih->get_zapit_version();

					bit = e2db::FPORTS::all_bouquets_xml;
					filename = ver != 1 ? "ubouquets.xml" : "bouquets.xml";

					paths.push_back(filename);
				}
			}
			// userbouquet
			else
			{
				gde = gui::GUI_DPORTS::Userbouquets;
				bit = e2db::FPORTS::single_userbouquet;
			}
		}
	}
	// other views
	else
	{
		// picons view
		if (current == gui::TAB_VIEW::picons)
			infoMessage(tr("Nothing to export", "message"), tr("You are in Edit Picons.", "message"));
		// channelBook view
		else if (current == gui::TAB_VIEW::channelBook)
			infoMessage(tr("Nothing to export", "message"), tr("You are in Channel Book.", "message"));

		return;
	}

	if (paths.empty())
	{
		return;
	}

	if (this->data->hasChanged())
	{
		updateIndex();
	}

	string path = gid->exportFileDialog(gde, filename, bit);

	if (path.empty())
	{
		return;
	}
	if (paths.size() > 0)
	{
		int dirsize = 0;
		string basedir;
		if (std::filesystem::is_directory(path))
			basedir = path;
		else
			basedir = std::filesystem::path(path).parent_path().u8string();
		std::filesystem::directory_iterator dirlist (basedir);
		for (const auto & entry : dirlist)
		{
			if (std::filesystem::is_regular_file(entry))
				dirsize++;
		}
		if (dirsize != 0)
		{
			bool overwrite = saveQuestion(tr("The destination contains files that will be overwritten.", "message"), tr("Do you want to overwrite them?", "message"));
			if (! overwrite)
				return;
		}
	}

	debug("exportFile", "bit", bit);

	if (gde == gui::GUI_DPORTS::Services || gde == gui::GUI_DPORTS::Tunersets)
	{
		paths[0] = path;
	}
	else
	{
		std::filesystem::path fp = std::filesystem::path(path);
		string basedir = fp.parent_path().u8string();
		if (basedir.size() && basedir[basedir.size() - 1] != '/')
			basedir.append("/");

		for (string & path : paths)
			path = basedir + path;
	}

	theme::setWaitCursor();
	dbih->exportFile(bit, paths);
	theme::unsetWaitCursor();

	if (statusBarIsVisible())
	{
		string fname;
		if (paths.size() > 0)
			fname = std::filesystem::path(paths[0]).parent_path().u8string();
		else
			fname = paths[0];

		statusBarMessage(tr("Exported to %1", "message").arg(QString::fromStdString(fname)));
	}
	else
	{
		infoMessage(tr("Saved!", "message"));
	}
}

void tab::printFile(bool all)
{
	debug("printFile");

#ifdef E2SE_DEMO
	return this->demoMessage();
#endif

	gui::TAB_VIEW current = getTabView();
	printable* printer = new printable(this->cwid, this->data);

	// print all
	if (all)
	{
		printer->documentAll();
	}
	// tunersets view
	else if (current == gui::TAB_VIEW::tunersets)
	{
		tunersetsView* view = reinterpret_cast<tunersetsView*>(this->view);
		auto state = view->currentState();

		printer->documentTunersets(state.yx);
	}
	// main view
	else if (current == gui::TAB_VIEW::main)
	{
		mainView* view = reinterpret_cast<mainView*>(this->view);
		auto state = view->currentState();

		// services
		if (state.tc == 0)
		{
			int ti = view->side->indexOfTopLevelItem(view->side->currentItem());
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
			printer->documentServices(stype);
		}
		// bouquets
		else if (state.tc == 1)
		{
			int ti = -1;
			QList<QTreeWidgetItem*> selected = view->tree->selectedItems();

			if (selected.empty())
			{
				delete printer;
				return;
			}
			for (auto & item : selected)
			{
				ti = view->tree->indexOfTopLevelItem(item);
				string bname = item->data(0, Qt::UserRole).toString().toStdString();

				// bouquet | userbouquets
				if (ti != -1)
					printer->documentBouquet(bname);
				// userbouquet
				else
					printer->documentUserbouquet(bname);
			}
		}
	}
	// channelBook view
	else if (current == gui::TAB_VIEW::channelBook)
	{
		printer->documentAll();
	}

	printer->print();
	delete printer;

	if (statusBarIsVisible())
		statusBarMessage(tr("Printing …", "message"));
}

void tab::settingsDialog()
{
	debug("settingsDialog");

	gid->settingsDialog();
}

void tab::toolsInspector()
{
	for (auto & q : QGuiApplication::allWindows())
	{
		if (q->isWindowType() && q->objectName() == "inspectorWindow")
		{
			debug("toolsInspector", "raise", 1);

			q->requestActivate();
			return q->raise();
		}
	}

	debug("toolsInspector");

	tools->inspector();
}

void tab::toolsImportFromFile(TOOLS_FILE ftype, e2db::FCONVS fci)
{
	debug("toolsImportFromFile");

#ifdef E2SE_DEMO
	return this->demoMessage();
#endif

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

	this->data->setChanged(true);
}

void tab::toolsExportToFile(TOOLS_FILE ftype, e2db::FCONVS fco)
{
	debug("toolsExportToFile");

#ifdef E2SE_DEMO
	return this->demoMessage();
#endif

	gui::TAB_VIEW current = getTabView();
	e2db::fcopts opts;
	opts.fc = fco;
	string filename;

	if (fco == e2db::FCONVS::convert_current)
	{
		// tunersets view
		if (current == gui::TAB_VIEW::tunersets)
		{
			tunersetsView* view = reinterpret_cast<tunersetsView*>(this->view);
			auto state = view->currentState();

			switch (state.yx)
			{
				case e2db::YTYPE::satellite:
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
			opts.ytype = state.yx;
			fco = e2db::FCONVS::convert_tunersets;
		}
		// main view
		else if (current == gui::TAB_VIEW::main)
		{
			mainView* view = reinterpret_cast<mainView*>(this->view);
			auto state = view->currentState();

			// services
			if (state.tc == 0)
			{
				int ti = view->side->indexOfTopLevelItem(view->side->currentItem());
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
			else if (state.tc == 1)
			{
				int ti = -1;
				QList<QTreeWidgetItem*> selected = view->tree->selectedItems();

				if (selected.empty())
				{
					return;
				}
				for (auto & item : selected)
				{
					ti = view->tree->indexOfTopLevelItem(item);
					string bname = item->data(0, Qt::UserRole).toString().toStdString();

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
		}
		opts.filename = filename;
	}
	else
	{
		switch (fco)
		{
			case e2db::FCONVS::convert_all:
				filename = "all";
			break;
			case e2db::FCONVS::convert_index:
				filename = "index";
			break;
			case e2db::FCONVS::convert_services:
				filename = "services";
			break;
			case e2db::FCONVS::convert_bouquets:
				filename = "bouquets";
			break;
			case e2db::FCONVS::convert_userbouquets:
				filename = "userbouquets";
			break;
			case e2db::FCONVS::convert_tunersets:
				filename = "tunersets";
			break;
			default:
			return;
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

void tab::actionCall(int bit)
{
	debug("actionCall", "bit", bit);

	switch (bit)
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

		case gui::TAB_ATS::EditTransponders:
			gid->openTab(gui::TAB_VIEW::transponders);
		break;
		case gui::TAB_ATS::EditTunersetsSat:
			gid->openTab(gui::TAB_VIEW::tunersets, e2db::YTYPE::satellite);
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
		case gui::TAB_ATS::EditPicons:
			gid->openTab(gui::TAB_VIEW::picons);
		break;
		case gui::TAB_ATS::ShowChannelBook:
			gid->openTab(gui::TAB_VIEW::channelBook);
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
			toolsInspector();
		break;
	}
}

void tab::ftpComboItems()
{
	// debug("ftpComboItems");

	QSettings settings;

	int size = settings.beginReadArray("profile");
	for (int i = 0; i < size; i++)
	{
		settings.setArrayIndex(i);

		if (! settings.contains("profileName"))
			continue;

		//TODO improve profile/selected and array index differs trouble
		ftp_combo->addItem(settings.value("profileName").toString(), i + 1);
	}
	settings.endArray();

	int selected = settings.value("profile/selected", 0).toInt();
	ftp_combo->setCurrentIndex(selected);
}

void tab::ftpComboChanged(int index)
{
	debug("profileComboChanged", "selected", index);

	QSettings().setValue("profile/selected", index);

	ftp_combo->setCurrentIndex(index);
}

void tab::ftpConnect()
{
	debug("ftpConnect");

#ifdef E2SE_DEMO
	return this->demoMessage();
#endif

#ifndef Q_OS_WASM
	QThread* thread = QThread::create([=]()
	{
		if (this->ftph->openConnection())
			QMetaObject::invokeMethod(this->cwid, [=]() { this->ftpStbConnectSuccessNotify(); }, Qt::QueuedConnection);
		else
			QMetaObject::invokeMethod(this->cwid, [=]() { this->ftpStbConnectErrorNotify(); }, Qt::QueuedConnection);
	});
	thread->connect(thread, &QThread::started, [=]()
	{
		QMetaObject::invokeMethod(this->cwid, [=]() { this->ftpStbConnectingNotify(); }, Qt::QueuedConnection);
	});
	thread->start();
	thread->quit();
#endif
}

void tab::ftpDisconnect()
{
	debug("ftpDisconnect");

#ifdef E2SE_DEMO
	return this->demoMessage();
#endif

#ifndef Q_OS_WASM
	QThread* thread = QThread::create([=]()
	{
		if (this->ftph->closeConnection())
			QMetaObject::invokeMethod(this->cwid, [=]() { this->ftpStbDisconnectSuccessNotify(); }, Qt::QueuedConnection);
		else
			QMetaObject::invokeMethod(this->cwid, [=]() { this->ftpStbDisconnectErrorNotify(); }, Qt::QueuedConnection);
	});
	thread->connect(thread, &QThread::started, [=]()
	{
		QMetaObject::invokeMethod(this->cwid, [=]() { this->ftpStbDisconnectingNotify(); }, Qt::QueuedConnection);
	});
	thread->start();
	thread->quit();
#endif
}

void tab::ftpUpload()
{
	debug("ftpUpload");

#ifdef E2SE_DEMO
	return this->demoMessage();
#endif

#ifndef Q_OS_WASM
	{
		QThread* thread = QThread::create([=]()
		{
			if (! this->ftph->handleConnection())
				QMetaObject::invokeMethod(this->cwid, [=]() { this->ftpStbConnectErrorNotify(); }, Qt::QueuedConnection);
		});
		thread->connect(thread, &QThread::started, [=]()
		{
			QMetaObject::invokeMethod(this->cwid, [=]() { this->ftpStbHandlingNotify(); }, Qt::QueuedConnection);
		});
		thread->start();
		thread->wait();
		thread->quit();
	}

	if (! this->ftph->isConnected())
		return;

	QSettings settings;
	int profile_sel = settings.value("profile/selected", 0).toInt();
	settings.beginReadArray("profile");
	settings.setArrayIndex(profile_sel);
	string baset = settings.value("pathTransponders").toString().toStdString();
	string bases = settings.value("pathServices").toString().toStdString();
	string baseb = settings.value("pathBouquets").toString().toStdString();
	string basep = settings.value("pathPicons").toString().toStdString();
	settings.endArray();

	auto* ftih = this->ftph->ftih;
	auto* dbih = this->data->dbih;

	this->files = dbih->get_output();

	if (this->files.empty())
		return;

	for (auto & x : this->files)
	{
		string filename = x.first;
		string basedir;
		string path;

		if (filename.find(".tv") != string::npos || filename.find(".radio") != string::npos)
			basedir = baseb;
		else if (filename == "satellites.xml" || filename == "terrestrial.xml" || filename == "cables.xml" || filename == "atsc.xml")
			basedir = baset;
		//TODO upload services, other data ... (eg. picons)
		else
			basedir = bases;

		if (basedir.size() && basedir[basedir.size() - 1] != '/')
			basedir.append("/");

		path = basedir + filename;

		e2se_ftpcom::ftpcom::ftpcom_file file;
		file.filename = x.second.filename;
		file.mime = x.second.mime;
		file.data = x.second.data;
		file.size = x.second.size;
		ftp_files.emplace(path, file);

		debug("ftpUpload", "file path", basedir + file.filename);
		debug("ftpUpload", "file size", to_string(file.size));
	}

	files.clear();

	{
		QThread* thread = QThread::create([=]()
		{
			if (this->ftp_files.empty())
				return;

			for (auto & x : this->ftp_files)
			{
				std::filesystem::path fpath = std::filesystem::path(x.first);
				string basedir = fpath.parent_path().u8string();
				string filename = fpath.filename().u8string();

				QMetaObject::invokeMethod(this->cwid, [=]() { this->ftpStbUploadNotify(filename); }, Qt::QueuedConnection);

				ftih->upload_data(basedir, filename, x.second);
			}
		});
		thread->connect(thread, &QThread::finished, [=]()
		{
			this->ftp_files.clear();

			int files_count = int (ftp_files.size());
			QMetaObject::invokeMethod(this->cwid, [=]() { this->ftpStbUploadNotify(files_count); }, Qt::QueuedConnection);

			QMetaObject::invokeMethod(this->cwid, [=]() { this->ftpReloadStb(); }, Qt::QueuedConnection);
		});
		thread->start();
		thread->quit();
	}
#endif
}

void tab::ftpDownload()
{
	debug("ftpDownload");

#ifdef E2SE_DEMO
	return this->demoMessage();
#endif

#ifndef Q_OS_WASM
	{
		QThread* thread = QThread::create([=]()
		{
			if (! this->ftph->handleConnection())
				QMetaObject::invokeMethod(this->cwid, [=]() { this->ftpStbConnectErrorNotify(); }, Qt::QueuedConnection);
		});
		thread->connect(thread, &QThread::started, [=]()
		{
			QMetaObject::invokeMethod(this->cwid, [=]() { this->ftpStbHandlingNotify(); }, Qt::QueuedConnection);
		});
		thread->start();
		thread->wait();
		thread->quit();
	}

	if (! this->ftph->isConnected())
		return;

	auto* ftih = this->ftph->ftih;
	auto* dbih = this->data->dbih;

	{
		QThread* thread = QThread::create([=]()
		{
			//TODO improve
			ftih->fetch_paths();

			for (string & w : ftih->ftdb)
			{
				std::filesystem::path fpath = std::filesystem::path(w);
				string basedir = fpath.parent_path().u8string();
				string filename = fpath.filename().u8string();

				QMetaObject::invokeMethod(this->cwid, [=]() { this->ftpStbDownloadNotify(filename); }, Qt::QueuedConnection);

				e2se_ftpcom::ftpcom::ftpcom_file file;

				ftih->download_data(basedir, filename, file);

				this->ftp_files[filename] = file;
			}
		});
		thread->connect(thread, &QThread::finished, [=]()
		{
			for (auto & x : this->ftp_files)
			{
				e2db::e2db_file file;
				file.filename = x.second.filename;
				file.mime = x.second.mime;
				file.data = x.second.data;
				file.size = x.second.size;

				debug("ftpDownload", "file path", x.first);
				debug("ftpDownload", "file size", to_string(x.second.size));

				this->files.emplace(file.filename, file);
			}

			this->ftp_files.clear();

			int files_count = int (this->files.size());
			QMetaObject::invokeMethod(this->cwid, [=]() { this->ftpStbDownloadNotify(files_count); }, Qt::QueuedConnection);

			if (this->files.empty())
				return;

			QMetaObject::invokeMethod(this->cwid, [=]() { updateIndex(); }, Qt::QueuedConnection);

			dbih->importBlob(this->files);

			QMetaObject::invokeMethod(this->cwid, [=]()
			{
				view->reset();
				view->load();

				this->data->setChanged(true);
			}, Qt::QueuedConnection);

			this->files.clear();
		});
		thread->start();
		thread->quit();
	}
#endif
}

void tab::ftpReloadStb()
{
	debug("ftpReloadStb");

#ifdef E2SE_DEMO
	return this->demoMessage();
#endif

#ifndef Q_OS_WASM
	auto* ftih = this->ftph->ftih;

	QThread* thread = QThread::create([=]()
	{
		if (ftih->cmd_ifreload() || ftih->cmd_tnreload())
			QMetaObject::invokeMethod(this->cwid, [=]() { this->ftpStbReloadSuccessNotify(); }, Qt::QueuedConnection);
		else
			QMetaObject::invokeMethod(this->cwid, [=]() { this->ftpStbReloadErrorNotify(); }, Qt::QueuedConnection);
	});
	thread->connect(thread, &QThread::started, [=]()
	{
		QMetaObject::invokeMethod(this->cwid, [=]() { this->ftpStbReloadingNotify(); }, Qt::QueuedConnection);
	});
	thread->start();
	thread->quit();
#endif
}

void tab::ftpStbConnectingNotify()
{
	if (statusBarIsVisible())
		statusBarMessage(tr("FTP connecting ...", "message"));
}

void tab::ftpStbDisconnectingNotify()
{
	if (statusBarIsVisible())
		statusBarMessage(tr("FTP disconnecting ...", "message"));
}

void tab::ftpStbHandlingNotify()
{
	if (statusBarIsVisible())
	{
		if (ftph->isConnected())
			statusBarMessage(tr("Trying to resume FTP connection ...", "message"));
		else
			statusBarMessage(tr("FTP connecting ...", "message"));
	}
}

void tab::ftpStbReloadingNotify()
{
	if (statusBarIsVisible())
		statusBarMessage(tr("STB reloading ...", "message"));
}

void tab::ftpStbConnectSuccessNotify()
{
	if (statusBarIsVisible())
		statusBarMessage(tr("FTP connected successfully.", "message"));
	else
		infoMessage(tr("FTP connected successfully.", "message"));
}

void tab::ftpStbConnectErrorNotify()
{
	string hostname = this->ftph->getServerHostname();
	error("ftpStbConnectErrorNotify", tr("FTP Error", "error").toStdString(), tr("Cannot connect to FTP \"%1\".", "error").arg(hostname.data()).toStdString());

	errorMessage(tr("FTP Error", "error"), tr("Cannot connect to FTP Server!", "error"));
}

void tab::ftpStbDisconnectSuccessNotify()
{
	if (statusBarIsVisible())
		statusBarMessage(tr("FTP disconnected successfully.", "message"));
	else
		infoMessage(tr("FTP disconnected successfully.", "message"));
}

void tab::ftpStbDisconnectErrorNotify()
{
	string hostname = this->ftph->getServerHostname();
	error("ftpStbDisconnectErrorNotify", tr("FTP Error", "error").toStdString(), tr("Cannot disconnect from FTP \"%1\".", "error").arg(hostname.data()).toStdString());

	errorMessage(tr("FTP Error", "error"), tr("Cannot disconnect from FTP Server!", "error"));
}

void tab::ftpStbUploadNotify(int files_count)
{
	if (statusBarIsVisible())
		statusBarMessage(tr("Uploaded %n files", "message", files_count));
	else
		infoMessage(tr("Uploaded %n files", "message", files_count));
}

void tab::ftpStbUploadNotify(string filename)
{
	if (statusBarIsVisible())
		statusBarMessage(tr("Uploading file: %1", "message").arg(filename.data()));
}

void tab::ftpStbDownloadNotify(int files_count)
{
	if (statusBarIsVisible())
		statusBarMessage(tr("Downloaded %n files", "message", files_count));
}

void tab::ftpStbDownloadNotify(string filename)
{
	if (statusBarIsVisible())
		statusBarMessage(tr("Downloading file: %1", "message").arg(filename.data()));
}

void tab::ftpStbReloadSuccessNotify()
{
	if (statusBarIsVisible())
		statusBarMessage(tr("STB reload done.", "message"));
	else
		infoMessage(tr("STB reload done.", "message"));
}

void tab::ftpStbReloadErrorNotify()
{
	string hostname = this->ftph->getServerHostname();
	error("ftpStbReloadErrorNotify", tr("FTP Error", "error").toStdString(), tr("Cannot reload STB \"%1\".", "error").arg(hostname.data()).toStdString());

	errorMessage(tr("FTP Error", "error"), tr("Cannot reload STB!", "error"));
}

void tab::updateIndex()
{
	debug("updateIndex");

	if (this->child)
	{
		parent->updateIndex();
	}
	view->updateIndex();
}

//TODO potential SEGFAULT
QTimer* tab::statusBarMessage(QString text)
{
	gui::status msg;
	msg.info = true;
	msg.message = text.toStdString();
	setStatusBar(msg);

	QTimer* timer = new QTimer(widget);
	timer->setSingleShot(true);
	timer->setInterval(STATUSBAR_MESSAGE_TIMEOUT);
	timer->callOnTimeout([=]() { this->resetStatusBar(); delete timer; });
	timer->start();
	return timer;
}

void tab::statusBarMessage(QTimer* timer)
{
	if (timer == nullptr)
	{
		QTimer* timer = new QTimer(widget);
		timer->setSingleShot(true);
		timer->setInterval(STATUSBAR_MESSAGE_DELAY);
		timer->callOnTimeout([=]() { this->resetStatusBar(); delete timer; });
	}
	else
	{
		timer->stop();
		timer->setInterval(STATUSBAR_MESSAGE_DELAY);
		timer->start();
	}
}

bool tab::saveQuestion(QString title, QString text)
{
	text.append("\n");
	QMessageBox msg = QMessageBox(this->cwid);

	msg.setWindowFlags(Qt::Sheet | Qt::MSWindowsFixedSizeDialogHint);
#ifdef Q_OS_MAC
	msg.setAttribute(Qt::WA_TranslucentBackground);
#endif

	msg.setText(title);
	msg.setInformativeText(text);
	msg.setStandardButtons(QMessageBox::Save | QMessageBox::Discard);
	msg.setDefaultButton(QMessageBox::Save);
	return (msg.exec() == QMessageBox::Save);
}

bool tab::removeQuestion(QString title, QString text)
{
	if (! QSettings().value("preference/askConfirmation", false).toBool())
		return true;

	text.prepend("<span style=\"white-space: nowrap\">");
	text.append("</span><br>");
	QMessageBox msg = QMessageBox(this->cwid);

	msg.setWindowFlags(Qt::Sheet | Qt::MSWindowsFixedSizeDialogHint);
#ifdef Q_OS_MAC
	msg.setAttribute(Qt::WA_TranslucentBackground);
#endif

	msg.setText(title);
	msg.setInformativeText(text);
	msg.setStandardButtons(QMessageBox::Ok | QMessageBox::Retry);
	msg.setDefaultButton(QMessageBox::Ok);
	return (msg.exec() == QMessageBox::Ok);
}

void tab::infoMessage(QString title)
{
	QMessageBox msg = QMessageBox(this->cwid);

	msg.setWindowFlags(Qt::Popup);

	msg.setText(title);
	QRect pos = msg.geometry();
	pos.moveCenter(QPoint(this->cwid->width() / 2, this->cwid->height() / 2));
	msg.setGeometry(pos);
	msg.exec();
}

void tab::infoMessage(QString title, QString text)
{
	text.prepend("<span style=\"white-space: nowrap\">");
	text.append("</span><br>");
	QMessageBox msg = QMessageBox(this->cwid);

	msg.setWindowFlags(Qt::Popup);

	msg.setText(title);
	msg.setInformativeText(text);
	QRect pos = msg.geometry();
	pos.moveCenter(QPoint(this->cwid->width() / 2, this->cwid->height() / 2));
	msg.setGeometry(pos);
	msg.exec();
}

void tab::errorMessage(QString title, QString text)
{
	QMessageBox::critical(this->cwid, title, text);
}

void tab::demoMessage()
{
	QMessageBox::information(this->cwid, NULL, tr("DEMO MODE", "message"));
}

void tab::loadSeeds()
{
	QSettings settings;

	if (settings.contains("application/seeds"))
	{
#ifdef E2SE_DEMO
		gid->blobs.clear();

		vector<QString> files = {
			":/e2se-seeds/enigma_db/blacklist",
			":/e2se-seeds/enigma_db/bouquets.radio",
			":/e2se-seeds/enigma_db/bouquets.tv",
			":/e2se-seeds/enigma_db/lamedb",
			":/e2se-seeds/enigma_db/lamedb5",
			":/e2se-seeds/enigma_db/userbouquet.dbe01.radio",
			":/e2se-seeds/enigma_db/userbouquet.dbe01.tv",
			":/e2se-seeds/enigma_db/userbouquet.dbe02.radio",
			":/e2se-seeds/enigma_db/userbouquet.dbe02.tv",
			":/e2se-seeds/enigma_db/userbouquet.dbe03.radio",
			":/e2se-seeds/enigma_db/userbouquet.dbe03.tv",
			":/e2se-seeds/enigma_db/userbouquet.dbe04.tv",
			":/e2se-seeds/enigma_db/whitelist",
			":/e2se-seeds/enigma_db/atsc.xml",
			":/e2se-seeds/enigma_db/cables.xml",
			":/e2se-seeds/enigma_db/satellites.xml",
			":/e2se-seeds/enigma_db/terrestrial.xml"
		};

		for (auto & path : files)
		{
			gui::gui_file file;
			file.data = QResource(path).uncompressedData().toStdString();
			file.size = QResource(path).size();
			file.filename = path.remove(0, 1).toStdString();

			gid->blobs.emplace_back(file);
		}
#endif

		readFile(settings.value("application/seeds").toString().toStdString());
	}
	else if (settings.value("applications/seeds").toString().isEmpty())
	{
		settings.setValue("application/seeds", "");

		//: HTML formattation: text%1text%2text%3 treat them as spaces
		QMessageBox::information(this->cwid, NULL, tr("For debugging purpose.%1Set application.seeds absolute path under Settings > Advanced tab, then restart the software.%2Source seeds available at:%3", "message").arg("<br><br>").arg("<br><br>").arg("<br>%1").arg(QString("<a href=\"%1\">%1</a>").arg("https://github.com/ctlcltd/e2se-seeds")));
	}
}

QToolBar* tab::toolBar(int type)
{
	QToolBar* toolbar = new QToolBar;
	// 1: top
	if (type)
	{
		toolbar->setObjectName("tab_top_toolbar");
		toolbar->setIconSize(QSize(32, 32));
		toolbar->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
		toolbar->setStyleSheet("QToolBar { padding: 0 12px } QToolButton { font-size: 18px }");
	}
	else
	// 0: bottom
	{
		toolbar->setObjectName("tab_bottom_toolbar");
		toolbar->setStyleSheet("QToolBar { padding: 8px 12px } QToolButton { font-size: 16px; font-weight: bold; }");
	}

#ifndef Q_OS_MAC
	platform::osWidgetOpaque(toolbar);
#else
	toolbar->setAttribute(Qt::WA_TranslucentBackground);
	toolbar->setAutoFillBackground(false);

	platform::osWidgetBlend(toolbar, platform::fx_translucent_background, platform::fx_opaque);
#endif

	return toolbar;
}

QAction* tab::toolBarAction(QToolBar* toolbar, QString text, std::function<void()> trigger)
{
	QAction* action = new QAction(toolbar);
	action->setText(text);
	action->connect(action, &QAction::triggered, trigger);
	toolbar->addAction(action);
	return action;
}

QAction* tab::toolBarAction(QToolBar* toolbar, QString text, pair<e2se_gui::theme*, QString> icon, std::function<void()> trigger)
{
	QAction* action = new QAction(toolbar);
	action->setText(text);
	action->setIcon(icon.first->dynamicIcon(icon.second, action));
	action->connect(action, &QAction::triggered, trigger);
	toolbar->addAction(action);
	return action;
}

QAction* tab::toolBarAction(QToolBar* toolbar, QString text, std::function<void()> trigger, QKeySequence shortcut)
{
	QAction* action = new QAction(toolbar);
	action->setText(text);
	action->setShortcut(shortcut);
	action->connect(action, &QAction::triggered, trigger);
	toolbar->addAction(action);
	return action;
}

QAction* tab::toolBarAction(QToolBar* toolbar, QString text, pair<e2se_gui::theme*, QString> icon, std::function<void()> trigger, QKeySequence shortcut)
{
	QAction* action = new QAction(toolbar);
	action->setText(text);
	action->setIcon(icon.first->dynamicIcon(icon.second, action));
	action->setShortcut(shortcut);
	action->connect(action, &QAction::triggered, trigger);
	toolbar->addAction(action);
	return action;
}

QWidget* tab::toolBarWidget(QToolBar* toolbar, QString text, QWidget* widget)
{
	widget->setAccessibleName(text);
	toolbar->addWidget(widget);
	return widget;
}

QWidget* tab::toolBarWidget(QToolBar* toolbar, QString text, QWidget* widget, std::function<void()> trigger)
{
	QAction* action = new QAction(toolbar);
	action->setText(text);
	action->connect(action, &QAction::triggered, trigger);
	toolbar->addWidget(widget);
	toolbar->addAction(action);
	toolbar->widgetForAction(action)->setFixedWidth(0);
	return widget;
}

QAction* tab::toolBarSeparator(QToolBar* toolbar)
{
	QAction* action = new QAction(toolbar);
	action->setSeparator(true);
	toolbar->addAction(action);
	return action;
}

QWidget* tab::toolBarSpacer(QToolBar* toolbar)
{
	QWidget* spacer = new QWidget;
	spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Ignored);
	toolbar->addWidget(spacer);
	return spacer;
}

void tab::toolBarStyleSheet()
{
	if (! theme::isDefault())
	{
		theme->dynamicStyleSheet(widget, "#tab_top_toolbar, #tab_bottom_toolbar { background: palette(mid) }");
	}
#ifndef Q_OS_MAC
	else
	{
		QColor tbshade;
		QString tbshade_hexArgb;

		tbshade = theme::absLuma() ? QPalette().color(QPalette::Shadow).darker() : QPalette().color(QPalette::Shadow).lighter();
		tbshade.setAlpha(theme::absLuma() ? 60 : 90);
		tbshade_hexArgb = tbshade.name(QColor::HexArgb);

		theme->dynamicStyleSheet(widget, "#tab_top_toolbar, #tab_bottom_toolbar { border-style: solid; border-width: 1px 0; border-color: " + tbshade_hexArgb + " }");
	}
#else
	theme->dynamicStyleSheet(widget, "#tab_top_toolbar, #tab_bottom_toolbar { border-style: solid; border-width: 1px 0 }");

	QColor tbshade;
	QString tbshade_hexArgb;

	tbshade = QColor(Qt::black);
	tbshade.setAlphaF(0.08);
	tbshade_hexArgb = tbshade.name(QColor::HexArgb);

	theme->dynamicStyleSheet(widget, "#tab_top_toolbar, #tab_bottom_toolbar { border-color: " + tbshade_hexArgb + " }", theme::light);

	tbshade = QPalette().color(QPalette::Dark).darker();
	tbshade.setAlphaF(0.32);
	tbshade_hexArgb = tbshade.name(QColor::HexArgb);

	theme->dynamicStyleSheet(widget, "#tab_top_toolbar, #tab_bottom_toolbar { border-color: " + tbshade_hexArgb + " }", theme::dark);
#endif
}

}
