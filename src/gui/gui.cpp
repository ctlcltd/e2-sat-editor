/*!
 * e2-sat-editor/src/gui/gui.cpp
 *
 * @link https://github.com/ctlcltd/e2-sat-editor
 * @copyright e2 SAT Editor Team
 * @author Leonardo Laureti
 * @version 1.4.0
 * @license MIT License
 * @license GNU GPLv3 License
 */

#include <clocale>
#include <functional>
#include <filesystem>

#include <QtGlobal>
#include <QTimer>
#include <QRegularExpression>
#include <QSettings>
#include <QTranslator>
#include <QLibraryInfo>
#include <QCommandLineParser>
#include <QByteArray>
#include <QMessageBox>
#include <QSplitter>
#include <QGroupBox>
#include <QTabWidget>
#include <QPushButton>
#include <QFileDialog>
#include <QClipboard>
#include <QUrl>
#include <QDesktopServices>
#include <QMouseEvent>
#ifdef Q_OS_WIN
#include <QStyleFactory>
#endif

#include "../e2se_defs.h"
#include "platforms/platform.h"

#ifndef Q_OS_WASM
#include <QThread>
#endif

#ifdef E2SE_CHECKUPDATE
#include "checkUpdate.h"
#endif

#include "toolkit/ThemeChangeEventObserver.h"
#include "toolkit/TabBarProxyStyle.h"
#include "gui.h"
#include "theme.h"
#include "tab.h"
#include "settings.h"
#include "about.h"

using std::to_string;

using namespace e2se;

