/*!
 * e2-sat-editor/src/gui/editBouquet.cpp
 *
 * @link https://github.com/ctlcltd/e2-sat-editor
 * @copyright e2 SAT Editor Team
 * @author Leonardo Laureti
 * @version 0.5
 * @license MIT License
 * @license GNU GPLv3 License
 */

#include <QFormLayout>
#include <QGroupBox>
#include <QLineEdit>
#include <QComboBox>

#include "platforms/platform.h"

#include "editBouquet.h"

using namespace e2se;

namespace e2se_gui
{

//TODO improve custom bname (eg. userbouquet.favourites.tv)
editBouquet::editBouquet(dataHandler* data, int ti)
{
	this->log = new logger("gui", "editBouquet");

	this->data = data;
	this->state.ti = ti;
}

void editBouquet::display(QWidget* cwid)
{
	layout(cwid);

	if (this->state.edit)
		retrieve();

	dial->exec();
}

void editBouquet::layout(QWidget* cwid)
{
	this->dialAbstract::layout(cwid);

	QString dtitle = this->state.edit ? tr("Edit Bouquet") : tr("Add Bouquet");
	dial->setWindowTitle(dtitle);

	QGroupBox* dtl0 = new QGroupBox(tr("Userbouquet"));
	QFormLayout* dtf0 = new QFormLayout;
	dtf0->setRowWrapPolicy(QFormLayout::WrapAllRows);

	QLineEdit* dtf0bn = new QLineEdit;
	dtf0bn->setProperty("field", "name");
	fields.emplace_back(dtf0bn);
	dtf0bn->setMinimumWidth(240);
	dtf0bn->setMaxLength(255);
	dtf0->addRow(tr("Bouquet name"), dtf0bn);
	dtf0->addItem(new QSpacerItem(0, 0));

	QGroupBox* dtl1 = new QGroupBox(tr("Bouquet"));
	QFormLayout* dtf1 = new QFormLayout;
	dtf1->setRowWrapPolicy(QFormLayout::WrapAllRows);

	auto* dbih = this->data->dbih;

	QComboBox* dtf1bt = new QComboBox;
	dtf1bt->setProperty("field", "pname");
	fields.emplace_back(dtf1bt);
	dtf1bt->setMaximumWidth(100);
	platform::osComboBox(dtf1bt);
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
	debug("store");

	auto* dbih = this->data->dbih;

	e2db::userbouquet ub;
	if (this->state.edit)
	{
		if (! dbih->userbouquets.count(bname))
			return error("store", "Error", "Userbouquet \"" + bname + "\" not exists.");

		ub = dbih->userbouquets[bname];
	}

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
	debug("retrieve");

	auto* dbih = this->data->dbih;

	if (! dbih->userbouquets.count(bname))
		return error("retrieve", "Error", "Userbouquet \"" + bname + "\" not exists.");

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
			int index = field->findData(QString::fromStdString(val), Qt::UserRole);
			field->setCurrentIndex(index);
		}
	}
}

void editBouquet::setEditId(string bname)
{
	debug("setEditId");

	this->state.edit = true;
	this->bname = bname;
}

string editBouquet::getEditId()
{
	debug("getEditId");

	return this->bname;
}

string editBouquet::getAddId()
{
	debug("getAddId");

	return this->bname;
}

}
