/*!
 * e2-sat-editor/src/gui/gui.cpp
 *
 * @author Leonardo Laureti
 * @version 0.1
 * @license MIT License
 */

#include <iostream>
#include <filesystem>
#include <algorithm>
#include <string>
#include <vector>
#include <map>
#include <Qt>
#include <QApplication>
#include <QWidget>
#include <QProxyStyle>
#include <QScreen>
#include <QMenuBar>
#include <QStatusBar>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QSplitter>
#include <QGroupBox>
#include <QHeaderView>
#include <QTreeWidget>
#include <QTabWidget>
#include <QToolBar>
#include <QAction>
#include <QPushButton>
#include <QLabel>
#include <QFileDialog>
#include <QString>
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
	mfile->addAction("Save", todo);
	mfile->addSeparator();
	mfile->addAction("Import", todo);
	mfile->addAction("Export", todo);
	mfile->addSeparator();
	mfile->addAction("Close Tab", [=]() { this->closeTab(-1); });
	mfile->addAction("Close All Tabs", [=]() { this->closeAllTabs(); });
	mfile->addSeparator();
	mfile->addAction("Settings", [=]() { this->settings(); });
	mfile->addAction("About", [=]() { this->about(); });
	mfile->addSeparator();
	mfile->addAction("Quit", [=]() { this->mroot->quit(); });

	QMenu* medit = menu->addMenu("Edit");
	medit->addAction("TODO", todo);

	QMenu* mwind = menu->addMenu("Window");
	mwind->addAction("Minimize", [=]() { this->mwid->showMinimized(); });
	mwind->addSeparator();

	QMenu* mhelp = menu->addMenu("Help");
	mhelp->addAction("TODO", todo);
	mhelp->addAction("About", [=]() { this->about(); })->setMenuRole(QAction::NoRole);

	this->menu = menu;
	this->mwind = mwind;
}

void gui::statusCtl()
{
	debug("gui", "statusCtl()");

	this->sbwid = new QStatusBar(mwid);

	mstatusb->addWidget(sbwid);
}

//TODO FIX EXC_BAD_ACCESS
void gui::tabCtl()
{
	debug("gui", "tabCtl()");

	this->twid = new QTabWidget(mwid);
	twid->setTabsClosable(true);
	twid->setMovable(true);
	twid->setStyleSheet("QTabWidget::tab-bar { left: 0px } QTabWidget::pane { border: 0; border-radius: 0 } QTabBar::tab { height: 32px; padding: 5px; background: palette(mid); border: 1px solid transparent; border-radius: 0 } QTabBar::tab:selected { background: palette(highlight) } QTabWidget::tab QLabel { margin-left: 5px }");
	twid->connect(twid, &QTabWidget::currentChanged, [=](int index) { this->tabChanged(index); });
	twid->connect(twid, &QTabWidget::tabCloseRequested, [=] (int index) { this->closeTab(index); });

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

	ttab->setIndex(index);
	twid->setCurrentIndex(index);
//	mwind->addAction(tname, [=]() { this->twid->setCurrentIndex(index); });

	debug("gui", "newTab()", "index", to_string(index));

	return index;
}

void gui::closeTab(int index)
{
	debug("gui", "closeTab()", "index", to_string(index));

	if (index == -1)
		index = twid->currentIndex();

	//TODO destruct
	twid->removeTab(index);
	if (twid->count() == 0) newTab();
}

void gui::closeAllTabs()
{
	debug("gui", "closeAllTabs()");

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

//TODO FIX index changes on move
void gui::tabChangeName(int index, string filename)
{
	debug("gui", "tabChangeName()", "index", to_string(index));

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
}

void gui::save()
{
	debug("gui", "save()");
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
