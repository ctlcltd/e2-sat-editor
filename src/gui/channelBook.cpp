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

#include "channelBook.h"

using std::to_string;
using namespace e2se;

namespace e2se_gui
{

channelBook::channelBook(e2db* dbih)
{
	this->log = new logger("channelBook");
	debug("channelBook()");

	this->dbih = dbih;
	this->sets = new QSettings;

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
	debug("side()");

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
	debug("layout()");

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

	list->header()->connect(list->header(), &QHeaderView::sectionClicked, [=](int column) { this->trickySortByColumn(column); });
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
	debug("sideRowChanged()", "index", to_string(index));

	tree->clearSelection();
	tree->scrollToTop();
	tree->clear();
	list->clearSelection();
	list->scrollToTop();
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
	debug("stacker()", "index", to_string(vv));

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
			int pos = std::stoi(q.first);
			if (dbih->tuners.count(pos))
			{
				e2db::tuner_sets tn = dbih->tuners.at(std::stoi(q.first));
				name = QString::fromStdString(tn.name);
			}
			else
			{
				name = QString::fromStdString(q.first);
			}
			item = new QTreeWidgetItem({name});

			for (auto & x: q.second)
			{
				e2db::transponder tx = dbih->db.transponders[x.second];
				QString subindex = QString::fromStdString(x.second);
				string ptxp;
				switch (tx.ttype)
				{
					case 's':
						ptxp = to_string(tx.freq) + '/' + e2db::SAT_POL[tx.pol] + '/' + to_string(tx.sr);
					break;
					case 't':
						ptxp = to_string(tx.freq) + '/' + e2db::TER_MOD[tx.termod] + '/' + e2db::TER_BAND[tx.band];
					break;
					case 'c':
						ptxp = to_string(tx.freq) + '/' + e2db::CAB_MOD[tx.cabmod] + '/' + to_string(tx.sr);
					break;
					case 'a':
						ptxp = to_string(tx.freq);
					break;
				}
				QString txp = QString::fromStdString(ptxp);
				subitem = new QTreeWidgetItem(item, {txp});
				subitem->setData(0, Qt::UserRole, subindex);
				tree->addTopLevelItem(subitem);
			}
		}
		else if (vv == views::Bouquets)
		{
			e2db::userbouquet ub = dbih->userbouquets[q.first];
			e2db::bouquet bs = dbih->bouquets[ub.pname];
			name = QString::fromStdString(ub.name);
			name.prepend(QString::fromStdString("[" + bs.nname + "]\t"));
			item = new QTreeWidgetItem({name});
		}
		//TODO test
		else if (vv == views::Resolution)
		{
			int stype = std::stoi(q.first);
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

	debug("populate()", "curr", curr);

	list->header()->setSortIndicatorShown(false);
	list->header()->setSectionsClickable(false);
	list->clear();

	int i = 0;

	for (auto & chdata: data[curr])
	{
		char ci[7];
		std::sprintf(ci, "%06d", i++);
		QString x = QString::fromStdString(ci);

		if (dbih->db.services.count(chdata.second))
		{
			e2db::service ch = dbih->db.services[chdata.second];
			e2db::transponder tx = dbih->db.transponders[ch.txid];

			QString idx = QString::fromStdString(to_string(chdata.first));
			QString chid = QString::fromStdString(chdata.second);
			QString chname;
			if (sets->value("preference/fixUnicodeChars").toBool())
				chname = QString::fromStdString(ch.chname).remove(QRegularExpression("[^\\p{L}\\p{N}\\p{Sm}\\p{M}\\p{P}\\s]+"));
			else
				chname = QString::fromStdString(ch.chname);
			QString stype = e2db::STYPES.count(ch.stype) ? QString::fromStdString(e2db::STYPES.at(ch.stype).second) : "Data";
			QString pname = QString::fromStdString(ch.data.count(e2db::SDATA::p) ? ch.data[e2db::SDATA::p][0] : "");

			string ptxp;
			switch (tx.ttype)
			{
				case 's':
					ptxp = to_string(tx.freq) + '/' + e2db::SAT_POL[tx.pol] + '/' + to_string(tx.sr);
				break;
				case 't':
					ptxp = to_string(tx.freq) + '/' + e2db::TER_MOD[tx.termod] + '/' + e2db::TER_BAND[tx.band];
				break;
				case 'c':
					ptxp = to_string(tx.freq) + '/' + e2db::CAB_MOD[tx.cabmod] + '/' + to_string(tx.sr);
				break;
				case 'a':
					ptxp = to_string(tx.freq);
				break;
			}
			QString txp = QString::fromStdString(ptxp);
			string ppos;
			if (tx.ttype == 's')
			{
				if (dbih->tuners.count(tx.pos))
				{
					ppos = dbih->tuners.at(tx.pos).name;
				}
				else
				{
					char cposdeg[5];
					std::sprintf(cposdeg, "%.1f", float (std::abs (tx.pos)) / 10);
					ppos = (string (cposdeg) + (tx.pos > 0 ? 'E' : 'W'));
				}
			}
			QString pos = QString::fromStdString(ppos);

			QTreeWidgetItem* item = new QTreeWidgetItem({x, idx, chname, stype, pname, txp, pos});
			item->setData(0, Qt::UserRole, chid);
			list->addTopLevelItem(item);
		}
	}

	list->header()->setSectionsClickable(true);
	// sorting default column 0|asc
	if (vx)
	{
		list->sortItems(0, Qt::AscendingOrder);
		list->header()->setSortIndicator(1, Qt::AscendingOrder);
	}
}

void channelBook::trickySortByColumn(int column)
{
	debug("trickySortByColumn()", "column", to_string(column));

	Qt::SortOrder order = list->header()->sortIndicatorOrder();
	column = column == 1 ? 0 : column;

	// sorting by
	if (column)
	{
		list->sortItems(column, order);
		list->header()->setSortIndicatorShown(true);
	}
	// sorting default
	else
	{
		list->sortItems(column, order);
		list->header()->setSortIndicator(1, order);

		// default column 0|asc
		if (order == Qt::AscendingOrder)
			list->header()->setSortIndicatorShown(false);
		else
			list->header()->setSortIndicatorShown(true);
	}
}

vector<QString> channelBook::getSelected()
{
	debug("getSelected()");

	vector<QString> channels;

	for (auto & item : list->selectedItems())
	{
		channels.emplace_back(item->data(0, Qt::UserRole).toString());
	}
	return channels;
}

}
