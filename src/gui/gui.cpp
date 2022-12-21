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

gui::gui(int argc, char* argv[], e2se::logger::session* log)
{
	std::setlocale(LC_NUMERIC, "C");

	this->log = new logger(log, "gui");
	debug("gui()");

	this->mroot = new QApplication(argc, argv);
	mroot->setOrganizationName("e2 SAT Editor Team");
	mroot->setOrganizationDomain("e2se.org");
	mroot->setApplicationName("e2-sat-editor");
	mroot->setApplicationVersion("0.1");
	mroot->setStyle(new TabBarProxyStyle);
	mroot->connect(mroot, &QApplication::focusChanged, [=]() { this->windowChanged(); });

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

	layout();

	mwid->show();

	mroot->exec();
}

void gui::layout()
{
	debug("layout()");

	this->mfrm = new QGridLayout(mwid);

	this->mcnt = new QHBoxLayout;
	this->mstatusb = new QHBoxLayout;

	menuLayout();
	statusLayout();
	tabLayout();

	mfrm->setContentsMargins(0, 0, 0, 0);
	mfrm->setSpacing(0);

	mfrm->addLayout(mcnt, 0, 0);
	mfrm->addLayout(mstatusb, 1, 0);
}

void gui::menuLayout()
{
	debug("menuLayout()");

	QMenuBar* menu = new QMenuBar;
	menu->setNativeMenuBar(true);
	mfrm->setMenuBar(menu);

	QMenu* mfile = menu->addMenu(tr("&File"));
	gmenu[GUI_CXE::FileNew] = mfile->addAction(tr("&New"), [=]() { this->newTab(""); }, QKeySequence::New);
	gmenu[GUI_CXE::FileOpen] = mfile->addAction(tr("&Open"), [=]() { this->fileOpen(); }, QKeySequence::Open);
	mfile->addSeparator();
	gmenu[GUI_CXE::FileSave] = mfile->addAction(tr("&Save"), [=]() { this->fileSave(); }, QKeySequence::Save);
	gmenu[GUI_CXE::FileSaveAs] = mfile->addAction(tr("Save &As…"), [=]() { this->fileSaveAs(); }, QKeySequence::SaveAs);
	mfile->addSeparator();
	gmenu[GUI_CXE::FileImport] = mfile->addAction("Import", [=]() { this->fileImport(); });
	gmenu[GUI_CXE::FileExport] = mfile->addAction("Export", [=]() { this->fileExport(); });
	mfile->addSeparator();
	gmenu[GUI_CXE::CloseTab] = mfile->addAction("Close Tab", [=]() { this->closeTab(-1); }, QKeySequence::Close);
	gmenu[GUI_CXE::CloseAllTabs] = mfile->addAction("Close All Tabs", [=]() { this->closeAllTabs(); }, Qt::CTRL | Qt::ALT | Qt::Key_W);
	mfile->addSeparator();
	gmenu[GUI_CXE::FilePrint] = mfile->addAction("&Print", [=]() { this->filePrint(); }, QKeySequence::Print);
	gmenu[GUI_CXE::FilePrintAll] = mfile->addAction("Print &All", [=]() { this->filePrintAll(); }, Qt::CTRL | Qt::SHIFT | Qt::Key_P);
	mfile->addSeparator();
	mfile->addAction("Settings", [=]() { this->settings(); }, QKeySequence::Preferences);
#ifdef Q_OS_MAC
		mfile->addAction(tr("&About"), [=]() { this->about(); });
#endif
	mfile->addSeparator();
	mfile->addAction(tr("E&xit"), [=]() { this->mroot->quit(); }, QKeySequence::Quit);
	
	QMenu* medit = menu->addMenu(tr("&Edit"));
	gmenu[GUI_CXE::TabListCut] = medit->addAction(tr("Cu&t"), [=]() { this->tabAction(TAB_ATS::ListCut); }, QKeySequence::Cut);
	gmenu[GUI_CXE::TabListCopy] = medit->addAction(tr("&Copy"), [=]() { this->tabAction(TAB_ATS::ListCopy); }, QKeySequence::Copy);
	gmenu[GUI_CXE::TabListPaste] = medit->addAction(tr("&Paste"), [=]() { this->tabAction(TAB_ATS::ListPaste); }, QKeySequence::Paste);
	QAction* medit_delete = gmenu[GUI_CXE::TabListDelete] = medit->addAction(tr("&Delete"), [=]() { this->tabAction(TAB_ATS::ListDelete); });
#ifdef Q_OS_MAC
	medit_delete->setShortcut(Qt::Key_Backspace);
#else
	medit_delete->setShortcut(QKeySequence::Delete);
#endif
	medit->addSeparator();
	gmenu[GUI_CXE::TabListSelectAll] = medit->addAction(tr("Select &All"), [=]() { this->tabAction(TAB_ATS::ListSelectAll); }, QKeySequence::SelectAll);

	QMenu* mfind = menu->addMenu(tr("&Find"));
	gmenu[GUI_CXE::TabListFind] = mfind->addAction(tr("&Find Channel…"), [=]() { this->tabAction(TAB_ATS::ListFind); }, QKeySequence::Find);
	gmenu[GUI_CXE::TabListFindNext] = mfind->addAction(tr("Find &Next"), [=]() { this->tabAction(TAB_ATS::ListFindNext); }, QKeySequence::FindNext);
	gmenu[GUI_CXE::TabListFindPrev] = mfind->addAction(tr("Find &Previous"), [=]() { this->tabAction(TAB_ATS::ListFindPrev); }, QKeySequence::FindPrevious);
	gmenu[GUI_CXE::TabListFindAll] = mfind->addAction(tr("Find &All"), [=]() { this->tabAction(TAB_ATS::ListFindAll); });
	mfind->addSeparator();
	gmenu[GUI_CXE::TabTreeFind] = mfind->addAction(tr("Find &Bouquet…"), [=]() { this->tabAction(TAB_ATS::TreeFind); }, Qt::CTRL | Qt::ALT | Qt::Key_F);
	gmenu[GUI_CXE::TabTreeFindNext] = mfind->addAction(tr("Find N&ext Bouquet"), [=]() { this->tabAction(TAB_ATS::TreeFindNext); }, Qt::CTRL | Qt::ALT | Qt::Key_E);

	QMenu* mtool = menu->addMenu(tr("&Tools"));
	gmenu[GUI_CXE::TunersetsSat] = mtool->addAction("Edit satellites.xml", [=]() { this->tabAction(TAB_ATS::EditTunersetsSat); });
	gmenu[GUI_CXE::TunersetsTerrestrial] = mtool->addAction("Edit terrestrial.xml", [=]() { this->tabAction(TAB_ATS::EditTunersetsTerrestrial); });
	gmenu[GUI_CXE::TunersetsCable] = mtool->addAction("Edit cables.xml", [=]() { this->tabAction(TAB_ATS::EditTunersetsCable); });
	gmenu[GUI_CXE::TunersetsAtsc] = mtool->addAction("Edit atsc.xml", [=]() { this->tabAction(TAB_ATS::EditTunersetsAtsc); });
	mtool->addSeparator();
	gmenu[GUI_CXE::OpenChannelBook] = mtool->addAction("Show Channel book", [=]() { this->tabAction(TAB_ATS::ShowChannelBook); });
	mtool->addSeparator();
	
	QMenu* mimportcsv = new QMenu(tr("Import from CSV"));
	gmenu[GUI_CXE::ToolsImportCSV_services] = mimportcsv->addAction("Import Services", [=]() { this->tabAction(TAB_ATS::ImportCSV_services); });
	gmenu[GUI_CXE::ToolsImportCSV_bouquet] = mimportcsv->addAction("Import Bouquet", [=]() { this->tabAction(TAB_ATS::ImportCSV_bouquet); });
	gmenu[GUI_CXE::ToolsImportCSV_userbouquet] = mimportcsv->addAction("Import Userbouquet", [=]() { this->tabAction(TAB_ATS::ImportCSV_userbouquet); });
	gmenu[GUI_CXE::ToolsImportCSV_tunersets] = mimportcsv->addAction("Import Tuner settings", [=]() { this->tabAction(TAB_ATS::ImportCSV_tunersets); });
	QMenu* mexportcsv = new QMenu(tr("Export to CSV"));
	gmenu[GUI_CXE::ToolsExportCSV_current] = mexportcsv->addAction("Export current", [=]() { this->tabAction(TAB_ATS::ExportCSV_current); });
	gmenu[GUI_CXE::ToolsExportCSV_all] = mexportcsv->addAction("Export All", [=]() { this->tabAction(TAB_ATS::ExportCSV_all); });
	gmenu[GUI_CXE::ToolsExportCSV_services] = mexportcsv->addAction("Export Services", [=]() { this->tabAction(TAB_ATS::ExportCSV_services); });
	gmenu[GUI_CXE::ToolsExportCSV_bouquets] = mexportcsv->addAction("Export Bouquets", [=]() { this->tabAction(TAB_ATS::ExportCSV_bouquets); });
	gmenu[GUI_CXE::ToolsExportCSV_userbouquets] = mexportcsv->addAction("Export Userbouquets", [=]() { this->tabAction(TAB_ATS::ExportCSV_userbouquets); });
	gmenu[GUI_CXE::ToolsExportCSV_tunersets] = mexportcsv->addAction("Export Tuner settings", [=]() { this->tabAction(TAB_ATS::ExportCSV_tunersets); });
	QMenu* mexporthtml = new QMenu(tr("Export to HTML"));
	gmenu[GUI_CXE::ToolsExportHTML_current] = mexporthtml->addAction("Export current", [=]() { this->tabAction(TAB_ATS::ExportHTML_current); });
	gmenu[GUI_CXE::ToolsExportHTML_all] = mexporthtml->addAction("Export All", [=]() { this->tabAction(TAB_ATS::ExportHTML_all); });
	gmenu[GUI_CXE::ToolsExportHTML_index] = mexporthtml->addAction("Export Index", [=]() { this->tabAction(TAB_ATS::ExportHTML_index); });
	gmenu[GUI_CXE::ToolsExportHTML_services] = mexporthtml->addAction("Export Services", [=]() { this->tabAction(TAB_ATS::ExportHTML_services); });
	gmenu[GUI_CXE::ToolsExportHTML_bouquets] = mexporthtml->addAction("Export Bouquets", [=]() { this->tabAction(TAB_ATS::ExportHTML_bouquets); });
	gmenu[GUI_CXE::ToolsExportHTML_userbouquets] = mexporthtml->addAction("Export Userbouquets", [=]() { this->tabAction(TAB_ATS::ExportHTML_userbouquets); });
	gmenu[GUI_CXE::ToolsExportHTML_tunersets] = mexporthtml->addAction("Export Tuner settings", [=]() { this->tabAction(TAB_ATS::ExportHTML_tunersets); });
	mtool->addMenu(mimportcsv);
	mtool->addMenu(mexportcsv);
	mtool->addMenu(mexporthtml);
	mtool->addSeparator();
	gmenu[GUI_CXE::ToolsServicesOrder] = mtool->addAction("Order Services A-Z", todo);
	gmenu[GUI_CXE::ToolsBouquetsOrder] = mtool->addAction("Order Userbouquets A-Z", todo);
	gmenu[GUI_CXE::ToolsServicesCache] = mtool->addAction("Remove cached data from Services", todo);
	gmenu[GUI_CXE::ToolsBouquetsDelete] = mtool->addAction("Delete all Bouquets", todo);
	mtool->addSeparator();
	gmenu[GUI_CXE::ToolsInspector] = mtool->addAction(tr("Inspector Log"), [=]() { this->tabAction(TAB_ATS::Inspector); }, Qt::CTRL | Qt::ALT | Qt::Key_J);

	QMenu* mwind = menu->addMenu(tr("&Window"));
	gmenu[GUI_CXE::WindowMinimize] = mwind->addAction("&Minimize", [=]() { this->windowMinimize(); }, Qt::CTRL | Qt::Key_M);
	mwind->addSeparator();
	gmenu[GUI_CXE::NewTab] = mwind->addAction("New &Tab", [=]() { this->newTab(); }, Qt::CTRL | Qt::Key_T);
	mwind->addSeparator();
	QActionGroup* mwtabs = new QActionGroup(mwind);
	mwtabs->setExclusive(true);

	QMenu* mhelp = menu->addMenu(tr("&Help"));
	mhelp->addAction(tr("About &Qt"), [=]() { mroot->aboutQt(); })->setMenuRole(QAction::NoRole);
	mhelp->addSeparator();
	mhelp->addAction(tr("&About e2 SAT Editor"), [=]() { this->about(); })->setMenuRole(QAction::NoRole);

	this->menu = menu;
	this->mwind = mwind;
	this->mwtabs = mwtabs;
}

