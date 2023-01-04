/*!
 * e2-sat-editor/src/gui/dialAbstract.cpp
 *
 * @link https://github.com/ctlcltd/e2-sat-editor
 * @copyright e2 SAT Editor Team
 * @author Leonardo Laureti
 * @version 0.1
 * @license MIT License
 * @license GNU GPLv3 License
 */

#include <QVBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QToolButton>

#include "dialAbstract.h"
#include "theme.h"

namespace e2se_gui
{

void dialAbstract::layout(QWidget* cwid)
{
	debug("layout()");

	this->dial = new QDialog(cwid);
	dial->setWindowTitle("Edit");

	QGridLayout* dfrm = new QGridLayout(dial);
	QVBoxLayout* dvbox = new QVBoxLayout;

	this->dtbar = new QToolBar;
	dtbar->setIconSize(QSize(16, 16));
	dtbar->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
	dtbar->setStyleSheet("QToolBar { padding: 0 8px } QToolButton { font: 16px }");

	this->widget = new QWidget;
	this->dtform = new QGridLayout;

	dvbox->setSpacing(0);
	widget->setContentsMargins(this->frameMargins);
	widget->setLayout(dtform);

	dfrm->setColumnStretch(0, 1);
	dfrm->setRowStretch(0, 1);
	dfrm->setContentsMargins(0, 0, 0, 0);

	if (this->collapsible)
	{
		QToolButton* toggler = new QToolButton;
		toggler->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
		toggler->setArrowType(Qt::UpArrow);
		toggler->setText(" " + tr("collapse") + " ");
		toggler->setFixedSize(96, 20);
		toggler->connect(toggler, &QToolButton::pressed, [=]() {
			this->toggle();

			if (dial->property("collapsible_collapsed").toBool())
			{
				toggler->setArrowType(Qt::DownArrow);
				toggler->setText(" " + tr("expand") + " ");
			}
			else
			{
				toggler->setArrowType(Qt::UpArrow);
				toggler->setText(" " + tr("collapse") + " ");
			}
		});
		dvbox->addWidget(toggler, 0, Qt::AlignRight | Qt::AlignTop);
		dial->setProperty("collapsible_togglerHeight", toggler->height());
	}
	dvbox->addWidget(widget);
	dvbox->addWidget(dtbar);

	dfrm->addLayout(dvbox, 0, 0);

	if (this->frameFixed)
		dfrm->setSizeConstraint(QGridLayout::SetFixedSize);

	toolbar();
}

void dialAbstract::toolbar()
{
	debug("toolbar()");

	QWidget* dtspacer = new QWidget;
	dtspacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Ignored);
	QWidget* dtseparator = new QWidget;
	dtseparator->setMaximumWidth(5);

	dtbar->addWidget(dtspacer);

	this->action.cancel = dtbar->addAction(tr("Cancel"), [=]() { this->cancel(); });
	dtbar->addWidget(dtseparator);
	this->action.save = dtbar->addAction(theme::icon("edit"), tr("Save"), [=]() { this->save(); });
}

void dialAbstract::expand()
{
	debug("expand()");

	widget->show();
	dtbar->show();
	dial->setMinimumHeight(dial->property("collapsible_minimumHeight").toInt());
	dial->resize(dial->width(), dial->minimumHeight());
	dial->setProperty("collapsible_collapsed", false);
}

void dialAbstract::collapse()
{
	debug("collapse()");

	dial->setProperty("collapsible_minimumHeight", int (dial->minimumHeight()));
	dtbar->hide();
	widget->hide();
	dial->setMinimumHeight(dial->property("collapsible_togglerHeight").toInt());
	dial->resize(dial->width(), dial->property("collapsible_togglerHeight").toInt());
	dial->setProperty("collapsible_collapsed", true);
}

void dialAbstract::toggle()
{
	// debug("toggle()");

	if (dial->property("collapsible_collapsed").toBool())
		expand();
	else
		collapse();
}

void dialAbstract::cancel()
{
	debug("cancel()");

	dial->close();
}

void dialAbstract::save()
{
	debug("save()");

	store();

	dial->close();
}

void dialAbstract::destroy()
{
	delete this->dial;
	delete this;
}

}
