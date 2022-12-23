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

#include <QFormLayout>
#include <QLineEdit>

#include "editMarker.h"

using namespace e2se;

namespace e2se_gui
{

editMarker::editMarker(dataHandler* data, e2se::logger::session* log)
{
	this->log = new logger(log, "editMarker");
	debug("editMarker()");

	this->data = data;
}

void editMarker::display(QWidget* cwid)
{
	this->dbih = this->data->dbih;

	layout();

	if (this->state.edit)
		retrieve();

	this->dialAbstract::display(cwid);
}

void editMarker::layout()
{
	this->dialAbstract::layout();

	QString dtitle = this->state.edit ? tr("Edit Marker") : tr("Add Marker");
	dial->setWindowTitle(dtitle);

	QFormLayout* dtf0 = new QFormLayout;
	dtf0->setRowWrapPolicy(QFormLayout::WrapAllRows);

	QLineEdit* dtf0mt = new QLineEdit;
	dtf0mt->setProperty("field", "value");
	fields.emplace_back(dtf0mt);
	dtf0mt->setMinimumWidth(240);
	dtf0->addRow(tr("Marker Text"), dtf0mt);

	dtform->addLayout(dtf0, 0, 0);
}

void editMarker::store()
{
	debug("store()");

	if (! dbih->userbouquets.count(bname))
		return error("store()", "bname", bname);

	e2db::userbouquet& ub = dbih->userbouquets[bname];
	e2db::channel_reference chref;
	
	if (this->state.edit)
	{
		if (! ub.channels.count(chid))
			return error("store()", "chid", chid);

		chref = ub.channels[chid];

		if (! chref.marker)
			return error("store()", "chid", chid);
	}
	else
	{
		chref.marker = true;
		chref.type = 0;
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
	debug("retrieve()");

	if (! dbih->userbouquets.count(bname))
		return error("retrieve()", "bname", bname);

	e2db::userbouquet ub = dbih->userbouquets[bname];
	e2db::channel_reference chref;

	if (this->state.edit)
	{
		if (! ub.channels.count(chid))
			return error("store()", "chid", chid);

		chref = ub.channels[chid];

		if (! chref.marker)
			return error("store()", "chid", chid);
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
	debug("setEditId()");

	this->state.edit = true;
	this->chid = chid;
	this->bname = bname;
}

string editMarker::getEditId()
{
	debug("getEditId()");

	return this->chid;
}

void editMarker::setAddId(string bname)
{
	debug("setAddId()");

	this->bname = bname;
}

string editMarker::getAddId()
{
	debug("getAddId()");

	return this->chid;
}

}
