/*!
 * e2-sat-editor/src/gui/channelBookView.cpp
 *
 * @link https://github.com/ctlcltd/e2-sat-editor
 * @copyright e2 SAT Editor Team
 * @author Leonardo Laureti
 * @version 1.8.0
 * @license MIT License
 * @license GNU GPLv3 License
 */

#include <algorithm>

#include <QtGlobal>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QSplitter>
#include <QLabel>
#include <QClipboard>
#include <QMimeData>
#include <QHeaderView>
#ifdef Q_OS_WIN
#include <QStyleFactory>
#include <QScrollBar>
#endif

#include "platforms/platform.h"

#include "toolkit/ListProxyStyle.h"
#include "toolkit/TreeProxyStyle.h"
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

channelBookView::~channelBookView()
{
	debug("~channelBookView");

	delete this->widget;
	delete this->theme;
	delete this->log;
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

	//TODO TEST vertical expanding [Windows]
	tabv->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Ignored);

	TreeProxyStyle* tree_style = new TreeProxyStyle;
	TreeProxyStyle* list_style = new TreeProxyStyle;

#ifdef Q_OS_WIN
	if (! theme::isOverridden() && theme::isFluentWin())
	{
		QStyle* style = QStyleFactory::create("fusion");
		tree_style->setBaseStyle(style);
		list_style->setBaseStyle(style);
	}
#endif

	tree->setStyle(tree_style);
	list->setStyle(list_style);

#ifdef Q_OS_WIN
	if (! theme::isOverridden() && (theme::isFluentWin() || theme::absLuma() || ! theme::isDefault()))
	{
		QStyle* style;

		if (theme::isFluentWin())
			style = QStyleFactory::create("windows11");
		else
			style = QStyleFactory::create("fusion");

		tree->verticalScrollBar()->setStyle(style);
		tree->horizontalScrollBar()->setStyle(style);
		list->verticalScrollBar()->setStyle(style);
		list->horizontalScrollBar()->setStyle(style);
		list->header()->setStyle(style);
	}
#endif

	tree->setStyleSheet("QTreeWidget { border-style: none } QTreeWidget::item { padding: 2px 0 }");
	list->setStyleSheet("QTreeWidget { border-style: none } QTreeWidget::item { padding: 2px 0 }");

	//TODO improve ui [Windows]
#ifndef Q_OS_WIN
	tabv->setStyleSheet("QTabBar { width: 100% } QTabBar::tab { min-width: 48px; max-height: 0 }");
#else
	tabv->setStyleSheet("QTabBar { width: 100% } QTabBar::tab { min-width: 48px; max-height: 12px }");
#endif

#ifdef Q_OS_MAC
	if (theme::isDefault())
	{
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
	}
#endif

	tree->setHidden(true);
	tree->setHeaderHidden(true);
	tree->setMinimumWidth(180);
	tree->setUniformRowHeights(true);
	tree->setRootIsDecorated(false);
	tree->setIndentation(true);
	tree_style->setIndentation(12);

	list->setHidden(true);
	list->setUniformRowHeights(true);
	list->setRootIsDecorated(false);
	list->setSelectionBehavior(QTreeWidget::SelectRows);
	list->setSelectionMode(QTreeWidget::ExtendedSelection);
	list->setItemsExpandable(false);
	list->setExpandsOnDoubleClick(false);
	list->setRootIsDecorated(false);
	list->setIndentation(0);
	list_style->setIndentation(12, true);
	list_style->setFirstColumnIndent(1);

	QTreeWidgetItem* list_thead = new QTreeWidgetItem({NULL, tr("Index"), tr("Name"), tr("Type"), tr("Provider"), tr("Transponder"), tr("Position"), tr("System")});
	list->setHeaderItem(list_thead);
#if QT_VERSION >= QT_VERSION_CHECK(6, 6, 0)
	list->header()->setDefaultSectionSize(0);
