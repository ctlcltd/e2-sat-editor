/*!
 * e2-sat-editor/src/gui/channelBookView.cpp
 *
 * @link https://github.com/ctlcltd/e2-sat-editor
 * @copyright e2 SAT Editor Team
 * @author Leonardo Laureti
 * @version 0.4
 * @license MIT License
 * @license GNU GPLv3 License
 */

#include <algorithm>

#include <QtGlobal>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QSplitter>
#include <QHeaderView>
#include <QLabel>
#include <QClipboard>
#include <QMimeData>

#include "platforms/platform.h"

#include "channelBookView.h"
#include "theme.h"
#include "tab.h"

using std::to_string, std::sort;

using namespace e2se;

namespace e2se_gui
{

channelBookView::channelBookView(dataHandler* data, int stype)
{
	this->log = new logger("gui", "channelBookView");

	this->data = data;
	this->theme = new e2se_gui::theme;
	this->widget = new QWidget;

	this->state.sy = stype;

	layout();
}

channelBookView::channelBookView(tab* tid, QWidget* cwid, dataHandler* data)
{
	this->log = new logger("gui", "channelBookView");

	this->tid = tid;
	this->cwid = cwid;
	this->data = data;
	this->theme = new e2se_gui::theme;
	this->widget = new QWidget;

	layout();
}

void channelBookView::layout()
{
	debug("layout");

	QGridLayout* frm = new QGridLayout(widget);

	QWidget* afrm = new QWidget;
	QWidget* bfrm = new QWidget;

	QVBoxLayout* abox = new QVBoxLayout;
	QHBoxLayout* bbox = new QHBoxLayout;

	QSplitter* bswid = new QSplitter;

	frm->setContentsMargins(0, 0, 0, 0);
	abox->setContentsMargins(0, 0, 0, 0);
	bbox->setContentsMargins(0, 0, 0, 0);

	frm->setSpacing(0);
	abox->setSpacing(0);
	bbox->setSpacing(0);

	sideLayout();

	this->tree = new QTreeWidget;
	this->list = new QTreeWidget;
	this->tabv = new QTabBar;

	tabv->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Ignored);

	list->setStyleSheet("QTreeWidget::item { padding: 2px 0 }");
	tabv->setStyleSheet("QTabBar::tab { min-width: 48px; margin-top: 0 }");

#ifdef Q_OS_MAC
	QColor itembackground;
	QString itembackground_hexArgb;

	itembackground = QColor(Qt::black);
	itembackground.setAlphaF(0.08);
	itembackground_hexArgb = itembackground.name(QColor::HexArgb);

	theme->dynamicStyleSheet(widget, "QTreeWidget::item:selected:!active, QListWidget::item:selected:!active { selection-background-color: " + itembackground_hexArgb + " }", theme::light);

	itembackground = QPalette().color(QPalette::Dark);
	itembackground.setAlphaF(0.15);
	itembackground_hexArgb = itembackground.name(QColor::HexArgb);

	theme->dynamicStyleSheet(widget, "QTreeWidget::item:selected:!active, QListWidget::item:selected:!active { selection-background-color: " + itembackground_hexArgb + " }", theme::dark);
#endif

	tree->setHidden(true);
	tree->setHeaderHidden(true);
	tree->setUniformRowHeights(true);
	tree->setMinimumWidth(180);

	list->setHidden(true);
	list->setUniformRowHeights(true);
	list->setRootIsDecorated(false);
	list->setSelectionBehavior(QAbstractItemView::SelectRows);
	list->setSelectionMode(QAbstractItemView::ExtendedSelection);
	list->setItemsExpandable(false);
	list->setExpandsOnDoubleClick(false);

	QTreeWidgetItem* list_thead = new QTreeWidgetItem({NULL, "Index", "Name", "Type", "Provider", "Transponder", "Position", "System"});
	list->setHeaderItem(list_thead);
	list->setColumnHidden(ITEM_ROW_ROLE::x, true);		// hidden index
	list->setColumnWidth(ITEM_ROW_ROLE::chnum, 60);		// (Channel Number) Index
	list->setColumnWidth(ITEM_ROW_ROLE::chname, 175);	// Name
	list->setColumnWidth(ITEM_ROW_ROLE::chtype, 70);	// Type
	list->setColumnWidth(ITEM_ROW_ROLE::chpname, 125);	// Provider
	list->setColumnWidth(ITEM_ROW_ROLE::chtxp, 165);	// Transponder
	list->setColumnWidth(ITEM_ROW_ROLE::chpos, 80);		// Position
	list->setColumnWidth(ITEM_ROW_ROLE::chsys, 75);		// System

