/*!
 * e2-sat-editor/src/gui_qt6.cpp
 * 
 * @author Leonardo Laureti
 * @version 0.1
 * @license MIT License
 */

#include <iostream>
#include <QApplication>
#include <QWidget>
#include <QScreen>
#include <QGridLayout>
#include <QGroupBox>
#include <QTreeWidget>
#include <QToolBar>

int gui_qt6(int argc, char* argv[])
{
	QApplication root = QApplication(argc, argv);

	QScreen* screen = root.primaryScreen();
	QSize wsize = screen->availableSize();

	QWidget* mwid = new QWidget;
	mwid->setWindowTitle("enigma2 channel editor");
	mwid->resize(wsize);

	QGridLayout* frm = new QGridLayout(mwid);

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

	QTreeWidgetItem* header_item = new QTreeWidgetItem({"Index", "Name", "CHID", "TXID", "Type", "Provider", "Frequency", "Polarization", "Symbol Rate", "FEC", "SAT", "System", "DATA"});
	list_tree->setHeaderItem(header_item);

	QToolBar* top_toolbar = new QToolBar;
	top_toolbar->addAction("New");
	top_toolbar->addAction("Open");
	top_toolbar->addAction("Save");
	top_toolbar->addAction("Import");
	top_toolbar->addAction("Export");

	QToolBar* bottom_toolbar = new QToolBar;

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


	mwid->show();

	root.exec();

	return 0;
}
