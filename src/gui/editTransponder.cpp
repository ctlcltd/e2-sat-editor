/*!
 * e2-sat-editor/src/gui/editTransponder.cpp
 *
 * @link https://github.com/ctlcltd/e2-sat-editor
 * @copyright e2 SAT Editor Team
 * @author Leonardo Laureti
 * @version 1.8.0
 * @license MIT License
 * @license GNU GPLv3 License
 */

#include <QtGlobal>
#include <QTimer>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QComboBox>
#include <QPushButton>

#include "platforms/platform.h"

#include "editTransponder.h"

using namespace e2se;

namespace e2se_gui
{

editTransponder::editTransponder(dataHandler* data)
{
	this->log = new logger("gui", "editTransponder");

	this->data = data;
	this->state.yx = -1;
}

editTransponder::~editTransponder()
{
	debug("~editTransponder");

	delete this->log;
}

void editTransponder::display(QWidget* cwid)
{
	layout(cwid);

	if (this->state.edit)
		retrieve();

	dial->exec();
}

void editTransponder::show(bool retr)
{
	debug("show");

	if (dtform == nullptr)
		return;

	if (retr && this->state.edit)
		retrieve();
}

void editTransponder::reset()
{
	debug("reset");

	if (dtform == nullptr)
		return;

	for (auto & item : fields)
	{
		if (QLineEdit* field = qobject_cast<QLineEdit*>(item))
			field->setText(NULL);
	}
}

void editTransponder::layout(QWidget* cwid)
{
	this->dialAbstract::layout(cwid);

	QString dtitle = this->state.edit ? tr("Edit Transponder", "dialog") : tr("New Transponder", "dialog");
	dial->setWindowTitle(dtitle);

	QStringList types = QStringList ({tr("Satellite"), tr("Terrestrial"), tr("Cable"), tr("ATSC")});

	QGroupBox* dtl0 = new QGroupBox(tr("Transponder", "dialog"));
	QFormLayout* dtf0 = new QFormLayout;
	dtf0->setRowWrapPolicy(QFormLayout::WrapAllRows);

	this->dtf0yx = new QComboBox;
	dtf0yx->setProperty("field", "ytype");
	fields.emplace_back(dtf0yx);
	dtf0yx->setMaximumWidth(100);
	platform::osComboBox(dtf0yx);
	dtf0->addRow(tr("Type"), dtf0yx);
	dtf0->addItem(new QSpacerItem(0, 0));
	for (int i = 0; i < types.size(); i++)
	{
		dtf0yx->addItem(types.at(i), i);
	}
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
	dtf0yx->connect(dtf0yx, &QComboBox::currentIndexChanged, [=](int index) {
#else
	dtf0yx->connect(dtf0yx, QOverload<int>::of(&QComboBox::currentIndexChanged), [=](int index) {
#endif
		this->typeComboChanged(index);
	});
	if (! this->state.edit)
	{
		typeComboChanged(0);
	}

	QHBoxLayout* dtb01 = new QHBoxLayout;
	dtb01->setSpacing(10);
	dtf0->addRow(tr("Transport ID"), dtb01);
	dtf0->addItem(new QSpacerItem(0, 0));
	QLineEdit* dtf0ts = new QLineEdit;
	dtf0ts->setProperty("field", "tsid");
	fields.emplace_back(dtf0ts);
	dtf0ts->setMaximumWidth(60);
	dtf0ts->setValidator(new QIntValidator(0, 65535));
	platform::osLineEdit(dtf0ts);
	dtb01->addWidget(dtf0ts);
	dtb01->addWidget(new QLabel("[TSID]"));

	QWidget* dtw10 = new QWidget;
	dtw10->setMinimumHeight(50);
	QHBoxLayout* dtb10 = new QHBoxLayout(dtw10);
	dtb10->setContentsMargins(0, 0, 0, 0);
	dtf0->addRow(tr("DVBNS"), dtw10);
	dtf0->addItem(new QSpacerItem(0, 0));

	QLineEdit* dtf0ns = new QLineEdit;
	dtf0ns->setProperty("field", "dvbns");
	fields.emplace_back(dtf0ns);
	dtf0ns->setMaximumWidth(80);
	dtf0ns->setInputMask(">HHHHHHHH");
	dtf0ns->setMaxLength(9);
	platform::osLineEdit(dtf0ns);

	QPushButton* dtf0nb = new QPushButton;
	dtf0nb->setDefault(false);
	dtf0nb->setText(tr("calc"));

	dtf0nb->connect(dtf0nb, &QPushButton::pressed, [=]() {
		// delay too fast
		QTimer::singleShot(100, [=]() {
			this->computeDvbns();

			dtf0ns->setFocus();
		});
	});

	dtb10->addWidget(dtf0ns);
	dtb10->addWidget(dtf0nb);

	QLineEdit* dtf0on = new QLineEdit;
	dtf0on->setProperty("field", "onid");
	fields.emplace_back(dtf0on);
	dtf0on->setMaximumWidth(60);
	dtf0on->setValidator(new QIntValidator(0, 65535));
	platform::osLineEdit(dtf0on);
	dtf0->addRow(tr("ONID"), dtf0on);
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

	afields.insert(afields.begin(), fields.begin(), fields.end());

	dtl0->setLayout(dtf0);
	dtform->addWidget(dtl0, 0, 0);
}

void editTransponder::layoutChange(int vx)
{
	debug("layoutChange");

	for (int i = 1; i < 4; i++)
	{
		QLayoutItem* item;
		item = dtform->itemAtPosition(0, i);
		if (item != nullptr)
		{
			item->widget()->setVisible(false);
			delete item->widget();

#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
			dtform->takeAt(dtform->indexOf(item));
#endif
		}
	}
	fields.clear();
	fields.insert(fields.begin(), afields.begin(), afields.end());

	switch (vx)
	{
		case e2db::YTYPE::satellite:
			leadSatLayout();
			sideSatLayout();
			thirdSatLayout();
		break;
		case e2db::YTYPE::terrestrial:
			leadTerrestrialLayout();
			sideTerrestrialLayout();
			thirdTerrestrialLayout();
		break;
		case e2db::YTYPE::cable:
			leadCableLayout();
			sideCableLayout();
			thirdCableLayout();
		break;
		case e2db::YTYPE::atsc:
			leadAtscLayout();
			thirdAtscLayout();
		break;
	}
}

void editTransponder::leadSatLayout()
{
	debug("leadSatLayout");

	QGroupBox* dtl0 = new QGroupBox(" ");
	QFormLayout* dtf0 = new QFormLayout;
	dtf0->setRowWrapPolicy(QFormLayout::WrapAllRows);

	QLineEdit* dtf0sf = new QLineEdit;
	dtf0sf->setProperty("field", "s_freq");
	fields.emplace_back(dtf0sf);
	dtf0sf->setMinimumWidth(100);
	dtf0sf->setValidator(new QIntValidator(0, 999999));
	dtf0sf->setMaxLength(6);
	platform::osLineEdit(dtf0sf);
	dtf0->addRow(tr("Frequency"), dtf0sf);
	dtf0->addItem(new QSpacerItem(0, 0));

	QComboBox* dtf0sp = new QComboBox;
	dtf0sp->setProperty("field", "s_pol");
	fields.emplace_back(dtf0sp);
	dtf0sp->setMaximumWidth(100);
	dtf0sp->setValidator(new QIntValidator);
	platform::osComboBox(dtf0sp);
	dtf0->addRow(tr("Polarization"), dtf0sp);
	dtf0->addItem(new QSpacerItem(0, 0));
	dtf0sp->addItem(tr("empty", "ui"), -1);
	for (int i = 0; i < (int (sizeof(e2db::SAT_POL) / sizeof(e2db::SAT_POL[0]))); i++)
	{
		string w = e2db::SAT_POL[i];
		dtf0sp->addItem(QString::fromStdString(w), i);
	}

	QLineEdit* dtf0ss = new QLineEdit;
	dtf0ss->setProperty("field", "s_sr");
	fields.emplace_back(dtf0ss);
	dtf0ss->setMinimumWidth(100);
	dtf0ss->setValidator(new QIntValidator(-1, 65535));
	dtf0ss->setMaxLength(6);
	platform::osLineEdit(dtf0ss);
	dtf0->addRow(tr("Symbol Rate"), dtf0ss);
	dtf0->addItem(new QSpacerItem(0, 0));

	QComboBox* dtf0sc = new QComboBox;
	dtf0sc->setProperty("field", "s_fec");
	fields.emplace_back(dtf0sc);
	dtf0sc->setMaximumWidth(100);
	dtf0sc->setValidator(new QIntValidator);
	platform::osComboBox(dtf0sc);
	dtf0->addRow(tr("FEC"), dtf0sc);
	dtf0->addItem(new QSpacerItem(0, 0));
	dtf0sc->addItem(tr("empty", "ui"), -1);
	for (int i = 0; i < (int (sizeof(e2db::SAT_FEC) / sizeof(e2db::SAT_FEC[0]))); i++)
	{
		string w = e2db::SAT_FEC[i];
		dtf0sc->addItem(QString::fromStdString(w), i);
	}

	QComboBox* dtf0sy = new QComboBox;
	dtf0sy->setProperty("field", "s_sys");
	fields.emplace_back(dtf0sy);
	dtf0sy->setMaximumWidth(100);
	dtf0sy->setValidator(new QIntValidator);
	platform::osComboBox(dtf0sy);
	dtf0->addRow(tr("System"), dtf0sy);
	dtf0->addItem(new QSpacerItem(0, 0));
	dtf0sy->addItem(tr("empty", "ui"), -1);
	for (int i = 0; i < (int (sizeof(e2db::SAT_SYS) / sizeof(e2db::SAT_SYS[0]))); i++)
	{
		string w = e2db::SAT_SYS[i];
		dtf0sy->addItem(QString::fromStdString(w), i);
	}

	dtl0->setLayout(dtf0);
	dtform->addWidget(dtl0, 0, 1);
}

void editTransponder::leadTerrestrialLayout()
{
	debug("leadTerrestrialLayout");

	QGroupBox* dtl0 = new QGroupBox(" ");
	QFormLayout* dtf0 = new QFormLayout;
	dtf0->setRowWrapPolicy(QFormLayout::WrapAllRows);

	QLineEdit* dtf0tf = new QLineEdit;
	dtf0tf->setProperty("field", "t_freq");
	fields.emplace_back(dtf0tf);
	dtf0tf->setMinimumWidth(100);
	dtf0tf->setValidator(new QIntValidator(0, 999999));
	dtf0tf->setMaxLength(6);
	platform::osLineEdit(dtf0tf);
	dtf0->addRow(tr("Frequency"), dtf0tf);
	dtf0->addItem(new QSpacerItem(0, 0));

	QComboBox* dtf0tm = new QComboBox;
	dtf0tm->setProperty("field", "t_tmod");
	fields.emplace_back(dtf0tm);
	dtf0tm->setMaximumWidth(100);
	dtf0tm->setValidator(new QIntValidator);
	platform::osComboBox(dtf0tm);
	dtf0->addRow(tr("Constellation"), dtf0tm);
	dtf0->addItem(new QSpacerItem(0, 0));
	dtf0tm->addItem(tr("empty", "ui"), -1);
	for (int i = 0; i < (int (sizeof(e2db::TER_MOD) / sizeof(e2db::TER_MOD[0]))); i++)
	{
		string w = e2db::TER_MOD[i];
		dtf0tm->addItem(QString::fromStdString(w), i);
	}

	QComboBox* dtf0tb = new QComboBox;
	dtf0tb->setProperty("field", "t_band");
	fields.emplace_back(dtf0tb);
	dtf0tb->setMaximumWidth(100);
	dtf0tb->setValidator(new QIntValidator);
	platform::osComboBox(dtf0tb);
	dtf0->addRow(tr("Bandwidth"), dtf0tb);
	dtf0->addItem(new QSpacerItem(0, 0));
	dtf0tb->addItem(tr("empty", "ui"), -1);
	for (int i = 0; i < (int (sizeof(e2db::TER_BAND) / sizeof(e2db::TER_BAND[0]))); i++)
	{
		string w = e2db::TER_BAND[i];
		dtf0tb->addItem(QString::fromStdString(w), i);
	}

	QComboBox* dtf0th = new QComboBox;
	dtf0th->setProperty("field", "t_hpfec");
	fields.emplace_back(dtf0th);
	dtf0th->setMaximumWidth(100);
	dtf0th->setValidator(new QIntValidator);
	platform::osComboBox(dtf0th);
	dtf0->addRow(tr("HP FEC"), dtf0th);
	dtf0->addItem(new QSpacerItem(0, 0));
	dtf0th->addItem(tr("empty", "ui"), -1);
	for (int i = 0; i < (int (sizeof(e2db::TER_FEC) / sizeof(e2db::TER_FEC[0]))); i++)
	{
		string w = e2db::TER_FEC[i];
		dtf0th->addItem(QString::fromStdString(w), i);
	}

	QComboBox* dtf0tl = new QComboBox;
	dtf0tl->setProperty("field", "t_lpfec");
	fields.emplace_back(dtf0tl);
	dtf0tl->setMaximumWidth(100);
	dtf0tl->setValidator(new QIntValidator);
	platform::osComboBox(dtf0tl);
	dtf0->addRow(tr("LP FEC"), dtf0tl);
	dtf0->addItem(new QSpacerItem(0, 0));
	dtf0tl->addItem(tr("empty", "ui"), -1);
	for (int i = 0; i < (int (sizeof(e2db::TER_FEC) / sizeof(e2db::TER_FEC[0]))); i++)
	{
		string w = e2db::TER_FEC[i];
		dtf0tl->addItem(QString::fromStdString(w), i);
	}

	QComboBox* dtf0ty = new QComboBox;
	dtf0ty->setProperty("field", "t_sys");
	fields.emplace_back(dtf0ty);
	dtf0ty->setMaximumWidth(100);
	dtf0ty->setValidator(new QIntValidator);
	platform::osComboBox(dtf0ty);
	dtf0->addRow(tr("System"), dtf0ty);
	dtf0->addItem(new QSpacerItem(0, 0));
	dtf0ty->addItem(tr("empty", "ui"), -1);
	for (int i = 0; i < (int (sizeof(e2db::TER_SYS) / sizeof(e2db::TER_SYS[0]))); i++)
	{
		string w = e2db::TER_SYS[i];
		dtf0ty->addItem(QString::fromStdString(w), i);
	}

	dtl0->setLayout(dtf0);
	dtform->addWidget(dtl0, 0, 1);
}

void editTransponder::leadCableLayout()
{
	debug("leadCableLayout");

	QGroupBox* dtl0 = new QGroupBox(" ");
	QFormLayout* dtf0 = new QFormLayout;
	dtf0->setRowWrapPolicy(QFormLayout::WrapAllRows);

	QLineEdit* dtf0cf = new QLineEdit;
	dtf0cf->setProperty("field", "c_freq");
	fields.emplace_back(dtf0cf);
	dtf0cf->setMinimumWidth(100);
	dtf0cf->setValidator(new QIntValidator(0, 999999));
	dtf0cf->setMaxLength(6);
	platform::osLineEdit(dtf0cf);
	dtf0->addRow(tr("Frequency"), dtf0cf);
	dtf0->addItem(new QSpacerItem(0, 0));

	QComboBox* dtf0cm = new QComboBox;
	dtf0cm->setProperty("field", "c_cmod");
	fields.emplace_back(dtf0cm);
	dtf0cm->setMaximumWidth(100);
	dtf0cm->setValidator(new QIntValidator);
	platform::osComboBox(dtf0cm);
	dtf0->addRow(tr("Modulation"), dtf0cm);
	dtf0->addItem(new QSpacerItem(0, 0));
	dtf0cm->addItem(tr("empty", "ui"), -1);
	for (int i = 0; i < (int (sizeof(e2db::CAB_MOD) / sizeof(e2db::CAB_MOD[0]))); i++)
	{
		string w = e2db::CAB_MOD[i];
		dtf0cm->addItem(QString::fromStdString(w), i);
	}

	QLineEdit* dtf0cs = new QLineEdit;
	dtf0cs->setProperty("field", "c_sr");
	fields.emplace_back(dtf0cs);
	dtf0cs->setMinimumWidth(100);
	dtf0cs->setValidator(new QIntValidator(-1, 65535));
	dtf0cs->setMaxLength(6);
	platform::osLineEdit(dtf0cs);
	dtf0->addRow(tr("Symbol Rate"), dtf0cs);
	dtf0->addItem(new QSpacerItem(0, 0));

	QComboBox* dtf0ci = new QComboBox;
	dtf0ci->setProperty("field", "c_cfec");
	fields.emplace_back(dtf0ci);
	dtf0ci->setMaximumWidth(100);
	dtf0ci->setValidator(new QIntValidator);
	platform::osComboBox(dtf0ci);
	dtf0->addRow(tr("Inner FEC"), dtf0ci);
	dtf0->addItem(new QSpacerItem(0, 0));
	dtf0ci->addItem(tr("empty", "ui"), -1);
	for (int i = 0; i < (int (sizeof(e2db::CAB_FEC) / sizeof(e2db::CAB_FEC[0]))); i++)
	{
		string w = e2db::CAB_FEC[i];
		dtf0ci->addItem(QString::fromStdString(w), i);
	}

	dtl0->setLayout(dtf0);
	dtform->addWidget(dtl0, 0, 1);
}

void editTransponder::leadAtscLayout()
{
	debug("leadAtscLayout");

	QGroupBox* dtl0 = new QGroupBox(" ");
	QFormLayout* dtf0 = new QFormLayout;
	dtf0->setRowWrapPolicy(QFormLayout::WrapAllRows);

	QLineEdit* dtf0af = new QLineEdit;
	dtf0af->setProperty("field", "a_freq");
	fields.emplace_back(dtf0af);
	dtf0af->setMinimumWidth(100);
	dtf0af->setValidator(new QIntValidator(0, 999999));
	dtf0af->setMaxLength(6);
	platform::osLineEdit(dtf0af);
	dtf0->addRow(tr("Frequency"), dtf0af);
	dtf0->addItem(new QSpacerItem(0, 0));

	QComboBox* dtf0am = new QComboBox;
	dtf0am->setProperty("field", "a_amod");
	fields.emplace_back(dtf0am);
	dtf0am->setMaximumWidth(100);
	dtf0am->setValidator(new QIntValidator);
	platform::osComboBox(dtf0am);
	dtf0->addRow(tr("Modulation"), dtf0am);
	dtf0->addItem(new QSpacerItem(0, 0));
	dtf0am->addItem(tr("empty", "ui"), -1);
	for (int i = 0; i < (int (sizeof(e2db::ATS_MOD) / sizeof(e2db::ATS_MOD[0]))); i++)
	{
		string w = e2db::ATS_MOD[i];
		dtf0am->addItem(QString::fromStdString(w), i);
	}

	QComboBox* dtf0ay = new QComboBox;
	dtf0ay->setProperty("field", "a_sys");
	fields.emplace_back(dtf0ay);
	dtf0ay->setMaximumWidth(100);
	dtf0ay->setValidator(new QIntValidator);
	platform::osComboBox(dtf0ay);
	dtf0->addRow(tr("System"), dtf0ay);
	dtf0->addItem(new QSpacerItem(0, 0));
	dtf0ay->addItem(tr("empty", "ui"), -1);
	for (int i = 0; i < (int (sizeof(e2db::ATS_SYS) / sizeof(e2db::ATS_SYS[0]))); i++)
	{
		string w = e2db::ATS_SYS[i];
		dtf0ay->addItem(QString::fromStdString(w), i);
	}

	dtl0->setLayout(dtf0);
	dtform->addWidget(dtl0, 0, 1);
}

void editTransponder::sideSatLayout()
{
	debug("sideSatLayout");

	QGroupBox* dtl1 = new QGroupBox(" ");
	QFormLayout* dtf1 = new QFormLayout;
	dtf1->setRowWrapPolicy(QFormLayout::WrapAllRows);

	QComboBox* dtf1sm = new QComboBox;
	dtf1sm->setProperty("field", "s_mod");
	fields.emplace_back(dtf1sm);
	dtf1sm->setMaximumWidth(100);
	dtf1sm->setValidator(new QIntValidator);
	platform::osComboBox(dtf1sm);
	dtf1->addRow(tr("Modulation"), dtf1sm);
	dtf1->addItem(new QSpacerItem(0, 0));
	dtf1sm->addItem(tr("empty", "ui"), -1);
	for (int i = 0; i < (int (sizeof(e2db::SAT_MOD) / sizeof(e2db::SAT_MOD[0]))); i++)
	{
		string w = e2db::SAT_MOD[i];
		dtf1sm->addItem(QString::fromStdString(w), i);
	}

	QComboBox* dtf1si = new QComboBox;
	dtf1si->setProperty("field", "s_inv");
	fields.emplace_back(dtf1si);
	dtf1si->setMaximumWidth(100);
	dtf1si->setValidator(new QIntValidator);
	platform::osComboBox(dtf1si);
	dtf1->addRow(tr("Inversion"), dtf1si);
	dtf1->addItem(new QSpacerItem(0, 0));
	dtf1si->addItem(tr("empty", "ui"), -1);
	for (int i = 0; i < (int (sizeof(e2db::SAT_INV) / sizeof(e2db::SAT_INV[0]))); i++)
	{
		string w = e2db::SAT_INV[i];
		dtf1si->addItem(QString::fromStdString(w), i);
	}

	QComboBox* dtf1sr = new QComboBox;
	dtf1sr->setProperty("field", "s_rol");
	fields.emplace_back(dtf1sr);
	dtf1sr->setMaximumWidth(100);
	dtf1sr->setValidator(new QIntValidator);
	platform::osComboBox(dtf1sr);
	dtf1->addRow(tr("Roll Offset"), dtf1sr);
	dtf1->addItem(new QSpacerItem(0, 0));
	dtf1sr->addItem(tr("empty", "ui"), -1);
	for (int i = 0; i < (int (sizeof(e2db::SAT_ROL) / sizeof(e2db::SAT_ROL[0]))); i++)
	{
		string w = e2db::SAT_ROL[i];
		dtf1sr->addItem(QString::fromStdString(w), i);
	}

	QComboBox* dtf1sp = new QComboBox;
	dtf1sp->setProperty("field", "s_pil");
	fields.emplace_back(dtf1sp);
	dtf1sp->setMaximumWidth(100);
	dtf1sp->setValidator(new QIntValidator);
	platform::osComboBox(dtf1sp);
	dtf1->addRow(tr("Pilot"), dtf1sp);
	dtf1->addItem(new QSpacerItem(0, 0));
	dtf1sp->addItem(tr("empty", "ui"), -1);
	for (int i = 0; i < (int (sizeof(e2db::SAT_PIL) / sizeof(e2db::SAT_PIL[0]))); i++)
	{
		string w = e2db::SAT_PIL[i];
		dtf1sp->addItem(QString::fromStdString(w), i);
	}

	dtl1->setLayout(dtf1);
	dtform->addWidget(dtl1, 0, 2);
}

void editTransponder::sideTerrestrialLayout()
{
	debug("sideTerrestrialLayout");

	QGroupBox* dtl1 = new QGroupBox(" ");
	QFormLayout* dtf1 = new QFormLayout;
	dtf1->setRowWrapPolicy(QFormLayout::WrapAllRows);

	QComboBox* dtf1tx = new QComboBox;
	dtf1tx->setProperty("field", "t_tmx");
	fields.emplace_back(dtf1tx);
	dtf1tx->setMaximumWidth(100);
	dtf1tx->setValidator(new QIntValidator);
	platform::osComboBox(dtf1tx);
	dtf1->addRow(tr("Transmission Mode"), dtf1tx);
	dtf1->addItem(new QSpacerItem(0, 0));
	dtf1tx->addItem(tr("empty", "ui"), -1);
	for (int i = 0; i < (int (sizeof(e2db::TER_TMXMODE) / sizeof(e2db::TER_TMXMODE[0]))); i++)
	{
		string w = e2db::TER_TMXMODE[i];
		dtf1tx->addItem(QString::fromStdString(w), i);
	}

	QComboBox* dtf1ti = new QComboBox;
	dtf1ti->setProperty("field", "t_inv");
	fields.emplace_back(dtf1ti);
	dtf1ti->setMaximumWidth(100);
	dtf1ti->setValidator(new QIntValidator);
	platform::osComboBox(dtf1ti);
	dtf1->addRow(tr("Inversion"), dtf1ti);
	dtf1->addItem(new QSpacerItem(0, 0));
	dtf1ti->addItem(tr("empty", "ui"), -1);
	for (int i = 0; i < (int (sizeof(e2db::TER_INV) / sizeof(e2db::TER_INV[0]))); i++)
	{
		string w = e2db::TER_INV[i];
		dtf1ti->addItem(QString::fromStdString(w), i);
	}

	QComboBox* dtf1tg = new QComboBox;
	dtf1tg->setProperty("field", "t_guard");
	fields.emplace_back(dtf1tg);
	dtf1tg->setMaximumWidth(100);
	dtf1tg->setValidator(new QIntValidator);
	platform::osComboBox(dtf1tg);
	dtf1->addRow(tr("Guard Interval"), dtf1tg);
	dtf1->addItem(new QSpacerItem(0, 0));
	dtf1tg->addItem(tr("empty", "ui"), -1);
	for (int i = 0; i < (int (sizeof(e2db::TER_GUARD) / sizeof(e2db::TER_GUARD[0]))); i++)
	{
		string w = e2db::TER_GUARD[i];
		dtf1tg->addItem(QString::fromStdString(w), i);
	}

	QComboBox* dtf1th = new QComboBox;
	dtf1th->setProperty("field", "t_hier");
	fields.emplace_back(dtf1th);
	dtf1th->setMaximumWidth(100);
	dtf1th->setValidator(new QIntValidator);
	platform::osComboBox(dtf1th);
	dtf1->addRow(tr("Hierarchy"), dtf1th);
	dtf1->addItem(new QSpacerItem(0, 0));
	dtf1th->addItem(tr("empty", "ui"), -1);
	for (int i = 0; i < (int (sizeof(e2db::TER_HIER) / sizeof(e2db::TER_HIER[0]))); i++)
	{
		string w = e2db::TER_HIER[i];
		dtf1th->addItem(QString::fromStdString(w), i);
	}

	dtl1->setLayout(dtf1);
	dtform->addWidget(dtl1, 0, 2);
}

void editTransponder::sideCableLayout()
{
	debug("sideCableLayout");

	QGroupBox* dtl1 = new QGroupBox(" ");
	QFormLayout* dtf1 = new QFormLayout;
	dtf1->setRowWrapPolicy(QFormLayout::WrapAllRows);

	QComboBox* dtf1ci = new QComboBox;
	dtf1ci->setProperty("field", "c_inv");
	fields.emplace_back(dtf1ci);
	dtf1ci->setMaximumWidth(100);
	dtf1ci->setValidator(new QIntValidator);
	platform::osComboBox(dtf1ci);
	dtf1->addRow(tr("Inversion"), dtf1ci);
	dtf1->addItem(new QSpacerItem(0, 0));
	dtf1ci->addItem(tr("empty", "ui"), -1);
	for (int i = 0; i < (int (sizeof(e2db::CAB_INV) / sizeof(e2db::CAB_INV[0]))); i++)
	{
		string w = e2db::CAB_INV[i];
		dtf1ci->addItem(QString::fromStdString(w), i);
	}

	dtl1->setLayout(dtf1);
	dtform->addWidget(dtl1, 0, 2);
}

void editTransponder::thirdSatLayout()
{
	debug("thirdSatLayout");

	QGroupBox* dtl2 = new QGroupBox;
	QFormLayout* dtf2 = new QFormLayout;
	dtf2->setRowWrapPolicy(QFormLayout::DontWrapRows);

	QLineEdit* dtf2sf = new QLineEdit;
	dtf2sf->setProperty("field", "s_flags");
	fields.emplace_back(dtf2sf);
	dtf2sf->setMaximumWidth(50);
	dtf2sf->setValidator(new QIntValidator(0, 99999999));
	dtf2sf->setMaxLength(8);
	platform::osLineEdit(dtf2sf);
	dtf2->addRow(tr("flags"), dtf2sf);
	dtf2->addItem(new QSpacerItem(0, 0));

	QLineEdit* dtf2ss = new QLineEdit;
	dtf2ss->setProperty("field", "s_isid");
	fields.emplace_back(dtf2ss);
	dtf2ss->setMaximumWidth(50);
	dtf2ss->setValidator(new QIntValidator(0, 99999999));
	dtf2ss->setMaxLength(8);
	platform::osLineEdit(dtf2ss);
	dtf2->addRow(tr("is id"), dtf2ss);
	dtf2->addItem(new QSpacerItem(0, 0));

	QLineEdit* dtf2sc = new QLineEdit;
	dtf2sc->setProperty("field", "s_plscode");
	fields.emplace_back(dtf2sc);
	dtf2sc->setMaximumWidth(50);
	dtf2sc->setValidator(new QIntValidator(0, 99999999));
	dtf2sc->setMaxLength(8);
	platform::osLineEdit(dtf2sc);
	dtf2->addRow(tr("pls code"), dtf2sc);
	dtf2->addItem(new QSpacerItem(0, 0));

	QLineEdit* dtf2sd = new QLineEdit;
	dtf2sd->setProperty("field", "s_plsmode");
	fields.emplace_back(dtf2sd);
	dtf2sd->setMaximumWidth(50);
	dtf2sd->setValidator(new QIntValidator(0, 99999999));
	dtf2sd->setMaxLength(8);
	platform::osLineEdit(dtf2sd);
	dtf2->addRow(tr("pls mode"), dtf2sd);
	dtf2->addItem(new QSpacerItem(0, 0));

	QLineEdit* dtf2tp = new QLineEdit;
	dtf2tp->setProperty("field", "s_t2mi_plpid");
	fields.emplace_back(dtf2tp);
	dtf2tp->setMaximumWidth(50);
	dtf2tp->setValidator(new QIntValidator(0, 99999999));
	dtf2tp->setMaxLength(8);
	platform::osLineEdit(dtf2tp);
	dtf2->addRow(tr("t2mi plpid"), dtf2tp);
	dtf2->addItem(new QSpacerItem(0, 0));

	QLineEdit* dtf2ti = new QLineEdit;
	dtf2ti->setProperty("field", "s_t2mi_pid");
	fields.emplace_back(dtf2ti);
	dtf2ti->setMaximumWidth(50);
	dtf2ti->setValidator(new QIntValidator(0, 99999999));
	dtf2ti->setMaxLength(8);
	platform::osLineEdit(dtf2ti);
	dtf2->addRow(tr("t2mi pid"), dtf2ti);
	dtf2->addItem(new QSpacerItem(0, 0));

	dtl2->setLayout(dtf2);
	dtform->addWidget(dtl2, 0, 3);
}

void editTransponder::thirdTerrestrialLayout()
{
	debug("thirdTerrestrialLayout");

	QGroupBox* dtl2 = new QGroupBox;
	QFormLayout* dtf2 = new QFormLayout;
	dtf2->setRowWrapPolicy(QFormLayout::DontWrapRows);

	QLineEdit* dtf2tf = new QLineEdit;
	dtf2tf->setProperty("field", "t_flags");
	fields.emplace_back(dtf2tf);
	dtf2tf->setMaximumWidth(50);
	dtf2tf->setValidator(new QIntValidator(0, 99999999));
	dtf2tf->setMaxLength(8);
	platform::osLineEdit(dtf2tf);
	dtf2->addRow(tr("flags"), dtf2tf);
	dtf2->addItem(new QSpacerItem(0, 0));

	QLineEdit* dtf2tp = new QLineEdit;
	dtf2tp->setProperty("field", "t_plpid");
	fields.emplace_back(dtf2tp);
	dtf2tp->setMaximumWidth(50);
	dtf2tp->setValidator(new QIntValidator(0, 99999999));
	dtf2tp->setMaxLength(8);
	platform::osLineEdit(dtf2tp);
	dtf2->addRow(tr("plp id"), dtf2tp);
	dtf2->addItem(new QSpacerItem(0, 0));

	dtl2->setLayout(dtf2);
	dtform->addWidget(dtl2, 0, 3);
}

void editTransponder::thirdCableLayout()
{
	debug("thirdCableLayout");

	QGroupBox* dtl2 = new QGroupBox;
	QFormLayout* dtf2 = new QFormLayout;
	dtf2->setRowWrapPolicy(QFormLayout::DontWrapRows);

	QLineEdit* dtf2cf = new QLineEdit;
	dtf2cf->setProperty("field", "c_flags");
	fields.emplace_back(dtf2cf);
	dtf2cf->setMaximumWidth(50);
	dtf2cf->setValidator(new QIntValidator(0, 99999999));
	dtf2cf->setMaxLength(8);
	platform::osLineEdit(dtf2cf);
	dtf2->addRow(tr("flags"), dtf2cf);
	dtf2->addItem(new QSpacerItem(0, 0));

	QLineEdit* dtf2cs = new QLineEdit;
	dtf2cs->setProperty("field", "c_sys");
	fields.emplace_back(dtf2cs);
	dtf2cs->setMaximumWidth(50);
	dtf2cs->setValidator(new QIntValidator(0, 9));
	dtf2cs->setMaxLength(1);
	platform::osLineEdit(dtf2cs);
	dtf2->addRow(tr("system"), dtf2cs);
	dtf2->addItem(new QSpacerItem(0, 0));

	dtl2->setLayout(dtf2);
	dtform->addWidget(dtl2, 0, 3);
}

void editTransponder::thirdAtscLayout()
{
	debug("thirdAtscLayout");

	QGroupBox* dtl2 = new QGroupBox;
	QFormLayout* dtf2 = new QFormLayout;
	dtf2->setRowWrapPolicy(QFormLayout::DontWrapRows);

	QLineEdit* dtf2af = new QLineEdit;
	dtf2af->setProperty("field", "a_flags");
	fields.emplace_back(dtf2af);
	dtf2af->setMaximumWidth(50);
	dtf2af->setValidator(new QIntValidator(0, 99999999));
	dtf2af->setMaxLength(8);
	platform::osLineEdit(dtf2af);
	dtf2->addRow(tr("flags"), dtf2af);
	dtf2->addItem(new QSpacerItem(0, 0));

	dtl2->setLayout(dtf2);
	dtform->addWidget(dtl2, 0, 2);
}

void editTransponder::typeComboChanged(int index)
{
	debug("typeComboChanged", "index", index);

	if (this->state.yx == index)
		return;

	this->state.yx = index;

	layoutChange(this->state.yx);

	if (this->state.edit)
		retrieve(txid);
}

void editTransponder::computeDvbns()
{
	debug("computeDvbns");

	auto* dbih = this->data->dbih;

	int yx, dvbns, tsid, onid, pos, freq;
	yx = 0, dvbns = 0, tsid = 0, onid = 0, pos = 0, freq = 0;

	for (auto & item : fields)
	{
		string key = item->property("field").toString().toStdString();
		int val = -1;

		if (QLineEdit* field = qobject_cast<QLineEdit*>(item))
		{
			if (key == "pos")
				val = dbih->value_transponder_position(field->text().toStdString());
			else if (key == "dvbns")
				val = dbih->value_transponder_dvbns(field->text().toStdString());
			else
				val = field->text().isEmpty() ? -1 : field->text().toInt();
		}
		else if (QComboBox* field = qobject_cast<QComboBox*>(item))
		{
			val = field->currentData().toInt();
		}

		if (key == "ytype")
			yx = val;
		else if (key == "tsid")
			tsid = val;
		else if (key == "dvbns")
			dvbns = val;
		else if (key == "onid")
			onid = val;

		if (this->state.yx == e2db::YTYPE::satellite)
		{
			if (key == "pos")
				pos = val;
			else if (key == "s_freq")
				freq = val;
		}
		else if (this->state.yx == e2db::YTYPE::terrestrial)
		{
			if (key == "pos")
				pos = 0;
			else if (key == "t_freq")
				freq = val;
		}
		else if (this->state.yx == e2db::YTYPE::cable)
		{
			if (key == "pos")
				pos = 0;
			else if (key == "c_freq")
				freq = val;
		}
		else if (this->state.yx == e2db::YTYPE::atsc)
		{
			if (key == "pos")
				pos = val;
			else if (key == "a_freq")
				freq = val;
		}
	}

	e2db::YTYPE ytype = static_cast<e2db::YTYPE>(yx);
	int nw_dvbns = dbih->value_transponder_dvbns(ytype, tsid, onid, pos, freq);

	debug("computeDvbns", "current dvbns", dbih->value_transponder_dvbns(dvbns));
	debug("computeDvbns", "calc dvbns", dbih->value_transponder_dvbns(nw_dvbns));

	if (dvbns == nw_dvbns)
		return;

	for (auto & item : fields)
	{
		string key = item->property("field").toString().toStdString();
		int val = -1;

		if (key == "dvbns")
		{
			val = nw_dvbns;

			if (QLineEdit* field = qobject_cast<QLineEdit*>(item))
			{
				field->setText(QString::fromStdString(dbih->value_transponder_dvbns(val)));
			}

			break;
		}
	}
}

void editTransponder::store()
{
	debug("store");

	auto* dbih = this->data->dbih;

	e2db::transponder tx;

	if (this->state.edit)
	{
		if (dbih->db.transponders.count(txid))
			tx = dbih->db.transponders[txid];
		else
			return error("store", tr("Error", "error").toStdString(), tr("Transponder \"%1\" not exists.", "error").arg(txid.data()).toStdString());
	}

	for (auto & item : fields)
	{
		string key = item->property("field").toString().toStdString();
		int val = -1;

		if (QLineEdit* field = qobject_cast<QLineEdit*>(item))
		{
			if (key == "pos")
				val = dbih->value_transponder_position(field->text().toStdString());
			else if (key == "dvbns")
				val = dbih->value_transponder_dvbns(field->text().toStdString());
			else
				val = field->text().isEmpty() ? -1 : field->text().toInt();
		}
		else if (QComboBox* field = qobject_cast<QComboBox*>(item))
		{
			val = field->currentData().toInt();
		}

		if (key == "ytype")
			tx.ytype = val;
		else if (key == "tsid")
			tx.tsid = val;
		else if (key == "dvbns")
			tx.dvbns = val;
		else if (key == "onid")
			tx.onid = val;

		if (this->state.yx == e2db::YTYPE::satellite)
		{
			if (key == "pos")
				tx.pos = val;
			else if (key == "s_freq")
				tx.freq = val;
			else if (key == "s_sr")
				tx.sr = val;
			else if (key == "s_pol")
				tx.pol = val;
			else if (key == "s_fec")
				tx.fec = val;
			else if (key == "s_inv")
				tx.inv = val;
			else if (key == "s_sys")
				tx.sys = val;
			else if (key == "s_mod")
				tx.mod = val;
			else if (key == "s_rol")
				tx.rol = val;
			else if (key == "s_pil")
				tx.pil = val;
			else if (key == "s_flags")
				tx.flags = val;
			else if (key == "s_isid")
				tx.isid = val;
			else if (key == "s_plscode")
				tx.plscode = val;
			else if (key == "s_plsmode")
				tx.plsmode = val;
			else if (key == "s_t2mi_plpid")
				tx.t2mi_plpid = val;
			else if (key == "s_t2mi_pid")
				tx.t2mi_pid = val;

			tx.mispls = tx.mispls ?: tx.isid != -1 || tx.plscode != -1 || tx.plsmode != -1;
			tx.t2mi = tx.t2mi ?: tx.t2mi_plpid != -1 || tx.t2mi_pid != -1;
		}
		else if (this->state.yx == e2db::YTYPE::terrestrial)
		{
			if (key == "pos")
				tx.pos = -1;
			else if (key == "t_freq")
				tx.freq = val;
			else if (key == "t_tmod")
				tx.tmod = val;
			else if (key == "t_band")
				tx.band = val;
			else if (key == "t_tmod")
				tx.tmod = val;
			else if (key == "t_tmx")
				tx.tmx = val;
			else if (key == "t_hpfec")
				tx.hpfec = val;
			else if (key == "t_lpfec")
				tx.lpfec = val;
			else if (key == "t_inv")
				tx.inv = val;
			else if (key == "t_guard")
				tx.guard = val;
			else if (key == "t_hier")
				tx.hier = val;
			else if (key == "t_flags")
				tx.flags = val;
			else if (key == "t_sys")
				tx.sys = val;
			else if (key == "t_plpid")
				tx.plpid = val;
		}
		else if (this->state.yx == e2db::YTYPE::cable)
		{
			if (key == "pos")
				tx.pos = -1;
			else if (key == "c_freq")
				tx.freq = val;
			else if (key == "c_sr")
				tx.sr = val;
			else if (key == "c_cfec")
				tx.cfec = val;
			else if (key == "c_inv")
				tx.inv = val;
			else if (key == "c_cmod")
				tx.cmod = val;
			else if (key == "c_flags")
				tx.flags = val;
			else if (key == "c_sys")
				tx.sys = val;
		}
		else if (this->state.yx == e2db::YTYPE::atsc)
		{
			if (key == "pos")
				tx.pos = val;
			else if (key == "a_freq")
				tx.freq = val;
			else if (key == "a_amod")
				tx.amod = val;
			else if (key == "a_flags")
				tx.flags = val;
			else if (key == "a_sys")
				tx.sys = val;
		}
	}

	if (this->state.edit)
		this->txid = dbih->editTransponder(txid, tx);
	else
		this->txid = dbih->addTransponder(tx);

	this->changes = true;
}

void editTransponder::retrieve()
{
	debug("retrieve");

	auto* dbih = this->data->dbih;

	if (! dbih->db.transponders.count(txid))
		return error("retrieve", tr("Error", "error").toStdString(), tr("Transponder \"%1\" not exists.", "error").arg(txid.data()).toStdString());

	e2db::transponder tx = dbih->db.transponders[txid];

	if (tx.ytype != dtf0yx->currentIndex())
		dtf0yx->setCurrentIndex(tx.ytype);
	else
		typeComboChanged(tx.ytype);

	retrieve(txid);
}

void editTransponder::retrieve(string txid)
{
	debug("retrieve", "txid", txid);

	auto* dbih = this->data->dbih;

	if (! dbih->db.transponders.count(txid))
		return error("retrieve", tr("Error", "error").toStdString(), tr("Transponder \"%1\" not exists.", "error").arg(txid.data()).toStdString());

	e2db::transponder tx = dbih->db.transponders[txid];

	for (auto & item : fields)
	{
		string key = item->property("field").toString().toStdString();
		int val = -1;

		if (key == "ytype")
			continue;
		else if (key == "tsid")
			val = tx.tsid;
		else if (key == "dvbns")
			val = tx.dvbns;
		else if (key == "onid")
			val = tx.onid;
		else if (key == "pos")
			val = tx.pos;

		if (this->state.yx == e2db::YTYPE::satellite)
		{
			if (key == "s_freq")
				val = tx.freq;
			else if (key == "s_sr")
				val = tx.sr;
			else if (key == "s_pol")
				val = tx.pol;
			else if (key == "s_fec")
				val = tx.fec;
			else if (key == "s_inv")
				val = tx.inv;
			else if (key == "s_sys")
				val = tx.sys;
			else if (key == "s_mod")
				val = tx.mod;
			else if (key == "s_rol")
				val = tx.rol;
			else if (key == "s_pil")
				val = tx.pil;
			else if (key == "s_flags")
				val = tx.flags;
			else if (key == "s_isid")
				val = tx.isid;
			else if (key == "s_plscode")
				val = tx.plscode;
			else if (key == "s_plsmode")
				val = tx.plsmode;
			else if (key == "s_t2mi_plpid")
				val = tx.t2mi_plpid;
			else if (key == "s_t2mi_pid")
				val = tx.t2mi_pid;
		}
		else if (this->state.yx == e2db::YTYPE::terrestrial)
		{
			if (key == "t_freq")
				val = tx.freq;
			else if (key == "t_tmod")
				val = tx.tmod;
			else if (key == "t_band")
				val = tx.band;
			else if (key == "t_tmod")
				val = tx.tmod;
			else if (key == "t_tmx")
				val = tx.tmx;
			else if (key == "t_hpfec")
				val = tx.hpfec;
			else if (key == "t_lpfec")
				val = tx.lpfec;
			else if (key == "t_inv")
				val = tx.inv;
			else if (key == "t_guard")
				val = tx.guard;
			else if (key == "t_hier")
				val = tx.hier;
			else if (key == "t_flags")
				val = tx.flags;
			else if (key == "t_sys")
				val = tx.sys;
			else if (key == "t_plpid")
				val = tx.plpid;
		}
		else if (this->state.yx == e2db::YTYPE::cable)
		{
			if (key == "c_freq")
				val = tx.freq;
			else if (key == "c_sr")
				val = tx.sr;
			else if (key == "c_cfec")
				val = tx.cfec;
			else if (key == "c_inv")
				val = tx.inv;
			else if (key == "c_cmod")
				val = tx.cmod;
			else if (key == "c_flags")
				val = tx.flags;
			else if (key == "c_sys")
				val = tx.sys;
		}
		else if (this->state.yx == e2db::YTYPE::atsc)
		{
			if (key == "a_freq")
				val = tx.freq;
			else if (key == "a_amod")
				val = tx.amod;
			else if (key == "a_flags")
				val = tx.flags;
			else if (key == "a_sys")
				val = tx.sys;
		}

		if (QLineEdit* field = qobject_cast<QLineEdit*>(item))
		{
			if (key == "pos")
				field->setText(QString::fromStdString(dbih->value_transponder_position(val)));
			else if (key == "dvbns")
				field->setText(QString::fromStdString(dbih->value_transponder_dvbns(val)));
			else
				field->setText(val != -1 ? QString::number(val) : "");
		}
		else if (QComboBox* field = qobject_cast<QComboBox*>(item))
		{
			int index = field->findData(val, Qt::UserRole);
			field->setCurrentIndex(index);
		}
	}
}

void editTransponder::setEditId(string txid)
{
	debug("setEditId");

	this->state.edit = true;
	this->txid = txid;
}

string editTransponder::getEditId()
{
	debug("getEditId");

	return this->txid;
}

void editTransponder::setAddId()
{
	debug("setAddId");

	this->state.edit = false;
}

}
