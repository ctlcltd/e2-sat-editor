/*!
 * e2-sat-editor/src/gui/channelBook.cpp
 *
 * @author Leonardo Laureti
 * @version 0.1
 * @license MIT License
 */

#include <vector>
#include <map>
#include <cstdio>

#include <QGridLayout>
#include <QTreeWidget>
#include <QLabel>

#include "../commons.h"
#include "channelBook.h"

using namespace std;

namespace e2se_gui
{
channelBook::channelBook(e2db* dbih)
{
	debug("channelBook");

	//TODO FIX EXC_BAD_ACCESS
	// this->dbih = dbih;
	this->dbih = new e2db;

	QGridLayout* afrm = new QGridLayout();

	side();
	stacked();

	afrm->addWidget(lwid, 0, 0);
	afrm->addLayout(swid, 0, 1);
	afrm->setColumnMinimumWidth(0, 120);
	afrm->setColumnStretch(0, 1);
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
//	lwid->setSortingEnabled(true);
	lwid->setStyleSheet("QListWidget { background: transparent; font: 15px } QListView::item { height: 36px }");

	lwid->addItem(" Services ");
	lwid->addItem(" Bouquets ");
	lwid->addItem(" Satellites ");
	lwid->addItem(" Providers ");
	lwid->addItem(" Resolution ");
	lwid->addItem(" Encryption ");
	lwid->addItem(" A-Z ");

	lwid->connect(lwid, &QListWidget::currentRowChanged, [=](int index) { this->sideRowChanged(index); });
}

void channelBook::stacked()
{
	debug("channelBook", "stacked()");

	this->swid = new QHBoxLayout;
	swid->setContentsMargins(0, 0, 0, 0);
	swid->setSpacing(0);

	this->tabv = new QTabBar;
	tabv->setHidden(true);
	tabv->setShape(QTabBar::RoundedWest);
	tabv->setDocumentMode(true);
	tabv->setUsesScrollButtons(true);
	tabv->setExpanding(false);
	tabv->setDrawBase(true);
	tabv->setStyleSheet("QTabBar::tab { margin-top: 0; width: 48px }");

	string chars[27] = {"0-9","A","B","C","D","E","F","G","H","I","J","L","K","M","N","O","P","Q","R","S","T","U","V","W","Z","Y","Z"};

	for (unsigned int i=0; i < 27; i++)
	{
		tabv->addTab("");
		tabv->setTabButton(i, QTabBar::LeftSide, new QLabel(QString::fromStdString(chars[i])));
	}

	this->tree = new QTreeWidget;
	tree->setHidden(true);
	tree->setHeaderHidden(true);
	tree->setUniformRowHeights(true);

	for (int i=0; i < 3; i++)
	{
		QTreeWidgetItem* item = new QTreeWidgetItem({QString::fromStdString("Dummy " + to_string(i + 1))});
		tree->addTopLevelItem(item);
	}

	this->list = new QTreeWidget;
	list->setHidden(true);
	list->setUniformRowHeights(true);

	//Qt5
	QTreeWidgetItem* thead = new QTreeWidgetItem({"", "Index", "Name", "Type", "Provider"});
	list->setHeaderItem(thead);
	list->setColumnHidden(0, true);

	swid->addWidget(tabv);
	swid->addWidget(tree);
	swid->addWidget(list);
}

void channelBook::sideRowChanged(int index)
{
	debug("channelBook", "sideRowChanged", "index", to_string(index));

	switch (index)
	{
		case views::Services:
			tabv->setHidden(true);
			tree->setHidden(true);
			list->setVisible(true);
			flag = 2;
		break;
		case views::A_Z:
			tabv->setVisible(true);
			tree->setHidden(true);
			list->setVisible(true);
			flag = 0;
		break;
		default:
			tabv->setHidden(true);
			tree->setVisible(true);
			list->setVisible(true);
			flag = 1;
		break;
	}

	populate(index);
}

void channelBook::populate(int vv)
{
	debug("channelBook", "populate");

	map<string, vector<pair<int, string>>> data;

	switch (vv)
	{
		case views::Services:
			data = dbih->get_services_index();
		break;
		case views::A_Z:
			data = dbih->get_az_index();
		break;
		case views::Bouquets:
			data = dbih->get_bouquets_index();
		break;
		case views::Satellites:
			data = dbih->get_transponders_index();
		break;
		case views::Providers:
			data = dbih->get_packages_index();
		break;
		case views::Resolution:
			data = dbih->get_resolution_index();
		break;
		case views::Encryption:
			data = dbih->get_encryption_index();
		break;
	}

	tree->scrollToItem(tree->topLevelItem(0));
	tree->clear();
	list->scrollToItem(list->topLevelItem(0));
	list->clear();

	for (auto & q: data)
	{
		//Qt5
		QTreeWidgetItem* item = new QTreeWidgetItem({QString::fromStdString(q.first)});
		tree->addTopLevelItem(item);

		int i = 0;

		for (auto & ch: q.second)
		{
			char ci[6];
			sprintf(ci, "%05d", i++);
			QString x = QString::fromStdString(ci);

			if (dbih->db.services.count(ch.second))
			{
				e2db::service chdata = dbih->db.services[ch.second];
				e2db::transponder txdata = dbih->db.transponders[chdata.txid];

				QString idx = QString::fromStdString(to_string(ch.first));
				QString chname = QString::fromStdString(chdata.chname);
				QString stype = STYPES.count(chdata.stype) ? QString::fromStdString(STYPES.at(chdata.stype)) : "Data";
				QString pname = QString::fromStdString(chdata.data.count(PVDR_DATA.at('p')) ? chdata.data[PVDR_DATA.at('p')][0] : "");

				//Qt5
				QTreeWidgetItem* item = new QTreeWidgetItem({x, idx, chname, stype, pname});
				list->addTopLevelItem(item);
			}
		}
	}
}
}
