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
#include <QButtonGroup>
#include <QCheckBox>
#include <QComboBox>
#include <QValidator>
#include <QHeaderView>

#include "settings.h"
#include "theme.h"

using std::to_string;
using e2se_gui::theme;
using namespace e2se;

namespace e2se_gui_dialog
{

settings::settings(QWidget* cwid, e2se::logger::session* log)
{
	this->log = new logger(log, "settings");
	debug("settings()");

	this->sets = new QSettings;

	this->state.prev = -1;

	display(cwid);
}

void settings::display(QWidget* cwid)
{
	debug("display()");

	layout(cwid);

	retrieve();

	dial->exec();
}

void settings::layout(QWidget* cwid)
{
	debug("layout()");

	this->dial = new QDialog(cwid);
	dial->setWindowTitle(tr("Settings"));
	dial->setStyleSheet("QGroupBox { spacing: 0; padding: 20px 0 0 0; border: 0 } QGroupBox::title { margin: 0 10px; font: bold }");
	dial->connect(dial, &QDialog::finished, [=]() { delete dial; delete this; });

	QGridLayout* dfrm = new QGridLayout(dial);
	QHBoxLayout* dhbox = new QHBoxLayout;
	QVBoxLayout* dvbox = new QVBoxLayout;
	this->dtwid = new QTabWidget;
	dtwid->connect(dtwid, &QTabWidget::currentChanged, [=](int index) { this->tabChanged(index); });

	QPushButton* dtsave = new QPushButton;
	dtsave->setDefault(true);
	dtsave->setText(tr("Save Settings"));
	dtsave->connect(dtsave, &QPushButton::pressed, [=]() { this->save(); });
	QPushButton* dtcancel = new QPushButton;
	dtcancel->setDefault(false);
	dtcancel->setText(tr("Cancel"));
	dtcancel->connect(dtcancel, &QPushButton::pressed, [=]() { dial->close(); });

	connectionsLayout();
	preferencesLayout();
	advancedLayout();

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
}

void settings::preferencesLayout()
{
	QWidget* dtpage = new QWidget;
	QHBoxLayout* dtcnt = new QHBoxLayout(dtpage);
	dtpage->setStyleSheet("QGroupBox { font: bold }");
	
	QFormLayout* dtform = new QFormLayout;
	dtform->setSpacing(10);
	dtform->setFormAlignment(Qt::AlignLeft | Qt::AlignTop);

	QGroupBox* dtl0 = new QGroupBox(tr("General"));
	QFormLayout* dtf0 = new QFormLayout;
	dtf0->setSpacing(20);
	dtf0->setFormAlignment(Qt::AlignLeft);
	dtf0->setFieldGrowthPolicy(QFormLayout::FieldsStayAtSizeHint);

	QCheckBox* dtf0g0 = new QCheckBox(tr("Suppress ask for confirmation messages (shown before deleting)"));
	dtf0g0->setProperty("pref", "askConfirmation");
	prefs[PREF_SECTIONS::Preferences].emplace_back(dtf0g0);
	dtf0->addRow(dtf0g0);

	QCheckBox* dtf0g1 = new QCheckBox(tr("Non-destructive edit (try to preserve origin channel lists)"));
	dtf0g1->setProperty("pref", "nonDestructiveEdit");
	prefs[PREF_SECTIONS::Preferences].emplace_back(dtf0g1);
	dtf0->addRow(dtf0g1);

	QCheckBox* dtf0g2 = new QCheckBox(tr("Visually fix for unwanted unicode characters (less performant)"));
	dtf0g2->setProperty("pref", "fixUnicodeChars");
	prefs[PREF_SECTIONS::Preferences].emplace_back(dtf0g2);
	dtf0->addRow(dtf0g2);

	QGroupBox* dtl1 = new QGroupBox(tr("Theme"));
	QFormLayout* dtf1 = new QFormLayout;
	dtf1->setSpacing(20);
	dtf1->setFormAlignment(Qt::AlignLeft);
	dtf1->setFieldGrowthPolicy(QFormLayout::FieldsStayAtSizeHint);

	//TODO FIX SEGFAULT previous settings
	QComboBox* dtf1g3 = new QComboBox;
	dtf1g3->setProperty("pref", "theme");
	prefs[PREF_SECTIONS::Preferences].emplace_back(dtf1g3);
	dtf1g3->addItem(tr("Default (system theme)"), "");
	dtf1g3->addItem(tr("Dark"), "dark");
	dtf1g3->addItem(tr("Light"), "light");
	dtf1->addRow(dtf1g3);
	dtf1->addRow(new QLabel(tr("<small>The software needs to be restarted after switching theme.</small>")));

	QGroupBox* dtl2 = new QGroupBox(tr("Tools"));
	QFormLayout* dtf2 = new QFormLayout;
	dtf2->setSpacing(20);
	dtf2->setFormAlignment(Qt::AlignLeft);
	dtf2->setFieldGrowthPolicy(QFormLayout::FieldsStayAtSizeHint);

	//label: CSV Import/Export

	QCheckBox* dtf2g4 = new QCheckBox(tr("Allow header columns in CSV"));
	dtf2g4->setProperty("pref", "fields_default"); //
	prefs[PREF_SECTIONS::Preferences].emplace_back(dtf2g4);
	dtf2->addRow(dtf2g4);

	//TODO win32 convert \n to \r\n ?
	QLineEdit* dtf2g5 = new QLineEdit("\\n");
	dtf2g5->setProperty("pref", "toolsCsvDelimiter");
	prefs[PREF_SECTIONS::Connections].emplace_back(dtf2g5);
	dtf2g5->setMaxLength(2);
	dtf2g5->setMaximumWidth(50);
	dtf2->addRow(tr("CSV delimiter character"), dtf2g5);

	QLineEdit* dtf2g6 = new QLineEdit(",");
	dtf2g6->setProperty("pref", "toolsCsvSeparator");
	prefs[PREF_SECTIONS::Connections].emplace_back(dtf2g6);
	dtf2g6->setMaxLength(1);
	dtf2g6->setMaximumWidth(50);
	dtf2->addRow(tr("CSV separator character"), dtf2g6);

	QLineEdit* dtf2g7 = new QLineEdit(",");
	dtf2g7->setProperty("pref", "toolsCsvEscape");
	prefs[PREF_SECTIONS::Connections].emplace_back(dtf2g7);
	dtf2g7->setMaxLength(1);
	dtf2g7->setMaximumWidth(50);
	dtf2->addRow(tr("CSV escape character"), dtf2g7);

	//label: Fields Import/Export

	QButtonGroup* dtg2 = new QButtonGroup;
	dtg2->setExclusive(true);

	QCheckBox* dtf2g8 = new QCheckBox(tr("Default (same fields as visual)"));
	dtf2g8->setProperty("pref", "fields_default"); //
	prefs[PREF_SECTIONS::Preferences].emplace_back(dtf2g8);
	dtg2->addButton(dtf2g8);
	dtf2->addRow(dtf2g8);

	QCheckBox* dtf2g9 = new QCheckBox(tr("Extendend (all fields)"));
	dtf2g9->setProperty("pref", "fields_extended"); //
	prefs[PREF_SECTIONS::Preferences].emplace_back(dtf2g9);
	dtg2->addButton(dtf2g9);
	dtf2->addRow(dtf2g9);

	dtl0->setLayout(dtf0);
	dtl1->setLayout(dtf1);
	dtl2->setLayout(dtf2);
	dtform->addItem(new QSpacerItem(0, 0));
	dtform->addWidget(dtl0);
	dtform->addItem(new QSpacerItem(0, 5));
	dtform->addWidget(dtl1);
	dtform->addItem(new QSpacerItem(0, 0));
	dtform->addWidget(dtl2);
	dtform->addItem(new QSpacerItem(0, 0));

	dtcnt->setAlignment(Qt::AlignLeft | Qt::AlignTop);
	dtcnt->addLayout(dtform, 0);
	dtpage->setLayout(dtcnt);

	dtwid->addTab(dtpage, tr("Preferences"));
}

void settings::connectionsLayout()
{
	this->rppage = new WidgetWithBackdrop;
	QHBoxLayout* dtcnt = new QHBoxLayout(rppage);

	QVBoxLayout* dtvbox = new QVBoxLayout;
	this->rplist = new QListWidget;
	rplist->setEditTriggers(QAbstractItemView::EditKeyPressed | QAbstractItemView::DoubleClicked);
	rplist->setStyleSheet("QListView::item { height: 44px; font: 16px } QListView QLineEdit { border: 1px solid palette(alternate-base) }");
	rplist->connect(rplist, &QListWidget::currentItemChanged, [=](QListWidgetItem* current, QListWidgetItem* previous) { this->currentProfileChanged(current, previous); });
	rplist->connect(rplist, &QListWidget::currentTextChanged, [=](QString text) { this->profileNameChanged(text); });
	rplist->connect(rplist, &QAbstractItemView::viewportEntered, [=]() { this->renameProfile(false); });
	rppage->connect(rppage, &WidgetWithBackdrop::backdrop, [=]() { this->renameProfile(false); });

	QToolBar* dttbar = new QToolBar;
	dttbar->setIconSize(QSize(12, 12));
	dttbar->setToolButtonStyle(Qt::ToolButtonIconOnly);
	dttbar->addAction(theme::icon("add"), tr("Add"), [=]() { this->addProfile(); });
	dttbar->addAction(theme::icon("remove"), tr("Remove"), [=]() { this->deleteProfile(); });

	dtvbox->setSpacing(0);
	dtvbox->addWidget(rplist);
	dtvbox->addWidget(dttbar);

	QFormLayout* dtform = new QFormLayout;

	QGroupBox* dtl0 = new QGroupBox(tr("Connection"));
	QFormLayout* dtf0 = new QFormLayout;
	dtf0->setFormAlignment(Qt::AlignLeft);
	dtf0->setFieldGrowthPolicy(QFormLayout::FieldsStayAtSizeHint);

	QLineEdit* dtf0ia = new QLineEdit("192.168.0.2");
	dtf0ia->setProperty("pref", "ipAddress");
	prefs[PREF_SECTIONS::Connections].emplace_back(dtf0ia);
	dtf0ia->setMinimumWidth(140);
	dtf0->addRow(tr("IP address"), dtf0ia);

	QHBoxLayout* dtb0 = new QHBoxLayout;
	dtf0->addRow(tr("FTP port"), dtb0);

	QLineEdit* dtf0fp = new QLineEdit("21");
	dtf0fp->setProperty("pref", "ftpPort");
	dtf0fp->setValidator(new QIntValidator(1, 65535));
	dtf0fp->setMaxLength(5);
	dtf0fp->setMaximumWidth(50);

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
	dtf0hp->setMaximumWidth(50);
	prefs[PREF_SECTIONS::Connections].emplace_back(dtf0hp);
	dtf0->addRow(tr("HTTP port"), dtf0hp);

	QGroupBox* dtl1 = new QGroupBox(tr("Login"));
	QFormLayout* dtf1 = new QFormLayout;
	dtf1->setFormAlignment(Qt::AlignLeft);
	dtf1->setFieldGrowthPolicy(QFormLayout::FieldsStayAtSizeHint);

	QLineEdit* dtf1lu = new QLineEdit("root");
	dtf1lu->setProperty("pref", "username");
	prefs[PREF_SECTIONS::Connections].emplace_back(dtf1lu);
	dtf1lu->setMinimumWidth(120);
	dtf1->addRow(tr("Username"), dtf1lu);

	QLineEdit* dtf1lp = new QLineEdit;
	dtf1lp->setProperty("pref", "password");
	dtf1lp->setEchoMode(QLineEdit::Password);
	prefs[PREF_SECTIONS::Connections].emplace_back(dtf1lp);
	dtf1lp->setMinimumWidth(120);
	dtf1->addRow(tr("Password"), dtf1lp); // show/hide

	QGroupBox* dtl2 = new QGroupBox("Configuration");
	QFormLayout* dtf2 = new QFormLayout;
	dtf2->setFormAlignment(Qt::AlignLeft);

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
	dtf3->setFormAlignment(Qt::AlignLeft);
	dtf3->setFieldGrowthPolicy(QFormLayout::ExpandingFieldsGrow);

	QLineEdit* dtf3ca = new QLineEdit;
	dtf3ca->setProperty("pref", "customWebifReloadUrl");
	prefs[PREF_SECTIONS::Connections].emplace_back(dtf3ca);
	dtf3->addRow(tr("Custom webif reload URL address"), dtf3ca);

	QLineEdit* dtf3cc = new QLineEdit;
	dtf3cc->setProperty("pref", "customTelnetReloadCmd");
	prefs[PREF_SECTIONS::Connections].emplace_back(dtf3cc);
	dtf3->addRow(tr("Custom telnet reload command"), dtf3cc);

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

//TODO implement contextual and delete row
void settings::advancedLayout()
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
	QHBoxLayout* dtnthb = new QHBoxLayout;
	QWidget* dtntsp = new QWidget;
	dtntsp->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	QLabel* dtntcl = new QLabel(tr("<h2>Please be carefull!</h2><br><p>Modifing these settings could break the program.</p>"));
	dtntcl->setAlignment(Qt::AlignCenter);
	QPushButton* dtntcb = new QPushButton;
	dtntcb->setText(tr("OK, I understood this."));
	dtntcb->connect(dtntcb, &QPushButton::pressed, [=]() { adntc->setHidden(true); adtbl->setVisible(true); retrieve(adtbl); });
	dtnthb->addWidget(dtntsp);
	dtnthb->addWidget(dtntcb);
	dtnthb->addWidget(dtntsp);
	dtntcg->addItem(new QSpacerItem(0, 100), 0, 0);
	dtntcg->addWidget(dtntcl, 1, 0);
	dtntcg->addLayout(dtnthb, 2, 0);
	dtntcg->addItem(new QSpacerItem(0, 100), 3, 0);
	adntc->setLayout(dtntcg);

	dtcnt->addWidget(adntc, 0);
	dtcnt->addWidget(adtbl, 1);
	dtpage->setLayout(dtcnt);

	dtwid->addTab(dtpage, tr("Advanced"));
}

QListWidgetItem* settings::addProfile(int i)
{
	if (i == -1)
	{
		i = sets->value("profile/size").toInt();
		// i++;
		tmpps[i]["profileName"] = tr("Profile");
	}
	debug("addProfile()", "index", i);

	QListWidgetItem* item = new QListWidgetItem(tr("Profile"), rplist);
	item->setText(item->text() + ' ' + QString::fromStdString(to_string(i)));
	item->setData(Qt::UserRole, i);
	item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEditable | Qt::ItemIsEnabled);
	if (! this->state.retr)
	{
		renameProfile(false);
		item->setSelected(true);
		rplist->setCurrentItem(item);
	
		if (rplist->count() != 1)
			renameProfile();
	}
	return item;
}