void gui::tabLayout()
{
	debug("tabLayout()");

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

	twid->setStyleSheet("QTabWidget::tab-bar { left: 0px } QTabWidget::pane { border: 0; border-radius: 0 } QTabBar::tab { height: 32px; padding: 0.8ex 1ex; background: palette(mid); border: 1px solid transparent; border-radius: 0 } QTabBar::tab:selected { background: palette(highlight) } QTabBar::tab { padding-left: 1.2ex } QTabBar::close-button { margin: 0.5ex; image: url(" + ttclose_icon + ") } QTabBar::close-button:selected { image: url(" + ttclose_icon__selected + ") }");
	twid->connect(twid, &QTabWidget::currentChanged, [=](int index) { this->tabChanged(index); });
	twid->connect(twid, &QTabWidget::tabCloseRequested, [=](int index) { this->closeTab(index); });
	twid->tabBar()->connect(twid->tabBar(), &QTabBar::tabMoved, [=](int from, int to) { this->tabMoved(from, to); });

	QPushButton* ttbnew = new QPushButton(theme::icon("add"), tr("New &Tab"));
	ttbnew->setMinimumHeight(32);
	ttbnew->setIconSize(QSize(12, 12));
	ttbnew->setShortcut(QKeySequence::AddTab);
	//TODO FIX fixed height, ::left-corner needs padding
	ttbnew->setStyleSheet("width: 8ex; height: 32px; font: bold 12px");
	ttbnew->connect(ttbnew, &QPushButton::pressed, [=]() { this->newTab(); });
	twid->setCornerWidget(ttbnew, Qt::TopLeftCorner);

	launcher();

	mcnt->addWidget(twid);
}

