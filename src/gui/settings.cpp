/*!
 * e2-sat-editor/src/gui/settings.cpp
 *
 * @author Leonardo Laureti
 * @version 0.1
 * @license MIT License
 */

#include <Qt>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFormLayout>
#include <QListWidget>
#include <QToolBar>
#include <QPushButton>
#include <QLabel>
#include <QLineEdit>

#include "../commons.h"
#include "settings.h"

using namespace std;

namespace e2se_gui_settings
{
settings::settings(QWidget* mwid)
{
	debug("gui", "settingsDialog()");

	this->dial = new QDialog(mwid);
	dial->setWindowTitle("Settings");
	dial->setMinimumSize(530, 420);

	QGridLayout* dfrm = new QGridLayout(dial);
	QHBoxLayout* dhbox = new QHBoxLayout;
	QVBoxLayout* dvbox = new QVBoxLayout;
	this->dtwid = new QTabWidget;

	QPushButton* dtsave = new QPushButton;
	dtsave->setDefault(true);
	dtsave->setText("Save Settings");
	dtsave->connect(dtsave, &QPushButton::pressed, [=]() { dial->close(); });
	QPushButton* dtcancel = new QPushButton;
	dtcancel->setDefault(false);
	dtcancel->setText("Cancel");
	dtcancel->connect(dtcancel, &QPushButton::pressed, [=]() { dial->close(); });

	QLabel* ttodo0 = new QLabel;
	ttodo0->setText("General TODO");
	ttodo0->setAlignment(Qt::AlignCenter);

	QLabel* ttodo2 = new QLabel;
	ttodo2->setText("App TODO");
	ttodo2->setAlignment(Qt::AlignCenter);

	dtwid->addTab(ttodo0, "General");
	connections();
	dtwid->addTab(ttodo2, "TODO");

	dfrm->setColumnStretch(0, 1);
	dfrm->setRowStretch(0, 1);
	dhbox->setAlignment(Qt::AlignRight);

	dvbox->addWidget(dtwid);
	dhbox->addWidget(dtcancel);
	dhbox->addWidget(dtsave);
	dvbox->addLayout(dhbox);

	dfrm->addLayout(dvbox, 0, 0);
	dial->setLayout(dfrm);
	dial->exec();
}

void settings::connections()
{
	QWidget* dtpage = new QWidget;
	QHBoxLayout* dtcnt = new QHBoxLayout(dtpage);

	QVBoxLayout* dtvbox = new QVBoxLayout;
	QListWidget* dtlist = new QListWidget;
	new QListWidgetItem("Profile", dtlist);

	QToolBar* dttbar = new QToolBar;
	QWidget* dtspacer = new QWidget;
	dtspacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	QPushButton* dtladd = new QPushButton;
	dtladd->setText("+");
	dtladd->connect(dtladd, &QPushButton::pressed, [=]() { new QListWidgetItem("Profile", dtlist); });
	QPushButton* dtlremove = new QPushButton;
	dtlremove->setText("-");
	dtlremove->connect(dtlremove, &QPushButton::pressed, [=]() { dtlist->takeItem(dtlist->currentRow()); });

	dttbar->addWidget(dtspacer);
	dttbar->addWidget(dtladd);
	dttbar->addWidget(dtlremove);

	dtvbox->addWidget(dtlist);
	dtvbox->addWidget(dttbar);

	QFormLayout* dtform = new QFormLayout;
	QLabel* legend = new QLabel;
	legend->setText("Connections TODO");
	dtform->addRow(legend);
	dtform->addRow("Connection field", new QLineEdit);
	dtform->addRow("Connection field", new QLineEdit);
	dtform->addRow("Connection field", new QLineEdit);

	dtcnt->addLayout(dtvbox, 0);
	dtcnt->addLayout(dtform, 1);
	dtpage->setLayout(dtcnt);

	dtwid->addTab(dtpage, "Connections");
}
}
