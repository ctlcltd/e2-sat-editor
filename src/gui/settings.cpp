/*!
 * e2-sat-editor/src/gui/settings.cpp
 *
 * @link https://github.com/ctlcltd/e2-sat-editor
 * @copyright e2 SAT Editor Team
 * @author Leonardo Laureti
 * @version 1.4.0
 * @license MIT License
 * @license GNU GPLv3 License
 */

#include <cstring>

// note: std::runtime_error behaviour
#include <stdexcept>

#include <Qt>
#include <QTimer>
#include <QApplication>
#include <QRegularExpression>
#include <QByteArray>
#include <QMessageBox>
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
#include <QMenu>
#include <QWidgetAction>
#include <QHeaderView>
#include <QMouseEvent>
#ifdef Q_OS_WIN
#include <QStyleFactory>
#include <QScrollBar>
#endif
#include <QFile>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>

#include "../e2se_defs.h"

#include "platforms/platform.h"

#include "toolkit/ThemeChangeEventObserver.h"
#include "settings.h"
#include "theme.h"
#include "l10n.h"

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

	this->theme = new e2se_gui::theme;

	this->dial = new QDialog(cwid);
	dial->setWindowTitle(tr("Settings", "dialog"));

#ifdef Q_OS_WIN
	theme->early_win_flavor_fix(dial);
#endif

	dial->setStyleSheet("QGroupBox { spacing: 0; padding: 0; padding-top: 20px; border: 0; font-weight: bold } QGroupBox::title { margin: 0 10px }");

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
	this->action.dtsave->setText(tr("Save", "dialog"));
	this->action.dtsave->connect(this->action.dtsave, &QPushButton::pressed, [=]() { this->save(); });

	this->action.dtcancel = new QPushButton;
	this->action.dtcancel->setText(tr("Cancel", "dialog"));
	this->action.dtcancel->connect(this->action.dtcancel, &QPushButton::pressed, [=]() { this->cancel(); });

	connectionsLayout();
	preferencesLayout();
	engineLayout();
	advancedLayout();

	dfrm->setColumnStretch(0, 1);
	dfrm->setRowStretch(0, 1);

	dhbox->setAlignment(Qt::AlignTrailing);

	dvbox->addWidget(dtwid);
	dhbox->addWidget(this->action.dtcancel);
	dhbox->addWidget(this->action.dtsave);
	dvbox->addLayout(dhbox);

	dfrm->addLayout(dvbox, 0, 0);
	dfrm->setSizeConstraint(QGridLayout::SetFixedSize);

	dial->setLayout(dfrm);
}

void settings::connectionsLayout()
{
	this->rppage = new WidgetWithBackdrop;
	QHBoxLayout* dtcnt = new QHBoxLayout(rppage);

	QVBoxLayout* dtvbox = new QVBoxLayout;

	this->rplist = new QListWidget;
	rplist->setUniformItemSizes(true);
	rplist->setSelectionMode(QListWidget::ExtendedSelection);
	rplist->setEditTriggers(QListWidget::EditKeyPressed | QListWidget::DoubleClicked);

#ifdef Q_OS_WIN
	if (! theme::isOverridden() && (theme::absLuma() || ! theme::isDefault()))
	{
		QStyle* style;

		if (theme::isFluentWin())
			style = QStyleFactory::create("windows11");
		else
			style = QStyleFactory::create("fusion");

		rplist->verticalScrollBar()->setStyle(style);
		rplist->horizontalScrollBar()->setStyle(style);
	}
#endif

	rplist->setStyleSheet("QListView::item { height: 44px; font: 16px } QListView QLineEdit { border: 1px solid palette(alternate-base) }");

	rplist->connect(rplist, &QListWidget::currentItemChanged, [=](QListWidgetItem* current, QListWidgetItem* previous) { this->currentProfileChanged(current, previous); });
	rplist->connect(rplist, &QListWidget::currentTextChanged, [=](QString text) { this->profileNameChanged(text); });
	rplist->connect(rplist, &QListWidget::viewportEntered, [=]() { this->renameProfile(false); });
	rppage->connect(rppage, &WidgetWithBackdrop::backdrop, [=]() { this->renameProfile(false); });
	platform::osPersistentEditor(rplist);
	rplist->setContextMenuPolicy(Qt::CustomContextMenu);
	rplist->connect(rplist, &QListWidget::customContextMenuRequested, [=](QPoint pos) { this->showProfileEditContextMenu(pos); });

	QToolBar* dttbar = new QToolBar;
	dttbar->setObjectName("profile_toolbar");
	dttbar->setToolButtonStyle(Qt::ToolButtonIconOnly);
#ifndef Q_OS_MAC
	dttbar->setIconSize(QSize(16, 16));
	dttbar->setStyleSheet("QToolBar { spacing: 0 } QToolButton { margin-top: 2px; width: 20px; border: 1px solid }");
#else
	dttbar->setIconSize(QSize(40, 20));
	dttbar->setStyleSheet("QToolBar { spacing: 0 } QToolButton { width: 21px; height: 16px; border: 1px solid }");
#endif

	dttbar->addAction(theme::icon("tool-add"), tr("Add"), [=]() { this->addProfile(); });
	dttbar->addAction(theme::icon("tool-remove"), tr("Remove"), [=]() { this->deleteProfile(); });
#ifndef Q_OS_MAC
	QWidget* spacer = new QWidget;
	spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Ignored);
	dttbar->addWidget(spacer);
#endif
	dttbar->addAction(theme::icon("tool-menu"), tr("Menu"));
	dttbar->actions().last()->connect(dttbar->actions().last(), &QAction::triggered, [=]() {
		QMenu* menu = this->profileMenu();
		QWidget* widget = dttbar->widgetForAction(dttbar->actions().last());
		QPoint pos = widget->mapFrom(dttbar, widget->pos());
		// menu->popup(widget->mapToGlobal(pos)));
		platform::osMenuPopup(menu, widget, pos);

		QMouseEvent mouseRelease(QEvent::MouseButtonRelease, pos, widget->mapToGlobal(QPoint(0, 0)), Qt::LeftButton, Qt::MouseButtons(Qt::LeftButton), {});
		QCoreApplication::sendEvent(widget, &mouseRelease);
	});

	QColor tbshade;
	QColor tbfocus;
	QString tbshade_hexArgb;
	QString tbfocus_hexArgb;
#ifndef Q_OS_MAC
	tbshade = QPalette().color(QPalette::Dark);
	tbshade.setAlphaF(0.55);
	tbshade_hexArgb = tbshade.name(QColor::HexArgb);

	tbfocus = QPalette().color(QPalette::Dark);
	tbfocus.setAlphaF(0.19);
	tbfocus_hexArgb = tbfocus.name(QColor::HexArgb);

	theme->dynamicStyleSheet(dttbar, "#profile_toolbar QToolButton { border-color: " + tbshade_hexArgb + " } #profile_toolbar QToolButton:pressed { background-color: " + tbfocus_hexArgb + " }", theme::light);

	tbshade = QPalette().color(QPalette::Base);
	tbshade_hexArgb = tbshade.name(QColor::HexArgb);

	tbfocus = QPalette().color(QPalette::Mid);
	tbfocus.setAlphaF(0.15);
	tbfocus_hexArgb = tbfocus.name(QColor::HexArgb);

	theme->dynamicStyleSheet(dttbar, "#profile_toolbar QToolButton { border-color: " + tbshade_hexArgb + " } #profile_toolbar QToolButton:pressed { background-color: " + tbfocus_hexArgb + " }", theme::dark);
#else
	tbshade = QColor(Qt::black);
	tbshade.setAlphaF(0.2);
	tbshade_hexArgb = tbshade.name(QColor::HexArgb);

	tbfocus = QColor(Qt::black);
	tbfocus.setAlphaF(0.08);
	tbfocus_hexArgb = tbfocus.name(QColor::HexArgb);

	theme->dynamicStyleSheet(dttbar, "#profile_toolbar QToolButton { border-color: " + tbshade_hexArgb + " } #profile_toolbar QToolButton:pressed { background-color: " + tbfocus_hexArgb + " }", theme::light);

	tbshade = QPalette().color(QPalette::Dark).darker();
	tbshade.setAlphaF(0.63);
	tbshade_hexArgb = tbshade.name(QColor::HexArgb);

	tbfocus = QPalette().color(QPalette::Dark).darker();
	tbfocus.setAlphaF(0.34);
	tbfocus_hexArgb = tbfocus.name(QColor::HexArgb);

	theme->dynamicStyleSheet(dttbar, "#profile_toolbar QToolButton { border-color: " + tbshade_hexArgb + " } #profile_toolbar QToolButton:pressed { background-color: " + tbfocus_hexArgb + " }", theme::dark);