void settings::deleteProfile()
{
	debug("deleteProfile()");

	QListWidgetItem* curr = rplist->currentItem();
	int i = curr->data(Qt::UserRole).toInt();
	tmpps[i].clear();
	this->state.dele = true;

	renameProfile(false);
	if (rplist->count() != 1)
		rplist->takeItem(rplist->currentRow());
}

void settings::renameProfile(bool enabled)
{
	QListWidgetItem* curr = rplist->currentItem();
	if (enabled && ! rplist->isPersistentEditorOpen(curr))
		rplist->openPersistentEditor(curr);
	else
		rplist->closePersistentEditor(curr);
}

void settings::updateProfile(QListWidgetItem* item)
{
	debug("updateProfile()");

	int i = item->data(Qt::UserRole).toInt();
	for (auto & item : prefs[PREF_SECTIONS::Connections])
	{
		QString pref = item->property("pref").toString();
		if (QLineEdit* field = qobject_cast<QLineEdit*>(item))
			tmpps[i][pref] = field->text();
		else if (QCheckBox* field = qobject_cast<QCheckBox*>(item))
			tmpps[i][pref] = field->isChecked();
	}
}

void settings::profileNameChanged(QString text)
{
	debug("profileNameChanged()");

	QListWidgetItem* curr = rplist->currentItem();
	int i = curr->data(Qt::UserRole).toInt();
	tmpps[i]["profileName"] = text;
}

