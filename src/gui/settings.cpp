/*!
 * e2-sat-editor/src/gui/settings.cpp
 *
 * @author Leonardo Laureti
 * @version 0.1
 * @license MIT License
 */

#include <iostream>
#include <Qt>
#include <QWidget>
#include <QDialog>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QTabWidget>
#include "../commons.h"
#include "settings.h"
#include "todo.h"

using namespace std;

namespace e2se_gui
{
void settingsDialog(QWidget* mwid)
{
	debug("gui", "settings()");

	QDialog* dial = new QDialog(mwid);
	dial->setWindowTitle("Settings");
	dial->setMinimumSize(530, 420);

	QGridLayout* dfrm = new QGridLayout(dial);
	QVBoxLayout* dvbox = new QVBoxLayout;
	QHBoxLayout* dhbox = new QHBoxLayout;
	QTabWidget* dtwid = new QTabWidget;

	QPushButton* dtsave = new QPushButton;
	dtsave->setDefault(true);
	dtsave->setText("Save Settings");
	dtsave->connect(dtsave, &QPushButton::pressed, todo);
	QPushButton* dtcancel = new QPushButton;
	dtcancel->setDefault(false);
	dtcancel->setText("Cancel");
	dtcancel->connect(dtcancel, &QPushButton::pressed, [=]() { dial->close(); });

	QLabel* ttodo0 = new QLabel;
	ttodo0->setText("General TODO");
	ttodo0->setAlignment(Qt::AlignCenter);
	QLabel* ttodo1 = new QLabel;
	ttodo1->setText("Connections TODO");
	ttodo1->setAlignment(Qt::AlignCenter);
	QLabel* ttodo2 = new QLabel;
	ttodo2->setText("App TODO");
	ttodo2->setAlignment(Qt::AlignCenter);

	dtwid->addTab(ttodo0, "General");
	dtwid->addTab(ttodo1, "Connections");
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
}
