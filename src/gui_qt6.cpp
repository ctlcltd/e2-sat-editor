/*!
 * e2-sat-editor/src/gui_qt6.cpp
 * 
 * @author Leonardo Laureti
 * @version 0.1
 * @license MIT License
 */

#include <iostream>
#include <algorithm>
#include <string>
#include <vector>
#include <map>
#include <Qt>
#include <QApplication>
#include <QWidget>
#include <QScreen>
#include <QGridLayout>
#include <QGroupBox>
#include <QTreeWidget>
#include <QToolBar>
#include <QMessageBox>
#include <QFileDialog>
#include <QString>
#include <cstdio>
#include <stdlib.h>
#include "commons.h"
#include "e2db.h"

using namespace std;


void todo()
{
	cout << "app TODO" << endl;
	QMessageBox dial = QMessageBox();
	dial.setText("app TODO");
	dial.exec();
}


class gui
{
	public:
		void root(int argc, char* argv[]);
		void main(QWidget& mwid);
		void tab(QWidget& ttab);
		void newFile();
		bool load(string filename = "");
		void populate(QTreeWidgetItem& item);
	private:
		QTreeWidget* bouquets_tree;
		QTreeWidget* list_tree;
		e2db_parser temp_parser;
};

void gui::root(int argc, char* argv[])
{
	cout << "gui qt6" << endl;

	QApplication mroot = QApplication(argc, argv);

	QScreen* screen = mroot.primaryScreen();
	QSize wsize = screen->availableSize();

	QWidget mwid = QWidget();
	mwid.setWindowTitle("enigma2 channel editor");
	mwid.resize(wsize);

	mroot.setStyleSheet("QGroupBox { spacing: 0; padding: 20px 0 0 0; border: 0 } QGroupBox::title { margin: 0 12px }");

	main(mwid);

	mwid.show();

	cout << "\tmroot.exec()" << endl;

	mroot.exec();
}

void gui::main(QWidget& mwid)
{
	cout << "gui main()" << endl;

	tab(mwid);

	//TEST
	char* ccwd = getenv("DYLD_FRAMEWORK_PATH");
	string cwd = string (ccwd);
	cwd = cwd.substr(0, cwd.length() - 10); // rstrip /src/Debug
	filesystem::path path = cwd + "/seeds./enigma_db";

	e2db_parser* parser = new e2db_parser;
	parser->load(filesystem::absolute(path));
	parser->debug();
	//TEST
}

void gui::tab(QWidget& ttab)
{
	cout << "gui tab()" << endl;

	QGridLayout* frm = new QGridLayout(&ttab);

	QHBoxLayout* top = new QHBoxLayout;
	QGridLayout* container = new QGridLayout;
	QHBoxLayout* bottom = new QHBoxLayout;

	QGroupBox* bouquets = new QGroupBox("Bouquets");
	QGroupBox* channels = new QGroupBox("Channels");

	QVBoxLayout* bouquets_box = new QVBoxLayout;
	QVBoxLayout* list_box = new QVBoxLayout;

	container->addWidget(bouquets, 1, 0);
	container->addWidget(channels, 1, 1);

	QTreeWidget* bouquets_tree = new QTreeWidget;
	QTreeWidget* list_tree = new QTreeWidget;
	bouquets_tree->setStyleSheet("::item { padding: 2px auto }");
	list_tree->setStyleSheet("::item { padding: 4px auto }");

	QTreeWidgetItem* bheader_item = bouquets_tree->headerItem();
	bheader_item->setText(0, "Bouquets");
	bheader_item->setSizeHint(0, QSize(0, 0));

	QTreeWidgetItem* lheader_item = new QTreeWidgetItem({"Index", "Name", "CHID", "TXID", "Type", "Provider", "Frequency", "Polarization", "Symbol Rate", "FEC", "SAT", "System", "DATA"});
	list_tree->setHeaderItem(lheader_item);

	QToolBar* top_toolbar = new QToolBar();
	top_toolbar->setStyleSheet("QToolButton { font: 20px }");

	QToolBar* bottom_toolbar = new QToolBar;
	bottom_toolbar->setStyleSheet("QToolButton { font: bold 16px }");

	QAction* anew = new QAction("New");

	top_toolbar->addAction(anew);
	top_toolbar->addAction("Open", todo);
	top_toolbar->addAction("Save", todo);
	top_toolbar->addAction("Import", todo);
	top_toolbar->addAction("Export", todo);

	top->addWidget(top_toolbar);
	bottom->addWidget(bottom_toolbar);

	bouquets_box->addWidget(bouquets_tree);
	bouquets->setLayout(bouquets_box);

	list_box->addWidget(list_tree);
	channels->setLayout(list_box);

	bouquets->setFlat(true);
	channels->setFlat(true);

	container->setColumnStretch(0, 0);
	container->setColumnStretch(1, 1);

	frm->addLayout(top, 0, 0);
	frm->addLayout(container, 1, 0);
	frm->addLayout(bottom, 2, 0);
}