void settings::currentProfileChanged(QListWidgetItem* current, QListWidgetItem* previous)
{
	debug("currentProfileChanged()");

	if (previous != nullptr && ! this->state.dele)
		updateProfile(previous);

	this->retrieve(current);
	this->state.dele = false;
}

void settings::tabChanged(int index)
{
	debug("tabChanged()", "index", index);

	if (this->state.prev == -1)
	{
		this->state.prev = index;
		return;
	}
	switch (this->state.prev)
	{
		case PREF_SECTIONS::Connections:
			renameProfile(false);
		break;
		case PREF_SECTIONS::Advanced:
			adntc->setVisible(true);
			adtbl->setHidden(true);
		break;
	}
	this->state.prev = index;
}

void settings::store()
{
	debug("store()");

	updateProfile(rplist->currentItem());

	int size = sets->value("profile/size").toInt();
	sets->beginWriteArray("profile");
	for (size_t i = 0; i < tmpps.size(); i++)
	{
		sets->setArrayIndex(i);
		if (tmpps[i].size())
		{
			if (! sets->contains("profileName"))
				size++;
			for (auto & field : tmpps[i])
				sets->setValue(field.first, field.second);
		}
		else
		{
			sets->remove("profileName");
			for (auto & item : prefs[PREF_SECTIONS::Connections])
			{
				QString pref = item->property("pref").toString();
				sets->remove(pref);
			}
		}
	}
	sets->endArray();
	sets->setValue("profile/size", size);

	sets->beginGroup("preference");
	for (auto & item : prefs[PREF_SECTIONS::Preferences])
	{
		QString pref = item->property("pref").toString();
		if (QCheckBox* field = qobject_cast<QCheckBox*>(item))
			sets->setValue(pref, field->isChecked());
		else if (QLineEdit* field = qobject_cast<QLineEdit*>(item))
			sets->setValue(pref, field->text());
		else if (QComboBox* field = qobject_cast<QComboBox*>(item))
			sets->setValue(pref, field->currentData());
	}
	sets->endGroup();
}

