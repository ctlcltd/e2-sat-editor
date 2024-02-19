/*!
 * e2-sat-editor/src/gui/dialAbstract.cpp
 *
 * @link https://github.com/ctlcltd/e2-sat-editor
 * @copyright e2 SAT Editor Team
 * @author Leonardo Laureti
 * @version 1.2.0
 * @license MIT License
 * @license GNU GPLv3 License
 */

#include <QTimer>
#include <QVBoxLayout>
#include <QFormLayout>
#include <QGroupBox>

#include "platforms/platform.h"

#include "toolkit/ThemeChangeEventObserver.h"
#include "dialAbstract.h"
#include "theme.h"

namespace e2se_gui
{

void dialAbstract::layout(QWidget* cwid)
{
	debug("layout");

	this->theme = new e2se_gui::theme;

	this->dial = new QDialog(cwid);
	dial->setWindowTitle(tr("Edit", "dialog"));
	theme->fix(dial);

	ThemeChangeEventObserver* gce = new ThemeChangeEventObserver;
	gce->setEventCallback([=]() { this->themeChanged(); });
	dial->installEventFilter(gce);

	platform::osWindowBlend(dial);

	QGridLayout* dfrm = new QGridLayout(dial);

	this->widget = new QWidget;

	QVBoxLayout* dvbox = new QVBoxLayout;
	this->dtform = new QGridLayout;

	dfrm->setContentsMargins(0, 0, 0, 0);

	dvbox->setSpacing(0);

	widget->setContentsMargins(this->frameMargins);
	widget->setLayout(dtform);

	if (this->collapsible)
	{
		collapsibleLayout();

		dvbox->addWidget(dttoggler, 0, Qt::AlignTrailing | Qt::AlignTop);
	}
	toolbarLayout();

	dvbox->addWidget(widget);
	dvbox->addWidget(dtbar);

	dfrm->setColumnStretch(0, 1);
	dfrm->setRowStretch(0, 1);

	dfrm->addLayout(dvbox, 0, 0);

	if (this->frameFixed)
		dfrm->setSizeConstraint(QGridLayout::SetFixedSize);
}

void dialAbstract::toolbarLayout()
{
	debug("toolbarLayout");

	this->dtbar = toolBar();
	toolBarStyleSheet();

	toolBarSpacer(dtbar);
	this->action.cancel = toolBarAction(dtbar, tr("Cancel", "dialog"), [=]() { this->cancel(); });
	toolBarSeparator(dtbar);
	this->action.save = toolBarAction(dtbar, tr("Save", "dialog"), theme->dynamicIcon("edit"), [=]() { this->save(); });
}

void dialAbstract::collapsibleLayout()
{
	debug("collapsibleLayout");

	this->dttoggler = new QToolButton;
	dttoggler->setObjectName("dial_toggler");
	dttoggler->setFixedHeight(20);
	dttoggler->setIconSize(QSize(8, 8));
	dttoggler->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
	dttoggler->setArrowType(Qt::UpArrow);
	dttoggler->setText(" " + tr("collapse", "ui") + " ");
	dttoggler->connect(dttoggler, &QToolButton::pressed, [=]() {
		this->toggle();

		if (dial->property("collapsible_collapsed").toBool())
		{
			dttoggler->setArrowType(Qt::DownArrow);
			dttoggler->setText(" " + tr("expand", "ui") + " ");
		}
		else
		{
			dttoggler->setArrowType(Qt::UpArrow);
			dttoggler->setText(" " + tr("collapse", "ui") + " ");
		}
	});

	dial->setProperty("collapsible_togglerHeight", dttoggler->height());

#ifdef Q_OS_MAC
	dttoggler->setStyleSheet("border: 0; background: none; font-weight: bold");
#else
	dttoggler->setStyleSheet("width: 96px");
#endif
}

void dialAbstract::themeChanged()
{
	debug("themeChanged");

	theme->changed();
}

void dialAbstract::expand()
{
	debug("expand");

	widget->show();
	dtbar->show();
	dial->setMinimumHeight(dial->property("collapsible_minimumHeight").toInt());
	dial->resize(dial->width(), dial->minimumHeight());
	dial->setProperty("collapsible_collapsed", false);
}

void dialAbstract::collapse()
{
	debug("collapse");

	dial->setProperty("collapsible_minimumHeight", int (dial->minimumHeight()));
	dtbar->hide();
	widget->hide();
	dial->setMinimumHeight(dial->property("collapsible_togglerHeight").toInt());
	dial->resize(dial->width(), dial->property("collapsible_togglerHeight").toInt());
	dial->setProperty("collapsible_collapsed", true);
}

void dialAbstract::toggle()
{
	// debug("toggle");

	if (dial->property("collapsible_collapsed").toBool())
		expand();
	else
		collapse();
}

void dialAbstract::cancel()
{
	debug("cancel");

	// delay too fast
	QTimer::singleShot(100, [=]() {
		dial->close();
	});
}

void dialAbstract::save()
{
	debug("save");

	store();

	// delay too fast
	QTimer::singleShot(150, [=]() {
		dial->close();
	});
}

void dialAbstract::destroy()
{
	debug("destroy");

	delete this->dial;
	delete this->theme;
	delete this;
}

QToolBar* dialAbstract::toolBar()
{
	QToolBar* toolbar = new QToolBar;
	toolbar->setObjectName("dial_toolbar");
	toolbar->setIconSize(QSize(16, 16));
	toolbar->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);

#ifndef Q_OS_MAC
	toolbar->setStyleSheet("QToolBar { border-top: 1px solid; padding: 0 8px } QToolButton { font-size: 16px }");
#else
	toolbar->setStyleSheet("QToolBar { border-top: 1px solid; padding: 0 8px; background: transparent } QToolButton { font-size: 16px }");
#endif