namespace e2se_gui
{

gui::gui(int argc, char* argv[])
{
	std::setlocale(LC_NUMERIC, "C");

	this->log = new logger("gui", "gui");

	theme::checkStyleOverride(argc, argv);

	this->mroot = new QApplication(argc, argv);
	std::setlocale(LC_NUMERIC, "C");

	mroot->setOrganizationName("e2 SAT Editor Team");
	mroot->setOrganizationDomain("io.github.ctlcltd");
	mroot->setApplicationName("e2se");
	mroot->setApplicationVersion("1.4");

	mroot->connect(mroot, &QApplication::focusChanged, [=]() { this->windowChanged(); });

	QString appPath = mroot->applicationDirPath();
#if E2SE_BUILD == E2SE_TARGET_DEBUG
	if (appPath.contains("/src"))
		appPath.truncate(appPath.indexOf("/src"));
#endif

#ifdef E2SE_PORTABLE
	QSettings::setDefaultFormat(QSettings::IniFormat);
	QSettings::setPath(QSettings::IniFormat, QSettings::UserScope, appPath);
#endif

	if (QSettings().value("settings/reset", false).toBool())
		resetSettings();
	else if (QSettings().value("settings/version").isValid())
		updateSettings();
	else
		initSettings();

	// L10n
	QString appLang = QSettings().value("preference/language").toString();
	QLocale appLocale = appLang.isEmpty() ? QLocale::system() : QLocale(appLang);
#if E2SE_BUILD == E2SE_TARGET_DEBUG
	QString appTranslationsPath = QString(appPath).append("/dist/translations");
#else
	QString appTranslationsPath = mroot->applicationDirPath();
#ifndef Q_OS_MAC
	if (appTranslationsPath.contains("/bin"))
	{
		appTranslationsPath.truncate(appTranslationsPath.indexOf("/bin"));
		appTranslationsPath.append("/share/e2-sat-editor/translations");
	}
	else
	{
		appTranslationsPath.append("/translations");
	}
#else
	if (appTranslationsPath.contains("/Contents/MacOS"))
	{
		appTranslationsPath.truncate(appTranslationsPath.indexOf("/Contents/MacOS"));
		appTranslationsPath.append("/Contents/Resources/translations");
	}
#endif
#endif

#if defined E2SE_PORTABLE
	QString qtTranslationsPath = appTranslationsPath;
#elif defined Q_OS_MAC && E2SE_BUILD == E2SE_TARGET_RELEASE
	QString qtTranslationsPath = appTranslationsPath;
#else
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
	QString qtTranslationsPath = QLibraryInfo::path(QLibraryInfo::TranslationsPath);
#else
	QString qtTranslationsPath = QLibraryInfo::location(QLibraryInfo::TranslationsPath);
#endif
#endif

	debug("gui", "language preference", appLang.isEmpty() ? "system" : appLang.toStdString());

	QTranslator qtTranslator;
	if (qtTranslator.load(appLocale, "qtbase", "_", qtTranslationsPath))
	{
		mroot->installTranslator(&qtTranslator);
	}
	QTranslator appTranslator;
	if (appTranslator.load(appLocale, "e2se", "_", appTranslationsPath))
	{
		debug("gui", "language loaded", appTranslator.language().toStdString());

		mroot->installTranslator(&appTranslator);
	}

	// i18n rtl
	// mroot->setLayoutDirection(Qt::RightToLeft);

	QCommandLineParser clp;
	QCommandLineOption debugOption ("debug");
	clp.addOption(debugOption);
	clp.addVersionOption();
	clp.addPositionalArgument("file", "");
	clp.process((*mroot->instance()));

	QStringList args = clp.positionalArguments();
	if (! args.isEmpty() && ! args.first().isEmpty())
	{
		this->ifpath = args.first().toStdString();
	}

	this->theme = new e2se_gui::theme(mroot);
	theme->initStyle();

	this->mwid = new QWidget;
	mwid->setWindowTitle("e2 SAT Editor");

#ifdef Q_OS_WIN
	theme->early_win_flavor_fix(mwid);
	theme->early_win_flavor_fix(mroot);
#endif

	ThemeChangeEventObserver* gce = new ThemeChangeEventObserver;
	gce->setEventCallback([=]() { this->themeChanged(); });
	mwid->installEventFilter(gce);

	QClipboard* clipboard = mroot->clipboard();
	//TODO TEST potential SEGFAULT
	clipboard->connect(clipboard, &QClipboard::dataChanged, [=]() { this->clipboardDataChanged(); });

	platform::osWindowBlend(mwid);

	layout();

	mainWindowShowAndGainFocus();

#ifdef E2SE_CHECKUPDATE
	if (QSettings().value("preference/autoCheckUpdate", false).toBool())
		autoCheckUpdate();
#endif

	mroot->exec();
}

gui::~gui()
{
	debug("~gui");

	delete this->mroot;
	delete this->mwid;
	delete this->theme;
	delete this->log;
}

int gui::exited()
{
	return 0;
}

// maybe a regression in Qt 6.6.x main window lowered [macOS]
// no way: lower, raise, activateWindow, requestActivateWindow
// temporary workaround
void gui::mainWindowShowAndGainFocus()
{
#ifdef Q_OS_MAC
	if (QSettings().value("geometry").isNull())
	{
		mwid->resize(960, 670);
#if QT_VERSION <= QT_VERSION_CHECK(6, 5, 1)
		mwid->showMaximized();
#endif
	}
	else
	{
		mwid->restoreGeometry(QSettings().value("geometry").toByteArray());
#if QT_VERSION <= QT_VERSION_CHECK(6, 5, 1)
		mwid->show();
#endif
	}

#if QT_VERSION >= QT_VERSION_CHECK(6, 5, 1)
	mroot->connect(mroot, &QApplication::applicationStateChanged, [=]() { this->mainWindowDelayedShow(); });

	QTimer::singleShot(6e3, [=]() { this->mainWindowDelayedShow(); });
#endif
#else
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
#endif
}

void gui::mainWindowDelayedShow()
{
	if (! mroot->property("_started_macx").toBool())
	{
		if (QSettings().value("geometry").isNull())
			mwid->showMaximized();
		else
			mwid->show();

		mroot->setProperty("_started_macx", true);
	}
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

	QMenuBar* menubar = menuBar(mfrm);

	//: Platform: File menu
	QMenu* mfile = menuBarMenu(menubar, tr("&File", "menu"));
	gmenu[GUI_CXE::FileNew] = menuBarAction(mfile, tr("&New", "menu"), [=]() { this->newTab(); }, QKeySequence::New);
	gmenu[GUI_CXE::FileOpen] = menuBarAction(mfile, tr("&Open", "menu"), [=]() { this->fileOpen(); }, QKeySequence::Open);
	menuBarSeparator(mfile);
	gmenu[GUI_CXE::FileSave] = menuBarAction(mfile, tr("&Save", "menu"), [=]() { this->fileSave(); }, QKeySequence::Save);
	//: Encoding: preserve 3-dots ellipsis symbol
	gmenu[GUI_CXE::FileSaveAs] = menuBarAction(mfile, tr("Save &As…", "menu"), [=]() { this->fileSaveAs(); }, QKeySequence::SaveAs);
	menuBarSeparator(mfile);
	gmenu[GUI_CXE::FileImport] = menuBarAction(mfile, tr("Import", "menu"), [=]() { this->fileImport(); });
	gmenu[GUI_CXE::FileExport] = menuBarAction(mfile, tr("Export", "menu"), [=]() { this->fileExport(); });
	menuBarSeparator(mfile);
	gmenu[GUI_CXE::CloseTab] = menuBarAction(mfile, tr("Close Tab", "menu"), [=]() { this->closeTab(); }, QKeySequence::Close);
	gmenu[GUI_CXE::CloseAllTabs] = menuBarAction(mfile, tr("Close All Tabs", "menu"), [=]() { this->closeAllTabs(); }, Qt::CTRL | Qt::ALT | Qt::Key_W);
	menuBarSeparator(mfile);
	gmenu[GUI_CXE::FileInfo] = menuBarAction(mfile, tr("File Information", "menu"), [=]() { this->fileInfo(); });
	menuBarSeparator(mfile);
	//: Encoding: preserve 3-dots ellipsis symbol
	gmenu[GUI_CXE::FilePrint] = menuBarAction(mfile, tr("&Print…", "menu"), [=]() { this->filePrint(); }, QKeySequence::Print);
	gmenu[GUI_CXE::FilePrintAll] = menuBarAction(mfile, tr("Print &All", "menu"), [=]() { this->filePrintAll(); }, Qt::CTRL | Qt::SHIFT | Qt::Key_P);
	menuBarSeparator(mfile);
#ifdef Q_OS_MAC
	//: Platform: Preferences item in macOS Application Menu |
	//: Encoding: preserve 3-dots ellipsis symbol
	menuBarAction(mfile, tr("Preferences…", "menu"), [=]() { this->settingsDialog(); }, QKeySequence::Preferences)->setMenuRole(QAction::PreferencesRole);
	//: Platform: About [$BUNDLE_NAME] item in macOS Application Menu
	menuBarAction(mfile, tr("About", "menu"), [=]() { this->aboutDialog(); })->setMenuRole(QAction::AboutRole);
	menuBarAction(mfile, tr("Check for updates…", "menu"), [=]() { this->checkUpdate(); })->setMenuRole(QAction::AboutQtRole);
#endif
	menuBarAction(mfile, tr("Settings", "menu"), [=]() { this->settingsDialog(); }, QKeySequence::Preferences);
	menuBarSeparator(mfile);
	//: Platform: Exit | Quit item
	menuBarAction(mfile, tr("E&xit", "menu"), [=]() { this->mroot->quit(); }, QKeySequence::Quit);

#ifdef Q_OS_MAC
	//: Platform: Edit menu in macOS Menu bar
	QT_TRANSLATE_NOOP("QCocoaMenu", "Edit");
	//: Platform: edit item in macOS Edit Menu |
	//: Encoding: preserve 3 singular dots
	QT_TRANSLATE_NOOP("QCocoaMenuItem", "Start Dictation...");
	//: Platform: edit item in macOS Edit Menu |
	//: Encoding: note double ampersand
	QT_TRANSLATE_NOOP("QCocoaMenuItem", "Emoji && Symbols");
#endif

	//: Platform: Edit menu
	QMenu* medit = menuBarMenu(menubar, tr("&Edit", "menu"));
	gmenu[GUI_CXE::EditUndo] = menuBarAction(medit, tr("&Undo", "menu"), [=]() { this->editAction(GUI_CXE::EditUndo); }, QKeySequence::Undo);
	gmenu[GUI_CXE::EditRedo] = menuBarAction(medit, tr("&Redo", "menu"), [=]() { this->editAction(GUI_CXE::EditRedo); }, QKeySequence::Redo);
	menuBarSeparator(medit);
	gmenu[GUI_CXE::EditCut] = menuBarAction(medit, tr("Cu&t", "menu"), [=]() { this->editAction(GUI_CXE::EditCut); }, QKeySequence::Cut);
	gmenu[GUI_CXE::EditCopy] = menuBarAction(medit, tr("&Copy", "menu"), [=]() { this->editAction(GUI_CXE::EditCopy); }, QKeySequence::Copy);
	gmenu[GUI_CXE::EditPaste] = menuBarAction(medit, tr("&Paste", "menu"), [=]() { this->editAction(GUI_CXE::EditPaste); }, QKeySequence::Paste);
	gmenu[GUI_CXE::EditDelete] = menuBarAction(medit, tr("&Delete", "menu"), [=]() { this->editAction(GUI_CXE::EditDelete); }, QKeySequence::Delete);
#ifdef Q_OS_MAC
	gmenu[GUI_CXE::EditDelete]->setShortcut(Qt::Key_Backspace);
#endif
	menuBarSeparator(medit);
	gmenu[GUI_CXE::EditSelectAll] = menuBarAction(medit, tr("Select &All", "menu"), [=]() { this->editAction(GUI_CXE::EditSelectAll); }, QKeySequence::SelectAll);

	//: Platform: Find menu
	QMenu* mfind = menuBarMenu(menubar, tr("&Find", "menu"));
	//: Encoding: preserve 3-dots ellipsis symbol
	gmenu[GUI_CXE::TabListFind] = menuBarAction(mfind, tr("&Find Channel…", "menu"), [=]() { this->tabAction(TAB_ATS::ListFind); }, QKeySequence::Find);
	gmenu[GUI_CXE::TabListFindNext] = menuBarAction(mfind, tr("Find &Next", "menu"), [=]() { this->tabAction(TAB_ATS::ListFindNext); }, QKeySequence::FindNext);
	gmenu[GUI_CXE::TabListFindPrev] = menuBarAction(mfind, tr("Find &Previous", "menu"), [=]() { this->tabAction(TAB_ATS::ListFindPrev); }, QKeySequence::FindPrevious);
	gmenu[GUI_CXE::TabListFindAll] = menuBarAction(mfind, tr("Find &All", "menu"), [=]() { this->tabAction(TAB_ATS::ListFindAll); });
	menuBarSeparator(mfind);
	//: Encoding: preserve 3-dots ellipsis symbol
	gmenu[GUI_CXE::TabTreeFind] = menuBarAction(mfind, tr("Find &Bouquet…", "menu"), [=]() { this->tabAction(TAB_ATS::TreeFind); }, Qt::CTRL | Qt::ALT | Qt::Key_F);
	gmenu[GUI_CXE::TabTreeFindNext] = menuBarAction(mfind, tr("Find N&ext Bouquet", "menu"), [=]() { this->tabAction(TAB_ATS::TreeFindNext); }, Qt::CTRL | Qt::ALT | Qt::Key_E);

	QMenu* mtools = menuBarMenu(menubar, tr("&Tools", "menu"));
	gmenu[GUI_CXE::Transponders] = menuBarAction(mtools, tr("Edit Transponders", "menu"), [=]() { this->tabAction(TAB_ATS::EditTransponders); });
	menuBarSeparator(mtools);
	//: Note: %1 is xml filename
	gmenu[GUI_CXE::TunersetsSat] = menuBarAction(mtools, tr("Edit %1", "menu").arg("satellites.xml"), [=]() { this->tabAction(TAB_ATS::EditTunersetsSat); });
	//: Note: %1 is xml filename
	gmenu[GUI_CXE::TunersetsTerrestrial] = menuBarAction(mtools, tr("Edit %1", "menu").arg("terrestrial.xml"), [=]() { this->tabAction(TAB_ATS::EditTunersetsTerrestrial); });
	//: Note: %1 is xml filename
	gmenu[GUI_CXE::TunersetsCable] = menuBarAction(mtools, tr("Edit %1", "menu").arg("cables.xml"), [=]() { this->tabAction(TAB_ATS::EditTunersetsCable); });
	//: Note: %1 is xml filename
	gmenu[GUI_CXE::TunersetsAtsc] = menuBarAction(mtools, tr("Edit %1", "menu").arg("atsc.xml"), [=]() { this->tabAction(TAB_ATS::EditTunersetsAtsc); });
	menuBarSeparator(mtools);
	gmenu[GUI_CXE::Picons] = menuBarAction(mtools, tr("Picons editor", "menu"), [=]() { this->tabAction(TAB_ATS::EditPicons); });
	gmenu[GUI_CXE::OpenChannelBook] = menuBarAction(mtools, tr("Channel Book", "menu"), [=]() { this->tabAction(TAB_ATS::ShowChannelBook); });
	menuBarSeparator(mtools);
	QMenu* mimportcsv = menuBarMenu(mtools, tr("Import from CSV", "menu"));
	gmenu[GUI_CXE::ToolsImportCSV_services] = menuBarAction(mimportcsv, tr("Import Services", "menu"), [=]() { this->tabAction(TAB_ATS::ImportCSV_services); });
	gmenu[GUI_CXE::ToolsImportCSV_bouquet] = menuBarAction(mimportcsv, tr("Import Bouquet", "menu"), [=]() { this->tabAction(TAB_ATS::ImportCSV_bouquet); });
	gmenu[GUI_CXE::ToolsImportCSV_userbouquet] = menuBarAction(mimportcsv, tr("Import Userbouquet", "menu"), [=]() { this->tabAction(TAB_ATS::ImportCSV_userbouquet); });
	gmenu[GUI_CXE::ToolsImportCSV_tunersets] = menuBarAction(mimportcsv, tr("Import Tuner settings", "menu"), [=]() { this->tabAction(TAB_ATS::ImportCSV_tunersets); });
	QMenu* mexportcsv = menuBarMenu(mtools, tr("Export to CSV", "menu"));
	gmenu[GUI_CXE::ToolsExportCSV_current] = menuBarAction(mexportcsv, tr("Export current", "menu"), [=]() { this->tabAction(TAB_ATS::ExportCSV_current); });
	gmenu[GUI_CXE::ToolsExportCSV_all] = menuBarAction(mexportcsv, tr("Export All", "menu"), [=]() { this->tabAction(TAB_ATS::ExportCSV_all); });
	gmenu[GUI_CXE::ToolsExportCSV_services] = menuBarAction(mexportcsv, tr("Export Services", "menu"), [=]() { this->tabAction(TAB_ATS::ExportCSV_services); });
	gmenu[GUI_CXE::ToolsExportCSV_bouquets] = menuBarAction(mexportcsv, tr("Export Bouquets", "menu"), [=]() { this->tabAction(TAB_ATS::ExportCSV_bouquets); });
	gmenu[GUI_CXE::ToolsExportCSV_userbouquets] = menuBarAction(mexportcsv, tr("Export Userbouquets", "menu"), [=]() { this->tabAction(TAB_ATS::ExportCSV_userbouquets); });
	gmenu[GUI_CXE::ToolsExportCSV_tunersets] = menuBarAction(mexportcsv, tr("Export Tuner settings", "menu"), [=]() { this->tabAction(TAB_ATS::ExportCSV_tunersets); });
	menuBarSeparator(mtools);
	gmenu[GUI_CXE::ToolsImportM3U] = menuBarAction(mtools, tr("Import from M3U", "menu"), [=]() { this->tabAction(TAB_ATS::ImportM3U); });
	gmenu[GUI_CXE::ToolsExportM3U] = menuBarAction(mtools, tr("Export to M3U", "menu"), [=]() { this->tabAction(TAB_ATS::ExportM3U); });
	menuBarSeparator(mtools);
	QMenu* mexporthtml = menuBarMenu(mtools, tr("Export to HTML", "menu"));
	gmenu[GUI_CXE::ToolsExportHTML_current] = menuBarAction(mexporthtml, tr("Export current", "menu"), [=]() { this->tabAction(TAB_ATS::ExportHTML_current); });
	gmenu[GUI_CXE::ToolsExportHTML_all] = menuBarAction(mexporthtml, tr("Export All", "menu"), [=]() { this->tabAction(TAB_ATS::ExportHTML_all); });
	gmenu[GUI_CXE::ToolsExportHTML_index] = menuBarAction(mexporthtml, tr("Export Index", "menu"), [=]() { this->tabAction(TAB_ATS::ExportHTML_index); });
	gmenu[GUI_CXE::ToolsExportHTML_services] = menuBarAction(mexporthtml, tr("Export Services", "menu"), [=]() { this->tabAction(TAB_ATS::ExportHTML_services); });
	gmenu[GUI_CXE::ToolsExportHTML_bouquets] = menuBarAction(mexporthtml, tr("Export Bouquets", "menu"), [=]() { this->tabAction(TAB_ATS::ExportHTML_bouquets); });
	gmenu[GUI_CXE::ToolsExportHTML_userbouquets] = menuBarAction(mexporthtml, tr("Export Userbouquets", "menu"), [=]() { this->tabAction(TAB_ATS::ExportHTML_userbouquets); });
	gmenu[GUI_CXE::ToolsExportHTML_tunersets] = menuBarAction(mexporthtml, tr("Export Tuner settings", "menu"), [=]() { this->tabAction(TAB_ATS::ExportHTML_tunersets); });
	// menuBarSeparator(mtools);
	// gmenu[GUI_CXE::ToolsServicesOrder] = menuBarAction(mtools, "Order Services A-Z", []() {});
	// gmenu[GUI_CXE::ToolsBouquetsOrder] = menuBarAction(mtools, "Order Userbouquets A-Z", []() {});
	// gmenu[GUI_CXE::ToolsServicesCache] = menuBarAction(mtools, "Remove cached data from Services", []() {});
	// gmenu[GUI_CXE::ToolsBouquetsDelete] = menuBarAction(mtools, "Delete all Bouquets", []() {});
	menuBarSeparator(mtools);
	gmenu[GUI_CXE::ToolsInspector] = menuBarAction(mtools, tr("Log Inspector", "menu"), [=]() { this->tabAction(TAB_ATS::Inspector); }, Qt::CTRL | Qt::ALT | Qt::Key_J);

	//: Platform: Window menu
	QMenu* mwind = menuBarMenu(menubar, tr("&Window", "menu"));
#ifndef Q_OS_WASM
	gmenu[GUI_CXE::WindowMinimize] = menuBarAction(mwind, tr("&Minimize", "menu"), [=]() { this->windowMinimize(); }, Qt::CTRL | Qt::Key_M);
	menuBarSeparator(mwind);
#endif
	gmenu[GUI_CXE::StatusBar] = menuBarAction(mwind, tr("Hide &Status Bar", "menu"), [=]() { this->statusBarToggle(); }, Qt::CTRL | Qt::ALT | Qt::Key_B);
	menuBarSeparator(mwind);
	gmenu[GUI_CXE::NewTab] = menuBarAction(mwind, tr("New &Tab", "menu"), [=]() { this->newTab(); }, Qt::CTRL | Qt::Key_T);
	menuBarSeparator(mwind);
	QActionGroup* mwtabs = menuBarActionGroup(mwind, true);

	//: Platform: Help menu
	QMenu* mhelp = menuBarMenu(menubar, tr("&Help", "menu"));
	menuBarAction(mhelp, tr("Quick start", "menu"), [=]() { this->linkToOnlineHelp(1); });
	menuBarAction(mhelp, tr("Online help", "menu"), [=]() { this->linkToOnlineHelp(0); });
	menuBarAction(mhelp, tr("Troubleshooting", "menu"), [=]() { this->linkToOnlineHelp(2); });
	menuBarSeparator(mhelp);
	menuBarAction(mhelp, tr("Software licenses…", "menu"), [=]() { this->linkToWebsite(3); });
	menuBarAction(mhelp, tr("About &Qt", "menu"), [=]() { this->mroot->aboutQt(); })->setMenuRole(QAction::NoRole);
	menuBarSeparator(mhelp);
	menuBarAction(mhelp, tr("Report a bug", "menu"), [=]() { this->linkToRepository(2); });
	menuBarAction(mhelp, tr("Help with translations", "menu"), [=]() { this->linkToWebsite(2); });
	menuBarAction(mhelp, tr("Go to e2SE repository…", "menu"), [=]() { this->linkToRepository(0); });
	menuBarAction(mhelp, tr("Donate", "menu"), [=]() { this->linkToWebsite(1); });
	menuBarSeparator(mhelp);
	menuBarAction(mhelp, tr("Check for updates…", "menu"), [=]() { this->checkUpdate(); });
	menuBarAction(mhelp, tr("&About e2 SAT Editor", "menu"), [=]() { this->aboutDialog(); })->setMenuRole(QAction::NoRole);

	this->menu = menu;
	this->mwind = mwind;
	this->mwtabs = mwtabs;
}

//TODO QTabBar::tab elide and icon size [Windows]
//TODO font-size height hidpi
//TODO tab-bar left border 1px gap
void gui::tabStackerLayout()
{
	debug("tabStackerLayout");

	this->twid = new QTabWidget;
	twid->setObjectName("tabwidget");
	twid->setTabsClosable(true);
	twid->setMovable(true);
	twid->setUsesScrollButtons(true);
	twid->setStyle(new TabBarProxyStyle);
	twid->tabBar()->setObjectName("tabwidget_tabbar");
	twid->tabBar()->setChangeCurrentOnDrag(false);
	twid->tabBar()->setElideMode(Qt::ElideRight);

#ifndef Q_OS_WIN
	twid->setStyleSheet("#tabwidget::tab-bar { left: 0; border-style: solid } #tabwidget::pane { border: 0; border-radius: 0 } #tabwidget_tabbar::tab { min-width: 12ex; max-width: 25ex; height: 44px; padding-left: 8px; padding-right: 8px; font-size: 13px; border-style: solid; border-width: 0 1px; color:palette(button-text); background: palette(button) } #tabwidget_tabbar::tab:selected { color: palette(highlighted-text); background: palette(highlight); border-color: transparent }");
#else
	twid->setStyleSheet("#tabwidget::tab-bar { left: 0; border-style: solid } #tabwidget::pane { border: 0; border-radius: 0 } #tabwidget_tabbar::tab { min-width: 25ex; height: 22px; padding-top: 11px; padding-bottom: 11px; padding-left: 8px; padding-right: 8px; font-size: 13px; border-style: solid; border-width: 0 1px; color:palette(button-text); background: palette(button) } #tabwidget_tabbar::tab:selected { color: palette(highlighted-text); background: palette(highlight); border-color: transparent }");
#endif

	QColor twtbshade;
	QString twtbshade_hexArgb;
#ifndef Q_OS_MAC
	twtbshade = QPalette().color(QPalette::Shadow).lighter();
	twtbshade.setAlphaF(0.25);
	twtbshade_hexArgb = twtbshade.name(QColor::HexArgb);

	theme->dynamicStyleSheet(twid, "#tabwidget_tabbar, #tabwidget_tabbar::tab { border-color: " + twtbshade_hexArgb + " }", theme::light);

	twtbshade = QPalette().color(QPalette::Shadow).darker();
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

	QWidget* ttcornerwid = new QWidget;
	QHBoxLayout* ttcornerlayout = new QHBoxLayout(ttcornerwid);

	QPushButton* ttbnew = new QPushButton;
	ttbnew->setText(tr("New &Tab", "tab"));
	ttbnew->setIconSize(QSize(12, 12));
	ttbnew->setIcon(theme->dynamicIcon("add", ttbnew));
	ttbnew->setShortcut(QKeySequence::AddTab);

	int ttbspacer = 1;
#if defined Q_OS_WIN
	ttbnew->setStyleSheet("QPushButton { min-width: 8ex; height: 22px; margin: 5px 4px 3px 4px; padding-top: 4px; padding-bottom: 4px; padding-left: 3px; padding-right: 3px; font-size: 12px; font-weight: bold }");
#elif defined Q_OS_MAC
	if (theme::isDefault())
	{
		ttbspacer = 5;
		ttbnew->setStyleSheet("QPushButton { min-width: 8ex; height: 44px; padding-left: 3px; padding-right: 3px; font-size: 12px; font-weight: bold }");
	}
	else
	{
		ttbnew->setStyleSheet("QPushButton { min-width: 8ex; height: 32px; margin: 4px; padding-top: 4px; padding-bottom: 4px; padding-left: 3px; padding-right: 3px; font-size: 12px; font-weight: bold }");
	}
#else
	ttbnew->setStyleSheet("QPushButton { min-width: 8ex; height: 32px; margin: 5px 4px 3px 4px; padding-top: 4px; padding-bottom: 4px; padding-left: 3px; padding-right: 3px; font-size: 12px; font-weight: bold }");
#endif

	ttbnew->connect(ttbnew, &QPushButton::pressed, [=]() { this->newTab(); });

	ttcornerlayout->addWidget(ttbnew);
	ttcornerlayout->addItem(new QSpacerItem(ttbspacer, 0));
	ttcornerlayout->setContentsMargins(0, 0, 0, 0);
	ttcornerlayout->setSpacing(0);

	platform::osWidgetBlend(ttcornerwid);

	twid->setCornerWidget(ttcornerwid, Qt::TopLeftCorner);

	launcher();

	mcnt->addWidget(twid);
}

void gui::statusBarLayout()
{
	debug("statusBarLayout");

	this->sbwid = new QStatusBar;
	this->sbwidl = new QLabel;
	this->sbwidc = new QWidget;
	this->sbwidr = new QLabel;
	this->sbwidi = new QPushButton;

	sbwidl->setStyleSheet("padding: 0 2ex");
	sbwidr->setStyleSheet("padding: 0 1ex");

#ifdef Q_OS_MAC
	theme->dynamicStyleSheet(sbwid, "QStatusBar { background: transparent }");
#endif

	//TODO QStatusBar layout expanding
	sbwid->layout()->setSpacing(0);

	sbwidl->setAlignment(Qt::AlignVCenter);
	sbwidr->setAlignment(Qt::AlignVCenter);
	sbwidl->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Ignored);
	sbwidc->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Ignored);
	sbwidr->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Ignored);
	sbwidi->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Ignored);

	sbwidi->setFlat(true);
	sbwidi->setIcon(theme->dynamicIcon("file-info", sbwidi));
	sbwidi->connect(sbwidi, &QPushButton::pressed, [=]() {
		this->fileInfo();

		QMouseEvent mouseRelease(QEvent::MouseButtonRelease, sbwidi->pos(), sbwidi->mapToGlobal(QPoint(0, 0)), Qt::LeftButton, Qt::MouseButtons(Qt::LeftButton), {});
		QCoreApplication::sendEvent(sbwidi, &mouseRelease);
	});

	sbwid->addWidget(sbwidl, 0);
	sbwid->addWidget(sbwidc, 1);
	sbwid->addWidget(sbwidr, 0);
	sbwid->addPermanentWidget(sbwidi, 0);

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