void settings::store(QTableWidget* adtbl)
{
	debug("store()", "", "advanced");

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
	debug("retrieve()");

	this->state.retr = true;
	int selected = sets->value("profile/selected").toInt();
	int size = sets->beginReadArray("profile");
	for (int i = 0; i < size; i++)
	{
		sets->setArrayIndex(i);
		if (! sets->contains("profileName"))
			continue;

		tmpps[i]["profileName"] = sets->value("profileName");
		QListWidgetItem* item = addProfile(i);
		item->setText(sets->value("profileName").toString());

		for (auto & item : prefs[PREF_SECTIONS::Connections])
		{
			QString pref = item->property("pref").toString();
			tmpps[i][pref] = sets->value(pref);
			if (i == selected)
			{
				if (QLineEdit* field = qobject_cast<QLineEdit*>(item))
					field->setText(sets->value(pref).toString());
				else if (QCheckBox* field = qobject_cast<QCheckBox*>(item))
					field->setChecked(sets->value(pref).toBool());
			}
		}
	}
	sets->endArray();
	rplist->setCurrentRow(selected);
	this->state.retr = false;

	sets->beginGroup("preference");
	for (auto & item : prefs[PREF_SECTIONS::Preferences])
	{
		QString pref = item->property("pref").toString();
		if (QCheckBox* field = qobject_cast<QCheckBox*>(item))
		{
			field->setChecked(sets->value(pref).toBool());
		}
		else if (QLineEdit* field = qobject_cast<QLineEdit*>(item))
		{
			field->setText(sets->value(pref).toString());
		}
		else if (QComboBox* field = qobject_cast<QComboBox*>(item))
		{
			if (int index = field->findData(sets->value(pref).toString(), Qt::UserRole))
				field->setCurrentIndex(index);
		}
	}
	sets->endGroup();
}

