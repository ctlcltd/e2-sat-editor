/*!
 * e2-sat-editor/src/gui/settings.cpp
 *
 * @link https://github.com/ctlcltd/e2-sat-editor
 * @copyright e2 SAT Editor Team
 * @author Leonardo Laureti
 * @version 0.5
 * @license MIT License
 * @license GNU GPLv3 License
 */

#include <Qt>
#include <QTimer>
#include <QRegularExpression>
#include <QScrollArea>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGroupBox>
#include <QFormLayout>
#include <QToolBar>
#include <QLabel>
#include <QLineEdit>
#include <QButtonGroup>
#include <QCheckBox>
#include <QComboBox>
#include <QValidator>
#include <QHeaderView>

#include "platforms/platform.h"

#include "toolkit/ThemeChangeEventObserver.h"
#include "settings.h"
#include "theme.h"

using std::to_string;

using namespace e2se;

namespace e2se_gui
{

settings::settings(gui* gid)
{
	this->log = new logger("gui", "settings");

	this->gid = gid;
	this->sets = new QSettings;

	this->state.prev = -1;
}

settings::~settings()
{
	debug("~settings");

	delete this->sets;
	delete this->log;
}

void settings::display(QWidget* cwid)
{
	debug("display");

	layout(cwid);

	retrieve();

	dial->exec();
}

void settings::layout(QWidget* cwid)
{
	debug("layout");

	this->dial = new QDialog(cwid);
	dial->setWindowTitle(tr("Settings"));
	dial->setStyleSheet("QGroupBox { spacing: 0; padding: 0; padding-top: 20px; border: 0; font-weight: bold } QGroupBox::title { margin: 0 10px }");

	this->theme = new e2se_gui::theme;

	ThemeChangeEventObserver* gce = new ThemeChangeEventObserver;
	gce->setEventCallback([=]() { this->themeChanged(); });
	dial->installEventFilter(gce);

	platform::osWindowBlend(dial);

	QGridLayout* dfrm = new QGridLayout(dial);

	QHBoxLayout* dhbox = new QHBoxLayout;
	QVBoxLayout* dvbox = new QVBoxLayout;

	this->dtwid = new QTabWidget;
	dtwid->connect(dtwid, &QTabWidget::currentChanged, [=](int index) { this->tabChanged(index); });

	this->action.dtsave = new QPushButton;
	this->action.dtsave->setDefault(true);
	this->action.dtsave->setText(tr("Save Settings"));
	this->action.dtsave->connect(this->action.dtsave, &QPushButton::pressed, [=]() { this->save(); });

	this->action.dtcancel = new QPushButton;
	this->action.dtcancel->setText(tr("Cancel"));
	this->action.dtcancel->connect(this->action.dtcancel, &QPushButton::pressed, [=]() { this->cancel(); });

	connectionsLayout();
	preferencesLayout();
	advancedLayout();

	dfrm->setColumnStretch(0, 1);
	dfrm->setRowStretch(0, 1);
	dhbox->setAlignment(Qt::AlignRight);

	dvbox->addWidget(dtwid);
	dhbox->addWidget(this->action.dtcancel);
	dhbox->addWidget(this->action.dtsave);
	dvbox->addLayout(dhbox);

	dfrm->addLayout(dvbox, 0, 0);
	dfrm->setSizeConstraint(QGridLayout::SetFixedSize);

	dial->setLayout(dfrm);
}

void settings::preferencesLayout()
{
	QWidget* dtpage = new QWidget;
	dtpage->setObjectName("preferences_page");
	QScrollArea* dtarea = new QScrollArea;

	QHBoxLayout* dtcnt = new QHBoxLayout(dtpage);
	dtpage->setStyleSheet("QGroupBox { font-weight: bold }");

	QFormLayout* dtform = new QFormLayout;
	dtform->setSpacing(10);
	dtform->setFormAlignment(Qt::AlignLeft | Qt::AlignTop);

	QGroupBox* dtl0 = new QGroupBox(tr("General"));
	QFormLayout* dtf0 = new QFormLayout;
	dtf0->setSpacing(20);
	dtf0->setFormAlignment(Qt::AlignLeft);
	dtf0->setFieldGrowthPolicy(QFormLayout::FieldsStayAtSizeHint);

	QCheckBox* dtf0ac = new QCheckBox(tr("Show confirmation messages when deleting"));
	dtf0ac->setProperty("pref", "askConfirmation");
	prefs[PREF_SECTIONS::Preferences].emplace_back(dtf0ac);
	dtf0->addRow(dtf0ac);

	QCheckBox* dtf0nd = new QCheckBox(tr("Non-destructive edit (try to preserve origin lists)"));
	dtf0nd->setProperty("pref", "nonDestructiveEdit");
	dtf0nd->setChecked(true);
	prefs[PREF_SECTIONS::Preferences].emplace_back(dtf0nd);
	dtf0->addRow(dtf0nd);

	QCheckBox* dtf0li = new QCheckBox(tr("Parental lock whitelist (exclusion instead inclusion list)"));
	dtf0li->setProperty("pref", "parentalLockInvert");
	prefs[PREF_SECTIONS::Preferences].emplace_back(dtf0li);
	dtf0->addRow(dtf0li);

	QGroupBox* dtl1 = new QGroupBox(tr("Drag and Drop"));
	QFormLayout* dtf1 = new QFormLayout;
	dtf1->setSpacing(20);
	dtf1->setFormAlignment(Qt::AlignLeft);
	dtf1->setFieldGrowthPolicy(QFormLayout::FieldsStayAtSizeHint);

	QCheckBox* dtf1sc = new QCheckBox(tr("Switch to current bouquet item after the drop"));
	dtf1sc->setProperty("pref", "treeCurrentAfterDrop");
	dtf1sc->setChecked(true);
	prefs[PREF_SECTIONS::Preferences].emplace_back(dtf1sc);
	dtf1->addRow(dtf1sc);

	QFormLayout* dtf11 = new QFormLayout;
	dtf11->setSpacing(20);
	dtf11->setFormAlignment(Qt::AlignLeft);
	dtf11->setFieldGrowthPolicy(QFormLayout::FieldsStayAtSizeHint);
	QLabel* dth11 = new QLabel(tr("Channel operations"));
	dtf11->addRow(dth11);

	QButtonGroup* dtg1 = new QButtonGroup;
	dtg1->setExclusive(true);

	QCheckBox* dtf1ci = new QCheckBox(tr("Copy channels (preserving)"));
	dtf1ci->setProperty("pref", "treeDropCopy");
	prefs[PREF_SECTIONS::Preferences].emplace_back(dtf1ci);
	dtf1ci->setChecked(true);
	dtg1->addButton(dtf1ci);
	dtf11->addRow(dtf1ci);

	QCheckBox* dtf1mi = new QCheckBox(tr("Move channels (deleting)"));
	dtf1mi->setProperty("pref", "treeDropMove");
	prefs[PREF_SECTIONS::Preferences].emplace_back(dtf1mi);
	dtg1->addButton(dtf1mi);
	dtf11->addRow(dtf1mi);

	dtf1->addRow(dtf11);

	QGroupBox* dtl2 = new QGroupBox(tr("Theme"));
	QFormLayout* dtf2 = new QFormLayout;
	dtf2->setSpacing(20);
	dtf2->setFormAlignment(Qt::AlignLeft);
	dtf2->setFieldGrowthPolicy(QFormLayout::FieldsStayAtSizeHint);

	QComboBox* dtf2td = new QComboBox;
	dtf2td->setProperty("pref", "theme");
	prefs[PREF_SECTIONS::Preferences].emplace_back(dtf2td);
	dtf2td->addItem(tr("Default (system theme)"), "");
	dtf2td->addItem(tr("Dark"), "dark");
	dtf2td->addItem(tr("Light"), "light");
	platform::osComboBox(dtf2td);
	dtf2->addRow(dtf2td);

	QCheckBox* dtf2oe = new QCheckBox(tr("Enable experimental features"));
	dtf2oe->setProperty("pref", "osExperiment");
	prefs[PREF_SECTIONS::Preferences].emplace_back(dtf2oe);
	dtf2oe->setChecked(true);
	dtf2->addRow(dtf2oe);

	dtf2->addRow(new QLabel(tr("<small>The software might need to be restarted.</small>")));

	QGroupBox* dtl3 = new QGroupBox(tr("Tools"));
	QHBoxLayout* dtb3 = new QHBoxLayout;

	QFormLayout* dtf30 = new QFormLayout;
	dtf30->setSpacing(20);
	dtf30->setFormAlignment(Qt::AlignLeft);
	dtf30->setFieldGrowthPolicy(QFormLayout::FieldsStayAtSizeHint);
	QLabel* dth30 = new QLabel(tr("CSV Import/Export"));
	dtf30->addRow(dth30);

	QCheckBox* dtf3th = new QCheckBox(tr("Allow header columns in CSV"));
	dtf3th->setProperty("pref", "toolsCsvHeader"); //
	prefs[PREF_SECTIONS::Preferences].emplace_back(dtf3th);
	dtf30->addRow(dtf3th);

	QLineEdit* dtf3cd = new QLineEdit("\\n");
	dtf3cd->setProperty("pref", "toolsCsvDelimiter");
	prefs[PREF_SECTIONS::Preferences].emplace_back(dtf3cd);
	dtf3cd->setMaxLength(2);
	dtf3cd->setMaximumWidth(50);
	platform::osLineEdit(dtf3cd);
	dtf30->addRow(tr("CSV delimiter character"), dtf3cd);

	QLineEdit* dtf3cs = new QLineEdit(",");
	dtf3cs->setProperty("pref", "toolsCsvSeparator");
	prefs[PREF_SECTIONS::Preferences].emplace_back(dtf3cs);
	dtf3cs->setMaxLength(1);
	dtf3cs->setMaximumWidth(50);
	platform::osLineEdit(dtf3cs);
	dtf30->addRow(tr("CSV separator character"), dtf3cs);

	QLineEdit* dtf3ce = new QLineEdit(",");
	dtf3ce->setProperty("pref", "toolsCsvEscape");
	prefs[PREF_SECTIONS::Preferences].emplace_back(dtf3ce);
	dtf3ce->setMaxLength(1);
	dtf3ce->setMaximumWidth(50);
	platform::osLineEdit(dtf3ce);
	dtf30->addRow(tr("CSV escape character"), dtf3ce);

	QFormLayout* dtf31 = new QFormLayout;
	dtf31->setSpacing(20);
	dtf31->setFormAlignment(Qt::AlignLeft);
	dtf31->setFieldGrowthPolicy(QFormLayout::FieldsStayAtSizeHint);
	QLabel* dth31 = new QLabel(tr("Fields Import/Export"));
	dtf31->addRow(dth31);

	QButtonGroup* dtg3 = new QButtonGroup;
	dtg3->setExclusive(true);

	QCheckBox* dtf3df = new QCheckBox(tr("Default (same fields as visual)"));
	dtf3df->setProperty("pref", "toolsFieldsDefault");
	prefs[PREF_SECTIONS::Preferences].emplace_back(dtf3df);
	dtf3df->setChecked(true);
	dtg3->addButton(dtf3df);
	dtf31->addRow(dtf3df);

	QCheckBox* dtf3ef = new QCheckBox(tr("Extended (all fields)"));
	dtf3ef->setProperty("pref", "toolsFieldsExtended");
	prefs[PREF_SECTIONS::Preferences].emplace_back(dtf3ef);
	dtg3->addButton(dtf3ef);
	dtf31->addRow(dtf3ef);

	dtb3->addLayout(dtf30);
	dtb3->addItem(new QSpacerItem(10, 0));
	dtb3->addLayout(dtf31);

	dtl0->setLayout(dtf0);
	dtl1->setLayout(dtf1);
	dtl2->setLayout(dtf2);
	dtl3->setLayout(dtb3);

	dtform->addItem(new QSpacerItem(0, 0));
	dtform->addWidget(dtl0);
	dtform->addItem(new QSpacerItem(0, 5));
	dtform->addWidget(dtl1);
	dtform->addItem(new QSpacerItem(0, 5));
	dtform->addWidget(dtl2);
	dtform->addItem(new QSpacerItem(0, 5));
	//TODO FIX column gap [Linux] [Windows]
	dtform->addWidget(dtl3);
	dtform->addItem(new QSpacerItem(0, 0));

	dtcnt->setAlignment(Qt::AlignLeft | Qt::AlignTop);
	dtcnt->addLayout(dtform, 0);
	dtpage->setLayout(dtcnt);

	dtarea->setWidget(dtpage);
	dtarea->setWidgetResizable(true);
	dtarea->setStyleSheet("QScrollArea, #preferences_page { background: transparent }");

	dtwid->addTab(dtarea, tr("Preferences"));
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
	platform::osPersistentEditor(rplist);

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
	platform::osLineEdit(dtf0ia);
	dtf0->addRow(tr("IP address"), dtf0ia);

	QHBoxLayout* dtb0 = new QHBoxLayout;
	dtf0->addRow(tr("FTP port"), dtb0);

	QLineEdit* dtf0fp = new QLineEdit("21");
	dtf0fp->setProperty("pref", "ftpPort");
	dtf0fp->setValidator(new QIntValidator(1, 65535));
	dtf0fp->setMaxLength(5);
	dtf0fp->setMaximumWidth(50);
	platform::osLineEdit(dtf0fp);

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
	platform::osLineEdit(dtf0hp);
	dtf0->addRow(tr("HTTP port"), dtf0hp);

	QGroupBox* dtl1 = new QGroupBox(tr("Login"));
	QFormLayout* dtf1 = new QFormLayout;
	dtf1->setFormAlignment(Qt::AlignLeft);
	dtf1->setFieldGrowthPolicy(QFormLayout::FieldsStayAtSizeHint);

	QLineEdit* dtf1lu = new QLineEdit("root");
	dtf1lu->setProperty("pref", "username");
	prefs[PREF_SECTIONS::Connections].emplace_back(dtf1lu);
	dtf1lu->setMinimumWidth(120);
	platform::osLineEdit(dtf1lu);
	dtf1->addRow(tr("Username"), dtf1lu);

	QLineEdit* dtf1lp = new QLineEdit;
	dtf1lp->setProperty("pref", "password");
	dtf1lp->setEchoMode(QLineEdit::Password);
	prefs[PREF_SECTIONS::Connections].emplace_back(dtf1lp);
	dtf1lp->setMinimumWidth(120);
	platform::osLineEdit(dtf1lp);
	dtf1->addRow(tr("Password"), dtf1lp); // show/hide

	QGroupBox* dtl2 = new QGroupBox("Configuration");
	QFormLayout* dtf2 = new QFormLayout;
	dtf2->setFormAlignment(Qt::AlignLeft);

	QHBoxLayout* dtb20 = new QHBoxLayout;
	dtf2->addRow(tr("Transponders"), dtb20);
	QLineEdit* dtf2pt = new QLineEdit("/etc/tuxbox");
	dtf2pt->setProperty("pref", "pathTransponders");
	prefs[PREF_SECTIONS::Connections].emplace_back(dtf2pt);
	platform::osLineEdit(dtf2pt);
	dtb20->addWidget(dtf2pt);
	dtb20->addWidget(new QLabel("<small>(satellites.xml)</small>"));

	QHBoxLayout* dtb21 = new QHBoxLayout;
	dtf2->addRow(tr("Services"), dtb21);
	QLineEdit* dtf2ps = new QLineEdit("/etc/enigma2");
	dtf2ps->setProperty("pref", "pathServices");
	prefs[PREF_SECTIONS::Connections].emplace_back(dtf2ps);
	platform::osLineEdit(dtf2ps);
	dtb21->addWidget(dtf2ps);
	dtb21->addWidget(new QLabel("<small>(lamedb)</small>"));

	QHBoxLayout* dtb22 = new QHBoxLayout;
	dtf2->addRow(tr("Bouquets"), dtb22);
	QLineEdit* dtf2pb = new QLineEdit("/etc/enigma2");
	dtf2pb->setProperty("pref", "pathBouquets");
	prefs[PREF_SECTIONS::Connections].emplace_back(dtf2pb);
	platform::osLineEdit(dtf2pb);
	dtb22->addWidget(dtf2pb);
	dtb22->addWidget(new QLabel("<small>(*.bouquet, *.userbouquet)</small>"));

	QGroupBox* dtl3 = new QGroupBox(tr("Commands"));
	QFormLayout* dtf3 = new QFormLayout;
	dtf3->setFormAlignment(Qt::AlignLeft);
	dtf3->setFieldGrowthPolicy(QFormLayout::ExpandingFieldsGrow);

	QLineEdit* dtf3ca = new QLineEdit;
	dtf3ca->setProperty("pref", "customWebifReloadUrl");
	prefs[PREF_SECTIONS::Connections].emplace_back(dtf3ca);
	platform::osLineEdit(dtf3ca);
	dtf3->addRow(tr("Custom webif reload URL address"), dtf3ca);

	QLineEdit* dtf3cc = new QLineEdit;
	dtf3cc->setProperty("pref", "customTelnetReloadCmd");
	prefs[PREF_SECTIONS::Connections].emplace_back(dtf3cc);
	platform::osLineEdit(dtf3cc);
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

void settings::advancedLayout()
{
	QWidget* dtpage = new QWidget;
	QVBoxLayout* dtcnt = new QVBoxLayout(dtpage);

	this->adtbl = new QTableWidget(0, 2);
	adtbl->setHidden(true);
	adtbl->setHorizontalHeaderLabels({"ID", "VALUE"});
	adtbl->horizontalHeader()->setSectionsClickable(false);
	adtbl->verticalHeader()->setVisible(false);
	platform::osPersistentEditor(adtbl);

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

void settings::themeChanged()
{
	debug("themeChanged");

	theme->changed();
}

QListWidgetItem* settings::addProfile(int i)
{
	if (i == -1)
	{
		i = sets->value("profile/size").toInt();
		// i++;
		tmpps[i]["profileName"] = tr("Profile");
	}
	debug("addProfile", "index", i);

	QListWidgetItem* item = new QListWidgetItem(tr("Profile"), rplist);
	item->setText(item->text() + ' ' + QString::number(i));
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
	debug("deleteProfile");

	QListWidgetItem* item = rplist->currentItem();
	int i = item->data(Qt::UserRole).toInt();
	tmpps[i].clear();
	this->state.dele = true;

	renameProfile(false);
	if (rplist->count() != 1)
		rplist->takeItem(rplist->currentRow());
}

void settings::renameProfile(bool enabled)
{
	debug("renameProfile", "enabled", enabled);

	QListWidgetItem* item = rplist->currentItem();

	if (enabled && ! rplist->isPersistentEditorOpen(item))
		rplist->openPersistentEditor(item);
	else
		rplist->closePersistentEditor(item);
}

void settings::updateProfile(QListWidgetItem* item)
{
	debug("updateProfile");

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
	debug("profileNameChanged");

	QListWidgetItem* item = rplist->currentItem();
	int i = item->data(Qt::UserRole).toInt();
	tmpps[i]["profileName"] = text;
}

void settings::currentProfileChanged(QListWidgetItem* current, QListWidgetItem* previous)
{
	debug("currentProfileChanged");

	if (previous != nullptr && ! this->state.dele)
		updateProfile(previous);

	this->retrieve(current);
	this->state.dele = false;
}

void settings::tabChanged(int index)
{
	debug("tabChanged", "index", index);

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
	debug("store");

	updateProfile(rplist->currentItem());

	int size = sets->value("profile/size").toInt();
	sets->beginWriteArray("profile");
	for (size_t i = 0; i < tmpps.size(); i++)
	{
		int x = int (i);
		sets->setArrayIndex(x);
		if (tmpps[x].size())
		{
			if (! sets->contains("profileName"))
				size++;
			for (auto & field : tmpps[x])
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
	debug("store", "overload", "advanced");

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
	debug("retrieve");

	this->state.retr = true;
	int selected = sets->value("profile/selected", 0).toInt();
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
				if (sets->value(pref).isNull())
					continue;
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
		if (sets->value(pref).isNull())
		{
			continue;
		}
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
			int index = field->findData(sets->value(pref).toString(), Qt::UserRole);
			field->setCurrentIndex(index);
		}
	}
	sets->endGroup();
}

void settings::retrieve(QListWidgetItem* item)
{
	debug("retrieve", "overload", "item");

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
	debug("retrieve", "overload", "advanced");

	QStringList keys = sets->allKeys().filter(QRegularExpression("^(application|preference|profile|settings)/"));
	QStringList::const_iterator iq;
	adtbl->setRowCount(int (keys.count()));
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
	debug("save");

	if (dtwid->currentIndex() == PREF_SECTIONS::Advanced)
		store(adtbl);
	else
		store();

	gid->settingsChanged();

	// delay too fast
	QTimer::singleShot(150, [=]() {
		dial->close();
	});
}

void settings::cancel()
{
	debug("cancel");

	// delay too fast
	QTimer::singleShot(150, [=]() {
		dial->close();
	});
}

void settings::destroy()
{
	debug("destroy");

	delete this->theme;
	delete this->dial;
	delete this;
}

}
