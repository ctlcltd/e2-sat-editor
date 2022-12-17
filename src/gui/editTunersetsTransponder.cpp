/*!
 * e2-sat-editor/src/gui/editTunersetsTransponder.cpp
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
#include <QComboBox>

#include "editTunersetsTransponder.h"
#include "todo.h"

using std::to_string;
using namespace e2se;

namespace e2se_gui
{

editTunersetsTransponder::editTunersetsTransponder(e2db* dbih, int ty, e2se::logger::session* log)
{
	this->log = new logger(log, "editTunersetsTransponder");
	debug("editTunersetsTransponder()");

	this->state.ty = ty;
	this->dbih = dbih;
}

void editTunersetsTransponder::display(QWidget* cwid)
{
	layout();

	if (this->state.edit)
		retrieve();

	this->dialAbstract::display(cwid);
}

void editTunersetsTransponder::layout()
{
	this->dialAbstract::layout();

	QString dtitle = this->state.edit ? "Edit Transponder" : "Add Transponder";
	dial->setWindowTitle(dtitle);

	switch (this->state.ty)
	{
		case e2db::YTYPE::sat:
			leadSatLayout();
			sideSatLayout();
		break;
		case e2db::YTYPE::terrestrial:
			leadTerrestrialLayout();
			sideTerrestrialLayout();
		break;
		case e2db::YTYPE::cable:
			leadCableLayout();
			sideTerrestrialLayout();
		break;
		case e2db::YTYPE::atsc:
			leadAtscLayout();
			sideAtscLayout();
		break;
	}
}

void editTunersetsTransponder::leadSatLayout()
{
	debug("leadSatLayout()");

	QGroupBox* dtl0 = new QGroupBox;
	QFormLayout* dtf0 = new QFormLayout;
	dtf0->setRowWrapPolicy(QFormLayout::WrapAllRows);

	QLineEdit* dtf0sf = new QLineEdit;
	dtf0sf->setProperty("field", "s_freq");
	fields.emplace_back(dtf0sf);
	dtf0sf->setMinimumWidth(100);
	dtf0sf->setValidator(new QIntValidator);
	dtf0->addRow(tr("Frequency"), dtf0sf);
	dtf0->addItem(new QSpacerItem(0, 0));

	QComboBox* dtf0sp = new QComboBox;
	dtf0sp->setProperty("field", "s_pol");
	fields.emplace_back(dtf0sp);
	dtf0sp->setMaximumWidth(100);
	dtf0sp->setValidator(new QIntValidator);
	dtf0->addRow(tr("Polarization"), dtf0sp);
	dtf0->addItem(new QSpacerItem(0, 0));
	for (int i = 0; i < 4; i++)
	{
		string w = e2db::SAT_POL[i];
		dtf0sp->addItem(QString::fromStdString(w), i);
	}

	QLineEdit* dtf0ss = new QLineEdit;
	dtf0ss->setProperty("field", "s_sr");
	fields.emplace_back(dtf0ss);
	dtf0ss->setMinimumWidth(100);
	dtf0ss->setValidator(new QIntValidator);
	dtf0->addRow(tr("Symbol Rate"), dtf0ss);
	dtf0->addItem(new QSpacerItem(0, 0));
	
	QComboBox* dtf0sc = new QComboBox;
	dtf0sc->setProperty("field", "s_fec");
	fields.emplace_back(dtf0sp);
	dtf0sc->setMaximumWidth(100);
	dtf0sc->setValidator(new QIntValidator);
	dtf0->addRow(tr("FEC"), dtf0sc);
	dtf0->addItem(new QSpacerItem(0, 0));
	for (int i = 0; i < 11; i++)
	{
		string w = e2db::SAT_FEC[i];
		dtf0sc->addItem(QString::fromStdString(w), i);
	}

	QComboBox* dtf0sy = new QComboBox;
	dtf0sy->setProperty("field", "s_sys");
	fields.emplace_back(dtf0sp);
	dtf0sy->setMaximumWidth(100);
	dtf0sy->setValidator(new QIntValidator);
	dtf0->addRow(tr("System"), dtf0sy);
	dtf0->addItem(new QSpacerItem(0, 0));
	for (int i = 0; i < 2; i++)
	{
		string w = e2db::SAT_SYS[i];
		dtf0sy->addItem(QString::fromStdString(w), i);
	}

	dtl0->setLayout(dtf0);
	dtform->addWidget(dtl0, 0, 0);
}

void editTunersetsTransponder::leadTerrestrialLayout()
{
	debug("leadTerrestrialLayout()");

	QGroupBox* dtl0 = new QGroupBox;
	QFormLayout* dtf0 = new QFormLayout;
	dtf0->setRowWrapPolicy(QFormLayout::WrapAllRows);

	QLineEdit* dtf0tf = new QLineEdit;
	dtf0tf->setProperty("field", "t_freq");
	fields.emplace_back(dtf0tf);
	dtf0tf->setMinimumWidth(100);
	dtf0tf->setValidator(new QIntValidator);
	dtf0->addRow(tr("Frequency"), dtf0tf);
	dtf0->addItem(new QSpacerItem(0, 0));

	QComboBox* dtf0tm = new QComboBox;
	dtf0tm->setProperty("field", "t_tmod");
	fields.emplace_back(dtf0tm);
	dtf0tm->setMaximumWidth(100);
	dtf0tm->setValidator(new QIntValidator);
	dtf0->addRow(tr("Constellation"), dtf0tm);
	dtf0->addItem(new QSpacerItem(0, 0));
	for (int i = 0; i < 4; i++)
	{
		string w = e2db::TER_MOD[i];
		dtf0tm->addItem(QString::fromStdString(w), i);
	}

	QComboBox* dtf0tb = new QComboBox;
	dtf0tb->setProperty("field", "t_band");
	fields.emplace_back(dtf0tb);
	dtf0tb->setMaximumWidth(100);
	dtf0tb->setValidator(new QIntValidator);
	dtf0->addRow(tr("Bandwidth"), dtf0tb);
	dtf0->addItem(new QSpacerItem(0, 0));
	for (int i = 0; i < 4; i++)
	{
		string w = e2db::TER_BAND[i];
		dtf0tb->addItem(QString::fromStdString(w), i);
	}
	
	QComboBox* dtf0th = new QComboBox;
	dtf0th->setProperty("field", "t_hpfec");
	fields.emplace_back(dtf0th);
	dtf0th->setMaximumWidth(100);
	dtf0th->setValidator(new QIntValidator);
	dtf0->addRow(tr("HP FEC"), dtf0th);
	dtf0->addItem(new QSpacerItem(0, 0));
	for (int i = 0; i < 7; i++)
	{
		string w = e2db::TER_HPFEC[i];
		dtf0th->addItem(QString::fromStdString(w), i);
	}

	QComboBox* dtf0tl = new QComboBox;
	dtf0tl->setProperty("field", "t_lpfec");
	fields.emplace_back(dtf0tl);
	dtf0tl->setMaximumWidth(100);
	dtf0tl->setValidator(new QIntValidator);
	dtf0->addRow(tr("LP FEC"), dtf0tl);
	dtf0->addItem(new QSpacerItem(0, 0));
	for (int i = 0; i < 7; i++)
	{
		string w = e2db::TER_LPFEC[i];
		dtf0tl->addItem(QString::fromStdString(w), i);
	}

	dtl0->setLayout(dtf0);
	dtform->addWidget(dtl0, 0, 0);
}

void editTunersetsTransponder::leadCableLayout()
{
	debug("leadCableLayout()");

	QGroupBox* dtl0 = new QGroupBox;
	QFormLayout* dtf0 = new QFormLayout;
	dtf0->setRowWrapPolicy(QFormLayout::WrapAllRows);

	QLineEdit* dtf0cf = new QLineEdit;
	dtf0cf->setProperty("field", "c_freq");
	fields.emplace_back(dtf0cf);
	dtf0cf->setMinimumWidth(100);
	dtf0cf->setValidator(new QIntValidator);
	dtf0->addRow(tr("Frequency"), dtf0cf);
	dtf0->addItem(new QSpacerItem(0, 0));

	QComboBox* dtf0cm = new QComboBox;
	dtf0cm->setProperty("field", "c_cmod");
	fields.emplace_back(dtf0cm);
	dtf0cm->setMaximumWidth(100);
	dtf0cm->setValidator(new QIntValidator);
	dtf0->addRow(tr("Modulation"), dtf0cm);
	dtf0->addItem(new QSpacerItem(0, 0));
	for (int i = 0; i < 6; i++)
	{
		string w = e2db::CAB_MOD[i];
		dtf0cm->addItem(QString::fromStdString(w), i);
	}

	QLineEdit* dtf0cs = new QLineEdit;
	dtf0cs->setProperty("field", "c_sr");
	fields.emplace_back(dtf0cs);
	dtf0cs->setMinimumWidth(100);
	dtf0cs->setValidator(new QIntValidator);
	dtf0->addRow(tr("Symbol Rate"), dtf0cs);
	dtf0->addItem(new QSpacerItem(0, 0));
	
	QComboBox* dtf0ci = new QComboBox;
	dtf0ci->setProperty("field", "s_cfec");
	fields.emplace_back(dtf0ci);
	dtf0ci->setMaximumWidth(100);
	dtf0ci->setValidator(new QIntValidator);
	dtf0->addRow(tr("Inner FEC"), dtf0ci);
	dtf0->addItem(new QSpacerItem(0, 0));
	for (int i = 0; i < 10; i++)
	{
		string w = e2db::CAB_IFEC[i];
		dtf0ci->addItem(QString::fromStdString(w), i);
	}

	dtl0->setLayout(dtf0);
	dtform->addWidget(dtl0, 0, 0);
}

void editTunersetsTransponder::leadAtscLayout()
{
	debug("leadAtscLayout()");

	QGroupBox* dtl0 = new QGroupBox;
	QFormLayout* dtf0 = new QFormLayout;
	dtf0->setRowWrapPolicy(QFormLayout::WrapAllRows);

	QLineEdit* dtf0cf = new QLineEdit;
	dtf0cf->setProperty("field", "a_freq");
	fields.emplace_back(dtf0cf);
	dtf0cf->setMinimumWidth(100);
	dtf0cf->setValidator(new QIntValidator);
	dtf0->addRow(tr("Frequency"), dtf0cf);
	dtf0->addItem(new QSpacerItem(0, 0));

	QLineEdit* dtf0am = new QLineEdit;
	dtf0am->setProperty("field", "a_amod");
	fields.emplace_back(dtf0am);
	dtf0am->setMaximumWidth(100);
	dtf0am->setValidator(new QIntValidator);
	dtf0->addRow(tr("Modulation"), dtf0am);
	dtf0->addItem(new QSpacerItem(0, 0));

	dtl0->setLayout(dtf0);
	dtform->addWidget(dtl0, 0, 0);
}

void editTunersetsTransponder::sideSatLayout()
{
	debug("sideSatLayout()");

	QGroupBox* dtl1 = new QGroupBox;
	QFormLayout* dtf1 = new QFormLayout;
	dtf1->setRowWrapPolicy(QFormLayout::WrapAllRows);

	QComboBox* dtf1sm = new QComboBox;
	dtf1sm->setProperty("field", "s_mod");
	fields.emplace_back(dtf1sm);
	dtf1sm->setMaximumWidth(100);
	dtf1sm->setValidator(new QIntValidator);
	dtf1->addRow(tr("Modulation"), dtf1sm);
	dtf1->addItem(new QSpacerItem(0, 0));
	for (int i = 0; i < 4; i++)
	{
		string w = e2db::SAT_POL[i];
		dtf1sm->addItem(QString::fromStdString(w), i);
	}
	
	QComboBox* dtf1si = new QComboBox;
	dtf1si->setProperty("field", "s_inv");
	fields.emplace_back(dtf1si);
	dtf1si->setMaximumWidth(100);
	dtf1si->setValidator(new QIntValidator);
	dtf1->addRow(tr("Inversion"), dtf1si);
	dtf1->addItem(new QSpacerItem(0, 0));
	for (int i = 0; i < 3; i++)
	{
		string w = e2db::SAT_INV[i];
		dtf1si->addItem(QString::fromStdString(w), i);
	}

	QComboBox* dtf1sr = new QComboBox;
	dtf1sr->setProperty("field", "s_rol");
	fields.emplace_back(dtf1sr);
	dtf1sr->setMaximumWidth(100);
	dtf1sr->setValidator(new QIntValidator);
	dtf1->addRow(tr("Roll Offset"), dtf1sr);
	dtf1->addItem(new QSpacerItem(0, 0));
	for (int i = 0; i < 4; i++)
	{
		string w = e2db::SAT_ROL[i];
		dtf1sr->addItem(QString::fromStdString(w), i);
	}

	QComboBox* dtf1sp = new QComboBox;
	dtf1sp->setProperty("field", "s_pil");
	fields.emplace_back(dtf1sp);
	dtf1sp->setMaximumWidth(100);
	dtf1sp->setValidator(new QIntValidator);
	dtf1->addRow(tr("Pilot"), dtf1sp);
	dtf1->addItem(new QSpacerItem(0, 0));
	for (int i = 0; i < 3; i++)
	{
		string w = e2db::SAT_PIL[i];
		dtf1sp->addItem(QString::fromStdString(w), i);
	}

	dtf1->addItem(new QSpacerItem(0, 0));

	QLineEdit* dtf1ss = new QLineEdit;
	dtf1ss->setProperty("field", "s_isid");
	fields.emplace_back(dtf1ss);
	dtf1ss->setMinimumWidth(100);
	dtf1ss->setValidator(new QIntValidator);
	dtf1->addRow(tr("isid"), dtf1ss);
	dtf1->addItem(new QSpacerItem(0, 0));

	QLineEdit* dtf1st = new QLineEdit;
	dtf1st->setProperty("field", "s_mts");
	fields.emplace_back(dtf1st);
	dtf1st->setMinimumWidth(100);
	dtf1st->setValidator(new QIntValidator);
	dtf1->addRow(tr("mts"), dtf1st);
	dtf1->addItem(new QSpacerItem(0, 0));

	QLineEdit* dtf1sd = new QLineEdit;
	dtf1sd->setProperty("field", "s_plsmode");
	fields.emplace_back(dtf1sd);
	dtf1sd->setMinimumWidth(100);
	dtf1sd->setValidator(new QIntValidator);
	dtf1->addRow(tr("plsmode"), dtf1sd);
	dtf1->addItem(new QSpacerItem(0, 0));
	
	QLineEdit* dtf1sc = new QLineEdit;
	dtf1sc->setProperty("field", "s_plscode");
	fields.emplace_back(dtf1sc);
	dtf1sc->setMinimumWidth(100);
	dtf1sc->setValidator(new QIntValidator);
	dtf1->addRow(tr("plscode"), dtf1sc);
	dtf1->addItem(new QSpacerItem(0, 0));

	QLineEdit* dtf1sn = new QLineEdit;
	dtf1sn->setProperty("field", "s_plsn");
	fields.emplace_back(dtf1sn);
	dtf1sn->setMinimumWidth(100);
	dtf1sn->setValidator(new QIntValidator);
	dtf1->addRow(tr("plsn"), dtf1sn);
	dtf1->addItem(new QSpacerItem(0, 0));

	dtl1->setLayout(dtf1);
	dtform->addWidget(dtl1, 0, 1);
}

void editTunersetsTransponder::sideTerrestrialLayout()
{
	debug("sideTerrestrialLayout()");

	QGroupBox* dtl1 = new QGroupBox;
	QFormLayout* dtf1 = new QFormLayout;
	dtf1->setRowWrapPolicy(QFormLayout::WrapAllRows);

	QComboBox* dtf1ti = new QComboBox;
	dtf1ti->setProperty("field", "t_inv");
	fields.emplace_back(dtf1ti);
	dtf1ti->setMaximumWidth(100);
	dtf1ti->setValidator(new QIntValidator);
	dtf1->addRow(tr("Inversion"), dtf1ti);
	dtf1->addItem(new QSpacerItem(0, 0));
	for (int i = 0; i < 3; i++)
	{
		string w = e2db::TER_INV[i];
		dtf1ti->addItem(QString::fromStdString(w), i);
	}
	
	QComboBox* dtf1tg = new QComboBox;
	dtf1tg->setProperty("field", "t_guard");
	fields.emplace_back(dtf1tg);
	dtf1tg->setMaximumWidth(100);
	dtf1tg->setValidator(new QIntValidator);
	dtf1->addRow(tr("Guard Interval"), dtf1tg);
	dtf1->addItem(new QSpacerItem(0, 0));
	for (int i = 0; i < 5; i++)
	{
		string w = e2db::TER_GUARD[i];
		dtf1tg->addItem(QString::fromStdString(w), i);
	}

	QComboBox* dtf1th = new QComboBox;
	dtf1th->setProperty("field", "t_hier");
	fields.emplace_back(dtf1th);
	dtf1th->setMaximumWidth(100);
	dtf1th->setValidator(new QIntValidator);
	dtf1->addRow(tr("Hierarchy"), dtf1th);
	dtf1->addItem(new QSpacerItem(0, 0));
	for (int i = 0; i < 5; i++)
	{
		string w = e2db::TER_HIER[i];
		dtf1th->addItem(QString::fromStdString(w), i);
	}

	dtl1->setLayout(dtf1);
	dtform->addWidget(dtl1, 0, 1);
}

void editTunersetsTransponder::sideCableLayout()
{
	debug("sideCableLayout()");

	QGroupBox* dtl1 = new QGroupBox;
	QFormLayout* dtf1 = new QFormLayout;
	dtf1->setRowWrapPolicy(QFormLayout::WrapAllRows);

	QComboBox* dtf1ci = new QComboBox;
	dtf1ci->setProperty("field", "c_cmod");
	fields.emplace_back(dtf1ci);
	dtf1ci->setMaximumWidth(100);
	dtf1ci->setValidator(new QIntValidator);
	dtf1->addRow(tr("Inversion"), dtf1ci);
	dtf1->addItem(new QSpacerItem(0, 0));
	for (int i = 0; i < 3; i++)
	{
		string w = e2db::CAB_INV[i];
		dtf1ci->addItem(QString::fromStdString(w), i);
	}

	dtl1->setLayout(dtf1);
	dtform->addWidget(dtl1, 0, 1);
}

void editTunersetsTransponder::sideAtscLayout()
{
	debug("sideAtscLayout()");
}

//TODO TEST
//TODO e2db::tunersets_transponder value
void editTunersetsTransponder::store()
{
	debug("store()");

	e2db::tunersets tvs;
	e2db::tunersets_table tns;

	if (dbih->tuners.count(tvid))
		tvs = dbih->tuners[tvid];
	else
		return error("store()", "tvid", tvid);

	if (tvs.tables.count(tnid))
		tns = tvs.tables[tnid];
	else
		return error("store()", "tnid", tnid);

	e2db::tunersets_transponder txp;

	if (this->state.edit)
	{
		if (! tns.transponders.count(trid))
			return error("store()", "trid", trid);

		txp = tns.transponders[trid];
	}

	for (auto & item : fields)
	{
		string key = item->property("field").toString().toStdString();
		int val;

		if (QLineEdit* field = qobject_cast<QLineEdit*>(item))
			val = field->text().toInt();
		else if (QComboBox* field = qobject_cast<QComboBox*>(item))
			val = field->currentData().toInt();

		if (this->state.ty == e2db::YTYPE::sat)
		{
			if (key == "s_freq")
				txp.freq = val;
			else if (key == "s_sr")
				txp.sr = val;
			else if (key == "s_pol")
				txp.pol = val;
			else if (key == "s_fec")
				txp.fec = val;
			else if (key == "s_inv")
				txp.inv = val;
			else if (key == "s_sys")
				txp.sys = val;
			else if (key == "s_mod")
				txp.mod = val;
			else if (key == "s_rol")
				txp.rol = val;
			else if (key == "s_pil")
				txp.pil = val;
			else if (key == "s_isid")
				txp.isid = val;
			else if (key == "s_mts")
				txp.mts = val;
			else if (key == "s_plsmode")
				txp.plsmode = val;
			else if (key == "s_plscode")
				txp.plscode = val;
			else if (key == "s_plsn")
				txp.plsn = val;
		}
		else if (this->state.ty == e2db::YTYPE::terrestrial)
		{
			if (key == "t_freq")
				txp.freq = val;
			else if (key == "t_tmod")
				txp.tmod = val;
			else if (key == "t_band")
				txp.band = val;
			else if (key == "t_sys")
				txp.sys = val;
			else if (key == "t_tmx")
				txp.tmx = val;
			else if (key == "t_hpfec")
				txp.hpfec = val;
			else if (key == "t_lpfec")
				txp.lpfec = val;
			else if (key == "t_inv")
				txp.inv = val;
			else if (key == "t_guard")
				txp.guard = val;
			else if (key == "t_hier")
				txp.hier = val;
		}
		else if (this->state.ty == e2db::YTYPE::cable)
		{
			if (key == "c_freq")
				txp.freq = val;
			else if (key == "c_sr")
				txp.sr = val;
			else if (key == "c_cfec")
				txp.cfec = val;
			else if (key == "c_inv")
				txp.inv = val;
			else if (key == "c_sys")
				txp.sys = val;
			else if (key == "c_cmod")
				txp.cmod = val;
		}
		else if (this->state.ty == e2db::YTYPE::atsc)
		{
			if (key == "a_freq")
				txp.freq = val;
			else if (key == "a_amod")
				txp.amod = val;
			else if (key == "a_sys")
				txp.sys = val;
		}
	}

	//TODO
	todo();

	/*if (this->state.edit)
		this->trid = dbih->add_tunersets_transponder(-1, tns, txp);
	else
		this->trid = dbih->add_tunersets_transponder(-1, tns, txp);*/
}

