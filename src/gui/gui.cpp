/*!
 * e2-sat-editor/src/gui/gui.cpp
 *
 * @link https://github.com/ctlcltd/e2-sat-editor
 * @copyright e2 SAT Editor Team
 * @author Leonardo Laureti
 * @version 0.3
 * @license MIT License
 * @license GNU GPLv3 License
 */

#include <clocale>
#include <functional>
#include <filesystem>

#include <QtGlobal>
#include <QSettings>
#include <QScreen>
#include <QSplitter>
#include <QGroupBox>
#include <QTabWidget>
#include <QPushButton>
#include <QFileDialog>

#include "platforms/platform.h"

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

	this->log = new logger("gui", "gui");

	this->mroot = new QApplication(argc, argv);
	mroot->setOrganizationName("e2 SAT Editor Team");
	mroot->setOrganizationDomain("e2se.org");
	mroot->setApplicationName("e2-sat-editor");
	mroot->setApplicationVersion("0.3");
	mroot->connect(mroot, &QApplication::focusChanged, [=]() { this->windowChanged(); });

	if (QSettings().value("settings/reset", false).toBool())
		resetSettings();
	else if (QSettings().value("settings/version").isValid())
		updateSettings();
	else
		initSettings();

	this->mwid = new QWidget;
	mwid->setWindowTitle("e2 SAT Editor");

	//TODO intl. rtl
	// mroot->setLayoutDirection(Qt::RightToLeft);

	this->theme = new e2se_gui::theme;
	theme->initStyle();

	themeChangedEventFilter* gce = new themeChangedEventFilter;
	gce->setEventCallback([=]() { this->themeChanged(); });
	mwid->installEventFilter(gce);

	platform::osWindowBlend(mwid);

	layout();

	if (QSettings().value("geometry").isNull())
	{
		mwid->resize(960, 670);
		mwid->showMaximized();
	}
	else
	{
		mwid->restoreGeometry(QSettings().value("geometry").toByteArray());
		mwid->show();
	}

	// screenshot
	// mwid->resize(QSize(1024, 720));

	//TODO FIX SEGFAULT
	mroot->exec();
}

int gui::exec()
{
	return 0;
}

void gui::layout()
{
	debug("layout");

	this->mfrm = new QGridLayout(mwid);

	this->mcnt = new QHBoxLayout;
	this->mstatusb = new QHBoxLayout;

	menuBarLayout();
	statusBarLayout();
	tabStackerLayout();

	mfrm->setContentsMargins(0, 0, 0, 0);
	mfrm->setSpacing(0);

	mfrm->addLayout(mcnt, 0, 0);
	mfrm->addLayout(mstatusb, 1, 0);
}

