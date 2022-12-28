/*!
 * e2-sat-editor/src/gui/channelBookView.cpp
 *
 * @link https://github.com/ctlcltd/e2-sat-editor
 * @copyright e2 SAT Editor Team
 * @author Leonardo Laureti
 * @version 0.1
 * @license MIT License
 * @license GNU GPLv3 License
 */

#include <QGridLayout>
#include <QSplitter>
#include <QHeaderView>
#include <QLabel>

#include "channelBookView.h"
#include "theme.h"
#include "tab.h"
#include "gui.h"

using std::to_string;
using namespace e2se;

namespace e2se_gui
{

channelBookView::channelBookView(dataHandler* data, e2se::logger::session* log)
{
	this->log = new logger(log, "channelBookView");
	debug("channelBookView()");

	this->data = data;
	this->sets = new QSettings;
	this->widget = new QWidget;

	layout();
}

channelBookView::channelBookView(tab* tid, QWidget* cwid, dataHandler* data, e2se::logger::session* log)
{
	this->log = new logger(log, "channelBookView");
	debug("channelBookView()");

	this->tid = tid;
	this->cwid = cwid;
	this->data = data;
	this->sets = new QSettings;
	this->widget = new QWidget;

	layout();
}

//TODO tabv broken when tooling
void channelBookView::layout()
{
	debug("layout()");

	QGridLayout* frm = new QGridLayout(widget);

	sideLayout();

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

	for (unsigned int i = 0; i < 27; i++)
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
	list->setStyleSheet("::item { padding: 2px auto }");

	QTreeWidgetItem* thead = new QTreeWidgetItem({NULL, "Index", "Name", "Type", "Provider", "Transponder", "Position", "System"});
	list->setHeaderItem(thead);
	list->setColumnHidden(ITEM_ROW_ROLE::x, true);		// hidden index
	list->setColumnWidth(ITEM_ROW_ROLE::chnum, 60);		// (Channel Number) Index
	list->setColumnWidth(ITEM_ROW_ROLE::chname, 175);	// Name
	list->setColumnWidth(ITEM_ROW_ROLE::chtype, 70);	// Type
	list->setColumnWidth(ITEM_ROW_ROLE::chpname, 125);	// Provider
	list->setColumnWidth(ITEM_ROW_ROLE::chsys, 65);		// System
	list->setColumnWidth(ITEM_ROW_ROLE::chtxp, 175);	// Transponder
	list->setColumnWidth(ITEM_ROW_ROLE::chpos, 120);	// Position

	list->header()->connect(list->header(), &QHeaderView::sectionClicked, [=](int column) { this->sortByColumn(column); });
	tree->connect(tree, &QTreeWidget::currentItemChanged, [=]() { this->populate(); });
	tabv->connect(tabv, &QTabBar::currentChanged, [=]() { this->populate(); });

	swid->addWidget(tree);
	swid->addWidget(list);

	swid->setStretchFactor(0, 1);
	swid->setStretchFactor(1, 5);

	awid->addWidget(tabv);
	awid->addWidget(swid);

	frm->addWidget(lwid, 0, 0);
	frm->addLayout(awid, 0, 1);
	frm->setColumnMinimumWidth(0, 140);
	frm->setColumnStretch(0, 1);
	frm->setColumnStretch(1, 5);
	frm->setSpacing(0);
	frm->setContentsMargins(0, 0, 0, 0);
}

void channelBookView::sideLayout()
{
	this->lwid = new QListWidget;
	lwid->setStyleSheet("QListWidget { background: transparent; font: 15px } QListView::item { padding: 10px auto }");
	lwid->setMaximumWidth(160);

	lwid->addItem("Services");
	lwid->addItem("Bouquets");
	lwid->addItem("Satellites");
	lwid->addItem("Providers");
	lwid->addItem("Resolution");
	lwid->addItem("Encryption");
	lwid->addItem("A-Z");

	lwid->item(0)->setIcon(theme::spacer(2));
	lwid->item(1)->setIcon(theme::spacer(2));
	lwid->item(2)->setIcon(theme::spacer(2));
	lwid->item(3)->setIcon(theme::spacer(2));
	lwid->item(4)->setIcon(theme::spacer(2));
	lwid->item(5)->setIcon(theme::spacer(2));
	lwid->item(6)->setIcon(theme::spacer(2));

	lwid->connect(lwid, &QListWidget::currentRowChanged, [=](int index) { this->sideRowChanged(index); });
}

void channelBookView::load()
{
	debug("load()");

	tabUpdateFlags(gui::init);
	this->dbih = this->data->dbih;

	sideRowChanged(0);
}

void channelBookView::reset()
{
	debug("reset()");

	tree->reset();
	tree->clear();

	list->reset();
	list->clear();
	list->header()->setSortIndicatorShown(false);
	list->header()->setSectionsClickable(false);
	list->header()->setSortIndicator(0, Qt::AscendingOrder);

	this->lsr_find.curr = -1;
	this->lsr_find.match.clear();

	tabResetStatus();

	this->dbih = nullptr;
}

void channelBookView::populate()
{
	string curr = "";

	if (this->state.vx == 2)
	{
		curr = "chs";
	}
	else if (this->state.vx == 1)
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

	this->state.curr = curr;

	debug("populate()", "current", curr);

	list->header()->setSortIndicatorShown(false);
	list->header()->setSectionsClickable(false);
	list->clear();

	int i = 0;

	for (auto & chdata : this->index[curr])
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
			// macos: unwanted chars [qt.qpa.fonts] Menlo notice
			QString chname;
			if (sets->value("preference/fixUnicodeChars").toBool())
				chname = QString::fromStdString(ch.chname).remove(QRegularExpression("[^\\p{L}\\p{M}\\p{N}\\p{P}\\p{S}\\s]+"));
			else
				chname = QString::fromStdString(ch.chname);
			QString stype = QString::fromStdString(dbih->value_service_type(ch.stype));
			//TODO value
			QString pname = QString::fromStdString(ch.data.count(e2db::SDATA::p) ? ch.data[e2db::SDATA::p][0] : "");
			QString sys = QString::fromStdString(dbih->value_transponder_system(tx));
			QString txp = QString::fromStdString(dbih->value_transponder_combo(tx));
			QString pos = QString::fromStdString(dbih->get_transponder_name_value(tx));

			QTreeWidgetItem* item = new QTreeWidgetItem({x, idx, chname, stype, pname, sys, txp, pos});
			item->setData(0, Qt::UserRole, chid);
			item->setIcon(1, theme::spacer(3));
			list->addTopLevelItem(item);
		}
	}

	list->header()->setSectionsClickable(true);
	// sorting default column 0|asc
	if (this->state.vx)
	{
		list->sortItems(0, Qt::AscendingOrder);
		list->header()->setSortIndicator(1, Qt::AscendingOrder);
	}
}