//TODO FIX intl. rtl
void gui::statusLayout()
{
	debug("statusLayout()");

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

void gui::tabViewSwitch(TAB_VIEW ttv)
{
	tabViewSwitch(ttv, 0);
}

void gui::tabViewSwitch(TAB_VIEW ttv, int arg)
{
	debug("tabViewSwitch()", "ttv", ttv);

	switch (ttv)
	{
		case TAB_VIEW::main:
			gmenu[GUI_CXE::TabListFind]->setText("&Find Channel");
			gmenu[GUI_CXE::TabTreeFind]->setText("Find &Bouquet");
			gmenu[GUI_CXE::TabTreeFindNext]->setText("Find N&ext Bouquet");
		break;
		case TAB_VIEW::tunersets:
			gmenu[GUI_CXE::TabListFind]->setText("&Find Transponder");
			gmenu[GUI_CXE::TabTreeFind]->setText("Find &Position");
			gmenu[GUI_CXE::TabTreeFindNext]->setText("Find N&ext Position");
		break;
		case TAB_VIEW::channelBook:
			gmenu[GUI_CXE::TabListFind]->setText("&Find Channel");
			gmenu[GUI_CXE::TabTreeFind]->setText("Find &Bouquet");
			gmenu[GUI_CXE::TabTreeFindNext]->setText("Find N&ext Bouquet");
		break;
	}
}

int gui::newTab(string filename)
{
	this->tt++;
	int ttid = this->tt;
	
	debug("newTab()", "ttid", ttid);

	tab* ttab = new tab(this, mwid, this->log->log);

	if (! filename.empty() && ! ttab->readFile(filename))
	{
		error("newTab()", "ttid", ttid);
		return -1;
	}

	bool read = ! filename.empty();
	ttab->setTabId(ttid);
	ttab->viewMain();

	QString ttname = "Untitled";

	int index = twid->addTab(ttab->widget, ttname);
	int count = index;
	twid->tabBar()->setTabData(index, ttid);

	if (read)
	{
		filename = std::filesystem::path(filename).filename().u8string(); //C++17
		ttname = QString::fromStdString(filename);
	}
	else
	{
		ttname.append(QString::fromStdString(count ? " " + to_string(count) : ""));
	}
	twid->setTabText(index, ttname);

	QAction* action = new QAction(ttname);
	action->connect(action, &QAction::triggered, [=]() { this->twid->setCurrentWidget(ttab->widget); });
	action->setCheckable(true);
	action->setActionGroup(mwtabs);
	mwind->addAction(action);
	ttmenu[ttid] = action;
	ttabs[ttid] = ttab;

	ttab->setTabName(ttname.toStdString());
	twid->setCurrentIndex(index);

	return index;
}

int gui::openTab(TAB_VIEW view)
{
	return openTab(view, NULL);
}

int gui::openTab(TAB_VIEW view, int arg)
{
	this->tt++;
	int ttid = this->tt;
	
	debug("openTab()", "ttid", ttid);

	tab* parent = getCurrentTabHandler();
	int current = twid->tabBar()->currentIndex();
	string parent_ttname = parent->getTabName();

	tab* ttab = new tab(this, mwid, this->log->log);
	ttab->setTabId(ttid);

	QIcon tticon;
	QString ttname = QString::fromStdString(parent_ttname);

	switch (view)
	{
		case TAB_VIEW::main:
			error("openTab()", "ttid", ttid);
			return -1;
		break;
		case TAB_VIEW::tunersets:
			ttab->viewTunersets(parent, arg);
			tticon = QIcon(theme::icon("tunersets-view"));
			ttname.append(" - ");
			ttname.append("Edit settings");
		break;
		case TAB_VIEW::channelBook:
			ttab->viewChannelBook(parent);
			tticon = QIcon(theme::icon("channelbook-view"));
			ttname.append(" - ");
			ttname.append("Channel book");
		break;
	}

	current++;
	int index = twid->insertTab(current, ttab->widget, tticon, ttname);
	twid->tabBar()->setTabData(index, ttid);

	QAction* action = new QAction(ttname);
	action->connect(action, &QAction::triggered, [=]() { this->twid->setCurrentWidget(ttab->widget); });
	action->setCheckable(true);
	action->setActionGroup(mwtabs);
	mwind->addAction(action);
	ttmenu[ttid] = action;
	ttabs[ttid] = ttab;

	twid->setCurrentIndex(index);
	ttab->setTabName(ttname.toStdString());

	return index;
}

void gui::closeTab(int index)
{
	// debug("closeTab()", "index", index);

	int ttid = getTabId(index);
	
	debug("closeTab()", "ttid", index);

	mwind->removeAction(ttmenu[ttid]);
	mwtabs->removeAction(ttmenu[ttid]);
	twid->removeTab(index);
	ttmenu.erase(ttid);

	tab* ttab = ttabs[ttid];
	if (ttab != nullptr && ttab->hasChildren())
	{
		for (auto & child : ttab->children())
		{
			int index = twid->indexOf(child->widget);
			if (index == -1)
				continue;
			ttab->removeChild(child);
			closeTab(index);
		}
	}
	delete ttabs[ttid];
	ttabs.erase(ttid);

	if (twid->count() == 0)
		launcher();
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
		debug("closeAllTabs()", "destroy", int (i));

		delete ttabs[i];
		ttabs.erase(i);
		ttmenu.erase(i);
	}
	ttabs.clear();
	ttmenu.clear();

	launcher();
}