void gui::menuBarLayout()
{
	debug("menuBarLayout");

	QMenuBar* menu = menuBar(mfrm);

	QMenu* mfile = menuBarMenu(menu, tr("&File"));
	gmenu[GUI_CXE::FileNew] = menuBarAction(mfile, tr("&New"), [=]() { this->newTab(); }, QKeySequence::New);
	gmenu[GUI_CXE::FileOpen] = menuBarAction(mfile, tr("&Open"), [=]() { this->fileOpen(); }, QKeySequence::Open);
	menuBarSeparator(mfile);
	gmenu[GUI_CXE::FileSave] = menuBarAction(mfile, tr("&Save"), [=]() { this->fileSave(); }, QKeySequence::Save);
	gmenu[GUI_CXE::FileSaveAs] = menuBarAction(mfile, tr("Save &As…"), [=]() { this->fileSaveAs(); }, QKeySequence::SaveAs);
	menuBarSeparator(mfile);
	gmenu[GUI_CXE::FileImport] = menuBarAction(mfile, tr("Import"), [=]() { this->fileImport(); });
	gmenu[GUI_CXE::FileExport] = menuBarAction(mfile, tr("Export"), [=]() { this->fileExport(); });
	menuBarSeparator(mfile);
	gmenu[GUI_CXE::CloseTab] = menuBarAction(mfile, tr("Close Tab"), [=]() { this->closeTab(); }, QKeySequence::Close);
	gmenu[GUI_CXE::CloseAllTabs] = menuBarAction(mfile, tr("Close All Tabs"), [=]() { this->closeAllTabs(); }, Qt::CTRL | Qt::ALT | Qt::Key_W);
	menuBarSeparator(mfile);
	gmenu[GUI_CXE::FilePrint] = menuBarAction(mfile, tr("&Print"), [=]() { this->filePrint(); }, QKeySequence::Print);
	gmenu[GUI_CXE::FilePrintAll] = menuBarAction(mfile, tr("Print &All"), [=]() { this->filePrintAll(); }, Qt::CTRL | Qt::SHIFT | Qt::Key_P);
	menuBarSeparator(mfile);
	menuBarAction(mfile, tr("Settings"), [=]() { this->settingsDialog(); }, QKeySequence::Preferences);
#ifdef Q_OS_MAC
	menuBarAction(mfile, tr("&About"), [=]() { this->aboutDialog(); });
#endif
	menuBarSeparator(mfile);
	menuBarAction(mfile, tr("E&xit"), [=]() { this->mroot->quit(); }, QKeySequence::Quit);

	QMenu* medit = menuBarMenu(menu, tr("&Edit"));
	gmenu[GUI_CXE::TabListCut] = menuBarAction(medit, tr("Cu&t"), [=]() { this->tabAction(TAB_ATS::ListCut); }, QKeySequence::Cut);
	gmenu[GUI_CXE::TabListCopy] = menuBarAction(medit, tr("&Copy"), [=]() { this->tabAction(TAB_ATS::ListCopy); }, QKeySequence::Copy);
	gmenu[GUI_CXE::TabListPaste] = menuBarAction(medit, tr("&Paste"), [=]() { this->tabAction(TAB_ATS::ListPaste); }, QKeySequence::Paste);
	gmenu[GUI_CXE::TabListDelete] = menuBarAction(medit, tr("&Delete"), [=]() { this->tabAction(TAB_ATS::ListDelete); }, QKeySequence::Delete);
#ifdef Q_OS_MAC
	gmenu[GUI_CXE::TabListDelete]->setShortcut(Qt::Key_Backspace);
#endif
	menuBarSeparator(medit);
	gmenu[GUI_CXE::TabListSelectAll] = menuBarAction(medit, tr("Select &All"), [=]() { this->tabAction(TAB_ATS::ListSelectAll); }, QKeySequence::SelectAll);

	QMenu* mfind = menuBarMenu(menu, tr("&Find"));
	gmenu[GUI_CXE::TabListFind] = menuBarAction(mfind, tr("&Find Channel…"), [=]() { this->tabAction(TAB_ATS::ListFind); }, QKeySequence::Find);
	gmenu[GUI_CXE::TabListFindNext] = menuBarAction(mfind, tr("Find &Next"), [=]() { this->tabAction(TAB_ATS::ListFindNext); }, QKeySequence::FindNext);
	gmenu[GUI_CXE::TabListFindPrev] = menuBarAction(mfind, tr("Find &Previous"), [=]() { this->tabAction(TAB_ATS::ListFindPrev); }, QKeySequence::FindPrevious);
	gmenu[GUI_CXE::TabListFindAll] = menuBarAction(mfind, tr("Find &All"), [=]() { this->tabAction(TAB_ATS::ListFindAll); });
	menuBarSeparator(mfind);
	gmenu[GUI_CXE::TabTreeFind] = menuBarAction(mfind, tr("Find &Bouquet…"), [=]() { this->tabAction(TAB_ATS::TreeFind); }, Qt::CTRL | Qt::ALT | Qt::Key_F);
	gmenu[GUI_CXE::TabTreeFindNext] = menuBarAction(mfind, tr("Find N&ext Bouquet"), [=]() { this->tabAction(TAB_ATS::TreeFindNext); }, Qt::CTRL | Qt::ALT | Qt::Key_E);

	QMenu* mtools = menuBarMenu(menu, tr("&Tools"));
	gmenu[GUI_CXE::Transponders] = menuBarAction(mtools, "Edit Transponders", [=]() { this->tabAction(TAB_ATS::EditTransponders); });
	menuBarSeparator(mtools);
	gmenu[GUI_CXE::TunersetsSat] = menuBarAction(mtools, "Edit satellites.xml", [=]() { this->tabAction(TAB_ATS::EditTunersetsSat); });
	gmenu[GUI_CXE::TunersetsTerrestrial] = menuBarAction(mtools, "Edit terrestrial.xml", [=]() { this->tabAction(TAB_ATS::EditTunersetsTerrestrial); });
	gmenu[GUI_CXE::TunersetsCable] = menuBarAction(mtools, "Edit cables.xml", [=]() { this->tabAction(TAB_ATS::EditTunersetsCable); });
	gmenu[GUI_CXE::TunersetsAtsc] = menuBarAction(mtools, "Edit atsc.xml", [=]() { this->tabAction(TAB_ATS::EditTunersetsAtsc); });
	menuBarSeparator(mtools);
	gmenu[GUI_CXE::OpenChannelBook] = menuBarAction(mtools, "Show Channel book", [=]() { this->tabAction(TAB_ATS::ShowChannelBook); });
	menuBarSeparator(mtools);
	QMenu* mimportcsv = menuBarMenu(mtools, tr("Import from CSV"));
	gmenu[GUI_CXE::ToolsImportCSV_services] = menuBarAction(mimportcsv, "Import Services", [=]() { this->tabAction(TAB_ATS::ImportCSV_services); });
	gmenu[GUI_CXE::ToolsImportCSV_bouquet] = menuBarAction(mimportcsv, "Import Bouquet", [=]() { this->tabAction(TAB_ATS::ImportCSV_bouquet); });
	gmenu[GUI_CXE::ToolsImportCSV_userbouquet] = menuBarAction(mimportcsv, "Import Userbouquet", [=]() { this->tabAction(TAB_ATS::ImportCSV_userbouquet); });
	gmenu[GUI_CXE::ToolsImportCSV_tunersets] = menuBarAction(mimportcsv, "Import Tuner settings", [=]() { this->tabAction(TAB_ATS::ImportCSV_tunersets); });
	QMenu* mexportcsv = menuBarMenu(mtools, tr("Export to CSV"));
	gmenu[GUI_CXE::ToolsExportCSV_current] = menuBarAction(mexportcsv, "Export current", [=]() { this->tabAction(TAB_ATS::ExportCSV_current); });
	gmenu[GUI_CXE::ToolsExportCSV_all] = menuBarAction(mexportcsv, "Export All", [=]() { this->tabAction(TAB_ATS::ExportCSV_all); });
	gmenu[GUI_CXE::ToolsExportCSV_services] = menuBarAction(mexportcsv, "Export Services", [=]() { this->tabAction(TAB_ATS::ExportCSV_services); });
	gmenu[GUI_CXE::ToolsExportCSV_bouquets] = menuBarAction(mexportcsv, "Export Bouquets", [=]() { this->tabAction(TAB_ATS::ExportCSV_bouquets); });
	gmenu[GUI_CXE::ToolsExportCSV_userbouquets] = menuBarAction(mexportcsv, "Export Userbouquets", [=]() { this->tabAction(TAB_ATS::ExportCSV_userbouquets); });
	gmenu[GUI_CXE::ToolsExportCSV_tunersets] = menuBarAction(mexportcsv, "Export Tuner settings", [=]() { this->tabAction(TAB_ATS::ExportCSV_tunersets); });
	QMenu* mexporthtml = menuBarMenu(mtools, tr("Export to HTML"));
	gmenu[GUI_CXE::ToolsExportHTML_current] = menuBarAction(mexporthtml, "Export current", [=]() { this->tabAction(TAB_ATS::ExportHTML_current); });
	gmenu[GUI_CXE::ToolsExportHTML_all] = menuBarAction(mexporthtml, "Export All", [=]() { this->tabAction(TAB_ATS::ExportHTML_all); });
	gmenu[GUI_CXE::ToolsExportHTML_index] = menuBarAction(mexporthtml, "Export Index", [=]() { this->tabAction(TAB_ATS::ExportHTML_index); });
	gmenu[GUI_CXE::ToolsExportHTML_services] = menuBarAction(mexporthtml, "Export Services", [=]() { this->tabAction(TAB_ATS::ExportHTML_services); });
	gmenu[GUI_CXE::ToolsExportHTML_bouquets] = menuBarAction(mexporthtml, "Export Bouquets", [=]() { this->tabAction(TAB_ATS::ExportHTML_bouquets); });
	gmenu[GUI_CXE::ToolsExportHTML_userbouquets] = menuBarAction(mexporthtml, "Export Userbouquets", [=]() { this->tabAction(TAB_ATS::ExportHTML_userbouquets); });
	gmenu[GUI_CXE::ToolsExportHTML_tunersets] = menuBarAction(mexporthtml, "Export Tuner settings", [=]() { this->tabAction(TAB_ATS::ExportHTML_tunersets); });
	menuBarSeparator(mtools);
	gmenu[GUI_CXE::ToolsServicesOrder] = menuBarAction(mtools, "Order Services A-Z", todo);
	gmenu[GUI_CXE::ToolsBouquetsOrder] = menuBarAction(mtools, "Order Userbouquets A-Z", todo);
	gmenu[GUI_CXE::ToolsServicesCache] = menuBarAction(mtools, "Remove cached data from Services", todo);
	gmenu[GUI_CXE::ToolsBouquetsDelete] = menuBarAction(mtools, "Delete all Bouquets", todo);
	menuBarSeparator(mtools);
	gmenu[GUI_CXE::ToolsInspector] = menuBarAction(mtools, tr("Inspector Log"), [=]() { this->tabAction(TAB_ATS::Inspector); }, Qt::CTRL | Qt::ALT | Qt::Key_J);

	QMenu* mwind = menuBarMenu(menu, tr("&Window"));
	gmenu[GUI_CXE::WindowMinimize] = menuBarAction(mwind, tr("&Minimize"), [=]() { this->windowMinimize(); }, Qt::CTRL | Qt::Key_M);
	menuBarSeparator(mwind);
	gmenu[GUI_CXE::StatusBar] = menuBarAction(mwind, tr("Hide &Status Bar"), [=]() { this->statusBarToggle(); }, Qt::CTRL | Qt::ALT | Qt::Key_B);
	menuBarSeparator(mwind);
	gmenu[GUI_CXE::NewTab] = menuBarAction(mwind, tr("New &Tab"), [=]() { this->newTab(); }, Qt::CTRL | Qt::Key_T);
	menuBarSeparator(mwind);
	QActionGroup* mwtabs = menuBarActionGroup(mwind, true);

	QMenu* mhelp = menuBarMenu(menu, tr("&Help"));
	menuBarAction(mhelp, tr("About &Qt"), [=]() { mroot->aboutQt(); })->setMenuRole(QAction::NoRole);
	menuBarSeparator(mhelp);
	menuBarAction(mhelp, tr("&About e2 SAT Editor"), [=]() { this->aboutDialog(); });;//->setMenuRole(QAction::NoRole);

	this->menu = menu;
	this->mwind = mwind;
	this->mwtabs = mwtabs;
}

