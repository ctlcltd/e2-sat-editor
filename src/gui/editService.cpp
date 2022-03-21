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

#include <QDialog>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QComboBox>

#include "../commons.h"
#include "editService.h"

using namespace std;
using namespace e2se;

namespace e2se_gui
{

editService::editService(e2db* dbih, QWidget* cwid, bool add)
{
	debug("editService");
	
	this->dbih = dbih;

	QString wtitle = add ? "Add Service" : "Edit Service";
	QDialog* dial = new QDialog(cwid);
	dial->setWindowTitle(wtitle);
	dial->connect(dial, &QDialog::finished, [=]() { delete dial; delete this; });

	QGridLayout* layout = new QGridLayout;
	this->widget = new QWidget;
	this->dtform = new QGridLayout;

	// 0 | 0
	serviceLayout();
	// 1 | 0
	transponderLayout();
	// 2 | 0 collapsed
	// flagsLayout();

	// 0 | 1
	paramsLayout();

	dtform->setVerticalSpacing(32);
	widget->setLayout(dtform);
	layout->addWidget(widget, 0, 0, Qt::AlignLeft);
	layout->setContentsMargins(8, 8, 8, 8);
	dial->setLayout(layout);
	dial->exec();
}

void editService::serviceLayout()
{
	debug("editService", "layout()");

	QGroupBox* dtl0 = new QGroupBox(tr("Service"));
	QFormLayout* dtf0 = new QFormLayout;
	QLineEdit* dtf0sn = new QLineEdit;
	dtf0sn->setMinimumWidth(240);
	dtf0->addRow(tr("Service name"), dtf0sn);
	QLineEdit* dtf0st = new QLineEdit; //TODO autocompleter
	dtf0st->setMaximumWidth(64);
	dtf0->addRow(tr("Service type"), dtf0st);
	QHBoxLayout* dtb11 = new QHBoxLayout;
	dtf0->addRow(tr("Service ID"), dtb11);
	QLineEdit* dtf0sx = new QLineEdit;
	dtf0sx->setMaximumWidth(96);
	dtb11->addWidget(dtf0sx);
	dtb11->addWidget(new QLabel("[SID]"));

	dtl0->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Ignored);
	dtl0->setLayout(dtf0);
	dtform->addWidget(dtl0, 0, 0);
}

void editService::transponderLayout()
{
	QGroupBox* dtl1 = new QGroupBox(tr("Tuner"));
	QFormLayout* dtf1 = new QFormLayout;
	QComboBox* dtf1tn = new QComboBox;
	dtf1tn->setMinimumWidth(180);
	dtf1tn->setEditable(true);
	QComboBox* dtf1tx = new QComboBox;
	dtf1tx->setMinimumWidth(180);
	dtf1tx->setEditable(true);

	for (auto & q: dbih->get_transponders_index())
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

		for (auto & x: q.second)
		{
			e2db::transponder txdata = dbih->db.transponders[x.second];
			QString txid = QString::fromStdString(x.second);
			string ptxp;
			if (txdata.ttype == 's')
				ptxp = to_string(txdata.freq) + '/' + e2db::SAT_POL[txdata.pol] + '/' + to_string(txdata.sr);
			else if (txdata.ttype == 't')
				ptxp = to_string(txdata.freq) + '/' + e2db::TER_MOD[txdata.termod] + '/' + e2db::TER_BAND[txdata.band];
			else if (txdata.ttype == 'c')
				ptxp = to_string(txdata.freq) + '/' + e2db::CAB_MOD[txdata.cabmod] + '/' + to_string(txdata.sr);
			QString txp = QString::fromStdString(ptxp);
			dtf1tx->addItem(txp, txid);
		}
	}
	dtf1->addRow(tr("Satellite"), dtf1tn);
	dtf1->addRow(tr("Transponder"), dtf1tx);

	dtl1->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Ignored);
	dtl1->setLayout(dtf1);
	dtform->addWidget(dtl1, 1, 0);
}

// Package
// QToolBox PIDs, CAID, etc.
// Parameters LineEdit
void editService::paramsLayout()
{
	QGroupBox* dtl2 = new QGroupBox(tr("Parameters"));
	QVBoxLayout* dtb20 = new QVBoxLayout;
	QFormLayout* dtf2p = new QFormLayout;
	QLineEdit* dtf2pn = new QLineEdit;
	dtf2pn->setMinimumWidth(160);
	dtf2p->addRow(tr("Package"), dtf2pn);
	QGroupBox* dtb21 = new QGroupBox(tr("PIDs"));
	QFormLayout* dtf2c = new QFormLayout;

	QHBoxLayout* dtb22 = new QHBoxLayout;
	dtf2c->addRow(tr("video"), dtb22);
	QLineEdit* dtf2vp = new QLineEdit;
	dtf2vp->setMaximumWidth(64);
	dtb22->addWidget(dtf2vp);
	dtb22->addWidget(new QLabel("[VPID]"));

	QHBoxLayout* dtb23 = new QHBoxLayout;
	dtf2c->addRow(tr("pcr"), dtb23);
	QLineEdit* dtf2pp = new QLineEdit;
	dtf2pp->setMaximumWidth(64);
	dtb23->addWidget(dtf2pp);
	dtb23->addWidget(new QLabel("[PCRPID]"));

	QHBoxLayout* dtb24 = new QHBoxLayout;
	dtf2c->addRow(tr("mpeg-audio"), dtb24);
	QLineEdit* dtf2ma = new QLineEdit;
	dtf2ma->setMaximumWidth(64);
	dtb24->addWidget(dtf2ma);
	dtb24->addWidget(new QLabel("[APID]"));

	QHBoxLayout* dtb25 = new QHBoxLayout;
	dtf2c->addRow(tr("ac3"), dtb25);
	QLineEdit* dtf2a3 = new QLineEdit;
	dtf2a3->setMaximumWidth(64);
	dtb25->addWidget(dtf2a3);
	dtb25->addWidget(new QLabel);

	QHBoxLayout* dtb26 = new QHBoxLayout;
	dtf2c->addRow(tr("ac3+"), dtb26);
	QLineEdit* dtf2ap = new QLineEdit;
	dtf2ap->setMaximumWidth(64);
	dtb26->addWidget(dtf2ap);
	dtb26->addWidget(new QLabel);

	QHBoxLayout* dtb27 = new QHBoxLayout;
	dtf2c->addRow(tr("aac"), dtb27);
	QLineEdit* dtf2ac = new QLineEdit;
	dtf2ac->setMaximumWidth(64);
	dtb27->addWidget(dtf2ac);
	dtb27->addWidget(new QLabel);

	QHBoxLayout* dtb28 = new QHBoxLayout;
	dtf2c->addRow(tr("he-aac"), dtb28);
	QLineEdit* dtf2ha = new QLineEdit;
	dtf2ha->setMaximumWidth(64);
	dtb28->addWidget(dtf2ha);
	dtb28->addWidget(new QLabel);

	QHBoxLayout* dtb29 = new QHBoxLayout;
	dtf2c->addRow(tr("teletext"), dtb29);
	QLineEdit* dtf2xt = new QLineEdit;
	dtf2xt->setMaximumWidth(64);
	dtb29->addWidget(dtf2xt);
	dtb29->addWidget(new QLabel);

	dtb21->setLayout(dtf2c);
	dtb20->addLayout(dtf2p);
	dtb20->addWidget(dtb21);
	dtl2->setLayout(dtb20);
	dtform->addWidget(dtl2, 0, 1, 2, 1);
}

}
