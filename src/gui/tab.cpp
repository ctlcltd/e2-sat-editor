/*!
 * e2-sat-editor/src/gui/tab.cpp
 *
 * @link https://github.com/ctlcltd/e2-sat-editor
 * @copyright e2 SAT Editor Team
 * @author Leonardo Laureti
 * @version 1.2.0
 * @license MIT License
 * @license GNU GPLv3 License
 */

#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <algorithm>
#include <filesystem>
#include <sstream>
#include <stdexcept>

#include <QtGlobal>
#include <QGuiApplication>
#include <QWindow>
#include <QList>
#include <QSettings>
#include <QStyle>
#include <QMessageBox>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFormLayout>
#include <QSplitter>
#include <QGroupBox>
#include <QMenu>
#include <QScrollArea>
#include <QClipboard>
#include <QMimeData>
#ifdef Q_OS_WIN
#include <QStyleFactory>
#include <QScrollBar>
#endif

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

using std::pair, std::stringstream, std::to_string, std::sort;

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

void tab::tabSwitch()
{
	debug("tabSwitch");

	retrieveFlags();
	view->updateStatusBar();
	view->updateStatusBar(true);

	view->update();
}

void tab::updateTabName(string path)
{
	debug("updateTabName");

	string filename;

	if (! path.empty())
		filename = std::filesystem::path(path).filename().u8string();

	gid->changeTabName(ttid, filename);

	for (auto & child : childs)
	{
		int ttid = child->getTabId();
		gid->changeTabName(ttid, filename);
	}
}