void gui::clipboardDataChanged()
{
	debug("clipboardDataChanged");

	for (auto & x : ttabs)
	{
		tab* tab = x.second;
		tab->clipboardDataChanged();
	}
}

void gui::initSettings()
{
	debug("initSettings");

	QSettings settings;

	settings.setValue("settings/version", 1);
	settings.setValue("settings/reset", false);

	settings.setValue("application/version", mroot->applicationVersion());
#ifdef E2SE_DEMO
	settings.setValue("application/debug", true);
	settings.setValue("application/seeds", ":/e2se-seeds/enigma_db");
#else
	settings.setValue("application/debug", false);
#endif
#ifndef Q_OS_MAC
	settings.setValue("application/fixUnicodeChars", false);
#else
	settings.setValue("application/fixUnicodeChars", true);
#endif
	settings.setValue("application/piconsBrowsePath", "");

	settings.beginGroup("preference");
	settings.setValue("autoCheckUpdate", false);
	settings.setValue("askConfirmation", false);
	settings.setValue("language", "");
	settings.setValue("theme", "");
	settings.setValue("osExperiment", true);
	settings.setValue("treeCurrentAfterDrop", true);
	settings.setValue("treeDropCopy", true);
	settings.setValue("treeDropMove", false);
	settings.setValue("piconsBackup", true);
	settings.setValue("piconsUseRefid", true);
	settings.setValue("piconsUseChname", false);
	settings.endGroup();

	settings.beginGroup("engine");
	settings.setValue("dbTypeDefault", 0x1224);
	settings.setValue("parserPriorLamedb5", false);
	settings.setValue("parserTunerset", true);
	settings.setValue("makerTunerset", true);
	settings.setValue("parserParentalLock", true);
	settings.setValue("makerParentalLock", true);
	settings.setValue("userbouquetFilenameSuffix", "dbe");
	settings.setValue("markerGlobalIndex", false);
	settings.setValue("favouriteMatchService", true);
	settings.setValue("mergeSortId", false);
	settings.setValue("parentalLockInvert", false);
	settings.setValue("toolsCsvHeader", true);
	settings.setValue("toolsCsvDelimiter", "\\n");
	settings.setValue("toolsCsvSeparator", ",");
	settings.setValue("toolsCsvEscape", "\"");
	settings.setValue("toolsFieldsDefault", true);
	settings.setValue("toolsFieldsExtended", false);
	settings.endGroup();

	settings.beginWriteArray("profile");
	settings.setArrayIndex(0);
	settings.setValue("profileName", "Default");
	settings.setValue("ipAddress", "192.168.0.2");
	settings.setValue("ftpPort", 21);
	settings.setValue("ftpActive", false);
	settings.setValue("httpPort", 80);
	settings.setValue("telnetPort", 23);
	settings.setValue("username", "root");
	settings.setValue("password", "");
	settings.setValue("pathTransponders", "/etc/tuxbox");
	settings.setValue("pathServices", "/etc/enigma2");
	settings.setValue("pathBouquets", "/etc/enigma2");
	settings.setValue("pathPicons", "/usr/share/enigma2/picon");
	settings.setValue("customWebifReloadUrl", "");
	settings.setValue("customTelnetReloadCmd", ""); 
	settings.endArray();
	settings.setValue("profile/selected", 0);

	settings.beginWriteArray("ftpcom");
	settings.setValue("debug", false);
	settings.setValue("ftpConnectTimeout", 10);
	settings.setValue("httpTimeout", 15);
	settings.setValue("telnetTimeout", 15);
	settings.setValue("maxResumeAttempts", 5);
	settings.endArray();
}

