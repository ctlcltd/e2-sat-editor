/*!
 * e2-sat-editor/src/gui/editMarker.cpp
 *
 * @link https://github.com/ctlcltd/e2-sat-editor
 * @copyright e2 SAT Editor Team
 * @author Leonardo Laureti
 * @version 0.5
 * @license MIT License
 * @license GNU GPLv3 License
 */

#include <QFormLayout>
#include <QLineEdit>

#include "platforms/platform.h"

#include "editMarker.h"

using namespace e2se;

namespace e2se_gui
{

editMarker::editMarker(dataHandler* data)
{
	this->log = new logger("gui", "editMarker");

	this->data = data;
}

void editMarker::display(QWidget* cwid)
{
	layout(cwid);

	if (this->state.edit)
		retrieve();

	dial->exec();
}

void editMarker::layout(QWidget* cwid)
{
	this->dialAbstract::layout(cwid);

	QString dtitle = this->state.edit ? tr("Edit Marker") : tr("Add Marker");
	dial->setWindowTitle(dtitle);

	QFormLayout* dtf0 = new QFormLayout;
	dtf0->setRowWrapPolicy(QFormLayout::WrapAllRows);

	QLineEdit* dtf0mt = new QLineEdit;
	dtf0mt->setProperty("field", "value");
	fields.emplace_back(dtf0mt);
	dtf0mt->setMinimumWidth(240);
	dtf0mt->setMaxLength(255);
	platform::osLineEdit(dtf0mt);
	dtf0->addRow(tr("Marker Text"), dtf0mt);
	dtf0->addItem(new QSpacerItem(0, 0));

	dtform->addLayout(dtf0, 0, 0);
}

void editMarker::store()
{
	debug("store");

	auto* dbih = this->data->dbih;

	if (! dbih->userbouquets.count(bname))
		return error("store", "bname", bname);

	e2db::userbouquet& ub = dbih->userbouquets[bname];
	e2db::channel_reference chref;

	if (this->state.edit)
	{
		if (! ub.channels.count(chid))
			return error("store", "chid", chid);

		chref = ub.channels[chid];

		if (! chref.marker)
			return error("store", "chid", chid);
	}
	else
	{
		chref.marker = true;
		chref.atype = 0;
		chref.anum = 0;
		chref.index = 0;
	}

	for (auto & item : fields)
	{
		string key = item->property("field").toString().toStdString();
		string val;

		if (QLineEdit* field = qobject_cast<QLineEdit*>(item))
			val = field->text().toStdString();

		if (key == "value")
			chref.value = val;
	}

	if (this->state.edit)
		this->chid = dbih->editChannelReference(chid, chref, bname);
	else
		this->chid = dbih->addChannelReference(chref, bname);
}

void editMarker::retrieve()
{
	debug("retrieve");

	auto* dbih = this->data->dbih;

	if (! dbih->userbouquets.count(bname))
		return error("retrieve", "bname", bname);

	e2db::userbouquet ub = dbih->userbouquets[bname];
	e2db::channel_reference chref;

	if (this->state.edit)
	{
		if (! ub.channels.count(chid))
			return error("store", "chid", chid);

		chref = ub.channels[chid];

		if (! chref.marker)
			return error("store", "chid", chid);
	}

	for (auto & item : fields)
	{
		string key = item->property("field").toString().toStdString();
		string val;

		if (key == "value")
			val = chref.value;

		if (QLineEdit* field = qobject_cast<QLineEdit*>(item))
		{
			field->setText(QString::fromStdString(val));
		}
	}
}

void editMarker::setEditId(string chid, string bname)
{
	debug("setEditId");

	this->state.edit = true;
	this->chid = chid;
	this->bname = bname;
}

string editMarker::getEditId()
{
	debug("getEditId");

	return this->chid;
}

void editMarker::setAddId(string bname)
{
	debug("setAddId");

	this->bname = bname;
}

string editMarker::getAddId()
{
	debug("getAddId");

	return this->chid;
}

}