#endif
	list->setColumnHidden(ITEM_ROW_ROLE::x, true);		// hidden index
	list->setColumnWidth(ITEM_ROW_ROLE::chnum, 60);		// (Channel Number) Index
	list->setColumnWidth(ITEM_ROW_ROLE::chname, 175);	// Name
	list->setColumnWidth(ITEM_ROW_ROLE::chtype, 70);	// Type
	list->setColumnWidth(ITEM_ROW_ROLE::chpname, 125);	// Provider
	list->setColumnWidth(ITEM_ROW_ROLE::chtxp, 165);	// Transponder
	list->setColumnWidth(ITEM_ROW_ROLE::chpos, 80);		// Position
	list->setColumnWidth(ITEM_ROW_ROLE::chsys, 75);		// System

	tabv->setHidden(true);
	tabv->setDocumentMode(true);
	tabv->setUsesScrollButtons(true);
	tabv->setExpanding(true);
	tabv->setDrawBase(true);
	tabv->setShape(QTabBar::RoundedWest);

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

	abox->addWidget(side);
	afrm->setLayout(abox);

	bbox->addWidget(tabv);
	bbox->addWidget(bswid);
	bfrm->setLayout(bbox);

	platform::osWidgetBlend(afrm);
	platform::osWidgetOpaque(bfrm);

	afrm->setMinimumWidth(150);
	afrm->setMaximumWidth(250);

	frm->addWidget(afrm, 0, 0);
	frm->addWidget(bfrm, 0, 1);

	frm->setColumnStretch(0, 1);
	frm->setColumnStretch(1, 5);
}

void channelBookView::sideLayout()
{
	this->side = new QListWidget;

	ListProxyStyle* side_style = new ListProxyStyle;

#ifdef Q_OS_WIN
	if (! theme::isOverridden() && theme::isFluentWin())
	{
		QStyle* style = QStyleFactory::create("fusion");
		side_style->setBaseStyle(style);
	}
#endif

	side->setStyle(side_style);

#ifdef Q_OS_WIN
	if (! theme::isOverridden() && (theme::isFluentWin() || theme::absLuma() || ! theme::isDefault()))
	{
		QStyle* style;

		if (theme::isFluentWin())
			style = QStyleFactory::create("windows11");
		else
			style = QStyleFactory::create("fusion");

		side->verticalScrollBar()->setStyle(style);
		side->horizontalScrollBar()->setStyle(style);
	}
#endif

	side->setStyleSheet("QListWidget { background: transparent; font-size: 15px; border-style: none } QListView::item { padding: 10px 0 }");

	side_style->setIndentation(10);

	side->addItems({tr("Services"), tr("Bouquets"), tr("Positions"), tr("Providers"), tr("Resolution"), tr("Encryption"), tr("A-Z")});

	side->connect(side, &QListWidget::currentRowChanged, [=](int index) { this->sideRowChanged(index); });
}

void channelBookView::load()
{
	debug("load");

	tabUpdateFlags(gui::init);

	side->setCurrentRow(0);
}

void channelBookView::reset()
{
	debug("reset");

	this->state.curr = "";
	this->state.vx = -1;
	this->state.sy = -1;
	this->state.sort = pair (-1, Qt::AscendingOrder);
	this->state.tab_pending = false;

	side->reset();
	tree->reset();
	tree->clear();

	list->reset();
	list->clear();
	list->header()->setSortIndicatorShown(false);
	list->header()->setSectionsClickable(false);
	list->header()->setSortIndicator(0, Qt::AscendingOrder);

	listFindClear();

	resetStatusBar();
}

