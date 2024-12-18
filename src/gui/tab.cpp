/*!
 * e2-sat-editor/src/gui/tab.cpp
 *
 * @link https://github.com/ctlcltd/e2-sat-editor
 * @copyright e2 SAT Editor Team
 * @author Leonardo Laureti
 * @version 1.8.0
 * @license MIT License
 * @license GNU GPLv3 License
 */

#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <algorithm>
#include <filesystem>
#include <sstream>

// note: std::runtime_error behaviour
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
#include <QTreeWidget>
#include <QHeaderView>
#include <QMenu>
#include <QScrollArea>
#include <QClipboard>
#include <QMimeData>
#include <QMouseEvent>
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

	bool found = false;
	vector<tab*>::iterator pos;
	for (auto it = childs.begin(); it != childs.end(); it++)
	{
		if (*it == child)
		{
			found = true;
			pos = it;
			break;
		}
	}
	if (found && pos != childs.end())
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

int tab::getTabArgument()
{
	return this->tty;
}

void tab::tabSwitch()
{
	debug("tabSwitch");

	retrieveFlags();

	view->updateFlags();
	view->updateStatusBar();
	view->updateStatusBar(true);

	view->update();
}

bool tab::hasChanged()
{
	debug("hasChanged");

	if (this->data != nullptr)
	{
		return this->data->hasChanged();
	}

	return false;
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

// note: tab::statusBarMessage this (gui*) is 0x0 with deleted tab* QTimer
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
	this->tty = ytype;

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

void tab::reload()
{
	debug("reload");

	view->reload();

	for (auto & child : childs)
		child->view->reload();
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
	ftp_combo->connect(ftp_combo, &QComboBox::currentIndexChanged, [=](int index) {
#else
	ftp_combo->connect(ftp_combo, QOverload<int>::of(&QComboBox::currentIndexChanged), [=](int index) {
#endif
		this->ftpComboChanged(index);
	});
	platform::osComboBox(ftp_combo);

	tbars[gui::FileOpen] = toolBarAction(top_toolbar, tr("&Open", "toolbar"), theme->dynamicIcon("file-open"), [=]() { this->openFile(); }, QKeySequence::Open);
	tbars[gui::FileSave] = toolBarAction(top_toolbar, tr("&Save", "toolbar"), theme->dynamicIcon("file-save"), [=]() { this->saveFile(false); }, QKeySequence::Save);
	toolBarSeparator(top_toolbar);
	tbars[gui::FileImport] = toolBarAction(top_toolbar, tr("Import", "toolbar"), theme->dynamicIcon("file-import"), [=]() { this->importFile(); });
	tbars[gui::FileExport] = toolBarAction(top_toolbar, tr("Export", "toolbar"), theme->dynamicIcon("file-export"), [=]() { this->exportFile(); });
	toolBarSeparator(top_toolbar);
	toolBarAction(top_toolbar, tr("Settings", "toolbar"), theme->dynamicIcon("settings"), [=]() { this->settingsDialog(); });
	toolBarSpacer(top_toolbar);
	toolBarWidget(top_toolbar, tr("Select profile", "toolbar"), ftp_combo, [=]() { ftp_combo->showPopup(); });
	toolBarAction(top_toolbar, tr("Connect", "toolbar"), [=]() { this->ftpConnect(); });
	toolBarAction(top_toolbar, tr("Disconnect", "toolbar"), [=]() { this->ftpDisconnect(); });
	toolBarSeparator(top_toolbar);
	toolBarAction(top_toolbar, tr("Upload", "toolbar"), [=]() { this->ftpUpload(); });
	toolBarAction(top_toolbar, tr("Download", "toolbar"), [=]() { this->ftpDownload(); });

	bool DEMO = false;
#ifdef E2SE_DEMO
	DEMO = true;
#endif

	toolBarEnding(bottom_toolbar);
	QAction* btt_action = new QAction(bottom_toolbar);
	btt_action->setText(tr("Tools", "toolbar"));
	btt_action->connect(btt_action, &QAction::triggered, [=]() {
		QMenu* menu = this->toolsMenu();

		QWidget* wid = bottom_toolbar->widgetForAction(btt_action);
		QPoint pos = wid->mapFrom(bottom_toolbar, wid->pos());
		this->lastPopupFocusWidget(wid, pos);
		platform::osMenuPopup(menu, wid, pos);
	});
	bottom_toolbar->addAction(btt_action);

	toolBarSeparator(bottom_toolbar);
	toolBarAction(bottom_toolbar, tr("Inspect", "toolbar"), [=]() { this->toolsErrorChecker(); });
	if (QSettings().value("application/debug", false).toBool() || DEMO)
	{
		toolBarAction(bottom_toolbar, "§ Load seeds", [=]() { this->loadSeeds(); });
		toolBarAction(bottom_toolbar, "§ Reset", [=]() { this->newFile(); this->updateTabName(); });
	}
	toolBarSpacer(bottom_toolbar);
	toolBarAction(bottom_toolbar, tr("autofix", "toolbar"), [=]() { this->toolsAutofixMacro(); });
#ifndef E2SE_DEMO
	toolBarSeparator(bottom_toolbar);
	toolBarAction(bottom_toolbar, tr("Donate", "toolbar"), [=]() { this->linkToWebsite(1); });
#endif
	toolBarEnding(bottom_toolbar);

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

	if (this->data->hasChanged())
	{
		int replacing = saveQuestion();

		if (replacing == QMessageBox::Save)
			saveFile(false);

		if (replacing == QMessageBox::Cancel)
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

	this->data->clearErrors();

#ifndef Q_OS_WASM
	theme::setWaitCursor();
	bool read = this->data->readFile(path);
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

		debug("readFile", "file path", q.path);
		debug("readFile", "file size", to_string(q.size));

		files.emplace(file.filename, file);
	}

	bool read = this->data->readBlob(path, files);
#endif

	if (read)
	{
		updateTabName(path);

		if (this->data->haveErrors())
			QMetaObject::invokeMethod(this->cwid, [=]() { this->e2dbError(this->data->getErrors(), MSG_CODE::readNotice); }, Qt::QueuedConnection);
	}
	else
	{
		updateTabName();

		error("readFile", tr("File Error", "error").toStdString(), tr("Error reading file \"%1\".", "error").arg(path.data()).toStdString());

		if (this->data->haveErrors())
			QMetaObject::invokeMethod(this->cwid, [=]() { this->e2dbError(this->data->getErrors(), MSG_CODE::readNotice); }, Qt::QueuedConnection);
		else
			QMetaObject::invokeMethod(this->cwid, [=]() { this->e2dbError(tr("File Error", "error"), tr("Error opening files.", "error")); }, Qt::QueuedConnection);

		if (statusBarIsVisible())
			statusBarMessage(timer);

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

	this->data->clearErrors();

	string path = this->data->getPath();
	string nw_path;

	if (this->data->hasChanged())
		updateIndex();

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
				bool overwriting = overwriteQuestion();
				if (! overwriting)
					return;
			}
		}
	}

	if (path.empty())
	{
		error("saveFile", tr("File Error", "error").toStdString(), tr("Empty path.", "error").toStdString());

		return;
	}

	debug("saveFile", "path", path);

	theme::setWaitCursor();
	bool write = this->data->writeFile(path);
	theme::unsetWaitCursor();

	if (write)
	{
		this->data->setChanged(false);

		if (saveas)
			updateTabName(path);

		if (statusBarIsVisible())
			statusBarMessage(tr("Saved to %1", "message").arg(path.data()));
		else
			QMetaObject::invokeMethod(this->cwid, [=]() { infoMessage(tr("Saved!", "message")); }, Qt::QueuedConnection);
	}
	else
	{
		error("saveFile", tr("File Error", "error").toStdString(), tr("Error writing file \"%1\".", "error").arg(path.data()).toStdString());

		if (this->data->haveErrors())
			QMetaObject::invokeMethod(this->cwid, [=]() { this->e2dbError(this->data->getErrors(), MSG_CODE::writeNotice); }, Qt::QueuedConnection);
		else
			QMetaObject::invokeMethod(this->cwid, [=]() { this->e2dbError(tr("File Error", "error"), tr("Error writing files.", "error")); }, Qt::QueuedConnection);
	}
}

void tab::importFile()
{
	debug("importFile");

#ifdef E2SE_DEMO
	return this->demoMessage();
#endif

	auto* dbih = this->data->dbih;

	this->data->clearErrors();

	gui::TAB_VIEW current = getTabView();
	gui::GUI_DPORTS gde = gui::GUI_DPORTS::AllFiles;
	vector<string> paths;
	int bit = -1;

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

	paths = gid->importFileDialog(gde, bit);

	if (paths.empty())
		return;

	// directories
	if (bit == 0x0001)
	{
		for (string & path : paths)
			path = std::filesystem::path(path).parent_path().u8string();
	}

	if (statusBarIsVisible())
	{
		string filename;
		if (paths.size() > 1)
			filename = std::filesystem::path(paths[0]).parent_path().u8string();
		else
			filename = paths[0];

		statusBarMessage(tr("Importing from %1 …", "message").arg(filename.data()));
	}

	theme::setWaitCursor();
	dbih->importFile(paths);
	theme::unsetWaitCursor();

	if (this->data->haveErrors())
		QMetaObject::invokeMethod(this->cwid, [=]() { this->e2dbError(this->data->getErrors(), MSG_CODE::importNotice); }, Qt::QueuedConnection);

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

	this->data->clearErrors();

	gui::TAB_VIEW current = getTabView();
	gui::GUI_DPORTS gde = gui::GUI_DPORTS::AllFiles;
	vector<string> paths;
	string fname;
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
				fname = "satellites.xml";
			break;
			case e2db::YTYPE::terrestrial:
				fname = "terrestrial.xml";
			break;
			case e2db::YTYPE::cable:
				fname = "cables.xml";
			break;
			case e2db::YTYPE::atsc:
				fname = "atsc.xml";
			break;
		}

		paths.push_back(fname);
	}
	// transponders view
	else if (current == gui::TAB_VIEW::transponders)
	{
		gde = gui::GUI_DPORTS::Services;
		if (dbtype == 0)
		{
			int ver = dbih->get_zapit_version();

			bit = e2db::FPORTS::all_services;
			fname = ver > 4 ? "lamedb5" : "lamedb";
		}
		else
		{
			bit = e2db::FPORTS::all_services_xml;
			fname = "services.xml";
		}

		paths.push_back(fname);
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
				fname = ver > 4 ? "lamedb5" : "lamedb";
			}
			else
			{
				bit = e2db::FPORTS::all_services_xml;
				fname = "services.xml";
			}

			paths.push_back(fname);
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
				fname = paths[0];
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

					if (dbih->bouquets.count(fname))
					{
						for (string & fname : dbih->bouquets[fname].userbouquets)
							paths.push_back(fname);
					}
				}
				else
				{
					int ver = dbih->get_zapit_version();

					bit = e2db::FPORTS::all_bouquets_xml;
					fname = ver != 1 ? "ubouquets.xml" : "bouquets.xml";

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
		updateIndex();

	string path = gid->exportFileDialog(gde, fname, bit);

	if (path.empty())
	{
		return;
	}
	if (paths.size() > 1)
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
			bool overwriting = overwriteQuestion();
			if (! overwriting)
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
	dbih->exportFile(bit, paths, path);
	theme::unsetWaitCursor();

	if (this->data->haveErrors())
		QMetaObject::invokeMethod(this->cwid, [=]() { this->e2dbError(this->data->getErrors(), MSG_CODE::exportNotice); }, Qt::QueuedConnection);

	if (statusBarIsVisible())
	{
		string filename;
		if (paths.size() > 1)
			filename = std::filesystem::path(path).parent_path().u8string();
		else
			filename = path;

		statusBarMessage(tr("Exported to %1", "message").arg(QString::fromStdString(filename)));
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

#ifdef Q_OS_WIN
	theme->win_flavor_fix(dial);
#endif

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

	string filepath = this->data->getPath();
	string filename = std::filesystem::path(filepath).filename().u8string();

	int srctype = dbih->get_e2db_services_type();
	int srcver = dbih->get_e2db_services_version();
	int dstver = 0;
	int lamedb_ver = dbih->get_lamedb_version();
	int zapit_ver = dbih->get_zapit_version();

	if (srctype == 0 && lamedb_ver != -1)
		dstver = 0x1220 + lamedb_ver;
	else if (srctype == 1 && zapit_ver != -1)
		dstver = 0x1010 + zapit_ver;

	QString fformat = gid->getFileFormatName(srcver);
	QString fconvert = "–";
	if (dstver != 0 && srcver != dstver)
		fconvert = gid->getFileFormatName(dstver);

	QString fpath = QString::fromStdString(filepath);
	QString fname = QString::fromStdString(filename);
	QUrl furl = QUrl::fromLocalFile(fpath);

	QLabel* dtf0fn = new QLabel;
	if (fname.size() > 54)
	{
		dtf0fn->setToolTip(fname);
		dtf0fn->setMaximumWidth(330);
	}
	dtf0fn->setText(fname);
	dtf0fn->setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::TextSelectableByKeyboard);
	platform::osLabel(dtf0fn);
	dtf0->addRow(QString(QApplication::layoutDirection() == Qt::RightToLeft ? ":%1" : "%1:").arg(tr("File name")), dtf0fn);

	QLabel* dtf0fp = new QLabel;
	if (furl.isLocalFile())
	{
		if (fpath.size() > 54)
		{
			dtf0fp->setToolTip(fpath);
			dtf0fp->setMaximumWidth(330);
		}
		dtf0fp->setText(QString("<a href=\"%1\">%2</a>").arg(furl.toString()).arg(fpath));
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

	QLabel* dtf1bt = new QLabel;
	dtf1bt->setText(QString::number(int (dbih->index["ubs"].size())));
	dtf1bt->setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::TextSelectableByKeyboard);
	platform::osLabel(dtf1bt);
	dtf1->addRow(QString(QApplication::layoutDirection() == Qt::RightToLeft ? ":%1" : "%1:").arg(tr("Bouquets")), dtf1bt);

	QGroupBox* dtl2 = new QGroupBox(tr("File Tree"));
	QVBoxLayout* dtb2 = new QVBoxLayout;

	QTreeWidget* dtw2ft = new QTreeWidget;
	dtw2ft->setHeaderLabels({tr("Filename"), tr("Size"), tr("Origin"), tr("Time")});
	dtw2ft->setSortingEnabled(true);
#if QT_VERSION >= QT_VERSION_CHECK(6, 6, 0)
	dtw2ft->header()->setDefaultSectionSize(0);
#endif
	dtw2ft->setColumnWidth(0, 200);
	dtw2ft->setColumnWidth(1, 100);
	dtw2ft->setColumnWidth(2, 75);
	dtw2ft->setColumnWidth(3, 150);
#ifdef Q_OS_WIN
	if (! theme::isOverridden() && (theme::isFluentWin() || theme::absLuma() || ! theme::isDefault()))
	{
		QStyle* style;

		if (theme::isFluentWin())
			style = QStyleFactory::create("windows11");
		else
			style = QStyleFactory::create("fusion");

		dtw2ft->verticalScrollBar()->setStyle(style);
		dtw2ft->horizontalScrollBar()->setStyle(style);
		dtw2ft->header()->setStyle(style);
	}
#endif

	auto flist = dbih->get_file_list();
	QLocale appLocale = gid->getLocale();

	QList<QTreeWidgetItem*> ftree;
	for (auto & x : flist)
	{
		QString filename = QString::fromStdString(x.filename);
		QString filesize = appLocale.formattedDataSize(qint64 (x.size), 2, QLocale::DataSizeTraditionalFormat);
		QString fileorigin = "–";
		QString filetime = QString::fromStdString(x.t).section(' ', 1);
		switch (x.origin)
		{
			case e2db::FORG::filesys: fileorigin = tr("local", "file"); break;
			case e2db::FORG::fileport: fileorigin = tr("remote", "file"); break;
			case e2db::FORG::fileblob: fileorigin = tr("blob", "file"); break;
		}

		QTreeWidgetItem* item = new QTreeWidgetItem;
		item->setText(0, filename);
		item->setText(1, filesize);
		item->setText(2, fileorigin);
		item->setText(3, filetime);
		ftree.append(item);
	}
	dtw2ft->addTopLevelItems(ftree);
	dtw2ft->sortItems(3, Qt::AscendingOrder);

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

void tab::convertFormat(gui::GUI_CXE bit)
{
	int x = 0;

	switch (bit)
	{
		case gui::GUI_CXE::FileConvertLamedb24: x = e2db::FPORTS::all_services__2_4; break;
		case gui::GUI_CXE::FileConvertLamedb25: x = e2db::FPORTS::all_services__2_5; break;
		case gui::GUI_CXE::FileConvertLamedb23: x = e2db::FPORTS::all_services__2_3; break;
		case gui::GUI_CXE::FileConvertLamedb22: x = e2db::FPORTS::all_services__2_2; break;
		case gui::GUI_CXE::FileConvertZapit4: x = e2db::FPORTS::all_services_xml__4; break;
		case gui::GUI_CXE::FileConvertZapit3: x = e2db::FPORTS::all_services_xml__3; break;
		case gui::GUI_CXE::FileConvertZapit2: x = e2db::FPORTS::all_services_xml__2; break;
		case gui::GUI_CXE::FileConvertZapit1: x = e2db::FPORTS::all_services_xml__1; break;
		default: return;
	}

	convertFormat(x);
}

void tab::convertFormat(int bit)
{
	debug("convertFormat", "bit", bit);

	auto* dbih = this->data->dbih;

	e2db::FPORTS fpx = static_cast<e2db::FPORTS>(bit);
	gui::GUI_CXE dstbit;

	int srctype = dbih->get_e2db_services_type();
	int srcver = dbih->get_e2db_services_version();
	int lamedb_ver = dbih->get_lamedb_version();
	int zapit_ver = dbih->get_zapit_version();

	if (srctype == 0)
	{
		if (lamedb_ver == -1)
			lamedb_ver = srcver - 0x1220;

		debug("convertFormat", "from Lamedb version", lamedb_ver);
	}
	else if (srctype == 1)
	{
		if (zapit_ver == -1)
			zapit_ver = srcver - 0x1010;

		debug("convertFormat", "from Zapit version", zapit_ver);
	}

	switch (fpx)
	{
		case e2db::FPORTS::all_services__2_4:
			dbih->set_e2db_services_type(0);
			dbih->set_lamedb_version(4);
			dstbit = gui::GUI_CXE::FileConvertLamedb24;
		break;
		case e2db::FPORTS::all_services__2_5:
			dbih->set_e2db_services_type(0);
			dbih->set_lamedb_version(5);
			dstbit = gui::GUI_CXE::FileConvertLamedb25;
		break;
		case e2db::FPORTS::all_services__2_3:
			dbih->set_e2db_services_type(0);
			dbih->set_lamedb_version(3);
			dstbit = gui::GUI_CXE::FileConvertLamedb23;
		break;
		case e2db::FPORTS::all_services__2_2:
			dbih->set_e2db_services_type(0);
			dbih->set_lamedb_version(2);
			dstbit = gui::GUI_CXE::FileConvertLamedb22;
		break;
		case e2db::FPORTS::all_services_xml__4:
			dbih->set_e2db_services_type(1);
			dbih->set_zapit_version(4);
			dstbit = gui::GUI_CXE::FileConvertZapit4;
		break;
		case e2db::FPORTS::all_services_xml__3:
			dbih->set_e2db_services_type(1);
			dbih->set_zapit_version(3);
			dstbit = gui::GUI_CXE::FileConvertZapit3;
		break;
		case e2db::FPORTS::all_services_xml__2:
			dbih->set_e2db_services_type(1);
			dbih->set_zapit_version(2);
			dstbit = gui::GUI_CXE::FileConvertZapit2;
		break;
		case e2db::FPORTS::all_services_xml__1:
			dbih->set_e2db_services_type(1);
			dbih->set_zapit_version(1);
			dstbit = gui::GUI_CXE::FileConvertZapit1;
		break;
		default:
			return;
	}

	int dsttype = dbih->get_e2db_services_type();
	int dstver = bit;
	lamedb_ver = dbih->get_lamedb_version();
	zapit_ver = dbih->get_zapit_version();

	if (dsttype == 0)
	{
		if (lamedb_ver == -1)
			lamedb_ver = dstver - 0x1220;

		debug("convertFormat", "to Lamedb version", lamedb_ver);
	}
	else if (dsttype == 1)
	{
		if (zapit_ver == -1)
			zapit_ver = dstver - 0x1010;

		debug("convertFormat", "to Zapit version", zapit_ver);
	}

	vector<int> bits = {72, 73, 74, 75, 76, 77, 78, 79};

	for (int & bit : bits)
		setFlag(gui::GUI_CXE (bit), bit == dstbit);

	storeFlags();

	view->updateStatusBar();
}

void tab::settingsDialog()
{
	debug("settingsDialog");

	gid->settingsDialog();
}

void tab::lastPopupFocusWidget(QWidget* wid, QPoint pos)
{
	this->popup_wid = wid;
	this->popup_pos = pos;
}

QWidget* tab::lastPopupFocusWidget()
{
	return this->popup_wid;
}

QPoint tab::lastPopupFocusPos()
{
	return this->popup_pos;
}

QMenu* tab::toolsMenu()
{
	if (this->tools_menu == nullptr)
	{
		QMenu* menu = this->tools_menu = new QMenu;

		QMenu* tmclean = menuMenu(menu, tr("Clean", "menu"));
		menuAction(tmclean, tr("Remove orphaned services", "menu"), [=]() { this->toolsUtils(gui::TAB_ATS::UtilsOrphaned_services); });
		menuAction(tmclean, tr("Remove orphaned references", "menu"), [=]() { this->toolsUtils(gui::TAB_ATS::UtilsOrphaned_references); });
		menuAction(tmclean, tr("Fix bouquets", "menu"), [=]() { this->toolsUtils(gui::TAB_ATS::UtilsFixBouquets); });
		menuAction(tmclean, tr("Fix bouquets unique userbouquets", "menu"), [=]() { this->toolsUtils(gui::TAB_ATS::UtilsFixBouquetsUniq); });
		menuAction(tmclean, tr("Fix (remove) reference with errors", "menu"), [=]() { this->toolsUtils(gui::TAB_ATS::UtilsFixRemove); });
		QMenu* tmparams = menuMenu(menu, tr("Params", "menu"));
		menuAction(tmparams, tr("Remove service cached", "menu"), [=]() { this->toolsUtils(gui::TAB_ATS::UtilsClearServicesCached); });
		menuAction(tmparams, tr("Remove service CAID", "menu"), [=]() { this->toolsUtils(gui::TAB_ATS::UtilsClearServicesCAID); });
		menuAction(tmparams, tr("Remove service flags", "menu"), [=]() { this->toolsUtils(gui::TAB_ATS::UtilsClearServicesFlags); });
		menuAction(tmparams, tr("Remove all service data", "menu"), [=]() { this->toolsUtils(gui::TAB_ATS::UtilsClearServicesData); });
		menuSeparator(tmparams);
		menuAction(tmparams, tr("Recalculate DVBNS for services", "menu"), [=]() { this->toolsUtils(gui::TAB_ATS::UtilsFixDVBNS); });
		QMenu* tmrmove = menuMenu(menu, tr("Remove", "menu"));
		menuAction(tmrmove, tr("Remove unreferenced entries (favourites)", "menu"), [=]() { this->toolsUtils(gui::TAB_ATS::UtilsClearFavourites); });
		menuAction(tmrmove, tr("Remove from bouquets (unused services)", "menu"), [=]() { this->toolsUtils(gui::TAB_ATS::UtilsClearBouquetsUnused); });
		menuSeparator(tmrmove);
		menuAction(tmrmove, tr("Remove parental lock", "menu"), [=]() { this->toolsUtils(gui::TAB_ATS::UtilsRemove_parentallock); });
		menuAction(tmrmove, tr("Remove all bouquets", "menu"), [=]() { this->toolsUtils(gui::TAB_ATS::UtilsRemove_bouquets); });
		menuAction(tmrmove, tr("Remove all userbouquets", "menu"), [=]() { this->toolsUtils(gui::TAB_ATS::UtilsRemove_userbouquets); });
		QMenu* tmdups = menuMenu(menu, tr("Duplicates", "menu"));
		menuAction(tmdups, tr("Remove duplicate markers (names)", "menu"), [=]() { this->toolsUtils(gui::TAB_ATS::UtilsDuplicates_markers); });
		menuAction(tmdups, tr("Remove duplicate references", "menu"), [=]() { this->toolsUtils(gui::TAB_ATS::UtilsDuplicates_references); });
		menuAction(tmdups, tr("Remove duplicate services", "menu"), [=]() { this->toolsUtils(gui::TAB_ATS::UtilsDuplicates_services); });
		menuAction(tmdups, tr("Remove duplicate transponders", "menu"), [=]() { this->toolsUtils(gui::TAB_ATS::UtilsDuplicates_transponders); });
		menuAction(tmdups, tr("Remove all duplicates", "menu"), [=]() { this->toolsUtils(gui::TAB_ATS::UtilsDuplicates_all); });
		QMenu* tmtsform = menuMenu(menu, tr("Transform", "menu"));
		menuAction(tmtsform, tr("Transform transponders to XML settings", "menu"), [=]() { this->toolsUtils(gui::TAB_ATS::UtilsTransform_tunersets); });
		menuAction(tmtsform, tr("Transform XML settings to transponders", "menu"), [=]() { this->toolsUtils(gui::TAB_ATS::UtilsTransform_transponders); });
		QMenu* tmsort = menuMenu(menu, tr("Sort", "menu"));
		menuAction(tmsort, tr("Sort references…", "menu"), [=]() { this->toolsUtils(gui::TAB_ATS::UtilsSort_references); });
		menuAction(tmsort, tr("Sort services…", "menu"), [=]() { this->toolsUtils(gui::TAB_ATS::UtilsSort_services); });
		menuAction(tmsort, tr("Sort transponders…", "menu"), [=]() { this->toolsUtils(gui::TAB_ATS::UtilsSort_transponders); });
		menuAction(tmsort, tr("Sort userbouquets…", "menu"), [=]() { this->toolsUtils(gui::TAB_ATS::UtilsSort_userbouquets); });
		menuSeparator(menu);
		menuAction(menu, tr("Error Checker", "menu"), [=]() { this->toolsErrorChecker(); }, Qt::CTRL | Qt::ALT | Qt::Key_I);
		menuAction(menu, tr("Autofix", "menu"), [=]() { this->toolsAutofixMacro(); }, Qt::CTRL | Qt::ALT | Qt::Key_A);
		menuSeparator(menu);
		QMenu* tmimport = menuMenu(menu, tr("Import", "menu"));
		menuAction(tmimport, tr("Import…", "menu"), [=]() { this->importFile(); });
		menuSeparator(tmimport);
		QMenu* tmimportcsv = menuMenu(tmimport, tr("Import from CSV", "menu"));
		menuAction(tmimportcsv, tr("Import Services", "menu"), [=]() { this->toolsImportFromFile(TOOLS_FILE::tools_csv, e2db::FCONVS::convert_services); });
		menuAction(tmimportcsv, tr("Import Bouquet", "menu"), [=]() { this->toolsImportFromFile(TOOLS_FILE::tools_csv, e2db::FCONVS::convert_bouquets); });
		menuAction(tmimportcsv, tr("Import Userbouquet", "menu"), [=]() { this->toolsImportFromFile(TOOLS_FILE::tools_csv, e2db::FCONVS::convert_userbouquets); });
		menuAction(tmimportcsv, tr("Import Tuner settings", "menu"), [=]() { this->toolsImportFromFile(TOOLS_FILE::tools_csv, e2db::FCONVS::convert_tunersets); });
		menuAction(tmimport, tr("Import from M3U", "menu"), [=]() { this->toolsImportFromFile(TOOLS_FILE::tools_m3u, e2db::FCONVS::convert_all); });
		QMenu* tmexport = menuMenu(menu, tr("Export", "menu"));
		menuAction(tmexport, tr("Export…", "menu"), [=]() { this->exportFile(); });
		menuSeparator(tmexport);
		QMenu* tmexportcsv = menuMenu(tmexport, tr("Export to CSV", "menu"));
		menuAction(tmexportcsv, tr("Export current", "menu"), [=]() { this->toolsExportToFile(TOOLS_FILE::tools_csv, e2db::FCONVS::convert_current); });
		menuAction(tmexportcsv, tr("Export All", "menu"), [=]() { this->toolsExportToFile(TOOLS_FILE::tools_csv, e2db::FCONVS::convert_all); });
		menuAction(tmexportcsv, tr("Export Services", "menu"), [=]() { this->toolsExportToFile(TOOLS_FILE::tools_csv, e2db::FCONVS::convert_services); });
		menuAction(tmexportcsv, tr("Export Bouquets", "menu"), [=]() { this->toolsExportToFile(TOOLS_FILE::tools_csv, e2db::FCONVS::convert_bouquets); });
		menuAction(tmexportcsv, tr("Export Userbouquets", "menu"), [=]() { this->toolsExportToFile(TOOLS_FILE::tools_csv, e2db::FCONVS::convert_userbouquets); });
		menuAction(tmexportcsv, tr("Export Tuner settings", "menu"), [=]() { this->toolsExportToFile(TOOLS_FILE::tools_csv, e2db::FCONVS::convert_tunersets); });
		menuAction(tmexport, tr("Export to M3U", "menu"), [=]() { this->toolsExportToFile(TOOLS_FILE::tools_m3u, e2db::FCONVS::convert_current); });
		QMenu* tmexporthtml = menuMenu(tmexport, tr("Export to HTML", "menu"));
		menuAction(tmexporthtml, tr("Export current", "menu"), [=]() { this->toolsExportToFile(TOOLS_FILE::tools_html, e2db::FCONVS::convert_current); });
		menuAction(tmexporthtml, tr("Export All", "menu"), [=]() { this->toolsExportToFile(TOOLS_FILE::tools_html, e2db::FCONVS::convert_all); });
		menuAction(tmexporthtml, tr("Export Index", "menu"), [=]() { this->toolsExportToFile(TOOLS_FILE::tools_html, e2db::FCONVS::convert_index); });
		menuAction(tmexporthtml, tr("Export Services", "menu"), [=]() { this->toolsExportToFile(TOOLS_FILE::tools_html, e2db::FCONVS::convert_services); });
		menuAction(tmexporthtml, tr("Export Bouquets", "menu"), [=]() { this->toolsExportToFile(TOOLS_FILE::tools_html, e2db::FCONVS::convert_bouquets); });
		menuAction(tmexporthtml, tr("Export Userbouquets", "menu"), [=]() { this->toolsExportToFile(TOOLS_FILE::tools_html, e2db::FCONVS::convert_userbouquets); });
		menuAction(tmexporthtml, tr("Export Tuner settings", "menu"), [=]() { this->toolsExportToFile(TOOLS_FILE::tools_html, e2db::FCONVS::convert_tunersets); });
		menuSeparator(menu);
		menuAction(menu, tr("Log Inspector", "menu"), [=]() { this->toolsLogInspector(); }, Qt::CTRL | Qt::ALT | Qt::Key_J);
	}

	QMenu* menu = this->tools_menu;

	return menu;
}

void tab::toolsLogInspector()
{
	for (auto & q : QGuiApplication::allWindows())
	{
		if (q->isWindowType() && q->objectName() == "logInspectorWindow")
		{
			debug("toolsLogInspector", "hit", 1);

			q->requestActivate();
			return q->raise();
		}
	}

	debug("toolsLogInspector");

	tools->logInspector();
}

void tab::toolsErrorChecker()
{
	debug("toolsErrorChecker");

	tools->errorChecker();
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
		case TOOLS_FILE::tools_m3u:
			this->tools->importFileM3U(fci, opts);
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

	if (ftype == TOOLS_FILE::tools_m3u)
	{
		filename = "untitled";

		opts.fc = e2db::FCONVS::convert_all;

		// main view
		if (current == gui::TAB_VIEW::main)
		{
			mainView* view = reinterpret_cast<mainView*>(this->view);
			auto state = view->currentState();

			// userbouquets
			if (state.ti == -1)
			{
				string bname = state.curr;
				opts.bname = bname;
				opts.fc = e2db::FCONVS::convert_current;

				filename = bname;
				std::transform(filename.begin(), filename.end(), filename.begin(), [](unsigned char c) { return c == '.' ? '-' : c; });
			}
		}

		opts.filename = filename;
	}
	else if (fco == e2db::FCONVS::convert_current)
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

	if (this->data->hasChanged())
		updateIndex();

	switch (ftype)
	{
		case TOOLS_FILE::tools_csv:
			opts.filename += ".csv";
			this->tools->exportFileCSV(fco, opts);
		break;
		case TOOLS_FILE::tools_m3u:
			opts.filename += ".m3u8";
			this->tools->exportFileM3U(fco, opts);
		break;
		case TOOLS_FILE::tools_html:
			opts.filename += ".html";
			this->tools->exportFileHTML(fco, opts);
		break;
	}
}

void tab::toolsAutofixMacro()
{
	// debug("toolsAutofixMacro");

	if (this->data->hasChanged())
		updateIndex();

	this->tools->macroAutofix();
}

void tab::toolsUtils(int bit, bool selecting, bool contextual)
{
	// debug("toolsUtils", "bit", bit);

	gui::TAB_VIEW current = getTabView();
	e2db::uoopts opts;
	bool tree_switch = false;

	// main view
	if (current == gui::TAB_VIEW::main)
	{
		mainView* view = reinterpret_cast<mainView*>(this->view);
		auto state = view->currentState();

		// services
		if (state.tc == 0)
		{
			if (bit == gui::TAB_ATS::UtilsSort_references)
				bit = gui::TAB_ATS::UtilsSort_services;
		}
		// bouquets
		else if (state.tc == 1)
		{
			if (bit == gui::TAB_ATS::UtilsSort_references)
			{
				int ti = -1;
				QList<QTreeWidgetItem*> selected = view->tree->selectedItems();

				for (auto & item : selected)
				{
					ti = view->tree->indexOfTopLevelItem(item);
					string bname = item->data(0, Qt::UserRole).toString().toStdString();

					// userbouquet
					if (ti == -1)
					{
						opts.iname = bname;
					}
				}
			}
			else if (bit == gui::TAB_ATS::UtilsSort_services)
			{
				tree_switch = true;
			}
		}

		if (bit != gui::TAB_ATS::UtilsSort_userbouquets)
		{
			QList<QTreeWidgetItem*> selected = view->list->selectedItems();

			for (auto & x : selected)
			{
				int i = view->list->indexOfTopLevelItem(x);
				opts.selection.emplace_back(i);
			}
			opts.selecting = (selecting || ! contextual) && opts.selection.size() > 1;
		}
	}
	// transponders view
	else if (current == gui::TAB_VIEW::transponders)
	{
		transpondersView* view = reinterpret_cast<transpondersView*>(this->view);

		if (bit == gui::TAB_ATS::UtilsSort_references)
			bit = gui::TAB_ATS::UtilsSort_transponders;

		if (bit != gui::TAB_ATS::UtilsSort_userbouquets)
		{
			QList<QTreeWidgetItem*> selected = view->list->selectedItems();

			for (auto & x : selected)
			{
				int i = view->list->indexOfTopLevelItem(x);
				opts.selection.emplace_back(i);
			}
			opts.selecting = (selecting || ! contextual) && opts.selection.size() > 1;
		}
	}
	else
	{
		return;
	}

	if (this->data->hasChanged())
		updateIndex();

	this->tools->applyUtils(bit, opts, contextual);

	if (bit == gui::TAB_ATS::UtilsSort_services && opts.exec && tree_switch)
	{
		mainView* view = reinterpret_cast<mainView*>(this->view);

		view->side->setFocus();
		view->side->setCurrentItem(nullptr); // should reset QTreeView::currentIndex
		view->side->setCurrentItem(view->side->topLevelItem(0));
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

		case gui::TAB_ATS::TreeEditBouquet:
		case gui::TAB_ATS::TreeEditPosition:
		case gui::TAB_ATS::DialEditSettings:
		case gui::TAB_ATS::ListEditTransponder:
		case gui::TAB_ATS::ListEditService:
		case gui::TAB_ATS::ListEditFavourite:
		case gui::TAB_ATS::ListEditMarker:
		case gui::TAB_ATS::ListEditPicon:
			view->actionCall(bit);
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

		case gui::TAB_ATS::ConvertLamedb24:
		case gui::TAB_ATS::ConvertLamedb25:
		case gui::TAB_ATS::ConvertLamedb23:
		case gui::TAB_ATS::ConvertLamedb22:
		case gui::TAB_ATS::ConvertZapit4:
		case gui::TAB_ATS::ConvertZapit3:
		case gui::TAB_ATS::ConvertZapit2:
		case gui::TAB_ATS::ConvertZapit1:
			convertFormat(gui::GUI_CXE (bit));
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
		case gui::TAB_ATS::ImportM3U:
			toolsImportFromFile(TOOLS_FILE::tools_m3u, e2db::FCONVS::convert_all);
		break;
		case gui::TAB_ATS::ExportM3U:
			toolsExportToFile(TOOLS_FILE::tools_m3u, e2db::FCONVS::convert_current);
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

		case gui::TAB_ATS::ErrorChecker:
			toolsErrorChecker();
		break; 
		case gui::TAB_ATS::AutofixMacro:
			toolsAutofixMacro();
		break;

		case gui::TAB_ATS::UtilsOrphaned_services:
		case gui::TAB_ATS::UtilsOrphaned_references:
		case gui::TAB_ATS::UtilsFixRemove:
		case gui::TAB_ATS::UtilsClearServicesCached:
		case gui::TAB_ATS::UtilsClearServicesCAID:
		case gui::TAB_ATS::UtilsClearServicesFlags:
		case gui::TAB_ATS::UtilsClearServicesData:
		case gui::TAB_ATS::UtilsFixDVBNS:
		case gui::TAB_ATS::UtilsClearFavourites:
		case gui::TAB_ATS::UtilsRemove_parentallock:
		case gui::TAB_ATS::UtilsClearBouquetsUnused:
		case gui::TAB_ATS::UtilsRemove_bouquets:
		case gui::TAB_ATS::UtilsRemove_userbouquets:
		case gui::TAB_ATS::UtilsDuplicates_markers:
		case gui::TAB_ATS::UtilsDuplicates_references:
		case gui::TAB_ATS::UtilsDuplicates_services:
		case gui::TAB_ATS::UtilsDuplicates_transponders:
		case gui::TAB_ATS::UtilsDuplicates_all:
		case gui::TAB_ATS::UtilsTransform_tunersets:
		case gui::TAB_ATS::UtilsTransform_transponders:
		case gui::TAB_ATS::UtilsSort_transponders:
		case gui::TAB_ATS::UtilsSort_services:
		case gui::TAB_ATS::UtilsSort_userbouquets:
		case gui::TAB_ATS::UtilsSort_references:
			toolsUtils(bit);
		break;

		case gui::TAB_ATS::Inspect:
			toolsLogInspector();
		break;
	}
}

void tab::ftpComboItems()
{
	// debug("ftpComboItems");

	if  (ftp_combo->count())
	{
		ftp_combo->clear();

		for (int i = 0; i < ftp_combo->count(); i++)
		{
			ftp_combo->removeItem(i);
		}
	}

	QSettings settings;

	int idx = 0;
	int size = settings.beginReadArray("profile");
	for (int i = 0; i < size; i++)
	{
		settings.setArrayIndex(i);

		if (settings.contains("profileName"))
		{
			idx = settings.group().section('/', 1).toInt();
			ftp_combo->addItem(settings.value("profileName").toString(), idx);
		}
	}
	settings.endArray();

	int profile_sel = settings.value("profile/selected", -1).toInt();
	int index = ftp_combo->findData(profile_sel, Qt::UserRole);

	ftp_combo->setCurrentIndex(index);
}

void tab::ftpComboChanged(int index)
{
	debug("profileComboChanged", "index", index);

	this->ftph->closeConnection();

	ftp_combo->setCurrentIndex(index);

	int profile_sel = ftp_combo->itemData(index, Qt::UserRole).toInt();
	QSettings().setValue("profile/selected", profile_sel);
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
	int profile_sel = settings.value("profile/selected", -1).toInt();
	int profile_i = -1;

	int idx = 0;
	int size = settings.beginReadArray("profile");
	for (int i = 0; i < size; i++)
	{
		settings.setArrayIndex(i);
		idx = settings.group().section('/', 1).toInt();
		if (profile_sel == idx)
		{
			profile_i = i;
			break;
		}
	}
	settings.endArray();

	if (profile_i == -1)
		return;

	settings.beginReadArray("profile");
	settings.setArrayIndex(profile_i);
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
		else if (filename == "bouquets.xml" || filename == "ubouquets.xml")
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
					this->ftp_errors.emplace_back(pair (fname, err.what()));

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
			QMetaObject::invokeMethod(this->cwid, [=]() { this->resetStatusBar(true); }, Qt::QueuedConnection);

			if (! this->ftp_errors.empty())
			{
				QMetaObject::invokeMethod(this->cwid, [=]() { this->ftpcomError(this->ftp_errors, MSG_CODE::ftpNotice); }, Qt::QueuedConnection);
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

	this->data->clearErrors();

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
					this->ftp_errors.emplace_back(pair (fname, err.what()));
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
			QMetaObject::invokeMethod(this->cwid, [=]() { this->resetStatusBar(true); }, Qt::QueuedConnection);

			if (! this->ftp_errors.empty())
			{
				QMetaObject::invokeMethod(this->cwid, [=]() { this->ftpcomError(this->ftp_errors, MSG_CODE::ftpNotice); }, Qt::QueuedConnection);
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
				dbih->importBlob(this->files);
			}
			catch (...)
			{
				this->files.clear();

				//TODO TEST potential SEGFAULT
				QMetaObject::invokeMethod(this->cwid, [=]() { this->e2dbError(); }, Qt::QueuedConnection);

				return;
			}

			if (this->data->haveErrors())
				QMetaObject::invokeMethod(this->cwid, [=]() { this->e2dbError(this->data->getErrors(), MSG_CODE::parseNotice); }, Qt::QueuedConnection);

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
			this->ftp_errors.emplace_back(pair ("Webif", err.what()));
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
			this->ftp_errors.emplace_back(pair ("Telnet", err.what()));
		}
		catch (...)
		{
		}
	});
	thread->connect(thread, &QThread::started, [=]() {
		QMetaObject::invokeMethod(this->cwid, [=]() { this->ftpStbReloadingNotify(); }, Qt::QueuedConnection);
	});
	thread->connect(thread, &QThread::finished, [=]() {
		//TODO FIX status message not readable glitch [Linux]
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
		statusBarQuickMessage(tr("Uploading file: %1", "message").arg(filename.data()));
}

void tab::ftpStbDownloadNotify(int files_count)
{
	if (statusBarIsVisible())
		statusBarMessage(tr("Downloaded %n files", "message", files_count));
}

void tab::ftpStbDownloadNotify(string filename)
{
	if (statusBarIsVisible())
		statusBarQuickMessage(tr("Downloading file: %1", "message").arg(filename.data()));
}

void tab::ftpStbReloadSuccessNotify()
{
	if (statusBarIsVisible())
		statusBarMessage(tr("STB reload done.", "message"));
	else
		infoMessage(tr("STB reload done.", "message"));
}

void tab::ftpStbReloadErrorNotify(vector<pair<string, string>> errors)
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
		ftpcomError(errors, MSG_CODE::stbReloadNotice);
}