void settings::retrieve(QListWidgetItem* item)
{
	debug("retrieve()", "", "item");

	int i = item->data(Qt::UserRole).toInt();
	for (auto & item : prefs[PREF_SECTIONS::Connections])
	{
		QString pref = item->property("pref").toString();
		if (QLineEdit* field = qobject_cast<QLineEdit*>(item))
			field->setText(tmpps[i][pref].toString());
		else if (QCheckBox* field = qobject_cast<QCheckBox*>(item))
			field->setChecked(tmpps[i][pref].toBool());
	}
}

void settings::retrieve(QTableWidget* adtbl)
{
	debug("retrieve()", "", "advanced");

	QStringList keys = sets->allKeys().filter(QRegularExpression("^(application|preference|profile)/"));
	QStringList::const_iterator iq;
	adtbl->setRowCount(keys.count());
	int i = 0;
	for (iq = keys.constBegin(); iq != keys.constEnd(); ++iq)
	{
		QTableWidgetItem* field = new QTableWidgetItem;
		field->setText((*iq).toLocal8Bit().replace("/", ".")); //Qt5
		field->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
		adtbl->setItem(i, 0, field);
		adtbl->setItem(i, 1, new QTableWidgetItem(sets->value(*iq).toString()));
		i++;
	}
	adtbl->resizeColumnsToContents();
}

void settings::save()
{
	debug("save()");

	if (dtwid->currentIndex() == PREF_SECTIONS::Advanced)
		store(adtbl);
	else
		store();

	dial->close();
}

}