void channelBookView::sideRowChanged(int index)
{
	debug("sideRowChanged()", "view", index);

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
			this->state.vx = 2;
		break;
		case views::A_Z:
			tabv->setVisible(true);
			tree->setHidden(true);
			list->setVisible(true);
			this->state.vx = 0;
		break;
		default:
			tabv->setHidden(true);
			tree->setVisible(true);
			list->setVisible(true);
			this->state.vx = 1;
	}

	stacker(index);
}

void channelBookView::stacker(int vv)
{
	debug("stacker()", "view", vv);

	switch (vv)
	{
		case views::Services:
			this->index = dbih->get_services_index();
		break;
		case views::A_Z:
			this->index = dbih->get_az_index();
		break;
		case views::Bouquets:
			this->index = dbih->get_bouquets_index();
		break;
		case views::Satellites:
			this->index = dbih->get_transponders_index();
		break;
		case views::Providers:
			this->index = dbih->get_packages_index();
		break;
		case views::Resolution:
			this->index = dbih->get_resolution_index();
		break;
		case views::Encryption:
			this->index = dbih->get_encryption_index();
		break;
	}

	QString index;
	QString name;
	QTreeWidgetItem* item;
	QTreeWidgetItem* subitem;

	for (auto & q : this->index)
	{
		//TODO pos value 0 with terrestrial, cable, atsc
		if (vv == views::Satellites)
		{
			int pos = std::stoi(q.first);
			if (dbih->tuners_pos.count(pos))
			{
				string tnid = dbih->tuners_pos.at(pos);
				e2db::tunersets_table tn = dbih->tuners[0].tables[tnid];
				name = QString::fromStdString(tn.name);
			}
			else
			{
				name = QString::fromStdString(q.first);
			}
			item = new QTreeWidgetItem({name});

			for (auto & x : q.second)
			{
				e2db::transponder tx = dbih->db.transponders[x.second];
				QString subindex = QString::fromStdString(x.second);
				string ptxp = dbih->value_transponder_combo(tx);
				QString txp = QString::fromStdString(ptxp);
				subitem = new QTreeWidgetItem(item, {txp});
				subitem->setData(0, Qt::UserRole, subindex);
				tree->addTopLevelItem(subitem);
			}
		}
		//TODO sort order: TV, Radio
		else if (vv == views::Bouquets)
		{
			e2db::bouquet bs = dbih->bouquets[q.first];
			name = QString::fromStdString(bs.nname.empty() ? bs.name : bs.nname);
			item = new QTreeWidgetItem({name});

			for (string & ubname : bs.userbouquets)
			{
				e2db::userbouquet ub = dbih->userbouquets[ubname];
				QString subindex = QString::fromStdString(ubname);
				QString name = QString::fromStdString(ub.name);
				subitem = new QTreeWidgetItem(item, {name});
				subitem->setData(0, Qt::UserRole, subindex);
				tree->addTopLevelItem(subitem);
			}
		}
		else if (vv == views::Resolution)
		{
			int stype = std::stoi(q.first);
			name = QString::fromStdString(dbih->value_service_type(stype));
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
		this->index = dbih->get_channels_index();
	}
	else if (vv == views::Bouquets)
	{
		this->index.merge(dbih->get_userbouquets_index()); //C++17
		tree->expandAll();
	}

	populate();
}

//TODO
void channelBookView::listItemCopy(bool cut)
{
	debug("listItemCopy()");

	QList<QTreeWidgetItem*> selected = list->selectedItems();
	
	if (selected.empty())
		return;

	if (cut)
		listItemDelete();
}

vector<QString> channelBookView::getSelected()
{
	debug("getSelected()");

	vector<QString> items;

	for (auto & item : list->selectedItems())
	{
		QString qchid = item->data(0, Qt::UserRole).toString();
		string chid = qchid.toStdString();
		string crefid = dbih->get_reference_id(chid);
		QString refid = QString::fromStdString(crefid);
		items.emplace_back(refid);
	}
	return items;
}

}