void gui::updateSettings()
{
	debug("updateSettings");

	QSettings settings;
	float version = settings.value("application/version", 0).toFloat();

	if (! settings.contains("settings/version"))
	{
		settings.setValue("settings/version", 1);
		settings.setValue("settings/reset", false);

		settings.setValue("application/version", mroot->applicationVersion());
	}
	else if (version != mroot->applicationVersion().toFloat())
	{
		settings.setValue("application/version", mroot->applicationVersion());
		settings.setValue("settings/version", 1);

		if (version < 1.3)
		{
			settings.setValue("preference/autoCheckUpdate", false);
			settings.setValue("engine/userbouquetFilenameSuffix", "dbe");
			settings.setValue("engine/markerGlobalIndex", false);
			settings.setValue("engine/favouriteMatchService", true);
			settings.setValue("engine/mergeSortId", false);
			settings.setValue("engine/parentalLockInvert", settings.value("preference/parentalLockInvert", false).toBool());
			settings.remove("preference/parentalLockInvert");

			if (settings.value("engine/toolsCsvDelimiter").toString().contains(QRegularExpression("[\n|\r|\t| ]")))
			{
				QString text = settings.value("engine/toolsCsvDelimiter").toString();
				text = text.replace("\n", "\\n").replace("\r", "\\r").replace("\t", "\\t").replace(" ", "\\s");
				settings.setValue("engine/toolsCsvDelimiter", text);
			}
			if (settings.value("engine/toolsCsvSeparator").toString().contains(QRegularExpression("[\n|\r|\t| ]")))
			{
				QString text = settings.value("engine/toolsCsvSeparator").toString();
				text = text.replace("\n", "\\n").replace("\r", "\\r").replace("\t", "\\t").replace(" ", "\\s");
				settings.setValue("engine/toolsCsvSeparator", text);
			}
			if (settings.value("engine/toolsCsvEscape").toString().contains(QRegularExpression("[\n|\r|\t| ]")))
			{
				QString text = settings.value("engine/toolsCsvEscape").toString();
				text = text.remove(QRegularExpression("[\n|\r|\t| ]"));
				settings.setValue("engine/toolsCsvEscape", text);
			}

			if (settings.value("ftpcom/httpTimeout").toInt() == 60)
			{
				settings.setValue("ftpcom/httpTimeout", 15);
			}

			int size = settings.beginReadArray("profile");
			for (int i = 0; i < size; i++)
			{
				settings.setArrayIndex(i);
				settings.setValue("telnetPort", 23);
			}
			settings.endArray();
		}
		if (version < 1.0)
		{
			settings.setValue("ftpcom/debug", false);
			settings.setValue("ftpcom/ftpConnectTimeout", 10);
			settings.setValue("ftpcom/httpTimeout", 60);
			settings.setValue("ftpcom/maxResumeAttempts", 5);
		}
		if (version < 0.8)
		{
			int size = settings.beginReadArray("profile");
			for (int i = 0; i < size; i++)
			{
				settings.setArrayIndex(i);
				QByteArray ba (settings.value("password").toString().toUtf8());
				settings.setValue("password", ba.toBase64());
			}
			settings.endArray();

			settings.setValue("application/piconsBrowsePath", "");
			settings.setValue("preference/piconsBackup", true);
			settings.setValue("preference/piconsUseRefid", true);
			settings.setValue("preference/piconsUseChname", false);
		}
	}
}

