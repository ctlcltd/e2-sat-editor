/*!
 * e2-sat-editor/src/gui/editTunersets.cpp
 *
 * @link https://github.com/ctlcltd/e2-sat-editor
 * @copyright e2 SAT Editor Team
 * @author Leonardo Laureti
 * @version 0.1
 * @license MIT License
 * @license GNU GPLv3 License
 */

#include <QGridLayout>
#include <QVBoxLayout>
#include <QSplitter>
#include <QGroupBox>
#include <QHeaderView>

#include "editTunersets.h"

using std::to_string;
using namespace e2se;

namespace e2se_gui
{

editTunersets::editTunersets(e2db* dbih, int ytype)
{
	this->log = new logger("editTunersets");
	debug("editTunersets()");

	this->dbih = dbih;
	this->yx = ytype;
	this->sets = new QSettings;

	QGridLayout* afrm = new QGridLayout();

	QSplitter* swid = new QSplitter;
	QGroupBox* tfrm = new QGroupBox;
	QGroupBox* lfrm = new QGroupBox;
	QVBoxLayout* tbox = new QVBoxLayout;
	QVBoxLayout* lbox = new QVBoxLayout;
	tfrm->setFlat(true);
	lfrm->setFlat(true);

	switch (yx)
	{
		case 0: // SAT
			tfrm->setTitle("Satellites");
			lfrm->setTitle("Transponders");
		break;
		default:
			error("editTunersets()", "Error", "Not supported yet.");
	}

	layout();
	load();

	tbox->addWidget(tree);
	lbox->addWidget(list);
	tfrm->setLayout(tbox);
	lfrm->setLayout(lbox);

	swid->addWidget(tfrm);
	swid->addWidget(lfrm);

	swid->setStretchFactor(0, 1);
	swid->setStretchFactor(1, 5);

	afrm->addWidget(swid, 0, 0);
	afrm->setSpacing(0);
	afrm->setContentsMargins(0, 0, 0, 0);

	this->widget = new QWidget;
	// widget->setStyleSheet("background: green");
	widget->setLayout(afrm);
}

void editTunersets::layout()
{
	debug("layout()");

	this->tree = new QTreeWidget;
	tree->setUniformRowHeights(true);
	tree->setMinimumWidth(240);

	QTreeWidgetItem* tree_thead = new QTreeWidgetItem({"", "Name", "Position"});
	tree->setHeaderItem(tree_thead);
	tree->setColumnHidden(0, true);
	tree->setColumnWidth(1, 200);
	tree->setColumnWidth(2, 75);
	tree->setStyleSheet("::item { padding: 6px auto }");

	this->list = new QTreeWidget;
	list->setUniformRowHeights(true);
	list->setRootIsDecorated(false);
	list->setSelectionBehavior(QAbstractItemView::SelectRows);
	list->setSelectionMode(QAbstractItemView::ExtendedSelection);
	list->setItemsExpandable(false);
	list->setExpandsOnDoubleClick(false);
	list->setStyleSheet("::item { padding: 6px auto }");

	QTreeWidgetItem* list_thead = new QTreeWidgetItem({"", "TRID", "Frequency", "Symbol Rate", "Polarization", "FEC Inner", "System", "Modulation"});
	list->setHeaderItem(list_thead);
	list->setColumnHidden(0, true);
	list->setColumnWidth(1, 150);
	tree->setColumnWidth(2, 75);
	tree->setColumnWidth(3, 75);
	tree->setColumnWidth(4, 65);
	tree->setColumnWidth(5, 65);
	tree->setColumnWidth(6, 65);

	tree->connect(tree, &QTreeWidget::currentItemChanged, [=]() { this->treeItemChanged(); });
	list->header()->connect(list->header(), &QHeaderView::sectionClicked, [=](int column) { this->trickySortByColumn(column); });
}

void editTunersets::treeItemChanged()
{
	debug("treeItemChanged()");

	list->clearSelection();
	list->scrollToTop();
	list->clear();

	populate();
}

void editTunersets::load()
{
	debug("load()");

	for (auto & x : dbih->tuners)
	{
		char cposdeg[5];
		std::sprintf(cposdeg, "%.1f", float (std::abs (x.first)) / 10);
		string ppos = (string (cposdeg) + (x.first > 0 ? 'E' : 'W'));

		QString idx = QString::fromStdString(to_string(x.first));
		QString pos = QString::fromStdString(ppos);
		QString name = QString::fromStdString(x.second.name);
		QTreeWidgetItem* item = new QTreeWidgetItem({idx, name, pos});
		item->setData(0, Qt::UserRole, idx);
		tree->addTopLevelItem(item);
	}
}

void editTunersets::populate()
{
	int curr = -1;
	QTreeWidgetItem* selected;
	selected = tree->currentItem();

	if (selected == NULL)
		selected = tree->topLevelItem(0);
	if (selected != NULL)
	{
		selected->setExpanded(true);
		curr = selected->data(0, Qt::UserRole).toInt();
	}

	debug("populate()", "curr", to_string(curr));

	if (! dbih->tuners.count(curr))
		error("populate()", "curr", to_string(curr));

	e2db::tuner_sets tn = dbih->tuners[curr];

	list->header()->setSortIndicatorShown(false);
	list->header()->setSectionsClickable(false);
	list->clear();

	int i = 0;

	for (auto & txp : tn.transponders)
	{
		char ci[7];
		std::sprintf(ci, "%06d", i++);
		QString x = QString::fromStdString(ci);

		QString trid = QString::fromStdString(txp.first);
		QString freq = QString::fromStdString(to_string(txp.second.freq));
		QString sr = QString::fromStdString(to_string(txp.second.sr));
		QString pol = QString::fromStdString(e2db::SAT_POL[txp.second.pol]);
		QString fec = QString::fromStdString(to_string(txp.second.fec));
		QString sys = QString::fromStdString(e2db::SAT_SYS[txp.second.sys]);
		QString mod = QString::fromStdString(to_string(txp.second.mod));

		QTreeWidgetItem* item = new QTreeWidgetItem({x, trid, freq, sr, pol, fec, sys, mod});
		list->addTopLevelItem(item);
	}

	list->header()->setSectionsClickable(true);

	// sorting default column 0|asc
	list->sortItems(0, Qt::AscendingOrder);
	list->header()->setSortIndicator(1, Qt::AscendingOrder);
}

void editTunersets::trickySortByColumn(int column)
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

}
