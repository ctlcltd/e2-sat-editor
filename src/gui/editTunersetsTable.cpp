/*!
 * e2-sat-editor/src/gui/editTunersetsTable.cpp
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
#include <QCheckBox>
#include <QRegularExpression>

#include "platforms/platform.h"

#include "editTunersetsTable.h"

using std::to_string;

using namespace e2se;

namespace e2se_gui
{

editTunersetsTable::editTunersetsTable(dataHandler* data, int yx)
{
	this->log = new logger("gui", "editTunersetsTable");

	this->data = data;
	this->state.yx = yx;
}

void editTunersetsTable::display(QWidget* cwid)
{
	layout(cwid);

	if (this->state.edit)
		retrieve();

	dial->exec();
}

void editTunersetsTable::layout(QWidget* cwid)
{
	this->dialAbstract::layout(cwid);

	QString dtitle = this->state.edit ? tr("Edit Position") : tr("Add Position");
	dial->setWindowTitle(dtitle);

	switch (this->state.yx)
	{
		case e2db::YTYPE::satellite:
			tableSatLayout();
		break;
		case e2db::YTYPE::terrestrial:
			tableTerrestrialLayout();
		break;
		case e2db::YTYPE::cable:
			tableCableLayout();
		break;
		case e2db::YTYPE::atsc:
			tableAtscLayout();
		break;
	}
}

void editTunersetsTable::tableSatLayout()
{
	debug("tableSatLayout");

	QGroupBox* dtl0 = new QGroupBox;
	QFormLayout* dtf0 = new QFormLayout;
	dtf0->setRowWrapPolicy(QFormLayout::WrapAllRows);

	QLineEdit* dtf0sn = new QLineEdit;
	dtf0sn->setProperty("field", "name");
	fields.emplace_back(dtf0sn);
	dtf0sn->setMinimumWidth(240);
	dtf0sn->setMaxLength(255);
	platform::osLineEdit(dtf0sn);
	dtf0->addRow(tr("Name"), dtf0sn);
	dtf0->addItem(new QSpacerItem(0, 0));

	QLineEdit* dtf0sp = new QLineEdit;
	dtf0sp->setProperty("field", "pos");
	fields.emplace_back(dtf0sp);
	dtf0sp->setMinimumWidth(100);
	dtf0sp->setInputMask("000.0>A");
	dtf0sp->setValidator(new QRegularExpressionValidator(QRegularExpression("[\\d]{,3}.\\d\\w")));
	dtf0sp->setText("0.00W");
	platform::osLineEdit(dtf0sp);
	dtf0->addRow(tr("Position"), dtf0sp);
	dtf0->addItem(new QSpacerItem(0, 0));

	dtf0->addItem(new QSpacerItem(0, 0));

	QLineEdit* dtf0ss = new QLineEdit;
	dtf0ss->setProperty("field", "flgs");
	fields.emplace_back(dtf0ss);
	dtf0ss->setMaximumWidth(100);
	dtf0ss->setValidator(new QIntValidator);
	dtf0ss->setMaxLength(2);
	platform::osLineEdit(dtf0ss);
	dtf0->addRow(tr("Flags"), dtf0ss);
	dtf0->addItem(new QSpacerItem(0, 0));

	dtl0->setLayout(dtf0);
	dtform->addWidget(dtl0, 0, 0);
}

void editTunersetsTable::tableTerrestrialLayout()
{
	debug("tableTerrestrialLayout");

	QGroupBox* dtl0 = new QGroupBox;
	QFormLayout* dtf0 = new QFormLayout;
	dtf0->setRowWrapPolicy(QFormLayout::WrapAllRows);

	QLineEdit* dtf0tn = new QLineEdit;
	dtf0tn->setProperty("field", "name");
	fields.emplace_back(dtf0tn);
	dtf0tn->setMinimumWidth(240);
	dtf0tn->setMaxLength(255);
	platform::osLineEdit(dtf0tn);
	dtf0->addRow(tr("Name"), dtf0tn);
	dtf0->addItem(new QSpacerItem(0, 0));

	QLineEdit* dtf0tc = new QLineEdit;
	dtf0tc->setProperty("field", "country");
	fields.emplace_back(dtf0tc);
	dtf0tc->setMinimumWidth(60);
	dtf0tc->setInputMask(">AAA");
	dtf0tc->setMaxLength(3);
	dtf0tc->setText("XYZ");
	platform::osLineEdit(dtf0tc);
	dtf0->addRow(tr("Country"), dtf0tc);
	dtf0->addItem(new QSpacerItem(0, 0));

	dtf0->addItem(new QSpacerItem(0, 0));

	QLineEdit* dtf0ts = new QLineEdit;
	dtf0ts->setProperty("field", "flgs");
	fields.emplace_back(dtf0ts);
	dtf0ts->setMaximumWidth(100);
	dtf0ts->setValidator(new QIntValidator);
	dtf0ts->setMaxLength(2);
	platform::osLineEdit(dtf0ts);
	dtf0->addRow(tr("Flags"), dtf0ts);
	dtf0->addItem(new QSpacerItem(0, 0));

	dtl0->setLayout(dtf0);
	dtform->addWidget(dtl0, 0, 0);
}

void editTunersetsTable::tableCableLayout()
{
	debug("tableCableLayout");

	QGroupBox* dtl0 = new QGroupBox;
	QFormLayout* dtf0 = new QFormLayout;
	dtf0->setRowWrapPolicy(QFormLayout::WrapAllRows);

	QLineEdit* dtf0cn = new QLineEdit;
	dtf0cn->setProperty("field", "name");
	fields.emplace_back(dtf0cn);
	dtf0cn->setMinimumWidth(240);
	dtf0cn->setMaxLength(255);
	platform::osLineEdit(dtf0cn);
	dtf0->addRow(tr("Name"), dtf0cn);
	dtf0->addItem(new QSpacerItem(0, 0));

	QLineEdit* dtf0cc = new QLineEdit;
	dtf0cc->setProperty("field", "country");
	fields.emplace_back(dtf0cc);
	dtf0cc->setMinimumWidth(60);
	dtf0cc->setInputMask(">AAA");
	dtf0cc->setMaxLength(3);
	platform::osLineEdit(dtf0cc);
	dtf0->addRow(tr("Country"), dtf0cc);
	dtf0->addItem(new QSpacerItem(0, 0));

	dtf0->addItem(new QSpacerItem(0, 0));

	QLineEdit* dtf0cs = new QLineEdit;
	dtf0cs->setProperty("field", "flgs");
	fields.emplace_back(dtf0cs);
	dtf0cs->setMaximumWidth(100);
	dtf0cs->setValidator(new QIntValidator);
	dtf0cs->setMaxLength(2);
	platform::osLineEdit(dtf0cs);
	dtf0->addRow(tr("Flags"), dtf0cs);
	dtf0->addItem(new QSpacerItem(0, 0));

	QCheckBox* dtf0cf = new QCheckBox;
	dtf0cf->setProperty("field", "feed");
	fields.emplace_back(dtf0cf);
	dtf0->addRow(tr("Feed"), dtf0cf);
	dtf0->addItem(new QSpacerItem(0, 0));

	dtl0->setLayout(dtf0);
	dtform->addWidget(dtl0, 0, 0);
}

void editTunersetsTable::tableAtscLayout()
{
	debug("tableAtscLayout");

	QGroupBox* dtl0 = new QGroupBox;
	QFormLayout* dtf0 = new QFormLayout;
	dtf0->setRowWrapPolicy(QFormLayout::WrapAllRows);

	QLineEdit* dtf0an = new QLineEdit;
	dtf0an->setProperty("field", "name");
	fields.emplace_back(dtf0an);
	dtf0an->setMinimumWidth(240);
	dtf0an->setMaxLength(255);
	platform::osLineEdit(dtf0an);
	dtf0->addRow(tr("Name"), dtf0an);
	dtf0->addItem(new QSpacerItem(0, 0));

	QLineEdit* dtf0as = new QLineEdit;
	dtf0as->setProperty("field", "flgs");
	fields.emplace_back(dtf0as);
	dtf0as->setMaximumWidth(100);
	dtf0as->setValidator(new QIntValidator);
	dtf0as->setMaxLength(2);
	platform::osLineEdit(dtf0as);
	dtf0->addRow(tr("Flags"), dtf0as);
	dtf0->addItem(new QSpacerItem(0, 0));

	dtl0->setLayout(dtf0);
	dtform->addWidget(dtl0, 0, 0);
}

void editTunersetsTable::store()
{
	debug("store");

	auto* dbih = this->data->dbih;

	e2db::tunersets tvs;

	if (dbih->tuners.count(tvid))
		tvs = dbih->tuners[tvid];
	else
		return error("store", "Error", "Tuner settings \"" + to_string(tvid) + "\" not exists.");

	e2db::tunersets_table tns;

	if (this->state.edit)
	{
		if (! tvs.tables.count(tnid))
			return error("store", "Error", "Tuner settings table \"" + tnid + "\" not exists.");

		tns = tvs.tables[tnid];
	}
	else
	{
		tns.ytype = this->state.yx;
	}

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

		if (this->state.yx == e2db::YTYPE::satellite)
		{
			if (key == "name")
				tns.name = val;
			else if (key == "flgs")
				tns.flgs = val.empty() ? -1 : std::stoi(val);
			else if (key == "pos")
				tns.pos = val.empty() ? -1 : dbih->value_transponder_position(val);
		}
		else if (this->state.yx == e2db::YTYPE::terrestrial)
		{
			if (key == "name")
				tns.name = val;
			else if (key == "flgs")
				tns.flgs = val.empty() ? -1 : std::stoi(val);
			else if (key == "country")
				tns.country = val;
		}
		else if (this->state.yx == e2db::YTYPE::cable)
		{
			if (key == "name")
				tns.name = val;
			else if (key == "flgs")
				tns.flgs = val.empty() ? -1 : std::stoi(val);
			else if (key == "country")
				tns.country = val;
			else if (key == "feed")
				tns.feed = val.empty() ? -1 : std::stoi(val);
		}
		else if (this->state.yx == e2db::YTYPE::atsc)
		{
			if (key == "name")
				tns.name = val;
			else if (key == "flgs")
				tns.flgs = val.empty() ? -1 : std::stoi(val);
		}
	}

	if (this->state.edit)
		this->tnid = dbih->editTunersetsTable(tnid, tns, tvs);
	else
		this->tnid = dbih->addTunersetsTable(tns, tvs);
}

void editTunersetsTable::retrieve()
{
	debug("retrieve");

	auto* dbih = this->data->dbih;

	e2db::tunersets tvs;

	if (dbih->tuners.count(tvid))
		tvs = dbih->tuners[tvid];
	else
		return error("retrieve", "Error", "Tuner settings \"" + to_string(tvid) + "\" not exists.");

	if (! tvs.tables.count(tnid))
		return error("retrieve", "Error", "Tuner settings table \"" + tnid + "\" not exists.");

	e2db::tunersets_table tns = tvs.tables[tnid];

	for (auto & item : fields)
	{
		string key = item->property("field").toString().toStdString();
		string val;

		if (this->state.yx == e2db::YTYPE::satellite)
		{
			if (key == "name")
				val = tns.name;
			else if (key == "flgs")
				val = tns.flgs != -1 ? to_string(tns.flgs) : "";
			else if (key == "pos")
				val = dbih->value_transponder_position(tns.pos);
		}
		else if (this->state.yx == e2db::YTYPE::terrestrial)
		{
			if (key == "name")
				val = tns.name;
			else if (key == "flgs")
				val = tns.flgs != -1 ? to_string(tns.flgs) : "";
			else if (key == "country")
				val = tns.country;
		}
		else if (this->state.yx == e2db::YTYPE::cable)
		{
			if (key == "name")
				val = tns.name;
			else if (key == "flgs")
				val = to_string(tns.flgs);
			else if (key == "country")
				val = tns.country;
			else if (key == "feed")
				val = tns.feed != -1 ? to_string(tns.feed) : "";
		}
		else if (this->state.yx == e2db::YTYPE::atsc)
		{
			if (key == "name")
				val = tns.name;
			else if (key == "flgs")
				val = tns.flgs != -1 ? to_string(tns.flgs) : "";
		}

		if (QLineEdit* field = qobject_cast<QLineEdit*>(item))
		{
			field->setText(QString::fromStdString(val));
		}
		else if (QComboBox* field = qobject_cast<QComboBox*>(item))
		{
			int index = field->findData(QString::fromStdString(val), Qt::UserRole);
			field->setCurrentIndex(index);
		}
		else if (QCheckBox* field = qobject_cast<QCheckBox*>(item))
		{
			field->setChecked(!! std::atoi(val.data()));
		}
	}
}

void editTunersetsTable::setEditId(string tnid, int tvid)
{
	debug("setEditId");

	this->state.edit = true;
	this->tnid = tnid;
	this->tvid = tvid;
}

string editTunersetsTable::getEditId()
{
	debug("getEditId");

	return this->tnid;
}

void editTunersetsTable::setAddId(int tvid)
{
	debug("setAddId");

	this->tvid = tvid;
}

string editTunersetsTable::getAddId()
{
	debug("getAddId");

	return this->tnid;
}

}