void gui::resetSettings()
{
	debug("resetSettings");

	QSettings().clear();

	initSettings();
}

int gui::newTab(string path, bool launch)
{
	tab* ttab = new tab(this, mwid);
	int ttid = ttab->getTabId();

	debug("newTab", "ttid", ttid);

	ttab->viewMain();

	bool read = ! path.empty();
	QString ttname = tr("Untitled", "tab");
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
		if (ttab->readFile(path))
		{
			string filename = std::filesystem::path(path).filename().u8string();
			ttname = QString::fromStdString(filename);
		}
		else
		{
			error("newTab", tr("Error", "error").toStdString(), tr("Error reading file \"%1\".", "error").arg(path.data()).toStdString());

			if (launch)
			{
				ttname.append(QString::fromStdString(count ? " " + to_string(count) : ""));
			}
			else
			{
				twid->removeTab(index);
				delete ttmenu[ttid];
				delete ttabs[ttid];
				ttmenu.erase(ttid);
				ttabs.erase(ttid);

				return -1;
			}
		}
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

int gui::openTab(TAB_VIEW ttv)
{
	return openTab(ttv, 0);
}

int gui::openTab(TAB_VIEW ttv, int arg)
{
	tab* current = getCurrentTabHandler();
	tab* parent = current;

	if (current == nullptr)
	{
		error("openTab", tr("Error", "error").toStdString(), tr("Missing current tab reference.", "error").toStdString());

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

	QIcon tticon;
	QString ttname = QString::fromStdString(parent->getTabName());

	switch (ttv)
	{
		case TAB_VIEW::main:
			error("openTab", tr("Error", "error").toStdString(), tr("Missing parent tab for tab reference \"%1\".", "error").arg(ttid).toStdString());

			delete ttab;

			return -1;
		break;
		case TAB_VIEW::transponders:
			ttab->viewTransponders(parent);
			tticon = QIcon(theme::icon("transponders-view", theme::icon_highlight));
			ttname = QString("%1 - %2").arg(ttname).arg(tr("Edit Transponders", "tab"));
		break;
		case TAB_VIEW::tunersets:
			ttab->viewTunersets(parent, arg);
			tticon = QIcon(theme::icon("tunersets-view", theme::icon_highlight));
			ttname = QString("%1 - %2").arg(ttname).arg(tr("Edit Settings", "tab"));
		break;
		case TAB_VIEW::picons:
			ttab->viewPicons(parent);
			tticon = QIcon(theme::icon("picons-view", theme::icon_highlight));
			ttname = QString("%1 - %2").arg(ttname).arg(tr("Picons editor", "tab"));
		break;
		case TAB_VIEW::channelBook:
			ttab->viewChannelBook(parent);
			tticon = QIcon(theme::icon("channelbook-view", theme::icon_highlight));
			ttname = QString("%1 - %2").arg(ttname).arg(tr("Channel Book", "tab"));
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

bool gui::closeTab(int index)
{
	// debug("closeTab", "index", index);

	int ttid = getTabId(index);

	if (! ttabs.count(ttid))
	{
		error("closeTab", tr("Error", "error").toStdString(), tr("Missing tab reference \"%1\".", "error").arg(ttid).toStdString());

		return true;
	}

	tab* ttab = ttabs[ttid];

	debug("closeTab", "ttid", ttid);

	if (! ttab->isChild() && ttab->hasChanged())
	{
		twid->setCurrentIndex(index);

		int closing = closeQuestion();

		if (closing == QMessageBox::Save)
			ttab->saveFile(false);

		if (closing == QMessageBox::Cancel)
			return false;
	}

	mwind->removeAction(ttmenu[ttid]);
	mwtabs->removeAction(ttmenu[ttid]);
	twid->removeTab(index);
	ttmenu.erase(ttid);

	if (ttab != nullptr)
	{
		if (ttab->isChild())
		{
			for (auto & x : ttabs)
			{
				tab* tab = x.second;

				if (tab != nullptr && tab->hasChildren())
				{
					for (auto & child : tab->children())
					{
						if (child == ttab)
						{
							tab->removeChild(ttab);
							break;
						}
					}
				}
			}
		}
		if (ttab->hasChildren())
		{
			for (auto & child : ttab->children())
			{
				int index = twid->indexOf(child->widget);

				ttab->removeChild(child);

				closeTab(index);
			}
		}
	}
	delete ttabs[ttid];
	ttabs.erase(ttid);

	if (twid->count() == 0)
		launcher();

	return true;
}

void gui::closeAllTabs()
{
	debug("closeAllTabs");

	vector<QWidget*> i_wids;

	for (auto & x : ttabs)
	{
		tab* tab = x.second;

		if (! tab->isChild())
		{
			i_wids.emplace_back(tab->widget);
		}
	}

	for (auto & wid : i_wids)
	{
		int index = twid->indexOf(wid);

		if (! closeTab(index))
			return;
	}
}

//TODO TEST multiple tabs
void gui::windowChanged()
{
	// debug("windowChanged");

#ifndef Q_OS_WASM
	// main window busy
	if (mroot->activeWindow() || mroot->activeModalWidget() || mroot->activePopupWidget())
	{
		debug("windowChanged", "mwind", "busy");
		this->gxe = this->gex;
		updateMenu();
	}
	// main window idle
	else
	{
		debug("windowChanged", "mwind", "idle");
		this->gex = this->gxe;
		setFlags(GUI_CXE::idle);
	}
#else
	this->gxe = this->gex;
	updateMenu();
#endif

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
		ttab->tabSwitch();
		TAB_VIEW ttv = ttab->getTabView();
		tabViewChanged(ttv);
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

void gui::changeTabName(int ttid, string path)
{
	debug("changeTabName", "ttid", ttid);

	if (! ttabs.count(ttid))
	{
		return error("changeTabName", tr("Error", "error").toStdString(), tr("Missing tab reference \"%1\".", "error").arg(ttid).toStdString());
	}

	tab* ttab = ttabs[ttid];
	int index = twid->indexOf(ttab->widget);
	int count = index;
	int ttv = ttab->getTabView();

	QString ttname = tr("Untitled", "tab");

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
						changeTabName(ttid, path);
						break;
					}
				}
			}
		}
	}
	if (path.empty())
	{
		ttname.append(QString::fromStdString(count ? " " + to_string(count) : ""));
	}
	else
	{
		ttname = QString::fromStdString(path);
	}

	// debug("changeTabName", "index", index);

	switch (ttv)
	{
		case TAB_VIEW::transponders:
			ttname = QString("%1 - %2").arg(ttname).arg(tr("Edit Transponders", "tab"));
		break;
		case TAB_VIEW::tunersets:
			ttname = QString("%1 - %2").arg(ttname).arg(tr("Edit Settings", "tab"));
		break;
		case TAB_VIEW::picons:
			ttname = QString("%1 - %2").arg(ttname).arg(tr("Picons editor", "tab"));
		break;
		case TAB_VIEW::channelBook:
			ttname = QString("%1 - %2").arg(ttname).arg(tr("Channel Book", "tab"));
		break;
	}

	twid->setTabText(index, ttname);
	twid->setTabToolTip(index, ttname);
	ttmenu[ttid]->setText(ttname);
	ttab->setTabName(ttname.toStdString());
}

void gui::tabViewChanged(TAB_VIEW ttv)
{
	tabViewChanged(ttv, 0);
}

void gui::tabViewChanged(TAB_VIEW ttv, int arg)
{
	debug("tabViewChanged", "ttv", ttv);

	switch (ttv)
	{
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
		default:
			gmenu[GUI_CXE::TabListFind]->setText(tr("&Find Channel"));
			gmenu[GUI_CXE::TabTreeFind]->setText(tr("Find &Bouquet"));
			gmenu[GUI_CXE::TabTreeFindNext]->setText(tr("Find N&ext Bouquet"));
	}
}