void tab::updateIndex()
{
	debug("updateIndex");

	if (this->child)
		parent->updateIndex();

	view->updateIndex();
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

// note: tab::statusBarMessage this (gui*) is 0x0 with deleted tab* QTimer [Qt5]
void tab::resetStatusBar(bool message)
{
	if (gid != nullptr)
		gid->resetStatusBar(message);
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
	toolBarSeparator(bottom_toolbar);
	if (QSettings().value("application/debug", DEBUG).toBool())
	{
		toolBarAction(bottom_toolbar, "§ Load seeds", [=]() { this->loadSeeds(); });
		toolBarAction(bottom_toolbar, "§ Reset", [=]() { this->newFile(); this->updateTabName(); });
	}
	toolBarSpacer(bottom_toolbar);
#ifndef E2SE_DEMO
	toolBarAction(bottom_toolbar, tr("Donate", "toolbar"), [=]() { this->linkToWebsite(1); });
#endif
	toolBarSeparator(bottom_toolbar);

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

void tab::propagateChanges()
{
	debug("propagateChanges");

	if (this->child)
	{
		parent->propagateChanges();
	}
	else
	{
		view->updateFromTab();

		for (auto & child : childs)
			child->view->updateFromTab();
	}
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
		file.origin = e2db::FORG::fileblob;
		file.path = q.path;
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
		updateTabName(path);
	}
	else
	{
		updateTabName();

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

			try
			{
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
			}
			catch (const std::filesystem::filesystem_error& err)
			{
				error("saveFile", tr("File Error", "error").toStdString(), e2se::logger::msg(e2se::logger::MSG::except_filesystem, err.what()));

				errorMessage(tr("File Error", "error"), tr("Error opening files.", "error"));

				return;
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
			updateTabName(path);

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
			infoMessage(tr("Nothing to import", "message"), tr("You are in Picons editor.", "message"));
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
			infoMessage(tr("Nothing to export", "message"), tr("You are in Picons editor.", "message"));
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

		try
		{
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
		}
		catch (const std::filesystem::filesystem_error& err)
		{
			error("exportFile", tr("File Error", "error").toStdString(), e2se::logger::msg(e2se::logger::MSG::except_filesystem, err.what()));

			errorMessage(tr("File Error", "error"), tr("Error opening files.", "error"));

			return;
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

void tab::infoFile()
{
	debug("infoFile");

	auto* dbih = this->data->dbih;

	Qt::WindowFlags wflags = Qt::Dialog;
	wflags |= Qt::CustomizeWindowHint;
	wflags |= Qt::WindowTitleHint;
	wflags |= Qt::WindowMinMaxButtonsHint;
	wflags &= ~Qt::WindowMaximizeButtonHint;
	wflags |= Qt::WindowCloseButtonHint;

	QDialog* dial = new QDialog(cwid, wflags);
	dial->setObjectName("fileinfo");
	dial->setWindowTitle(tr("File Information", "dialog"));
	dial->setMinimumSize(450, 520);
	theme->fix(dial);

#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
	dial->connect(dial, &QDialog::finished, [=]() { QTimer::singleShot(0, [=]() { delete dial; }); });
#else
	dial->connect(dial, &QDialog::finished, [=]() { delete dial; });
#endif

	QGridLayout* dfrm = new QGridLayout(dial);

	QFormLayout* dtform = new QFormLayout;

	QGroupBox* dtl0 = new QGroupBox(tr("File Info"));
	QFormLayout* dtf0 = new QFormLayout;
	dtf0->setFormAlignment(Qt::AlignLeading);
	dtf0->setFieldGrowthPolicy(QFormLayout::FieldsStayAtSizeHint);

	string fpath = this->data->getPath();
	string fname = std::filesystem::path(fpath).filename().u8string();

	int srcver = dbih->db.version;
	int dstver = 0;
	int lamedb_ver = dbih->get_lamedb_version();
	int zapit_ver = dbih->get_zapit_version();

	if (lamedb_ver != -1)
		dstver = 0x1220 + lamedb_ver;
	else if (zapit_ver != -1)
		dstver = 0x1010 + zapit_ver;

	QString fformat = gid->getFileFormatName(srcver);
	QString fconvert = "–";
	if (srcver != dstver)
		fconvert = gid->getFileFormatName(dstver);

	auto flist = dbih->get_input();

	QUrl qfpath = QUrl::fromLocalFile(QString::fromStdString(fpath));

	QLabel* dtf0fn = new QLabel;
	dtf0fn->setText(QString::fromStdString(fname));
	dtf0fn->setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::TextSelectableByKeyboard);
	platform::osLabel(dtf0fn);
	dtf0->addRow(QString(QApplication::layoutDirection() == Qt::RightToLeft ? ":%1" : "%1:").arg(tr("File name")), dtf0fn);

	QLabel* dtf0fp = new QLabel;
	if (qfpath.isLocalFile())
	{
		dtf0fp->setText(QString("<a href=\"%1\">%2</a>").arg(qfpath.toString()).arg(QString::fromStdString(fpath)));
#ifndef E2SE_DEMO
		dtf0fp->setOpenExternalLinks(true);
#endif
	}
	dtf0fp->setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::TextSelectableByKeyboard | Qt::LinksAccessibleByMouse | Qt::LinksAccessibleByKeyboard);
	platform::osLabel(dtf0fp);
	dtf0->addRow(QString(QApplication::layoutDirection() == Qt::RightToLeft ? ":%1" : "%1:").arg(tr("File path")), dtf0fp);

	QLabel* dtf0ff = new QLabel;
	dtf0ff->setText(fformat);
	dtf0ff->setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::TextSelectableByKeyboard);
	platform::osLabel(dtf0ff);
	dtf0->addRow(QString(QApplication::layoutDirection() == Qt::RightToLeft ? ":%1" : "%1:").arg(tr("File format")), dtf0ff);

	QLabel* dtf0fc = new QLabel;
	dtf0fc->setText(fconvert);
	dtf0fc->setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::TextSelectableByKeyboard);
	platform::osLabel(dtf0fc);
	dtf0->addRow(QString(QApplication::layoutDirection() == Qt::RightToLeft ? ":%1" : "%1:").arg(tr("File convert")), dtf0fc);

	QGroupBox* dtl1 = new QGroupBox(tr("Counters"));
	QFormLayout* dtf1 = new QFormLayout;
	dtf1->setFormAlignment(Qt::AlignLeading);
	dtf1->setFieldGrowthPolicy(QFormLayout::FieldsStayAtSizeHint);

	QLabel* dtf1tv = new QLabel;
	dtf1tv->setText(QString::number(int (dbih->index["chs:1"].size())));
	dtf1tv->setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::TextSelectableByKeyboard);
	platform::osLabel(dtf1tv);
	dtf1->addRow(QString(QApplication::layoutDirection() == Qt::RightToLeft ? ":%1" : "%1:").arg(tr("TV")), dtf1tv);

	QLabel* dtf1rd = new QLabel;
	dtf1rd->setText(QString::number(int (dbih->index["chs:2"].size())));
	dtf1rd->setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::TextSelectableByKeyboard);
	platform::osLabel(dtf1rd);
	dtf1->addRow(QString(QApplication::layoutDirection() == Qt::RightToLeft ? ":%1" : "%1:").arg(tr("Radio")), dtf1rd);

	QLabel* dtf1dt = new QLabel;
	dtf1dt->setText(QString::number(int (dbih->index["chs:0"].size())));
	dtf1dt->setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::TextSelectableByKeyboard);
	platform::osLabel(dtf1dt);
	dtf1->addRow(QString(QApplication::layoutDirection() == Qt::RightToLeft ? ":%1" : "%1:").arg(tr("Data")), dtf1dt);

	QLabel* dtf1tt = new QLabel;
	dtf1tt->setText(QString::number(int (dbih->index["chs"].size())));
	dtf1tt->setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::TextSelectableByKeyboard);
	platform::osLabel(dtf1tt);
	dtf1->addRow(QString(QApplication::layoutDirection() == Qt::RightToLeft ? ":%1" : "%1:").arg(tr("Total")), dtf1tt);

	QGroupBox* dtl2 = new QGroupBox(tr("File Tree"));
	QVBoxLayout* dtb2 = new QVBoxLayout;

	QTreeWidget* dtw2ft = new QTreeWidget;
	dtw2ft->setHeaderLabels({tr("Filename"), tr("Size"), tr("Origin")});
	dtw2ft->setColumnWidth(0, 200);
	dtw2ft->setColumnWidth(1, 100);
