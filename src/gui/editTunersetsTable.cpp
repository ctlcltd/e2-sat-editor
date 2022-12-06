/*!
 * e2-sat-editor/src/gui/editTunersetsTable.cpp
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

#include "editTunersetsTable.h"
#include "theme.h"

using namespace e2se;

namespace e2se_gui
{

editTunersetsTable::editTunersetsTable(e2db* dbih, e2se::logger::session* log)
{
	this->log = new logger(log, "editTunersetsTable");
	debug("editTunersetsTable()");

	this->dbih = dbih;
}

void editTunersetsTable::display(QWidget* cwid)
{
	debug("display()");

	QString wtitle = this->state.edit ? "Edit" : "Add";
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

void editTunersetsTable::layout()
{
	debug("layout()");
}

void editTunersetsTable::store()
{
	debug("store()");
}

void editTunersetsTable::retrieve()
{
	debug("retrieve()");
}

void editTunersetsTable::save()
{
	debug("save()");

	store();

	dial->close();
}

void editTunersetsTable::setEditID(string todo)
{
	debug("setEditID()");

	this->state.edit = true;
}

string editTunersetsTable::getEditID()
{
	debug("getEditID()");

	return "";
}

void editTunersetsTable::destroy()
{
	delete this->dial;
	delete this;
}

}