#endif

	if (dttbar->layoutDirection() == Qt::LeftToRight)
	{
		dttbar->widgetForAction(dttbar->actions().first())->setStyleSheet("margin-right: -1px");
#ifndef Q_OS_MAC
		dttbar->widgetForAction(dttbar->actions().last())->setStyleSheet("width: 23px");
#else
		dttbar->widgetForAction(dttbar->actions().last())->setStyleSheet("width: 25px; margin-left: -1px");
#endif
	}
	else
	{
		dttbar->widgetForAction(dttbar->actions().first())->setStyleSheet("margin-left: -1px");
#ifndef Q_OS_MAC
		dttbar->widgetForAction(dttbar->actions().last())->setStyleSheet("width: 23px");
#else
		dttbar->widgetForAction(dttbar->actions().last())->setStyleSheet("width: 25px; margin-right: -1px");
#endif
	}

	dtvbox->setSpacing(0);
	dtvbox->addWidget(rplist);
	dtvbox->addWidget(dttbar);

	QWidget* dtpage = new QWidget;
	dtpage->setObjectName("connections_page");
	QScrollArea* dtarea = new QScrollArea;

#ifdef Q_OS_WIN
	if (! theme::isOverridden() && (theme::absLuma() || ! theme::isDefault()))
	{
		QStyle* style;

		if (theme::isFluentWin())
			style = QStyleFactory::create("windows11");
		else
			style = QStyleFactory::create("fusion");

		dtarea->verticalScrollBar()->setStyle(style);
		dtarea->horizontalScrollBar()->setStyle(style);
	}
#endif

	QFormLayout* dtform = new QFormLayout;

	QGroupBox* dtl0 = new QGroupBox(tr("Connection"));
	QFormLayout* dtf0 = new QFormLayout;
	dtf0->setFormAlignment(Qt::AlignLeading);
	dtf0->setFieldGrowthPolicy(QFormLayout::FieldsStayAtSizeHint);

	QLineEdit* dtf0ia = new QLineEdit("192.168.0.2");
	dtf0ia->setProperty("field", "ipAddress");
	prefs[PREF_SECTIONS::Connections].emplace_back(dtf0ia);
	dtf0ia->setMinimumWidth(140);
	platform::osLineEdit(dtf0ia);
	dtf0->addRow(tr("IP address"), dtf0ia);

	QHBoxLayout* dtb0 = new QHBoxLayout;
	dtf0->addRow(tr("FTP port"), dtb0);

	QLineEdit* dtf0fp = new QLineEdit("21");
	dtf0fp->setProperty("field", "ftpPort");
	dtf0fp->setValidator(new QIntValidator(1, 65535));
	dtf0fp->setMaxLength(5);
	dtf0fp->setMaximumWidth(50);
	platform::osLineEdit(dtf0fp);

	QCheckBox* dtf0fa = new QCheckBox(tr("Use active FTP"));
	dtf0fa->setProperty("field", "ftpActive");
	prefs[PREF_SECTIONS::Connections].emplace_back(dtf0fp);
	prefs[PREF_SECTIONS::Connections].emplace_back(dtf0fa);
	dtb0->addWidget(dtf0fp);
	dtb0->addWidget(dtf0fa);

	QLineEdit* dtf0hp = new QLineEdit("80");
	dtf0hp->setProperty("field", "httpPort");
	dtf0hp->setValidator(new QIntValidator(1, 65535));
	dtf0hp->setMaxLength(5);
	dtf0hp->setMaximumWidth(50);
	prefs[PREF_SECTIONS::Connections].emplace_back(dtf0hp);
	platform::osLineEdit(dtf0hp);
	dtf0->addRow(tr("HTTP port"), dtf0hp);

	QGroupBox* dtl1 = new QGroupBox(tr("Login"));
	QFormLayout* dtf1 = new QFormLayout;
	dtf1->setFormAlignment(Qt::AlignLeading);
	dtf1->setFieldGrowthPolicy(QFormLayout::FieldsStayAtSizeHint);

	QLineEdit* dtf1lu = new QLineEdit("root");
	dtf1lu->setProperty("field", "username");
	prefs[PREF_SECTIONS::Connections].emplace_back(dtf1lu);
	dtf1lu->setMinimumWidth(120);
	platform::osLineEdit(dtf1lu);
	dtf1->addRow(tr("Username"), dtf1lu);

	// Qt bug password deleted ui [Linux] [Qt6]
	QLineEdit* dtf1lp = new QLineEdit();
	QAction* dtf1lpr = dtf1lp->addAction(theme::icon("show"), QLineEdit::TrailingPosition);
	dtf1lpr->connect(dtf1lpr, &QAction::triggered, [=]() {
		if (dtf1lp->echoMode() == QLineEdit::Normal)
		{
#if defined(Q_OS_WIN) || defined(Q_OS_MAC) || defined(Q_OS_WASM)
			dtf1lp->setEchoMode(QLineEdit::PasswordEchoOnEdit);
#else
			dtf1lp->setEchoMode(QLineEdit::Password);
#endif
			dtf1lpr->setIcon(theme::icon("show"));
		}
		else
		{
			dtf1lp->setEchoMode(QLineEdit::Normal);
			dtf1lpr->setIcon(theme::icon("hide"));
		}
	});
	dtf1lp->setProperty("field", "password");
#if defined(Q_OS_WIN) || defined(Q_OS_MAC) || defined(Q_OS_WASM)
	dtf1lp->setEchoMode(QLineEdit::PasswordEchoOnEdit);
#else

#endif
	prefs[PREF_SECTIONS::Connections].emplace_back(dtf1lp);
	dtf1lp->setMinimumWidth(120);
	platform::osLineEdit(dtf1lp);
	dtf1->addRow(tr("Password"), dtf1lp);

	QGroupBox* dtl2 = new QGroupBox(tr("Configuration"));
	QFormLayout* dtf2 = new QFormLayout;
	dtf2->setFormAlignment(Qt::AlignLeading);

	QHBoxLayout* dtb20 = new QHBoxLayout;
	dtf2->addRow(tr("Transponders"), dtb20);
	QLineEdit* dtf2pt = new QLineEdit("/etc/tuxbox");
	dtf2pt->setProperty("field", "pathTransponders");
	prefs[PREF_SECTIONS::Connections].emplace_back(dtf2pt);
	platform::osLineEdit(dtf2pt);
	dtb20->addWidget(dtf2pt);
	dtb20->addWidget(new QLabel("<small>(satellites.xml)</small>"));

	QHBoxLayout* dtb21 = new QHBoxLayout;
	dtf2->addRow(tr("Services"), dtb21);
	QLineEdit* dtf2ps = new QLineEdit("/etc/enigma2");
	dtf2ps->setProperty("field", "pathServices");
	prefs[PREF_SECTIONS::Connections].emplace_back(dtf2ps);
	platform::osLineEdit(dtf2ps);
	dtb21->addWidget(dtf2ps);
	dtb21->addWidget(new QLabel("<small>(lamedb)</small>"));

	QHBoxLayout* dtb22 = new QHBoxLayout;
	dtf2->addRow(tr("Bouquets"), dtb22);
	QLineEdit* dtf2pb = new QLineEdit("/etc/enigma2");
	dtf2pb->setProperty("field", "pathBouquets");
	prefs[PREF_SECTIONS::Connections].emplace_back(dtf2pb);
	platform::osLineEdit(dtf2pb);
	dtb22->addWidget(dtf2pb);
	dtb22->addWidget(new QLabel("<small>(*.bouquet, *.userbouquet)</small>"));

	QHBoxLayout* dtb23 = new QHBoxLayout;
	dtf2->addRow(tr("Picons"), dtb23);
	QLineEdit* dtf2pi = new QLineEdit("/usr/share/enigma2/picon");
	dtf2pi->setProperty("field", "pathPicons");
	prefs[PREF_SECTIONS::Connections].emplace_back(dtf2pi);
	platform::osLineEdit(dtf2pi);
	dtb23->addWidget(dtf2pi);
	dtb23->addWidget(new QLabel("<small>(*.png)</small>"));

	QGroupBox* dtl3 = new QGroupBox(tr("Commands"));
	QFormLayout* dtf3 = new QFormLayout;
	dtf3->setFormAlignment(Qt::AlignLeading);
	dtf3->setFieldGrowthPolicy(QFormLayout::ExpandingFieldsGrow);

	QLineEdit* dtf3ca = new QLineEdit;
	dtf3ca->setProperty("field", "customWebifReloadUrl");
	prefs[PREF_SECTIONS::Connections].emplace_back(dtf3ca);
	platform::osLineEdit(dtf3ca);
	dtf3->addRow(tr("Custom webif reload URL address"), dtf3ca);

	QLineEdit* dtf3cc = new QLineEdit;
	dtf3cc->setProperty("field", "customTelnetReloadCmd");
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

	dtpage->setLayout(dtform);

	dtarea->setFrameRect(QRect());
	dtarea->setWidget(dtpage);
	dtarea->setWidgetResizable(true);
	dtarea->setStyleSheet("QScrollArea, #connections_page { background: transparent }");

	dtcnt->addLayout(dtvbox, 0);
	dtcnt->addWidget(dtarea, 1);
	rppage->setLayout(dtcnt);

	dtwid->addTab(rppage, tr("Connections", "dialog"));
}