	return toolbar;
}

QAction* dialAbstract::toolBarAction(QToolBar* toolbar, QString text, std::function<void()> trigger)
{
	QAction* action = new QAction(toolbar);
	action->setText(text);
	action->connect(action, &QAction::triggered, trigger);
	toolbar->addAction(action);
	return action;
}

QAction* dialAbstract::toolBarAction(QToolBar* toolbar, QString text, pair<e2se_gui::theme*, QString> icon, std::function<void()> trigger)
{
	QAction* action = new QAction(toolbar);
	action->setText(text);
	action->setIcon(icon.first->dynamicIcon(icon.second, action));
	action->connect(action, &QAction::triggered, trigger);
	toolbar->addAction(action);
	return action;
}

QAction* dialAbstract::toolBarAction(QToolBar* toolbar, QString text, std::function<void()> trigger, QKeySequence shortcut)
{
	QAction* action = new QAction(toolbar);
	action->setText(text);
	action->setShortcut(shortcut);
	action->connect(action, &QAction::triggered, trigger);
	toolbar->addAction(action);
	return action;
}

QAction* dialAbstract::toolBarAction(QToolBar* toolbar, QString text, pair<e2se_gui::theme*, QString> icon, std::function<void()> trigger, QKeySequence shortcut)
{
	QAction* action = new QAction(toolbar);
	action->setText(text);
	action->setIcon(icon.first->dynamicIcon(icon.second, action));
	action->setShortcut(shortcut);
	action->connect(action, &QAction::triggered, trigger);
	toolbar->addAction(action);
	return action;
}

QWidget* dialAbstract::toolBarWidget(QToolBar* toolbar, QString text, QWidget* widget)
{
	widget->setAccessibleName(text);
	toolbar->addWidget(widget);
	return widget;
}

QWidget* dialAbstract::toolBarSeparator(QToolBar* toolbar)
{
	QWidget* separator = new QWidget;
	separator->setMaximumWidth(5);
	toolbar->addWidget(separator);
	return separator;
}

QWidget* dialAbstract::toolBarSpacer(QToolBar* toolbar)
{
	QWidget* spacer = new QWidget;
	spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Ignored);
	toolbar->addWidget(spacer);
	return spacer;
}

void dialAbstract::toolBarStyleSheet()
{
	if (! theme::isDefault())
	{
		theme->dynamicStyleSheet(dtbar, "#dial_toolbar { background: palette(mid) }");
	}
#ifndef Q_OS_MAC
	else
	{
		QColor tbshade;
		QString tbshade_hexArgb;

		tbshade = QColor(Qt::black);
		tbshade.setAlpha(18);
		tbshade_hexArgb = tbshade.name(QColor::HexArgb);

		theme->dynamicStyleSheet(dtbar, "#dial_toolbar { border-color: " + tbshade_hexArgb + " }");
	}
#else
	QColor tbshade;
	QString tbshade_hexArgb;

	tbshade = QColor(Qt::black);
	tbshade.setAlphaF(0.08);
	tbshade_hexArgb = tbshade.name(QColor::HexArgb);

	theme->dynamicStyleSheet(dtbar, "#dial_toolbar { border-color: " + tbshade_hexArgb + " }", theme::light);

	tbshade = QPalette().color(QPalette::Dark).darker();
	tbshade.setAlphaF(0.32);
	tbshade_hexArgb = tbshade.name(QColor::HexArgb);

	theme->dynamicStyleSheet(dtbar, "#dial_toolbar { border-color: " + tbshade_hexArgb + " }", theme::dark);
#endif
}

}