#ifdef Q_OS_WIN
	if (theme::absLuma() || ! theme::isDefault())
	{
		QStyle* style = QStyleFactory::create("fusion");
		dtw2ft->verticalScrollBar()->setStyle(style);
		dtw2ft->horizontalScrollBar()->setStyle(style);
	}
#endif

	QList<QTreeWidgetItem*> ftree;
	for (auto & x : flist)
	{
		QString filename = QString::fromStdString(x.second.filename);
		QString filesize = "–";
		QString fileorigin = "–";
		switch (x.second.origin)
		{
			case e2db::FORG::filesys: fileorigin = tr("local", "file"); break;
			case e2db::FORG::fileport: fileorigin = tr("remote", "file"); break;
			case e2db::FORG::fileblob: fileorigin = tr("blob", "file"); break;
		}
		{
			QLocale appLocale = gid->getLocale();
			filesize = appLocale.formattedDataSize(qint64 (x.second.size), 2, QLocale::DataSizeTraditionalFormat);
		}

		QTreeWidgetItem* item = new QTreeWidgetItem;
		item->setText(0, filename);
		item->setText(1, filesize);
		item->setText(2, fileorigin);
		ftree.append(item);
	}
	dtw2ft->addTopLevelItems(ftree);

	dtb2->addWidget(dtw2ft);

	dtl0->setLayout(dtf0);
	dtl1->setLayout(dtf1);
	dtl2->setLayout(dtb2);

	QString qss = "QLabel { min-width: 12ex; min-height: 20px }";
	dtl0->setStyleSheet(qss);
	dtl1->setStyleSheet(qss);

	dtform->addRow(dtl0);
	dtform->addRow(dtl1);
	dtform->addRow(dtl2);

	dfrm->setContentsMargins(0, 0, 0, 0);
	dfrm->addLayout(dtform, 0, 0);
	dial->setLayout(dfrm);
	dial->exec();
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
			filename += "-xml";

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

				filename = "services";
				switch (ti)
				{
					// TV
					case 1:
						stype = e2db::STYPE::tv;
						filename += "-tv";
					break;
					// Radio
					case 2:
						stype = e2db::STYPE::radio;
						filename += "-radio";
					break;
					// Data
					case 3:
						stype = e2db::STYPE::data;
						filename += "-data";
					break;
					// All Services
					default:
						stype = -1;
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
					std::transform(filename.begin(), filename.end(), filename.begin(), [](unsigned char c) { return c == '.' ? '-' : c; });
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
				filename = "index";
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
				filename = "tunersets-xml";
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
			toolsExportToFile(TOOLS_FILE::tools_html, e2db::FCONVS::convert_userbouquets);
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
	debug("ftpComboItems");

	if  (ftp_combo->count())
	{
		ftp_combo->clear();

		for (int i = 0; i < ftp_combo->count(); i++)
		{
			ftp_combo->removeItem(i);
		}
	}

	QSettings settings;

	int size = settings.beginReadArray("profile");
	for (int i = 0; i < size; i++)
	{
		settings.setArrayIndex(i);

		if (! settings.contains("profileName"))
			continue;

		ftp_combo->addItem(settings.value("profileName").toString(), i);
	}
	settings.endArray();

	int selected = settings.value("profile/selected", 0).toInt();
	ftp_combo->setCurrentIndex(selected);
}

