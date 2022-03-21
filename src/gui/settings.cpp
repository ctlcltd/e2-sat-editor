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
#include <QRegularExpression>
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
#include <QHeaderView>

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
	dial->setWindowTitle(tr("Settings"));
	dial->setStyleSheet("QGroupBox { spacing: 0; padding: 20px 0 0 0; border: 0 } QGroupBox::title { margin: 0 10px; font: bold }");
	dial->connect(dial, &QDialog::finished, [=]() { delete dial; delete this; });

	this->sets = new QSettings;

	QGridLayout* dfrm = new QGridLayout(dial);
	QHBoxLayout* dhbox = new QHBoxLayout;
	QVBoxLayout* dvbox = new QVBoxLayout;
	this->dtwid = new QTabWidget;
	dtwid->connect(dtwid, &QTabWidget::currentChanged, [=](int index) { this->tabChanged(index); });
	this->_state_prev = -1;

	QPushButton* dtsave = new QPushButton;
	dtsave->setDefault(true);
	dtsave->setText(tr("Save Settings"));
	dtsave->connect(dtsave, &QPushButton::pressed, [=]() { this->save(); });
	QPushButton* dtcancel = new QPushButton;
	dtcancel->setDefault(false);
	dtcancel->setText(tr("Cancel"));
	dtcancel->connect(dtcancel, &QPushButton::pressed, [=]() { dial->close(); });

	connections();
	preferences();
	advanced();
	retrieve();

	dfrm->setColumnStretch(0, 1);
	dfrm->setRowStretch(0, 1);
	dhbox->setAlignment(Qt::AlignRight);

	dvbox->addWidget(dtwid);
	dhbox->addWidget(dtcancel);
	dhbox->addWidget(dtsave);
	dvbox->addLayout(dhbox);

	dfrm->addLayout(dvbox, 0, 0);
	dfrm->setSizeConstraint(QGridLayout::SetFixedSize);
	dial->setLayout(dfrm);
	dial->exec();
}

//TODO FIX [macos] connections tab interferes with focus
void settings::preferences()
{
	QWidget* dtpage = new QWidget;
	QHBoxLayout* dtcnt = new QHBoxLayout(dtpage);
	
	QFormLayout* dtform = new QFormLayout;
	
	QCheckBox* dtfg0 = new QCheckBox(tr("Suppress ask for confirmation messages (shown before deleting)"));
	dtfg0->setProperty("pref", "askConfirmation");
	prefs[PREF_SECTIONS::Preferences].emplace_back(dtfg0);
	dtform->addRow(dtfg0);

	QCheckBox* dtfg1 = new QCheckBox(tr("Non-destructive edit (try to preserve origin rplists)"));
	dtfg1->setProperty("pref", "nonDestructiveEdit");
	prefs[PREF_SECTIONS::Preferences].emplace_back(dtfg1);
	dtform->addRow(dtfg1);

	QCheckBox* dtfg2 = new QCheckBox(tr("Visually fix for unwanted unicode characters (less performant)"));
	dtfg2->setProperty("pref", "fixUnicodeChars");
	prefs[PREF_SECTIONS::Preferences].emplace_back(dtfg2);
	dtform->addRow(dtfg2);

	dtcnt->setAlignment(Qt::AlignTop | Qt::AlignCenter);
	dtcnt->addLayout(dtform, 0);
	dtpage->setLayout(dtcnt);

	dtwid->addTab(dtpage, tr("Preferences"));
}

