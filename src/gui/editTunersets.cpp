/*!
 * e2-sat-editor/src/gui/editTunersets.cpp
 *
 * @link https://github.com/ctlcltd/e2-sat-editor
 * @copyright e2 SAT Editor Team
 * @author Leonardo Laureti
 * @version 0.7
 * @license MIT License
 * @license GNU GPLv3 License
 */

#include <QFormLayout>
#include <QGroupBox>
#include <QLineEdit>
#include <QTextEdit>

#include "platforms/platform.h"

#include "editTunersets.h"

using std::to_string;

using namespace e2se;

namespace e2se_gui
{

editTunersets::editTunersets(dataHandler* data, int yx)
{
	this->log = new logger("gui", "editTunersets");

	this->data = data;
	this->state.yx = yx;
}

editTunersets::~editTunersets()
{
	debug("~editTunersets");

	delete this->log;
}

void editTunersets::display(QWidget* cwid)
{
	layout(cwid);

	if (this->state.edit)
		retrieve();

	dial->exec();
}

void editTunersets::layout(QWidget* cwid)
{
	this->dialAbstract::layout(cwid);

	QString dtitle = tr("Edit Settings", "dialog");
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
	dtf0cs->setMaxLength(32);
	platform::osLineEdit(dtf0cs);
	dtf0->addRow(tr("Charset"), dtf0cs);
	dtf0->addItem(new QSpacerItem(0, 0));

	dtl0->setLayout(dtf0);
	dtform->addWidget(dtl0, 0, 0);
}

void editTunersets::store()
{
	debug("store");

	auto* dbih = this->data->dbih;

	e2db::tunersets tvs;

	if (this->state.edit)
	{
		if (! dbih->tuners.count(tvid))
			return error("store", tr("Error", "error").toStdString(), tr("Tuner settings \"%1\" not exists.", "error").arg(tvid).toStdString());

		tvs = dbih->tuners[tvid];
	}
	else
	{
		tvs.ytype = this->state.yx;
	}

	string commhead;

	for (auto & item : fields)
	{
		string key = item->property("field").toString().toStdString();
		string val;

		if (QLineEdit* field = qobject_cast<QLineEdit*>(item))
			val = field->text().toStdString();
		//TODO FIX line return
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

	if (dbih->comments.count(iname))
	{
		vector<e2db::comment>::iterator pos;
		for (auto it = dbih->comments[iname].begin(); it != dbih->comments[iname].end(); it++)
		{
			pos = it;
			break;
		}
		if (pos != dbih->comments[iname].end())
		{
			if (commhead.empty())
			{
				dbih->comments[iname].erase(pos);
			}
			else
			{
				pos->type = commhead.find('\n') != string::npos;
				pos->text = commhead;
			}
		}
	}
	else if (! commhead.empty())
	{
		e2db::comment s;
		s.type = commhead.find('\n') != string::npos;
		s.ln = 1;
		s.text = commhead;
		dbih->comments[iname].emplace_back(s);
	}
}

void editTunersets::retrieve()
{
	debug("retrieve");

	auto* dbih = this->data->dbih;

	if (! dbih->tuners.count(tvid))
		return error("retrieve", tr("Error", "error").toStdString(), tr("Tuner settings \"%1\" not exists.", "error").arg(tvid).toStdString());

	e2db::tunersets tvs = dbih->tuners[tvid];

	string iname = "tns:";
	char yname = dbih->value_transponder_type(tvs.ytype);
	iname += yname;

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

void editTunersets::setEditId(int tvid)
{
	debug("setEditId");

	this->state.edit = true;
	this->tvid = tvid;
}

int editTunersets::getEditId()
{
	debug("getEditId");

	return this->tvid;
}

int editTunersets::getAddId()
{
	debug("getAddId");

	return this->tvid;
}

}