void gui::windowChanged()
{
	// debug("windowChanged()");

	// main window busy
	if (mroot->activeWindow())
	{
		debug("windowChanged()", "mwind", "busy");
		this->gxe = this->gex;
		update();
	}
	// main window idle
	else
	{
		debug("windowChanged()", "mwind", "idle");
		this->gex = this->gxe;
		update(GUI_CXE::idle);
	}
}

void gui::tabChanged(int index)
{
	// debug("tabChanged()", "index", index);

	int ttid = getTabId(index);

	debug("tabChanged()", "ttid", ttid);

	if (ttid != -1)
	{
		tab* ttab = ttabs[ttid];
		ttmenu[ttid]->setChecked(true);
		ttab->tabSwitched();
		TAB_VIEW ttv = ttab->getTabView();
		tabViewSwitch(ttv);
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

void gui::tabChangeName(int ttid, string filename)
{
	debug("tabChangeName()", "ttid", ttid);

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
	
	debug("tabChangeName()", "index", index);

	switch (v)
	{
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
	ttmenu[ttid]->setText(ttname);
	ttab->setTabName(ttname.toStdString());
}

string gui::openFileDialog()
{
	debug("openFileDialog()");

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
	debug("saveFileDialog()", "filename", filename);

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
	debug("importFileDialog()");

	QString caption = "Select one or more files to open";
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
			opts.append("CSV File (*.csv,txt)");
			opts.append("All Files (*)");
		break;
		case GUI_DPORTS::HTML:
			opts.append("HTML File (*.html,htm)");
			opts.append("All Files (*)");
		break;
		default:
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
	fdial.setFileMode(QFileDialog::ExistingFiles);
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
	debug("exportFileDialog()", "filename", filename);

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

		debug("exportFileDialog()", "bit", bit);

		QUrl url = fdial.selectedUrls().value(0);
		if (url.isLocalFile() || url.isEmpty())
			url = url.toLocalFile();
		path = url.toString().toStdString();
	}
	return path;
}

string gui::exportFileDialog(GUI_DPORTS gde, string filename)
{
	int bit;
	return exportFileDialog(gde, filename, bit);
}

void gui::setStatus(STATUS status)
{
	QString separator = "   ";
	QString text;

	if (status.view == TAB_VIEW::main)
	{
		if (status.current)
		{
			if (status.counters[COUNTER::bouquet])
			{
				text.append("Channels: " + QString::fromStdString(to_string(status.counters[COUNTER::bouquet])));
			}
			if (! status.bname.empty())
			{
				text.append(separator);
				text.append("Bouquet: " + QString::fromStdString(status.bname));
			}
			sbwidl->setText(text);
		}
		else
		{
			text.append("TV: " + QString::fromStdString(to_string(status.counters[COUNTER::tv])));
			text.append(separator);
			text.append("Radio: " + QString::fromStdString(to_string(status.counters[COUNTER::radio])));
			text.append(separator);
			text.append("Data: " + QString::fromStdString(to_string(status.counters[COUNTER::data])));
			text.append(separator);
			text.append("Total: " + QString::fromStdString(to_string(status.counters[COUNTER::services])));
			sbwidr->setText(text);
		}
	}
	else if (status.view == TAB_VIEW::tunersets)
	{
		if (status.current)
		{
			if (status.counters[COUNTER::position])
			{
				text.append("Transponders: " + QString::fromStdString(to_string(status.counters[COUNTER::position])));
			}
			if (! status.position.empty())
			{
				text.append(separator);
				text.append("Position: " + QString::fromStdString(status.position));
			}
			sbwidl->setText(text);
		}
		else
		{
			text.append("Total: " + QString::fromStdString(to_string(status.counters[COUNTER::transponders])));
			sbwidr->setText(text);
		}
	}
}

void gui::resetStatus()
{
	debug("resetStatus()");

	sbwidl->setText("");
	sbwidr->setText("");
}

void gui::fileOpen()
{
	debug("fileOpen()");

	string path = openFileDialog();

	if (! path.empty())
		newTab(path);
}

void gui::fileSave()
{
	debug("fileSave()");

	tab* ttab = getCurrentTabHandler();
	if (ttab != nullptr)
		ttab->saveFile(false);
}

void gui::fileSaveAs()
{
	debug("fileSaveAs()");

	tab* ttab = getCurrentTabHandler();
	if (ttab != nullptr)
		ttab->saveFile(true);
}

void gui::fileImport()
{
	debug("fileImport()");

	tab* ttab = getCurrentTabHandler();
	if (ttab != nullptr)
		ttab->importFile();
}

void gui::fileExport()
{
	debug("fileExport()");

	tab* ttab = getCurrentTabHandler();
	if (ttab != nullptr)
		ttab->exportFile();
}

void gui::filePrint()
{
	debug("filePrint()");

	tab* ttab = getCurrentTabHandler();
	if (ttab != nullptr)
		ttab->printFile(false);
}

void gui::filePrintAll()
{
	debug("filePrintAll()");

	tab* ttab = getCurrentTabHandler();
	if (ttab != nullptr)
		ttab->printFile(true);
}

void gui::tabAction(TAB_ATS action)
{
	debug("tabAction()", "action", action);

	tab* ttab = getCurrentTabHandler();
	if (ttab != nullptr)
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
	new e2se_gui_dialog::settings(mwid, this->log->log);
}

void gui::about()
{
	new e2se_gui_dialog::about(this->log->log);
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
	if (ttabs[ttid] == nullptr)
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
	debug("launcher()");

	this->tt = 0;
	update(GUI_CXE::init);
	newTab();
	tabChanged(0);
}

void gui::update()
{
	debug("update()");

	for (auto & x : gmenu)
	{
		if (this->gxe[x.first])
			x.second->setEnabled(true);
		else
			x.second->setDisabled(true);
	}

	// note: is out of range
	// debug("update()", "flags", getActionFlags().to_ullong());
}

void gui::update(int bit, bool flag)
{
	 // debug("update()", "overload", bit);

	typedef size_t position_t;
	QAction* action = gmenu.count(bit) ? gmenu[bit] : nullptr;

	if (action != nullptr)
		action->setEnabled(flag);

	this->gxe.set(position_t (bit), flag);

	this->gex = this->gxe;
}

void gui::update(vector<int> bits, bool flag)
{
	// debug("update()", "overload", 1);
	
	typedef size_t position_t;

	for (int & bit : bits)
		this->gxe.set(position_t (bit), flag);

	update();
}

void gui::update(vector<int> bits)
{
	// debug("update()", "overload", 0);
	
	typedef size_t position_t;

	for (int & bit : bits)
		this->gxe.set(position_t (bit), true);

	update();
}

void gui::update(int bit)
{
	// debug("update()", "overload", 0);

	this->gxe.reset();

	if (bit == 0)
		update(GUI_CXE__init);
	else if (bit == -1)
		update(GUI_CXE__idle);

	// note: is out of range
	// debug("update()", "flags", getFlags().to_ullong());
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