void gui::newFile()
{
	cout << "gui newFile()" << endl;

	e2db_parser* temp_parser = new e2db_parser;

	bouquets_tree->scrollToItem(bouquets_tree->topLevelItem(0));
	bouquets_tree->clear();
	list_tree->scrollToItem(list_tree->topLevelItem(0));
	list_tree->clear();
}

bool gui::load(string filename)
{
	cout << "gui load() " << filename << endl;

	string dirname;

	if (filename != "")
	{
		dirname = filename;
	}
	else
	{
		QString qdirname = QFileDialog::getExistingDirectory(nullptr, "Select enigma2 db folder", "~", QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
		dirname = qdirname.toStdString();
	}

	if (dirname != "")
	{
		newFile();
		temp_parser.load(dirname);
	}
	else
	{
		return false;
	}

	QTreeWidgetItem* titem = new QTreeWidgetItem;
	QVariantMap tdata;
	tdata["bouquet_id"] = "all";
	titem->setData(0, 1, QVariant (tdata));
	titem->setText(0, "All channels");

	bouquets_tree->addTopLevelItem(titem);

	map<string, map<string, string>> chdata;
	map<string, QTreeWidgetItem> bgroups;

	/*for (auto & bname : chdata)
	{
		cout << "gui load() bname " << bname << endl;

		if (bname == "transponders" || bname == "channels")
		{
			continue;
		}

		string bdata = chdata[bname];
		string bgroup = bname; //py bname.split(":")[0]

		// cout << "gui load() bdata " << bdata << endl;

		if (bdata["name"] == "0")
		{
			QTreeWidgetItem* pgroup = new QTreeWidgetItem();
			QMap<QString, QVariant> tdata;
			tdata["bouquet_id"] = bgroup;
			pgroup->setData(0, Qt::UserRole, QVariant (tdata));
			pgroup->setText(0, bgroup); //py bgroup.upper()
			bouquets_tree->addTopLevelItem(pgroup);
			bouquets_tree->expandItem(pgroup);
			bgroups[bgroup] = pgroup;
		}
		else
		{
			QTreeWidgetItem* pgroup = bgroups[bgroup];
			QTreeWidgetItem* bitem = new QTreeWidgetItem(pgroup);
			QMap<QString, QVariant> tdata;
			tdata["bouquet_id"] = bgroup;
			bitem->setData(0, Qt::UserRole, QVariant (tdata));
			bitem->setText(0, bdata["name"]);
			bouquets_tree.addTopLevelItem(bitem);
		}
	}

	populate();*/

	return true;
}

void gui::populate(QTreeWidgetItem& item)
{
	QTreeWidgetItem* selected = bouquets_tree->currentItem();
	string cur_bouquet = "";

	QVariantMap tdata = selected->data(0, Qt::UserRole).toMap();
	QString qcur_bouquet = tdata["bouquet_id"].toString();

	if (! qcur_bouquet.isEmpty())
	{
		cur_bouquet = qcur_bouquet.toStdString();
	}

	cout << "gui populate()" << cur_bouquet << endl;
	
	map<string, map<string, string>> chdata;

	string cur_chlist = "channels";
	map<string, string> cur_chdata = chdata["channels"];

	if (cur_bouquet != "" && cur_bouquet != "all")
	{
		cur_chlist = cur_bouquet;
		cur_chdata = chdata[cur_bouquet]; //py chdata[cur_bouquet]["list"]
	}

	list_tree->scrollToItem(list_tree->topLevelItem(0));
	list_tree->clear();

	// cout << "gui populate() chdata[cur_chlist] " << chdata[cur_chlist] << endl;

	//py STYPES = {0: "Data", 1: "TV", 2: "Radio", 10: "Radio", 12: "TV", 17: "UHD", 22: "H.264", 25: "HD", 31: "UHD"}; //TODO move

	/*for (auto & cid : cur_chdata)
	{
		//py if (cid in chdata["channels"])
		{
			cdata = chdata["channels"][cid];

			if cur_chlist != "channels"
				idx = cur_chdata[cid];
			else
				idx = cdata["index"];

			pname = cdata["data"][0][1];
			//py stype = cdata["stype"] in STYPES and STYPES[cdata["stype"]] or "Data";
			txdata = txdata[cdata["txid"]];

			QTreeWidgetItem* item = new QTreeWidgetItem((str(idx), cdata["chname"], cid, cdata["txid"], stype, pname, txdata["freq"], txdata["pol"], txdata["sr"], txdata["fec"], txdata["pos"], txdata["sys"], str(cdata["data"])))
			item->setTextAlignment(12, Qt::AlignRight);
			list_tree->addTopLevelItem(item);
		}
		else
		{
			QTreeWidgetItem* item = new QTreeWidgetItem(("", cur_chdata[cid], cid));
			list_tree.addTopLevelItem(item);
		}
	}*/
}