void settings::connections()
{
	this->rppage = new WidgetWithBackdrop;
	QHBoxLayout* dtcnt = new QHBoxLayout(rppage);

	QVBoxLayout* dtvbox = new QVBoxLayout;
	this->rplist = new QListWidget;
	rplist->setEditTriggers(QAbstractItemView::EditKeyPressed | QAbstractItemView::DoubleClicked);
	rplist->setStyleSheet("QListView::item { height: 44px; font: 16px } QListView QLineEdit { border: 1px solid palette(alternate-base) }");
	rplist->connect(rplist, &QListWidget::currentItemChanged, [=](QListWidgetItem* current, QListWidgetItem* previous) { this->currentProfileChanged(current, previous); });
	rplist->connect(rplist, &QListWidget::currentTextChanged, [=](QString text) { this->profileNameChanged(text); });
	//TODO inplace edit dismissed only under certain conditions
	rplist->connect(rplist, &QAbstractItemView::viewportEntered, [=]() { this->renameProfile(false); });
	rppage->connect(rppage, &WidgetWithBackdrop::backdrop, [=]() { this->renameProfile(false); });

	QToolBar* dttbar = new QToolBar;
//	QWidget* dtspacer = new QWidget;
//	dtspacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	QPushButton* dtladd = new QPushButton;
	dtladd->setText("+");
	dtladd->setFlat(true);
	dtladd->connect(dtladd, &QPushButton::pressed, [=]() { this->addProfile(); });
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

	QGroupBox* dtl0 = new QGroupBox(tr("Connection"));
	QFormLayout* dtf0 = new QFormLayout;

	QLineEdit* dtf0ia = new QLineEdit("192.168.0.2");
	dtf0ia->setProperty("pref", "ipAddress");
	prefs[PREF_SECTIONS::Connections].emplace_back(dtf0ia);
	dtf0->addRow(tr("IP address"), dtf0ia);

	QHBoxLayout* dtb0 = new QHBoxLayout;
	dtf0->addRow(tr("FTP port"), dtb0);

	QLineEdit* dtf0fp = new QLineEdit("21");
	dtf0fp->setProperty("pref", "ftpPort");
	dtf0fp->setValidator(new QIntValidator(1, 65535));
	dtf0fp->setMaxLength(5);

	QCheckBox* dtf0fa = new QCheckBox(tr("Use active FTP"));
	dtf0fa->setProperty("pref", "ftpActive");
	prefs[PREF_SECTIONS::Connections].emplace_back(dtf0fp);
	prefs[PREF_SECTIONS::Connections].emplace_back(dtf0fa);
	dtb0->addWidget(dtf0fp);
	dtb0->addWidget(dtf0fa);

	QLineEdit* dtf0hp = new QLineEdit("80");
	dtf0hp->setProperty("pref", "httpPort");
	dtf0hp->setValidator(new QIntValidator(1, 65535));
	dtf0hp->setMaxLength(5);
	prefs[PREF_SECTIONS::Connections].emplace_back(dtf0hp);
	dtf0->addRow(tr("HTTP port"), dtf0hp);

	//TODO FIX left align
	QGroupBox* dtl1 = new QGroupBox(tr("Login"));
	QFormLayout* dtf1 = new QFormLayout;

	QLineEdit* dtf1lu = new QLineEdit("root");
	dtf1lu->setProperty("pref", "username");
	prefs[PREF_SECTIONS::Connections].emplace_back(dtf1lu);
	dtf1->addRow(tr("Username"), dtf1lu);

	QLineEdit* dtf1lp = new QLineEdit;
	dtf1lp->setProperty("pref", "password");
	dtf1lp->setEchoMode(QLineEdit::Password);
	prefs[PREF_SECTIONS::Connections].emplace_back(dtf1lp);
	dtf1->addRow(tr("Password"), dtf1lp); // show/hide

	QGroupBox* dtl2 = new QGroupBox("Configuration");
	QFormLayout* dtf2 = new QFormLayout;

	QHBoxLayout* dtb20 = new QHBoxLayout;
	dtf2->addRow(tr("Transponders"), dtb20);
	QLineEdit* dtf2pt = new QLineEdit("/etc/tuxbox");
	dtf2pt->setProperty("pref", "pathTransponders");
	prefs[PREF_SECTIONS::Connections].emplace_back(dtf2pt);
	dtb20->addWidget(dtf2pt);
	dtb20->addWidget(new QLabel("<small>(satellites.xml)</small>"));

	QHBoxLayout* dtb21 = new QHBoxLayout;
	dtf2->addRow(tr("Services"), dtb21);
	QLineEdit* dtf2ps = new QLineEdit("/etc/enigma2");
	dtf2ps->setProperty("pref", "pathServices");
	prefs[PREF_SECTIONS::Connections].emplace_back(dtf2ps);
	dtb21->addWidget(dtf2ps);
	dtb21->addWidget(new QLabel("<small>(lamedb)</small>"));

	QHBoxLayout* dtb22 = new QHBoxLayout;
	dtf2->addRow(tr("Bouquets"), dtb22);
	QLineEdit* dtf2pb = new QLineEdit("/etc/enigma2");
	dtf2pb->setProperty("pref", "pathBouquets");
	prefs[PREF_SECTIONS::Connections].emplace_back(dtf2pb);
	dtb22->addWidget(dtf2pb);
	dtb22->addWidget(new QLabel("<small>(*.bouquet, *.userbouquet)</small>"));

	QGroupBox* dtl3 = new QGroupBox(tr("Commands"));
	QFormLayout* dtf3 = new QFormLayout;

	QLineEdit* dtf3ca = new QLineEdit;
	dtf3ca->setProperty("pref", "customWebifReloadUrl");
	prefs[PREF_SECTIONS::Connections].emplace_back(dtf3ca);
	dtf3->addRow(tr("Custom webif reload URL address"), dtf3ca);

	QLineEdit* dtf3cc = new QLineEdit;
	dtf3cc->setProperty("pref", "customFallbackReloadCmd");
	prefs[PREF_SECTIONS::Connections].emplace_back(dtf3cc);
	dtf3->addRow(tr("Fallback reload command"), dtf3cc);

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

	dtwid->addTab(rppage, tr("Connections"));
}

