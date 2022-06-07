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
#include <QComboBox>

#include "editBouquet.h"
#include "theme.h"

using namespace e2se;

namespace e2se_gui
{

editBouquet::editBouquet(e2db* dbih, int ti)
{
	this->log = new logger("editBouquet");
	debug("editBouquet()");

	this->state.ti = ti;
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

void editBouquet::layout()
{
	debug("layout()");

	QGroupBox* dtl0 = new QGroupBox(tr("Userbouquet"));
	QFormLayout* dtf0 = new QFormLayout;
	dtf0->setRowWrapPolicy(QFormLayout::WrapAllRows);

	QLineEdit* dtf0bn = new QLineEdit;
	dtf0bn->setProperty("field", "name");
	fields.emplace_back(dtf0bn);
	dtf0bn->setMinimumWidth(240);
	dtf0->addRow(tr("Bouquet name"), dtf0bn);
	dtf0->addItem(new QSpacerItem(0, 0));

	QGroupBox* dtl1 = new QGroupBox(tr("Bouquet"));
	QFormLayout* dtf1 = new QFormLayout;
	dtf1->setRowWrapPolicy(QFormLayout::WrapAllRows);

	QComboBox* dtf1bt = new QComboBox;
	dtf1bt->setProperty("field", "pname");
	fields.emplace_back(dtf1bt);
	dtf1bt->setMaximumWidth(100);
	dtf1->addRow(dtf1bt);
	dtf1->addItem(new QSpacerItem(0, 0));
	for (auto & bsi : dbih->index["bss"])
	{
		e2db::bouquet gboq = dbih->bouquets[bsi.second];
		QString bgroup = QString::fromStdString(bsi.second);
		QString name = QString::fromStdString(gboq.nname.empty() ? gboq.name : gboq.nname);
		dtf1bt->addItem(name, bgroup);
	}

	// bouquet: tv | radio
	if (this->state.ti != -1)
	{
		dtl1->setVisible(true);
	}
	// userbouquet
	else
	{
		dtl1->setHidden(true);
	}

	dtl0->setLayout(dtf0);
	dtform->addWidget(dtl0, 0, 0);
	dtl1->setLayout(dtf1);
	dtform->addWidget(dtl1, 1, 0);
}

void editBouquet::store()
{
	debug("store()");

	e2db::userbouquet ub;
	if (this->state.edit)
		ub = dbih->userbouquets[bname];

	for (auto & item : fields)
	{
		string key = item->property("field").toString().toStdString();
		string val;

		if (QLineEdit* field = qobject_cast<QLineEdit*>(item))
			val = field->text().toStdString();
		else if (QComboBox* field = qobject_cast<QComboBox*>(item))
			val = field->currentData().toString().toStdString();

		if (key == "name")
			ub.name = val;
		else if (key == "pname")
			ub.pname = val;
	}

	if (this->state.edit)
		this->bname = dbih->editUserbouquet(ub);
	else
		this->bname = dbih->addUserbouquet(ub);
}

void editBouquet::retrieve()
{
	debug("retrieve()");
	
	e2db::userbouquet ub = dbih->userbouquets[bname];

	for (auto & item : fields)
	{
		string key = item->property("field").toString().toStdString();
		string val;

		if (key == "name")
			val = ub.name;
		else if (key == "pname")
			val = ub.pname;

		if (QLineEdit* field = qobject_cast<QLineEdit*>(item))
		{
			field->setText(QString::fromStdString(val));
		}
		else if (QComboBox* field = qobject_cast<QComboBox*>(item))
		{
			if (int index = field->findData(QString::fromStdString(val), Qt::UserRole))
				field->setCurrentIndex(index);
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