//TODO improve
//
//
// see link: https://github.com/KDE/falkon/blob/master/src/lib/app/proxystyle.cpp
//
void gui::tabStackerLayout()
{
	debug("tabStackerLayout");

	QWidget* twwrap = new QWidget(mwid);
	QGridLayout* twlayout = new QGridLayout(twwrap);

	this->twid = new QTabWidget;
	twid->setTabsClosable(true);
	twid->setMovable(true);
	twid->setUsesScrollButtons(true);
	twid->setStyle(new TabBarProxyStyle);
	twid->tabBar()->setObjectName("tabwidget_tabbar");
	twid->tabBar()->setChangeCurrentOnDrag(false);
	twid->tabBar()->setElideMode(Qt::ElideRight);

	twid->setStyleSheet("QTabWidget::tab-bar { left: 0 } QTabBar { border-style: solid } QTabWidget::pane { border: 0; border-radius: 0 } QTabBar::tab { min-width: 12ex; max-width: 25ex; height: 6.3ex; padding-left: 8px; padding-right: 8px; font-size: 13px; border-style: solid; border-width: 0 1px; color:palette(button-text); background: palette(button) } QTabBar::tab:selected { color:palette(highlighted-text); background: palette(highlight); border-color: transparent }");

	//TODO FIX
#ifdef Q_OS_WIN
	twid->tabBar()->setFixedHeight(44);
#endif

	QColor twtbshade;
	QString twtbshade_hexArgb;
#ifndef Q_OS_MAC
	twtbshade = QPalette().color(QPalette::Base);
	twtbshade.setAlphaF(0.25);
	twtbshade_hexArgb = twtbshade.name(QColor::HexArgb);

	theme->dynamicStyleSheet(twid, "#tabwidget_tabbar, #tabwidget_tabbar::tab { border-color: " + twtbshade_hexArgb + " }", theme::light);

	twtbshade = QPalette().color(QPalette::Mid);
	twtbshade.setAlphaF(0.22);
	twtbshade_hexArgb = twtbshade.name(QColor::HexArgb);

	theme->dynamicStyleSheet(twid, "#tabwidget_tabbar, #tabwidget_tabbar::tab { border-color: " + twtbshade_hexArgb + " }", theme::dark);
#else
	twtbshade = QColor(Qt::black);
	twtbshade.setAlphaF(0.08);
	twtbshade_hexArgb = twtbshade.name(QColor::HexArgb);

	theme->dynamicStyleSheet(twid, "#tabwidget_tabbar, #tabwidget_tabbar::tab { border-color: " + twtbshade_hexArgb + " }", theme::light);

	twtbshade = QPalette().color(QPalette::Dark).darker();
	twtbshade.setAlphaF(0.28);
	twtbshade_hexArgb = twtbshade.name(QColor::HexArgb);

	theme->dynamicStyleSheet(twid, "#tabwidget_tabbar, #tabwidget_tabbar::tab { border-color: " + twtbshade_hexArgb + " }", theme::dark);
#endif

	if (twid->layoutDirection() == Qt::LeftToRight)
		twid->tabBar()->setStyleSheet("QTabBar { border-width: 0 0 0 1px } QTabBar::tab { margin: 0 0 0 -1px }");
	else
		twid->tabBar()->setStyleSheet("QTabBar { border-width: 0 1px 0 0 } QTabBar::tab { margin: 0 -1px 0 0 }");

	twid->connect(twid, &QTabWidget::currentChanged, [=](int index) { this->tabChanged(index); });
	twid->connect(twid, &QTabWidget::tabCloseRequested, [=](int index) { this->closeTab(index); });
	twid->tabBar()->connect(twid->tabBar(), &QTabBar::tabMoved, [=](int from, int to) { this->tabMoved(from, to); });

	QWidget* twtbbase = new QWidget;
	twtbbase->setStyleSheet("min-height: 7ex");
	twtbbase->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
	//TODO FIX 4px frame

	QWidget* ttcornerwid = new QWidget;
	QHBoxLayout* ttcornerlayout = new QHBoxLayout(ttcornerwid);

	QPushButton* ttbnew = new QPushButton;
	ttbnew->setText(tr("New &Tab"));
	ttbnew->setIconSize(QSize(12, 12));
	ttbnew->setIcon(theme->dynamicIcon("add", ttbnew));
	ttbnew->setShortcut(QKeySequence::AddTab);
	ttbnew->setStyleSheet("QPushButton { min-width: 8ex; min-height: 7ex; padding-left: 3px; padding-right: 3px; font-size: 12px; font-weight: bold }");
	ttbnew->connect(ttbnew, &QPushButton::pressed, [=]() { this->newTab(); });

	ttcornerlayout->addWidget(ttbnew);
	ttcornerlayout->addItem(new QSpacerItem(4, 0));
	ttcornerlayout->setContentsMargins(0, 0, 0, 0);
	ttcornerlayout->setSpacing(0);

	platform::osWidgetBlend(ttcornerwid);

	twid->setCornerWidget(ttcornerwid, Qt::TopLeftCorner);

	launcher();

	platform::osWidgetOpaque(twtbbase);

	twlayout->addWidget(twtbbase, 0, 0, Qt::AlignTop);
	twlayout->addWidget(twid, 0, 0);
	twlayout->setContentsMargins(0, 0, 0, 0);

	mcnt->addWidget(twwrap);
}

