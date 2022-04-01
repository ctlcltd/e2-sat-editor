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

#include <Qt>
#include <QProxyStyle>
#include <QScreen>
#include <QSplitter>
#include <QGroupBox>
#include <QTabWidget>
#include <QPushButton>
#include <QFileDialog>
#include <QTimer>

#include "theme.h"
#include "gui.h"
#include "tab.h"
#include "settings.h"
#include "about.h"
#include "todo.h"

using std::to_string;
using namespace e2se;

namespace e2se_gui
{

class guiProxyStyle : public QProxyStyle
{
	public:
		int styleHint(StyleHint hint, const QStyleOption* option = 0, const QWidget* widget = 0, QStyleHintReturn* returnData = 0) const override
		{
			if (hint == QStyle::SH_TabBar_CloseButtonPosition)
				return QTabBar::RightSide;
			return QProxyStyle::styleHint(hint, option, widget, returnData);
		}
};

gui::gui(int argc, char* argv[])
{
	std::setlocale(LC_NUMERIC, "C");

	this->log = new logger("tab");
	debug("gui()");

	this->mroot = new QApplication(argc, argv);
	mroot->setOrganizationName("e2 SAT Editor Team");
	mroot->setOrganizationDomain("e2se.org");
	mroot->setApplicationName("e2-sat-editor");
	mroot->setApplicationVersion("0.1");
	mroot->setStyle(new guiProxyStyle);

	this->sets = new QSettings;
	if (! sets->contains("application/version"))
		setDefaultSets();

	QScreen* screen = mroot->primaryScreen();
	QSize wsize = screen->availableSize();

	this->mwid = new QWidget;
	mwid->setWindowTitle("e2 SAT Editor");
	mwid->setMinimumSize(760, 550);
	mwid->resize(wsize);

	// mroot->setLayoutDirection(Qt::RightToLeft);
	theme();

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

	mfrm->addLayout(mcnt, 1, 0);
	mfrm->addLayout(mstatusb, 2, 0);
}

void gui::menuCtl()
{
	debug("menuCtl()");

	QMenuBar* menu = new QMenuBar(nullptr);
	menu->setNativeMenuBar(true);

	if (! menu->isNativeMenuBar())
	{
		menu->setParent(mwid);
		mfrm->addWidget(menu);
	}

	QMenu* mfile = menu->addMenu(tr("&File"));
	mfile->addAction(tr("&New"), [=]() { this->newTab(""); })->setShortcut(QKeySequence::New);
	mfile->addAction(tr("&Open"), [=]() { this->open(); })->setShortcut(QKeySequence::Open);
	mfile->addAction(tr("&Save"), [=]() { this->save(); })->setShortcut(QKeySequence::Save);
	mfile->addSeparator();
	mfile->addAction("Import", todo);
	mfile->addAction("Export", todo);
	mfile->addSeparator();
	mfile->addAction("Close Tab", [=]() { this->closeTab(-1); });
	mfile->addAction("Close All Tabs", [=]() { this->closeAllTabs(); });
	mfile->addSeparator();
	mfile->addAction("Settings", [=]() { this->settings(); })->setShortcut(QKeySequence::Preferences);
	if (QSysInfo::productType().contains(QRegularExpression("macos|osx")))
		mfile->addAction(tr("&About"), [=]() { this->about(); });
	mfile->addSeparator();
	mfile->addAction(tr("&Exit"), [=]() { this->mroot->quit(); })->setShortcut((QSysInfo::productType().contains("windows") ? QKeySequence::Close : QKeySequence::Quit));

	QMenu* medit = menu->addMenu(tr("&Edit"));
	medit->addAction(tr("Cu&t"), [=]() { this->tabEditAction(TAB_EDIT_ATS::Cut); })->setShortcut(QKeySequence::Cut);
	medit->addAction(tr("&Copy"), [=]() { this->tabEditAction(TAB_EDIT_ATS::Copy); })->setShortcut(QKeySequence::Copy);
	medit->addAction(tr("&Paste"), [=]() { this->tabEditAction(TAB_EDIT_ATS::Paste); })->setShortcut(QKeySequence::Paste);
	medit->addSeparator();
	medit->addAction(tr("Select &All"), [=]() { this->tabEditAction(TAB_EDIT_ATS::SelectAll); })->setShortcut(QKeySequence::SelectAll);

	QMenu* mwind = menu->addMenu(tr("&Window"));
	mwind->addAction("&Minimize", [=]() { this->mwid->showMinimized(); })->setShortcut(Qt::CTRL | Qt::Key_M);
	mwind->addSeparator();
	QActionGroup* mwtabs = new QActionGroup(mwind);
	mwtabs->setExclusive(true);

	QMenu* mhelp = menu->addMenu(tr("&Help"));
	mhelp->addAction("TODO", todo); //TODO FIX macos QAction::NoRole ignored
	mhelp->addAction(tr("&About"), [=]() { this->about(); })->setMenuRole(QAction::NoRole);

	this->menu = menu;
	this->mwind = mwind;
	this->mwtabs = mwtabs;
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

void gui::tabCtl()
{
	debug("tabCtl()");

	ttidx = 0;

	this->twid = new QTabWidget(mwid);
	twid->setTabsClosable(true);
	twid->setMovable(true);
//	twid->setDocumentMode(true);
//	twid->setUsesScrollButtons(true);
//	twid->tabBar()->setDrawBase(false);

	//TODO FIX label text color in dark theme
	twid->setStyleSheet("QTabWidget::tab-bar { left: 0px } QTabWidget::pane { border: 0; border-radius: 0 } QTabBar::tab { height: 32px; padding: 5px; background: palette(mid); border: 1px solid transparent; border-radius: 0 } QTabBar::tab:selected { background: palette(highlight) } QTabWidget::tab QLabel { margin-left: 5px } QTabBar { background: red } QTabBar::close-button { margin: 0.4ex; image: url(" + theme::getIcon("close") + ") }");
	twid->connect(twid, &QTabWidget::currentChanged, [=](int index) { this->tabChanged(index); });
	twid->connect(twid, &QTabWidget::tabBarClicked, [=](int index) { this->tabClicked(index); });
	twid->connect(twid, &QTabWidget::tabCloseRequested, [=](int index) { this->closeTab(index); });
	twid->tabBar()->connect(twid->tabBar(), &QTabBar::tabMoved, [=](int from, int to) { this->tabMoved(from, to); });

	QPushButton* ttbnew = new QPushButton(theme::icon("add"), tr("New &Tab"));
	ttbnew->setIconSize(QSize(12, 12));
	ttbnew->setShortcut(QKeySequence::AddTab);
	ttbnew->setMinimumHeight(32);
	ttbnew->setStyleSheet("width: 8ex; height: 32px; font: bold 12px"); //TODO FIX height & ::left-corner padding
	ttbnew->connect(ttbnew, &QPushButton::pressed, [=]() { this->newTab(""); });
	twid->setCornerWidget(ttbnew, Qt::TopLeftCorner);

	newTab("");

	mcnt->addWidget(twid);
}

int gui::newTab(string filename = "")
{
	tab* ttab = new tab(this, mwid, filename);
	int ttid = ttidx++;
	ttab->widget->setProperty("ttid", QVariant (ttid));

	int ttcount = twid->count();
	string tname;

	if (filename.empty())
		tname = "Untitled" + (ttcount ? " " + to_string(ttcount++) : "");
	else
		tname = std::filesystem::path(filename).filename().u8string();

	QString ttname = QString::fromStdString(tname);

	int index = twid->addTab(ttab->widget, ttname);

	QTabBar* ttabbar = twid->tabBar();
	QLabel* ttlabel = new QLabel;
	ttlabel->setText(ttname);
	ttabbar->setTabButton(index, QTabBar::LeftSide, ttlabel);
	ttabbar->setTabText(index, "");

	ttab->setTabId(ttid);
	twid->setCurrentIndex(index);

	QAction* action = new QAction(ttname);
	action->connect(action, &QAction::triggered, [=]() { this->twid->setCurrentWidget(ttab->widget); });
	action->setCheckable(true);
	action->setActionGroup(mwtabs);
	mwind->addAction(action);
	ttmenu[ttid] = action;
	ttabs[ttid] = ttab;

	ttmenu[ttid]->setChecked(true);

	debug("newTab()", "ttid", to_string(ttid));

	return index;
}

void gui::closeTab(int index)
{
	debug("closeTab()", "index", to_string(index));

	//TODO ?
	// if (index == -1)
	//	index = twid->currentIndex();

	QWidget* curr_wid = twid->currentWidget();
	int ttid = curr_wid->property("ttid").toInt();
	//TODO FIX
	mwind->removeAction(ttmenu[ttid]);
	mwtabs->removeAction(ttmenu[ttid]);
	twid->removeTab(index);

	ttabs[ttid]->destroy();
	ttabs[ttid] = nullptr;
	delete ttabs[ttid];

	if (twid->count() == 0) newTab();
}

//TODO FIX SEGFAULT
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
		debug("tab.destroy()");
		ttabs[i]->destroy();
		ttabs[i] = nullptr;
		ttabs.erase(i);
	}

