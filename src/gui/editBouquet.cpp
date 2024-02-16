/*!
 * e2-sat-editor/src/gui/editBouquet.cpp
 *
 * @link https://github.com/ctlcltd/e2-sat-editor
 * @copyright e2 SAT Editor Team
 * @author Leonardo Laureti
 * @version 1.2.0
 * @license MIT License
 * @license GNU GPLv3 License
 */

#include <cstring>

#include <QFormLayout>
#include <QVBoxLayout>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QComboBox>

#include "platforms/platform.h"

#include "editBouquet.h"

using namespace e2se;

namespace e2se_gui
{

editBouquet::editBouquet(dataHandler* data)
{
	this->log = new logger("gui", "editBouquet");

	this->data = data;
}

editBouquet::~editBouquet()
{
	debug("~editBouquet");

	delete this->log;
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

	QString dtitle = this->state.edit ? tr("Edit Bouquet", "dialog") : tr("New Bouquet", "dialog");
	dial->setWindowTitle(dtitle);

	QGroupBox* dtl0 = new QGroupBox(tr("Bouquet", "dialog"));
	QFormLayout* dtf0 = new QFormLayout;
	dtf0->setRowWrapPolicy(QFormLayout::WrapAllRows);
	dtl0->setStyleSheet("#dial_field_description { margin-right: 1px; margin-left: 1px }");

	QVBoxLayout* dtb10 = new QVBoxLayout;
	dtf0->addRow(tr("Name"), dtb10);
	QLabel* dtd10 = new QLabel(QString("<small>%1</small>").arg(tr("full name")));
	dtd10->setObjectName("dial_field_description");
	QLineEdit* dtf0bn = new QLineEdit;
	dtf0bn->setProperty("field", "name");
	fields.emplace_back(dtf0bn);
	dtf0bn->setMinimumWidth(240);
	dtf0bn->setMaxLength(255);
	dtb10->addWidget(dtf0bn);
	dtb10->addWidget(dtd10);
	dtf0->addItem(new QSpacerItem(0, 0));

	if (! this->state.edit)
	{
		QComboBox* dtf0bt = new QComboBox;
		dtf0bt->setProperty("field", "btype");
		fields.emplace_back(dtf0bt);
		dtf0bt->setMaximumWidth(100);
		dtf0bt->setValidator(new QIntValidator);
		platform::osComboBox(dtf0bt);
		dtf0->addRow(tr("Type"), dtf0bt);
		dtf0->addItem(new QSpacerItem(0, 0));

		dtf0bt->addItem("TV", e2db::STYPE::tv);
		dtf0bt->addItem("Radio", e2db::STYPE::radio);

		dtf0bt->setCurrentIndex(e2db::STYPE::tv);

		QVBoxLayout* dtb11 = new QVBoxLayout;
		dtf0->addRow(tr("Nice name"), dtb11);
		QLabel* dtd11 = new QLabel(QString("<small>%1</small>").arg(tr("condensed name (eg. TV, Radio)")));
		dtd11->setObjectName("dial_field_description");
		QLineEdit* dtf0bb = new QLineEdit;
		dtf0bb->setProperty("field", "nname");
		fields.emplace_back(dtf0bb);
		dtf0bb->setMinimumWidth(240);
		dtf0bb->setMaxLength(255);
		dtb11->addWidget(dtf0bb);
		dtb11->addWidget(dtd11);
		dtf0->addItem(new QSpacerItem(0, 0));

		QVBoxLayout* dtb12 = new QVBoxLayout;
		dtf0->addRow(tr("Filename"), dtb12);
		QLabel* dtd12 = new QLabel(QString("<small>%1</small>").arg(tr("custom filename, leave empty to auto-fill")));
		dtd12->setObjectName("dial_field_description");
		QLineEdit* dtf0bf = new QLineEdit;
		dtf0bf->setProperty("field", "rname");
		fields.emplace_back(dtf0bf);
		dtf0bf->setMinimumWidth(240);
		dtf0bf->setMaxLength(255);
		dtb12->addWidget(dtf0bf);
		dtb12->addWidget(dtd11);
		dtf0->addItem(new QSpacerItem(0, 0));
	}

	dtl0->setLayout(dtf0);
	dtform->addWidget(dtl0, 0, 0);
}

void editBouquet::store()
{
	debug("store");

	auto* dbih = this->data->dbih;

	e2db::bouquet bs;
	if (this->state.edit)
	{
		if (! dbih->bouquets.count(bname))
			return error("store", tr("Error", "error").toStdString(), tr("Bouquet \"%1\" not exists.", "error").arg(bname.data()).toStdString());

		bs = dbih->bouquets[bname];
	}

	for (auto & item : fields)
	{
		string key = item->property("field").toString().toStdString();
		string val;

		if (QLineEdit* field = qobject_cast<QLineEdit*>(item))
			val = field->text().toStdString();
		else if (QComboBox* field = qobject_cast<QComboBox*>(item))
			val = field->currentData().toString().toStdString();

		if (key == "btype")
			bs.btype = std::stoi(val);
		else if (key == "rname")
			bs.rname = val != bs.bname ? val : "";
		else if (key == "name")
			bs.name = val;
		else if (key == "nname")
			bs.nname = val;
	}

	if (this->state.edit)
		this->bname = dbih->editBouquet(bs);
	else
		this->bname = dbih->addBouquet(bs);
}

void editBouquet::retrieve()
{
	debug("retrieve");

	auto* dbih = this->data->dbih;

	if (! dbih->bouquets.count(bname))
		return error("retrieve", tr("Error", "error").toStdString(), tr("Bouquet \"%1\" not exists.", "error").arg(bname.data()).toStdString());

	e2db::bouquet bs = dbih->bouquets[bname];

	for (auto & item : fields)
	{
		string key = item->property("field").toString().toStdString();
		string val;

		if (key == "btype")
			val = bs.btype;
		else if (key == "rname")
			val = bs.rname.empty() ? bs.bname : bs.rname;
		else if (key == "name")
			val = bs.name;
		else if (key == "nname")
			val = bs.nname;

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