void settings::preferencesLayout()
{
	QWidget* dtpage = new QWidget;
	dtpage->setObjectName("preferences_page");
	QScrollArea* dtarea = new QScrollArea;

#ifdef Q_OS_WIN
	if (! theme::isOverridden() && (theme::absLuma() || ! theme::isDefault()))
	{
		QStyle* style;

		if (theme::isFluentWin())
			style = QStyleFactory::create("windows11");
		else
			style = QStyleFactory::create("fusion");

		dtarea->verticalScrollBar()->setStyle(style);
		dtarea->horizontalScrollBar()->setStyle(style);
	}
#endif

	QHBoxLayout* dtcnt = new QHBoxLayout(dtpage);
	dtpage->setStyleSheet("QGroupBox { font-weight: bold }");

	QFormLayout* dtform = new QFormLayout;
	dtform->setSpacing(10);
	dtform->setFormAlignment(Qt::AlignLeading | Qt::AlignTop);

	QGroupBox* dtl0 = new QGroupBox(tr("General"));
	QFormLayout* dtf0 = new QFormLayout;
	dtf0->setSpacing(15);
	dtf0->setFormAlignment(Qt::AlignLeading);
	dtf0->setFieldGrowthPolicy(QFormLayout::FieldsStayAtSizeHint);

	QCheckBox* dtf0up = new QCheckBox(tr("Check for updates on launch"));
	dtf0up->setProperty("field", "autoCheckUpdate");
	prefs[PREF_SECTIONS::Preferences].emplace_back(dtf0up);
	dtf0->addRow(dtf0up);

	QCheckBox* dtf0ac = new QCheckBox(tr("Show confirmation messages when deleting"));
	dtf0ac->setProperty("field", "askConfirmation");
	prefs[PREF_SECTIONS::Preferences].emplace_back(dtf0ac);
	dtf0->addRow(dtf0ac);

	QGroupBox* dtl1 = new QGroupBox(tr("Language"));
	QFormLayout* dtf1 = new QFormLayout;
	dtf1->setSpacing(20);
	dtf1->setFormAlignment(Qt::AlignLeading);
	dtf1->setFieldGrowthPolicy(QFormLayout::FieldsStayAtSizeHint);

	QComboBox* dtf1ln = new QComboBox;
	dtf1ln->setProperty("field", "language");
	prefs[PREF_SECTIONS::Preferences].emplace_back(dtf1ln);
	dtf1ln->addItem(tr("Default (system language)"), "");
	dtf1ln->addItem(languageName(Language::English), languageCode(Language::English));
#ifndef E2SE_DEMO
	dtf1ln->addItem(languageName(Language::Arabic), languageCode(Language::Arabic));
	dtf1ln->addItem(languageName(Language::Bulgarian), languageCode(Language::Bulgarian));
	dtf1ln->addItem(languageName(Language::Catalan), languageCode(Language::Catalan));
	dtf1ln->addItem(languageName(Language::Czech), languageCode(Language::Czech));
	dtf1ln->addItem(languageName(Language::Danish), languageCode(Language::Danish));
	dtf1ln->addItem(languageName(Language::German), languageCode(Language::German));
	dtf1ln->addItem(languageName(Language::English_UnitedKingdom), languageCode(Language::English_UnitedKingdom));
	dtf1ln->addItem(languageName(Language::English_UnitedStates), languageCode(Language::English_UnitedStates));
	dtf1ln->addItem(languageName(Language::Spanish), languageCode(Language::Spanish));
	dtf1ln->addItem(languageName(Language::Persian), languageCode(Language::Persian));
	dtf1ln->addItem(languageName(Language::Finnish), languageCode(Language::Finnish));
	dtf1ln->addItem(languageName(Language::French), languageCode(Language::French));
	dtf1ln->addItem(languageName(Language::Gaelic), languageCode(Language::Gaelic));
	dtf1ln->addItem(languageName(Language::Galician), languageCode(Language::Galician));
	dtf1ln->addItem(languageName(Language::Hebrew), languageCode(Language::Hebrew));
	dtf1ln->addItem(languageName(Language::Croatian), languageCode(Language::Croatian));
	dtf1ln->addItem(languageName(Language::Hungarian), languageCode(Language::Hungarian));
	dtf1ln->addItem(languageName(Language::Italian), languageCode(Language::Italian));
	dtf1ln->addItem(languageName(Language::Japanese), languageCode(Language::Japanese));
	dtf1ln->addItem(languageName(Language::Korean), languageCode(Language::Korean));
	dtf1ln->addItem(languageName(Language::Lithuanian), languageCode(Language::Lithuanian));
	dtf1ln->addItem(languageName(Language::Latvian), languageCode(Language::Latvian));
	dtf1ln->addItem(languageName(Language::Dutch), languageCode(Language::Dutch));
	dtf1ln->addItem(languageName(Language::NorwegianNynorsk), languageCode(Language::NorwegianNynorsk));
	dtf1ln->addItem(languageName(Language::Polish), languageCode(Language::Polish));
	dtf1ln->addItem(languageName(Language::Portuguese_Brazil), languageCode(Language::Portuguese_Brazil));
	dtf1ln->addItem(languageName(Language::Portuguese_Portugal), languageCode(Language::Portuguese_Portugal));
	dtf1ln->addItem(languageName(Language::Russian), languageCode(Language::Russian));
	dtf1ln->addItem(languageName(Language::Slovak), languageCode(Language::Slovak));
	dtf1ln->addItem(languageName(Language::Slovenian), languageCode(Language::Slovenian));
	dtf1ln->addItem(languageName(Language::Swedish), languageCode(Language::Swedish));
	dtf1ln->addItem(languageName(Language::Turkish), languageCode(Language::Turkish));
	dtf1ln->addItem(languageName(Language::Ukrainian), languageCode(Language::Ukrainian));
	dtf1ln->addItem(languageName(Language::Chinese_China), languageCode(Language::Chinese_China));
	dtf1ln->addItem(languageName(Language::Chinese_Taiwan), languageCode(Language::Chinese_Taiwan));
#endif
	platform::osComboBox(dtf1ln);
	dtf1->addRow(dtf1ln);
	dtf1->addRow(new QLabel(QString("<small>%1</small>").arg(tr("The software might need to be restarted."))));

	QGroupBox* dtl2 = new QGroupBox(tr("Theme"));
	QFormLayout* dtf2 = new QFormLayout;
	dtf2->setSpacing(20);
	dtf2->setFormAlignment(Qt::AlignLeading);
	dtf2->setFieldGrowthPolicy(QFormLayout::FieldsStayAtSizeHint);

	QComboBox* dtf2td = new QComboBox;
	dtf2td->setProperty("field", "theme");
	prefs[PREF_SECTIONS::Preferences].emplace_back(dtf2td);
	dtf2td->addItem(tr("Default (system theme)"), "");
	dtf2td->addItem(tr("Dark"), "dark");
	dtf2td->addItem(tr("Light"), "light");
	platform::osComboBox(dtf2td);
	dtf2->addRow(dtf2td);

	QCheckBox* dtf2oe = new QCheckBox(tr("Enable experimental features"));
	dtf2oe->setProperty("field", "osExperiment");
	prefs[PREF_SECTIONS::Preferences].emplace_back(dtf2oe);
	dtf2oe->setChecked(true);
	dtf2->addRow(dtf2oe);
	dtf2->addRow(new QLabel(QString("<small>%1</small>").arg(tr("The software might need to be restarted."))));

	QGroupBox* dtl3 = new QGroupBox(tr("Drag and Drop"));
	QFormLayout* dtf3 = new QFormLayout;
	dtf3->setSpacing(20);
	dtf3->setFormAlignment(Qt::AlignLeading);
	dtf3->setFieldGrowthPolicy(QFormLayout::FieldsStayAtSizeHint);

	QCheckBox* dtf3sc = new QCheckBox(tr("Switch to current bouquet item after the drop"));
	dtf3sc->setProperty("field", "treeCurrentAfterDrop");
	dtf3sc->setChecked(true);
	prefs[PREF_SECTIONS::Preferences].emplace_back(dtf3sc);
	dtf3->addRow(dtf3sc);

	QFormLayout* dtf31 = new QFormLayout;
	dtf31->setSpacing(10);
	dtf31->setFormAlignment(Qt::AlignLeading);
	dtf31->setFieldGrowthPolicy(QFormLayout::FieldsStayAtSizeHint);

	QLabel* dth31 = new QLabel(tr("Channel operations"));
	dth31->setStyleSheet("min-height: 20px");
	dth31->setAlignment(dth31->layoutDirection() == Qt::LeftToRight ? Qt::AlignLeft : Qt::AlignRight);
	dtf31->addRow(dth31);

	QButtonGroup* dtg3 = new QButtonGroup;
	dtg3->setExclusive(true);

	QCheckBox* dtf3ci = new QCheckBox(tr("Copy channels (preserving)"));
	dtf3ci->setProperty("field", "treeDropCopy");
	prefs[PREF_SECTIONS::Preferences].emplace_back(dtf3ci);
	dtf3ci->setChecked(true);
	dtg3->addButton(dtf3ci);
	dtf31->addRow(dtf3ci);

	QCheckBox* dtf3mi = new QCheckBox(tr("Move channels (deleting)"));
	dtf3mi->setProperty("field", "treeDropMove");
	prefs[PREF_SECTIONS::Preferences].emplace_back(dtf3mi);
	dtg3->addButton(dtf3mi);
	dtf31->addRow(dtf3mi);

	dtf3->addRow(dtf31);

	QGroupBox* dtl4 = new QGroupBox(tr("Picons editor"));
	QFormLayout* dtf4 = new QFormLayout;
	dtf4->setSpacing(20);
	dtf4->setFormAlignment(Qt::AlignLeading);
	dtf4->setFieldGrowthPolicy(QFormLayout::FieldsStayAtSizeHint);

	QCheckBox* dtf4bp = new QCheckBox(tr("Backup picon when replaced"));
	dtf4bp->setProperty("field", "piconsBackup");
	dtf4bp->setChecked(true);
	prefs[PREF_SECTIONS::Preferences].emplace_back(dtf4bp);
	dtf4->addRow(dtf4bp);

	QFormLayout* dtf41 = new QFormLayout;
	dtf41->setSpacing(10);
	dtf41->setFormAlignment(Qt::AlignLeading);
	dtf41->setFieldGrowthPolicy(QFormLayout::FieldsStayAtSizeHint);

	QLabel* dth41 = new QLabel(tr("Filename format"));
	dth41->setStyleSheet("min-height: 20px");
	dth41->setAlignment(dth41->layoutDirection() == Qt::LeftToRight ? Qt::AlignLeft : Qt::AlignRight);
	dtf41->addRow(dth41);

	QButtonGroup* dtg4 = new QButtonGroup;
	dtg4->setExclusive(true);

	QCheckBox* dtf4ri = new QCheckBox(tr("Use reference ID"));
	dtf4ri->setProperty("field", "piconsUseRefid");
	prefs[PREF_SECTIONS::Preferences].emplace_back(dtf4ri);
	dtf4ri->setChecked(true);
	dtg4->addButton(dtf4ri);
	dtf41->addRow(dtf4ri);

	QCheckBox* dtf4sn = new QCheckBox(tr("Use service name"));
	dtf4sn->setProperty("field", "piconsUseChname");
	prefs[PREF_SECTIONS::Preferences].emplace_back(dtf4sn);
	dtg4->addButton(dtf4sn);
	dtf41->addRow(dtf4sn);

	dtf4->addRow(dtf41);

	dtl0->setLayout(dtf0);
	dtl1->setLayout(dtf1);
	dtl2->setLayout(dtf2);
	dtl3->setLayout(dtf3);
	dtl4->setLayout(dtf4);

	dtform->addItem(new QSpacerItem(0, 0));
	dtform->addWidget(dtl0);
	dtform->addItem(new QSpacerItem(0, 5));
	dtform->addWidget(dtl1);
	dtform->addItem(new QSpacerItem(0, 5));
	dtform->addWidget(dtl2);
	dtform->addItem(new QSpacerItem(0, 5));
	dtform->addWidget(dtl3);
	dtform->addItem(new QSpacerItem(0, 5));
	dtform->addWidget(dtl4);
	dtform->addItem(new QSpacerItem(0, 0));

	dtcnt->setAlignment(Qt::AlignLeading | Qt::AlignTop);
	dtcnt->addLayout(dtform, 0);
	dtpage->setLayout(dtcnt);

	dtarea->setFrameRect(QRect());
	dtarea->setWidget(dtpage);
	dtarea->setWidgetResizable(true);
	dtarea->setStyleSheet("QScrollArea, #preferences_page { background: transparent }");

	dtwid->addTab(dtarea, tr("Preferences", "dialog"));
}