void settings::advanced()
{
	QWidget* dtpage = new QWidget;
	QVBoxLayout* dtcnt = new QVBoxLayout(dtpage);

	this->adtbl = new QTableWidget(0, 2);
	adtbl->setHidden(true);
	adtbl->setHorizontalHeaderLabels({"ID", "Value"});
	adtbl->horizontalHeader()->setSectionsClickable(false);
	adtbl->verticalHeader()->setVisible(false);

	this->adntc = new QWidget;
	QGridLayout* dtntcg = new QGridLayout;
	QLabel* dtntcl = new QLabel(tr("<b>Please be carefull!</b><br>Modifing these settings could break the program."));
	QPushButton* dtntcb = new QPushButton;
	dtntcb->setText(tr("OK, I understood this."));
	dtntcb->connect(dtntcb, &QPushButton::pressed, [=]() { adntc->setHidden(true); adtbl->setVisible(true); retrieve(adtbl); });
	dtntcg->addWidget(dtntcl, 0, 0);
	dtntcg->addWidget(dtntcb, 1, 0);
	adntc->setLayout(dtntcg);

	dtcnt->addWidget(adntc, 0);
	dtcnt->addWidget(adtbl, 1);
	dtpage->setLayout(dtcnt);

	dtwid->addTab(dtpage, tr("Advanced"));
}

QListWidgetItem* settings::addProfile(int id)
{
	if (! id)
	{
		id = sets->beginReadArray("profile");
		id++;
		sets->endArray();
	}
	debug("settings", "addProfile()", "id", to_string(id));

	QListWidgetItem* item = new QListWidgetItem(tr("Profile"), rplist);
	item->setText(item->text() + " " + QString::fromStdString(to_string(id)));
	item->setData(Qt::UserRole, id);
	renameProfile(false);
	item->setSelected(true);
	item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEditable | Qt::ItemIsEnabled);
	rplist->setCurrentItem(item);
	if (rplist->count() != 1)
		renameProfile();
	return item;
}

void settings::delProfile()
{
	debug("settings", "delProfile()");

	QListWidgetItem* curr = rplist->currentItem();
	int id = curr->data(Qt::UserRole).toInt();
	tmpps.erase(id);

	renameProfile(false);
	if (rplist->count() != 1)
		rplist->takeItem(rplist->currentRow());
}

void settings::renameProfile(bool enabled)
{
	QListWidgetItem* curr = rplist->currentItem();
	if (enabled && ! rplist->isPersistentEditorOpen(curr))
	{
		rplist->openPersistentEditor(curr);
		rppage->activateBackdrop();
	}
	else
	{
		rplist->closePersistentEditor(curr);
		rppage->deactivateBackdrop();
	}
}

void settings::profileNameChanged(QString text)
{
	debug("settings", "profileNameChanged()");

	QListWidgetItem* curr = rplist->currentItem();
	int id = curr->data(Qt::UserRole).toInt();
	tmpps[id]["profileName"] = text;
}

void settings::currentProfileChanged(QListWidgetItem* current, QListWidgetItem* previous)
{
	debug("settings", "currentProfileChanged()");

	if (previous != nullptr)
	{
		int id = previous->data(Qt::UserRole).toInt();
		for (auto & item : prefs[PREF_SECTIONS::Connections])
		{
			QString pref = item->property("pref").toString();
			if (QLineEdit* field = qobject_cast<QLineEdit*>(item))
				tmpps[id][pref] = field->text();
			else if (QCheckBox* field = qobject_cast<QCheckBox*>(item))
				tmpps[id][pref] = field->isChecked();
		}
		this->retrieve(current);
	}
}