	if (twid->count() == 0) newTab();
}

void gui::tabChanged(int index)
{
	debug("tabChanged()", "index", to_string(index));
}

void gui::tabClicked(int index)
{
	debug("tabClicked()", "index", to_string(index));

	//TODO FIX
	// is prev. currentWidget on tabClicked
	QTimer::singleShot(200, [=]() {
		QWidget* curr_wid = twid->currentWidget();
		int ttid = curr_wid->property("ttid").toInt();
		ttmenu[ttid]->setChecked(true);
	});
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

void gui::open()
{
	debug("open()");

	string dirname = openFileDialog();

	if (! dirname.empty())
		newTab(dirname);
}

string gui::openFileDialog()
{
	debug("openFileDialog()");
	
	string dirname;

	//TODO ~ $HOME
	QString qdirname = QFileDialog::getExistingDirectory(nullptr, "Select enigma2 db folder", "~", QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
	dirname = qdirname.toStdString();

	return dirname;
}

//TODO native file dialog button "Open"
string gui::saveFileDialog(string filename)
{
	debug("saveFileDialog()", "filename", filename);

	string dirname;

	QFileDialog qfdial = QFileDialog(nullptr, "Select where save", QString::fromStdString(filename));
	qfdial.setFilter(QDir::AllDirs | QDir::NoSymLinks);
	qfdial.setAcceptMode(QFileDialog::AcceptSave);
	QString qdirname = qfdial.getExistingDirectory();
	dirname = qdirname.toStdString();

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

void gui::loaded(int counters[5])
{
	QString qstr;
	if (counters[4])
	{
		qstr = "Channels: " + QString::fromStdString(to_string(counters[4]));
		sbwidl->setText(qstr);
	}
	else
	{
		qstr.append("TV: " + QString::fromStdString(to_string(counters[1])) + "   ");
		qstr.append("Radio: " + QString::fromStdString(to_string(counters[2])) + "   ");
		qstr.append("Data: " + QString::fromStdString(to_string(counters[0])) + "   ");
		qstr.append("   ");
		qstr.append("Total: " + QString::fromStdString(to_string(counters[3])) + "   ");
		sbwidr->setText(qstr);
	}
}

void gui::reset()
{
	debug("reset()");

	sbwidl->setText("");
	sbwidr->setText("");
}

void gui::save()
{
	debug("save()");

	tab* ttab = getCurrentTabHandler();
	ttab->saveFile(true); //TODO temporarly set to save as
}

//TODO tab actions ctl
void gui::tabEditAction(TAB_EDIT_ATS action)
{
	debug("tabEditAction()", "action", to_string(action));

	tab* ttab = getCurrentTabHandler();
	ttab->listItemAction(action);
}

void gui::settings()
{
	new e2se_gui_dialog::settings(mwid);
}

void gui::about()
{
	new e2se_gui_dialog::about(mwid);
}

tab* gui::getCurrentTabHandler()
{
	QWidget* curr_wid = twid->currentWidget();
	int ttid = curr_wid->property("ttid").toInt();
	return ttabs[ttid];
}

void gui::setDefaultSets()
{
	debug("setDefaultSets()");

	sets->setValue("application/version", mroot->applicationVersion());

	sets->beginGroup("preference");
	sets->setValue("askConfirmation", true);
	sets->setValue("nonDestructiveEdit", true);
	sets->setValue("fixUnicodeChars", QSysInfo::productType() == "macos" ? true : false);
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
	sets->setValue("customFallbackReloadCmd", "");
	sets->endArray();
	sets->setValue("profile/selected", 1);
}

}