void tab::ftpcomError()
{
	errorMessage(tr("FTP Error", "error"), tr("Errors occurred during FTP operations.", "error"));
}

void tab::ftpcomError(string error)
{
	errorMessage(error);
}

void tab::ftpcomError(vector<pair<string, string>> errors, MSG_CODE code)
{
	noticeMessage(errors, code);
}

void tab::e2dbError()
{
	errorMessage(tr("File Error", "error"), tr("Errors occurred during parsing operations.", "error"));
}

void tab::e2dbError(string error)
{
	errorMessage(error);
}

void tab::e2dbError(QString title, QString message)
{
	errorMessage(title, message);
}

void tab::e2dbError(vector<string> errors, MSG_CODE code)
{
	vector<pair<string, string>> _errors;
	int errcount = 0;

	for (auto & error : errors)
	{
		string errnum = "i=" + to_string(errcount);
		_errors.emplace_back(pair (errnum, error));
		errcount++;
	}

	noticeMessage(_errors, code);
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

// note: tab::statusBarMessage this (gui*) is 0x0 with deleted tab* QTimer
QTimer* tab::statusBarMessage(QString message)
{
	gui::status msg;
	msg.info = true;
	msg.message = message.toStdString();
	setStatusBar(msg);

	// note: rand SEGFAULT with widget in thread
	QTimer* timer = new QTimer(this->widget);
	timer->setSingleShot(true);
	timer->setInterval(STATUSBAR_MESSAGE_TIMEOUT);
	timer->callOnTimeout([=]() { this->resetStatusBar(true); timer->stop(); });
	timer->start();

	return timer;
}

void tab::statusBarMessage(QTimer* timer)
{
	if (timer != nullptr)
	{
		timer->stop();
		timer->setInterval(STATUSBAR_MESSAGE_DELAY);
		timer->start();
	}
}

void tab::statusBarQuickMessage(QString message)
{
	gui::status msg;
	msg.info = true;
	msg.message = message.toStdString();
	setStatusBar(msg);
}

int tab::saveQuestion()
{
	return saveQuestion(tr("The file has been modified", "message"), tr("Do you want to save your changes?", "message"));
}

int tab::saveQuestion(QString title, QString message)
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
	msg.setStandardButtons(QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
	msg.setDefaultButton(QMessageBox::Save);

	return msg.exec();
}

bool tab::overwriteQuestion()
{
	return overwriteQuestion(tr("The destination contains files that will be overwritten.", "message"), tr("Do you want to overwrite them?", "message"));
}

bool tab::overwriteQuestion(QString title, QString message)
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

bool tab::removeQuestion()
{
	return removeQuestion(tr("Confirm deletetion", "message"), tr("Do you want to delete items?", "message"));
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

	// note: rand SEGFAULT with cwid in thread
	QMessageBox msg = QMessageBox(nullptr);

	msg.setTextFormat(Qt::PlainText);
	msg.setText(title);

	msg.exec();
}

void tab::infoMessage(QString title, QString message)
{
	title = title.toHtmlEscaped();
	message = message.replace("<", "&lt;").replace(">", "&gt;");

	// note: rand SEGFAULT with cwid in thread
	QMessageBox msg = QMessageBox(nullptr);

	msg.setTextFormat(Qt::PlainText);
	msg.setText(title);
	msg.setInformativeText(message);

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

	// note: rand SEGFAULT with cwid in thread
	QMessageBox::critical(nullptr, title, text);
}

void tab::errorMessage(string error)
{
	stringstream ss (error);

	string optk, optv, fn;
	std::getline(ss, optk, '\t');
	std::getline(ss, optv, '\t');
	std::getline(ss, fn, '\t');

	QString title = QString(optk.data());
	QString message = QString(optv.data());

	title = title.toHtmlEscaped();
	message = message.replace("<", "&lt;").replace(">", "&gt;");

#ifndef Q_OS_MAC
	QString text = message;
#else
	QString text = QString("%1\n\n%2").arg(title).arg(message);
#endif

	// note: rand SEGFAULT with cwid in thread
	QMessageBox::critical(nullptr, title, text);
}

void tab::noticeMessage(vector<pair<string, string>> errors, MSG_CODE code)
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

		QString error = QString(QApplication::layoutDirection() == Qt::RightToLeft ? "(%4) [%3] %2 :%1" : "%1: %2 [%3] (%4)").arg(optk.data()).arg(optv.data()).arg(fn.data()).arg(filename.data());

		errlist.append(error);
	}

	QString title, message;
	QString error_detailed = errlist.join("\n");

	if (code == MSG_CODE::ftpNotice)
	{
		title = tr("FTP Notice", "error");
		message = QString("%1\n\n%2")
			.arg(tr("Successfull transfer.", "message"))
			.arg(tr("Errors occurred during FTP operations.", "error"));
	}
	else if (code == MSG_CODE::stbReloadNotice)
	{
		title = tr("STB Reload Notice", "error");
		message = QString("%1\n\n%2")
			.arg(tr("STB reload done.", "message"))
			.arg(tr("Errors occurred during STB reload operations.", "error"));
	}
	else if (code == MSG_CODE::parseNotice || code == MSG_CODE::readNotice || code == MSG_CODE::importNotice)
	{
		title = e2db::tr("Parser Error", "error");
		message = tr("Errors occurred during parsing operations.", "error");
	}
	else if (code == MSG_CODE::writeNotice || code == MSG_CODE::exportNotice)
	{
		title = e2db::tr("Maker Error", "error");
		message = tr("Error writing files.", "error");
	}
	else if (code == MSG_CODE::utilsNotice)
	{
		title = tr("Utils Error", "error");
		message = tr("Error executing utils.", "error");
	}

	title = title.toHtmlEscaped();
	message = message.replace("<", "&lt;").replace(">", "&gt;");
	error_detailed = error_detailed.replace("<", "&lt;").replace(">", "&gt;");

	// note: rand SEGFAULT with cwid in thread
	QMessageBox msg = QMessageBox(nullptr);

	msg.setIcon(QMessageBox::Warning);
	msg.setTextFormat(Qt::PlainText);
	msg.setText(title);
	msg.setInformativeText(message);
	msg.setDetailedText(error_detailed);

	msg.exec();
}