void settings::tabChanged(int index)
{
	debug("settings", "tabChanged()", "index", to_string(index));

	if (this->_state_prev == -1)
	{
		this->_state_prev = index;
		return;
	}
	switch (this->_state_prev)
	{
		case PREF_SECTIONS::Connections:
			renameProfile(false);
		break;
		case PREF_SECTIONS::Advanced:
			adntc->setVisible(true);
			adtbl->setHidden(true);
		break;
	}
	this->_state_prev = index;
}

void settings::store()
{
	debug("settings", "store()");

	sets->beginWriteArray("profile");
	for (auto & item : tmpps)
	{
		sets->setArrayIndex(item.first);
		for (auto & field : item.second)
			sets->setValue(field.first, field.second);
	}
	sets->endArray();

	for (auto & item : prefs[PREF_SECTIONS::Preferences])
	{
		QString pref = item->property("pref").toString();
		if (QCheckBox* field = qobject_cast<QCheckBox*>(item))
			sets->setValue(pref, field->isChecked());
		else if (QLineEdit* field = qobject_cast<QLineEdit*>(item))
			sets->setValue(pref, field->text());
	}
}

void settings::store(QTableWidget* adtbl)
{
	debug("settings", "store()", "", to_string(PREF_SECTIONS::Advanced));

	for (int i = 0; i < adtbl->rowCount(); i++)
	{
		QString pref = adtbl->item(i, 0)->text().replace(".", "/");
		QString field = adtbl->item(i, 1)->text();
		if (field.contains(QRegularExpression("false|true")))
			sets->setValue(pref, (field == "true" ? true : false));
		else
			sets->setValue(pref, field);
	}
}

void settings::retrieve()
{
	debug("settings", "retrieve()");

	int selected = sets->value("profile/selected").toInt();
	int size = sets->beginReadArray("profile");
	for (int i = 0; i < size; i++)
	{
		sets->setArrayIndex(i);
		QListWidgetItem* item = addProfile(i);
		item->setText(sets->value("profileName").toString());
		tmpps[i]["profileName"] = sets->value("profileName");

		for (auto & item : prefs[PREF_SECTIONS::Connections])
		{
			QString pref = item->property("pref").toString();
			if (QLineEdit* field = qobject_cast<QLineEdit*>(item))
			{
				tmpps[i][pref] = field->text();
				if (i == selected)
					field->setText(sets->value(pref).toString());
			}
			else if (QCheckBox* field = qobject_cast<QCheckBox*>(item))
			{
				tmpps[i][pref] = field->isChecked();
				if (i == selected)
					field->setChecked(sets->value(pref).toBool());
			}
		}
	}
	sets->endArray();

	for (auto & item : prefs[PREF_SECTIONS::Preferences])
	{
		QString pref = item->property("pref").toString();
		if (QCheckBox* field = qobject_cast<QCheckBox*>(item))
			field->setChecked(sets->value(pref).toBool());
		else if (QLineEdit* field = qobject_cast<QLineEdit*>(item))
			field->setText(sets->value(pref).toString());
	}
}

void settings::retrieve(QListWidgetItem* item)
{
	debug("settings", "retrieve()", "", to_string(PREF_SECTIONS::Connections));

	int id = item->data(Qt::UserRole).toInt();
	for (auto & item : prefs[PREF_SECTIONS::Connections])
	{
		QString pref = item->property("pref").toString();
		if (QLineEdit* field = qobject_cast<QLineEdit*>(item))
			field->setText(tmpps[id][pref].toString());
		else if (QCheckBox* field = qobject_cast<QCheckBox*>(item))
			field->setChecked(tmpps[id][pref].toBool());
	}
}

void settings::retrieve(QTableWidget* adtbl)
{
	debug("settings", "retrieve()", "", to_string(PREF_SECTIONS::Advanced));

	QStringList keys = sets->allKeys().filter(QRegularExpression("^(application|preference|profile)/"));
	QStringList::const_iterator iq;
	adtbl->setRowCount(keys.count());
	int i = 0;
	for (iq = keys.constBegin(); iq != keys.constEnd(); ++iq)
	{
		QTableWidgetItem* field = new QTableWidgetItem((*iq).toLocal8Bit().replace("/", "."));
		field->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
		adtbl->setItem(i, 0, field);
		adtbl->setItem(i, 1, new QTableWidgetItem(sets->value(*iq).toString()));
		i++;
	}
	adtbl->resizeColumnsToContents();
}

void settings::save()
{
	debug("settings", "save()");

	if (dtwid->currentIndex() == PREF_SECTIONS::Advanced)
		store(adtbl);
	else
		store();

	dial->close();
}

}