void gui::statusBarLayout()
{
	debug("statusBarLayout");

	this->sbwid = new QStatusBar(mwid);
	this->sbwidl = new QLabel;
	this->sbwidc = new QWidget;
	this->sbwidr = new QLabel;

	sbwid->setStyleSheet("QStatusBar QLabel { padding: 0 2ex }");

#ifdef Q_OS_MAC
	theme->dynamicStyleSheet(sbwid, "QStatusBar { background: transparent }");
#endif

	sbwidl->setAlignment(Qt::AlignVCenter);
	sbwidr->setAlignment(Qt::AlignVCenter);
	sbwidl->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Ignored);
	sbwidr->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Ignored);

	sbwid->addWidget(sbwidl, 0);
	sbwid->addWidget(sbwidc, 1);
	sbwid->addWidget(sbwidr, 0);

	mstatusb->addWidget(sbwid);
}

void gui::settingsChanged()
{
	debug("settingsChanged");

	for (auto & x : ttabs)
	{
		tab* tab = x.second;
		tab->settingsChanged();
	}
}

void gui::themeChanged()
{
	debug("themeChanged");

	theme->changed();

	for (auto & x : ttabs)
	{
		tab* tab = x.second;
		tab->themeChanged();
	}
}

void gui::initSettings()
{
	debug("initSettings");

	QSettings settings;

	settings.setValue("settings/version", 1);
	settings.setValue("settings/reset", false);

	settings.setValue("application/version", mroot->applicationVersion());
	settings.setValue("application/debug", false);
#ifndef Q_OS_MAC
	settings.setValue("application/fixUnicodeChars", false);
#else
	settings.setValue("application/fixUnicodeChars", true);
#endif

	settings.beginGroup("preference");
	settings.setValue("askConfirmation", false);
	settings.setValue("nonDestructiveEdit", true);
	settings.endGroup();

	settings.beginWriteArray("profile");
	settings.setArrayIndex(0);
	settings.setValue("profileName", "Default");
	settings.setValue("ipAddress", "127.0.0.1");
	settings.setValue("ftpPort", 2121);
	settings.setValue("ftpActive", false);
	settings.setValue("httpPort", 80);
	settings.setValue("username", "root");
	settings.setValue("password", "test");
	settings.setValue("pathTransponders", "/enigma_db");
	settings.setValue("pathServices", "/enigma_db");
	settings.setValue("pathBouquets", "/enigma_db");
	settings.setValue("customWebifReloadUrl", "");
	settings.setValue("customTelnetReloadCmd", ""); 
	settings.endArray();
	settings.setValue("profile/selected", 0);
}

void gui::updateSettings()
{
	debug("updateSettings");

	if (! QSettings().contains("settings/version"))
	{
		QSettings settings;

		settings.setValue("settings/version", 1);
		settings.setValue("settings/reset", false);

		settings.setValue("application/version", mroot->applicationVersion());
	}
#ifdef Q_OS_MAC
	else if (QSettings().contains("preference/fixUnicodeChars"))
	{
		QSettings().setValue("application/fixUnicodeChars", true);
		QSettings().remove("preference/fixUnicodeChars");
	}
#endif
}

void gui::resetSettings()
{
	debug("resetSettings");

	QSettings().clear();

	initSettings();
}

void gui::tabViewSwitch(TAB_VIEW ttv)
{
	tabViewSwitch(ttv, 0);
}