void editTunersetsTransponder::retrieve()
{
	debug("retrieve()");

	e2db::tunersets tvs;
	e2db::tunersets_table tns;

	if (dbih->tuners.count(tvid))
		tvs = dbih->tuners[tvid];
	else
		return error("retrieve()", "tvid", tvid);

	if (tvs.tables.count(tnid))
		tns = tvs.tables[tnid];
	else
		return error("retrieve()", "tnid", tnid);

	if (! tns.transponders.count(trid))
		return error("retrieve()", "trid", trid);

	e2db::tunersets_transponder txp = tns.transponders[trid];

	for (auto & item : fields)
	{
		string key = item->property("field").toString().toStdString();
		int val;

		if (this->state.ty == e2db::YTYPE::sat)
		{
			if (key == "s_freq")
				val = txp.freq;
			else if (key == "s_sr")
				val = txp.sr;
			else if (key == "s_pol")
				val = txp.pol;
			else if (key == "s_fec")
				val = txp.fec;
			else if (key == "s_inv")
				val = txp.inv;
			else if (key == "s_sys")
				val = txp.sys;
			else if (key == "s_mod")
				val = txp.mod;
			else if (key == "s_rol")
				val = txp.rol;
			else if (key == "s_pil")
				val = txp.pil;
			else if (key == "s_isid")
				val = txp.isid;
			else if (key == "s_mts")
				val = txp.mts;
			else if (key == "s_plsmode")
				val = txp.plsmode;
			else if (key == "s_plscode")
				val = txp.plscode;
			else if (key == "s_plsn")
				val = txp.plsn;
		}
		else if (this->state.ty == e2db::YTYPE::terrestrial)
		{
			if (key == "t_freq")
				val = txp.freq;
			else if (key == "t_tmod")
				val = txp.tmod;
			else if (key == "t_band")
				val = txp.band;
			else if (key == "t_sys")
				val = txp.sys;
			else if (key == "t_tmx")
				val = txp.tmx;
			else if (key == "t_hpfec")
				val = txp.hpfec;
			else if (key == "t_lpfec")
				val = txp.lpfec;
			else if (key == "t_inv")
				val = txp.inv;
			else if (key == "t_guard")
				val = txp.guard;
			else if (key == "t_hier")
				val = txp.hier;
		}
		else if (this->state.ty == e2db::YTYPE::cable)
		{
			if (key == "c_freq")
				val = txp.freq;
			else if (key == "c_sr")
				val = txp.sr;
			else if (key == "c_cfec")
				val = txp.cfec;
			else if (key == "c_inv")
				val = txp.inv;
			else if (key == "c_sys")
				val = txp.sys;
			else if (key == "c_cmod")
				val = txp.cmod;
		}
		else if (this->state.ty == e2db::YTYPE::atsc)
		{
			if (key == "a_freq")
				val = txp.freq;
			else if (key == "a_amod")
				val = txp.amod;
			else if (key == "a_sys")
				val = txp.sys;
		}

		if (QLineEdit* field = qobject_cast<QLineEdit*>(item))
		{
			field->setText(QString().setNum(val));
		}
		else if (QComboBox* field = qobject_cast<QComboBox*>(item))
		{
			if (int index = field->findData(val, Qt::UserRole))
				field->setCurrentIndex(index);
		}
	}
}

void editTunersetsTransponder::setEditID(string tnid, string trid)
{
	debug("setEditID()");

	this->state.edit = true;
	this->tvid = this->state.ty;
	this->tnid = tnid;
	this->trid = trid;
}

string editTunersetsTransponder::getEditID()
{
	debug("getEditID()");

	return this->trid;
}

}