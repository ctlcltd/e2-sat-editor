/*!
 * e2-sat-editor/src/gui/tunersets.cpp
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
#include <QVBoxLayout>
#include <QSplitter>
#include <QGroupBox>
#include <QHeaderView>

#include "tunersets.h"

using std::to_string;
using namespace e2se;

namespace e2se_gui
{

tunersets::tunersets(e2db* dbih, int ytype, e2se::logger::session* log)
{
	this->log = new logger(log, "tunersets");
	debug("tunersets()");

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
			tfrm->setTitle("Package"); //TODO rename
		break;
		default:
			error("tunersets()", "Error", "Not supported yet.");
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

void tunersets::layout()
{
	debug("layout()");

	QStringList ths, lhs;
	switch (yx)
	{
		case e2db::YTYPE::sat:
			ths = QStringList ({NULL, "Name", "Position"});
			lhs = QStringList ({NULL, "TRID", "Frequency", "Polarization", "Symbol Rate", "FEC", "System", "Modulation", "Inversion", "Pilot", "Roll offset"});
		break;
		case e2db::YTYPE::terrestrial:
			ths = QStringList ({NULL, "Name", "Country"});
			lhs = QStringList ({NULL, "TRID", "Frequency", "Constellation", "Bandwidth", "System", "Tx Mode", "HP FEC", "LP FEC", "Inversion", "Guard", "Hierarchy"});
		break;
		case e2db::YTYPE::cable:
			ths = QStringList ({NULL, "Name", "Country"});
			lhs = QStringList ({NULL, "TRID", "Frequency", "Modulation", "Symbol Rate", "FEC", "Inversion", "System"});
		break;
		case e2db::YTYPE::atsc:
			ths = QStringList ({NULL, "Name"});
			lhs = QStringList ({NULL, "TRID", "Frequency", "Modulation", "System"});
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
	if (this->sets->value("application/debug", true).toBool()) {
		list->setColumnWidth(1, 150);
	}
	else
	{
		list->setColumnHidden(1, true);
	}
	tree->setColumnWidth(2, 75);
	tree->setColumnWidth(3, 75);
	tree->setColumnWidth(4, 65);
	tree->setColumnWidth(5, 65);
	tree->setColumnWidth(6, 65);

	tree->connect(tree, &QTreeWidget::currentItemChanged, [=]() { this->treeItemChanged(); });
	list->header()->connect(list->header(), &QHeaderView::sectionClicked, [=](int column) { this->trickySortByColumn(column); });
}

void tunersets::treeItemChanged()
{
	debug("treeItemChanged()");

	list->clearSelection();
	list->scrollToTop();
	list->clear();

	populate();
}

void tunersets::load()
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

void tunersets::populate()
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
		QString idx = QString::fromStdString(to_string(tp.first));
		QString trid = QString::fromStdString(tp.second);
		QString freq = QString::fromStdString(to_string(txp.freq));

		if (yx == e2db::YTYPE::sat)
		{
			QString sr = QString::fromStdString(to_string(txp.sr));
			QString pol = QString::fromStdString(e2db::SAT_POL[txp.pol]);
			QString fec = QString::fromStdString(e2db::SAT_FEC[txp.fec]);
			QString sys = QString::fromStdString(e2db::SAT_SYS[txp.sys]);
			QString mod = QString::fromStdString(e2db::SAT_MOD[txp.mod]);
			QString inv = QString::fromStdString(e2db::SAT_INV[txp.inv]);
			QString pil = QString::fromStdString(e2db::SAT_PIL[txp.pil]);
			QString rol = QString::fromStdString(e2db::SAT_ROL[txp.rol]);
			item = new QTreeWidgetItem({x, trid, freq, sr, pol, fec, sys, mod, inv, pil, rol});
		}
		else if (yx == e2db::YTYPE::terrestrial)
		{
			QString tmod = QString::fromStdString(e2db::TER_MOD[txp.tmod]);
			QString band = QString::fromStdString(e2db::TER_BAND[txp.band]);
			QString sys = "DVB-T";
			QString tmx = QString::fromStdString(e2db::TER_TRXMODE[txp.tmx]);
			QString hpfec = QString::fromStdString(e2db::TER_HPFEC[txp.hpfec]);
			QString lpfec = QString::fromStdString(e2db::TER_LPFEC[txp.lpfec]);
			QString inv = QString::fromStdString(e2db::TER_INV[txp.inv]);
			QString guard = QString::fromStdString(e2db::TER_GUARD[txp.hier]);
			QString hier = QString::fromStdString(e2db::TER_HIER[txp.guard]);
			item = new QTreeWidgetItem({x, trid, freq, tmod, band, sys, tmx, hpfec, lpfec, inv, guard, hier});
		}
		else if (yx == e2db::YTYPE::cable)
		{
			QString cmod = QString::fromStdString(e2db::CAB_MOD[txp.cmod]);
			QString sr = QString::fromStdString(to_string(txp.sr));
			QString cfec = QString::fromStdString(e2db::CAB_IFEC[txp.cfec]);
			QString inv = QString::fromStdString(e2db::CAB_INV[txp.inv]);
			QString sys = "DVB-C";
			item = new QTreeWidgetItem({x, trid, freq, cmod, sr, cfec, inv, sys});
		}
		else if (yx == e2db::YTYPE::atsc)
		{
			QString amod = QString::fromStdString(to_string(txp.amod));
			QString sys = "ATSC";
			item = new QTreeWidgetItem({x, trid, freq, amod, sys});
		}
		else
		{
			continue;
		}
		item->setData(0, Qt::UserRole, idx);
		item->setData(1, Qt::UserRole, trid);

		list->addTopLevelItem(item);
	}

	list->header()->setSectionsClickable(true);

	// sorting default column 0|asc
	list->sortItems(0, Qt::AscendingOrder);
	list->header()->setSortIndicator(1, Qt::AscendingOrder);
}

void tunersets::trickySortByColumn(int column)
{
	debug("trickySortByColumn()", "column", column);

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