void tab::demoMessage()
{
	gid->demoMessage();
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
		toolbar->setStyleSheet("QToolBar { padding: 8px 12px } QToolButton { font-size: 16px; font-weight: bold }");
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

QWidget* tab::toolBarEnding(QToolBar* toolbar)
{
	QWidget* spacer = new QWidget;
	spacer->setFixedWidth(6);
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

QMenu* tab::menuMenu(QMenu* menu, QString title)
{
	QMenu* submenu = new QMenu(menu);
	submenu->setTitle(title);
	menu->addMenu(submenu);
	return submenu;
}

QAction* tab::menuAction(QMenu* menu, QString text, std::function<void()> trigger)
{
	QAction* action = new QAction(menu);
	action->setText(text);
	action->connect(action, &QAction::triggered, trigger);
	menu->addAction(action);
	return action;
}

QAction* tab::menuAction(QMenu* menu, QString text, std::function<void()> trigger, QKeySequence shortcut)
{
	QAction* action = new QAction(menu);
	action->setText(text);
	action->setShortcut(shortcut);
	action->connect(action, &QAction::triggered, trigger);
	menu->addAction(action);
	return action;
}

QAction* tab::menuTitle(QMenu* menu, QString title)
{
	QWidgetAction* action = new QWidgetAction(nullptr);
	QLabel* label = new QLabel(title);
#ifndef Q_OS_MAC
	label->setStyleSheet("QLabel { margin: 5px 10px }");
#else
	label->setStyleSheet("QLabel { margin: 5px 10px; font-weight: bold }");
#endif
	action->setDefaultWidget(label);
	menu->addAction(action);
	return action;
}

QAction* tab::menuSeparator(QMenu* menu)
{
	QAction* action = new QAction(menu);
	action->setSeparator(true);
	menu->addAction(action);
	return action;
}

}
