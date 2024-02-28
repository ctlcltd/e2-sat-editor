/*!
 * e2-sat-editor/src/gui/editFavourite.cpp
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
#include <QLabel>
#include <QLineEdit>
#include <QComboBox>
#include <QTextEdit>
#include <QPushButton>
#ifdef Q_OS_WIN
#include <QStyleFactory>
#include <QScrollBar>
#endif

#include "platforms/platform.h"

#include "editFavourite.h"

using std::to_string, std::sort, std::reverse;

using namespace e2se;

namespace e2se_gui
{

editFavourite::editFavourite(dataHandler* data)
{
	this->log = new logger("gui", "editFavourite");

	this->data = data;
}

editFavourite::~editFavourite()
{
	debug("~editFavourite");

	delete this->log;
}

void editFavourite::display(QWidget* cwid)
{
	layout(cwid);

	if (this->state.edit)
		retrieve();

	dial->exec();
}

void editFavourite::show(bool retr)
{
	debug("show");

	if (dtform == nullptr)
		return;

	if (retr && this->state.edit)
		retrieve();
}

void editFavourite::reset()
{
	debug("reset");

	if (dtform == nullptr)
		return;

	for (auto & item : fields)
	{
		if (QLineEdit* field = qobject_cast<QLineEdit*>(item))
			field->setText(NULL);
		else if (QComboBox* field = qobject_cast<QComboBox*>(item))
			field->setCurrentIndex(0);
	}
}

void editFavourite::layout(QWidget* cwid)
{
	this->dialAbstract::layout(cwid);

	QString dtitle = this->state.edit ? tr("Edit Favourite", "dialog") : tr("New Favourite", "dialog");
	dial->setWindowTitle(dtitle);

	referenceLayout();
	streamLayout();
}

void editFavourite::referenceLayout()
{
	debug("referenceLayout");

	QGroupBox* dtl0 = new QGroupBox(tr("Reference", "dialog"));
	QHBoxLayout* dth0 = new QHBoxLayout;
	dth0->setSpacing(12);

	QFormLayout* dtf0 = new QFormLayout;
	dtf0->setRowWrapPolicy(QFormLayout::WrapAllRows);

	QWidget* dtw10 = new QWidget;
	dtw10->setMinimumHeight(50);
	QHBoxLayout* dtb10 = new QHBoxLayout(dtw10);
	dtb10->setContentsMargins(0, 0, 0, 0);
	dtf0->addRow(tr("Favourite type"), dtw10);

	QLineEdit* dtf0ft = new QLineEdit;
	dtf0ft->setProperty("field", "etype");
	dtf0ft->setVisible(false);
	fields.emplace_back(dtf0ft);
	dtf0ft->setMinimumWidth(50);
	dtf0ft->setValidator(new QIntValidator);

	QComboBox* dtf0fc = new QComboBox;
	dtf0fc->setMaximumWidth(110);
	dtf0fc->setValidator(new QIntValidator);
	platform::osComboBox(dtf0fc);

	vector<int> etypes;
	for (auto & x : e2db::ETYPE_EXT_TYPE)
	{
		etypes.emplace_back(x.first);
	}
	reverse(etypes.begin(), etypes.end());
	for (auto & q : etypes)
	{
		string pad = q > 9 ? "  " : "  ";
		dtf0fc->addItem(QString::fromStdString(to_string(q) + pad + e2db::ETYPE_EXT_LABEL.at(q)), q);
	}

	QPushButton* dtf0fb = new QPushButton;
	dtf0fb->setDefault(false);
	dtf0fb->setCheckable(true);
	dtf0fb->setText(tr("custom"));
	dtf0fb->setChecked(false);

	dtf0fb->connect(dtf0fb, &QPushButton::pressed, [=]() {
		// delay too fast
		QTimer::singleShot(100, [=]() {
			if (dtf0fc->isHidden())
			{
				dtf0ft->hide();
				dtf0fc->show();
				dtf0fb->setChecked(true);
				dtf0fc->setFocus();
			}
			else
			{
				dtf0fc->hide();
				dtf0ft->show();
				dtf0fb->setChecked(false);
				dtf0ft->setFocus();
			}
		});
	});

	dtf0ft->connect(dtf0ft, &QLineEdit::textChanged, [=](QString text) {
		int index = dtf0fc->findData(text, Qt::UserRole);
		dtf0fc->setCurrentIndex(index);
	});

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
	dtf0fc->connect(dtf0fc, &QComboBox::currentIndexChanged, [=](int index) {
#else
	dtf0fc->connect(dtf0fc, QOverload<int>::of(&QComboBox::currentIndexChanged), [=](int index) {
#endif
		if (index == -1 && ! dtf0fb->isChecked())
		{
			dtf0fc->hide();
			dtf0ft->show();
			dtf0ft->setFocus();
			dtf0fb->setChecked(true);
		}
		else
		{
			dtf0fb->setChecked(false);
			dtf0ft->setText(dtf0fc->currentData().toString());
		}
	});

	dtb10->addWidget(dtf0fc);
	dtb10->addWidget(dtf0ft);
	dtb10->addWidget(dtf0fb);

	QWidget* dtw11 = new QWidget;
	dtw11->setMinimumHeight(50);
	QHBoxLayout* dtb11 = new QHBoxLayout(dtw11);
	dtb11->setContentsMargins(0, 0, 0, 0);
	dtf0->addRow(tr("Entry flag"), dtw11);

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

	QFormLayout* dtf1 = new QFormLayout;
	dtf1->setRowWrapPolicy(QFormLayout::WrapAllRows);

	QWidget* dtw12 = new QWidget;
	dtw12->setMinimumHeight(50);
	QHBoxLayout* dtb12 = new QHBoxLayout(dtw12);
	dtb12->setContentsMargins(0, 0, 0, 0);
	dtf1->addRow(tr("Service type"), dtw12);

	QLineEdit* dtf1st = new QLineEdit;
	dtf1st->setProperty("field", "anum");
	dtf1st->setVisible(false);
	fields.emplace_back(dtf1st);
	dtf1st->setMinimumWidth(50);
	dtf1st->setValidator(new QIntValidator);

	QComboBox* dtf1sc = new QComboBox;
	dtf1sc->setMaximumWidth(100);
	dtf1sc->setValidator(new QIntValidator);
	platform::osComboBox(dtf1sc);

	vector<int> stypes;
	for (auto & x : e2db::STYPE_EXT_TYPE)
	{
		stypes.emplace_back(x.first);
	}
	sort(stypes.begin(), stypes.end());
	for (auto & q : stypes)
	{
		string pad = q > 9 ? "  " : "  ";
		dtf1sc->addItem(QString::fromStdString(to_string(q) + pad + e2db::STYPE_EXT_LABEL.at(q)), q);
	}

	QPushButton* dtf1sb = new QPushButton;
	dtf1sb->setDefault(false);
	dtf1sb->setCheckable(true);
	dtf1sb->setText(tr("custom"));
	dtf1sb->setChecked(false);

	dtf1sb->connect(dtf1sb, &QPushButton::pressed, [=]() {
		// delay too fast
		QTimer::singleShot(100, [=]() {
			if (dtf1sc->isHidden())
			{
				dtf1st->hide();
				dtf1sc->show();
				dtf1sb->setChecked(true);
				dtf1sc->setFocus();
			}
			else
			{
				dtf1sc->hide();
				dtf1st->show();
				dtf1sb->setChecked(false);
				dtf1st->setFocus();
			}
		});
	});

	dtf1st->connect(dtf1st, &QLineEdit::textChanged, [=](QString text) {
		int index = dtf1sc->findData(text, Qt::UserRole);
		dtf1sc->setCurrentIndex(index);
	});

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
	dtf1sc->connect(dtf1sc, &QComboBox::currentIndexChanged, [=](int index) {
#else
	dtf1sc->connect(dtf1sc, QOverload<int>::of(&QComboBox::currentIndexChanged), [=](int index) {
#endif
		if (index == -1 && ! dtf1sb->isChecked())
		{
			dtf1sc->hide();
			dtf1st->show();
			dtf1st->setFocus();
			dtf1sb->setChecked(true);
		}
		else
		{
			dtf1sb->setChecked(false);
			dtf1st->setText(dtf1sc->currentData().toString());
		}
	});

	dtb12->addWidget(dtf1sc);
	dtb12->addWidget(dtf1st);
	dtb12->addWidget(dtf1sb);

	QHBoxLayout* dtb20 = new QHBoxLayout;
	dtf1->addRow(tr("Service ID"), dtb20);
	QLineEdit* dtf1sx = new QLineEdit;
	dtf1sx->setProperty("field", "ssid");
	fields.emplace_back(dtf1sx);
	dtf1sx->setMaximumWidth(60);
	dtf1sx->setValidator(new QIntValidator(0, 999999999));
	platform::osLineEdit(dtf1sx);
	dtb20->addWidget(dtf1sx);
	dtb20->addWidget(new QLabel("[SID]"));

	QFormLayout* dtf2 = new QFormLayout;
	dtf2->setRowWrapPolicy(QFormLayout::WrapAllRows);
	dtf2->setVerticalSpacing(5);

	QHBoxLayout* dtb21 = new QHBoxLayout;
	dtf2->addRow(tr("Transport ID"), dtb21);
	dtf2->addItem(new QSpacerItem(0, 0));
	QLineEdit* dtf2ts = new QLineEdit;
	dtf2ts->setProperty("field", "tsid");
	fields.emplace_back(dtf2ts);
	dtf2ts->setMaximumWidth(60);
	dtf2ts->setValidator(new QIntValidator(0, 65535));
	platform::osLineEdit(dtf2ts);
	dtb21->addWidget(dtf2ts);
	dtb21->addWidget(new QLabel("[TSID]"));
	dtf2->addItem(new QSpacerItem(0, 0));

	QLineEdit* dtf2ns = new QLineEdit;
	dtf2ns->setProperty("field", "dvbns");
	fields.emplace_back(dtf2ns);
	dtf2ns->setMaximumWidth(80);
	dtf2ns->setInputMask(">HHHHHHHH");
	dtf2ns->setMaxLength(9);
	platform::osLineEdit(dtf2ns);
	dtf2->addRow(tr("DVBNS"), dtf2ns);
	dtf2->addItem(new QSpacerItem(0, 0));

	QLineEdit* dtf2on = new QLineEdit;
	dtf2on->setProperty("field", "onid");
	fields.emplace_back(dtf2on);
	dtf2on->setMaximumWidth(60);
	dtf2on->setValidator(new QIntValidator(0, 65535));
	platform::osLineEdit(dtf2on);
	dtf2->addRow(tr("ONID"), dtf2on);
	dtf0->addItem(new QSpacerItem(0, 0));

	dth0->addItem(dtf0);
	dth0->addItem(dtf1);
	dth0->addItem(dtf2);

	dtl0->setLayout(dth0);
	dtform->addWidget(dtl0, 0, 0);
}

void editFavourite::streamLayout()
{
	debug("streamLayout");

	QGroupBox* dtl1 = new QGroupBox(tr("Stream", "dialog"));
	QFormLayout* dtf0 = new QFormLayout;
	dtf0->setRowWrapPolicy(QFormLayout::WrapAllRows);
	dtl1->setStyleSheet("#dial_field_description { margin-right: 1px; margin-left: 1px }");

	QTextEdit* dtf0rr = new QTextEdit;
	dtf0rr->setProperty("field", "uri");
	dtf0rr->setAcceptRichText(false);
	fields.emplace_back(dtf0rr);
	dtf0rr->setMinimumWidth(280);
	dtf0rr->setMaximumHeight(30);
#ifdef Q_OS_WIN
	if (theme::absLuma() || ! theme::isDefault())
	{
		QStyle* style = QStyleFactory::create("fusion");
		dtf0rr->verticalScrollBar()->setStyle(style);
		dtf0rr->horizontalScrollBar()->setStyle(style);
	}
#endif
	platform::osTextEdit(dtf0rr);
	dtf0->addRow(tr("Channel URL"), dtf0rr);
	dtf0->addItem(new QSpacerItem(0, 0));

	QVBoxLayout* dtb11 = new QVBoxLayout;
	dtf0->addRow(tr("Channel name"), dtb11);
	//TODO test
	QLabel* dtd11 = new QLabel(QString("<small>%1</small>").arg(tr("custom channel name, leave empty to auto-fill with service reference")));
	dtd11->setObjectName("dial_field_description");
	QLineEdit* dtf0rv = new QLineEdit;
	dtf0rv->setProperty("field", "value");
	fields.emplace_back(dtf0rv);
	dtf0rv->setMinimumWidth(240);
	dtf0rv->setMaxLength(255);
	platform::osLineEdit(dtf0rv);
	dtb11->addWidget(dtf0rv);
	dtb11->addWidget(dtd11);
	dtf0->addItem(new QSpacerItem(0, 0));

	dtl1->setLayout(dtf0);
	dtform->addWidget(dtl1, 1, 0);
}

void editFavourite::store()
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
	}

	for (auto & item : fields)
	{
		string key = item->property("field").toString().toStdString();
		string val;

		if (QLineEdit* field = qobject_cast<QLineEdit*>(item))
		{
			if (key == "dvbns")
				val = to_string(dbih->value_transponder_dvbns(field->text().toStdString()));
			else
				val = field->text().toStdString();
		}
		else if (QComboBox* field = qobject_cast<QComboBox*>(item))
		{
			val = field->currentData().toString().toStdString();
		}
		else if (QTextEdit* field = qobject_cast<QTextEdit*>(item))
		{
			val = field->toPlainText().toStdString();
		}

		if (key == "etype")
			chref.etype = val.empty() ? 1 : std::stoi(val);
		else if (key == "atype")
			chref.atype = val.empty() ? 0 : std::stoi(val);
		else if (key == "anum")
			chref.anum = val.empty() ? 0 : std::stoi(val);
		else if (key == "ssid")
			chref.ref.ssid = val.empty() ? 0 : std::stoi(val);
		else if (key == "tsid")
			chref.ref.tsid = val.empty() ? 0 : std::stoi(val);
		else if (key == "dvbns")
			chref.ref.dvbns = val.empty() ? 0 : std::stoi(val);
		else if (key == "onid")
			chref.ref.onid = val.empty() ? 0 : std::stoi(val);
		else if (key == "uri")
			chref.uri = val;
		else if (key == "value")
			chref.value = val;
	}

	if (chref.chid != this->chid)
	{
		char nw_chid[25];

		if (chref.marker)
			// %4d:%2x:%d
			std::snprintf(nw_chid, 25, "1:%d:%x:%d", chref.atype, chref.anum != 0 ? chref.anum : dbih->db.imarkers + 1, ub.index);
		else if (chref.stream)
			// %4d:%4x:%d
			std::snprintf(nw_chid, 25, "2:%d:%x:%d", chref.atype, dbih->db.istreams + 1, ub.index);
		else
			// %4x:%4x:%8x
			std::snprintf(nw_chid, 25, "%x:%x:%x", chref.ref.ssid, chref.ref.tsid, chref.ref.dvbns);

		chref.chid = nw_chid;
	}

	//TODO chref.ref <--> ch changes

	if (this->state.edit)
		this->chid = dbih->editChannelReference(chid, chref, bname);
	else
		this->chid = dbih->addChannelReference(chref, bname);

	this->changes = true;
}

void editFavourite::retrieve()
{
	debug("retrieve");

	auto* dbih = this->data->dbih;

	if (! dbih->userbouquets.count(bname))
		return error("retrieve", tr("Error", "error").toStdString(), tr("Userbouquet \"%1\" not exists.", "error").arg(bname.data()).toStdString());

	e2db::userbouquet& ub = dbih->userbouquets[bname];

	if (! ub.channels.count(chid))
		return error("retrieve", tr("Error", "error").toStdString(), tr("Channel reference \"%1\" not exists.", "error").arg(chid.data()).toStdString());

	retrieve(chid);
}

void editFavourite::retrieve(string chid)
{
	debug("retrieve", "chid", chid);

	auto* dbih = this->data->dbih;

	if (! dbih->userbouquets.count(bname))
		return error("retrieve", tr("Error", "error").toStdString(), tr("Userbouquet \"%1\" not exists.", "error").arg(bname.data()).toStdString());

	e2db::userbouquet ub = dbih->userbouquets[bname];
	e2db::channel_reference chref;

	if (this->state.edit)
	{
		if (! ub.channels.count(chid))
			return error("retrieve", tr("Error", "error").toStdString(), tr("Channel reference \"%1\" not exists.", "error").arg(chid.data()).toStdString());

		chref = ub.channels[chid];
	}

	for (auto & item : fields)
	{
		string key = item->property("field").toString().toStdString();
		string val;

		if (key == "etype")
			val = to_string(chref.etype);
		else if (key == "atype")
			val = to_string(chref.atype);
		else if (key == "anum")
			val = to_string(chref.anum);
		else if (key == "ssid")
			val = to_string(chref.ref.ssid);
		else if (key == "tsid")
			val = to_string(chref.ref.tsid);
		else if (key == "dvbns")
			val = to_string(chref.ref.dvbns);
		else if (key == "onid")
			val = to_string(chref.ref.onid);
		else if (key == "uri")
			val = chref.uri;
		else if (key == "value")
			val = chref.value;

		if (QLineEdit* field = qobject_cast<QLineEdit*>(item))
		{
			if (key == "dvbns")
				field->setText(QString::fromStdString(dbih->value_transponder_dvbns(std::stoi(val))));
			else
				field->setText(QString::fromStdString(val));
		}
		else if (QComboBox* field = qobject_cast<QComboBox*>(item))
		{
			int index = field->findData(QString::fromStdString(val), Qt::UserRole);
			field->setCurrentIndex(index);
		}
		else if (QTextEdit* field = qobject_cast<QTextEdit*>(item))
		{
			field->setPlainText(QString::fromStdString(val));
		}
	}
}

void editFavourite::setEditId(string chid, string bname)
{
	debug("setEditId");

	this->state.edit = true;
	this->chid = chid;
	this->bname = bname;
}

string editFavourite::getEditId()
{
	debug("getEditId");

	return this->chid;
}

void editFavourite::setAddId(string bname)
{
	debug("setAddId");

	this->bname = bname;
}

string editFavourite::getAddId()
{
	debug("getAddId");

	return this->chid;
}

}
