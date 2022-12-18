/*!
 * e2-sat-editor/src/gui/editTunersets.cpp
 *
 * @link https://github.com/ctlcltd/e2-sat-editor
 * @copyright e2 SAT Editor Team
 * @author Leonardo Laureti
 * @version 0.1
 * @license MIT License
 * @license GNU GPLv3 License
 */

#include <QFormLayout>
#include <QGroupBox>
#include <QLineEdit>
#include <QTextEdit>

#include "editTunersets.h"

using std::to_string;
using namespace e2se;

namespace e2se_gui
{

editTunersets::editTunersets(e2db* dbih, int ty, e2se::logger::session* log)
{
	this->log = new logger(log, "editTunersets");
	debug("editTunersets()");

	this->state.ty = ty;
	this->dbih = dbih;
}

void editTunersets::display(QWidget* cwid)
{
	layout();

	if (this->state.edit)
		retrieve();

	this->dialAbstract::display(cwid);
}

void editTunersets::layout()
{
	this->dialAbstract::layout();

	QString dtitle = tr("Edit Settings");
	dial->setWindowTitle(dtitle);

	QGroupBox* dtl0 = new QGroupBox;
	QFormLayout* dtf0 = new QFormLayout;
	dtf0->setRowWrapPolicy(QFormLayout::WrapAllRows);

	QTextEdit* dtf0ch = new QTextEdit;
	dtf0ch->setProperty("field", "commhead");
	dtf0ch->setAcceptRichText(false);
	fields.emplace_back(dtf0ch);
	dtf0ch->setMinimumWidth(240);
	dtf0ch->setMaximumHeight(100);
	dtf0->addRow(tr("Comment"), dtf0ch);
	dtf0->addItem(new QSpacerItem(0, 0));

	dtf0->addItem(new QSpacerItem(0, 0));

	QLineEdit* dtf0cs = new QLineEdit;
	dtf0cs->setProperty("field", "charset");
	fields.emplace_back(dtf0cs);
	dtf0cs->setMinimumWidth(100);
	dtf0->addRow(tr("Charset"), dtf0cs);
	dtf0->addItem(new QSpacerItem(0, 0));

	dtl0->setLayout(dtf0);
	dtform->addWidget(dtl0, 0, 0);
}

void editTunersets::store()
{
	debug("store()");

	e2db::tunersets tvs;

	if (this->state.edit)
	{
		if (! dbih->tuners.count(tvid))
			return error("store()", "tvid", tvid);

		tvs = dbih->tuners[tvid];
	}

	//TODO
	string commhead;

	for (auto & item : fields)
	{
		string key = item->property("field").toString().toStdString();
		string val;

		if (QLineEdit* field = qobject_cast<QLineEdit*>(item))
			val = field->text().toStdString();
		else if (QTextEdit* field = qobject_cast<QTextEdit*>(item))
			val = field->toPlainText().toStdString();

		if (key == "charset")
			tvs.charset = val;
		else if (key == "commhead")
			commhead = val;
	}

	if (this->state.edit)
		this->tvid = dbih->editTunersets(tvid, tvs);
	else
		this->tvid = dbih->addTunersets(tvs);
}

void editTunersets::retrieve()
{
	debug("retrieve()");

	if (! dbih->tuners.count(tvid))
		return error("retrieve()", "tvid", tvid);

	e2db::tunersets tvs = dbih->tuners[tvid];

	string iname = "tns:";
	switch (tvs.ytype)
	{
		case e2db::YTYPE::sat:
			iname += 's';
		break;
		case e2db::YTYPE::terrestrial:
			iname += 't';
		break;
		case e2db::YTYPE::cable:
			iname += 'c';
		break;
		case e2db::YTYPE::atsc:
			iname += 'a';
		break;
	}
	this->iname = iname;

	string commhead;
	if (dbih->comments.count(iname))
	{
		for (auto & x : dbih->comments[iname])
		{
			commhead = x.text;
			break;
		}
		//TODO improve
		if (! commhead.empty())
		{
			if (commhead[0] == '\n')
				commhead = commhead.substr(1);
			if (commhead.size() >= 1 && commhead[commhead.size() - 1] == '\n')
				commhead = commhead.substr(0, commhead.size() - 1);
		}
	}

	for (auto & item : fields)
	{
		string key = item->property("field").toString().toStdString();
		string val;

		if (key == "charset")
			val = tvs.charset;
		else if (key == "commhead")
			val = commhead;

		if (QLineEdit* field = qobject_cast<QLineEdit*>(item))
		{
			field->setText(QString::fromStdString(val));
		}
		else if (QTextEdit* field = qobject_cast<QTextEdit*>(item))
		{
			field->setPlainText(QString::fromStdString(val));
		}
	}
}

void editTunersets::setEditID(int tvid)
{
	debug("setEditID()");

	this->state.edit = true;
	this->tvid = tvid;
}

int editTunersets::getEditID()
{
	debug("getEditID()");

	return this->tvid;
}

}