string gui::openFileDialog()
{
	debug("openFileDialog");

	QString caption = tr("Select data folder", "file-dialog");

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

string gui::saveFileDialog(string path)
{
	debug("saveFileDialog", "path", path);

	QString caption = tr("Select where to save", "file-dialog");

	string nw_path;

	QFileDialog fdial = QFileDialog(mwid, caption, QString::fromStdString(path));
	/*
	 * qt-base/src/plugins/platformthemes/gtk3/qgtk3dialoghelpers.cpp
	 * Gtk.FileChooserAction.GTK_FILE_CHOOSER_ACTION_CREATE_FOLDER
	 */
	// fdial.setAcceptMode(QFileDialog::AcceptOpen);
	fdial.setFileMode(QFileDialog::Directory);
	fdial.setFilter(QDir::AllDirs | QDir::NoSymLinks);
	fdial.setLabelText(QFileDialog::Accept, QFileDialog::tr("&Save"));
	if (fdial.exec() == QDialog::Accepted)
	{
		QUrl url = fdial.selectedUrls().value(0);
		if (url.isLocalFile() || url.isEmpty())
			url = url.toLocalFile();
		nw_path = url.toString().toStdString();
	}
	return nw_path;
}

vector<string> gui::importFileDialog(GUI_DPORTS gde)
{
	debug("importFileDialog");

	QString caption = tr("Select one or more files to open", "file-dialog");
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
			opts.append("Neutrino Services (services.xml)");
			opts.append("Parental lock blacklist (blacklist)");
			opts.append("Parental lock whitelist (whitelist)");
			opts.append("Parental lock services (locked)");
		break;
		case GUI_DPORTS::Bouquets:
			opts.append("Bouquet (bouquets.*)");
			opts.append("Bouquet epl (*.epl)");
			opts.append("Neutrino Bouquets (ubouquets.xml)");
			opts.append("Neutrino api-1 Bouquets (bouquets.xml)");
		break;
		case GUI_DPORTS::Userbouquets:
			opts.append("Userbouquet (userbouquet.*)");
			opts.append("Neutrino Bouquets (ubouquets.xml)");
			opts.append("Neutrino api-1 Bouquets (bouquets.xml)");
		break;
		case GUI_DPORTS::Tunersets:
			opts.append("Tuner settings (*.xml)");
		break;
		case GUI_DPORTS::CSV:
			opts.append(QString("%1 (*.csv *.txt)").arg(tr("CSV File", "file-dialog")));
			opts.append(QString("%1 (*)").arg(tr("All Files", "file-dialog")));
		break;
		case GUI_DPORTS::M3U:
			opts.append(QString("%1 (*.m3u *.m3u8)").arg(tr("M3U File", "file-dialog")));
			opts.append(QString("%1 (*)").arg(tr("All Files", "file-dialog")));
		break;
		case GUI_DPORTS::ConnectionProfile:
			opts.append(QString("%1 (*.profile)").arg(tr("Connection profile File", "file-dialog")));
			opts.append(QString("%1 (*)").arg(tr("All Files", "file-dialog")));
		break;
		default:
			fmode = QFileDialog::AnyFile;
			opts.append("Enigma2 folder (*)");
			opts.append("Neutrino folder (*)");
			opts.append("Lamedb 2.4 (lamedb)");
			opts.append("Lamedb 2.5 (lamedb5)");
			opts.append("Lamedb 2.3 (services)");
			opts.append("Lamedb 2.2 (services)");
			opts.append("Bouquet (bouquets.*)");
			opts.append("Bouquet epl (*.epl)");
			opts.append("Userbouquet (userbouquet.*)");
			opts.append("Tuner settings (*.xml)");
			opts.append("Neutrino Services (services.xml)");
			opts.append("Neutrino Bouquets (ubouquets.xml)");
			opts.append("Neutrino api-1 Bouquets (bouquets.xml)");
			opts.append("Parental lock blacklist (blacklist)");
			opts.append("Parental lock whitelist (whitelist)");
			opts.append("Parental lock services (services.locked)");
			opts.append(QString("%1 (*)").arg(tr("All Files", "file-dialog")));
	}

	vector<string> paths;

	QFileDialog fdial = QFileDialog(mwid, caption);
	fdial.setAcceptMode(QFileDialog::AcceptOpen);
	fdial.setFileMode(fmode);
	fdial.setNameFilters(opts);
	fdial.setLabelText(QFileDialog::Accept, tr("Import", "file-dialog"));
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

string gui::exportFileDialog(GUI_DPORTS gde, string path, int& bit)
{
	debug("exportFileDialog", "path", path);

	QString caption = tr("Select where to save", "file-dialog");
	QStringList opts;

	switch (gde)
	{
		case GUI_DPORTS::Services:
			opts.append("Lamedb default (*)");
			opts.append("Lamedb 2.4 (lamedb)");
			opts.append("Lamedb 2.5 (lamedb5)");
			opts.append("Lamedb 2.3 (services)");
			opts.append("Lamedb 2.2 (services)");
			opts.append("Neutrino default (services.xml)");
			opts.append("Neutrino api-4 (services.xml)");
			opts.append("Neutrino api-3 (services.xml)");
			opts.append("Neutrino api-2 (services.xml)");
			opts.append("Neutrino api-1 (services.xml)");
			opts.append("Parental lock blacklist (blacklist)");
			opts.append("Parental lock whitelist (whitelist)");
			opts.append("Parental lock services (services.locked)");
		break;
		case GUI_DPORTS::Bouquets:
			opts.append("Bouquet (bouquets.*)");
			opts.append("Bouquet epl (*.epl)");
			opts.append("Neutrino api-4 Bouquets (ubouquets.xml)");
			opts.append("Neutrino api-3 Bouquets (ubouquets.xml)");
			opts.append("Neutrino api-2 Bouquets (ubouquets.xml)");
			opts.append("Neutrino api-1 Bouquets (bouquets.xml)");
		break;
		case GUI_DPORTS::Userbouquets:
			opts.append("Userbouquet (userbouquet.*)");
			opts.append("Neutrino api-4 Bouquets (ubouquets.xml)");
			opts.append("Neutrino api-3 Bouquets (ubouquets.xml)");
			opts.append("Neutrino api-2 Bouquets (ubouquets.xml)");
			opts.append("Neutrino api-1 Bouquets (bouquets.xml)");
		break;
		case GUI_DPORTS::Tunersets:
			opts.append("Tuner settings (*.xml)");
		break;
		case GUI_DPORTS::CSV:
			opts.append(QString("%1 (*.csv *.txt)").arg(tr("CSV File", "file-dialog")));
			opts.append(QString("%1 (*)").arg(tr("All Files", "file-dialog")));
		break;
		case GUI_DPORTS::M3U:
			opts.append(QString("%1 (*.m3u *.m3u8)").arg(tr("M3U File", "file-dialog")));
			opts.append(QString("%1 (*)").arg(tr("All Files", "file-dialog")));
		break;
		case GUI_DPORTS::HTML:
			opts.append(QString("%1 (*.html)").arg(tr("HTML File", "file-dialog")));
			opts.append(QString("%1 (*)").arg(tr("All Files", "file-dialog")));
		break;
		case GUI_DPORTS::ConnectionProfile:
			opts.append(QString("%1 (*.profile)").arg(tr("Connection profile File", "file-dialog")));
			opts.append(QString("%1 (*)").arg(tr("All Files", "file-dialog")));
		break;
		default:
			opts.append(QString("%1 (*)").arg(tr("All Files", "file-dialog")));
	}

	string nw_path;

	QString selected;
	QFileDialog fdial = QFileDialog(mwid, caption, QString::fromStdString(path));
	fdial.setAcceptMode(QFileDialog::AcceptSave);
	fdial.setFilter(QDir::AllDirs | QDir::NoSymLinks);
	fdial.setNameFilters(opts);
	fdial.setLabelText(QFileDialog::Accept, tr("Export", "file-dialog"));
	if (fdial.exec() == QDialog::Accepted)
	{
		selected = fdial.selectedNameFilter();

		// straight copy of e2db_abstract::FPORTS
		if (selected == "Lamedb 2.4 (lamedb)")
			bit = 0x1224;
		else if (selected == "Lamedb 2.5 (lamedb5)")
			bit = 0x1225;
		else if (selected == "Lamedb 2.3 (services)")
			bit = 0x1223;
		else if (selected == "Lamedb 2.2 (services)")
			bit = 0x1222;
		else if (selected == "Neutrino default (services.xml)")
			bit = 0x1010;
		else if (selected == "Neutrino api-4 (services.xml)")
			bit = 0x1014;
		else if (selected == "Neutrino api-3 (services.xml)")
			bit = 0x1013;
		else if (selected == "Neutrino api-2 (services.xml)")
			bit = 0x1012;
		else if (selected == "Neutrino api-1 (services.xml)")
			bit = 0x1011;
		else if (selected == "Bouquet epl (*.epl)")
			bit = 0x0020;
		else if (selected == "Neutrino api-4 Bouquets (ubouquets.xml)")
			bit = 0x4014;
		else if (selected == "Neutrino api-3 Bouquets (ubouquets.xml)")
			bit = 0x4013;
		else if (selected == "Neutrino api-2 Bouquets (ubouquets.xml)")
			bit = 0x4012;
		else if (selected == "Neutrino api-1 Bouquets (bouquets.xml)")
			bit = 0x4011;
		else if (selected == "Parental lock blacklist (blacklist)")
			bit = 0xfa;
		else if (selected == "Parental lock whitelist (whitelist)")
			bit = 0xfe;
		else if (selected == "Parental lock services (services.locked)")
			bit = 0xff;

		debug("exportFileDialog", "bit", bit);

		QUrl url = fdial.selectedUrls().value(0);
		if (url.isLocalFile() || url.isEmpty())
			url = url.toLocalFile();
		nw_path = url.toString().toStdString();
	}
	return nw_path;
}

string gui::exportFileDialog(GUI_DPORTS gde, string path)
{
	int bit = 0;
	return exportFileDialog(gde, path, bit);
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
	QStringList content;

	if (msg.info && ! msg.message.empty())
	{
		QString text = QString::fromStdString(msg.message);
		sbwid->showMessage(text);
	}
	else if (msg.view == TAB_VIEW::main)
	{
		if (msg.update)
		{
			if (msg.counters[COUNTER::n_bouquet])
				content.append(QString(QApplication::layoutDirection() == Qt::RightToLeft ? "%2 :%1" : "%1: %2").arg(tr("Channels", "status-bar")).arg(msg.counters[COUNTER::n_bouquet]));
			if (! msg.curr.empty())
				content.append(QString(QApplication::layoutDirection() == Qt::RightToLeft ? "%2 :%1" : "%1: %2").arg(tr("Bouquet", "status-bar")).arg(msg.curr.data()));

			if (QApplication::layoutDirection() == Qt::RightToLeft)
				std::reverse(content.begin(), content.end());
			sbwidl->setText(content.join(separator));
		}
		else
		{
			content.append(QString(QApplication::layoutDirection() == Qt::RightToLeft ? "%2 :%1" : "%1: %2").arg(tr("TV", "status-bar")).arg(msg.counters[COUNTER::n_tv]));
			content.append(QString(QApplication::layoutDirection() == Qt::RightToLeft ? "%2 :%1" : "%1: %2").arg(tr("Radio", "status-bar")).arg(msg.counters[COUNTER::n_radio]));
			content.append(QString(QApplication::layoutDirection() == Qt::RightToLeft ? "%2 :%1" : "%1: %2").arg(tr("Data", "status-bar")).arg(msg.counters[COUNTER::n_data]));
			content.append(QString(QApplication::layoutDirection() == Qt::RightToLeft ? "%2 :%1" : "%1: %2").arg(tr("Total", "status-bar")).arg(msg.counters[COUNTER::n_services]));

			if (QApplication::layoutDirection() == Qt::RightToLeft)
				std::reverse(content.begin(), content.end());
			sbwidr->setText(content.join(separator));

			QString text = getFileFormatName(msg.version);
			if (msg.convert != 0)
			{
				QString convert = getFileFormatName(msg.convert);
				text = QString(QApplication::layoutDirection() == Qt::RightToLeft ? "%2 < %1" : "%1 > %2").arg(text).arg(convert);
			}
			sbwidi->setToolTip(text);
		}
	}
	else if (msg.view == TAB_VIEW::transponders)
	{
		content.append(QString(QApplication::layoutDirection() == Qt::RightToLeft ? "%2 :%1" : "%1: %2").arg(tr("Total", "status-bar")).arg(msg.counters[COUNTER::n_transponders]));

		sbwidl->setText("");
		sbwidr->setText(content.join(separator));
	}
	else if (msg.view == TAB_VIEW::tunersets)
	{
		if (msg.update)
		{
			if (msg.counters[COUNTER::n_position])
				content.append(QString(QApplication::layoutDirection() == Qt::RightToLeft ? "%2 :%1" : "%1: %2").arg(tr("Transponders", "status-bar")).arg(msg.counters[COUNTER::n_position]));
			if (! msg.curr.empty())
				content.append(QString(QApplication::layoutDirection() == Qt::RightToLeft ? "%2 :%1" : "%1: %2").arg(tr("Position", "status-bar")).arg(msg.curr.data()));

			if (QApplication::layoutDirection() == Qt::RightToLeft)
				std::reverse(content.begin(), content.end());
			sbwidl->setText(content.join(separator));
		}
		else
		{
			content.append(QString(QApplication::layoutDirection() == Qt::RightToLeft ? "%2 :%1" : "%1: %2").arg(tr("Total", "status-bar")).arg(msg.counters[COUNTER::n_transponders]));

			sbwidr->setText(content.join(separator));
		}
	}
	else if (msg.view == TAB_VIEW::picons)
	{
		if (msg.update)
		{
			if (! msg.curr.empty())
				content.append(QString(QApplication::layoutDirection() == Qt::RightToLeft ? "%2 :%1" : "%1: %2").arg(tr("Filename", "status-bar")).arg(msg.curr.data()));

			if (QApplication::layoutDirection() == Qt::RightToLeft)
				std::reverse(content.begin(), content.end());
			sbwidl->setText(content.join(separator));
		}
		else
		{
			sbwidl->setText("");
			sbwidr->setText("");
		}
	}
}

void gui::resetStatusBar(bool message)
{
	debug("resetStatusBar");

	if (message)
	{
		sbwid->clearMessage();
		sbwid->addWidget(sbwidl, 0);
		sbwid->addWidget(sbwidc, 1);
		sbwid->addWidget(sbwidr, 0);
	}
	else
	{
		sbwidl->setText("");
		sbwidr->setText("");
	}
}

void gui::fileOpen()
{
	debug("fileOpen");

#ifdef E2SE_DEMO
	newTab();

	return;
#endif

#ifndef Q_OS_WASM
	string path = openFileDialog();

	if (! path.empty())
		newTab(path);
#else
	auto fileContentReady = [=](const QString& path, const QByteArray& filedata)
	{
		if (! path.isEmpty())
		{
			gui::gui_file file;
			file.data = filedata.toStdString();
			file.filename = path.toStdString();
			file.size = filedata.size();

			this->blobs.emplace_back(file);

			newTab(path.toStdString());
		}
	};

	QFileDialog::getOpenFileContent("All Files (*.*)", fileContentReady);
#endif
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

void gui::fileInfo()
{
	debug("fileInfo");

	tab* ttab = getCurrentTabHandler();
	if (ttab != nullptr)
		ttab->infoFile();
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

void gui::editAction(GUI_CXE bit)
{
	debug("editAction", "bit", bit);

	QWidget* w = QApplication::focusWidget();

	if (QLineEdit* input = qobject_cast<QLineEdit*>(w))
	{
		switch (bit)
		{
			case GUI_CXE::EditUndo: input->undo(); break;
			case GUI_CXE::EditRedo: input->redo(); break;
			// note: QWidgetLineControl::q_delete_selected
			case GUI_CXE::EditDelete: input->del(); break;
			case GUI_CXE::EditSelectAll: input->selectAll(); break;
			case GUI_CXE::EditCut: input->cut(); break;
			case GUI_CXE::EditCopy: input->copy(); break;
			case GUI_CXE::EditPaste: input->paste(); break;
			default: return;
		}
	}
	else if (QTextEdit* input = qobject_cast<QTextEdit*>(w))
	{
		switch (bit)
		{
			case GUI_CXE::EditUndo: input->undo(); break;
			case GUI_CXE::EditRedo: input->redo(); break;
			// note: QWidgetTextControl::q_delete_selected
			case GUI_CXE::EditDelete: input->clear(); break;
			case GUI_CXE::EditSelectAll: input->selectAll(); break;
			case GUI_CXE::EditCut: input->cut(); break;
			case GUI_CXE::EditCopy: input->copy(); break;
			case GUI_CXE::EditPaste: input->paste(); break;
			default: return;
		}
	}
	else
	{
		GUI_CXE act;

		switch (bit)
		{
			case GUI_CXE::EditDelete: act = GUI_CXE::TabListDelete; break;
			case GUI_CXE::EditSelectAll: act = GUI_CXE::TabListSelectAll; break;
			case GUI_CXE::EditCut: act = GUI_CXE::TabListCut; break;
			case GUI_CXE::EditCopy: act = GUI_CXE::TabListCopy; break;
			case GUI_CXE::EditPaste: act = GUI_CXE::TabListPaste; break;
			default: return;
		}

		tabAction(TAB_ATS (act));
	}
}

void gui::tabAction(TAB_ATS bit)
{
	debug("tabAction", "bit", bit);

	tab* ttab = getCurrentTabHandler();
	if (ttab != nullptr)
		ttab->actionCall(bit);
}

void gui::windowMinimize()
{
	debug("windowMinimize");

	if (! mwid->isMinimized())
		mwid->showMinimized();
}

void gui::settingsDialog()
{
	debug("settingsDialog");

	auto* dialog = new e2se_gui::settings(this);
	dialog->display(mwid);
	dialog->destroy();
}

void gui::aboutDialog()
{
	debug("aboutDialog");

	auto* dialog = new e2se_gui::about();
	dialog->display();
}

void gui::linkToRepository(int page)
{
	debug("linkToRepository", "page", page);

	QString url = "https://github.com/ctlcltd/e2-sat-editor";

	switch (page)
	{
		case 1: url.append("/releases"); break;
		case 2: url.append("/issues/new/choose"); break;
	}

	QDesktopServices::openUrl(QUrl(url));
}

void gui::linkToWebsite(int page)
{
	debug("linkToWebsite", "page", page);

	QString url = "https://e2sateditor.com";

	switch (page)
	{
		case 1: url.append("/donate.html"); break;
		case 2: url.append("/translate/"); break;
		case 3: url.append("/licenses.html"); break;
	}

	QDesktopServices::openUrl(QUrl(url));
}

void gui::linkToOnlineHelp(int page)
{
	debug("linkToOnlineHelp", "page", page);

	QString url = "https://e2sateditor.com/help/";

	switch (page)
	{
		case 1: url.append("quick-start.html"); break;
		case 2: url.append("troubleshooting.html"); break;
	}

	QDesktopServices::openUrl(QUrl(url));
}

void gui::checkUpdate()
{
	debug("checkUpdate");

#ifndef E2SE_CHECKUPDATE
	return this->linkToRepository(1);
#endif

	if (statusBarIsVisible())
	{
		QString message = tr("Checking for updates…", "message");

		gui::status msg;
		msg.info = true;
		msg.message = message.toStdString();
		setStatusBar(msg);

		QTimer* timer = new QTimer(this->mwid);
		timer->setSingleShot(true);
		timer->setInterval(tab::STATUSBAR_MESSAGE_TIMEOUT);
		timer->callOnTimeout([=]() { this->resetStatusBar(true); timer->stop(); });
		timer->start();
	}

#ifdef E2SE_CHECKUPDATE
	QThread* thread = QThread::create([=]() {
		auto* dialog = new e2se_gui::checkUpdate(this->mwid);
		dialog->check();
		dialog->destroy();
	});
	thread->start();
	thread->quit();
#endif
}

#ifdef E2SE_CHECKUPDATE
void gui::autoCheckUpdate()
{
	debug("autoCheckUpdate", "deferred", "10 s");

	QTimer* timer = new QTimer(this->mwid);
	timer->setSingleShot(true);
	timer->setInterval(1e4);
	timer->callOnTimeout([=]() {
		debug("autoCheckUpdate", "timer", 1);

		QThread* thread = QThread::create([=]() {
			auto* dialog = new e2se_gui::checkUpdate(this->mwid);
			dialog->autoCheck();
			dialog->destroy();
		});
		thread->start();
		thread->quit();

		timer->stop();
	});
	timer->start();
}
#endif

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

//TODO lowered window when opening bad file [macOS]
void gui::launcher()
{
	debug("launcher");

	setFlags(GUI_CXE::init);

	string path = this->ifpath;

	if (! path.empty())
	{
		if (
			std::filesystem::exists(path) &&
			! ((std::filesystem::status(path).permissions() & std::filesystem::perms::owner_read) == std::filesystem::perms::none) &&
			! ((std::filesystem::status(path).permissions() & std::filesystem::perms::group_read) == std::filesystem::perms::none)
		)
		{
			newTab(path, true);
		}
		else
		{
			error("launcher", tr("Error", "error").toStdString(), tr("Error reading file \"%1\".", "error").arg(path.data()).toStdString());

			errorMessage(tr("File Error", "error"), tr("Error opening files.", "error"));

			newTab();
		}
	}
	else
	{
		newTab();
	}

	tabChanged(0);
}

bool gui::getFlag(GUI_CXE bit)
{
	// debug("getFlag");

	return this->gxe[bit];
}

void gui::setFlag(GUI_CXE bit, bool flag)
{
	// debug("setFlag");

	typedef size_t position_t;

	this->gxe.set(position_t (bit), flag);
	setTabEditActionFlag(bit, flag);

	this->gex = this->gxe;

	QAction* action = gmenu.count(bit) ? gmenu[bit] : nullptr;

	if (action != nullptr)
		action->setEnabled(flag);
}

bitset<256> gui::getFlags()
{
	// debug("getFlags");

	return this->gxe;
}

void gui::setFlags(bitset<256> bits)
{
	// debug("setFlags", "set", 1);

	this->gxe = bits;

	updateMenu();
}

void gui::setFlags(vector<int> bits)
{
	// debug("setFlags", "set", 0);

	typedef size_t position_t;

	for (int & bit : bits)
	{
		this->gxe.set(position_t (bit), true);
		setTabEditActionFlag(GUI_CXE (bit), true);
	}

	updateMenu();
}

void gui::setFlags(vector<int> bits, bool flag)
{
	// debug("setFlags", "set", 0);

	typedef size_t position_t;

	for (int & bit : bits)
	{
		this->gxe.set(position_t (bit), flag);
		setTabEditActionFlag(GUI_CXE (bit), flag);
	}

	updateMenu();
}

void gui::setFlags(int preset)
{
	// debug("setFlags", "preset", preset);

	this->gxe.reset();

	if (preset == 0)
		setFlags(GUI_CXE__init);
	else if (preset == -1)
		setFlags(GUI_CXE__idle);

	// note: is out of range
	// debug("setFlags", "flags", getFlags().to_ullong());
}

int gui::getTabEditActionFlag(GUI_CXE bit)
{
	// debug("getTabEditActionFlag");

	GUI_CXE act;

	switch (bit)
	{
		case GUI_CXE::EditDelete: act = GUI_CXE::TabListDelete; break;
		case GUI_CXE::EditSelectAll: act = GUI_CXE::TabListSelectAll; break;
		case GUI_CXE::EditCut: act = GUI_CXE::TabListCut; break;
		case GUI_CXE::EditCopy: act = GUI_CXE::TabListCopy; break;
		case GUI_CXE::EditPaste: act = GUI_CXE::TabListPaste; break;
		default: return 0;
	}

	return act;
}

void gui::setTabEditActionFlag(GUI_CXE bit, bool flag)
{
	// debug("getTabEditActionFlag");

	GUI_CXE act;

	switch (bit)
	{
		case GUI_CXE::TabListDelete: act = GUI_CXE::EditDelete; break;
		case GUI_CXE::TabListSelectAll: act = GUI_CXE::EditSelectAll; break;
		case GUI_CXE::TabListCut: act = GUI_CXE::EditCut; break;
		case GUI_CXE::TabListCopy: act = GUI_CXE::EditCopy; break;
		case GUI_CXE::TabListPaste: act = GUI_CXE::EditPaste; break;
		default: return;
	}

	setFlag(act, flag);
}

void gui::updateMenu()
{
	debug("updateMenu");

	for (auto & x : gmenu)
	{
		int bit = x.first;

		if (this->gxe[bit])
			x.second->setEnabled(true);
		else
			x.second->setDisabled(true);
	}

	// note: is out of range
	// debug("updateMenu", "flags", getActionFlags().to_ullong());
}

QLocale gui::getLocale()
{
	QString appLang = QSettings().value("preference/language").toString();
	return appLang.isEmpty() ? QLocale::system() : QLocale(appLang);
}

int gui::closeQuestion()
{
	QString title = tr("The file has been modified", "message");
	QString message = tr("Do you want to save your changes?", "message");

	title = title.toHtmlEscaped();
	message = message.replace("<", "&lt;").replace(">", "&gt;");

	QMessageBox msg = QMessageBox(this->mwid);

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

void gui::errorMessage(QString title, QString message)
{
	title = title.toHtmlEscaped();
	message = message.replace("<", "&lt;").replace(">", "&gt;");

#ifndef Q_OS_MAC
	QString text = message; 
#else
	QString text = QString("%1\n\n%2").arg(title).arg(message);
#endif

	QMessageBox::critical(this->mwid, title, text);
}

void gui::demoMessage()
{
	QString message = tr("DEMO MODE", "message");

	message = message.replace("<", "&lt;").replace(">", "&gt;");

	QMessageBox::information(this->mwid, NULL, message);
}

QString gui::getFileFormatName(int ver)
{
	switch (ver)
	{
		case 0x1224: return tr("Lamedb 2.4 [Enigma 2]");
		case 0x1225: return tr("Lamedb 2.5 [Enigma 2]");
		case 0x1223: return tr("Lamedb 2.3 [Enigma 1]");
		case 0x1222: return tr("Lamedb 2.2 [Enigma 1]");
		case 0x1014: return tr("Zapit api-v4 [Neutrino]");
		case 0x1013: return tr("Zapit api-v3 [Neutrino]");
		case 0x1012: return tr("Zapit api-v2 [Neutrino]");
		case 0x1011: return tr("Zapit api-v1 [Neutrino]");
	}

	return "";
}

QMenuBar* gui::menuBar(QLayout* layout)
{
	QMenuBar* menubar = new QMenuBar;
	menubar->setNativeMenuBar(true);
#ifdef Q_OS_WIN
	if (! theme::isOverridden() && theme::isFluentWin())
	{
		QStyle* style = QStyleFactory::create("windows11");
		menubar->setStyle(style);
	}
#endif
	layout->setMenuBar(menubar);
	return menubar;
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
