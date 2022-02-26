/*!
 * e2-sat-editor/src/gui/gui.cpp
 *
 * @author Leonardo Laureti
 * @version 0.1
 * @license MIT License
 */

#include <Qt>
#include <QProxyStyle>
#include <QScreen>
#include <QSplitter>
#include <QGroupBox>
#include <QTabWidget>
#include <QPushButton>
#include <QLabel>
#include <QFileDialog>
#include <QTimer>

#include "../commons.h"
#include "gui.h"
#include "tab.h"
#include "settings.h"
#include "about.h"
#include "todo.h"

using namespace std;

namespace e2se_gui
{

class guiProxyStyle : public QProxyStyle
{
	public:
		int styleHint(StyleHint hint, const QStyleOption *option = 0, const QWidget *widget = 0, QStyleHintReturn *returnData = 0) const override
		{
			if (hint == QStyle::SH_TabBar_CloseButtonPosition)
				return QTabBar::RightSide;
			return QProxyStyle::styleHint(hint, option, widget, returnData);
		}
};

gui::gui(int argc, char* argv[])
{
	debug("gui", "qt6");

	this->mroot = new QApplication(argc, argv);
	mroot->setStyle(new guiProxyStyle);

	QScreen* screen = mroot->primaryScreen();
	QSize wsize = screen->availableSize();

	this->mwid = new QWidget;
	mwid->setWindowTitle("enigma2 channel editor");
	mwid->setMinimumSize(680, 510);
	mwid->resize(wsize);

	mroot->setStyleSheet("QGroupBox { spacing: 0; padding: 20px 0 0 0; border: 0 } QGroupBox::title { margin: 0 12px }");

	root();

	mwid->show();

	mroot->exec();
}

void gui::root()
{
	debug("gui", "root()");

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
	debug("gui", "menuCtl()");

	QMenuBar* menu = new QMenuBar(nullptr);
	menu->setNativeMenuBar(true);

	QString osprod = QSysInfo::productType();
	if (osprod != "macos" && osprod != "ubuntu")
	{
		menu->setParent(mwid);
		mfrm->addWidget(menu);
	}

	QMenu* mfile = menu->addMenu("File");
	mfile->addAction("New", [=]() { this->newTab(""); });
	mfile->addAction("Open", [=]() { this->open(); });
	mfile->addAction("Save", [=]() { this->save(); });
	mfile->addSeparator();
	mfile->addAction("Import", todo);
	mfile->addAction("Export", todo);
	mfile->addSeparator();
	mfile->addAction("Close Tab", [=]() { this->closeTab(-1); });
	mfile->addAction("Close All Tabs", [=]() { this->closeAllTabs(); });
	mfile->addSeparator();
	mfile->addAction("Settings", [=]() { this->settings(); });
	if (osprod == "macos")
		mfile->addAction("About", [=]() { this->about(); });
	mfile->addSeparator();
	mfile->addAction("Quit", [=]() { this->mroot->quit(); });

	QMenu* medit = menu->addMenu("Edit");
	medit->addAction("Cut", todo);
	medit->addAction("Copy", todo);
	medit->addAction("Paste", todo);
	medit->addSeparator();
	medit->addAction("Select All", todo);
	medit->addSeparator();
	medit->addAction("TODO", todo);

	QMenu* mwind = menu->addMenu("Window");
	mwind->addAction("Minimize", [=]() { this->mwid->showMinimized(); });
	mwind->addSeparator();
	QActionGroup* mwtabs = new QActionGroup(mwind);
	mwtabs->setExclusive(true);

	QMenu* mhelp = menu->addMenu("Help");
	mhelp->addAction("TODO", todo);
	mhelp->addAction("About", [=]() { this->about(); })->setMenuRole(QAction::NoRole);

	this->menu = menu;
	this->mwind = mwind;
	this->mwtabs = mwtabs;
}

void gui::statusCtl()
{
	debug("gui", "statusCtl()");

	this->sbwid = new QStatusBar(mwid);

	mstatusb->addWidget(sbwid);
}

void gui::tabCtl()
{
	debug("gui", "tabCtl()");

	ttidx = 0;

	this->twid = new QTabWidget(mwid);
	twid->setTabsClosable(true);
	twid->setMovable(true);
	twid->setStyleSheet("QTabWidget::tab-bar { left: 0px } QTabWidget::pane { border: 0; border-radius: 0 } QTabBar::tab { height: 32px; padding: 5px; background: palette(mid); border: 1px solid transparent; border-radius: 0 } QTabBar::tab:selected { background: palette(highlight) } QTabWidget::tab QLabel { margin-left: 5px }");
	twid->connect(twid, &QTabWidget::currentChanged, [=](int index) { this->tabChanged(index); });
	twid->connect(twid, &QTabWidget::tabBarClicked, [=](int index) { this->tabClicked(index); });
	twid->connect(twid, &QTabWidget::tabCloseRequested, [=](int index) { this->closeTab(index); });
	twid->tabBar()->connect(twid->tabBar(), &QTabBar::tabMoved, [=](int from, int to) { this->tabMoved(from, to); });

	QPushButton* ttbnew = new QPushButton();
	ttbnew->setText("+ New Tab");
//    ttbnew->setFlat(true);
	ttbnew->setStyleSheet("width: 8ex; height: 32px"); //TODO FIX height & ::left-corner padding
	ttbnew->setMinimumHeight(32);
	ttbnew->connect(ttbnew, &QPushButton::pressed, [=]() { this->newTab(""); });
	twid->setCornerWidget(ttbnew, Qt::TopLeftCorner);

	newTab("");

	mcnt->addWidget(twid);
}

int gui::newTab(string filename = "")
{
	tab* ttab = new tab(this, mwid, filename);
	int ttid = ttidx++;
	ttab->widget->setProperty("ttid", QVariant(ttid));

	int ttcount = twid->count();
	string tname;

	if (filename.empty())
		tname = "Untitled" + (ttcount ? " " + to_string(ttcount++) : "");
	else
		tname = filesystem::path(filename).filename();

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

	debug("gui", "newTab()", "ttid", to_string(ttid));

	return index;
}

void gui::closeTab(int index)
{
	debug("gui", "closeTab()", "index", to_string(index));

	if (index == -1)
		index = twid->currentIndex();

	QWidget* curr_wid = twid->currentWidget();
	int ttid = curr_wid->property("ttid").toInt();
	//TODO FIX
	mwind->removeAction(ttmenu[ttid]);
	mwtabs->removeAction(ttmenu[ttid]);
	//TODO destruct
	twid->removeTab(index);
	if (twid->count() == 0) newTab();
}

void gui::closeAllTabs()
{
	debug("gui", "closeAllTabs()");

	for (auto & action : mwtabs->actions())
	{
		mwind->removeAction(action);
		mwtabs->removeAction(action);
	}
	//TODO destruct
	twid->clear();
	if (twid->count() == 0) newTab();
}

void gui::tabChanged(int index)
{
	debug("gui", "tabChanged()", "index", to_string(index));

	QString msg = QString::fromStdString("Current tab index: " + to_string(index));
	sbwid->showMessage(msg);
}

void gui::tabClicked(int index)
{
	debug("gui", "tabClicked()", "index", to_string(index));

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
	debug("gui", "tabMoved()", "from|to", (to_string(from) + '|' + to_string(to)));

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
	debug("gui", "open()");

	string dirname = openFileDialog();

	if (! dirname.empty())
		newTab(dirname);
}

string gui::openFileDialog()
{
	debug("gui", "openFileDialog()");
	
	string dirname;

	//TODO ~ $HOME
	QString qdirname = QFileDialog::getExistingDirectory(nullptr, "Select enigma2 db folder", "~", QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
	dirname = qdirname.toStdString();

	return dirname;
}

void gui::tabChangeName(int ttid, string filename)
{
	debug("gui", "tabChangeName()", "ttid", to_string(ttid));

	tab* ttab = ttabs[ttid];
	int index = twid->indexOf(ttab->widget);
	string tname;

	if (filename.empty())
		tname = "Untitled" + (index ? " " + to_string(index) : "");
	else
		tname = filesystem::path(filename).filename();

	QString ttname = QString::fromStdString(tname);

	//TODO accessing nested QLabel
	QTabBar* ttabbar = twid->tabBar();
	QLabel* ttlabel = new QLabel;
	ttlabel->setText(ttname);
	ttabbar->setTabButton(index, QTabBar::LeftSide, ttlabel);

	ttmenu[ttid]->setText(ttname);
}

void gui::save()
{
	debug("gui", "save()");

	QWidget* curr_wid = twid->currentWidget();
	int ttid = curr_wid->property("ttid").toInt();
	ttabs[ttid]->save();
}

void gui::settings()
{
	settingsDialog(mwid);
}

void gui::about()
{
	aboutDialog(mwid);
}

}
