/*!
 * e2-sat-editor/src/gui/editService.cpp
 *
 * @link https://github.com/ctlcltd/e2-sat-editor
 * @copyright e2 SAT Editor Team
 * @author Leonardo Laureti
 * @version 0.1
 * @license MIT License
 * @license GNU GPLv3 License
 */

#include <sstream>

#include <QtGlobal>
#include <QList>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QToolBox>
#include <QToolBar>
#include <QLabel>
#include <QLineEdit>
#include <QCheckBox>

#include "editService.h"
#include "theme.h"

using std::stringstream, std::to_string;
using namespace e2se;

namespace e2se_gui
{

editService::editService(e2db* dbih, e2se::logger::session* log)
{
	this->log = new logger(log, "editService");
	debug("editService()");
	
	this->dbih = dbih;
	this->txdata = dbih->get_transponders_index();
}

void editService::display(QWidget* cwid)
{
	debug("display()");

	//TODO emit err
	if (this->state.edit && ! dbih->db.services.count(chid))
		return;

	QString wtitle = this->state.edit ? "Edit Service" : "Add Service";
	this->dial = new QDialog(cwid);
	dial->setWindowTitle(wtitle);
	dial->connect(dial, &QDialog::finished, [=]() { delete dial; });

	QGridLayout* dfrm = new QGridLayout(dial);
	QVBoxLayout* dvbox = new QVBoxLayout;

	QToolBar* dttbar = new QToolBar;
	dttbar->setIconSize(QSize(16, 16));
	dttbar->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
	dttbar->setStyleSheet("QToolBar { padding: 0 8px } QToolButton { font: 16px }");
	QWidget* dtspacer = new QWidget;
	dtspacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	dttbar->addWidget(dtspacer);
	dttbar->addAction(theme::icon("edit"), tr("Save"), [=]() { this->save(); });

	this->widget = new QWidget;
	this->dtform = new QGridLayout;

	serviceLayout();
	transponderLayout();
	paramsLayout();
	if (this->state.edit)
		retrieve();

	dtform->setVerticalSpacing(32);
	widget->setContentsMargins(12, 12, 12, 12);
	widget->setLayout(dtform);

	dfrm->setColumnStretch(0, 1);
	dfrm->setRowStretch(0, 1);
	dfrm->setContentsMargins(0, 0, 0, 0);

	dvbox->addWidget(widget);
	dvbox->addWidget(dttbar);

	dfrm->addLayout(dvbox, 0, 0);

	dfrm->setSizeConstraint(QGridLayout::SetFixedSize);
	dial->setLayout(dfrm);
	dial->exec();
}

void editService::serviceLayout()
{
	debug("serviceLayout()");

	QGroupBox* dtl0 = new QGroupBox(tr("Service"));
	QFormLayout* dtf0 = new QFormLayout;
	dtf0->setRowWrapPolicy(QFormLayout::WrapAllRows);

	QLineEdit* dtf0sn = new QLineEdit;
	dtf0sn->setProperty("field", "chname");
	fields.emplace_back(dtf0sn);
	dtf0sn->setMinimumWidth(240);
	dtf0->addRow(tr("Service name"), dtf0sn);
	dtf0->addItem(new QSpacerItem(0, 0));

	QComboBox* dtf0st = new QComboBox;
	dtf0st->setProperty("field", "stype");
	fields.emplace_back(dtf0st);
	dtf0st->setMaximumWidth(100);
	dtf0st->setValidator(new QIntValidator);
	dtf0st->setEditable(true);
	dtf0->addRow(tr("Service type"), dtf0st);
	dtf0->addItem(new QSpacerItem(0, 0));
	for (auto & stype : e2db::STYPES)
	{
		dtf0st->addItem(QString::fromStdString(to_string(stype.first) + ' ' + stype.second.second), stype.first);
	}

	QHBoxLayout* dtb11 = new QHBoxLayout;
	dtf0->addRow(tr("Service ID"), dtb11);
	QLineEdit* dtf0sx = new QLineEdit;
	dtf0sx->setProperty("field", "ssid");
	fields.emplace_back(dtf0sx);
	dtf0sx->setMaximumWidth(60);
	dtf0sx->setValidator(new QIntValidator);
	dtb11->addWidget(dtf0sx);
	dtb11->addWidget(new QLabel("[SID]"));

	dtl0->setLayout(dtf0);
	dtform->addWidget(dtl0, 0, 0);
}

void editService::transponderLayout()
{
	debug("transponderLayout()");

	QGroupBox* dtl1 = new QGroupBox(tr("Tuner"));
	QFormLayout* dtf1 = new QFormLayout;
	dtf1->setSpacing(12);

	//TODO validator
	this->dtf1tn = new QComboBox;
	dtf1tn->setProperty("field", "pos");
	fields.emplace_back(dtf1tn);
	dtf1tn->setMinimumWidth(180);
	dtf1tn->setEditable(true);
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
	dtf1tn->connect(dtf1tn, &QComboBox::currentIndexChanged, [=](int index) { this->tunerComboChanged(index); });
#else
	dtf1tn->connect(dtf1tn, QOverload<int>::of(&QComboBox::currentIndexChanged), [=](int index) { this->tunerComboChanged(index); });
#endif

	//TODO validator
	this->dtf1tx = new QComboBox;
	dtf1tx->setProperty("field", "txid");
	fields.emplace_back(dtf1tx);
	dtf1tx->setMinimumWidth(180);
	dtf1tx->setEditable(true);

	for (auto & q: txdata)
	{
		QString name;
		int pos = std::stoi(q.first);
		if (dbih->tuners_pos.count(pos))
		{
			string tnid = dbih->tuners_pos.at(pos);
			e2db::tunersets_table tns = dbih->tuners[0].tables[tnid];
			name = QString::fromStdString(tns.name);
		}
		else
		{
			name = QString::fromStdString(q.first);
		}
		dtf1tn->addItem(name, pos);
	}
	dtf1->addRow(tr("Satellite"), dtf1tn);
	dtf1->addRow(tr("Transponder"), dtf1tx);

	dtl1->setLayout(dtf1);
	dtform->addWidget(dtl1, 1, 0);
}

void editService::paramsLayout()
{
	debug("paramsLayout()");

	QGroupBox* dtl2 = new QGroupBox(tr("Parameters"));
	dtl2->setMinimumWidth(250);
	QVBoxLayout* dtb20 = new QVBoxLayout;

	QToolBox* dtt2 = new QToolBox;
	dtt2->setFixedWidth(240);
	dtt2->setFixedHeight(290);

	QWidget* dtw20 = new QWidget;
	QFormLayout* dtf2p = new QFormLayout;
	dtf2p->setRowWrapPolicy(QFormLayout::WrapAllRows);

	QLineEdit* dtf2pn = new QLineEdit;
	dtf2pn->setProperty("field", "p");
	fields.emplace_back(dtf2pn);
	dtf2pn->setMinimumWidth(200);
	dtf2p->addRow(tr("Provider name"), dtf2pn);

	dtw20->setLayout(dtf2p);
	dtt2->addItem(dtw20, "Package");

	QWidget* dtw21 = new QWidget;
	QFormLayout* dtf2c = new QFormLayout;

	QHBoxLayout* dtc20 = new QHBoxLayout;
	dtf2c->addRow(tr("video"), dtc20);
	QLineEdit* dtf2vp = new QLineEdit;
	dtf2vp->setProperty("field", "vpid");
	fields.emplace_back(dtf2vp);
	dtf2vp->setMaximumWidth(48);
	dtc20->addWidget(dtf2vp);
	dtc20->addWidget(new QLabel("[VPID]"));

	QHBoxLayout* dtc21 = new QHBoxLayout;
	dtf2c->addRow(tr("mpeg-audio"), dtc21);
	QLineEdit* dtf2ma = new QLineEdit;
	dtf2ma->setProperty("field", "mpegapid");
	fields.emplace_back(dtf2ma);
	dtf2ma->setMaximumWidth(48);
	dtc21->addWidget(dtf2ma);
	dtc21->addWidget(new QLabel("[APID]"));

	QHBoxLayout* dtc22 = new QHBoxLayout;
	dtf2c->addRow(tr("tp"), dtc22);
	QLineEdit* dtf2tp = new QLineEdit;
	dtf2tp->setProperty("field", "tpid");
	fields.emplace_back(dtf2tp);
	dtf2tp->setMaximumWidth(48);
	dtc22->addWidget(dtf2tp);
	dtc22->addWidget(new QLabel("[TPID]"));

	QHBoxLayout* dtc23 = new QHBoxLayout;
	dtf2c->addRow(tr("pcr"), dtc23);
	QLineEdit* dtf2pp = new QLineEdit;
	dtf2pp->setProperty("field", "pcrpid");
	fields.emplace_back(dtf2pp);
	dtf2pp->setMaximumWidth(48);
	dtc23->addWidget(dtf2pp);
	dtc23->addWidget(new QLabel("[PCRPID]"));

	QHBoxLayout* dtc24 = new QHBoxLayout;
	dtf2c->addRow(tr("ac3"), dtc24);
	QLineEdit* dtf2a3 = new QLineEdit;
	dtf2a3->setProperty("field", "ac3pid");
	fields.emplace_back(dtf2a3);
	dtf2a3->setMaximumWidth(48);
	dtc24->addWidget(dtf2a3);
	dtc24->addWidget(new QLabel("[AC3PID]"));

	QHBoxLayout* dtc25 = new QHBoxLayout;
	dtf2c->addRow(tr("video type"), dtc25);
	QLineEdit* dtf2vt = new QLineEdit;
	dtf2vt->setProperty("field", "vtype");
	fields.emplace_back(dtf2vt);
	dtf2vt->setMaximumWidth(48);
	dtc25->addWidget(dtf2vt);

	QHBoxLayout* dtc26 = new QHBoxLayout;
	dtf2c->addRow(tr("audio channel"), dtc26);
	QLineEdit* dtf2ac = new QLineEdit;
	dtf2ac->setProperty("field", "achannel");
	fields.emplace_back(dtf2ac);
	dtf2ac->setMaximumWidth(48);
	dtc26->addWidget(dtf2ac);

	QHBoxLayout* dtc27 = new QHBoxLayout;
	dtf2c->addRow(tr("ac3 delay"), dtc27);
	QLineEdit* dtf2ad = new QLineEdit;
	dtf2ad->setProperty("field", "ac3delay");
	fields.emplace_back(dtf2ad);
	dtf2ad->setMaximumWidth(48);
	dtc27->addWidget(dtf2ad);
	dtc27->addWidget(new QLabel("<small>ms.</small>"));

	QHBoxLayout* dtc28 = new QHBoxLayout;
	dtf2c->addRow(tr("pcm delay"), dtc28);
	QLineEdit* dtf2pd = new QLineEdit;
	dtf2pd->setProperty("field", "pcmdelay");
	fields.emplace_back(dtf2pd);
	dtf2pd->setMaximumWidth(48);
	dtc28->addWidget(dtf2pd);
	dtc28->addWidget(new QLabel("<small>ms.</small>"));

	QHBoxLayout* dtc29 = new QHBoxLayout;
	dtf2c->addRow(tr("subtitle"), dtc29);
	QLineEdit* dtf2xt = new QLineEdit;
	dtf2xt->setProperty("field", "subtitle");
	fields.emplace_back(dtf2xt);
	dtf2xt->setMaximumWidth(48);
	dtc29->addWidget(dtf2xt);

	QHBoxLayout* dtc2a = new QHBoxLayout;
	dtf2c->addRow(tr("audio type"), dtc2a);
	QLineEdit* dtf2at = new QLineEdit;
	dtf2at->setProperty("field", "atype");
	fields.emplace_back(dtf2at);
	dtf2at->setMaximumWidth(48);
	dtc2a->addWidget(dtf2at);

	QHBoxLayout* dtc2b = new QHBoxLayout;
	dtf2c->addRow(tr("apid"), dtc2b);
	QLineEdit* dtf2ap = new QLineEdit;
	dtf2ap->setProperty("field", "apid");
	fields.emplace_back(dtf2ap);
	dtf2ap->setMaximumWidth(48);
	dtc2b->addWidget(dtf2ap);

	QHBoxLayout* dtc2c = new QHBoxLayout;
	dtf2c->addRow(tr("cache max"), dtc2c);
	QLineEdit* dtf2cx = new QLineEdit;
	dtf2cx->setProperty("field", "cmax");
	fields.emplace_back(dtf2cx);
	dtf2cx->setMaximumWidth(48);
	dtc2c->addWidget(dtf2cx);

	dtw21->setLayout(dtf2c);
	dtt2->addItem(dtw21, "PIDs");

	QWidget* dtw22 = new QWidget;
	QFormLayout* dtf2C = new QFormLayout;
	dtf2C->setRowWrapPolicy(QFormLayout::WrapAllRows);

	QLineEdit* dtf2cd = new QLineEdit;
	dtf2cd->setProperty("field", "raw_C");
	fields.emplace_back(dtf2cd);
	dtf2cd->setMinimumWidth(200);
	dtf2C->addRow(tr("Card ID flags"), dtf2cd);
	dtf2C->addRow(new QLabel(tr("<small>Enter them in comma separated values.<br>(eg. C:0100,C:0200,...)</small>")));

	dtw22->setLayout(dtf2C);
	dtt2->addItem(dtw22, "CAIDs");

	QWidget* dtw23 = new QWidget;
	QFormLayout* dtf2f = new QFormLayout;

	QCheckBox* dtf2fk = new QCheckBox;
	dtf2fk->setProperty("field", "fkeep");
	fields.emplace_back(dtf2fk);
	dtf2f->addRow(tr("Do not update"), dtf2fk);

	QCheckBox* dtf2fh = new QCheckBox;
	dtf2fh->setProperty("field", "fhide");
	fields.emplace_back(dtf2fh);
	dtf2f->addRow(tr("Hide in service list"), dtf2fh);

	QCheckBox* dtf2fp = new QCheckBox;
	dtf2fp->setProperty("field", "fpid");
	fields.emplace_back(dtf2fp);
	dtf2f->addRow(tr("Use edited PIDs instead"), dtf2fp);

	QCheckBox* dtf2fn = new QCheckBox;
	dtf2fn->setProperty("field", "fname");
	fields.emplace_back(dtf2fn);
	dtf2f->addRow(tr("Hold service name"), dtf2fn);

	QCheckBox* dtf2fw = new QCheckBox;
	dtf2fw->setProperty("field", "fnew");
	fields.emplace_back(dtf2fw);
	dtf2f->addRow(tr("Flag as new service"), dtf2fw);

	dtw23->setLayout(dtf2f);
	dtt2->addItem(dtw23, "Flags");

	QWidget* dtw24 = new QWidget;
	QFormLayout* dtf2o = new QFormLayout;
	dtf2o->setRowWrapPolicy(QFormLayout::WrapAllRows);

	QLineEdit* dtf2oc = new QLineEdit;
	dtf2oc->setProperty("field", "raw_data");
	fields.emplace_back(dtf2oc);
	dtf2oc->setMinimumWidth(200);
	dtf2o->addRow(tr("Custom edit flags"), dtf2oc);
	dtf2o->addRow(new QLabel(tr("<small><b>It will overwrite any previously typed!</b><br>Enter them in comma separated values.<br>(eg. p:ProviderName,c:0100,C:0200,...)</small>")));

	dtw24->setLayout(dtf2o);
	dtt2->addItem(dtw24, "Extras");

	dtb20->addWidget(dtt2);
	dtl2->setLayout(dtb20);
	dtform->addWidget(dtl2, 0, 1, 2, 1);
}

void editService::tunerComboChanged(int index)
{
	debug("tunerComboChanged()", "index", index);

	QString qpos = dtf1tn->currentData().toString();
	string pos = qpos.toStdString();

	if (! txdata.count(pos))
		return;

	for (auto & x: txdata[pos])
	{
		e2db::transponder tx = dbih->db.transponders[x.second];
		QString txid = QString::fromStdString(x.second);
		string ptxp;
		if (tx.ttype == 's')
			ptxp = to_string(tx.freq) + '/' + e2db::SAT_POL[tx.pol] + '/' + to_string(tx.sr);
		else if (tx.ttype == 't')
			ptxp = to_string(tx.freq) + '/' + e2db::TER_MOD[tx.tmod] + '/' + e2db::TER_BAND[tx.band];
		else if (tx.ttype == 'c')
			ptxp = to_string(tx.freq) + '/' + e2db::CAB_MOD[tx.cmod] + '/' + to_string(tx.sr);
		else if (tx.ttype == 'a')
			ptxp = to_string(tx.freq);
		QString txp = QString::fromStdString(ptxp);
		dtf1tx->addItem(txp, txid);
	}
}

//TODO test
void editService::store()
{
	debug("store()");

	e2db::service ch;
	if (this->state.edit)
		ch = dbih->db.services[chid];

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

		if (key == "chname")
			ch.chname = val;
		else if (key == "stype")
			ch.stype = std::stoi(val);
		else if (key == "ssid")
			ch.ssid = std::stoi(val);
		else if (key == "txid")
			ch.txid = val;

		if (key == "raw_data" && this->state.raw_data != val)
		{
			stringstream ss (val);
			string line;
			map<char, vector<string>> cdata;

			while (std::getline(ss, line, ','))
			{
				char key = line[0];
				switch (key)
				{
					case 'p': key = e2db::SDATA::p; break;
					case 'c': key = e2db::SDATA::c; break;
					case 'C': key = e2db::SDATA::C; break;
					case 'f': key = e2db::SDATA::f; break;
				}
				string val = line.substr(2);
				cdata[key].push_back(val);
			}
			ch.data = cdata;
		}
		else
		{
			if (key == "p")
			{
				ch.data[e2db::SDATA::p][0] = val;
			}
			else if (key == "vpid")
			{
				ch.data[e2db::SDATA::c] = computePIDs(ch, e2db::SDATA_PIDS::vpid, val);
			}
			else if (key == "mpegapid")
			{
				ch.data[e2db::SDATA::c] = computePIDs(ch, e2db::SDATA_PIDS::mpegapid, val);
			}
			else if (key == "tpid")
			{
				ch.data[e2db::SDATA::c] = computePIDs(ch, e2db::SDATA_PIDS::tpid, val);
			}
			else if (key == "pcrpid")
			{
				ch.data[e2db::SDATA::c] = computePIDs(ch, e2db::SDATA_PIDS::pcrpid, val);
			}
			else if (key == "ac3pid")
			{
				ch.data[e2db::SDATA::c] = computePIDs(ch, e2db::SDATA_PIDS::ac3pid, val);
			}
			else if (key == "vtype")
			{
				ch.data[e2db::SDATA::c] = computePIDs(ch, e2db::SDATA_PIDS::vtype, val);
			}
			else if (key == "achannel")
			{
				ch.data[e2db::SDATA::c] = computePIDs(ch, e2db::SDATA_PIDS::achannel, val);
			}
			else if (key == "ac3delay")
			{
				ch.data[e2db::SDATA::c] = computePIDs(ch, e2db::SDATA_PIDS::ac3delay, val);
			}
			else if (key == "pcmdelay")
			{
				ch.data[e2db::SDATA::c] = computePIDs(ch, e2db::SDATA_PIDS::pcmdelay, val);
			}
			else if (key == "subtitle")
			{
				ch.data[e2db::SDATA::c] = computePIDs(ch, e2db::SDATA_PIDS::subtitle, val);
			}
			else if (key == "atype")
			{
				ch.data[e2db::SDATA::c] = computePIDs(ch, e2db::SDATA_PIDS::atype, val);
			}
			else if (key == "apid")
			{
				ch.data[e2db::SDATA::c] = computePIDs(ch, e2db::SDATA_PIDS::apid, val);
			}
			else if (key == "cmax")
			{
				ch.data[e2db::SDATA::c] = computePIDs(ch, e2db::SDATA_PIDS::cmax, val);
			}
			else if (key == "raw_C" && this->state.raw_C != val)
			{
				stringstream ss (val);
				string line;
				vector<string> cdata;

				while (std::getline(ss, line, ','))
				{
					char key = line[0];
					if (key != 'C')
						continue;
					string val = line.substr(2);
					cdata.push_back(val);
				}
				ch.data[e2db::SDATA::C] = cdata;
			}
			else if (key == "fkeep")
			{
				ch.data[e2db::SDATA::f] = computeFlags(ch, e2db::SDATA_FLAGS::fkeep, val);
			}
			else if (key == "fhide")
			{
				ch.data[e2db::SDATA::f] = computeFlags(ch, e2db::SDATA_FLAGS::fhide, val);
			}
			else if (key == "fpid")
			{
				ch.data[e2db::SDATA::f] = computeFlags(ch, e2db::SDATA_FLAGS::fpid, val);
			}
			else if (key == "fname")
			{
				ch.data[e2db::SDATA::f] = computeFlags(ch, e2db::SDATA_FLAGS::fname, val);
			}
			else if (key == "fnew")
			{
				ch.data[e2db::SDATA::f] = computeFlags(ch, e2db::SDATA_FLAGS::fnew, val);
			}
		}
	}
	if (ch.data[e2db::SDATA::f].empty())
	{
		ch.data.erase(e2db::SDATA::f);
	}