	tabv->setHidden(true);
	tabv->setShape(QTabBar::RoundedWest);
	tabv->setDocumentMode(true);
	tabv->setUsesScrollButtons(true);
	tabv->setExpanding(false);
	tabv->setDrawBase(true);

	string chars[27] = {"0-9","A","B","C","D","E","F","G","H","I","J","L","K","M","N","O","P","Q","R","S","T","U","V","W","X","Y","Z"};

	for (int i = 0; i < 27; i++)
	{
		tabv->addTab("");
		tabv->setTabButton(i, QTabBar::LeftSide, new QLabel(QString::fromStdString(chars[i])));
		tabv->setTabData(i, QString::fromStdString(chars[i]));
	}

	list->header()->connect(list->header(), &QHeaderView::sectionClicked, [=](int column) { this->sortByColumn(column); });
	tree->connect(tree, &QTreeWidget::currentItemChanged, [=]() { this->populate(); });
	tabv->connect(tabv, &QTabBar::currentChanged, [=]() { this->populate(); });

	if (tid != nullptr)
	{
		list->setContextMenuPolicy(Qt::CustomContextMenu);
		list->connect(list, &QTreeWidget::customContextMenuRequested, [=](QPoint pos) { this->showListEditContextMenu(pos); });

		list->connect(list, &QTreeWidget::itemSelectionChanged, [=]() { this->listItemSelectionChanged(); });
	}

	bswid->addWidget(tree);
	bswid->addWidget(list);

	bswid->setStretchFactor(0, 1);
	bswid->setStretchFactor(1, 5);

	abox->addWidget(lwid);
	afrm->setLayout(abox);

	bbox->addWidget(tabv);
	bbox->addWidget(bswid);
	bfrm->setLayout(bbox);

	platform::osWidgetBlend(afrm);
	platform::osWidgetOpaque(bfrm);

	//TODO FIX
	afrm->setMinimumWidth(150);
	afrm->setMaximumWidth(250);

	frm->addWidget(afrm, 0, 0);
	frm->addWidget(bfrm, 0, 1);

	frm->setColumnStretch(0, 1);
	frm->setColumnStretch(1, 5);
}

void channelBookView::sideLayout()
{
	this->lwid = new QListWidget;
	lwid->setStyleSheet("QListWidget { background: transparent; font-size: 15px } QListView::item { padding: 10px auto }");

	lwid->addItem("Services");
	lwid->addItem("Bouquets");
	lwid->addItem("Positions");
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
	debug("load");

	tabUpdateFlags(gui::init);

	sideRowChanged(0);
}

void channelBookView::reset()
{
	debug("reset");

	tree->reset();
	tree->clear();

	list->reset();
	list->clear();
	list->header()->setSortIndicatorShown(false);
	list->header()->setSectionsClickable(false);
	list->header()->setSortIndicator(0, Qt::AscendingOrder);

	this->lsr_find.curr = -1;
	this->lsr_find.match.clear();

	resetStatusBar();
}

void channelBookView::populate()
{
	string curr = "";

	QList<QTreeWidgetItem*> items;

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

	auto* dbih = this->data->dbih;

	debug("populate", "current", curr);

	list->header()->setSortIndicatorShown(false);
	list->header()->setSectionsClickable(false);
	list->clear();

	int i = 0;

	for (auto & chdata : this->index[curr])
	{
		char ci[7];
		std::snprintf(ci, 7, "%06d", i++);
		QString x = QString::fromStdString(ci);

		if (dbih->db.services.count(chdata.second))
		{
			e2db::service ch = dbih->db.services[chdata.second];

			int atype = dbih->value_service_super_type(ch);
			bool disabled = this->state.sy != -1 && this->state.sy != atype;
			QString idx = QString::number(chdata.first);
			QString chid = QString::fromStdString(chdata.second);

			QStringList entry = dbih->entries.services[chdata.second];
			QString txp;
			if (ch.tsid != 0)
			{
				e2db::transponder tx = dbih->db.transponders[ch.txid];
				txp = QString::fromStdString(dbih->value_transponder_combo(tx));

				entry.move(9, 10);
				entry.insert(9, txp);
			}
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
			entry.remove(7);
			entry.remove(1, 5);
#else
			entry.removeAt(7);
			for (int i = 0; i != 5; i++) entry.removeAt(1);
#endif
			entry.prepend(idx);
			entry.prepend(x);

			QTreeWidgetItem* item = new QTreeWidgetItem(entry);
			item->setData(0, Qt::UserRole, chid);
			item->setData(1, Qt::UserRole, disabled);
			item->setIcon(1, theme::spacer(3));
			item->setDisabled(disabled);

			items.append(item);
		}
	}

	list->addTopLevelItems(items);

	list->header()->setSectionsClickable(true);
	// sorting default column 0|asc
	if (this->state.vx)
	{
		list->sortItems(0, Qt::AscendingOrder);
		list->header()->setSortIndicator(1, Qt::AscendingOrder);
	}
}

