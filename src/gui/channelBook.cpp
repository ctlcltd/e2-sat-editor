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
#include <cmath>

#include <QGridLayout>
#include <QSplitter>
#include <QTreeWidget>
#include <QHeaderView>
#include <QLabel>

#include "../commons.h"
#include "channelBook.h"

using std::to_string, std::stoi;
using namespace e2se;

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
	afrm->addLayout(awid, 0, 1);
	afrm->setColumnMinimumWidth(0, 120);
	afrm->setColumnStretch(0, 1);
	afrm->setColumnStretch(1, 5);
	afrm->setSpacing(0);
	afrm->setContentsMargins(0, 0, 0, 0);

	this->widget = new QWidget;
	widget->setLayout(afrm);
}

//TODO draggable application state
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

	this->awid = new QHBoxLayout;
	awid->setContentsMargins(0, 0, 0, 0);
	awid->setSpacing(0);

	QSplitter* swid = new QSplitter;

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
	list->setRootIsDecorated(false);
	list->setSelectionBehavior(QAbstractItemView::SelectRows);
	list->setSelectionMode(QAbstractItemView::ExtendedSelection);
	list->setItemsExpandable(false);
	list->setExpandsOnDoubleClick(false);

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

	swid->addWidget(tree);
	swid->addWidget(list);

	swid->setStretchFactor(0, 1);
	swid->setStretchFactor(1, 5);

	awid->addWidget(tabv);
	awid->addWidget(swid);
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

//TODO FIX *index() load
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

	QString index;
	QString name;
	QTreeWidgetItem* item;
	QTreeWidgetItem* subitem;

	for (auto & q: data)
	{
		//TODO pos value & terrestrial, cable ...
		if (vv == views::Satellites)
		{
			int pos = stoi(q.first);
			if (dbih->tuners.count(pos))
			{
				e2db::tuner_sets tndata = dbih->tuners.at(stoi(q.first));
				name = QString::fromStdString(tndata.name);
			}
			else
			{
				name = QString::fromStdString(q.first);
			}
			item = new QTreeWidgetItem({name});

			for (auto & x: q.second)
			{
				e2db::transponder txdata = dbih->db.transponders[x.second];
				QString subindex = QString::fromStdString(x.second);
				string ptxp;
				if (txdata.ttype == 's')
					ptxp = to_string(txdata.freq) + '/' + e2db::SAT_POL[txdata.pol] + '/' + to_string(txdata.sr);
				else if (txdata.ttype == 't')
					ptxp = to_string(txdata.freq) + '/' + e2db::TER_MOD[txdata.termod] + '/' + e2db::TER_BAND[txdata.band];
				else if (txdata.ttype == 'c')
					ptxp = to_string(txdata.freq) + '/' + e2db::CAB_MOD[txdata.cabmod] + '/' + to_string(txdata.sr);
				QString txp = QString::fromStdString(ptxp);
				subitem = new QTreeWidgetItem(item, {txp});
				subitem->setData(0, Qt::UserRole, subindex);
				tree->addTopLevelItem(subitem);
			}
		}
		else if (vv == views::Bouquets)
		{
			e2db::userbouquet ubdata = dbih->userbouquets[q.first];
			e2db::bouquet bsdata = dbih->bouquets[ubdata.pname];
			name = QString::fromStdString(ubdata.name);
			name.prepend(QString::fromStdString("[" + bsdata.nname + "]\t"));
			item = new QTreeWidgetItem({name});
		}
		//TODO test
		else if (vv == views::Resolution)
		{
			int stype = stoi(q.first);
			name = e2db::STYPES.count(stype) ? QString::fromStdString(e2db::STYPES.at(stype).second) : "Data";
			name.append(QString::fromStdString("\tid: " + q.first));
			item = new QTreeWidgetItem({name});
		}
		else
		{
			name = QString::fromStdString(q.first);
			item = new QTreeWidgetItem({name});
		}

		index = QString::fromStdString(q.first);
		item->setData(0, Qt::UserRole, index);
		tree->addTopLevelItem(item);
	}
	if (vv == views::Satellites)
	{
		this->data = dbih->get_channels_index();
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
			selected->setExpanded(true);
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
			QString chid = QString::fromStdString(ch.second);
			QString chname = QString::fromStdString(chdata.chname);
			QString stype = e2db::STYPES.count(chdata.stype) ? QString::fromStdString(e2db::STYPES.at(chdata.stype).second) : "Data";
			QString pname = QString::fromStdString(chdata.data.count(e2db::PVDR_DATA.at('p')) ? chdata.data[e2db::PVDR_DATA.at('p')][0] : "");

			string ptxp;
			if (txdata.ttype == 's')
				ptxp = to_string(txdata.freq) + '/' + e2db::SAT_POL[txdata.pol] + '/' + to_string(txdata.sr);
			else if (txdata.ttype == 't')
				ptxp = to_string(txdata.freq) + '/' + e2db::TER_MOD[txdata.termod] + '/' + e2db::TER_BAND[txdata.band];
			else if (txdata.ttype == 'c')
				ptxp = to_string(txdata.freq) + '/' + e2db::CAB_MOD[txdata.cabmod] + '/' + to_string(txdata.sr);
			QString txp = QString::fromStdString(ptxp);
			string ppos;
			if (txdata.ttype == 's')
			{
				if (dbih->tuners.count(txdata.pos))
				{
					ppos = dbih->tuners.at(txdata.pos).name;
				}
				else
				{
					char cposdeg[5];
					sprintf(cposdeg, "%.1f", float (abs (txdata.pos)) / 10);
					ppos = (string (cposdeg) + (txdata.pos > 0 ? 'E' : 'W'));
				}
			}
			QString pos = QString::fromStdString(ppos);

			QTreeWidgetItem* item = new QTreeWidgetItem({x, idx, chname, stype, pname, txp, pos});
			item->setData(0, Qt::UserRole, chid);
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

vector<QString> channelBook::getSelected()
{
	debug("channelBook", "getSelected()");

	vector<QString> channels;

	for (auto & item : list->selectedItems())
	{
		channels.emplace_back(item->data(0, Qt::UserRole).toString());
	}
	return channels;
}

}