	if (this->state.edit)
		this->chid = dbih->editService(chid, ch);
	else
		this->chid = dbih->addService(ch);
}

void editService::retrieve()
{
	debug("retrieve()");

	e2db::service ch = dbih->db.services[chid];
	e2db::transponder tx = dbih->db.transponders[ch.txid];

	for (auto & item : fields)
	{
		string key = item->property("field").toString().toStdString();
		string val;

		if (key == "chname")
			val = ch.chname;
		else if (key == "stype")
			val = to_string(ch.stype);
		else if (key == "ssid")
			val = to_string(ch.ssid);
		else if (key == "pos")
			val = to_string(tx.pos);
		else if (key == "txid")
			val = ch.txid;

		if (key == "p" && ch.data.count(e2db::SDATA::p))
		{
			val = ch.data[e2db::SDATA::p][0];
		}
		else if (key == "vpid" && ch.data.count(e2db::SDATA::c))
		{
			val = getPIDValue(ch, e2db::SDATA_PIDS::vpid);
		}
		else if (key == "mpegapid" && ch.data.count(e2db::SDATA::c))
		{
			val = getPIDValue(ch, e2db::SDATA_PIDS::mpegapid);
		}
		else if (key == "tpid" && ch.data.count(e2db::SDATA::c))
		{
			val = getPIDValue(ch, e2db::SDATA_PIDS::tpid);
		}
		else if (key == "pcrpid" && ch.data.count(e2db::SDATA::c))
		{
			val = getPIDValue(ch, e2db::SDATA_PIDS::pcrpid);
		}
		else if (key == "ac3pid" && ch.data.count(e2db::SDATA::c))
		{
			val = getPIDValue(ch, e2db::SDATA_PIDS::ac3pid);
		}
		else if (key == "vtype" && ch.data.count(e2db::SDATA::c))
		{
			val = getPIDValue(ch, e2db::SDATA_PIDS::vtype);
		}
		else if (key == "achannel" && ch.data.count(e2db::SDATA::c))
		{
			val = getPIDValue(ch, e2db::SDATA_PIDS::achannel);
		}
		else if (key == "ac3delay" && ch.data.count(e2db::SDATA::c))
		{
			val = getPIDValue(ch, e2db::SDATA_PIDS::ac3delay);
		}
		else if (key == "pcmdelay" && ch.data.count(e2db::SDATA::c))
		{
			val = getPIDValue(ch, e2db::SDATA_PIDS::pcmdelay);
		}
		else if (key == "subtitle" && ch.data.count(e2db::SDATA::c))
		{
			val = getPIDValue(ch, e2db::SDATA_PIDS::subtitle);
		}
		else if (key == "atype" && ch.data.count(e2db::SDATA::c))
		{
			val = getPIDValue(ch, e2db::SDATA_PIDS::atype);
		}
		else if (key == "apid" && ch.data.count(e2db::SDATA::c))
		{
			val = getPIDValue(ch, e2db::SDATA_PIDS::apid);
		}
		else if (key == "cmax" && ch.data.count(e2db::SDATA::c))
		{
			val = getPIDValue(ch, e2db::SDATA_PIDS::cmax);
		}
		else if (key == "raw_C" && ch.data.count(e2db::SDATA::C))
		{
			auto last_key = (*prev(ch.data.at(e2db::SDATA::C).cend()));
			for (string & w: ch.data.at(e2db::SDATA::C))
			{
				val += "C:" + w;
				if (w != last_key)
					val += ',';
			}
			this->state.raw_C = val;
		}
		else if (key == "fkeep" && ch.data.count(e2db::SDATA::f))
		{
			val = getFlagValue(ch, e2db::SDATA_FLAGS::fkeep);
		}
		else if (key == "fhide" && ch.data.count(e2db::SDATA::f))
		{
			val = getFlagValue(ch, e2db::SDATA_FLAGS::fhide);
		}
		else if (key == "fpid" && ch.data.count(e2db::SDATA::f))
		{
			val = getFlagValue(ch, e2db::SDATA_FLAGS::fpid);
		}
		else if (key == "fname" && ch.data.count(e2db::SDATA::f))
		{
			val = getFlagValue(ch, e2db::SDATA_FLAGS::fname);
		}
		else if (key == "fnew" && ch.data.count(e2db::SDATA::f))
		{
			val = getFlagValue(ch, e2db::SDATA_FLAGS::fnew);
		}
		else if (key == "raw_data")
		{
			auto last_key = (*prev(ch.data.cend()));
			for (auto & q: ch.data)
			{
				char d;
				switch (q.first)
				{
					case e2db::SDATA::p: d = 'p'; break;
					case e2db::SDATA::c: d = 'c'; break;
					case e2db::SDATA::C: d = 'C'; break;
					case e2db::SDATA::f: d = 'f'; break;
					default: d = q.first;
				}
				for (unsigned int i = 0; i < q.second.size(); i++)
				{
					val += d;
					val += ':' + q.second[i];
					if (! q.second[i].empty() && (i != q.second.size() - 1 || q.first != last_key.first))
						val += ',';
				}
			}
			this->state.raw_data = val;
		}

		if (QLineEdit* field = qobject_cast<QLineEdit*>(item))
		{
			field->setText(QString::fromStdString(val));
		}
		else if (QComboBox* field = qobject_cast<QComboBox*>(item))
		{
			if (int index = field->findData(QString::fromStdString(val), Qt::UserRole))
				field->setCurrentIndex(index);
		}
		else if (QCheckBox* field = qobject_cast<QCheckBox*>(item))
		{
			field->setChecked(!! std::atoi(val.data()));
		}
	}
}

