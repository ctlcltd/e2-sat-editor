/*!
 * e2-sat-editor/src/gui/gui.cpp
 *
 * @link https://github.com/ctlcltd/e2-sat-editor
 * @copyright e2 SAT Editor Team
 * @author Leonardo Laureti
 * @version 0.1
 * @license MIT License
 * @license GNU GPLv3 License
 */

#include <clocale>
#include <filesystem>

#include <QScreen>
#include <QSplitter>
#include <QGroupBox>
#include <QTabWidget>
#include <QPushButton>
#include <QFileDialog>

#include "toolkit/TabBarProxyStyle.h"
#include "gui.h"
#include "theme.h"
#include "tab.h"
#include "settings.h"
#include "about.h"
#include "todo.h"

using std::to_string;
using namespace e2se;

namespace e2se_gui
{

gui::gui(int argc, char* argv[])
{
	std::setlocale(LC_NUMERIC, "C");

	this->log = new logger("gui");
	debug("gui()");

	this->mroot = new QApplication(argc, argv);
	mroot->setOrganizationName("e2 SAT Editor Team");
	mroot->setOrganizationDomain("e2se.org");
	mroot->setApplicationName("e2-sat-editor");
	mroot->setApplicationVersion("0.1");
	mroot->setStyle(new TabBarProxyStyle);
	mroot->connect(mroot, &QApplication::focusChanged, [=]() { this->windowFocusChanged(); });

	this->sets = new QSettings;
	if (! sets->contains("application/version"))
		setDefaultSets();

	QScreen* screen = mroot->primaryScreen();
	QSize wsize = screen->availableSize();

	this->mwid = new QWidget;
	mwid->setWindowTitle("e2 SAT Editor");
	mwid->setMinimumSize(760, 550);
	mwid->resize(wsize);

	theme();

	// mroot->setLayoutDirection(Qt::RightToLeft);
	if (! theme::isDefault() || ! QSysInfo::productType().contains(QRegularExpression("macos|osx")))
	{
		mwid->setStyleSheet("QToolBar { background: palette(mid) }");
	}

	root();

	mwid->show();

	mroot->exec();
}

void gui::root()
{
	debug("root()");

	this->mfrm = new QGridLayout(mwid);

	this->mcnt = new QHBoxLayout;
	this->mstatusb = new QHBoxLayout;

	menuCtl();
	statusCtl();
	tabCtl();

	mfrm->setContentsMargins(0, 0, 0, 0);
	mfrm->setSpacing(0);

	mfrm->addLayout(mcnt, 0, 0);
	mfrm->addLayout(mstatusb, 1, 0);
}

void gui::menuCtl()
{
	debug("menuCtl()");

	QMenuBar* menu = new QMenuBar;
	menu->setNativeMenuBar(true);
	mfrm->setMenuBar(menu);

	QMenu* mfile = menu->addMenu(tr("&File"));
	gmenu[GUI_CXE::FileNew] = mfile->addAction(tr("&New"), [=]() { this->newTab(""); }, QKeySequence::New);
	gmenu[GUI_CXE::FileOpen] = mfile->addAction(tr("&Open"), [=]() { this->open(); }, QKeySequence::Open);
	gmenu[GUI_CXE::FileSave] = mfile->addAction(tr("&Save"), [=]() { this->save(); }, QKeySequence::Save);
	mfile->addSeparator();
	gmenu[GUI_CXE::FileImport] = mfile->addAction("Import", todo);
	gmenu[GUI_CXE::FileExport] = mfile->addAction("Export", todo);
	mfile->addSeparator();
	gmenu[GUI_CXE::CloseTab] = mfile->addAction("Close Tab", [=]() { this->closeTab(-1); });
	gmenu[GUI_CXE::CloseAllTabs] = mfile->addAction("Close All Tabs", [=]() { this->closeAllTabs(); });
	mfile->addSeparator();
	mfile->addAction("Settings", [=]() { this->settings(); }, QKeySequence::Preferences);
#ifdef Q_OS_MAC
		mfile->addAction(tr("&About"), [=]() { this->about(); });
#endif
	mfile->addSeparator();
	QAction* mfile_exit = mfile->addAction(tr("&Exit"), [=]() { this->mroot->quit(); });
#ifndef Q_OS_WIN
	mfile_exit->setShortcut(QKeySequence::Quit);
#else
	mfile_exit->setShortcut(QKeySequence::Close);
#endif
	
	QMenu* medit = menu->addMenu(tr("&Edit"));
	gmenu[GUI_CXE::TabListCut] = medit->addAction(tr("Cu&t"), [=]() { this->tabAction(TAB_ATS::ListCut); }, QKeySequence::Cut);
	gmenu[GUI_CXE::TabListCopy] = medit->addAction(tr("&Copy"), [=]() { this->tabAction(TAB_ATS::ListCopy); },  QKeySequence::Copy);
	gmenu[GUI_CXE::TabListPaste] = medit->addAction(tr("&Paste"), [=]() { this->tabAction(TAB_ATS::ListPaste); }, QKeySequence::Paste);
	medit->addSeparator();
	gmenu[GUI_CXE::TabListSelectAll] = medit->addAction(tr("Select &All"), [=]() { this->tabAction(TAB_ATS::ListSelectAll); }, QKeySequence::SelectAll);

	QMenu* mfind = menu->addMenu(tr("&Find"));
	gmenu[GUI_CXE::TabListFind] = mfind->addAction(tr("Find Channel…"), [=]() { this->tabAction(TAB_ATS::ListFind); }, QKeySequence::Find);
	gmenu[GUI_CXE::TabListFindNext] = mfind->addAction(tr("Find Next"), [=]() { this->tabAction(TAB_ATS::ListFindNext); }, QKeySequence::FindNext);
	gmenu[GUI_CXE::TabListFindPrev] = mfind->addAction(tr("Find Previous"), [=]() { this->tabAction(TAB_ATS::ListFindPrev); }, QKeySequence::FindPrevious);
	gmenu[GUI_CXE::TabListFindAll] = mfind->addAction(tr("Find All"), [=]() { this->tabAction(TAB_ATS::ListFindAll); });
	mfind->addSeparator();
	gmenu[GUI_CXE::TabBouquetsFind] = mfind->addAction(tr("Find Bouquet…"), [=]() { this->tabAction(TAB_ATS::BouquetsFind); }, Qt::CTRL | Qt::ALT | Qt::Key_F);
	gmenu[GUI_CXE::TabBouquetsFindNext] = mfind->addAction(tr("Find Next"), [=]() { this->tabAction(TAB_ATS::BouquetsFindNext); }, Qt::CTRL | Qt::ALT | Qt::Key_G);

	QMenu* mtool = menu->addMenu(tr("Tools"));
	gmenu[GUI_CXE::ToolsTunersetsSat] = mtool->addAction("Edit satellites.xml", [=]() { this->tabAction(TAB_ATS::EditTunerSat); });
	gmenu[GUI_CXE::ToolsTunersetsTerrestrial] = mtool->addAction("Edit terrestrial.xml", [=]() { this->tabAction(TAB_ATS::EditTunerTerrestrial); });
	gmenu[GUI_CXE::ToolsTunersetsCable] = mtool->addAction("Edit cables.xml", [=]() { this->tabAction(TAB_ATS::EditTunerCable); });
	gmenu[GUI_CXE::ToolsTunersetsAtsc] = mtool->addAction("Edit atsc.xml", [=]() { this->tabAction(TAB_ATS::EditTunerAtsc); });
	mtool->addSeparator();
	mtool->addAction("Order services A-Z", todo);
	mtool->addAction("Order userbouquets A-Z", todo);
	mtool->addAction("Remove cached data from services", todo);
	mtool->addAction("Delete all bouquets", todo);

	QMenu* mwind = menu->addMenu(tr("&Window"));
	gmenu[GUI_CXE::WindowMinimize] = mwind->addAction("&Minimize", [=]() { this->windowMinimize(); }, Qt::CTRL | Qt::Key_M);
	mwind->addSeparator();
	gmenu[GUI_CXE::NewTab] = mwind->addAction("&New Tab", [=]() { this->newTab(); }, Qt::CTRL | Qt::Key_T);
	mwind->addSeparator();
	QActionGroup* mwtabs = new QActionGroup(mwind);
	mwtabs->setExclusive(true);

	QMenu* mhelp = menu->addMenu(tr("&Help"));
	//TODO FIX macos QAction::NoRole ignored
	mhelp->addAction("TODO", todo);
	mhelp->addAction(tr("&About"), [=]() { this->about(); })->setMenuRole(QAction::NoRole);

	this->menu = menu;
	this->mwind = mwind;
	this->mwtabs = mwtabs;
}

void gui::tabCtl()
{
	debug("tabCtl()");

	this->twid = new QTabWidget(mwid);
	twid->setTabsClosable(true);
	twid->setMovable(true);
//	twid->setDocumentMode(true);
//	twid->setUsesScrollButtons(true);
//	twid->tabBar()->setDrawBase(false);
//	twid->tabBar()->setAutoFillBackground(true);
	twid->tabBar()->setChangeCurrentOnDrag(false);

	QString ttclose_icon = ":/icons/" + QString (theme::absLuma() ? "dark" : "light") + "/close.png";
	QString ttclose_icon__selected = ":/icons/" + QString (theme::absLuma() ? "dark" : "dark") + "/close.png";
#ifdef Q_OS_MAC
	if (theme::isDefault())
		ttclose_icon__selected = ":/icons/" + QString (theme::absLuma() ? "dark" : "light") + "/close.png";
#endif

	twid->setStyleSheet("QTabWidget::tab-bar { left: 0px } QTabWidget::pane { border: 0; border-radius: 0 } QTabBar::tab { height: 32px; padding: 5px; background: palette(mid); border: 1px solid transparent; border-radius: 0 } QTabBar::tab:selected { background: palette(highlight) } QTabBar::tab QLabel { margin-left: 5px } QTabBar::close-button { margin: 0.4ex; image: url(" + ttclose_icon + ") } QTabBar::close-button:selected { image: url(" + ttclose_icon__selected + ") }");
	twid->connect(twid, &QTabWidget::currentChanged, [=](int index) { this->tabChanged(index); });
	twid->connect(twid, &QTabWidget::tabCloseRequested, [=](int index) { this->closeTab(index); });
	twid->tabBar()->connect(twid->tabBar(), &QTabBar::tabMoved, [=](int from, int to) { this->tabMoved(from, to); });

	QPushButton* ttbnew = new QPushButton(theme::icon("add"), tr("New &Tab"));
	ttbnew->setMinimumHeight(32);
	ttbnew->setIconSize(QSize(12, 12));
	ttbnew->setShortcut(QKeySequence::AddTab);
	//TODO FIX height & ::left-corner padding
	ttbnew->setStyleSheet("width: 8ex; height: 32px; font: bold 12px");
	ttbnew->connect(ttbnew, &QPushButton::pressed, [=]() { this->newTab(); });
	twid->setCornerWidget(ttbnew, Qt::TopLeftCorner);

	initialize();

	mcnt->addWidget(twid);
}

//TODO FIX dir:rtl
void gui::statusCtl()
{
	debug("statusCtl()");

	this->sbwid = new QStatusBar(mwid);
	this->sbwidl = new QLabel;
	this->sbwidr = new QLabel;

	sbwid->setStyleSheet("QStatusBar QLabel { padding: 0 2ex }");
	sbwidl->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
	sbwidr->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

	sbwid->addWidget(sbwidl, 1);
	sbwid->addWidget(sbwidr, 1);

	mstatusb->addWidget(sbwid);
}

void gui::windowFocusChanged()
{
	// debug("windowFocusChanged()");

	// main window focus in
	if (mroot->activeWindow())
	{
		debug("windowFocusChanged()", "mwind", "focus in");
		this->state.xe = this->state.ex;
	}
	// main window focus out
	else
	{
		debug("windowFocusChanged()", "mwind", "focus out");
		this->state.ex = this->state.xe;
		this->state.xe = GUI_CXE::deactivated;
	}
	update();
}

int gui::newTab(string filename)
{
	tab* ttab = new tab(this, mwid);
	int ttid = this->state.tt++;

	if (! filename.empty() && ! ttab->readFile(filename))
		return -1;

	bool read = ! filename.empty();
	int ttcount = twid->count();
	QString ttname = QString::fromStdString("Untitled" + (ttcount ? " " + to_string(ttcount) : ""));

	ttab->setTabId(ttid);
	int index = twid->addTab(ttab->widget, ttname);
	twid->tabBar()->setTabData(index, ttid);

	QTabBar* ttabbar = twid->tabBar();
	QLabel* ttlabel = new QLabel;
	ttlabel->setForegroundRole(QPalette::HighlightedText);
	ttlabel->setText(ttname);
	ttabbar->setTabButton(index, QTabBar::LeftSide, ttlabel);
	ttabbar->setTabText(index, "");

	QAction* action = new QAction(ttname);
	action->connect(action, &QAction::triggered, [=]() { this->twid->setCurrentWidget(ttab->widget); });
	action->setCheckable(true);
	action->setActionGroup(mwtabs);
	mwind->addAction(action);
	ttmenu[ttid] = action;
	ttabs[ttid] = ttab;

	if (read)
		tabChangeName(ttid, filename);
	twid->setCurrentIndex(index);

	debug("newTab()", "ttid", to_string(ttid));

	return index;
}

void gui::closeTab(int index)
{
	debug("closeTab()", "index", to_string(index));

	int ttid = getCurrentTabID(index);

	mwind->removeAction(ttmenu[ttid]);
	mwtabs->removeAction(ttmenu[ttid]);
	twid->removeTab(index);
	ttmenu.erase(ttid);

	delete ttabs[ttid];
	ttabs.erase(ttid);

	if (twid->count() == 0)
		initialize();
}

void gui::closeAllTabs()
{
	debug("closeAllTabs()");

	for (auto & action : mwtabs->actions())
	{
		mwind->removeAction(action);
		mwtabs->removeAction(action);
	}
	twid->clear();

	for (unsigned int i = 0; i < ttabs.size(); i++)
	{
		debug("closeAllTabs()", "destroy", to_string(i));

		delete ttabs[i];
		ttabs.erase(i);
		ttmenu.erase(i);
	}
	ttabs.clear();
	ttmenu.clear();

	initialize();
}

void gui::tabChanged(int index)
{
	debug("tabChanged()", "index", to_string(index));

	int ttid = getCurrentTabID(index);
	if (ttid != -1)
	{
		ttabs[ttid]->tabSwitched();
		ttmenu[ttid]->setChecked(true);

		QTabBar* ttabbar = twid->tabBar();
		for (unsigned int i = 0; i < ttabs.size(); i++)
		{
			tab* ttab = ttabs[i];
			if (ttab != nullptr)
			{
				int index = twid->indexOf(ttab->widget);
				QWidget* ttabbls = ttabbar->tabButton(index, QTabBar::LeftSide);
				if (QLabel* ttlabel = qobject_cast<QLabel*>(ttabbls))
					ttlabel->setForegroundRole(QPalette::ButtonText);
			}
		}
		tab* ttab = ttabs[ttid];
		if (ttab != nullptr)
		{
			int index = twid->indexOf(ttab->widget);
			QWidget* ttabbls = ttabbar->tabButton(index, QTabBar::LeftSide);
			if (QLabel* ttlabel = qobject_cast<QLabel*>(ttabbls))
				ttlabel->setForegroundRole(QPalette::HighlightedText);
		}
	}
}

void gui::tabMoved(int from, int to)
{
	debug("tabMoved()", "from|to", (to_string(from) + '|' + to_string(to)));

	auto actions = mwtabs->actions();
	actions.move(from, to);

	for (auto & action : actions)
	{
		mwind->removeAction(action);
		mwtabs->removeAction(action);
		mwind->addAction(action);
		mwtabs->addAction(action);
	}
}

string gui::openFileDialog()
{
	debug("openFileDialog()");
	
	string filename;

	//TODO ~ $HOME
	QString dirname = QFileDialog::getExistingDirectory(nullptr, "Select enigma2 db folder", "~", QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
	filename = dirname.toStdString();

	return filename;
}

//TODO native file dialog button "Open"
string gui::saveFileDialog(string filename)
{
	debug("saveFileDialog()", "filename", filename);

	string dirname;

	QFileDialog fdial = QFileDialog(nullptr, "Select where to save", QString::fromStdString(filename));
	fdial.setFilter(QDir::AllDirs | QDir::NoSymLinks);
	fdial.setAcceptMode(QFileDialog::AcceptSave);
	QString dir = fdial.getExistingDirectory();
	dirname = dir.toStdString();

	return dirname;
}

vector<string> gui::importFileDialog()
{
	debug("importFileDialog()");
	
	vector<string> filenames;

	//TODO ~ $HOME
	QStringList files = QFileDialog::getOpenFileNames(nullptr, "Select one or more files to open", "~", "Enigma2 db folder (*);;Lamedb 2.4 (lamedb);;Lamedb 2.5 (lamedb5);;Bouquet (bouquets.*);;Userbouquet (userbouquet.*);;Tuner settings (*.xml);;All Files (*)");

	return filenames;
}

string gui::exportFileDialog(string filename)
{
	debug("exportFileDialog()", "filename", filename);

	string dirname;
	
	//TODO ~ $HOME
	QFileDialog fdial = QFileDialog(nullptr, "Select where to save", QString::fromStdString(filename));
	fdial.setFilter(QDir::AllDirs | QDir::NoSymLinks);
	fdial.setAcceptMode(QFileDialog::AcceptSave);
	QString dir = fdial.getExistingDirectory();
	dirname = dir.toStdString();

	return dirname;
}

void gui::tabChangeName(int ttid, string filename)
{
	debug("tabChangeName()", "ttid", to_string(ttid));

	tab* ttab = ttabs[ttid];
	int index = twid->indexOf(ttab->widget);
	string tname;

	if (filename.empty())
		tname = "Untitled" + (index ? " " + to_string(index) : "");
	else
		tname = std::filesystem::path(filename).filename().u8string();

	QString ttname = QString::fromStdString(tname);
	QTabBar* ttabbar = twid->tabBar();
	QWidget* ttabbls = ttabbar->tabButton(index, QTabBar::LeftSide);
	if (QLabel* ttlabel = qobject_cast<QLabel*>(ttabbls))
	{
		ttlabel->setText(ttname);
		ttlabel->adjustSize();
	}
	ttabbar->setTabText(index, "");

	ttmenu[ttid]->setText(ttname);
}

void gui::setStatus(int counters[5])
{
	QString qstr;
	if (counters[COUNTER::current] != -1)
	{
		qstr = "Channels: " + QString::fromStdString(to_string(counters[COUNTER::current]));
		sbwidl->setText(qstr);
	}
	else
	{
		qstr.append("TV: " + QString::fromStdString(to_string(counters[COUNTER::tv])) + "   ");
		qstr.append("Radio: " + QString::fromStdString(to_string(counters[COUNTER::radio])) + "   ");
		qstr.append("Data: " + QString::fromStdString(to_string(counters[COUNTER::data])) + "   ");
		qstr.append("   ");
		qstr.append("Total: " + QString::fromStdString(to_string(counters[COUNTER::all])) + "   ");
		sbwidr->setText(qstr);
	}
}

void gui::resetStatus()
{
	debug("resetStatus()");

	sbwidl->setText("");
	sbwidr->setText("");
}

void gui::open()
{
	debug("open()");

	string dirname = openFileDialog();

	if (! dirname.empty())
		newTab(dirname);
}

void gui::save()
{
	debug("save()");

	tab* ttab = getCurrentTabHandler();
	ttab->saveFile(true); //TODO temporarly set to save as
}

//TODO tab actions ctl
void gui::tabAction(TAB_ATS action)
{
	debug("tabAction()", "action", to_string(action));

	tab* ttab = getCurrentTabHandler();
	ttab->actionCall(action);
}

void gui::windowMinimize()
{
	debug("windowMinimize()");

	if (! this->mwid->isMinimized())
		this->mwid->showMinimized();
}

void gui::settings()
{
	new e2se_gui_dialog::settings(mwid);
}

void gui::about()
{
	new e2se_gui_dialog::about(mwid);
}

int gui::getActionFlag(GUI_CXE connector)
{
	return (this->state.xe & connector);
}

void gui::setActionFlag(GUI_CXE connector, bool flag)
{
	update(connector, flag);
}

int gui::getActionFlags()
{
	return this->state.xe;
}

void gui::setActionFlags(int connectors)
{
	update(connectors);
}

void gui::setActionFlags(int connectors, bool flag)
{
	update(connectors, flag);
}

int gui::getCurrentTabID()
{
	int index = twid->tabBar()->currentIndex();
	return getCurrentTabID(index);
}

int gui::getCurrentTabID(int index)
{
	int ttid = twid->tabBar()->tabData(index).toInt();
	if (ttabs[ttid] == nullptr)
		ttid = -1;
	return ttid;
}

tab* gui::getCurrentTabHandler()
{
	int ttid = getCurrentTabID();
	return ttid != -1 ? ttabs[ttid] : nullptr;
}

void gui::initialize()
{
	debug("initialize()");

	this->state.tt = 0;
	this->state.ex = this->state.xe = GUI_CXE::init;
	newTab();
	tabChanged(0);
}

void gui::update()
{
	// debug("update()");

	for (auto & x : gmenu)
	{
		if (this->state.xe & x.first)
			x.second->setEnabled(true);
		else
			x.second->setDisabled(true);
	}
}

void gui::update(GUI_CXE connector, bool flag)
{
	// debug("update()", "connector", to_string(connector));

	QAction* action = gmenu.count(connector) ? gmenu[connector] : nullptr;

	if (flag)
	{
		if (action != nullptr)
			action->setEnabled(true);
		if (! (this->state.xe & connector))
			this->state.xe |= connector;
	}
	else
	{
		if (action != nullptr)
			action->setDisabled(true);
		if (this->state.xe & connector)
			this->state.xe &= ~connector;
	}

	this->state.ex = this->state.xe;
}

void gui::update(int connectors, bool flag)
{
	// debug("update()", "connectors", to_string(connector));

	if (flag)
		this->state.xe |= connectors;
	else
		this->state.xe &= ~connectors;

	this->state.ex = this->state.xe;
	update();
}

void gui::update(int connectors)
{
	// debug("update()", "connectors", to_string(connector));

	this->state.ex = this->state.xe = connectors;
	update();
}

void gui::setDefaultSets()
{
	debug("setDefaultSets()");

	sets->setValue("application/version", mroot->applicationVersion());

	sets->beginGroup("preference");
	sets->setValue("askConfirmation", true);
	sets->setValue("nonDestructiveEdit", true);
#ifndef Q_OS_MAC
	sets->setValue("fixUnicodeChars", false);
#else
	sets->setValue("fixUnicodeChars", true);
#endif
	sets->endGroup();
	
	sets->beginWriteArray("profile");
	sets->setArrayIndex(0);
	sets->setValue("profileName", "Default");
	sets->setValue("ipAddress", "127.0.0.1");
	sets->setValue("ftpPort", 2121);
	sets->setValue("ftpActive", false);
	sets->setValue("httpPort", 80);
	sets->setValue("username", "root");
	sets->setValue("password", "test");
	sets->setValue("pathTransponders", "/enigma_db");
	sets->setValue("pathServices", "/enigma_db");
	sets->setValue("pathBouquets", "/enigma_db");
	sets->setValue("customWebifReloadUrl", "");
	sets->setValue("customTelnetReloadCmd", "");
	sets->endArray();
	sets->setValue("profile/selected", 1);
}

}
