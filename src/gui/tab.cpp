/*!
 * e2-sat-editor/src/gui/tab.cpp
 *
 * @link https://github.com/ctlcltd/e2-sat-editor
 * @copyright e2 SAT Editor Team
 * @author Leonardo Laureti
 * @version 0.2
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
#include <QList>
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

using std::pair, std::to_string, std::sort;
using namespace e2se;

namespace e2se_gui
{

tab::tab(gui* gid, QWidget* cwid, e2se::logger::session* log)
{
	this->log = new logger(log, "tab");
	debug("tab()");

	int uniqtt = reinterpret_cast<std::uintptr_t>(this);
	std::srand(uniqtt);
	uniqtt &= std::rand();
	this->ttid = uniqtt;

	this->gid = gid;
	this->cwid = cwid;
	this->theme = new e2se_gui::theme;
	this->widget = new QWidget;
}

tab::~tab()
{
	if (! this->child)
	{
		delete this->data;
	}
	delete this->widget;
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
	view->updateStatusBar();
	view->updateStatusBar(true);
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

void tab::resetStatusBar()
{
	gid->resetStatusBar();
}

void tab::viewMain()
{
	debug("viewMain()");

	this->data = new dataHandler(this->log->log);
	this->ftph = new ftpHandler(this->log->log);
	this->tools = new e2se_gui::tools(this, this->gid, this->cwid, this->data, this->log->log);
	this->view = new mainView(this, this->cwid, this->data, this->log->log);

	this->ttv = gui::TAB_VIEW::main;

	layout();
	
	this->root->addWidget(view->widget, 0, 0, 1, 1);

	newFile();
}

void tab::viewTunersets(tab* parent, int ytype)
{
	debug("viewTunersets()");

	parent->addChild(this);

	this->parent = parent;
	this->child = true;

	this->data = parent->data;
	this->ftph = parent->ftph;
	this->tools = parent->tools;
	this->view = new tunersetsView(this, this->cwid, this->data, ytype, this->log->log);

	this->ttv = gui::TAB_VIEW::tunersets;

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
	this->ftph = parent->ftph;
	this->tools = parent->tools;
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

	for (auto & child : childs)
		child->view->load();
}

void tab::reset()
{
	debug("reset()");

	view->reset();

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

	this->top_toolbar = toolBar(1);
	this->bottom_toolbar = toolBar(0);

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
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
	profile_combo->connect(profile_combo, &QComboBox::currentIndexChanged, [=](int index) { this->profileComboChanged(index); });
#else
	profile_combo->connect(profile_combo, QOverload<int>::of(&QComboBox::currentIndexChanged), [=](int index) { this->profileComboChanged(index); });
#endif
	platform::osComboBox(profile_combo);

	tbars[gui::FileOpen] = toolBarAction(top_toolbar, "&Open", theme->dynamicIcon("file-open"), [=]() { this->openFile(); }, QKeySequence::Open);
	tbars[gui::FileSave] = toolBarAction(top_toolbar, "&Save", theme->dynamicIcon("save"), [=]() { this->saveFile(false); }, QKeySequence::Save);
	toolBarSeparator(top_toolbar);
	tbars[gui::FileImport] = toolBarAction(top_toolbar, "Import", theme->dynamicIcon("import"), [=]() { this->importFile(); });
	tbars[gui::FileExport] = toolBarAction(top_toolbar, "Export", theme->dynamicIcon("export"), [=]() { this->exportFile(); });
	toolBarSeparator(top_toolbar);
	toolBarAction(top_toolbar, "Settings", theme->dynamicIcon("settings"), [=]() { gid->settings(); });
	toolBarSpacer(top_toolbar);
	toolBarWidget(top_toolbar, profile_combo);
	toolBarAction(top_toolbar, "Connect", [=]() { this->ftpConnect(); });
	toolBarAction(top_toolbar, "Disconnect", [=]() { this->ftpDisconnect(); });
	toolBarSeparator(top_toolbar);
	toolBarAction(top_toolbar, "Upload", [=]() { this->ftpUpload(); });
	toolBarAction(top_toolbar, "Download", [=]() { this->ftpDownload(); });

	if (gid->sets->value("application/debug", true).toBool())
	{
		toolBarSeparator(bottom_toolbar);
		toolBarAction(bottom_toolbar, "§ Load seeds", [=]() { this->loadSeeds(); });
		toolBarAction(bottom_toolbar, "§ Reset", [=]() { this->newFile(); tabChangeName(); });
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

void tab::themeChanged()
{
	debug("themeChanged()");

	theme->changed();

	view->themeChanged();

	for (auto & child : childs)
		child->view->themeChanged();
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

	if (path.empty())
	{
		return;
	}
	
	readFile(path);
}

bool tab::readFile(string filename)
{
	debug("readFile()", "filename", filename);

	if (filename.empty())
		return false;

	reset();

	QTimer* timer = nullptr;

	if (statusBarIsVisible())
		timer = statusBarMessage("Reading from " + filename + " …");

	QGuiApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
	bool readen = this->data->readFile(filename);
	QGuiApplication::restoreOverrideCursor();

	if (readen)
	{
		tabChangeName(filename);
	}
	else
	{
		tabChangeName();
		errorMessage("File Error", "Error opening files.");
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
	debug("saveFile()", "saveas", saveas);

	string path;
	string filename = path = this->data->getFilename();

	if (this->data->hasChanged())
	{
		updateIndex();
	}
	if (saveas || this->data->isNewfile())
	{
		path = gid->saveFileDialog(filename);
	}
	else if (this->data->hasChanged())
	{
		bool overwrite = saveQuestion("The file has been modified", "Do you want to save your changes?");
		if (! overwrite)
			return;
	}
	else
	{
		bool overwrite = saveQuestion("The file will be overwritten", "Do you want to overwrite it?");
		if (! overwrite)
			return;
	}

	if (path.empty())
	{
		return;
	}

	debug("saveFile()", "path", path);

	QGuiApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
	bool written = this->data->writeFile(path);
	QGuiApplication::restoreOverrideCursor();

	if (written) {
		if (statusBarIsVisible())
			statusBarMessage("Saved to " + path);
		else
			infoMessage("Saved!");
	}
	else
	{
		errorMessage("File Error", "Error writing files.");
	}
}

void tab::importFile()
{
	debug("importFile()");

	auto* dbih = this->data->dbih;

	gui::TAB_VIEW current = getTabView();
	gui::GUI_DPORTS gde = gui::GUI_DPORTS::AllFiles;
	vector<string> paths;

	// channelBook view
	if (current == gui::TAB_VIEW::channelBook)
	{
		return infoMessage("Nothing to import", "You are in channel book.");
	}

	paths = gid->importFileDialog(gde);

	if (paths.empty())
		return;

	if (statusBarIsVisible())
	{
		string path;
		if (paths.size() > 0)
			path = std::filesystem::path(path).remove_filename().u8string(); //C++17
		else
			path = paths[0];

		statusBarMessage("Importing from " + path + " …");
	}

	QGuiApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
	dbih->importFile(paths);
	QGuiApplication::restoreOverrideCursor();

	view->reset();
	view->load();

	this->data->setChanged(true);
}

void tab::exportFile()
{
	debug("exportFile()");

	auto* dbih = this->data->dbih;

	gui::TAB_VIEW current = getTabView();
	gui::GUI_DPORTS gde = gui::GUI_DPORTS::AllFiles;
	vector<string> paths;
	string filename;
	int flags = -1;

	// tunersets view
	if (current == gui::TAB_VIEW::tunersets)
	{
		tunersetsView* view = reinterpret_cast<tunersetsView*>(this->view);
		auto state = view->currentState();

		gde = gui::GUI_DPORTS::Tunersets;
		flags = e2db::FPORTS::single_tunersets;
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
	// main view
	else if (current == gui::TAB_VIEW::main)
	{
		mainView* view = reinterpret_cast<mainView*>(this->view);
		auto state = view->currentState();

		// services
		if (state.tc == 0)
		{
			gde = gui::GUI_DPORTS::Services;
			flags = e2db::FPORTS::all_services;
			filename = "lamedb";

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
					for (string & fname : dbih->bouquets[filename].userbouquets)
						paths.push_back(fname);
				}
			}
			// userbouquet
			else
			{
				gde = gui::GUI_DPORTS::Userbouquets;
				flags = e2db::FPORTS::single_userbouquet;
			}
		}
	}
	// channelBook view
	else if (current == gui::TAB_VIEW::channelBook)
	{
		return infoMessage("Nothing to export", "You are in channel book.");
	}

	if (paths.empty())
	{
		return;
	}

	if (this->data->hasChanged())
	{
		updateIndex();
	}

	string path = gid->exportFileDialog(gde, filename, flags);

	if (path.empty())
	{
		return;
	}
	if (paths.size() > 0)
	{
		int dirsize = 0;
		string base;
		if (std::filesystem::is_directory(path)) //C++17
			base = path;
		else
			base = std::filesystem::path(path).parent_path().u8string(); //C++17
		std::filesystem::directory_iterator dirlist (base); //C++17
		for (const auto & entry : dirlist)
		{
			if (std::filesystem::is_regular_file(entry)) //C++17
				dirsize++;
		}
		if (dirsize != 0)
		{
			bool overwrite = saveQuestion("The destination contains files that will be overwritten.", "Do you want to overwrite them?");
			if (! overwrite)
				return;
		}
	}

	debug("exportFile()", "flags", flags);

	if (gde == gui::GUI_DPORTS::Services || gde == gui::GUI_DPORTS::Tunersets)
	{
		paths[0] = path;
	}
	else
	{
		string basedir = std::filesystem::path(path).remove_filename().u8string(); //C++17

		//TODO right-end trailing
		for (string & fname : paths)
			fname = basedir + fname;
	}

	QGuiApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
	dbih->exportFile(flags, paths);
	QGuiApplication::restoreOverrideCursor();

	if (statusBarIsVisible())
	{
		string path;
		if (paths.size() > 0)
			path = std::filesystem::path(path).remove_filename().u8string(); //C++17
		else
			path = paths[0];

		statusBarMessage("Exported to " + path);
	}
	else
	{
		infoMessage("Saved!");
	}
}

void tab::exportFile(QTreeWidgetItem* item)
{
	debug("exportFile()");

	auto* dbih = this->data->dbih;

	gui::TAB_VIEW current = getTabView();
	gui::GUI_DPORTS gde = gui::GUI_DPORTS::AllFiles;
	vector<string> paths;
	string filename;
	int bit = -1;

	if (item == NULL)
	{
		return;
	}
	// main view
	if (current == gui::TAB_VIEW::main)
	{
		mainView* view = reinterpret_cast<mainView*>(this->view);
		auto state = view->currentState();

		// services
		if (state.tc == 0)
		{
			gde = gui::GUI_DPORTS::Services;
			filename = "lamedb";

			paths.push_back(filename);
		}
		// bouquets
		else if (state.tc == 1)
		{
			int ti = view->tree->indexOfTopLevelItem(item);
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
					for (string & fname : dbih->bouquets[filename].userbouquets)
						paths.push_back(fname);
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
	// channelBook view
	else if (current == gui::TAB_VIEW::channelBook)
	{
		return infoMessage("Nothing to export", "You are in channel book.");
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
		string base;
		if (std::filesystem::is_directory(path)) //C++17
			base = path;
		else
			base = std::filesystem::path(path).parent_path().u8string(); //C++17
		std::filesystem::directory_iterator dirlist (base); //C++17
		for (const auto & entry : dirlist)
		{
			if (std::filesystem::is_regular_file(entry)) //C++17
				dirsize++;
		}
		if (dirsize != 0)
		{
			bool overwrite = saveQuestion("The destination contains files that will be overwritten.", "Do you want to overwrite them?");
			if (! overwrite)
				return;
		}
	}

	QGuiApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
	dbih->exportFile(bit, paths);
	QGuiApplication::restoreOverrideCursor();

	if (statusBarIsVisible())
	{
		string path;
		if (paths.size() > 0)
			path = std::filesystem::path(path).remove_filename().u8string(); //C++17
		else
			path = paths[0];

		statusBarMessage("Exported to " + path);
	}
	else
	{
		infoMessage("Saved!");
	}
}

void tab::printFile(bool all)
{
	debug("printFile()");

	gui::TAB_VIEW current = getTabView();
	printable* printer = new printable(this->cwid, this->data, this->log->log);

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
				QVariantMap tdata = item->data(0, Qt::UserRole).toMap();
				QString qchlist = tdata["id"].toString();
				string bname = qchlist.toStdString();

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
		statusBarMessage("Printing …");
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

	this->data->setChanged(true);
}

void tab::toolsExportToFile(TOOLS_FILE ftype, e2db::FCONVS fco)
{
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

void tab::ftpConnect()
{
	debug("ftpConnect()");

	if (this->ftph->handleConnection())
	{
		if (statusBarIsVisible())
			statusBarMessage("FTP connected successfully.");
		else
			infoMessage("Successfully connected!");
	}
	else
	{
		errorMessage("FTP Error", "Cannot connect to FTP Server!");
	}
}

void tab::ftpDisconnect()
{
	debug("ftpDisconnect()");

	if (this->ftph->closeConnection())
	{
		if (statusBarIsVisible())
			statusBarMessage("FTP disconnected successfully.");
		else
			infoMessage("Successfully disconnected!");
	}
	else
	{
		errorMessage("FTP Error", "Cannot disconnect from FTP Server!");
	}
}

//TODO improve for status bar
void tab::ftpUpload()
{
	debug("ftpUpload()");

	if (! this->ftph->handleConnection())
		return errorMessage("FTP Error", "Cannot connect to FTP Server!");

	auto* ftih = this->ftph->ftih;
	auto* dbih = this->data->dbih;

	unordered_map<string, e2db::e2db_file> files = dbih->get_output();

	if (files.empty())
		return;

	unordered_map<string, e2se_ftpcom::ftpcom::ftpcom_file> ftp_files;

	int profile_sel = gid->sets->value("profile/selected").toInt();
	gid->sets->beginReadArray("profile");
	gid->sets->setArrayIndex(profile_sel);
	for (auto & x : files)
	{
		string filename = x.first;
		string base;
		string path;

		if (filename.find(".tv") != string::npos || filename.find(".radio") != string::npos)
		{
			base = gid->sets->value("pathBouquets").toString().toStdString();
		}
		else if (filename == "satellites.xml" || filename == "terrestrial.xml" || filename == "cables.xml" || filename == "atsc.xml")
		{
			base = gid->sets->value("pathTransponders").toString().toStdString();
		}
		//TODO upload services, other data ... (eg. picons)
		else
		{
			base = gid->sets->value("pathServices").toString().toStdString();
		}
		path = base + '/' + filename;
		
		e2se_ftpcom::ftpcom::ftpcom_file file;
		file.filename = x.second.filename;
		file.data = x.second.data;
		file.mime = x.second.mime;
		file.size = x.second.size;
		ftp_files.emplace(path, file);

		debug("ftpUpload()", "file", base + '/' + file.filename + " | " + to_string(file.size));
	}
	gid->sets->endArray();
	files.clear();

	ftih->put_files(ftp_files);

	if (statusBarIsVisible())
		statusBarMessage("Uploaded " + to_string(files.size()) + " files");
	else
		infoMessage("Uploaded!");

	if (ftih->cmd_ifreload() || ftih->cmd_tnreload())
	{
		if (statusBarIsVisible())
			statusBarMessage("STB reload done.");
		else
			infoMessage("STB reloaded!");
	}
}

//TODO improve for status bar
void tab::ftpDownload()
{
	debug("ftpDownload()");

	if (! this->ftph->handleConnection())
		return errorMessage("FTP Error", "Cannot connect to FTP Server!");

	auto* ftih = this->ftph->ftih;
	auto* dbih = this->data->dbih;

	unordered_map<string, e2se_ftpcom::ftpcom::ftpcom_file> ftp_files = ftih->get_files();

	if (ftp_files.empty())
		return;

	unordered_map<string, e2db::e2db_file> files;

	for (auto & x : ftp_files)
	{
		e2db::e2db_file file;
		file.filename = x.second.filename;
		file.data = x.second.data;
		file.mime = x.second.mime;
		file.size = x.second.size;

		debug("ftpDownload()", "file", x.first + " | " + to_string(x.second.size));

		files.emplace(file.filename, file);
	}

	updateIndex();

	dbih->importBlob(files);

	view->reset();
	view->load();

	this->data->setChanged(true);
}

void tab::updateIndex()
{
	debug("updateIndex()");

	if (this->child)
	{
		parent->updateIndex();
	}
	view->updateIndex();
}

QTimer* tab::statusBarMessage(string text)
{
	gui::status msg;
	msg.info = true;
	msg.message = text;
	setStatusBar(msg);

	QTimer* timer = new QTimer(this->cwid);
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
		QTimer* timer = new QTimer(this->cwid);
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
	msg.setAttribute(Qt::WA_TranslucentBackground);

	msg.setText(title);
	msg.setInformativeText(text);
	msg.setStandardButtons(QMessageBox::Save | QMessageBox::Discard);
	msg.setDefaultButton(QMessageBox::Save);
	return (msg.exec() == QMessageBox::Save);
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

void tab::loadSeeds()
{
	if (gid->sets->contains("application/seeds"))
	{
		readFile(gid->sets->value("application/seeds").toString().toStdString());
	}
	else
	{
		gid->sets->setValue("application/seeds", "");

		QMessageBox::information(this->cwid, NULL, "For debugging purpose, set application.seeds absolute path under Settings > Advanced tab, then restart the software.");
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

QWidget* tab::toolBarWidget(QToolBar* toolbar, QWidget* widget)
{
	toolbar->addWidget(widget);
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
#ifndef Q_OS_MAC
	//TODO FIX
	if (! theme::isDefault())
		theme->dynamicStyleSheet(widget, "QToolBar { background: palette(mid) }");
#else
	theme->dynamicStyleSheet(widget, "QToolBar { border-style: solid; border-width: 1px 0 }");

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
