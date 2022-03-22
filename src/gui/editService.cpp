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

#include <QtGlobal>
#include <QList>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QToolBox>
#include <QPushButton>
#include <QLabel>
#include <QLineEdit>
#include <QCheckBox>

#include "../commons.h"
#include "editService.h"
#include "todo.h"

using namespace std;
using namespace e2se;

namespace e2se_gui
{

editService::editService(e2db* dbih)
{
	debug("editService");
	
	this->dbih = dbih;
	this->txdata = dbih->get_transponders_index();
}

void editService::display(QWidget* cwid)
{
	debug("editService", "display()");

	//TODO emit err
	if (this->_state_edit && ! dbih->db.services.count(chid))
		return;

	QString wtitle = this->_state_edit ? "Edit Service" : "Add Service";
	this->dial = new QDialog(cwid);
	dial->setWindowTitle(wtitle);
	dial->connect(dial, &QDialog::finished, [=]() { delete dial; delete this; });

	QGridLayout* dfrm = new QGridLayout(dial);
	QHBoxLayout* dhbox = new QHBoxLayout;
	QVBoxLayout* dvbox = new QVBoxLayout;

	QPushButton* dtsave = new QPushButton;
	dtsave->setDefault(true);
	dtsave->setText(tr("Save"));
	dtsave->connect(dtsave, &QPushButton::pressed, [=]() { this->save(); });
	QPushButton* dtcancel = new QPushButton;
	dtcancel->setDefault(false);
	dtcancel->setText(tr("Cancel"));
	dtcancel->connect(dtcancel, &QPushButton::pressed, [=]() { dial->close(); });

	this->widget = new QWidget;
	this->dtform = new QGridLayout;

	serviceLayout();
	transponderLayout();
	paramsLayout();
	if (this->_state_edit)
		retrieve();

	dtform->setVerticalSpacing(32);
	dtform->setContentsMargins(0, 0, 0, 0);
	widget->setLayout(dtform);

	dfrm->setColumnStretch(0, 1);
	dfrm->setRowStretch(0, 1);
	dhbox->setAlignment(Qt::AlignRight);

	dvbox->addWidget(widget);
	dhbox->addWidget(dtcancel);
	dhbox->addWidget(dtsave);
	dvbox->addLayout(dhbox);

	dfrm->addLayout(dvbox, 0, 0);

	dfrm->setSizeConstraint(QGridLayout::SetFixedSize);
	dial->setLayout(dfrm);
	dial->exec();
}

void editService::serviceLayout()
{
	debug("editService", "layout()");

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
	debug("editService", "transponderLayout()");

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
		int pos = stoi(q.first);
		if (dbih->tuners.count(pos))
		{
			e2db::tuner_sets tndata = dbih->tuners.at(stoi(q.first));
			name = QString::fromStdString(tndata.name);
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

//TODO delay flags
void editService::paramsLayout()
{
	debug("editService", "paramsLayout()");

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

	QHBoxLayout* dtb22 = new QHBoxLayout;
	dtf2c->addRow(tr("video"), dtb22);
	QLineEdit* dtf2vp = new QLineEdit;
	dtf2vp->setProperty("field", "vpid");
	fields.emplace_back(dtf2vp);
	dtf2vp->setMaximumWidth(64);
	dtb22->addWidget(dtf2vp);
	dtb22->addWidget(new QLabel("[VPID]"));

	QHBoxLayout* dtb23 = new QHBoxLayout;
	dtf2c->addRow(tr("pcr"), dtb23);
	QLineEdit* dtf2pp = new QLineEdit;
	dtf2pp->setProperty("field", "pcrpid");
	fields.emplace_back(dtf2pp);
	dtf2pp->setMaximumWidth(64);
	dtb23->addWidget(dtf2pp);
	dtb23->addWidget(new QLabel("[PCRPID]"));

	QHBoxLayout* dtb24 = new QHBoxLayout;
	dtf2c->addRow(tr("mpeg-audio"), dtb24);
	QLineEdit* dtf2ma = new QLineEdit;
	dtf2ma->setProperty("field", "mpegapid");
	fields.emplace_back(dtf2ma);
	dtf2ma->setMaximumWidth(64);
	dtb24->addWidget(dtf2ma);
	dtb24->addWidget(new QLabel("[APID]"));

	QHBoxLayout* dtb25 = new QHBoxLayout;
	dtf2c->addRow(tr("ac3"), dtb25);
	QLineEdit* dtf2a3 = new QLineEdit;
	dtf2a3->setProperty("field", "ac3pid");
	fields.emplace_back(dtf2a3);
	dtf2a3->setMaximumWidth(64);
	dtb25->addWidget(dtf2a3);
	dtb25->addWidget(new QLabel("[3PID]"));

	QHBoxLayout* dtb26 = new QHBoxLayout;
	dtf2c->addRow(tr("ac3+"), dtb26);
	QLineEdit* dtf2ap = new QLineEdit;
	dtf2ap->setProperty("field", "ac3ppid");
	fields.emplace_back(dtf2ap);
	dtf2ap->setMaximumWidth(64);
	dtb26->addWidget(dtf2ap);
	dtb26->addWidget(new QLabel("[PPID]"));

	QHBoxLayout* dtb27 = new QHBoxLayout;
	dtf2c->addRow(tr("aac"), dtb27);
	QLineEdit* dtf2ac = new QLineEdit;
	dtf2ac->setProperty("field", "aacpid");
	fields.emplace_back(dtf2ac);
	dtf2ac->setMaximumWidth(64);
	dtb27->addWidget(dtf2ac);
	dtb27->addWidget(new QLabel);

	QHBoxLayout* dtb28 = new QHBoxLayout;
	dtf2c->addRow(tr("he-aac"), dtb28);
	QLineEdit* dtf2ha = new QLineEdit;
	dtf2ha->setProperty("field", "heaacpid");
	fields.emplace_back(dtf2ha);
	dtf2ha->setMaximumWidth(64);
	dtb28->addWidget(dtf2ha);
	dtb28->addWidget(new QLabel);

	QHBoxLayout* dtb29 = new QHBoxLayout;
	dtf2c->addRow(tr("teletext"), dtb29);
	QLineEdit* dtf2xt = new QLineEdit;
	dtf2xt->setProperty("field", "tpid");
	fields.emplace_back(dtf2xt);
	dtf2xt->setMaximumWidth(64);
	dtb29->addWidget(dtf2xt);
	dtb29->addWidget(new QLabel("[TPID]"));

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
	dtf2fk->setProperty("field", "skeep");
	fields.emplace_back(dtf2fk);
	dtf2f->addRow(tr("Do not update"), dtf2fk);

	QCheckBox* dtf2fh = new QCheckBox;
	dtf2fh->setProperty("field", "shide");
	fields.emplace_back(dtf2fh);
	dtf2f->addRow(tr("Hide in service list"), dtf2fh);

	QCheckBox* dtf2fp = new QCheckBox;
	dtf2fp->setProperty("field", "spid");
	fields.emplace_back(dtf2fp);
	dtf2f->addRow(tr("Use edited PIDs instead"), dtf2fp);

	QCheckBox* dtf2fn = new QCheckBox;
	dtf2fn->setProperty("field", "snew");
	fields.emplace_back(dtf2fn);
	dtf2f->addRow(tr("Flag as new service"), dtf2fn);

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
	debug("editService", "tunerComboChanged()", "index", to_string(index));

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
			ptxp = to_string(tx.freq) + '/' + e2db::TER_MOD[tx.termod] + '/' + e2db::TER_BAND[tx.band];
		else if (tx.ttype == 'c')
			ptxp = to_string(tx.freq) + '/' + e2db::CAB_MOD[tx.cabmod] + '/' + to_string(tx.sr);
		QString txp = QString::fromStdString(ptxp);
		dtf1tx->addItem(txp, txid);
	}
}

//TODO
void editService::store()
{
	debug("editService", "store()");

	e2db::service ch = dbih->db.services[chid];

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
			ch.stype = stoi(val);
		else if (key == "ssid")
			ch.ssid = stoi(val);
		else if (key == "txid")
			ch.txid = val;

		if (key == "raw_data" && this->_state_raw_data != val)
			continue;
		else if (key == "raw_C" && this->_state_raw_C != val)
			continue;
	}

	//TODO e2db | e2db_gui
	dbih->entries.services[chid] = dbih->entry_service(ch);
}

//TODO PIDs
//TODO flags
void editService::retrieve()
{
	debug("editService", "retrieve()");

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
		
		if (key == "p" && ch.data.count(e2db::PVDR_DATA.at('p')))
		{
			val = ch.data[e2db::PVDR_DATA.at('p')][0];
		}
		else if (key == "raw_C" && ch.data.count(e2db::PVDR_DATA.at('C')))
		{
			auto last_key = (*prev(ch.data.at(e2db::PVDR_DATA.at('C')).cend()));
			for (auto & w: ch.data.at(e2db::PVDR_DATA.at('C')))
			{
				val += "C:" + w;
				if (w != last_key)
					val += ',';
			}
			this->_state_raw_C = val;
		}
		else if (key == "raw_data")
		{
			auto last_key = (*prev(ch.data.cend()));
			for (auto & q: ch.data)
			{
				char d = e2db::PVDR_DATA_DENUM.count(q.first) ? e2db::PVDR_DATA_DENUM.at(q.first) : q.first;
				for (unsigned int i = 0; i < q.second.size(); i++)
				{
					val += d;
					val += ':' + q.second[i];
					if (! q.second[i].empty() && (i != q.second.size() - 1 || q.first != last_key.first))
						val += ',';
				}
			}
			this->_state_raw_data = val;
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
			continue;
		}
	}
}

void editService::save()
{
	debug("editService", "save()");

	store();

	dial->close();
}

void editService::setEditID(string chid)
{
	debug("editService", "setEditID()");

	this->chid = chid;
	this->_state_edit = true;
}

}
