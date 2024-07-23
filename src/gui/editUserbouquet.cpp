/*!
 * e2-sat-editor/src/gui/editUserbouquet.cpp
 *
 * @link https://github.com/ctlcltd/e2-sat-editor
 * @copyright e2 SAT Editor Team
 * @author Leonardo Laureti
 * @version 1.6.0
 * @license MIT License
 * @license GNU GPLv3 License
 */

#include <cstring>
#include <algorithm>

#include <QtGlobal>
#include <QTimer>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QComboBox>
#include <QPushButton>

#include "platforms/platform.h"

#include "editUserbouquet.h"

using std::to_string, std::sort;

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

	QGroupBox* dtl2 = new QGroupBox(tr("Parameters", "dialog"));
	QFormLayout* dtf2 = new QFormLayout;
	dtf2->setRowWrapPolicy(QFormLayout::WrapAllRows);

	QWidget* dtw20 = new QWidget;
	dtw20->setMinimumHeight(50);
	QHBoxLayout* dtb20 = new QHBoxLayout(dtw20);
	dtb20->setContentsMargins(0, 0, 0, 0);
	dtf2->addRow(tr("Entry flag"), dtw20);

	QLineEdit* dtf2uf = new QLineEdit;
	dtf2uf->setProperty("field", "utype");
	dtf2uf->setVisible(false);
	fields.emplace_back(dtf2uf);
	dtf2uf->setMinimumWidth(50);
	dtf2uf->setMaxLength(9);
	dtf2uf->setValidator(new QIntValidator);

	QComboBox* dtf2uc = new QComboBox;
	dtf2uc->setMaximumWidth(110);
	dtf2uc->setValidator(new QIntValidator);
	platform::osComboBox(dtf2uc);

	vector<int> utypes;
	for (auto & x : e2db::UTYPE_EXT_TYPE)
	{
		utypes.emplace_back(x.first);
	}
	sort(utypes.begin(), utypes.end());
	for (auto & q : utypes)
	{
		string pad = q > 9 ? "  " : "  ";
		dtf2uc->addItem(QString::fromStdString(to_string(q) + pad + e2db::UTYPE_EXT_LABEL.at(q)), q);
	}

	QPushButton* dtf2ub = new QPushButton;
	dtf2ub->setDefault(false);
	dtf2ub->setCheckable(true);
	dtf2ub->setText(tr("custom"));
	dtf2ub->setChecked(false);

	dtf2ub->connect(dtf2ub, &QPushButton::pressed, [=]() {
		// delay too fast
		QTimer::singleShot(100, [=]() {
			if (dtf2uc->isHidden())
			{
				dtf2uf->hide();
				dtf2uc->show();
				dtf2ub->setChecked(true);
				dtf2uc->setFocus();
			}
			else
			{
				dtf2uc->hide();
				dtf2uf->show();
				dtf2ub->setChecked(false);
				dtf2uf->setFocus();
			}
		});
	});

	dtf2uf->connect(dtf2uf, &QLineEdit::textChanged, [=](QString text) {
		int index = dtf2uc->findData(text, Qt::UserRole);
		dtf2uc->setCurrentIndex(index);
	});

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
	dtf2uc->connect(dtf2uc, &QComboBox::currentIndexChanged, [=](int index) {
#else
	dtf2uc->connect(dtf2uc, QOverload<int>::of(&QComboBox::currentIndexChanged), [=](int index) {
#endif
		if (index == -1 && ! dtf2ub->isChecked())
		{
			dtf2uc->hide();
			dtf2uf->show();
			dtf2uf->setFocus();
			dtf2ub->setChecked(true);
		}
		else
		{
			dtf2ub->setChecked(false);
			dtf2uf->setText(dtf2uc->currentData().toString());
		}
	});
	dtf2uf->setText(dtf2uc->currentData().toString());

	dtb20->addWidget(dtf2uc);
	dtb20->addWidget(dtf2uf);
	dtb20->addWidget(dtf2ub);

	dtl0->setLayout(dtf0);
	dtl1->setLayout(dtf1);
	dtl2->setLayout(dtf2);

	dtform->addWidget(dtl0, 0, 0);
	dtform->addItem(new QSpacerItem(0, 18), 1, 0);
	dtform->addWidget(dtl2, 2, 0);
	dtform->addItem(new QSpacerItem(0, 18), 3, 0);
	dtform->addWidget(dtl1, 4, 0);
}

void editUserbouquet::store()
{
	debug("store");

	auto* dbih = this->data->dbih;

	e2db::userbouquet ub;

	if (this->state.edit)
	{
		if (dbih->userbouquets.count(bname))
			ub = dbih->userbouquets[bname];
		else
			return error("store", tr("Error", "error").toStdString(), tr("Userbouquet \"%1\" not exists.", "error").arg(bname.data()).toStdString());
	}

	for (auto & item : fields)
	{
		string key = item->property("field").toString().toStdString();
		string val;

		if (QLineEdit* field = qobject_cast<QLineEdit*>(item))
			val = field->text().toStdString();
		else if (QComboBox* field = qobject_cast<QComboBox*>(item))
			val = field->currentData().toString().toStdString();

		if (key == "utype")
			ub.utype = val.empty() ? e2db::ATYPE::bouquet_regular : std::stoi(val);
		else if (key == "rname")
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

		if (key == "utype")
			val = to_string(ub.utype);
		else if (key == "rname")
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

}
