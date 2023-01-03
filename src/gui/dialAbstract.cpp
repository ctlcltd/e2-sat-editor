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

#include "dialAbstract.h"
#include "theme.h"

namespace e2se_gui
{

void dialAbstract::layout(QWidget* cwid)
{
	debug("layout()");

	this->dial = new QDialog(cwid);
	dial->setWindowTitle("Edit");
	//TODO FIX SEGFAULT
	// dial->connect(dial, &QDialog::finished, [=]() { delete dial; });

	QGridLayout* dfrm = new QGridLayout(dial);
	QVBoxLayout* dvbox = new QVBoxLayout;

	this->dtbar = new QToolBar;
	dtbar->setIconSize(QSize(16, 16));
	dtbar->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
	dtbar->setStyleSheet("QToolBar { padding: 0 8px } QToolButton { font: 16px }");

	this->widget = new QWidget;
	this->dtform = new QGridLayout;

	dtform->setVerticalSpacing(32);
	widget->setContentsMargins(this->frameMargins);
	widget->setLayout(dtform);

	dfrm->setColumnStretch(0, 1);
	dfrm->setRowStretch(0, 1);
	dfrm->setContentsMargins(0, 0, 0, 0);

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
	dtspacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	dtbar->addWidget(dtspacer);
	//TODO default focused
	this->action.save = dtbar->addAction(theme::icon("edit"), tr("Save"), [=]() { this->save(); });
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