void settings::engineLayout()
{
	QWidget* dtpage = new QWidget;
	dtpage->setObjectName("engine_page");
	QScrollArea* dtarea = new QScrollArea;

#ifdef Q_OS_WIN
	if (! theme::isOverridden() && (theme::absLuma() || ! theme::isDefault()))
	{
		QStyle* style;

		if (theme::isFluentWin())
			style = QStyleFactory::create("windows11");
		else
			style = QStyleFactory::create("fusion");

		dtarea->verticalScrollBar()->setStyle(style);
		dtarea->horizontalScrollBar()->setStyle(style);
	}
#endif

	QHBoxLayout* dtcnt = new QHBoxLayout(dtpage);
	dtpage->setStyleSheet("QGroupBox { font-weight: bold }");

	QFormLayout* dtform = new QFormLayout;
	dtform->setSpacing(10);
	dtform->setFormAlignment(Qt::AlignLeading | Qt::AlignTop);

	QGroupBox* dtl0 = new QGroupBox(tr("Settings"));
	QFormLayout* dtf0 = new QFormLayout;
	dtf0->setSpacing(20);
	dtf0->setFormAlignment(Qt::AlignLeading);
	dtf0->setFieldGrowthPolicy(QFormLayout::FieldsStayAtSizeHint);

	QComboBox* dtf0td = new QComboBox;
	dtf0td->setProperty("field", "dbTypeDefault");
	prefs[PREF_SECTIONS::Engine].emplace_back(dtf0td);

	// straight copy of e2db_abstract::FPORTS
	dtf0td->addItem(tr("Lamedb 2.4 [Enigma 2]"), 0x1224);
	dtf0td->addItem(tr("Lamedb 2.5 [Enigma 2]"), 0x1225);
	dtf0td->addItem(tr("Lamedb 2.3 [Enigma 1]"), 0x1223);
	dtf0td->addItem(tr("Lamedb 2.2 [Enigma 1]"), 0x1222);
	dtf0td->addItem(tr("Zapit api-v4 [Neutrino]"), 0x1014);
	dtf0td->addItem(tr("Zapit api-v3 [Neutrino]"), 0x1013);
	dtf0td->addItem(tr("Zapit api-v2 [Neutrino]"), 0x1012);
	dtf0td->addItem(tr("Zapit api-v1 [Neutrino]"), 0x1011);

	platform::osComboBox(dtf0td);
	dtf0->addRow(tr("Default format"), dtf0td);

	QCheckBox* dtf1li = new QCheckBox(tr("Parental lock whitelist (exclusion instead inclusion list)"));
	dtf1li->setProperty("field", "parentalLockInvert");
	prefs[PREF_SECTIONS::Engine].emplace_back(dtf1li);
	dtf0->addRow(dtf1li);

	QGroupBox* dtl1 = new QGroupBox(tr("Preferences"));
	QFormLayout* dtf1 = new QFormLayout;
	dtf1->setSpacing(15);
	dtf1->setFormAlignment(Qt::AlignLeading);
	dtf1->setFieldGrowthPolicy(QFormLayout::FieldsStayAtSizeHint);

	QCheckBox* dtf1mi = new QCheckBox(tr("Use a global marker index (will change ID for markers)"));
	dtf1mi->setProperty("field", "markerGlobalIndex");
	prefs[PREF_SECTIONS::Engine].emplace_back(dtf1mi);
	dtf1->addRow(dtf1mi);

	QCheckBox* dtf1mx = new QCheckBox(tr("Sort by ID when merging lists (slower merge)"));
	dtf1mx->setProperty("field", "mergeSortId");
	prefs[PREF_SECTIONS::Engine].emplace_back(dtf1mx);
	dtf1->addRow(dtf1mx);

	QCheckBox* dtf1cf = new QCheckBox(tr("Match favourite reference with services"));
	dtf1cf->setProperty("field", "favouriteMatchService");
	dtf1cf->setChecked(true);
	prefs[PREF_SECTIONS::Engine].emplace_back(dtf1cf);
	dtf1->addRow(dtf1cf);

	QGroupBox* dtl2 = new QGroupBox(tr("Tools"));
	QHBoxLayout* dtb2 = new QHBoxLayout;

	QFormLayout* dtf20 = new QFormLayout;
	dtf20->setSpacing(10);
	dtf20->setFormAlignment(Qt::AlignLeading);
	dtf20->setFieldGrowthPolicy(QFormLayout::FieldsStayAtSizeHint);

	QLabel* dth20 = new QLabel(tr("CSV Import/Export"));
	dth20->setStyleSheet("min-height: 20px");
	dth20->setAlignment(dth20->layoutDirection() == Qt::LeftToRight ? Qt::AlignLeft : Qt::AlignRight);
	dtf20->addRow(dth20);

	QCheckBox* dtf2th = new QCheckBox(tr("Allow header columns in CSV"));
	dtf2th->setProperty("field", "toolsCsvHeader");
	prefs[PREF_SECTIONS::Engine].emplace_back(dtf2th);
	dtf20->addRow(dtf2th);
	dtf20->addItem(new QSpacerItem(0, 10));

	QLineEdit* dtf2cd = new QLineEdit("\n");
	dtf2cd->setProperty("field", "toolsCsvDelimiter");
	prefs[PREF_SECTIONS::Engine].emplace_back(dtf2cd);
	//TODO CRLF
	// dtf2cd->setMaxLength(6);
	dtf2cd->setMaxLength(3);
	dtf2cd->setMaximumWidth(50);
	dtf2cd->connect(dtf2cd, &QLineEdit::textChanged, [=](const QString& text) { this->textDoubleEscaped(dtf2cd, text); });
	platform::osLineEdit(dtf2cd);
	dtf20->addRow(tr("CSV delimiter character"), dtf2cd);

	QLineEdit* dtf2cs = new QLineEdit(",");
	dtf2cs->setProperty("field", "toolsCsvSeparator");
	prefs[PREF_SECTIONS::Engine].emplace_back(dtf2cs);
	dtf2cs->setMaxLength(3);
	dtf2cs->setMaximumWidth(50);
	dtf2cs->connect(dtf2cs, &QLineEdit::textChanged, [=](const QString& text) { this->textDoubleEscaped(dtf2cs, text); });
	platform::osLineEdit(dtf2cs);
	dtf20->addRow(tr("CSV separator character"), dtf2cs);

	QLineEdit* dtf2ce = new QLineEdit("\"");
	dtf2ce->setProperty("field", "toolsCsvEscape");
	prefs[PREF_SECTIONS::Engine].emplace_back(dtf2ce);
	dtf2ce->setMaxLength(1);
	dtf2ce->setMaximumWidth(50);
	dtf2ce->connect(dtf2ce, &QLineEdit::textChanged, [=](const QString& text) { this->textRemoveEscaped(dtf2ce, text); });
	platform::osLineEdit(dtf2ce);
	dtf20->addRow(tr("CSV escape character"), dtf2ce);

	QFormLayout* dtf21 = new QFormLayout;
	dtf21->setSpacing(10);
	dtf21->setFormAlignment(Qt::AlignLeading);
	dtf21->setFieldGrowthPolicy(QFormLayout::FieldsStayAtSizeHint);

	QLabel* dth21 = new QLabel(tr("Fields Import/Export"));
	dth21->setStyleSheet("min-height: 20px");
	dth21->setAlignment(dth21->layoutDirection() == Qt::LeftToRight ? Qt::AlignLeft : Qt::AlignRight);
	dtf21->addRow(dth21);

	QButtonGroup* dtg2 = new QButtonGroup;
	dtg2->setExclusive(true);

	QCheckBox* dtf2df = new QCheckBox(tr("Default (same fields as visual)"));
	dtf2df->setProperty("field", "toolsFieldsDefault");
	prefs[PREF_SECTIONS::Engine].emplace_back(dtf2df);
	dtf2df->setChecked(true);
	dtg2->addButton(dtf2df);
	dtf21->addRow(dtf2df);

	QCheckBox* dtf2ef = new QCheckBox(tr("Extended (all fields)"));
	dtf2ef->setProperty("field", "toolsFieldsExtended");
	prefs[PREF_SECTIONS::Engine].emplace_back(dtf2ef);
	dtg2->addButton(dtf2ef);
	dtf21->addRow(dtf2ef);

	dtb2->addLayout(dtf20);
	dtb2->addItem(new QSpacerItem(20, 0));
	dtb2->addLayout(dtf21);
	dtb2->addItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Preferred));

	dtl0->setLayout(dtf0);
	dtl1->setLayout(dtf1);
	dtl2->setLayout(dtb2);

	dtform->addItem(new QSpacerItem(0, 0));
	dtform->addWidget(dtl0);
	dtform->addItem(new QSpacerItem(0, 0));
	dtform->addWidget(dtl1);
	dtform->addItem(new QSpacerItem(0, 5));
	dtform->addWidget(dtl2);
	dtform->addItem(new QSpacerItem(0, 0));

	dtcnt->setAlignment(Qt::AlignLeading | Qt::AlignTop);
	dtcnt->addLayout(dtform, 0);
	dtpage->setLayout(dtcnt);

	dtarea->setFrameRect(QRect());
	dtarea->setWidget(dtpage);
	dtarea->setWidgetResizable(true);
	dtarea->setStyleSheet("QScrollArea, #engine_page { background: transparent }");

	dtwid->addTab(dtarea, tr("Engine", "dialog"));
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
#ifdef Q_OS_WIN
	if (! theme::isOverridden() && (theme::absLuma() || ! theme::isDefault()))
	{
		QStyle* style;

		if (theme::isFluentWin())
			style = QStyleFactory::create("windows11");
		else
			style = QStyleFactory::create("fusion");

		adtbl->verticalScrollBar()->setStyle(style);
		adtbl->horizontalScrollBar()->setStyle(style);
		adtbl->horizontalHeader()->setStyle(style);
	}