void tab::ftpComboChanged(int index)
{
	debug("profileComboChanged", "index", index);

	this->ftph->closeConnection();

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
	QThread* thread = QThread::create([=]() {
		try
		{
			if (this->ftph->openConnection())
				QMetaObject::invokeMethod(this->cwid, [=]() { this->ftpStbConnectSuccessNotify(); }, Qt::QueuedConnection);
			else
				QMetaObject::invokeMethod(this->cwid, [=]() { this->ftpStbConnectErrorNotify(); }, Qt::QueuedConnection);
		}
		catch (std::runtime_error& err)
		{
			QMetaObject::invokeMethod(this->cwid, [=]() { this->ftpcomError(err.what()); }, Qt::QueuedConnection);

			return;
		}
		catch (...)
		{
			QMetaObject::invokeMethod(this->cwid, [=]() { this->ftpcomError(); }, Qt::QueuedConnection);

			return;
		}
	});
	thread->connect(thread, &QThread::started, [=]() {
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
	QThread* thread = QThread::create([=]() {
		try
		{
			if (this->ftph->closeConnection())
				QMetaObject::invokeMethod(this->cwid, [=]() { this->ftpStbDisconnectSuccessNotify(); }, Qt::QueuedConnection);
			else
				QMetaObject::invokeMethod(this->cwid, [=]() { this->ftpStbDisconnectErrorNotify(); }, Qt::QueuedConnection);
		}
		catch (std::runtime_error& err)
		{
			QMetaObject::invokeMethod(this->cwid, [=]() { this->ftpcomError(err.what()); }, Qt::QueuedConnection);

			return;
		}
		catch (...)
		{
			QMetaObject::invokeMethod(this->cwid, [=]() { this->ftpcomError(); }, Qt::QueuedConnection);

			return;
		}
	});
	thread->connect(thread, &QThread::started, [=]() {
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
		QThread* thread = QThread::create([=]() {
			try
			{
				if (! this->ftph->handleConnection())
					QMetaObject::invokeMethod(this->cwid, [=]() { this->ftpStbConnectErrorNotify(); }, Qt::QueuedConnection);
			}
			catch (std::runtime_error& err)
			{
				QMetaObject::invokeMethod(this->cwid, [=]() { this->ftpcomError(err.what()); }, Qt::QueuedConnection);

				return;
			}
			catch (...)
			{
				QMetaObject::invokeMethod(this->cwid, [=]() { this->ftpcomError(); }, Qt::QueuedConnection);

				return;
			}
		});
		thread->connect(thread, &QThread::started, [=]() {
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

	this->files.clear();
	this->ftp_files.clear();

	try
	{
		this->files = dbih->get_output();

		if (this->files.empty())
			return;
	}
	catch (std::runtime_error& err)
	{
		this->e2dbError(err.what());

		return;
	}
	catch (...)
	{
		this->e2dbError();

		return;
	}

	for (auto & x : this->files)
	{
		string filename = x.first;
		string basedir;
		string fpath;

		if (filename.find(".tv") != string::npos || filename.find(".radio") != string::npos)
			basedir = baseb;
		else if (filename == "satellites.xml" || filename == "terrestrial.xml" || filename == "cables.xml" || filename == "atsc.xml")
			basedir = baset;
		//TODO upload services, other data ... (eg. picons)
		else
			basedir = bases;

		if (basedir.size() && basedir[basedir.size() - 1] != '/')
			basedir.append("/");

		fpath = basedir + filename;

		e2se_ftpcom::ftpcom::ftpcom_file file;
		file.path = basedir + x.second.filename;
		file.filename = x.second.filename;
		file.mime = x.second.mime;
		file.data = x.second.data;
		file.size = x.second.size;

		this->ftp_files.emplace(fpath, file);

		debug("ftpUpload", "file path", file.path);
		debug("ftpUpload", "file size", to_string(file.size));
	}

	this->files.clear();
	this->ftp_errors.clear();

	{
		QThread* thread = QThread::create([=]() {
			if (this->ftp_files.empty())
				return;

			for (auto & x : this->ftp_files)
			{
				string fname = x.first;

				std::filesystem::path fpath = std::filesystem::path(fname);
				string basedir = fpath.parent_path().u8string();
				string filename = fpath.filename().u8string();

				QMetaObject::invokeMethod(this->cwid, [=]() { this->ftpStbUploadNotify(filename); }, Qt::QueuedConnection);

				try
				{
					ftih->upload_data(basedir, filename, x.second);
				}
				catch (std::runtime_error& err)
				{
					this->ftp_errors.emplace(fname, err.what());

					this->ftp_files.erase(filename);
				}
				catch (...)
				{
					this->ftp_files.clear();

					QMetaObject::invokeMethod(this->cwid, [=]() { this->ftpcomError(); }, Qt::QueuedConnection);

					return;
				}
			}
		});
		thread->connect(thread, &QThread::finished, [=]() {
			if (! this->ftp_errors.empty())
			{
				QMetaObject::invokeMethod(this->cwid, [=]() { this->ftpcomError(this->ftp_errors); }, Qt::QueuedConnection);
			}

			if (this->ftp_files.empty())
			{
				return;
			}

			int files_count = int (this->ftp_files.size());

			QMetaObject::invokeMethod(this->cwid, [=]() { this->ftpStbUploadNotify(files_count); }, Qt::QueuedConnection);

			QMetaObject::invokeMethod(this->cwid, [=]() { this->ftpReloadStb(); }, Qt::QueuedConnection);

			this->ftp_files.clear();
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
		QThread* thread = QThread::create([=]() {
			try
			{
				if (! this->ftph->handleConnection())
					QMetaObject::invokeMethod(this->cwid, [=]() { this->ftpStbConnectErrorNotify(); }, Qt::QueuedConnection);
			}
			catch (std::runtime_error& err)
			{
				QMetaObject::invokeMethod(this->cwid, [=]() { this->ftpcomError(err.what()); }, Qt::QueuedConnection);

				return;
			}
			catch (...)
			{
				QMetaObject::invokeMethod(this->cwid, [=]() { this->ftpcomError(); }, Qt::QueuedConnection);

				return;
			}
		});
		thread->connect(thread, &QThread::started, [=]() {
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

	this->files.clear();
	this->ftp_files.clear();
	this->ftp_errors.clear();

	{
		QThread* thread = QThread::create([=]() {
			try
			{
				ftih->fetch_paths();
			}
			catch (std::runtime_error& err)
			{
				QMetaObject::invokeMethod(this->cwid, [=]() { this->ftpcomError(err.what()); }, Qt::QueuedConnection);

				return;
			}
			catch (...)
			{
				QMetaObject::invokeMethod(this->cwid, [=]() { this->ftpcomError(); }, Qt::QueuedConnection);

				return;
			}

			for (string & fname : ftih->ftdb)
			{
				std::filesystem::path fpath = std::filesystem::path(fname);
				string basedir = fpath.parent_path().u8string();
				string filename = fpath.filename().u8string();

				QMetaObject::invokeMethod(this->cwid, [=]() { this->ftpStbDownloadNotify(filename); }, Qt::QueuedConnection);

				e2se_ftpcom::ftpcom::ftpcom_file file;

				try
				{
					ftih->download_data(basedir, filename, file);

					this->ftp_files[filename] = file;
				}
				catch (std::runtime_error& err)
				{
					this->ftp_errors.emplace(fname, err.what());
				}
				catch (...)
				{
					this->ftp_files.clear();

					QMetaObject::invokeMethod(this->cwid, [=]() { this->ftpcomError(); }, Qt::QueuedConnection);

					return;
				}
			}
		});
		thread->connect(thread, &QThread::finished, [=]() {
			if (! this->ftp_errors.empty())
			{
				QMetaObject::invokeMethod(this->cwid, [=]() { this->ftpcomError(this->ftp_errors); }, Qt::QueuedConnection);
			}

			if (this->ftp_files.empty())
			{
				return;
			}

			for (auto & x : this->ftp_files)
			{
				e2db::e2db_file file;
				file.origin = e2db::FORG::fileport;
				file.path = x.second.path;
				file.filename = x.second.filename;
				file.mime = x.second.mime;
				file.data = x.second.data;
				file.size = x.second.size;

				debug("ftpDownload", "file path", file.path);
				debug("ftpDownload", "file size", to_string(file.size));

				this->files.emplace(file.filename, file);
			}

			this->ftp_files.clear();

			int files_count = int (this->files.size());
			QMetaObject::invokeMethod(this->cwid, [=]() { this->ftpStbDownloadNotify(files_count); }, Qt::QueuedConnection);

			if (this->files.empty())
				return;

			try
			{
				dbih->importBlob(this->files, true);
			}
			catch(std::runtime_error& err)
			{
				this->files.clear();

				QMetaObject::invokeMethod(this->cwid, [=]() {
					//TODO TEST potential SEGFAULT
					// note: consecutive call SEGFAULT
					// this->newFile();

					this->e2dbError(err.what());
				}, Qt::QueuedConnection);

				return;
			}
			catch (...)
			{
				this->files.clear();

				QMetaObject::invokeMethod(this->cwid, [=]() {
					//TODO TEST potential SEGFAULT
					// note: consecutive call SEGFAULT
					// this->newFile();

					this->e2dbError();
				}, Qt::QueuedConnection);

				return;
			}

			QMetaObject::invokeMethod(this->cwid, [=]() {
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

	this->ftp_errors.clear();
	this->stb_reload = 1;

	QThread* thread = QThread::create([=]() {
		try
		{
			if (ftih->cmd_ifreload())
				this->stb_reload++;
		}
		catch (std::runtime_error& err)
		{
			this->ftp_errors.emplace("Webif", err.what());
		}
		catch (...)
		{
		}

		try
		{
			if (ftih->cmd_tnreload())
				this->stb_reload++;
		}
		catch (std::runtime_error& err)
		{
			this->ftp_errors.emplace("Telnet", err.what());
		}
		catch (...)
		{
		}
	});
	thread->connect(thread, &QThread::started, [=]() {
		QMetaObject::invokeMethod(this->cwid, [=]() { this->ftpStbReloadingNotify(); }, Qt::QueuedConnection);
	});
	thread->connect(thread, &QThread::finished, [=]() {
		if (this->stb_reload)
			QMetaObject::invokeMethod(this->cwid, [=]() { this->ftpStbReloadSuccessNotify(); }, Qt::QueuedConnection);

		if (! this->stb_reload || ! this->ftp_errors.empty())
			QMetaObject::invokeMethod(this->cwid, [=]() { this->ftpStbReloadErrorNotify(this->ftp_errors); }, Qt::QueuedConnection);
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
		if (this->ftph->isConnected())
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
	string hostname;

	try
	{
		hostname = this->ftph->getServerHostname();
	}
	catch (...)
	{
	}

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
	string hostname;

	try
	{
		hostname = this->ftph->getServerHostname();
	}
	catch (...)
	{
	}

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

void tab::ftpStbReloadErrorNotify(unordered_map<string, string> errors)
{
	string hostname;

	try
	{
		hostname = this->ftph->getServerHostname();
	}
	catch (...)
	{
	}

	error("ftpStbReloadErrorNotify", tr("STB Reload Error", "error").toStdString(), tr("Cannot reload STB \"%1\".", "error").arg(hostname.data()).toStdString());

	if (errors.empty())
		errorMessage(tr("STB Reload Error", "error"), tr("Errors occurred during STB reload operations.", "error"));
	else
		ftpcomError(errors, true);
}

void tab::ftpcomError()
{
	errorMessage(tr("FTP Error", "error"), tr("Errors occurred during FTP operations.", "error"));
}

void tab::ftpcomError(string error)
{
	stringstream ss (error);

	string optk, optv, fn;
	std::getline(ss, optk, '\t');
	std::getline(ss, optv, '\t');
	std::getline(ss, fn, '\t');

	QString qoptk = QString(optk.data());
	QString qoptv = QString(optv.data());

	QString title = tr(qoptk.toStdString().data(), "error");
	QString message = tr(qoptv.toStdString().data(), "error");

	title = title.toHtmlEscaped();
	message = message.replace("<", "&lt;").replace(">", "&gt;");

#ifndef Q_OS_MAC
	QString text = message;
#else
	QString text = QString("%1\n\n%2").arg(title).arg(message);
#endif

	QMessageBox::critical(this->cwid, title, text);
}

void tab::ftpcomError(unordered_map<string, string> errors, bool reload)
{
	QStringList errlist;

	for (auto & x : errors)
	{
		string filename = x.first;
		stringstream ss (x.second);

		string optk, optv, fn;
		std::getline(ss, optk, '\t');
		std::getline(ss, optv, '\t');
		std::getline(ss, fn, '\t');

		QString error = QString(QApplication::layoutDirection() == Qt::RightToLeft ? "(%4) %3 :%2 \"%1\"" : "\"%1\" %2: %3 (%4)").arg(filename.data()).arg(optk.data()).arg(optv.data()).arg(fn.data());

		errlist.append(error);
	}

	QString title, message;
	QString error_detailed = errlist.join("\n");

	if (! reload)
	{
		title = tr("FTP Notice", "error");
		message = QString("%1\n\n%2")
			.arg(tr("Successfull transfer.", "message"))
			.arg(tr("Errors occurred during FTP operations.", "error"));
	}
	else
	{
		title = tr("STB Reload Notice", "error");
		message = QString("%1\n\n%2")
			.arg(tr("STB reload done.", "message"))
			.arg(tr("Errors occurred during STB reload operations.", "error"));
	}

	title = title.toHtmlEscaped();
	message = message.replace("<", "&lt;").replace(">", "&gt;");
	error_detailed = error_detailed.replace("<", "&lt;").replace(">", "&gt;");

	QMessageBox msg = QMessageBox(this->cwid);

	msg.setIcon(QMessageBox::Warning);
	msg.setTextFormat(Qt::PlainText);
	msg.setText(title);
	msg.setInformativeText(message);
	msg.setDetailedText(error_detailed);
	// QRect pos = msg.geometry();
	// pos.moveCenter(QPoint(this->cwid->width() / 2, this->cwid->height() / 2));
	// msg.setGeometry(pos);
	msg.exec();
}

void tab::e2dbError()
{
	errorMessage(tr("File Error", "error"), tr("Errors occurred during parsing operations.", "error"));
}

void tab::e2dbError(string error)
{
	stringstream ss (error);

	string optk, optv, fn;
	std::getline(ss, optk, '\t');
	std::getline(ss, optv, '\t');
	std::getline(ss, fn, '\t');

	QString qoptk = QString(optk.data());
	QString qoptv = QString(optv.data());

	QString title = e2db::tr(qoptk.toStdString().data(), "error");
	QString message = e2db::tr(qoptv.toStdString().data(), "error");

	title = title.toHtmlEscaped();
	message = message.replace("<", "&lt;").replace(">", "&gt;");

#ifndef Q_OS_MAC
	QString text = message;
#else
	QString text = QString("%1\n\n%2").arg(title).arg(message);
#endif

	QMessageBox::critical(this->cwid, title, text);
}

void tab::linkToRepository(int page)
{
	gid->linkToRepository(page);
}

void tab::linkToWebsite(int page)
{
	gid->linkToWebsite(page);
}

void tab::linkToOnlineHelp(int page)
{
	gid->linkToOnlineHelp(page);
}

//TODO TEST potential SEGFAULT
QTimer* tab::statusBarMessage(QString message)
{
	gui::status msg;
	msg.info = true;
	msg.message = message.toStdString();
	setStatusBar(msg);

	QTimer* timer = new QTimer(widget);
	timer->setSingleShot(true);
	timer->setInterval(STATUSBAR_MESSAGE_TIMEOUT);
	timer->callOnTimeout([=]() { this->resetStatusBar(true); delete timer; });
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
		timer->callOnTimeout([=]() { this->resetStatusBar(true); delete timer; });
	}
	else
	{
		timer->stop();
		timer->setInterval(STATUSBAR_MESSAGE_DELAY);
		timer->start();
	}
}

bool tab::saveQuestion(QString title, QString message)
{
	title = title.toHtmlEscaped();
	message = message.replace("<", "&lt;").replace(">", "&gt;");

	QMessageBox msg = QMessageBox(this->cwid);

	msg.setWindowFlags(Qt::Sheet | Qt::MSWindowsFixedSizeDialogHint);
#ifdef Q_OS_MAC
	msg.setAttribute(Qt::WA_TranslucentBackground);
#endif

	msg.setIcon(QMessageBox::Question);
	msg.setTextFormat(Qt::PlainText);
	msg.setText(title);
	msg.setInformativeText(message);
	msg.setStandardButtons(QMessageBox::Save | QMessageBox::Discard);
	msg.setDefaultButton(QMessageBox::Save);

	return (msg.exec() == QMessageBox::Save);
}

bool tab::removeQuestion(QString title, QString message)
{
	if (! QSettings().value("preference/askConfirmation", false).toBool())
		return true;

	title = title.toHtmlEscaped();
	message = message.replace("<", "&lt;").replace(">", "&gt;");

	QMessageBox msg = QMessageBox(this->cwid);

	msg.setWindowFlags(Qt::Sheet | Qt::MSWindowsFixedSizeDialogHint);
#ifdef Q_OS_MAC
	msg.setAttribute(Qt::WA_TranslucentBackground);
#endif

	msg.setIcon(QMessageBox::Question);
	msg.setTextFormat(Qt::PlainText);
	msg.setText(title);
	msg.setInformativeText(message);
	msg.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
	msg.setDefaultButton(QMessageBox::Yes);

	return (msg.exec() == QMessageBox::Yes);
}

void tab::infoMessage(QString title)
{
	title = title.toHtmlEscaped();

	QMessageBox msg = QMessageBox(this->cwid);

	// msg.setWindowFlags(Qt::Popup);

	msg.setTextFormat(Qt::PlainText);
	msg.setText(title);
	// QRect pos = msg.geometry();
	// pos.moveCenter(QPoint(this->cwid->width() / 2, this->cwid->height() / 2));
	// msg.setGeometry(pos);
	msg.exec();
}

void tab::infoMessage(QString title, QString message)
{
	title = title.toHtmlEscaped();
	message = message.replace("<", "&lt;").replace(">", "&gt;");

	QMessageBox msg = QMessageBox(this->cwid);

	// msg.setWindowFlags(Qt::Popup);

	msg.setTextFormat(Qt::PlainText);
	msg.setText(title);
	msg.setInformativeText(message);
	// QRect pos = msg.geometry();
	// pos.moveCenter(QPoint(this->cwid->width() / 2, this->cwid->height() / 2));
	// msg.setGeometry(pos);
	msg.exec();
}

void tab::errorMessage(QString title, QString message)
{
	title = title.toHtmlEscaped();
	message = message.replace("<", "&lt;").replace(">", "&gt;");

#ifndef Q_OS_MAC
	QString text = message;
#else
	QString text = QString("%1\n\n%2").arg(title).arg(message);
#endif

	QMessageBox::critical(this->cwid, title, text);
}

void tab::demoMessage()
{
	QString text = tr("DEMO MODE", "message");

	text = text.replace("<", "&lt;").replace(">", "&gt;");

	QMessageBox::information(this->cwid, NULL, text);
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
			file.path = path.toStdString();
			file.filename = path.remove(0, 23).toStdString();

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
