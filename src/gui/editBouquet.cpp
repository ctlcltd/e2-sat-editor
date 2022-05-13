/*!
 * e2-sat-editor/src/gui/editBouquet.cpp
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
#include <QCheckBox>

#include "theme.h"
#include "editBouquet.h"

using namespace e2se;

namespace e2se_gui
{

editBouquet::editBouquet(e2db* dbih, int ti)
{
	this->log = new logger("editBouquet");
	debug("editBouquet()");

	// bouquet: tv | radio
	if (ti == -1)
		error("editBouquet()", "Error", "TODO");

	this->dbih = dbih;
}

void editBouquet::display(QWidget* cwid)
{
	debug("display()");

	QString wtitle = this->state.edit ? "Edit Bouquet" : "Add Bouquet";
	this->dial = new QDialog(cwid);
	dial->setWindowTitle(wtitle);
	dial->connect(dial, &QDialog::finished, [=]() { delete dial; });

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

void editBouquet::store()
{
	debug("store()");

	for (auto & item : fields)
	{
		string key = item->property("field").toString().toStdString();
		string val;

		if (QLineEdit* field = qobject_cast<QLineEdit*>(item))
			val = field->text().toStdString();
		else if (QComboBox* field = qobject_cast<QComboBox*>(item))
			val = field->currentData().toString().toStdString();
		else if (QCheckBox* field = qobject_cast<QCheckBox*>(item))
			val = (field->isChecked() ? '1' : '0');

	}

	e2db::userbouquet ub;

	if (this->state.edit)
		dbih->editUserbouquet(ub);
	else
		dbih->addUserbouquet(ub);
}

void editBouquet::retrieve()
{
	debug("retrieve()");

	for (auto & item : fields)
	{
		string key = item->property("field").toString().toStdString();
		string val;

		if (QLineEdit* field = qobject_cast<QLineEdit*>(item))
		{
			field->setText(QString::fromStdString(val));
		}
		else if (QComboBox* field = qobject_cast<QComboBox*>(item))
		{
			if (int index = field->findData(QString::fromStdString(val), Qt::UserRole))
				field->setCurrentIndex(index);
		}
		else if (QCheckBox* field = qobject_cast<QCheckBox*>(item))
		{
			field->setChecked(!! std::atoi(val.data()));
		}
	}
}

void editBouquet::save()
{
	debug("save()");

	store();

	dial->close();
}

void editBouquet::setEditID(string bname)
{
	debug("setEditID()");

	this->bname = bname;
	this->state.edit = true;
}

string editBouquet::getEditID()
{
	debug("getEditID()");

	return this->bname;
}

void editBouquet::destroy()
{
	delete this;
}

}