#endif
	platform::osPersistentEditor(adtbl);

	this->adntc = new QWidget;
	QGridLayout* dtntcg = new QGridLayout;
	QHBoxLayout* dtnthb = new QHBoxLayout;
	QWidget* dtntsp = new QWidget;
	dtntsp->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

	//: HTML formattation: %1text%2%text%3 treat them as spaces
	QLabel* dtntcl = new QLabel(tr("%1Please be carefull!%2Modifing these settings could break the program.%3").arg("<h2>").arg("</h2><br><p>").arg("</p>"));
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

	dtwid->addTab(dtpage, tr("Advanced", "dialog"));
}

void settings::themeChanged()
{
	debug("themeChanged");

	theme->changed();
}

QListWidgetItem* settings::addProfile(int idx)
{
	if (idx == -1)
	{
		idx = int (tmpps.size());

		QString name = tr("Profile") + ' ' + QString::number(idx);

		map<QString, QVariant> values = {
			{"profileName", name},
			{"ipAddress", "192.168.0.2"},
			{"ftpPort", 21},
			{"ftpActive", false},
			{"httpPort", 80},
			{"username", "root"},
			{"password", ""},
			{"pathTransponders", "/etc/tuxbox"},
			{"pathServices", "/etc/enigma2"},
			{"pathBouquets", "/etc/enigma2"},
			{"pathPicons", "/usr/share/enigma2/picon"},
			{"customWebifReloadUrl", ""},
			{"customTelnetReloadCmd", ""}
		};

		tmpps.emplace(idx, values);
	}

	debug("addProfile", "item", idx);

	QString name = tmpps[idx]["profileName"].toString();

	QListWidgetItem* item = new QListWidgetItem(name, rplist);
	item->setData(Qt::UserRole, idx);
	item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEditable | Qt::ItemIsEnabled);

	if (! this->state.retr)
	{
		renameProfile(false);
		item->setSelected(true);
		rplist->setCurrentItem(item);

		if (rplist->count() != 1)
			renameProfile(true);
	}
	return item;
}

