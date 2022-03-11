/*!
 * e2-sat-editor/src/gui/settings.cpp
 *
 * @link https://github.com/ctlcltd/e2-sat-editor
 * @copyright e2 SAT Editor Team
 * @author Leonardo Laureti
 * @version 0.1
 * @license MIT License
 * @license GNU GPLv3 License
 */

#include <Qt>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGroupBox>
#include <QFormLayout>
#include <QToolBar>
#include <QPushButton>
#include <QLabel>
#include <QLineEdit>
#include <QCheckBox>
#include <QValidator>

#include "../commons.h"
#include "settings.h"

using namespace std;
using namespace e2se;

namespace e2se_gui_dialog
{

settings::settings(QWidget* mwid)
{
	debug("settings");

	this->dial = new QDialog(mwid);
//	dial->setMinimumSize(530, 420);
	dial->setWindowTitle("Settings");
	dial->setStyleSheet("QGroupBox { spacing: 0; padding: 20px 0 0 0; border: 0 } QGroupBox::title { margin: 0 10px; font: bold }");

	QGridLayout* dfrm = new QGridLayout(dial);
	QHBoxLayout* dhbox = new QHBoxLayout;
	QVBoxLayout* dvbox = new QVBoxLayout;
	this->dtwid = new QTabWidget;
	dtwid->connect(dtwid, &QTabWidget::currentChanged, [=](int index) { this->tabChanged(index); });
	this->_state_previ = -1;

	QPushButton* dtsave = new QPushButton;
	dtsave->setDefault(true);
	dtsave->setText("Save Settings");
	dtsave->connect(dtsave, &QPushButton::pressed, [=]() { dial->close(); });
	QPushButton* dtcancel = new QPushButton;
	dtcancel->setDefault(false);
	dtcancel->setText("Cancel");
	dtcancel->connect(dtcancel, &QPushButton::pressed, [=]() { dial->close(); });

	connections();
	preferences();
	advanced();

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

void settings::preferences()
{
	QWidget* dtpage = new QWidget;
	QHBoxLayout* dtcnt = new QHBoxLayout(dtpage);
	
	QFormLayout* dtform = new QFormLayout;

	//TODO FIX [macos] connections tab interferes with focus
	dtform->addRow(new QCheckBox("Suppress ask for confirmation messages (shown before deleting)"));
	dtform->addRow(new QCheckBox("Non-destructive edit (try to preserve origin rplists)"));

	dtcnt->setAlignment(Qt::AlignTop | Qt::AlignCenter);
	dtcnt->addLayout(dtform, 0);
	dtpage->setLayout(dtcnt);

	dtwid->addTab(dtpage, "Preferences");
}

void settings::connections()
{
	this->rppage = new WidgetWithBackdrop;
	QHBoxLayout* dtcnt = new QHBoxLayout(rppage);

	QVBoxLayout* dtvbox = new QVBoxLayout;
	this->rplist = new QListWidget;
	rplist->setStyleSheet("QListView::item { height: 44px; font: 16px } QListView QLineEdit { border: 1px solid palette(alternate-base) }");
	newProfile();
	rplist->connect(rplist, &QListWidget::doubleClicked, [=]() { this->renameProfile(false); });
	//TODO inplace edit dismissed only under certain conditions (? needs mouse tracking enabled)
	rplist->connect(rplist, &QAbstractItemView::viewportEntered, [=]() { this->renameProfile(true); });
	rppage->connect(rppage, &WidgetWithBackdrop::backdrop, [=]() { this->renameProfile(true); });

	QToolBar* dttbar = new QToolBar;
//	QWidget* dtspacer = new QWidget;
//	dtspacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	QPushButton* dtladd = new QPushButton;
	dtladd->setText("+");
	dtladd->setFlat(true);
	dtladd->connect(dtladd, &QPushButton::pressed, [=]() { this->newProfile(); });
	QPushButton* dtlremove = new QPushButton;
	dtlremove->setText("-");
	dtlremove->setFlat(true);
	dtlremove->connect(dtlremove, &QPushButton::pressed, [=]() { this->delProfile(); });

//	dttbar->addWidget(dtspacer);
	dttbar->addWidget(dtladd);
	dttbar->addWidget(dtlremove);

	dtvbox->setSpacing(0);
	dtvbox->addWidget(rplist);
	dtvbox->addWidget(dttbar);

	QFormLayout* dtform = new QFormLayout;

	QGroupBox* dtl0 = new QGroupBox("Connection");
	QFormLayout* dtf0 = new QFormLayout;
	QLineEdit* dtf0ia = new QLineEdit("192.168.0.2");
	dtf0->addRow("IP address", dtf0ia);
	QHBoxLayout* dtb0 = new QHBoxLayout;
	QValidator* dtv0 = new QIntValidator(1, 65535);
	dtf0->addRow("FTP port", dtb0);
	QLineEdit* dtf0fp = new QLineEdit("21");
	dtf0fp->setValidator(dtv0);
	dtf0fp->setMaxLength(5);
	dtb0->addWidget(dtf0fp);
	dtb0->addWidget(new QCheckBox("Use active FTP"));
	QLineEdit* dtf0hp = new QLineEdit("80");
	dtf0hp->setValidator(dtv0);
	dtf0hp->setMaxLength(5);
	dtf0->addRow("HTTP port", dtf0hp);

	//TODO FIX left align
	QGroupBox* dtl1 = new QGroupBox("Login");
	QFormLayout* dtf1 = new QFormLayout;
	dtf1->addRow("Username", new QLineEdit("root"));
	QLineEdit* dtf1lp = new QLineEdit;
	dtf1lp->setEchoMode(QLineEdit::Password);
	dtf1->addRow("Password", dtf1lp); // show/hide

	QGroupBox* dtl2 = new QGroupBox("Configuration");
	QFormLayout* dtf2 = new QFormLayout;
	QHBoxLayout* dtb20 = new QHBoxLayout;
	dtf2->addRow("Transponders", dtb20);
	dtb20->addWidget(new QLineEdit("/etc/tuxbox"));
	dtb20->addWidget(new QLabel("<small>(satellites.xml)</small>"));
	QHBoxLayout* dtb21 = new QHBoxLayout;
	dtf2->addRow("Services", dtb21);
	dtb21->addWidget(new QLineEdit("/etc/enigma2"));
	dtb21->addWidget(new QLabel("<small>(lamedb)</small>"));
	QHBoxLayout* dtb22 = new QHBoxLayout;
	dtf2->addRow("Bouquets", dtb22);
	dtb22->addWidget(new QLineEdit("/etc/enigma2"));
	dtb22->addWidget(new QLabel("<small>(*.bouquet, *.userbouquet)</small>"));

	QGroupBox* dtl3 = new QGroupBox("Commands");
	QFormLayout* dtf3 = new QFormLayout;
	dtf3->addRow("Custom webif reload URL address", new QLineEdit);
	dtf3->addRow("Fallback reload command", new QLineEdit);

	dtl0->setLayout(dtf0);
	dtl1->setLayout(dtf1);
	dtl2->setLayout(dtf2);
	dtl3->setLayout(dtf3);
	dtform->addRow(dtl0);
	dtform->addRow(dtl1);
	dtform->addRow(dtl2);
	dtform->addRow(dtl3);

	dtcnt->addLayout(dtvbox, 0);
	dtcnt->addLayout(dtform, 1);
	rppage->setLayout(dtcnt);

	dtwid->addTab(rppage, "Connections");
}

void settings::advanced()
{
	QWidget* dtpage = new QWidget;
	QVBoxLayout* dtcnt = new QVBoxLayout(dtpage);

	this->adtbl = new QTableWidget(3, 1);
	adtbl->setHidden(true);
	QTableWidgetItem* item = new QTableWidgetItem("dummy item");
	adtbl->setItem(0, 0, item);

	this->adntc = new QWidget;
	QGridLayout* dtntcg = new QGridLayout;
	QLabel* dtntcl = new QLabel("<b>Please be carefull!</b><br>Modifing these settings could break the program.");
	QPushButton* dtntcb = new QPushButton;
	dtntcb->setText("OK, I understood this.");
	dtntcb->connect(dtntcb, &QPushButton::pressed, [=]() { adntc->setHidden(true); adtbl->setVisible(true); });
	dtntcg->addWidget(dtntcl, 0, 0);
	dtntcg->addWidget(dtntcb, 1, 0);
	adntc->setLayout(dtntcg);

	dtcnt->addWidget(adntc, 0);
	dtcnt->addWidget(adtbl, 1);
	dtpage->setLayout(dtcnt);

	dtwid->addTab(dtpage, "Advanced");
}

void settings::newProfile()
{
	QListWidgetItem* item = new QListWidgetItem("Profile", rplist);
	renameProfile(true);
	item->setSelected(true);
	item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEditable | Qt::ItemIsEnabled);
	rplist->setCurrentItem(item);
	if (rplist->count() != 1) renameProfile(false);
}

void settings::delProfile()
{
	renameProfile(true);
	if (rplist->count() != 1) rplist->takeItem(rplist->currentRow());
}

void settings::renameProfile(bool dismiss)
{
	QListWidgetItem* item = rplist->currentItem();
	if (! dismiss && ! rplist->isPersistentEditorOpen(item))
	{
		rplist->openPersistentEditor(item);
		rppage->activateBackdrop();
	}
	else
	{
		rplist->closePersistentEditor(item);
		rppage->deactivateBackdrop();
	}
}

void settings::tabChanged(int index)
{
	if (this->_state_previ == -1)
	{
		this->_state_previ = index;
		return;
	}
	switch (this->_state_previ)
	{
		case 0:
			renameProfile(true);
		break;
		case 2:
			adntc->setVisible(true);
			adtbl->setHidden(true);
		break;
	}
	this->_state_previ = index;
}

}
