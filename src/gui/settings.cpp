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

namespace e2se_gui_dialog
{
settings::settings(QWidget* mwid)
{
	debug("gui settings", "settings()");

	this->dial = new QDialog(mwid);
//	dial->setMinimumSize(530, 420);
	dial->setWindowTitle("Settings");
	dial->setStyleSheet("QGroupBox { spacing: 0; padding: 20px 0 0 0; border: 0 } QGroupBox::title { margin: 0 10px; font: bold }");

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

	connections();
	preferences();
	todo();

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

	//TODO FIX connections tab interferes with focus
	dtform->addRow(new QCheckBox("Suppress ask for confirmation messages (shown before deleting)"));
	dtform->addRow(new QCheckBox("Non-destructive edit (try to preserve origin lists)"));

	dtcnt->setAlignment(Qt::AlignTop | Qt::AlignCenter);
	dtcnt->addLayout(dtform, 0);
	dtpage->setLayout(dtcnt);

	dtwid->addTab(dtpage, "Preferences");
}

void settings::connections()
{
	WidgetWithBackdrop* dtpage = new WidgetWithBackdrop;
	QHBoxLayout* dtcnt = new QHBoxLayout(dtpage);

	QVBoxLayout* dtvbox = new QVBoxLayout;
	QListWidget* dtlist = new QListWidget;
	dtlist->setStyleSheet("QListView::item { height: 44px; font: 16px } QListView QLineEdit { border: 1px solid palette(alternate-base) }");
	newProfile(dtlist, dtpage);
	dtlist->connect(dtlist, &QListWidget::doubleClicked, [=]() { this->renameProfile(dtlist, false, dtpage); });
	//TODO inplace edit dismissed only under certain conditions (? needs mouse tracking enabled)
	dtlist->connect(dtlist, &QAbstractItemView::viewportEntered, [=]() { this->renameProfile(dtlist, true, dtpage); });
	dtpage->connect(dtpage, &WidgetWithBackdrop::backdrop, [=]() { this->renameProfile(dtlist, true, dtpage); });

	QToolBar* dttbar = new QToolBar;
//	QWidget* dtspacer = new QWidget;
//	dtspacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	QPushButton* dtladd = new QPushButton;
	dtladd->setText("+");
	dtladd->setFlat(true);
	dtladd->connect(dtladd, &QPushButton::pressed, [=]() { this->newProfile(dtlist, dtpage); });
	QPushButton* dtlremove = new QPushButton;
	dtlremove->setText("-");
	dtlremove->setFlat(true);
	dtlremove->connect(dtlremove, &QPushButton::pressed, [=]() { this->delProfile(dtlist, dtpage); });

//	dttbar->addWidget(dtspacer);
	dttbar->addWidget(dtladd);
	dttbar->addWidget(dtlremove);

	dtvbox->setSpacing(0);
	dtvbox->addWidget(dtlist);
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
	dtpage->setLayout(dtcnt);

	dtwid->addTab(dtpage, "Connections");
}

void settings::todo()
{
	QLabel* ttodo = new QLabel;
	ttodo->setText("App TODO");
	ttodo->setAlignment(Qt::AlignCenter);

	dtwid->addTab(ttodo, "TODO");
}

void settings::newProfile(QListWidget* list, WidgetWithBackdrop* cnt)
{
	QListWidgetItem* item = new QListWidgetItem("Profile", list);
	renameProfile(list, true, cnt);
	item->setSelected(true);
	item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEditable | Qt::ItemIsEnabled);
	list->setCurrentItem(item);
	if (list->count() != 1) renameProfile(list, false, cnt);
}

void settings::delProfile(QListWidget* list, WidgetWithBackdrop* cnt)
{
	renameProfile(list, true, cnt);
	if (list->count() != 1) list->takeItem(list->currentRow());
}

void settings::renameProfile(QListWidget* list, bool dismiss, WidgetWithBackdrop* cnt)
{
	QListWidgetItem* item = list->currentItem();
	if (! dismiss && ! list->isPersistentEditorOpen(item))
	{
		list->openPersistentEditor(item);
		cnt->activateBackdrop();
	}
	else
	{
		list->closePersistentEditor(item);
		cnt->deactivateBackdrop();
	}
}
}