void editService::save()
{
	debug("save()");

	store();

	dial->close();
}

string editService::getPIDValue(e2db::service ch, e2db::SDATA_PIDS x)
{
	string cpx = (x > 9 ? "" : "0") + to_string(x);
	for (string & w : ch.data[e2db::SDATA::c])
	{
		if (w.substr(0, 2) == cpx)
			return to_string(std::strtol(w.substr(2).data(), NULL, 16));
	}
	return "";
}

vector<string> editService::computePIDs(e2db::service ch, e2db::SDATA_PIDS x, string val)
{
	bool found = false;
	char pid[7];
	int cval;
	vector<string> data = ch.data[e2db::SDATA::c];
	string cpx = (x > 9 ? "" : "0") + to_string(x);
	cval = std::atoi(val.data());
	std::sprintf(pid, "%2s%04x", cpx.c_str(), cval);
	for (auto it = data.begin(); it != data.end(); it++)
	{
		if ((*it).substr(0, 2) == cpx)
		{
			if (cval)
				*it = pid;
			else
				data.erase(it);
			found = true;
		}
	}
	if (! found && cval)
	{
		data.emplace_back(pid);
	}
	return data;
}

string editService::getFlagValue(e2db::service ch, e2db::SDATA_FLAGS x)
{
	
	int flags = std::strtol(ch.data[e2db::SDATA::f][0].data(), NULL, 16);
	if (flags & x)
		return "1";
	return "";
}

vector<string> editService::computeFlags(e2db::service ch, e2db::SDATA_FLAGS x, string val)
{
	vector<string> data = ch.data[e2db::SDATA::f];
	int flags = data.empty() ? 0 : std::strtol(data[0].data(), NULL, 16);
	if (flags & x)
		flags -= x;
	if (val == "1")
		flags += x;
	char cflags[3];
	std::sprintf(cflags, "%02x", flags);
	data.clear();
	if (flags)
		data.emplace_back(cflags);
	return data;
}

void editService::setEditID(string chid)
{
	debug("setEditID()");

	this->chid = chid;
	this->state.edit = true;
}

string editService::getEditID()
{
	debug("getEditID()");

	return this->chid;
}

void editService::destroy()
{
	delete this;
}

}
