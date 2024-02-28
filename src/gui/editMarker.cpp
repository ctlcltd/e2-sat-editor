/*!
 * e2-sat-editor/src/gui/editMarker.cpp
 *
 * @link https://github.com/ctlcltd/e2-sat-editor
 * @copyright e2 SAT Editor Team
 * @author Leonardo Laureti
 * @version 1.2.0
 * @license MIT License
 * @license GNU GPLv3 License
 */

#include <algorithm>

#include <QtGlobal>
#include <QTimer>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QLineEdit>
#include <QComboBox>
#include <QPushButton>

#include "platforms/platform.h"

#include "editMarker.h"

using std::to_string, std::sort;

using namespace e2se;

namespace e2se_gui
{

editMarker::editMarker(dataHandler* data)
{
	this->log = new logger("gui", "editMarker");

	this->data = data;
}

editMarker::~editMarker()
{
	debug("~editMarker");

	delete this->log;
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

	QString dtitle = this->state.edit ? tr("Edit Marker", "dialog") : tr("Add Marker", "dialog");
	dial->setWindowTitle(dtitle);

	QGroupBox* dtl0 = new QGroupBox(tr("Marker", "dialog"));
	QFormLayout* dtf0 = new QFormLayout;
	dtf0->setRowWrapPolicy(QFormLayout::WrapAllRows);

	QLineEdit* dtf0mt = new QLineEdit;
	dtf0mt->setProperty("field", "value");
	fields.emplace_back(dtf0mt);
	dtf0mt->setMinimumWidth(240);
	dtf0mt->setMaxLength(255);
	platform::osLineEdit(dtf0mt);
	dtf0->addRow(tr("Marker text"), dtf0mt);
	dtf0->addItem(new QSpacerItem(0, 0));

	QWidget* dtw11 = new QWidget;
	dtw11->setMinimumHeight(50);
	QHBoxLayout* dtb11 = new QHBoxLayout(dtw11);
	dtb11->setContentsMargins(0, 0, 0, 0);
	dtf0->addRow(tr("Marker flag"), dtw11);

	QLineEdit* dtf0af = new QLineEdit;
	dtf0af->setProperty("field", "atype");
	dtf0af->setVisible(false);
	fields.emplace_back(dtf0af);
	dtf0af->setMinimumWidth(50);
	dtf0af->setValidator(new QIntValidator);

	QComboBox* dtf0ac = new QComboBox;
	dtf0ac->setMaximumWidth(110);
	dtf0ac->setValidator(new QIntValidator);
	platform::osComboBox(dtf0ac);

	vector<int> atypes;
	for (auto & x : e2db::ATYPE_EXT_TYPE)
	{
		if (x.first == e2db::ATYPE::aaaa || x.first == e2db::ATYPE::group)
			continue;
		atypes.emplace_back(x.first);
	}
	sort(atypes.begin(), atypes.end());
	for (auto & q : atypes)
	{
		string pad = q > 9 ? "  " : "  ";
		dtf0ac->addItem(QString::fromStdString(to_string(q) + pad + e2db::ATYPE_EXT_LABEL.at(q)), q);
	}

	QPushButton* dtf0ab = new QPushButton;
	dtf0ab->setDefault(false);
	dtf0ab->setCheckable(true);
	dtf0ab->setText(tr("custom"));
	dtf0ab->setChecked(false);

	dtf0ab->connect(dtf0ab, &QPushButton::pressed, [=]() {
		// delay too fast
		QTimer::singleShot(100, [=]() {
			if (dtf0ac->isHidden())
			{
				dtf0af->hide();
				dtf0ac->show();
				dtf0ab->setChecked(true);
				dtf0ac->setFocus();
			}
			else
			{
				dtf0ac->hide();
				dtf0af->show();
				dtf0ab->setChecked(false);
				dtf0af->setFocus();
			}
		});
	});

	dtf0af->connect(dtf0af, &QLineEdit::textChanged, [=](QString text) {
		int index = dtf0ac->findData(text, Qt::UserRole);
		dtf0ac->setCurrentIndex(index);
	});

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
	dtf0ac->connect(dtf0ac, &QComboBox::currentIndexChanged, [=](int index) {
#else
	dtf0ac->connect(dtf0ac, QOverload<int>::of(&QComboBox::currentIndexChanged), [=](int index) {
#endif
		if (index == -1 && ! dtf0ab->isChecked())
		{
			dtf0ac->hide();
			dtf0af->show();
			dtf0af->setFocus();
			dtf0ab->setChecked(true);
		}
		else
		{
			dtf0ab->setChecked(false);
			dtf0af->setText(dtf0ac->currentData().toString());
		}
	});

	dtb11->addWidget(dtf0ac);
	dtb11->addWidget(dtf0af);
	dtb11->addWidget(dtf0ab);

	dtl0->setLayout(dtf0);
	dtform->addWidget(dtl0, 0, 0);
}

void editMarker::store()
{
	debug("store");

	auto* dbih = this->data->dbih;

	if (! dbih->userbouquets.count(bname))
		return error("store", tr("Error", "error").toStdString(), tr("Userbouquet \"%1\" not exists.", "error").arg(bname.data()).toStdString());

	e2db::userbouquet& ub = dbih->userbouquets[bname];
	e2db::channel_reference chref;

	if (this->state.edit)
	{
		if (! ub.channels.count(chid))
			return error("store", tr("Error", "error").toStdString(), tr("Channel reference \"%1\" not exists.", "error").arg(chid.data()).toStdString());

		chref = ub.channels[chid];

		if (! chref.marker)
			return error("store", tr("Error", "error").toStdString(), tr("Channel reference mismatch \"%1\".", "error").arg(chid.data()).toStdString());
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
		else if (QComboBox* field = qobject_cast<QComboBox*>(item))
			val = field->currentData().toString().toStdString();

		if (key == "value")
			chref.value = val;
		else if (key == "atype")
			chref.atype = val.empty() ? 0 : std::stoi(val);
	}

	if (this->state.edit)
		this->chid = dbih->editChannelReference(chid, chref, bname);
	else
		this->chid = dbih->addChannelReference(chref, bname);

	this->changes = true;
}

void editMarker::retrieve()
{
	debug("retrieve");

	auto* dbih = this->data->dbih;

	if (! dbih->userbouquets.count(bname))
		return error("retrieve", tr("Error", "error").toStdString(), tr("Userbouquet \"%1\" not exists.", "error").arg(bname.data()).toStdString());

	e2db::userbouquet ub = dbih->userbouquets[bname];
	e2db::channel_reference chref;

	if (! ub.channels.count(chid))
		return error("retrieve", tr("Error", "error").toStdString(), tr("Channel reference \"%1\" not exists.", "error").arg(chid.data()).toStdString());

	chref = ub.channels[chid];

	if (! chref.marker)
		return error("retrieve", tr("Error", "error").toStdString(), tr("Channel reference mismatch \"%1\".", "error").arg(chid.data()).toStdString());

	for (auto & item : fields)
	{
		string key = item->property("field").toString().toStdString();
		string val;

		if (key == "value")
			val = chref.value;
		else if (key == "atype")
			val = to_string(chref.atype);

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