void gui::tabViewSwitch(TAB_VIEW ttv, int arg)
{
	debug("tabViewSwitch", "ttv", ttv);

	switch (ttv)
	{
		case TAB_VIEW::main:
			gmenu[GUI_CXE::TabListFind]->setText(tr("&Find Channel"));
			gmenu[GUI_CXE::TabTreeFind]->setText(tr("Find &Bouquet"));
			gmenu[GUI_CXE::TabTreeFindNext]->setText(tr("Find N&ext Bouquet"));
		break;
		case TAB_VIEW::transponders:
			gmenu[GUI_CXE::TabListFind]->setText(tr("&Find Transponder"));
			gmenu[GUI_CXE::TabTreeFind]->setText(tr("Find &Bouquet"));
			gmenu[GUI_CXE::TabTreeFindNext]->setText(tr("Find N&ext Bouquet"));
		break;
		case TAB_VIEW::tunersets:
			gmenu[GUI_CXE::TabListFind]->setText(tr("&Find Transponder"));
			gmenu[GUI_CXE::TabTreeFind]->setText(tr("Find &Position"));
			gmenu[GUI_CXE::TabTreeFindNext]->setText(tr("Find N&ext Position"));
		break;
		case TAB_VIEW::channelBook:
			gmenu[GUI_CXE::TabListFind]->setText(tr("&Find Channel"));
			gmenu[GUI_CXE::TabTreeFind]->setText(tr("Find &Bouquet"));
			gmenu[GUI_CXE::TabTreeFindNext]->setText(tr("Find N&ext Bouquet"));
		break;
	}
}

int gui::newTab(string filename)
{
	tab* ttab = new tab(this, mwid);
	int ttid = ttab->getTabId();

	debug("newTab", "ttid", ttid);

	ttab->viewMain();

	bool read = ! filename.empty();
	QString ttname = "Untitled";
	int index = twid->addTab(ttab->widget, ttname);
	int count = index;

	QAction* action = new QAction(ttname);
	action->connect(action, &QAction::triggered, [=]() { this->twid->setCurrentWidget(ttab->widget); });
	action->setCheckable(true);
	action->setActionGroup(mwtabs);

	ttabs.emplace(ttid, ttab);
	ttmenu.emplace(ttid, action);
	twid->tabBar()->setTabData(index, ttid);

	if (read)
	{
		if (! ttab->readFile(filename))
		{
			error("newTab", "read", false);

			twid->removeTab(index);
			delete ttmenu[ttid];
			delete ttabs[ttid];
			ttmenu.erase(ttid);
			ttabs.erase(ttid);

			return -1;
		}

		filename = std::filesystem::path(filename).filename().u8string(); //C++17
		ttname = QString::fromStdString(filename);
	}
	else
	{
		ttname.append(QString::fromStdString(count ? " " + to_string(count) : ""));
	}
	twid->setTabText(index, ttname);
	twid->setTabToolTip(index, ttname);
	action->setText(ttname);
	ttab->setTabName(ttname.toStdString());

	twid->setCurrentIndex(index);
	mwind->addAction(action);

	return index;
}

int gui::openTab(TAB_VIEW view)
{
	return openTab(view, 0);
}

int gui::openTab(TAB_VIEW view, int arg)
{
	tab* current = getCurrentTabHandler();
	tab* parent = current;

	if (current == nullptr)
	{
		error("openTab", "current", false);
		return -1;
	}
	else if (current->isChild())
	{
		for (auto & x : ttabs)
		{
			tab* tab = x.second;

			if (tab != nullptr && tab->hasChildren())
			{
				for (auto & child : tab->children())
				{
					if (child == current)
					{
						parent = tab;
						break;
					}
				}
			}
		}
	}

	tab* ttab = new tab(this, mwid);
	int ttid = ttab->getTabId();

	debug("openTab", "ttid", ttid);

	int curr = twid->tabBar()->currentIndex();
	string parent_ttname = parent->getTabName();

	QIcon tticon;
	QString ttname = QString::fromStdString(parent_ttname);

	switch (view)
	{
		case TAB_VIEW::main:
			error("openTab", "parent", false);
			delete ttab;
			return -1;
		break;
		case TAB_VIEW::transponders:
			ttab->viewTransponders(parent);
			tticon = QIcon(theme::icon("transponders-view", theme::icon_highlight));
			ttname.append(" - ");
			ttname.append("Edit transponders");
		break;
		case TAB_VIEW::tunersets:
			ttab->viewTunersets(parent, arg);
			tticon = QIcon(theme::icon("tunersets-view", theme::icon_highlight));
			ttname.append(" - ");
			ttname.append("Edit settings");
		break;
		case TAB_VIEW::channelBook:
			ttab->viewChannelBook(parent);
			tticon = QIcon(theme::icon("channelbook-view", theme::icon_highlight));
			ttname.append(" - ");
			ttname.append("Channel book");
		break;
	}

	curr++;
	int index = twid->insertTab(curr, ttab->widget, tticon, ttname);
	twid->tabBar()->setTabData(index, ttid);

	QAction* action = new QAction(ttname);
	action->connect(action, &QAction::triggered, [=]() { this->twid->setCurrentWidget(ttab->widget); });
	action->setCheckable(true);
	action->setActionGroup(mwtabs);
	mwind->addAction(action);
	ttmenu.emplace(ttid, action);
	ttabs.emplace(ttid, ttab);

	twid->setTabToolTip(index, ttname);
	ttab->setTabName(ttname.toStdString());
	twid->setCurrentIndex(index);

	return index;
}

void gui::closeTab(int index)
{
	// debug("closeTab", "index", index);

	int ttid = getTabId(index);

	if (! ttabs.count(ttid))
	{
		return error("closeTab", "current", false);
	}

	tab* current = ttabs[ttid];

	debug("closeTab", "ttid", ttid);

	mwind->removeAction(ttmenu[ttid]);
	mwtabs->removeAction(ttmenu[ttid]);
	twid->removeTab(index);
	ttmenu.erase(ttid);

	if (current != nullptr)
	{
		if (current->isChild())
		{
			for (auto & x : ttabs)
			{
				tab* tab = x.second;

				if (tab != nullptr && tab->hasChildren())
				{
					for (auto & child : tab->children())
					{
						if (child == current)
						{
							tab->removeChild(current);
							break;
						}
					}
				}
			}
		}
		if (current->hasChildren())
		{
			for (auto & child : current->children())
			{
				int index = twid->indexOf(child->widget);

				current->removeChild(child);

				closeTab(index);
			}
		}
	}
	delete ttabs[ttid];
	ttabs.erase(ttid);

	if (twid->count() == 0)
		launcher();
}