void settings::deleteProfile()
{
	debug("deleteProfile");

	QList<QListWidgetItem*> selected = rplist->selectedItems();

	if (selected.empty() || rplist->count() == 0)
		return;

	// note: do not remove default profile
	if (rplist->count() == 1)
	{
		return debug("deleteProfile", "inhibit", "default");
	}

	for (auto & item : selected)
	{
		this->deleteProfile(item);
	}
}

void settings::deleteProfile(QListWidgetItem* item)
{
	int row = rplist->row(item);

	debug("deleteProfile", "row", row);

	int idx = item->data(Qt::UserRole).toInt();
	tmpps[idx].clear();

	this->state.dele = true;

	renameProfile(false);

	rplist->takeItem(row);
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
	int row = item != nullptr ? rplist->row(item) : -1;

	debug("updateProfile", "row", row);

	if (item == nullptr)
		return;

	int idx = item->data(Qt::UserRole).toInt();

	for (auto & item : prefs[PREF_SECTIONS::Connections])
	{
		QString pref = item->property("field").toString();

		if (QLineEdit* field = qobject_cast<QLineEdit*>(item))
		{
			QString val = field->text();

			if (pref == "password")
			{
				QByteArray ba (val.toUtf8());
				val = ba.toBase64();
			}

			tmpps[idx][pref] = val;
		}
		else if (QCheckBox* field = qobject_cast<QCheckBox*>(item))
		{
			tmpps[idx][pref] = field->isChecked();
		}
	}
}

void settings::importProfile()
{
	debug("importProfile");

#ifdef E2SE_DEMO
	return gid->demoMessage();
#endif

	vector<string> paths;

	paths = gid->importFileDialog(gui::GUI_DPORTS::ConnectionProfile);

	if (paths.empty())
	{
		return;
	}

	theme::setWaitCursor();

	updateProfile(rplist->currentItem());

	QListWidgetItem* current = nullptr;

	for (auto & path : paths)
	{
		if (path.empty())
		{
			continue;
		}

		QString filepath = QString::fromStdString(path);
		QFile file (filepath);

		if (! file.open(QIODevice::ReadOnly | QIODevice::Text))
		{
			theme::unsetWaitCursor();

			error("importProfile", tr("File Error", "error").toStdString(), tr("Error reading file \"%1\".", "error").arg(path.data()).toStdString());

			errorMessage(tr("File Error", "error"), tr("Error opening files.", "error"));

			return;
		}

		try
		{
			QXmlStreamReader xml (&file);
			xml.setNamespaceProcessing(false);

			int step = 0;
			bool valid = false;
			QString name, val;
			QXmlStreamAttributes attrs;

			map<QString, QVariant> values;

			while (! xml.atEnd())
			{
				xml.readNext();

				if (! step && xml.name().toString() == "e2se")
				{
					if (xml.isStartElement())
						step = 1;
					else if (xml.isEndElement())
						valid = true;
				}
				else if (step && xml.name().toString() == "Servers")
				{
					if (xml.isStartElement())
						step = 2;
					else if (xml.isEndElement())
						step = 0;
				}
				else if (step && xml.name().toString() == "Server")
				{
					if (xml.isStartElement())
						step = 3;
					else if (xml.isEndElement())
						step = 1;
				}
				else if (step == 3)
				{
					if (xml.isStartElement())
					{
						name = xml.name().toString();
						attrs = xml.attributes();
						continue;
					}
					else if (xml.isCharacters())
					{
						val = xml.text().toString();
						continue;
					}
					else if (xml.isEndElement() && xml.name().toString() == name)
					{
						step = 3;
					}
					else
					{
						continue;
					}

					QString pref;

					if (name == "ProfileName")
						pref = "profileName";
					else if (name == "Host")
						pref = "ipAddress";
					else if (name == "Port")
						pref = "ftpPort";
					else if (name == "User")
						pref = "username";
					else if (name == "Pass")
						pref = "Pass";
					else if (name == "PasvMode")
						pref = "ftpActive";
					else if (name == "PathTransponders")
						pref = "pathTransponders";
					else if (name == "PathServices")
						pref = "pathServices";
					else if (name == "PathBouquets")
						pref = "pathBouquets";
					else if (name == "PathPicons")
						pref = "pathPicons";
					else if (name == "HttpPort")
						pref = "httpPort";
					else if (name == "CustomWebifReloadUrl")
						pref = "customWebifReloadUrl";
					else if (name == "CustomTelnetReloadCmd")
						pref = "customTelnetReloadCmd";
					else
						continue;

					if (! pref.isEmpty())
					{
						QVariant value;

						if (name == "Pass")
						{
							if (attrs.hasAttribute("encoding"))
							{
								if (attrs.value("encoding").toString() == "base64")
								{
									QByteArray ba (val.toUtf8());

									if (! QByteArray::fromBase64(ba, QByteArray::AbortOnBase64DecodingErrors).isEmpty())
									{
										value = val;
									}
								}
							}
							else
							{
								QByteArray ba (val.toUtf8());
								val = ba.toBase64();
								value = val;
							}
						}
						else if (name == "ftpActive")
						{
							value = (val == "MODE_ACTIVE");
						}
						else
						{
							value = val;
						}

						values[pref] = value;
					}

					name = val = QString();
					attrs = QXmlStreamAttributes();
				}
			}

			if (xml.hasError())
			{
				error("importProfile", tr("Error", "error").toStdString(), xml.errorString().toStdString());

				throw std::runtime_error("QXmlStreamReader");
			}
			else if (valid)
			{
				int idx = int (tmpps.size());

				tmpps.emplace(idx, values);

				QString name = values["profileName"].toString();

				QListWidgetItem* item = new QListWidgetItem(name, rplist);
				item->setData(Qt::UserRole, idx);
				item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEditable | Qt::ItemIsEnabled);

				current = item;
			}
			else
			{
				theme::unsetWaitCursor();

				error("importProfile", tr("Error", "error").toStdString(), tr("Malformed or unknown XML file format.", "error").toStdString());

				errorMessage(tr("Error", "error"), tr("Malformed or unknown XML file format.", "error"));

				return;
			}
		}
		catch (...)
		{
			theme::unsetWaitCursor();

			error("importProfile", tr("Error", "error").toStdString(), tr("Error reading file \"%1\".", "error").arg(path.data()).toStdString());

			errorMessage(tr("Error", "error"), tr("Error reading files.", "error"));
		}
	}

	theme::unsetWaitCursor();

	if (current != nullptr)
	{
		rplist->reset(); // should reset QListView::currentIndex
		rplist->setCurrentItem(current);
	}
}