void channelBookView::reload()
{
	debug("reload");

	side->reset();
	tree->clear();
	list->clear();

	listFindClear();

	resetStatusBar();

	side->setCurrentRow(0);

	if (tid != nullptr)
	{
		if (! this->widget->isVisible())
			this->state.tab_pending = true;
	}
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
	size_t pad_width = std::to_string(int (dbih->index["chs"].size())).size() + 1;

	for (auto & chi : this->index[curr])
	{
		if (dbih->db.services.count(chi.second))
		{
			e2db::service ch = dbih->db.services[chi.second];

			int atype = dbih->value_service_super_type(ch);
			bool disabled = this->state.sy != -1 && this->state.sy != atype;
			QString x = QString::number(i++).rightJustified(pad_width, '0');
			QString idx = QString::number(chi.first);
			QString chid = QString::fromStdString(chi.second);

			QStringList entry = dbih->entries.services[chi.second];

			if (ch.tsid != 0 || ch.onid != 0 || ch.dvbns != 0)
			{
				if (dbih->db.transponders.count(ch.txid))
				{
					e2db::transponder tx = dbih->db.transponders[ch.txid];
					QString txp = QString::fromStdString(dbih->value_transponder_combo(tx));
					entry.move(9, 10);
					entry.insert(9, txp);
				}
			}
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
			entry.remove(7);
			entry.remove(1, 5);
#else
			entry.removeAt(7);
			for (int i = 0; i != 5; i++)
				entry.removeAt(1);
#endif
			entry.prepend(idx);
			entry.prepend(x);

			QTreeWidgetItem* item = new QTreeWidgetItem(entry);
			item->setData(0, Qt::UserRole, chid);
			item->setData(1, Qt::UserRole, disabled);
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
			if (dbih->tnloc.count(pos))
			{
				string tnid = dbih->tnloc.at(pos);
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

			for (string & w : bs.userbouquets)
			{
				e2db::userbouquet ub = dbih->userbouquets[w];
				string bname = ub.bname;

				QString subindex = QString::fromStdString(bname);
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
		this->index.merge(dbih->get_userbouquets_index());
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

	if (tid != nullptr)
	{
		if (this->widget->isVisible())
		{
			stacker(index);

			updateFlags();
		}
		else
		{
			this->state.tab_pending = true;
		}
	}
	else
	{
		stacker(index);

		updateFlags();
	}
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
	QStringList content;

	for (auto & item : selected)
	{
		QString qchid = item->data(0, Qt::UserRole).toString();
		string chid = qchid.toStdString();

		QStringList data;
		data.reserve(TSV_TABS + 1);

		// start from chnum column [1]
		for (int i = ITEM_ROW_ROLE::chnum; i < list->columnCount(); i++)
		{
			QString qstr = item->data(i, Qt::DisplayRole).toString();
			data.append(qstr);
		}

		// Reference ID
		QString refid;
		string crefid = dbih->get_reference_id(chid);
		refid = QString::fromStdString(crefid);
		data.insert(2, refid); // insert refid column [2]

		content.append(data.join("\t")); // TSV
	}
	clipboard->setText(content.join("\n"));
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

void channelBookView::showListEditContextMenu(QPoint& pos)
{
	// debug("showListEditContextMenu");

	QList<QTreeWidgetItem*> selected = list->selectedItems();

	if (selected.empty())
		return;

	QMenu* list_edit = contextMenu();

	contextMenuAction(list_edit, tr("&Copy", "context-menu"), [=]() { this->listItemCopy(); }, tabGetFlag(gui::TabListCopy), QKeySequence::Copy);

	tabLastPopupFocusWidget(list, pos);
	platform::osMenuPopup(list_edit, list, pos);
}

void channelBookView::updateFlags()
{
	if (tid == nullptr)
		return;

	// debug("updateFlags");

	viewAbstract::updateFlags();

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

	tabSetFlag(gui::TabListFind, false);
	tabSetFlag(gui::TabListFindNext, false);
	tabSetFlag(gui::TabListFindPrev, false);
	tabSetFlag(gui::TabListFindAll, false);
	tabSetFlag(gui::TabTreeFind, false);
	tabSetFlag(gui::TabTreeFindNext, false);

	tabSetFlag(gui::TunersetsSat, true);
	tabSetFlag(gui::TunersetsTerrestrial, true);
	tabSetFlag(gui::TunersetsCable, true);
	tabSetFlag(gui::TunersetsAtsc, true);

	tabUpdateFlags();
}

//TODO tree item selection position
void channelBookView::update()
{
	debug("update");

	if (this->state.tab_pending)
	{
		listFindClear();

		int vv = side->currentRow();

		int column = list->sortColumn();
		Qt::SortOrder order = list->header()->sortIndicatorOrder();

		tree->clear();
		tree->reset();
		list->reset();

		stacker(vv);

		if (column != 0)
		{
			list->header()->setSortIndicator(column, order);
			sortByColumn(column);
		}

		this->state.tab_pending = false;
	}
}

void channelBookView::updateFromTab()
{
	debug("updateFromTab");

	this->state.tab_pending = true;
}

}