void gui::closeAllTabs()
{
	debug("closeAllTabs");

	for (auto & action : mwtabs->actions())
	{
		mwind->removeAction(action);
		mwtabs->removeAction(action);
	}
	twid->clear();

	for (size_t i = 0; i < ttabs.size(); i++)
	{
		int x = int (i);
		debug("closeAllTabs", "index", x);

		delete ttabs[x];
		ttabs.erase(x);
		ttmenu.erase(x);
	}
	ttabs.clear();
	ttmenu.clear();

	launcher();
}

void gui::windowChanged()
{
	// debug("windowChanged");

	// main window busy
	if (mroot->activeWindow())
	{
		debug("windowChanged", "mwind", "busy");
		this->gxe = this->gex;
		update();
	}
	// main window idle
	else
	{
		debug("windowChanged", "mwind", "idle");
		this->gex = this->gxe;
		update(GUI_CXE::idle);
	}

	QSettings().setValue("geometry", mwid->saveGeometry());
}

void gui::tabChanged(int index)
{
	// debug("tabChanged", "index", index);

	int ttid = getTabId(index);

	if (ttid != -1)
	{
		debug("tabChanged", "current", ttid);

		tab* ttab = ttabs[ttid];
		ttmenu[ttid]->setChecked(true);
		ttab->tabSwitched();
		TAB_VIEW ttv = ttab->getTabView();
		tabViewSwitch(ttv);
	}
}

void gui::tabMoved(int from, int to)
{
	debug("tabMoved", "from|to", (to_string(from) + '|' + to_string(to)));

	auto actions = mwtabs->actions();
	actions.move(from, to);

	for (auto & action : actions)
	{
		mwtabs->removeAction(action);
		mwtabs->addAction(action);

		mwind->removeAction(action);
		mwind->addAction(action);
	}
}

void gui::tabChangeName(int ttid, string filename)
{
	debug("tabChangeName", "ttid", ttid);

	if (! ttabs.count(ttid))
	{
		return error("tabChangeName", "ttid", ttid);
	}

	tab* ttab = ttabs[ttid];
	int index = twid->indexOf(ttab->widget);
	int count = index;
	int v = ttab->getTabView();

	QString ttname = "Untitled";

	if (ttab->isChild())
	{
		for (auto & tab : ttabs)
		{
			if (tab.second != nullptr && tab.second->hasChildren())
			{
				for (auto & child : tab.second->children())
				{
					if (child == ttab)
					{
						int ttid = tab.second->getTabId();
						count = twid->indexOf(tab.second->widget);
						tabChangeName(ttid, filename);
						break;
					}
				}
			}
		}
	}
	if (filename.empty())
	{
		ttname.append(QString::fromStdString(count ? " " + to_string(count) : ""));
	}
	else
	{
		ttname = QString::fromStdString(filename);
	}

	// debug("tabChangeName", "index", index);

	switch (v)
	{
		case TAB_VIEW::transponders:
			ttname.append(" - ");
			ttname.append("Edit transponders");
		break;
		case TAB_VIEW::tunersets:
			ttname.append(" - ");
			ttname.append("Edit settings");
		break;
		case TAB_VIEW::channelBook:
			ttname.append(" - ");
			ttname.append("Channel book");
		break;
	}

	twid->setTabText(index, ttname);
	twid->setTabToolTip(index, ttname);
	ttmenu[ttid]->setText(ttname);
	ttab->setTabName(ttname.toStdString());
}

