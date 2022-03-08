/*!
 * e2-sat-editor/src/gui/channelBook.cpp
 *
 * @link https://github.com/ctlcltd/e2-sat-editor
 * @copyright e2 SAT Editor Team
 * @author Leonardo Laureti
 * @version 0.1
 * @license MIT License
 * @license GNU GPLv3 License
 */

#include <cstdio>

#include <QGridLayout>
#include <QTreeWidget>
#include <QHeaderView>
#include <QLabel>

#include "../commons.h"
#include "channelBook.h"

using namespace std;

namespace e2se_gui
{
channelBook::channelBook(e2db* dbih)
{
	debug("channelBook");

	this->dbih = dbih;

	QGridLayout* afrm = new QGridLayout();

	side();
	layout();

	afrm->addWidget(lwid, 0, 0);
	afrm->addLayout(swid, 0, 1);
	afrm->setColumnMinimumWidth(0, 120);
	afrm->setColumnStretch(0, 1);
	afrm->setColumnStretch(1, 5);
	afrm->setSpacing(0);
	afrm->setContentsMargins(0, 0, 0, 0);

	this->widget = new QWidget;
	widget->setLayout(afrm);
}

//TODO dragable application state
void channelBook::side()
{
	debug("channelBook", "side()");

	this->lwid = new QListWidget;
	lwid->setDragEnabled(true);
	//TODO FIX rlpadding
	lwid->setStyleSheet("QListWidget { background: transparent; font: 15px } QListView::item { height: 36px }");
	lwid->setMaximumWidth(160);

	lwid->addItem(" Services ");
	lwid->addItem(" Bouquets ");
	lwid->addItem(" Satellites ");
	lwid->addItem(" Providers ");
	lwid->addItem(" Resolution ");
	lwid->addItem(" Encryption ");
	lwid->addItem(" A-Z ");

	lwid->connect(lwid, &QListWidget::currentRowChanged, [=](int index) { this->sideRowChanged(index); });
}

void channelBook::layout()
{
	debug("channelBook", "layout()");

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

	string chars[27] = {"0-9","A","B","C","D","E","F","G","H","I","J","L","K","M","N","O","P","Q","R","S","T","U","V","W","X","Y","Z"};

	for (unsigned int i=0; i < 27; i++)
	{
		tabv->addTab("");
		tabv->setTabButton(i, QTabBar::LeftSide, new QLabel(QString::fromStdString(chars[i])));
		tabv->setTabData(i, QString::fromStdString(chars[i]));
	}

	this->tree = new QTreeWidget;
	tree->setHidden(true);
	tree->setHeaderHidden(true);
	tree->setUniformRowHeights(true);
	tree->setMinimumWidth(180);

	this->list = new QTreeWidget;
	list->setHidden(true);
	list->setUniformRowHeights(true);

	//Qt5
	QTreeWidgetItem* thead = new QTreeWidgetItem({"", "Index", "Name", "Type", "Provider", "Transponder", "SAT"});
	list->setHeaderItem(thead);
	list->setColumnHidden(0, true);
	list->setColumnWidth(1, 65);	// Index
	list->setColumnWidth(2, 175);	// Name
	list->setColumnWidth(3, 70);	// Type
	list->setColumnWidth(4, 125);	// Provider
	list->setColumnWidth(5, 110);	// Transponder
	list->setColumnWidth(6, 120);	// SAT

	list->header()->connect(list->header(), &::QHeaderView::sectionClicked, [=](int column) { this->trickySortByColumn(column); });
	tree->connect(tree, &QTreeWidget::currentItemChanged, [=]() { this->populate(); });
	tabv->connect(tabv, &QTabBar::currentChanged, [=]() { this->populate(); });

	swid->addWidget(tabv);
	swid->addWidget(tree);
	swid->addWidget(list);

	swid->setStretch(1, 1);
	swid->setStretch(2, 5);
}

void channelBook::sideRowChanged(int index)
{
	debug("channelBook", "sideRowChanged()", "index", to_string(index));

	tree->scrollToItem(tree->topLevelItem(0));
	tree->clear();
	list->setSortingEnabled(false);
	list->scrollToItem(list->topLevelItem(0));
	list->clear();

	switch (index)
	{
		case views::Services:
			tabv->setHidden(true);
			tree->setHidden(true);
			list->setVisible(true);
			vx = 2;
		break;
		case views::A_Z:
			tabv->setVisible(true);
			tree->setHidden(true);
			list->setVisible(true);
			vx = 0;
		break;
		default:
			tabv->setHidden(true);
			tree->setVisible(true);
			list->setVisible(true);
			vx = 1;
	}

	stacker(index);
}

void channelBook::stacker(int vv)
{
	debug("channelBook", "stacker()", "index", to_string(vv));

	switch (vv)
	{
		case views::Services:
			this->data = dbih->get_services_index();
		break;
		case views::A_Z:
			this->data = dbih->get_az_index();
		break;
		case views::Bouquets:
			this->data = dbih->get_userbouquets_index();
		break;
		case views::Satellites:
			this->data = dbih->get_transponders_index();
		break;
		case views::Providers:
			this->data = dbih->get_packages_index();
		break;
		case views::Resolution:
			this->data = dbih->get_resolution_index();
		break;
		case views::Encryption:
			this->data = dbih->get_encryption_index();
		break;
	}

	QString name;

	for (auto & q: data)
	{
		if (vv == views::Bouquets)
		{
			e2db::userbouquet ub = dbih->userbouquets[q.first];
			e2db::bouquet bs = dbih->bouquets[ub.pname];
			name = QString::fromStdString(ub.name);
			name.prepend(QString::fromStdString("[" + bs.nname + "]\t"));
		}
		else if (vv == views::Resolution)
		{
			int stype = stoi(q.first);
			name = e2db::STYPES.count(stype) ? QString::fromStdString(e2db::STYPES.at(stype)) : "Data";
			name.append(QString::fromStdString("\tid: " + q.first));
		}
		else
		{
			name = QString::fromStdString(q.first);
		}

		//Qt5
		QTreeWidgetItem* item = new QTreeWidgetItem({name});
		QString index = QString::fromStdString(q.first);
		item->setData(0, Qt::UserRole, index);
		tree->addTopLevelItem(item);
	}

	populate();
}

void channelBook::populate()
{
	string curr = "";

	if (vx == 2)
	{
		curr = "chs";
	}
	else if (vx == 1)
	{
		QTreeWidgetItem* selected;
		selected = tree->currentItem();

		if (selected == NULL)
			selected = tree->topLevelItem(0);
		if (selected != NULL)
		{
			QString index = selected->data(0, Qt::UserRole).toString();
			curr = index.toStdString();
		}
	}
	else
	{
		int selected = tabv->currentIndex();
		QString index = tabv->tabData(selected).toString();
		curr = index.toStdString();
	}

	debug("channelBook", "populate()", "curr", curr);

	list->setSortingEnabled(false);
	list->scrollToItem(list->topLevelItem(0));
	list->clear();

	int i = 0;

	for (auto & ch: data[curr])
	{
		char ci[7];
		sprintf(ci, "%06d", i++);
		QString x = QString::fromStdString(ci);

		if (dbih->db.services.count(ch.second))
		{
			e2db::service chdata = dbih->db.services[ch.second];
			e2db::transponder txdata = dbih->db.transponders[chdata.txid];

			QString idx = QString::fromStdString(to_string(ch.first));
			QString chname = QString::fromStdString(chdata.chname);
			QString stype = e2db::STYPES.count(chdata.stype) ? QString::fromStdString(e2db::STYPES.at(chdata.stype)) : "Data";
			QString pname = QString::fromStdString(chdata.data.count(e2db::PVDR_DATA.at('p')) ? chdata.data[e2db::PVDR_DATA.at('p')][0] : "");

			string ptxp;
			if (txdata.ttype == 's')
				ptxp = txdata.freq + '/' + e2db::SAT_POL[txdata.pol] + '/' + txdata.sr;
			else if (txdata.ttype == 't')
				ptxp = txdata.freq + '/' + e2db::TER_MOD[txdata.termod] + '/' + e2db::TER_BAND[txdata.band];
			else if (txdata.ttype == 'c')
				ptxp = txdata.freq + '/' + e2db::CAB_MOD[txdata.cabmod] + '/' + txdata.sr;
			QString txp = QString::fromStdString(ptxp);
			string ppos;
			if (txdata.ttype == 's')
			{
				if (dbih->tuners.count(txdata.pos)) {
					ppos = dbih->tuners.at(txdata.pos).name;
				} else {
					char cposdeg[5];
					sprintf(cposdeg, "%.1f", float(txdata.pos / 10));
					ppos = (string (cposdeg) + (txdata.pos ? 'E' : 'W'));
				}
			}
			QString pos = QString::fromStdString(ppos);

			//Qt5
			QTreeWidgetItem* item = new QTreeWidgetItem({x, idx, chname, stype, pname, txp, pos});
			list->addTopLevelItem(item);
		}
	}

	list->setSortingEnabled(true);
	if (vx) list->sortByColumn(0, Qt::AscendingOrder);
}

//TODO FIX
void channelBook::trickySortByColumn(int column)
{
	debug("channelBook", "trickySortByColumn()", "column", to_string(column));

	Qt::SortOrder order = list->header()->sortIndicatorOrder();
	column = column == 1 ? 0 : column;

	if (column)
	{
		list->sortItems(column, order);
	}
	else
	{
		list->sortByColumn(column, order);
		list->header()->setSortIndicator(1, order);
	}
}
}