void channelBookView::stacker(int vv)
{
	debug("stacker", "view", vv);

	auto* dbih = this->data->dbih;

	vector<string> index;

	if (vv == views::Services)
	{
		this->index = dbih->get_services_index();
		return populate();
	}
	else if (vv == views::Bouquets)
	{
		this->index = dbih->get_bouquets_index();
		for (auto & x : dbih->index["bss"])
			index.emplace_back(x.second);
	}
	else if (vv == views::Positions)
	{
		this->index = dbih->get_transponders_index();
		vector<int> tmpx;
		for (auto & x : this->index)
			tmpx.emplace_back(std::stoi(x.first));
		sort(tmpx.begin(), tmpx.end());
		for (int & q : tmpx)
			index.emplace_back(to_string(q));
	}
	else if (vv == views::Providers)
	{
		this->index = dbih->get_packages_index();
		for (auto & x : this->index)
			index.emplace_back(x.first);
	}
	else if (vv == views::Resolution)
	{
		this->index = dbih->get_resolution_index();
		vector<int> tmpx;
		for (auto & x : this->index)
			tmpx.emplace_back(std::stoi(x.first));
		sort(tmpx.begin(), tmpx.end());
		for (int & q : tmpx)
			index.emplace_back(std::to_string(q));
	}
	else if (vv == views::Encryption)
	{
		this->index = dbih->get_encryption_index();
		for (auto & x : this->index)
			index.emplace_back(x.first);
	}
	else if (vv == views::A_Z)
	{
		this->index = dbih->get_az_index();
		return populate();
	}

	QList<QTreeWidgetItem*> items;

	for (string & w : index)
	{
		QString idx;
		QString name;
		QTreeWidgetItem* item;
		QTreeWidgetItem* subitem;
		bool disabled = false;

		if (vv == views::Positions)
		{
			int pos = std::stoi(w);
			if (dbih->tuners_pos.count(pos))
			{
				string tnid = dbih->tuners_pos.at(pos);
				e2db::tunersets_table tn = dbih->tuners[0].tables[tnid];
				name = QString::fromStdString(tn.name);
			}
			else
			{
				string ppos = pos == -1 ? "NaN" : w;
				name = QString::fromStdString(ppos);
			}
			item = new QTreeWidgetItem({name});

			for (auto & x : this->index[w])
			{
				e2db::transponder tx = dbih->db.transponders[x.second];
				QString subindex = QString::fromStdString(x.second);
				string ptxp = dbih->value_transponder_combo(tx);
				QString txp = QString::fromStdString(ptxp);
				subitem = new QTreeWidgetItem(item, {txp});
				subitem->setData(0, Qt::UserRole, subindex);
				items.append(subitem);
			}
		}
		else if (vv == views::Bouquets)
		{
			e2db::bouquet bs = dbih->bouquets[w];
			name = QString::fromStdString(bs.nname.empty() ? bs.name : bs.nname);
			disabled = this->state.sy != -1 && this->state.sy != bs.btype;
			item = new QTreeWidgetItem({name});

			for (string & ubname : bs.userbouquets)
			{
				e2db::userbouquet ub = dbih->userbouquets[ubname];
				QString subindex = QString::fromStdString(ubname);
				QString name = QString::fromStdString(ub.name);
				subitem = new QTreeWidgetItem(item, {name});
				subitem->setData(0, Qt::UserRole, subindex);
				items.append(subitem);
			}
		}
		else if (vv == views::Resolution)
		{
			int stype = std::stoi(w);
			int atype = dbih->value_service_super_type(stype);
			QString pad = w.length() == 2 ? "  " : "  ";
			disabled = this->state.sy != -1 && this->state.sy != atype;
			name = QString::number(stype);
			name.append(pad);
			name.append(QString::fromStdString(dbih->value_service_type(stype)));
			item = new QTreeWidgetItem({name});
		}
		else
		{
			name = QString::fromStdString(w);
			item = new QTreeWidgetItem({name});
		}

		idx = QString::fromStdString(w);
		item->setData(0, Qt::UserRole, idx);
		item->setData(1, Qt::UserRole, disabled);
		item->setDisabled(disabled);

		items.append(item);
	}

	tree->addTopLevelItems(items);

	if (vv == views::Positions)
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

void channelBookView::sideRowChanged(int index)
{
	debug("sideRowChanged", "view", index);

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

	updateFlags();
}

void channelBookView::filterChanged(bool enabled)
{
	debug("filterChanged", "enabled", enabled);

	int i, count;

	i = 0;
	count = tree->topLevelItemCount();
	while (i != count)
	{
		QTreeWidgetItem* item = tree->topLevelItem(i);
		bool disabled = item->data(1, Qt::UserRole).toBool();
		item->setDisabled(enabled && disabled);
		i++;
	}
	i = 0;
	count = list->topLevelItemCount();
	while (i != count)
	{
		QTreeWidgetItem* item = list->topLevelItem(i);
		bool disabled = item->data(1, Qt::UserRole).toBool();
		item->setDisabled(enabled && disabled);
		i++;
	}
}

void channelBookView::listItemSelectionChanged()
{
	// debug("listItemSelectionChanged");

	QList<QTreeWidgetItem*> selected = list->selectedItems();

	if (selected.empty())
	{
		tabSetFlag(gui::TabListCopy, false);
	}
	else
	{
		tabSetFlag(gui::TabListCopy, true);
	}
}

void channelBookView::listItemCopy(bool cut)
{
	debug("listItemCopy");

	QList<QTreeWidgetItem*> selected = list->selectedItems();

	if (selected.empty())
		return;

	auto* dbih = this->data->dbih;

	QClipboard* clipboard = QGuiApplication::clipboard();
	QStringList text;
	for (auto & item : selected)
	{
		QString qchid = item->data(0, Qt::UserRole).toString();
		string chid = qchid.toStdString();

		QStringList data;
		// start from chnum column [1]
		for (int i = ITEM_ROW_ROLE::chnum; i < list->columnCount(); i++)
		{
			QString qstr = item->data(i, Qt::DisplayRole).toString();
			// chname
			if (i == ITEM_ROW_ROLE::chname)
				qstr.prepend("\"").append("\"");
			// chpname
			else if (i == ITEM_ROW_ROLE::chpname)
				qstr.prepend("\"").append("\"");
			// chtxp
			else if (i == ITEM_ROW_ROLE::chtxp)
				qstr.prepend("\"").append("\"");
			// chpos
			else if (i == ITEM_ROW_ROLE::chpos)
				qstr.prepend("\"").append("\"");
			data.append(qstr);
		}

		// Reference ID
		QString refid;
		string crefid = dbih->get_reference_id(chid);
		refid = QString::fromStdString(crefid);
		data.insert(2, refid); // insert refid column [2]
		text.append(data.join(",")); // CSV
	}
	clipboard->setText(text.join("\n")); // CSV

	if (cut)
		listItemDelete();
}

vector<QString> channelBookView::getSelected()
{
	debug("getSelected");

	auto* dbih = this->data->dbih;

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

void channelBookView::showListEditContextMenu(QPoint &pos)
{
	debug("showListEditContextMenu");

	QList<QTreeWidgetItem*> selected = list->selectedItems();

	if (selected.empty())
		return;

	QMenu* list_edit = contextMenu();

	contextMenuAction(list_edit, "&Copy", [=]() { this->listItemCopy(); }, tabGetFlag(gui::TabListCopy), QKeySequence::Copy);

	platform::osContextMenuPopup(list_edit, list, pos);
	// list_edit->exec(list->mapToGlobal(pos));
}

void channelBookView::updateFlags()
{
	if (tid == nullptr)
		return;

	debug("updateFlags");

	tabSetFlag(gui::FileImport, false);
	tabSetFlag(gui::FileExport, false);
	tabUpdateToolBars();

	if (list->topLevelItemCount())
	{
		tabSetFlag(gui::TabListSelectAll, true);
		// tabSetFlag(gui::TabListFind, true);
	}
	else
	{
		tabSetFlag(gui::TabListSelectAll, false);
		// tabSetFlag(gui::TabListFind, false);
	}

	auto* dbih = this->data->dbih;

	if (dbih->index.count("chs"))
	{
		tabSetFlag(gui::OpenChannelBook, true);
	}
	else
	{
		tabSetFlag(gui::OpenChannelBook, false);
	}

	tabSetFlag(gui::TabListFind, false);
	tabSetFlag(gui::TabListFindNext, false);
	tabSetFlag(gui::TabListFindPrev, false);
	tabSetFlag(gui::TabListFindAll, false);

	tabSetFlag(gui::TunersetsSat, true);
	tabSetFlag(gui::TunersetsTerrestrial, true);
	tabSetFlag(gui::TunersetsCable, true);
	tabSetFlag(gui::TunersetsAtsc, true);

	tabUpdateFlags();
}

}
