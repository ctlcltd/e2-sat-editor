/*!
 * e2-sat-editor/src/gui/editUserbouquet.cpp
 *
 * @link https://github.com/ctlcltd/e2-sat-editor
 * @copyright e2 SAT Editor Team
 * @author Leonardo Laureti
 * @version 1.2.0
 * @license MIT License
 * @license GNU GPLv3 License
 */

#include <QFormLayout>
#include <QVBoxLayout>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QComboBox>

#include "platforms/platform.h"

#include "editUserbouquet.h"

using namespace e2se;

namespace e2se_gui
{

editUserbouquet::editUserbouquet(dataHandler* data, int ti)
{
	this->log = new logger("gui", "editUserbouquet");

	this->data = data;
	this->state.ti = ti;
}

editUserbouquet::~editUserbouquet()
{
	debug("~editUserbouquet");

	delete this->log;
}

void editUserbouquet::display(QWidget* cwid)
{
	layout(cwid);

	if (this->state.edit)
		retrieve();

	dial->exec();
}

void editUserbouquet::layout(QWidget* cwid)
{
	this->dialAbstract::layout(cwid);

	QString dtitle = this->state.edit ? tr("Edit Userbouquet", "dialog") : tr("New Userbouquet", "dialog");
	dial->setWindowTitle(dtitle);

	QGroupBox* dtl0 = new QGroupBox(tr("Userbouquet", "dialog"));
	QFormLayout* dtf0 = new QFormLayout;
	dtf0->setRowWrapPolicy(QFormLayout::WrapAllRows);
	dtl0->setStyleSheet("#dial_field_description { margin-right: 1px; margin-left: 1px }");

	QLineEdit* dtf0bn = new QLineEdit;
	dtf0bn->setProperty("field", "name");
	fields.emplace_back(dtf0bn);
	dtf0bn->setMinimumWidth(240);
	dtf0bn->setMaxLength(255);
	dtf0->addRow(tr("Name"), dtf0bn);
	dtf0->addItem(new QSpacerItem(0, 0));

	QVBoxLayout* dtb10 = new QVBoxLayout;
	dtf0->addRow(tr("Filename"), dtb10);
	QLabel* dtd10 = new QLabel(QString("<small>%1</small>").arg(tr("custom filename, leave empty to auto-fill")));
	dtd10->setObjectName("dial_field_description");
	QLineEdit* dtf0bf = new QLineEdit;
	dtf0bf->setProperty("field", "rname");
	fields.emplace_back(dtf0bf);
	dtf0bf->setMinimumWidth(240);
	dtf0bf->setMaxLength(255);
	dtb10->addWidget(dtf0bf);
	dtb10->addWidget(dtd10);
	dtf0->addItem(new QSpacerItem(0, 0));

	QGroupBox* dtl1 = new QGroupBox(tr("Bouquet", "dialog"));
	QFormLayout* dtf1 = new QFormLayout;
	dtf1->setRowWrapPolicy(QFormLayout::WrapAllRows);

	auto* dbih = this->data->dbih;

	QComboBox* dtf1bp = new QComboBox;
	dtf1bp->setProperty("field", "pname");
	fields.emplace_back(dtf1bp);
	dtf1bp->setMaximumWidth(100);
	platform::osComboBox(dtf1bp);
	dtf1->addRow(tr("Parent Bouquet"), dtf1bp);
	dtf1->addItem(new QSpacerItem(0, 0));
	for (auto & bsi : dbih->index["bss"])
	{
		e2db::bouquet gboq = dbih->bouquets[bsi.second];
		QString bgroup = QString::fromStdString(bsi.second);
		QString name = QString::fromStdString(gboq.nname.empty() ? gboq.name : gboq.nname);
		dtf1bp->addItem(name, bgroup);
	}

	// bouquet: tv | radio
	if (this->state.ti != -1)
	{
		dtl1->setVisible(true);

		int idx = this->state.ti;
		dtf1bp->setCurrentIndex(idx);
	}
	// userbouquet
	else
	{
		dtl1->setHidden(true);
	}

	dtl0->setLayout(dtf0);
	dtl1->setLayout(dtf1);

	dtform->addWidget(dtl0, 0, 0);
	dtform->addItem(new QSpacerItem(0, 18), 1, 0);
	dtform->addWidget(dtl1, 2, 0);
}

void editUserbouquet::store()
{
	debug("store");

	auto* dbih = this->data->dbih;

	e2db::userbouquet ub;
	if (this->state.edit)
	{
		if (! dbih->userbouquets.count(bname))
			return error("store", tr("Error", "error").toStdString(), tr("Userbouquet \"%1\" not exists.", "error").arg(bname.data()).toStdString());

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

		if (key == "rname")
			ub.rname = val != ub.bname ? val : "";
		else if (key == "name")
			ub.name = val;
		else if (key == "pname")
			ub.pname = val;
	}

	if (this->state.edit)
		this->bname = dbih->editUserbouquet(ub);
	else
		this->bname = dbih->addUserbouquet(ub);

	this->changes = true;
}

void editUserbouquet::retrieve()
{
	debug("retrieve");

	auto* dbih = this->data->dbih;

	if (! dbih->userbouquets.count(bname))
		return error("retrieve", tr("Error", "error").toStdString(), tr("Userbouquet \"%1\" not exists.", "error").arg(bname.data()).toStdString());

	e2db::userbouquet ub = dbih->userbouquets[bname];

	for (auto & item : fields)
	{
		string key = item->property("field").toString().toStdString();
		string val;

		if (key == "rname")
			val = ub.rname.empty() ? ub.bname : ub.rname;
		else if (key == "name")
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

void editUserbouquet::setEditId(string bname)
{
	debug("setEditId");

	this->state.edit = true;
	this->bname = bname;
}

string editUserbouquet::getEditId()
{
	debug("getEditId");

	return this->bname;
}

string editUserbouquet::getAddId()
{
	debug("getAddId");

	return this->bname;
}

}
