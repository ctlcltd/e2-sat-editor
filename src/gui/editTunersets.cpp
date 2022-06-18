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
		case e2db::YTYPE::sat:
			tfrm->setTitle("Satellites");
		break;
		case e2db::YTYPE::terrestrial:
		case e2db::YTYPE::cable:
		case e2db::YTYPE::atsc:
			tfrm->setTitle("Package");
		break;
		default:
			error("editTunersets()", "Error", "Not supported yet.");
	}
	lfrm->setTitle("Transponders");

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
	widget->setLayout(afrm);
}

void editTunersets::layout()
{
	debug("layout()");

	QStringList ths, lhs;
	switch (yx)
	{
		case e2db::YTYPE::sat:
			ths = QStringList ({NULL, "Name", "Position"});
			lhs = QStringList ({NULL, "TRID", "Frequency", "Polarization", "Symbol Rate", "FEC", "System", "Modulation"});
		break;
		case e2db::YTYPE::terrestrial:
			ths = QStringList ({NULL, "Name", "Country"});
			lhs = QStringList ({NULL, "TRID", "Frequency", "Modulation", "Bandwidth"});
		break;
		case e2db::YTYPE::cable:
			ths = QStringList ({NULL, "Name", "Country"});
			lhs = QStringList ({NULL, "TRID", "Frequency", "Modulation", "Symbol Rate", "FEC"});
		break;
		case e2db::YTYPE::atsc:
			ths = QStringList ({NULL, "Name"});
			lhs = QStringList ({NULL, "TRID", "Frequency", "Modulation"});
		break;
	}

	this->tree = new QTreeWidget;
	tree->setUniformRowHeights(true);
	tree->setMinimumWidth(240);

	QTreeWidgetItem* tree_thead = new QTreeWidgetItem(ths);
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

	QTreeWidgetItem* list_thead = new QTreeWidgetItem(lhs);
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

	string iname = "tns:";
	switch (yx)
	{
		case e2db::YTYPE::sat: iname += 's'; break;
		case e2db::YTYPE::terrestrial: iname += 't'; break;
		case e2db::YTYPE::cable: iname += 'c'; break;
		case e2db::YTYPE::atsc: iname += 'a'; break;
	}

	e2db::tunersets tv = dbih->tuners[yx];

	for (auto & x : dbih->index[iname])
	{
		e2db::tunersets_table tn = tv.tables[x.second];
		QTreeWidgetItem* item;
		QString idx = QString::fromStdString(to_string(x.first));
		QString tnid = QString::fromStdString(x.second);
		QString name = QString::fromStdString(tn.name);

		if (yx == e2db::YTYPE::sat)
		{
			char cposdeg[6];
			// %3d.%1d%C
			std::sprintf(cposdeg, "%.1f", float (std::abs (tn.pos)) / 10);
			string ppos = (string (cposdeg) + (tn.pos > 0 ? 'E' : 'W'));

			QString pos = QString::fromStdString(ppos);
			item = new QTreeWidgetItem({idx, name, pos});
		}
		else if (yx == e2db::YTYPE::terrestrial || yx == e2db::YTYPE::cable)
		{
			QString country = QString::fromStdString(tn.country);
			item = new QTreeWidgetItem({idx, name, country});
		}
		else if (yx == e2db::YTYPE::atsc)
		{
			item = new QTreeWidgetItem({idx, name});
		}
		else
		{
			continue;
		}

		item->setData(0, Qt::UserRole, tnid);
		tree->addTopLevelItem(item);
	}
}

void editTunersets::populate()
{
	string curr;
	QTreeWidgetItem* selected;
	selected = tree->currentItem();

	if (selected == NULL)
		selected = tree->topLevelItem(0);
	if (selected != NULL)
	{
		selected->setExpanded(true);
		curr = selected->data(0, Qt::UserRole).toString().toStdString();
	}

	debug("populate()", "curr", curr);

	e2db::tunersets tvs = dbih->tuners[yx];

	if (! tvs.tables.count(curr))
		error("populate()", "curr", curr);

	e2db::tunersets_table tns = tvs.tables[curr];

	list->header()->setSortIndicatorShown(false);
	list->header()->setSectionsClickable(false);
	list->clear();

	int i = 0;

	for (auto & tp : dbih->index[curr])
	{
		e2db::tunersets_transponder txp = tns.transponders[tp.second];
		char ci[7];
		std::sprintf(ci, "%06d", i++);
		QString x = QString::fromStdString(ci);

		QTreeWidgetItem* item;
		QString trid = QString::fromStdString(tp.second);
		QString freq = QString::fromStdString(to_string(txp.freq));

		if (yx == e2db::YTYPE::sat)
		{
			QString sr = QString::fromStdString(to_string(txp.sr));
			QString pol = QString::fromStdString(e2db::SAT_POL[txp.pol]);
			QString fec = QString::fromStdString(to_string(txp.fec));
			QString sys = QString::fromStdString(e2db::SAT_SYS[txp.sys]);
			QString mod = QString::fromStdString(to_string(txp.mod));
			item = new QTreeWidgetItem({x, trid, freq, sr, pol, fec, sys, mod});
		}
		else if (yx == e2db::YTYPE::terrestrial)
		{
			QString tmod = QString::fromStdString(to_string(txp.tmod));
			QString band = QString::fromStdString(to_string(txp.band));
			item = new QTreeWidgetItem({x, trid, freq, tmod, band});
		}
		else if (yx == e2db::YTYPE::cable)
		{
			QString cmod = QString::fromStdString(to_string(txp.cmod));
			QString sr = QString::fromStdString(to_string(txp.sr));
			QString cfec = QString::fromStdString(to_string(txp.cfec));
			item = new QTreeWidgetItem({x, trid, freq, cmod, sr, cfec});
		}
		else if (yx == e2db::YTYPE::atsc)
		{
			QString amod = QString::fromStdString(to_string(txp.amod));
			item = new QTreeWidgetItem({x, trid, freq, amod});
		}
		else
		{
			continue;
		}

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