void settings::exportProfile()
{
	debug("exportProfile");

#ifdef E2SE_DEMO
	return gid->demoMessage();
#endif

	QList<QListWidgetItem*> items = rplist->selectedItems();

	string filename = "e2se";

	if (items.count() == 1)
	{
		QListWidgetItem* item = rplist->currentItem();
		filename = item->text().toStdString();
	}
	else
	{
		filename = "e2se";
	}

	filename.append(".profile");

	vector<int> rows;

	if (items.empty())
	{
		for (auto & x : tmpps)
		{
			rows.emplace_back(x.first);
		}
	}
	else
	{
		for (auto & item : items)
		{
			rows.emplace_back(item->data(Qt::UserRole).toInt());
		}
	}

	string path = gid->exportFileDialog(gui::GUI_DPORTS::ConnectionProfile, filename);

	if (path.empty())
	{
		return;
	}

	theme::setWaitCursor();

	updateProfile(rplist->currentItem());

	QString filepath = QString::fromStdString(path);
	QFile file (filepath);

	if (! file.open(QIODevice::WriteOnly | QIODevice::Text))
	{
		theme::unsetWaitCursor();

		error("exportProfile", tr("File Error", "error").toStdString(), tr("Error writing file \"%1\".", "error").arg(path.data()).toStdString());

		errorMessage(tr("File Error", "error"), tr("Error opening files.", "error"));

		return;
	}

	vector<QString> keys = {
		"profileName",
		"ipAddress",
		"ftpPort",
		"username",
		"password",
		"ftpActive",
		"_pathServices",
		"pathTransponders",
		"pathServices",
		"pathBouquets",
		"pathPicons",
		"httpPort",
		"customWebifReloadUrl",
		"customTelnetReloadCmd"
	};

	try
	{
		QXmlStreamWriter xml (&file);
		xml.setAutoFormatting(true);
		xml.writeStartDocument("1.0");
		xml.writeStartElement("e2se");
		xml.writeAttribute("version", QApplication::applicationVersion());
		xml.writeStartElement("Servers");

		for (int & idx : rows)
		{
			if (tmpps[idx].size())
			{
				xml.writeStartElement("Server");

				for (QString & pref : keys)
				{
					QString _pref = pref;

					if (pref == "_pathServices")
						_pref = "pathServices";

					QString name;
					QVariant value = tmpps[idx][_pref];

					if (pref == "ipAddress")
						name = "Host";
					else if (pref == "ftpPort")
						name = "Port";
					else if (pref == "username")
						name = "User";
					else if (pref == "password")
						name = "Pass";
					else if (pref == "_pathServices")
						name = "RemoteDir";
					else if (pref == "ftpActive")
					{
						name = "PasvMode";
						value = value.toBool() ? "MODE_ACTIVE" : "MODE_DEFAULT";
					}
					else
					{
						_pref.replace(0, 1, _pref[0].toUpper());
						name = QString("e2se:").append(_pref);
					}

					if (pref == "password")
					{
						xml.writeStartElement(name);
						xml.writeAttribute("encoding", "base64");
						xml.writeCharacters(value.toString());
						xml.writeEndElement();
					}
					else
					{
						xml.writeTextElement(name, value.toString());
					}
				}

				xml.writeEndElement();
			}
		}

		xml.writeEndElement();
		xml.writeEndElement();
		xml.writeEndDocument();

		if (xml.hasError())
		{
			throw std::runtime_error("QXmlStreamWriter");
		}
	}
	catch (...)
	{
		theme::unsetWaitCursor();

		error("exportProfile", tr("Error", "error").toStdString(), tr("Error writing file \"%1\".", "error").arg(path.data()).toStdString());

		errorMessage(tr("Error", "error"), tr("Error writing files.", "error"));
	}

	theme::unsetWaitCursor();

	infoMessage(tr("Saved!", "message"));
}

QMenu* settings::profileMenu()
{
	QMenu* menu = new QMenu;

	QList<QListWidgetItem*> selected = rplist->selectedItems();

	bool editable = false;

	if (selected.count() == 1)
	{
		editable = true;
	}

	{
		QAction* action = new QAction;
		action->setText(tr("Import Profiles…"));
		action->connect(action, &QAction::triggered, [=]() { this->importProfile(); });
		menu->addAction(action);
	}
	{
		QAction* action = new QAction;
		action->setText(tr("Export Profile(s)…", nullptr, int (selected.count())));
		action->connect(action, &QAction::triggered, [=]() { this->exportProfile(); });
		menu->addAction(action);
	}
	menu->addSeparator();
	{
		QWidgetAction* action = new QWidgetAction(nullptr);
		QLabel* label = new QLabel(tr("Presets"));
#ifndef Q_OS_MAC
		label->setStyleSheet("QLabel { margin: 5px 10px }");
#else
		label->setStyleSheet("QLabel { margin: 5px 10px; font-weight: bold }");
#endif
		action->setDefaultWidget(label);
		menu->addAction(action);
	}
	for (connectionPresets::PRESET & preset : connectionPresets::presets()) {
		QAction* action = new QAction;
		QString text;

		switch (preset)
		{
			case connectionPresets::enigma_24: text = "Enigma 2.4"; break;
			case connectionPresets::neutrino: text = "Neutrino"; break;
			case connectionPresets::enigma_23: text = "Enigma 2.3 / 2.4"; break;
			case connectionPresets::gx_24: text = "GX NationalChip Octagon 2.4"; break;
			case connectionPresets::dddragon: text = "Triple Dragon"; break;
			case connectionPresets::wtplay: text = "WeTek Play"; break;
			case connectionPresets::enigma_1: text = "Enigma 1"; break;
		}

		action->setText(text);
		action->setEnabled(editable);
		action->connect(action, &QAction::triggered, [=]() { this->applyPreset(preset); });
		menu->addAction(action);
	}

	return menu;
}

void settings::profileNameChanged(QString text)
{
	QListWidgetItem* item = rplist->currentItem();
	int row = item != nullptr ? rplist->row(item) : -1;

	debug("profileNameChanged", "row", row);

	if (item == nullptr)
		return;

	int idx = item->data(Qt::UserRole).toInt();

	if (QApplication::layoutDirection() == Qt::RightToLeft)
	{
		item->setText(item->text().append(QChar(0x200e))); // LRM
	}

	tmpps[idx]["profileName"] = text;
}

void settings::currentProfileChanged(QListWidgetItem* current, QListWidgetItem* previous)
{
	debug("currentProfileChanged");

	if (previous != nullptr && ! this->state.dele)
		updateProfile(previous);

	if (current != nullptr)
		this->retrieve(current);

	this->state.dele = false;
}

void settings::showProfileEditContextMenu(QPoint& pos)
{
	debug("showProfileEditContextMenu");

	QList<QListWidgetItem*> selected = rplist->selectedItems();

	if (selected.empty() && rplist->count() != 0)
		return;

	bool editable = false;

	if (selected.count() == 1)
	{
		editable = true;
	}

	QMenu* list_edit = contextMenu();

	contextMenuAction(list_edit, tr("Export Profile", "context-menu"), [=]() { this->exportProfile(); }, editable);
	contextMenuSeparator(list_edit);
	contextMenuAction(list_edit, tr("Rename", "context-menu"), [=]() { this->renameProfile(true); }, editable);
	contextMenuAction(list_edit, tr("&Delete", "context-menu"), [=]() { this->deleteProfile(); });

	platform::osMenuPopup(list_edit, rplist, pos);
}