string gui::openFileDialog()
{
	debug("openFileDialog");

	QString caption = "Select enigma2 folder";

	string path;

	QFileDialog fdial = QFileDialog(mwid, caption);
	fdial.setAcceptMode(QFileDialog::AcceptOpen);
	fdial.setFileMode(QFileDialog::Directory);
	fdial.setOptions(QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
	if (fdial.exec() == QDialog::Accepted)
	{
		QUrl url = fdial.selectedUrls().value(0);
		if (url.isLocalFile() || url.isEmpty())
			url = url.toLocalFile();
		path = url.toString().toStdString();
	}
	return path;
}

string gui::saveFileDialog(string filename)
{
	debug("saveFileDialog", "filename", filename);

	QString caption = "Select where to save";

	string path;

	QFileDialog fdial = QFileDialog(mwid, caption, QString::fromStdString(filename));
	fdial.setAcceptMode(QFileDialog::AcceptOpen);
	fdial.setFilter(QDir::AllDirs | QDir::NoSymLinks);
	if (fdial.exec() == QDialog::Accepted)
	{
		QUrl url = fdial.selectedUrls().value(0);
		if (url.isLocalFile() || url.isEmpty())
			url = url.toLocalFile();
		path = url.toString().toStdString();
	}
	return path;
}

vector<string> gui::importFileDialog(GUI_DPORTS gde)
{
	debug("importFileDialog");

	QString caption = "Select one or more files to open";
	QStringList opts;
	QFileDialog::FileMode fmode = QFileDialog::ExistingFiles;
	switch (gde)
	{
		case GUI_DPORTS::Services:
			opts.append("Lamedb default (*)");
			opts.append("Lamedb 2.4 (lamedb)");
			opts.append("Lamedb 2.5 (lamedb5)");
			opts.append("Lamedb 2.3 (services)");
			opts.append("Lamedb 2.2 (services)");
		break;
		case GUI_DPORTS::Bouquets:
			opts.append("Bouquet (bouquets.*)");
		break;
		case GUI_DPORTS::Userbouquets:
			opts.append("Userbouquet (userbouquet.*)");
		break;
		case GUI_DPORTS::Tunersets:
			opts.append("Tuner settings (*.xml)");
		break;
		case GUI_DPORTS::CSV:
			opts.append("CSV File (*.csv *.txt)");
			opts.append("All Files (*)");
		break;
		case GUI_DPORTS::HTML:
			opts.append("HTML File (*.html *.htm)");
			opts.append("All Files (*)");
		break;
		default:
			fmode = QFileDialog::AnyFile;
			opts.append("Enigma2 folder (*)");
			opts.append("Lamedb 2.4 (lamedb)");
			opts.append("Lamedb 2.5 (lamedb5)");
			opts.append("Lamedb 2.3 (services)");
			opts.append("Lamedb 2.2 (services)");
			opts.append("Bouquet (bouquets.*)");
			opts.append("Userbouquet (userbouquet.*)");
			opts.append("Tuner settings (*.xml)");
			opts.append("All Files (*)");
	}

	vector<string> paths;

	QFileDialog fdial = QFileDialog(mwid, caption);
	fdial.setAcceptMode(QFileDialog::AcceptOpen);
	fdial.setFileMode(fmode);
	fdial.setNameFilters(opts);
	if (fdial.exec() == QDialog::Accepted)
	{
		for (QUrl & url : fdial.selectedUrls())
		{
			if (url.isLocalFile() || url.isEmpty())
				url = url.toLocalFile();
			paths.push_back(url.toString().toStdString());
		}
	}
	return paths;
}

string gui::exportFileDialog(GUI_DPORTS gde, string filename, int& bit)
{
	debug("exportFileDialog", "filename", filename);

	QString caption = "Select where to save";
	QStringList opts;
	switch (gde)
	{
		case GUI_DPORTS::Services:
			opts.append("Lamedb default (*)");
			opts.append("Lamedb 2.4 (lamedb)");
			opts.append("Lamedb 2.5 (lamedb5)");
			opts.append("Lamedb 2.3 (services)");
			opts.append("Lamedb 2.2 (services)");
		break;
		case GUI_DPORTS::Bouquets:
			opts.append("Bouquet (bouquets.*)");
		break;
		case GUI_DPORTS::Userbouquets:
			opts.append("Userbouquet (userbouquet.*)");
		break;
		case GUI_DPORTS::Tunersets:
			opts.append("Tuner settings (*.xml)");
		break;
		case GUI_DPORTS::CSV:
			opts.append("CSV File (*.csv)");
			opts.append("All Files (*)");
		break;
		case GUI_DPORTS::HTML:
			opts.append("HTML File (*.html)");
			opts.append("All Files (*)");
		break;
		default:
			opts.append("All Files (*)");
	}

	string path;

	QString selected;
	QFileDialog fdial = QFileDialog(mwid, caption, QString::fromStdString(filename));
	fdial.setAcceptMode(QFileDialog::AcceptSave);
	fdial.setFilter(QDir::AllDirs | QDir::NoSymLinks);
	fdial.setNameFilters(opts);
	if (fdial.exec() == QDialog::Accepted)
	{
		selected = fdial.selectedNameFilter();

		// straight copy of e2db::FPORTS
		if (selected == "Lamedb 2.4 (lamedb)")
			bit = 0x1224;
		else if (selected == "Lamedb 2.5 (lamedb5)")
			bit = 0x1225;
		else if (selected == "Lamedb 2.3 (services)")
			bit = 0x1223;
		else if (selected == "Lamedb 2.2 (services)")
			bit = 0x1222;

		debug("exportFileDialog", "bit", bit);

		QUrl url = fdial.selectedUrls().value(0);
		if (url.isLocalFile() || url.isEmpty())
			url = url.toLocalFile();
		path = url.toString().toStdString();
	}
	return path;
}

string gui::exportFileDialog(GUI_DPORTS gde, string filename)
{
	int bit = 0;
	return exportFileDialog(gde, filename, bit);
}

bool gui::statusBarIsVisible()
{
	return sbwid->isVisible();
}

bool gui::statusBarIsHidden()
{
	return sbwid->isHidden();
}

void gui::statusBarShow()
{
	debug("statusBarShow");

	sbwid->show();
	gmenu[GUI_CXE::StatusBar]->setText(tr("Hide &Status Bar"));
}

void gui::statusBarHide()
{
	debug("statusBarHide");

	sbwid->hide();
	gmenu[GUI_CXE::StatusBar]->setText(tr("Show &Status Bar"));
}

void gui::statusBarToggle()
{
	// debug("statusBarToggle");

	if (sbwid->isHidden())
	{
		statusBarShow();
	}
	else
	{
		statusBarHide();
	}
}

void gui::setStatusBar(status msg)
{
	debug("setStatusBar");

	QString separator = "   ";
	QString text;

	if (msg.info && ! msg.message.empty())
	{
		text = QString::fromStdString(msg.message);
		sbwid->showMessage(text);
	}
	else if (msg.view == TAB_VIEW::main)
	{
		if (msg.update)
		{
			if (msg.counters[COUNTER::n_bouquet])
			{
				text.append("Channels: " + QString::number(msg.counters[COUNTER::n_bouquet]));
			}
			if (! msg.curr.empty())
			{
				text.append(separator);
				text.append("Bouquet: " + QString::fromStdString(msg.curr));
			}
			sbwidl->setText(text);
		}
		else
		{
			text.append("TV: " + QString::number(msg.counters[COUNTER::n_tv]));
			text.append(separator);
			text.append("Radio: " + QString::number(msg.counters[COUNTER::n_radio]));
			text.append(separator);
			text.append("Data: " + QString::number(msg.counters[COUNTER::n_data]));
			text.append(separator);
			text.append("Total: " + QString::number(msg.counters[COUNTER::n_services]));
			sbwidr->setText(text);
		}
	}
	else if (msg.view == TAB_VIEW::transponders)
	{
		text.append("Total: " + QString::number(msg.counters[COUNTER::n_transponders]));
		sbwidl->setText("");
		sbwidr->setText(text);
	}
	else if (msg.view == TAB_VIEW::tunersets)
	{
		if (msg.update)
		{
			if (msg.counters[COUNTER::n_position])
			{
				text.append("Transponders: " + QString::number(msg.counters[COUNTER::n_position]));
			}
			if (! msg.curr.empty())
			{
				text.append(separator);
				text.append("Position: " + QString::fromStdString(msg.curr));
			}
			sbwidl->setText(text);
		}
		else
		{
			text.append("Total: " + QString::number(msg.counters[COUNTER::n_transponders]));
			sbwidr->setText(text);
		}
	}
}

void gui::resetStatusBar()
{
	debug("resetStatusBar");

	if (sbwid->currentMessage().isEmpty())
	{
		sbwidl->setText("");
		sbwidr->setText("");
	}
	else
	{
		sbwid->clearMessage();
		sbwid->addWidget(sbwidl, 0);
		sbwid->addWidget(sbwidc, 1);
		sbwid->addWidget(sbwidr, 0);
	}
}

void gui::fileOpen()
{
	debug("fileOpen");

	string path = openFileDialog();

	if (! path.empty())
		newTab(path);
}

void gui::fileSave()
{
	debug("fileSave");

	tab* ttab = getCurrentTabHandler();
	if (ttab != nullptr)
		ttab->saveFile(false);
}

void gui::fileSaveAs()
{
	debug("fileSaveAs");

	tab* ttab = getCurrentTabHandler();
	if (ttab != nullptr)
		ttab->saveFile(true);
}

void gui::fileImport()
{
	debug("fileImport");

	tab* ttab = getCurrentTabHandler();
	if (ttab != nullptr)
		ttab->importFile();
}

void gui::fileExport()
{
	debug("fileExport");

	tab* ttab = getCurrentTabHandler();
	if (ttab != nullptr)
		ttab->exportFile();
}

void gui::filePrint()
{
	debug("filePrint");

	tab* ttab = getCurrentTabHandler();
	if (ttab != nullptr)
		ttab->printFile(false);
}

void gui::filePrintAll()
{
	debug("filePrintAll");

	tab* ttab = getCurrentTabHandler();
	if (ttab != nullptr)
		ttab->printFile(true);
}

void gui::tabAction(TAB_ATS action)
{
	debug("tabAction", "action", action);

	tab* ttab = getCurrentTabHandler();
	if (ttab != nullptr)
		ttab->actionCall(action);
}

void gui::windowMinimize()
{
	debug("windowMinimize");

	if (! this->mwid->isMinimized())
		this->mwid->showMinimized();
}

void gui::settingsDialog()
{
	new e2se_gui_dialog::settings(this, mwid);
}

void gui::aboutDialog()
{
	//TODO potential leak
	new e2se_gui_dialog::about();
}

bool gui::getFlag(GUI_CXE bit)
{
	return this->gxe[bit];
}

bool gui::getFlag(int bit)
{
	return this->gxe[bit];
}

void gui::setFlag(GUI_CXE bit, bool flag)
{
	update(bit, flag);
}

bitset<256> gui::getFlags()
{
	return this->gxe;
}

void gui::setFlags(bitset<256> bits)
{
	this->gxe = bits;
	update();
}

void gui::setFlags(vector<int> bits, bool flag)
{
	update(bits, flag);
}

int gui::getTabId(int index)
{
	int ttid = twid->tabBar()->tabData(index).toInt();
	if (! ttabs.count(ttid))
		ttid = -1;
	return ttid;
}

int gui::getCurrentTabId()
{
	int index = twid->tabBar()->currentIndex();
	return getTabId(index);
}

tab* gui::getCurrentTabHandler()
{
	int ttid = getCurrentTabId();
	return ttid != -1 ? ttabs[ttid] : nullptr;
}

void gui::launcher()
{
	debug("launcher");

	update(GUI_CXE::init);
	newTab();
	tabChanged(0);
}

void gui::update()
{
	debug("update");

	for (auto & x : gmenu)
	{
		if (this->gxe[x.first])
			x.second->setEnabled(true);
		else
			x.second->setDisabled(true);
	}

	// note: is out of range
	// debug("update", "flags", getActionFlags().to_ullong());
}

void gui::update(int bit, bool flag)
{
	 // debug("update", "overload", bit);

	typedef size_t position_t;
	QAction* action = gmenu.count(bit) ? gmenu[bit] : nullptr;

	if (action != nullptr)
		action->setEnabled(flag);

	this->gxe.set(position_t (bit), flag);

	this->gex = this->gxe;
}

void gui::update(vector<int> bits, bool flag)
{
	// debug("update", "overload", 1);

	typedef size_t position_t;

	for (int & bit : bits)
		this->gxe.set(position_t (bit), flag);

	update();
}

void gui::update(vector<int> bits)
{
	// debug("update", "overload", 0);

	typedef size_t position_t;

	for (int & bit : bits)
		this->gxe.set(position_t (bit), true);

	update();
}

void gui::update(int bit)
{
	// debug("update", "overload", 0);

	this->gxe.reset();

	if (bit == 0)
		update(GUI_CXE__init);
	else if (bit == -1)
		update(GUI_CXE__idle);

	// note: is out of range
	// debug("update", "flags", getFlags().to_ullong());
}

QMenuBar* gui::menuBar(QLayout* layout)
{
	QMenuBar* menu = new QMenuBar;
	menu->setNativeMenuBar(true);
	layout->setMenuBar(menu);
	return menu;
}

QMenu* gui::menuBarMenu(QMenuBar* menubar, QString title)
{
	QMenu* menu = new QMenu(menubar);
	menu->setTitle(title);
	menubar->addMenu(menu);
	return menu;
}

QMenu* gui::menuBarMenu(QMenu* menu, QString title)
{
	QMenu* submenu = new QMenu(menu);
	submenu->setTitle(title);
	menu->addMenu(submenu);
	return submenu;
}

QAction* gui::menuBarAction(QMenu* menu, QString text, std::function<void()> trigger)
{
	QAction* action = new QAction(menu);
	action->setText(text);
	action->connect(action, &QAction::triggered, trigger);
	menu->addAction(action);
	return action;
}

QAction* gui::menuBarAction(QMenu* menu, QString text, std::function<void()> trigger, QKeySequence shortcut)
{
	QAction* action = new QAction(menu);
	action->setText(text);
	action->setShortcut(shortcut);
	action->connect(action, &QAction::triggered, trigger);
	menu->addAction(action);
	return action;
}

QAction* gui::menuBarSeparator(QMenu* menu)
{
	QAction* action = new QAction(menu);
	action->setSeparator(true);
	menu->addAction(action);
	return action;
}

QActionGroup* gui::menuBarActionGroup(QMenu* menu, bool exclusive)
{
	QActionGroup* group = new QActionGroup(menu);
	group->setExclusive(exclusive);
	return group;
}

}
