/*!
 * e2-sat-editor/src/gui/channelBook.cpp
 *
 * @author Leonardo Laureti
 * @version 0.1
 * @license MIT License
 */

#include <QGridLayout>
#include <QTreeWidget>
#include <QLabel>

#include "../commons.h"
#include "channelBook.h"

using namespace std;

namespace e2se_gui
{
channelBook::channelBook()
{
	debug("channelBook");

	QGridLayout* afrm = new QGridLayout();

	side();
	stacked();

	afrm->addWidget(lwid, 0, 0);
	afrm->addWidget(swid, 0, 1);
	afrm->setColumnStretch(0, 2);
	afrm->setColumnStretch(1, 4);
	afrm->setSpacing(0);
	afrm->setContentsMargins(0, 0, 0, 0);

	this->widget = new QWidget;
	widget->setLayout(afrm);
}

//TODO sortable application state
void channelBook::side()
{
	debug("channelBook", "side()");

	this->lwid = new QListWidget;
	lwid->setStyleSheet("QListWidget { background: transparent; font: 15px } QListView::item { height: 36px }");

	lwid->addItem(" Services ");
	lwid->addItem(" Bouquets ");
	lwid->addItem(" Satellites ");
	lwid->addItem(" Providers ");
	lwid->addItem(" Resolutions ");
	lwid->addItem(" Encryptions ");
	lwid->addItem(" A-Z ");

	lwid->connect(lwid, &QListWidget::currentRowChanged, [=](int index) { this->sideRowChanged(index); });
}

void channelBook::stacked()
{
	debug("channelBook", "stacked()");

	this->swid = new QStackedWidget;

	listView(views::Services);
	treeView(views::Bouquets);
	treeView(views::Satellites);
	treeView(views::Providers);
	treeView(views::Resolutions);
	treeView(views::Encryptions);
	vtabView(views::A_Z);
}

void channelBook::listView(int vv, QLayout* ly, QTabWidget* tw, string tn)
{
	debug("channelBook", "listView()", "view", to_string(vv));

	QTreeWidget* tree = new QTreeWidget;
	tree->setUniformRowHeights(true);

	//Qt5
	QTreeWidgetItem* thead = new QTreeWidgetItem({"", "Index", "Name", "Type", "Provider"});
	tree->setHeaderItem(thead);
	tree->setColumnHidden(0, true);

	if (ly)
	{
		ly->addWidget(tree);
	}
	else if (tw)
	{
		int index = tw->addTab(tree, "");
		tw->tabBar()->setTabButton(index, QTabBar::LeftSide, new QLabel(QString::fromStdString(tn)));
	}
	else
	{
		swid->addWidget(tree);
	}
}

void channelBook::treeView(int vv)
{
	debug("channelBook", "treeView()", "view", to_string(vv));

	QWidget* widget = new QWidget;
	QHBoxLayout* layout = new QHBoxLayout;
	QTreeWidget* tree = new QTreeWidget;
	tree->setHeaderHidden(true);
	tree->setUniformRowHeights(true);

	for (int i=0; i < 3; i++)
	{
		QTreeWidgetItem* item = new QTreeWidgetItem({QString::fromStdString("Dummy " + to_string(i + 1))});
		tree->addTopLevelItem(item);
	}

	layout->setContentsMargins(0, 0, 0, 0);
	layout->setSpacing(0);
	layout->addWidget(tree);
	listView(vv, layout);
	widget->setLayout(layout);
	swid->addWidget(widget);
}

void channelBook::vtabView(int vv)
{
	debug("channelBook", "vtabView()", "view", to_string(vv));

	QTabWidget* tabv = new QTabWidget;
	tabv->setTabPosition(QTabWidget::West);
	tabv->setDocumentMode(true);
	tabv->setUsesScrollButtons(true);
	tabv->tabBar()->setExpanding(false);
	tabv->tabBar()->setDrawBase(true);
	tabv->setStyleSheet("QTabWidget::tab-bar { left: 0px } QTabWidget::pane { border: 0; border-radius: 0 } QTabBar::tab { margin-top: 0; width: 48px }");

	string chars[27] = {"0-9","A","B","C","D","E","F","G","H","I","J","L","K","M","N","O","P","Q","R","S","T","U","V","W","Z","Y","Z"};

	for (unsigned int i=0; i<27; i++)
		listView(vv, nullptr, tabv, chars[i]);

	swid->addWidget(tabv);
}

void channelBook::sideRowChanged(int index)
{
	debug("channelBook", "sideRowChanged", "index", to_string(index));
	swid->setCurrentIndex(index);
}

void channelBook::populate()
{
	debug("channelBook", "populate");
}
}
