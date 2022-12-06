/*!
 * e2-sat-editor/src/gui/editMarker.cpp
 *
 * @link https://github.com/ctlcltd/e2-sat-editor
 * @copyright e2 SAT Editor Team
 * @author Leonardo Laureti
 * @version 0.1
 * @license MIT License
 * @license GNU GPLv3 License
 */

#include <QtGlobal>
#include <QList>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QToolBox>
#include <QToolBar>
#include <QLabel>
#include <QLineEdit>
#include <QComboBox>

#include "editMarker.h"
#include "theme.h"

using namespace e2se;

namespace e2se_gui
{

editMarker::editMarker(e2db* dbih, e2se::logger::session* log)
{
	this->log = new logger(log, "editMarker");
	debug("editMarker()");

	this->dbih = dbih;
}

void editMarker::display(QWidget* cwid)
{
	debug("display()");

	QString wtitle = this->state.edit ? "Edit Marker" : "Add Marker";
	this->dial = new QDialog(cwid);
	dial->setWindowTitle(wtitle);
	//TODO FIX SEGFAULT
	// dial->connect(dial, &QDialog::finished, [=]() { delete dial; });

	QGridLayout* dfrm = new QGridLayout(dial);
	QVBoxLayout* dvbox = new QVBoxLayout;

	QToolBar* dttbar = new QToolBar;
	dttbar->setIconSize(QSize(16, 16));
	dttbar->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
	dttbar->setStyleSheet("QToolBar { padding: 0 8px } QToolButton { font: 16px }");
	QWidget* dtspacer = new QWidget;
	dtspacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	dttbar->addWidget(dtspacer);
	dttbar->addAction(theme::icon("edit"), tr("Save"), [=]() { this->save(); });

	this->widget = new QWidget;
	this->dtform = new QGridLayout;

	layout();
	if (this->state.edit)
		retrieve();

	dtform->setVerticalSpacing(32);
	widget->setContentsMargins(12, 12, 12, 12);
	widget->setLayout(dtform);

	dfrm->setColumnStretch(0, 1);
	dfrm->setRowStretch(0, 1);
	dfrm->setContentsMargins(0, 0, 0, 0);

	dvbox->addWidget(widget);
	dvbox->addWidget(dttbar);

	dfrm->addLayout(dvbox, 0, 0);

	dfrm->setSizeConstraint(QGridLayout::SetFixedSize);
	dial->setLayout(dfrm);
	dial->exec();
}

void editMarker::layout()
{
	debug("layout()");

	QGroupBox* dtl0 = new QGroupBox(tr("Marker"));
	QFormLayout* dtf0 = new QFormLayout;
	dtf0->setRowWrapPolicy(QFormLayout::WrapAllRows);

	QLineEdit* dtf0bn = new QLineEdit;
	dtf0bn->setProperty("field", "name");
	fields.emplace_back(dtf0bn);
	dtf0bn->setMinimumWidth(240);
	dtf0->addRow(tr("Marker Text"), dtf0bn);
	dtf0->addItem(new QSpacerItem(0, 0));

	dtl0->setLayout(dtf0);
	dtform->addWidget(dtl0, 0, 0);
}

void editMarker::store()
{
	debug("store()");
}

void editMarker::retrieve()
{
	debug("retrieve()");
}

void editMarker::save()
{
	debug("save()");

	store();

	dial->close();
}

void editMarker::setEditID(string chid)
{
	debug("setEditID()");

	this->chid = chid;
	this->state.edit = true;
}

string editMarker::getEditID()
{
	debug("getEditID()");

	return this->chid;
}

void editMarker::destroy()
{
	delete this->dial;
	delete this;
}

}