void settings::applyPreset(connectionPresets::PRESET preset)
{
	debug("applyPreset", "choice", int (preset));

	map<string, string> values;

	try
	{
		values = connectionPresets::call(preset);
	}
	catch (...)
	{
		return error("applyPreset", tr("Error", "error").toStdString(), tr("Preset Error", "error").toStdString());
	}

	for (auto & item : prefs[PREF_SECTIONS::Connections])
	{
		QString pref = item->property("field").toString();
		string key = pref.toStdString();

		if (! values.count(key))
		{
			continue;
		}
		if (QLineEdit* field = qobject_cast<QLineEdit*>(item))
		{
			field->setText(QString::fromStdString(values[key]));
		}
		else if (QCheckBox* field = qobject_cast<QCheckBox*>(item))
		{
			field->setChecked(values[key] == "true");
		}
	}
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
		int idx = int (i);

		sets->setArrayIndex(idx);

		if (tmpps[idx].size())
		{
			if (! sets->contains("profileName"))
				size++;
			for (auto & field : tmpps[idx])
				sets->setValue(field.first, field.second);
		}
		else
		{
			sets->remove("profileName");
			for (auto & item : prefs[PREF_SECTIONS::Connections])
			{
				QString pref = item->property("field").toString();
				sets->remove(pref);
			}
		}
	}
	sets->setValue("telnetPort", 23);
	sets->endArray();
	sets->setValue("profile/size", size);

	for (int i = 1; i < 3; i++)
	{
		QString prefix;
		switch (i)
		{
			case PREF_SECTIONS::Preferences: prefix = "preference"; break;
			case PREF_SECTIONS::Engine: prefix = "engine"; break;
		}

		sets->beginGroup(prefix);

		for (auto & item : prefs[i])
		{
			QString pref = item->property("field").toString();

			if (QCheckBox* field = qobject_cast<QCheckBox*>(item))
				sets->setValue(pref, field->isChecked());
			else if (QLineEdit* field = qobject_cast<QLineEdit*>(item))
				sets->setValue(pref, field->text());
			else if (QComboBox* field = qobject_cast<QComboBox*>(item))
				sets->setValue(pref, field->currentData());
		}
		sets->endGroup();
	}
}

void settings::store(QTableWidget* adtbl)
{
	debug("store", "index", 3);

	for (int i = 0; i < adtbl->rowCount(); i++)
	{
		QString pref = adtbl->item(i, 0)->text().replace(".", "/");
		QString value = adtbl->item(i, 1)->text();

		if (QApplication::layoutDirection() == Qt::RightToLeft)
		{
			value.remove(QChar(0x200e)); // LRM
		}

		if (value.contains(QRegularExpression("false|true")))
			sets->setValue(pref, (value == "true" ? true : false));
		else
			sets->setValue(pref, value);
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

		int idx = i;
		tmpps[idx]["profileName"] = sets->value("profileName");
		QListWidgetItem* item = addProfile(idx);
		item->setText(sets->value("profileName").toString());

		for (auto & item : prefs[PREF_SECTIONS::Connections])
		{
			QString pref = item->property("field").toString();
			tmpps[idx][pref] = sets->value(pref);

			if (i == selected)
			{
				if (sets->value(pref).isNull())
				{
					continue;
				}
				if (QLineEdit* field = qobject_cast<QLineEdit*>(item))
				{
					QString val = sets->value(pref).toString();

					if (pref == "password")
					{
						QByteArray ba (val.toUtf8());
						val = QByteArray::fromBase64(ba, QByteArray::AbortOnBase64DecodingErrors);
					}

					field->setText(val);
				}
				else if (QCheckBox* field = qobject_cast<QCheckBox*>(item))
				{
					field->setChecked(sets->value(pref).toBool());
				}
			}
		}
	}
	sets->endArray();

	rplist->setCurrentRow(selected);

	this->state.retr = false;

	for (int i = 1; i < 3; i++)
	{
		QString prefix;
		switch (i)
		{
			case PREF_SECTIONS::Preferences: prefix = "preference"; break;
			case PREF_SECTIONS::Engine: prefix = "engine"; break;
		}
		sets->beginGroup(prefix);

		for (auto & item : prefs[i])
		{
			QString pref = item->property("field").toString();

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
}

void settings::retrieve(QListWidgetItem* item)
{
	int row = item != nullptr ? rplist->row(item) : -1;

	debug("retrieve", "row", row);

	if (item == nullptr)
		return;

	int idx = item->data(Qt::UserRole).toInt();

	for (auto & item : prefs[PREF_SECTIONS::Connections])
	{
		QString pref = item->property("field").toString();

		if (QLineEdit* field = qobject_cast<QLineEdit*>(item))
		{
			QString val = tmpps[idx][pref].toString();

			if (pref == "password")
			{
				QByteArray ba (val.toUtf8());
				val = QByteArray::fromBase64(ba, QByteArray::AbortOnBase64DecodingErrors);
			}

			field->setText(val);
		}
		else if (QCheckBox* field = qobject_cast<QCheckBox*>(item))
		{
			field->setChecked(tmpps[idx][pref].toBool());
		}
	}
}

void settings::retrieve(QTableWidget* adtbl)
{
	debug("retrieve", "index", 3);

	QStringList keys = sets->allKeys().filter(QRegularExpression("^(application|engine|ftpcom|preference|profile|settings)/"));

	QStringList::const_iterator iq;
	adtbl->setRowCount(int (keys.count()));
	int i = 0;
	for (iq = keys.constBegin(); iq != keys.constEnd(); ++iq)
	{
		QString pref = (*iq).toLocal8Bit().replace("/", "."); //Qt5
		QString value = sets->value(*iq).toString();

		if (QApplication::layoutDirection() == Qt::RightToLeft)
		{
			value.prepend(QChar(0x200e)); // LRM
		}

		QTableWidgetItem* field = new QTableWidgetItem;
		field->setText(pref);
		field->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);

		adtbl->setItem(i, 0, field);
		adtbl->setItem(i, 1, new QTableWidgetItem(value));

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
	QTimer::singleShot(100, [=]() {
		dial->close();
	});
}

void settings::destroy()
{
	debug("destroy");

	delete this->dial;
	delete this->theme;
	delete this;
}

void settings::infoMessage(QString title)
{
	title = title.toHtmlEscaped();

	QMessageBox msg = QMessageBox(nullptr);

	msg.setTextFormat(Qt::PlainText);
	msg.setText(title);

	msg.exec();
}

void settings::infoMessage(QString title, QString message)
{
	title = title.toHtmlEscaped();
	message = message.replace("<", "&lt;").replace(">", "&gt;");

	QMessageBox msg = QMessageBox(nullptr);

	msg.setTextFormat(Qt::PlainText);
	msg.setText(title);
	msg.setInformativeText(message);

	msg.exec();
}

void settings::errorMessage(QString title, QString message)
{
	title = title.toHtmlEscaped();
	message = message.replace("<", "&lt;").replace(">", "&gt;");

#ifndef Q_OS_MAC
	QString text = message;
#else
	QString text = QString("%1\n\n%2").arg(title).arg(message);
#endif

	QMessageBox::critical(nullptr, title, text);
}

QMenu* settings::contextMenu()
{
	return new QMenu;
}

QMenu* settings::contextMenu(QMenu* menu)
{
	return new QMenu(menu);
}

QAction* settings::contextMenuAction(QMenu* menu, QString text, std::function<void()> trigger)
{
	QAction* action = new QAction(menu);
	action->setText(text);
	action->connect(action, &QAction::triggered, trigger);
	menu->addAction(action);
	return action;
}

QAction* settings::contextMenuAction(QMenu* menu, QString text, std::function<void()> trigger, bool enabled)
{
	QAction* action = new QAction(menu);
	action->setText(text);
	action->setEnabled(enabled);
	action->connect(action, &QAction::triggered, trigger);
	menu->addAction(action);
	return action;
}

QAction* settings::contextMenuAction(QMenu* menu, QString text, std::function<void()> trigger, QKeySequence shortcut)
{
	QAction* action = new QAction(menu);
	action->setText(text);
	action->setShortcut(shortcut);
	action->connect(action, &QAction::triggered, trigger);
	menu->addAction(action);
	return action;
}

QAction* settings::contextMenuAction(QMenu* menu, QString text, std::function<void()> trigger, bool enabled, QKeySequence shortcut)
{
	QAction* action = new QAction(menu);
	action->setText(text);
	action->setShortcut(shortcut);
	action->setEnabled(enabled);
	action->connect(action, &QAction::triggered, trigger);
	menu->addAction(action);
	return action;
}

QAction* settings::contextMenuSeparator(QMenu* menu)
{
	QAction* action = new QAction(menu);
	action->setSeparator(true);
	menu->addAction(action);
	return action;
}

void settings::textDoubleEscaped(QLineEdit* input, const QString& text)
{
	if (text.contains(QRegularExpression("[\n|\r|\t| ]")))
		input->setText(QString(text).replace("\n", "\\n").replace("\r", "\\r").replace("\t", "\\t").replace(" ", "\\s"));
}

void settings::textRemoveEscaped(QLineEdit* input, const QString& text)
{
	if (text.contains(QRegularExpression("[\n|\r|\t| ]")))
		input->setText(QString(text).remove(QRegularExpression("[\n|\r|\t| ]")));
}

}
