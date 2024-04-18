/*!
 * e2-sat-editor/src/gui/editService.cpp
 *
 * @link https://github.com/ctlcltd/e2-sat-editor
 * @copyright e2 SAT Editor Team
 * @author Leonardo Laureti
 * @version 1.4.0
 * @license MIT License
 * @license GNU GPLv3 License
 */

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <sstream>
#include <algorithm>

#include <QtGlobal>
#include <QTimer>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QToolBox>
#include <QLabel>
#include <QLineEdit>
#include <QCheckBox>
#include <QPushButton>
#ifdef Q_OS_WIN
#include <QStyleFactory>
#include <QScrollBar>
#endif

#include "platforms/platform.h"

#include "editService.h"
#include "editTransponder.h"
#include "editFavourite.h"

using std::stringstream, std::to_string, std::sort;

using namespace e2se;

namespace e2se_gui
{

editService::editService(dataHandler* data)
{
	this->log = new logger("gui", "editService");

	this->data = data;
}

editService::~editService()
{
	debug("~editService");

	delete this->log;
}

void editService::display(QWidget* cwid)
{
	layout(cwid);

	if (this->state.edit)
	{
		retrieve();

		if (isStream())
			dtwid->setCurrentIndex(2);
	}

	dial->exec();
}

void editService::layout(QWidget* cwid)
{
	this->dialAbstract::layout(cwid);

	QString dtitle = this->state.edit ? tr("Edit Service", "dialog") : tr("New Service", "dialog");
	dial->setWindowTitle(dtitle);

	auto* dbih = this->data->dbih;

	this->txp_index = dbih->get_transponders_index();

	this->dtwid = new QTabWidget;
	dtwid->connect(dtwid, &QTabWidget::currentChanged, [=](int index) { this->tabChanged(index); });

	this->edittxp = new editTransponder(this->data);

	if (isFavourite())
		this->editfav = new editFavourite(this->data);

	QWidget* dtchn = new QWidget;
	QWidget* dttxp = new QWidget;
	QWidget* dtfav = new QWidget;
	this->dtpage = new QGridLayout;

	serviceLayout();
	transponderLayout();
	paramsLayout();

	dtpage->addItem(new QSpacerItem(0, 0), 1, 0);

	dtchn->setLayout(dtpage);

	dtwid->addTab(dtchn, tr("Service", "dialog"));
	dtwid->addTab(dttxp, tr("Transponder", "dialog"));

	if (isFavourite())
		dtwid->addTab(dtfav, tr("Favourite", "dialog"));

	dtform->addWidget(dtwid, 0, 0);
}

//TODO custom toolbarLayout

void editService::serviceLayout()
{
	debug("serviceLayout");

	QGroupBox* dtl0 = new QGroupBox(tr("Service", "dialog"));
	QFormLayout* dtf0 = new QFormLayout;
	dtf0->setRowWrapPolicy(QFormLayout::WrapAllRows);

	QLineEdit* dtf0sn = new QLineEdit;
	dtf0sn->setProperty("field", "chname");
	fields.emplace_back(dtf0sn);
	dtf0sn->setMinimumWidth(240);
	dtf0sn->setMaxLength(255);
	platform::osLineEdit(dtf0sn);
	dtf0->addRow(tr("Service name"), dtf0sn);
	dtf0->addItem(new QSpacerItem(0, 0));

	QWidget* dtw10 = new QWidget;
	dtw10->setMinimumHeight(50);
	QHBoxLayout* dtb10 = new QHBoxLayout(dtw10);
	dtb10->setContentsMargins(0, 0, 0, 0);
	dtf0->addRow(tr("Service type"), dtw10);

	QLineEdit* dtf0st = new QLineEdit;
	dtf0st->setProperty("field", "stype");
	dtf0st->setVisible(false);
	fields.emplace_back(dtf0st);
	dtf0st->setMinimumWidth(50);
	dtf0st->setMaxLength(9);
	dtf0st->setValidator(new QIntValidator);

	QComboBox* dtf0sc = new QComboBox;
	dtf0sc->setMaximumWidth(100);
	dtf0sc->setValidator(new QIntValidator);
	platform::osComboBox(dtf0sc);

	vector<int> stypes;
	for (auto & x : e2db::STYPE_EXT_TYPE)
	{
		stypes.emplace_back(x.first);
	}
	sort(stypes.begin(), stypes.end());
	for (auto & q : stypes)
	{
		string pad = q > 9 ? "  " : "  ";
		dtf0sc->addItem(QString::fromStdString(to_string(q) + pad + e2db::STYPE_EXT_LABEL.at(q)), q);
	}

	QPushButton* dtf0sb = new QPushButton;
	dtf0sb->setDefault(false);
	dtf0sb->setCheckable(true);
	dtf0sb->setText(tr("custom"));
	dtf0sb->setChecked(false);

	dtf0sb->connect(dtf0sb, &QPushButton::pressed, [=]() {
		// delay too fast
		QTimer::singleShot(100, [=]() {
			if (dtf0sc->isHidden())
			{
				dtf0st->hide();
				dtf0sc->show();
				dtf0sb->setChecked(true);
				dtf0sc->setFocus();
			}
			else
			{
				dtf0sc->hide();
				dtf0st->show();
				dtf0sb->setChecked(false);
				dtf0st->setFocus();
			}
		});
	});

	dtf0st->connect(dtf0st, &QLineEdit::textChanged, [=](QString text) {
		int index = dtf0sc->findData(text, Qt::UserRole);
		dtf0sc->setCurrentIndex(index);
	});

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
	dtf0sc->connect(dtf0sc, &QComboBox::currentIndexChanged, [=](int index) {
#else
	dtf0sc->connect(dtf0sc, QOverload<int>::of(&QComboBox::currentIndexChanged), [=](int index) {
#endif
		if (index == -1 && ! dtf0sb->isChecked())
		{
			dtf0sc->hide();
			dtf0st->show();
			dtf0st->setFocus();
			dtf0sb->setChecked(true);
		}
		else
		{
			dtf0sb->setChecked(false);
			dtf0st->setText(dtf0sc->currentData().toString());
		}
	});
	dtf0st->setText(dtf0sc->currentData().toString());

	dtb10->addWidget(dtf0sc);
	dtb10->addWidget(dtf0st);
	dtb10->addWidget(dtf0sb);

	QHBoxLayout* dtb11 = new QHBoxLayout;
	dtb11->setSpacing(10);
	dtf0->addRow(tr("Service ID"), dtb11);
	QLineEdit* dtf0sx = new QLineEdit;
	dtf0sx->setProperty("field", "ssid");
	fields.emplace_back(dtf0sx);
	dtf0sx->setMaximumWidth(60);
	dtf0sx->setValidator(new QIntValidator(0, 99999999));
	platform::osLineEdit(dtf0sx);
	dtb11->addWidget(dtf0sx);
	dtb11->addWidget(new QLabel("[SID]"));

	dtl0->setLayout(dtf0);
	dtpage->addWidget(dtl0, 0, 0);
}

void editService::transponderLayout()
{
	debug("transponderLayout");

	QGroupBox* dtl1 = new QGroupBox(tr("Tuner", "dialog"));
	QFormLayout* dtf1 = new QFormLayout;
	dtf1->setSpacing(12);

	this->dtf1tn = new QComboBox;
	dtf1tn->setProperty("field", "pos");
	fields.emplace_back(dtf1tn);
	dtf1tn->setMinimumWidth(180);
	dtf1tn->setValidator(new QIntValidator);
	dtf1tn->setEditable(true);
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
	dtf1tn->connect(dtf1tn, &QComboBox::currentIndexChanged, [=](int index) {
#else
	dtf1tn->connect(dtf1tn, QOverload<int>::of(&QComboBox::currentIndexChanged), [=](int index) {
#endif
		this->tunerComboChanged(index);
	});
	platform::osComboBox(dtf1tn);

	auto* dbih = this->data->dbih;

	this->dtf1tx = new QComboBox;
	dtf1tx->setProperty("field", "txid");
	fields.emplace_back(dtf1tx);
	dtf1tx->setMinimumWidth(180);
	dtf1tx->setEditable(true);
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
	dtf1tx->connect(dtf1tx, &QComboBox::currentIndexChanged, [=](int index) {
#else
	dtf1tx->connect(dtf1tx, QOverload<int>::of(&QComboBox::currentIndexChanged), [=](int index) {
#endif
		this->transponderComboChanged(index);
	});
	for (auto & q : txp_index)
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
			string ppos = pos == -1 ? "NaN" : q.first;
			name = QString::fromStdString(ppos);
		}
		dtf1tn->addItem(name, pos);
	}
	platform::osComboBox(dtf1tx);
	dtf1->addRow(tr("Satellite"), dtf1tn);
	dtf1->addRow(tr("Transponder"), dtf1tx);

	QPushButton* dtf1nt = new QPushButton;
	dtf1nt->setDefault(false);
	dtf1nt->setText(tr("New Transponder"));
	dtf1nt->connect(dtf1nt, &QPushButton::pressed, [=]() { this->newTransponder(); });
	dtf1->addRow(NULL, dtf1nt);

	dtl1->setLayout(dtf1);
	dtpage->addWidget(dtl1, 2, 0);
}

void editService::paramsLayout()
{
	debug("paramsLayout");

	QGroupBox* dtl2 = new QGroupBox(tr("Parameters", "dialog"));
	dtl2->setMinimumWidth(250);
	QVBoxLayout* dtb20 = new QVBoxLayout;

	QToolBox* dtt2 = new QToolBox;
	dtt2->setFixedWidth(240);
	dtt2->setFixedHeight(320);
	//TODO improve ui [Windows]
#ifdef Q_OS_WIN
	if (! theme::isOverridden() && (theme::absLuma() || ! theme::isDefault()))
	{
		QStyle* style = QStyleFactory::create("fusion");
		dtt2->setStyle(style);
	}
#endif

	QWidget* dtw20 = new QWidget;
	QFormLayout* dtf2p = new QFormLayout;
	dtf2p->setRowWrapPolicy(QFormLayout::WrapAllRows);

	QLineEdit* dtf2pn = new QLineEdit;
	dtf2pn->setProperty("field", "p");
	fields.emplace_back(dtf2pn);
	dtf2pn->setMinimumWidth(200);
	dtf2pn->setMaxLength(255);
	platform::osLineEdit(dtf2pn);
	dtf2p->addRow(tr("Provider name"), dtf2pn);

	dtw20->setLayout(dtf2p);
	dtt2->addItem(dtw20, tr("Package"));

	QWidget* dtw21 = new QWidget;
	QFormLayout* dtf2c = new QFormLayout;

	QHBoxLayout* dtc20 = new QHBoxLayout;
	dtf2c->addRow(tr("video"), dtc20);
	QLineEdit* dtf2vp = new QLineEdit;
	dtf2vp->setProperty("field", "vpid");
	fields.emplace_back(dtf2vp);
	dtf2vp->setMaximumWidth(48);
	dtf2vp->setValidator(new QIntValidator(0, 65535));
	platform::osLineEdit(dtf2vp);
	dtc20->addWidget(dtf2vp);
	dtc20->addWidget(new QLabel("[VPID]"));

	QHBoxLayout* dtc21 = new QHBoxLayout;
	dtf2c->addRow(tr("mpeg-audio"), dtc21);
	QLineEdit* dtf2ma = new QLineEdit;
	dtf2ma->setProperty("field", "mpegapid");
	fields.emplace_back(dtf2ma);
	dtf2ma->setMaximumWidth(48);
	dtf2ma->setValidator(new QIntValidator(0, 65535));
	platform::osLineEdit(dtf2ma);
	dtc21->addWidget(dtf2ma);
	dtc21->addWidget(new QLabel("[APID]"));

	QHBoxLayout* dtc22 = new QHBoxLayout;
	dtf2c->addRow(tr("tp"), dtc22);
	QLineEdit* dtf2tp = new QLineEdit;
	dtf2tp->setProperty("field", "tpid");
	fields.emplace_back(dtf2tp);
	dtf2tp->setMaximumWidth(48);
	dtf2tp->setValidator(new QIntValidator(0, 65535));
	platform::osLineEdit(dtf2tp);
	dtc22->addWidget(dtf2tp);
	dtc22->addWidget(new QLabel("[TXTPID]"));

	QHBoxLayout* dtc23 = new QHBoxLayout;
	dtf2c->addRow(tr("pcr"), dtc23);
	QLineEdit* dtf2pp = new QLineEdit;
	dtf2pp->setProperty("field", "pcrpid");
	fields.emplace_back(dtf2pp);
	dtf2pp->setMaximumWidth(48);
	dtf2pp->setValidator(new QIntValidator(0, 65535));
	platform::osLineEdit(dtf2pp);
	dtc23->addWidget(dtf2pp);
	dtc23->addWidget(new QLabel("[PCRPID]"));

	QHBoxLayout* dtc24 = new QHBoxLayout;
	dtf2c->addRow(tr("ac3"), dtc24);
	QLineEdit* dtf2a3 = new QLineEdit;
	dtf2a3->setProperty("field", "ac3pid");
	fields.emplace_back(dtf2a3);
	dtf2a3->setMaximumWidth(48);
	dtf2a3->setValidator(new QIntValidator(0, 65535));
	platform::osLineEdit(dtf2a3);
	dtc24->addWidget(dtf2a3);
	dtc24->addWidget(new QLabel("[AC3PID]"));

	QHBoxLayout* dtc25 = new QHBoxLayout;
	dtf2c->addRow(tr("video type"), dtc25);
	QLineEdit* dtf2vt = new QLineEdit;
	dtf2vt->setProperty("field", "vtype");
	fields.emplace_back(dtf2vt);
	dtf2vt->setMaximumWidth(48);
	dtf2vt->setValidator(new QIntValidator(0, 65535));
	platform::osLineEdit(dtf2vt);
	dtc25->addWidget(dtf2vt);

	QHBoxLayout* dtc26 = new QHBoxLayout;
	dtf2c->addRow(tr("audio channel"), dtc26);
	QLineEdit* dtf2ac = new QLineEdit;
	dtf2ac->setProperty("field", "achannel");
	fields.emplace_back(dtf2ac);
	dtf2ac->setMaximumWidth(48);
	dtf2ac->setValidator(new QIntValidator(0, 65535));
	platform::osLineEdit(dtf2ac);
	dtc26->addWidget(dtf2ac);

	QHBoxLayout* dtc27 = new QHBoxLayout;
	dtf2c->addRow(tr("ac3 delay"), dtc27);
	QLineEdit* dtf2ad = new QLineEdit;
	dtf2ad->setProperty("field", "ac3delay");
	fields.emplace_back(dtf2ad);
	dtf2ad->setMaximumWidth(48);
	dtf2ad->setValidator(new QIntValidator(0, 65535));
	platform::osLineEdit(dtf2ad);
	dtc27->addWidget(dtf2ad);
	dtc27->addWidget(new QLabel(QString("<small>%1</small>").arg(tr("ms."))));

	QHBoxLayout* dtc28 = new QHBoxLayout;
	dtf2c->addRow(tr("pcm delay"), dtc28);
	QLineEdit* dtf2pd = new QLineEdit;
	dtf2pd->setProperty("field", "pcmdelay");
	fields.emplace_back(dtf2pd);
	dtf2pd->setMaximumWidth(48);
	dtf2pd->setValidator(new QIntValidator(0, 65535));
	platform::osLineEdit(dtf2pd);
	dtc28->addWidget(dtf2pd);
	dtc28->addWidget(new QLabel(QString("<small>%1</small>").arg(tr("ms."))));

	QHBoxLayout* dtc29 = new QHBoxLayout;
	dtf2c->addRow(tr("subtitle"), dtc29);
	QLineEdit* dtf2xt = new QLineEdit;
	dtf2xt->setProperty("field", "subtitle");
	fields.emplace_back(dtf2xt);
	dtf2xt->setMaximumWidth(48);
	dtf2xt->setValidator(new QIntValidator(0, 65535));
	platform::osLineEdit(dtf2xt);
	dtc29->addWidget(dtf2xt);

	QHBoxLayout* dtc2a = new QHBoxLayout;
	dtf2c->addRow(tr("audio type"), dtc2a);
	QLineEdit* dtf2at = new QLineEdit;
	dtf2at->setProperty("field", "atype");
	fields.emplace_back(dtf2at);
	dtf2at->setMaximumWidth(48);
	dtf2at->setValidator(new QIntValidator(0, 65535));
	platform::osLineEdit(dtf2at);
	dtc2a->addWidget(dtf2at);

	QHBoxLayout* dtc2b = new QHBoxLayout;
	dtf2c->addRow(tr("apid"), dtc2b);
	QLineEdit* dtf2ap = new QLineEdit;
	dtf2ap->setProperty("field", "apid");
	fields.emplace_back(dtf2ap);
	dtf2ap->setMaximumWidth(48);
	dtf2ap->setValidator(new QIntValidator(0, 65535));
	platform::osLineEdit(dtf2ap);
	dtc2b->addWidget(dtf2ap);

	QHBoxLayout* dtc2c = new QHBoxLayout;
	dtf2c->addRow(tr("cache max"), dtc2c);
	QLineEdit* dtf2cx = new QLineEdit;
	dtf2cx->setProperty("field", "cmax");
	fields.emplace_back(dtf2cx);
	dtf2cx->setMaximumWidth(48);
	dtf2cx->setValidator(new QIntValidator(0, 65535));
	platform::osLineEdit(dtf2cx);
	dtc2c->addWidget(dtf2cx);

	QHBoxLayout* dtc2d = new QHBoxLayout;
	dtf2c->addRow(tr("pmt"), dtc2d);
	QLineEdit* dtf2mt = new QLineEdit;
	dtf2mt->setProperty("field", "pmt");
	fields.emplace_back(dtf2mt);
	dtf2mt->setMaximumWidth(48);
	dtf2mt->setValidator(new QIntValidator(0, 65535));
	platform::osLineEdit(dtf2mt);
	dtc2d->addWidget(dtf2mt);

	dtw21->setLayout(dtf2c);
	dtt2->addItem(dtw21, tr("PIDs"));

	QWidget* dtw22 = new QWidget;
	QFormLayout* dtf2C = new QFormLayout;
	dtf2C->setRowWrapPolicy(QFormLayout::WrapAllRows);

	QLineEdit* dtf2cd = new QLineEdit;
	dtf2cd->setProperty("field", "raw_C");
	fields.emplace_back(dtf2cd);
	dtf2cd->setMinimumWidth(200);
	dtf2cd->setMaxLength(255);
	platform::osLineEdit(dtf2cd);
	dtf2C->addRow(tr("Card ID parameters"), dtf2cd);
	//: HTML formattation: text%1text treat them as spaces
	dtf2C->addRow(new QLabel(QString("<small>%1</small>").arg(tr("Enter them in comma separated values.%1(eg. C:0100,C:0200,…)").arg("<br>"))));

	dtw22->setLayout(dtf2C);
	dtt2->addItem(dtw22, tr("CAIDs"));

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
	dtt2->addItem(dtw23, tr("Flags"));

	QWidget* dtw24 = new QWidget;
	QFormLayout* dtf2r = new QFormLayout;
	dtf2r->setRowWrapPolicy(QFormLayout::WrapAllRows);

	QLineEdit* dtf2rc = new QLineEdit;
	dtf2rc->setProperty("field", "raw_data");
	fields.emplace_back(dtf2rc);
	dtf2rc->setMinimumWidth(200);
	dtf2rc->setMaxLength(511);
	platform::osLineEdit(dtf2rc);
	dtf2r->addRow(tr("Raw edit parameters"), dtf2rc);
	//: HTML formattation: %1text%2text%3text treat them as spaces
	dtf2r->addRow(new QLabel(QString("<small>%1</small>").arg(tr("%1It will overwrite any previously typed!%2Enter them in comma separated values.%3(eg. p:ProviderName,c:0100,C:0200,…)")).arg("<b>").arg("</b><br>").arg("<br>")));

	dtw24->setLayout(dtf2r);
	dtt2->addItem(dtw24, tr("Custom"));

	QWidget* dtw25 = new QWidget;
	QFormLayout* dtf2e = new QFormLayout;
	dtf2e->setFormAlignment(Qt::AlignLeading);
	dtf2e->setRowWrapPolicy(QFormLayout::WrapAllRows);

	QLineEdit* dtf2sn = new QLineEdit;
	dtf2sn->setProperty("field", "snum");
	fields.emplace_back(dtf2sn);
	dtf2sn->setMaximumWidth(60);
	dtf2sn->setValidator(new QIntValidator(0, 99999999));
	platform::osLineEdit(dtf2sn);
	dtf2e->addRow(tr("Service Number"), dtf2sn);

	QLineEdit* dtf2sr = new QLineEdit;
	dtf2sr->setProperty("field", "srcid");
	fields.emplace_back(dtf2sr);
	dtf2sr->setMaximumWidth(60);
	dtf2sr->setValidator(new QIntValidator(0, 99999999));
	platform::osLineEdit(dtf2sr);
	dtf2e->addRow(tr("Src ID"), dtf2sr);

	dtw25->setLayout(dtf2e);
	dtt2->addItem(dtw25, tr("Extras"));

	dtb20->addWidget(dtt2);
	dtl2->setLayout(dtb20);

	dtpage->addWidget(dtl2, 0, 1, 3, 1);
}

void editService::tunerComboChanged(int index)
{
	debug("tunerComboChanged", "index", index);

	QString qpos = dtf1tn->currentData().toString();
	string pos = qpos.toStdString();

	if (! txp_index.count(pos))
		return;

	auto* dbih = this->data->dbih;

	dtf1tx->clear();

	for (auto & x : txp_index[pos])
	{
		e2db::transponder tx = dbih->db.transponders[x.second];
		QString txid = QString::fromStdString(x.second);
		QString combo = QString::fromStdString(dbih->value_transponder_combo(tx));
		dtf1tx->addItem(combo, txid);
	}
}

void editService::transponderComboChanged(int index)
{
	debug("transponderComboChanged", "index", index);

	QString qtxid = dtf1tx->currentData().toString();
	string txid = qtxid.toStdString();

	edittxp->setEditId(txid);
	edittxp->reset();
}

void editService::tabChanged(int index)
{
	debug("tabChanged", "index", index);

	// tab: service = 0
	if (index == 0)
	{
		return;
	}
	// tab: transponder = 1
	else if (index == 1)
	{
		edittxp->show();

		bool retr = false;

		if (dtf1tx->currentData().isNull())
		{
			edittxp->setAddId();
		}
		else
		{
			QString qtxid = dtf1tx->currentData().toString();
			string txid = qtxid.toStdString();

			if (! this->state.transponder || this->txid != txid)
				retr = true;

			edittxp->setEditId(txid);
		}

		edittxp->layout(this->dial);
		edittxp->show(retr);

		QGridLayout* layout = new QGridLayout;
		layout->setContentsMargins(0, 0, 0, 0);
		layout->addWidget(edittxp->widget);
		dtwid->widget(1)->setLayout(layout);

		this->state.transponder = true;
	}
	// tab: favourite = 2
	else if (index == 2)
	{
		editfav->show();

		bool retr = false;

		if (this->state.edit)
		{
			if (! this->state.favourite)
				retr = true;

			editfav->setEditId(chid, bname);
		}
		else
		{
			editfav->setAddId(bname);
		}

		editfav->layout(this->dial);
		editfav->show(retr);

		QGridLayout* layout = new QGridLayout;
		layout->setContentsMargins(0, 0, 0, 0);
		layout->addWidget(editfav->widget);
		dtwid->widget(2)->setLayout(layout);

		this->state.favourite = true;
	}
}

void editService::newTransponder()
{
	debug("newTransponder");

	dtf1tx->setCurrentIndex(-1);
	edittxp->setAddId();
	edittxp->reset();

	dtwid->setCurrentIndex(1);
}

void editService::store()
{
	debug("store");

	auto* dbih = this->data->dbih;

	e2db::service ch;
	e2db::channel_reference chref;

	if (isFavourite() && dbih->userbouquets.count(bname))
	{
		e2db::userbouquet& ub = dbih->userbouquets[bname];

		if (this->state.edit)
		{
			if (! ub.channels.count(ref_chid))
				return error("store", tr("Error", "error").toStdString(), tr("Channel reference \"%1\" not exists.", "error").arg(ref_chid.data()).toStdString());

			chref = ub.channels[ref_chid];
		}
	}

	if (this->state.edit)
	{
		if (! dbih->db.services.count(ref_chid))
			return error("store", tr("Error", "error").toStdString(), tr("Service \"%1\" not exists.", "error").arg(ref_chid.data()).toStdString());

		ch = dbih->db.services[ref_chid];
	}

	string nw_txid;
	string nw_chid;

	if (this->state.transponder)
	{
		edittxp->store();

		string txid = edittxp->getEditId();
		if (! txid.empty())
			nw_txid = txid;
	}

	if (this->state.favourite)
	{
		editfav->store();

		string chid = edittxp->getEditId();
		if (! chid.empty())
			nw_chid = chid;
	}

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
			ch.stype = val.empty() ? 0 : std::stoi(val);
		else if (key == "ssid")
			ch.ssid = val.empty() ? 0 : std::stoi(val);
		else if (key == "txid")
			ch.txid = val;
		else if (key == "snum")
			ch.snum = val.empty() ? 0 : std::stoi(val);
		else if (key == "srcid")
			ch.srcid = val.empty() ? 0 : std::stoi(val);

		if (key == "raw_data" && this->state.raw_data != val)
		{
			map<char, vector<string>> cdata;

			char* token = std::strtok(val.data(), ",");
			while (token != 0)
			{
				char key = token[0];
				switch (key)
				{
					case 'p': key = e2db::SDATA::p; break;
					case 'c': key = e2db::SDATA::c; break;
					case 'C': key = e2db::SDATA::C; break;
					case 'f': key = e2db::SDATA::f; break;
				}
				string val = string (token).substr(2);
				cdata[key].push_back(val);

				token = std::strtok(NULL, ",");
			}

			ch.data = cdata;
		}
		else
		{
			if (key == "p")
				ch.data[e2db::SDATA::p] = {val};
			else if (key == "vpid")
				ch.data[e2db::SDATA::c] = computePIDs(ch, e2db::SDATA_PIDS::vpid, val);
			else if (key == "mpegapid")
				ch.data[e2db::SDATA::c] = computePIDs(ch, e2db::SDATA_PIDS::mpegapid, val);
			else if (key == "tpid")
				ch.data[e2db::SDATA::c] = computePIDs(ch, e2db::SDATA_PIDS::tpid, val);
			else if (key == "pcrpid")
				ch.data[e2db::SDATA::c] = computePIDs(ch, e2db::SDATA_PIDS::pcrpid, val);
			else if (key == "ac3pid")
				ch.data[e2db::SDATA::c] = computePIDs(ch, e2db::SDATA_PIDS::ac3pid, val);
			else if (key == "vtype")
				ch.data[e2db::SDATA::c] = computePIDs(ch, e2db::SDATA_PIDS::vtype, val);
			else if (key == "achannel")
				ch.data[e2db::SDATA::c] = computePIDs(ch, e2db::SDATA_PIDS::achannel, val);
			else if (key == "ac3delay")
				ch.data[e2db::SDATA::c] = computePIDs(ch, e2db::SDATA_PIDS::ac3delay, val);
			else if (key == "pcmdelay")
				ch.data[e2db::SDATA::c] = computePIDs(ch, e2db::SDATA_PIDS::pcmdelay, val);
			else if (key == "subtitle")
				ch.data[e2db::SDATA::c] = computePIDs(ch, e2db::SDATA_PIDS::subtitle, val);
			else if (key == "atype")
				ch.data[e2db::SDATA::c] = computePIDs(ch, e2db::SDATA_PIDS::atype, val);
			else if (key == "apid")
				ch.data[e2db::SDATA::c] = computePIDs(ch, e2db::SDATA_PIDS::apid, val);
			else if (key == "cmax")
				ch.data[e2db::SDATA::c] = computePIDs(ch, e2db::SDATA_PIDS::cmax, val);
			else if (key == "pmt")
				ch.data[e2db::SDATA::c] = computePIDs(ch, e2db::SDATA_PIDS::pmt, val);
			else if (key == "raw_C" && this->state.raw_C != val)
			{
				vector<string> cdata;

				char* token = std::strtok(val.data(), ",");
				while (token != 0)
				{
					char key = token[0];
					if (key != 'C')
						continue;
					string val = string (token).substr(2);
					cdata.push_back(val);

					token = std::strtok(NULL, ",");
				}

				ch.data[e2db::SDATA::C] = cdata;
			}
			else if (key == "fkeep")
				ch.data[e2db::SDATA::f] = computeFlags(ch, e2db::SDATA_FLAGS::fkeep, val);
			else if (key == "fhide")
				ch.data[e2db::SDATA::f] = computeFlags(ch, e2db::SDATA_FLAGS::fhide, val);
			else if (key == "fpid")
				ch.data[e2db::SDATA::f] = computeFlags(ch, e2db::SDATA_FLAGS::fpid, val);
			else if (key == "fname")
				ch.data[e2db::SDATA::f] = computeFlags(ch, e2db::SDATA_FLAGS::fname, val);
			else if (key == "fnew")
				ch.data[e2db::SDATA::f] = computeFlags(ch, e2db::SDATA_FLAGS::fnew, val);
		}
	}
	if (ch.data.count(e2db::SDATA::f) && ch.data[e2db::SDATA::f].empty())
	{
		ch.data.erase(e2db::SDATA::f);
	}

	if (! nw_txid.empty())
	{
		ch.txid = nw_txid;
	}
	if (ch.txid != ref_txid)
	{
		e2db::transponder tx = dbih->db.transponders[ch.txid];
		ch.tsid = tx.tsid;
		ch.dvbns = tx.dvbns;
		ch.onid = tx.onid;
		this->txid = ch.txid;

		char nw_chid[25];
		// %4x:%4x:%8x
		std::snprintf(nw_chid, 25, "%x:%x:%x", ch.ssid, tx.tsid, tx.dvbns);

		this->ref_chid = ch.chid = nw_chid;
	}

	if (this->state.edit)
		this->chid = dbih->editService(ref_chid, ch);
	else
		this->chid = dbih->addService(ch);

	if (isFavourite())
	{
		if (this->state.edit)
			this->chid = dbih->editChannelReference(chid, chref, bname);
		else
			this->chid = dbih->addChannelReference(chref, bname);
	}

	this->changes = true;
}

void editService::retrieve()
{
	debug("retrieve");

	auto* dbih = this->data->dbih;

	this->ref_chid = string (this->chid);
	this->ref_txid = string ();

	e2db::channel_reference chref;

	if (isFavourite() && dbih->userbouquets.count(bname))
	{
		e2db::userbouquet ub = dbih->userbouquets[bname];

		if (! ub.channels.count(ref_chid))
			return error("retrieve", tr("Error", "error").toStdString(), tr("Channel reference \"%1\" not exists.", "error").arg(ref_chid.data()).toStdString());

		chref = ub.channels[ref_chid];

		if (dbih->FAVOURITE_MATCH_SERVICE && chref.stream)
		{
			int dvbns = chref.ref.dvbns;

			for (auto & x : dbih->db.transponders)
			{
				e2db::transponder& tx = x.second;

				//TODO TEST (dvbns or onid reverse to pos)
				if (tx.tsid == chref.ref.tsid && (tx.dvbns == chref.ref.dvbns || tx.onid == chref.ref.onid))
				{
					ref_txid = tx.txid;
					dvbns = tx.dvbns;
					break;
				}
			}

			if (dvbns == chref.ref.dvbns)
			{
				ref_chid = chref.chid;
			}
			else
			{
				char chid[25];

				// %4x:%4x:%8x
				std::snprintf(chid, 25, "%x:%x:%x", chref.ref.ssid, chref.ref.tsid, dvbns);

				ref_chid = chid;
			}

			if (! dbih->db.services.count(ref_chid))
				ref_chid = "";
			else if (! dbih->db.transponders.count(ref_txid))
				ref_txid = "";
		}
	}

	if (! dbih->db.services.count(ref_chid))
		return error("retrieve", tr("Error", "error").toStdString(), tr("Service \"%1\" not exists.", "error").arg(ref_chid.data()).toStdString());

	e2db::service ch = dbih->db.services[ref_chid];
	e2db::transponder tx;

	if (ref_txid.empty() && (ch.tsid != 0 || ch.onid != 0 || ch.dvbns != 0))
	{
		ref_txid = string (ch.txid);
		tx = dbih->db.transponders[ch.txid];
	}
	else
	{
		tx = dbih->db.transponders[ref_txid];
	}

	this->txid = ref_txid;

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
		else if (key == "snum")
			val = to_string(ch.snum);
		else if (key == "srcid")
			val = to_string(ch.srcid);

		if (key == "p" && ch.data.count(e2db::SDATA::p))
			val = ch.data[e2db::SDATA::p][0];
		else if (key == "vpid" && ch.data.count(e2db::SDATA::c))
			val = getPIDValue(ch, e2db::SDATA_PIDS::vpid);
		else if (key == "mpegapid" && ch.data.count(e2db::SDATA::c))
			val = getPIDValue(ch, e2db::SDATA_PIDS::mpegapid);
		else if (key == "tpid" && ch.data.count(e2db::SDATA::c))
			val = getPIDValue(ch, e2db::SDATA_PIDS::tpid);
		else if (key == "pcrpid" && ch.data.count(e2db::SDATA::c))
			val = getPIDValue(ch, e2db::SDATA_PIDS::pcrpid);
		else if (key == "ac3pid" && ch.data.count(e2db::SDATA::c))
			val = getPIDValue(ch, e2db::SDATA_PIDS::ac3pid);
		else if (key == "vtype" && ch.data.count(e2db::SDATA::c))
			val = getPIDValue(ch, e2db::SDATA_PIDS::vtype);
		else if (key == "achannel" && ch.data.count(e2db::SDATA::c))
			val = getPIDValue(ch, e2db::SDATA_PIDS::achannel);
		else if (key == "ac3delay" && ch.data.count(e2db::SDATA::c))
			val = getPIDValue(ch, e2db::SDATA_PIDS::ac3delay);
		else if (key == "pcmdelay" && ch.data.count(e2db::SDATA::c))
			val = getPIDValue(ch, e2db::SDATA_PIDS::pcmdelay);
		else if (key == "subtitle" && ch.data.count(e2db::SDATA::c))
			val = getPIDValue(ch, e2db::SDATA_PIDS::subtitle);
		else if (key == "atype" && ch.data.count(e2db::SDATA::c))
			val = getPIDValue(ch, e2db::SDATA_PIDS::atype);
		else if (key == "apid" && ch.data.count(e2db::SDATA::c))
			val = getPIDValue(ch, e2db::SDATA_PIDS::apid);
		else if (key == "cmax" && ch.data.count(e2db::SDATA::c))
			val = getPIDValue(ch, e2db::SDATA_PIDS::cmax);
		else if (key == "pmt" && ch.data.count(e2db::SDATA::c))
			val = getPIDValue(ch, e2db::SDATA_PIDS::pmt);
		else if (key == "raw_C" && ch.data.count(e2db::SDATA::C))
		{
			if (ch.data.count(e2db::SDATA::C) && ! ch.data[e2db::SDATA::C].empty())
			{
				auto z = std::prev(ch.data[e2db::SDATA::C].cend());
				for (string & w : ch.data[e2db::SDATA::C])
				{
					val += "C:" + w;
					if (w != (*z))
						val += ',';
				}
			}
			this->state.raw_C = val;
		}
		else if (key == "fkeep" && ch.data.count(e2db::SDATA::f))
			val = getFlagValue(ch, e2db::SDATA_FLAGS::fkeep);
		else if (key == "fhide" && ch.data.count(e2db::SDATA::f))
			val = getFlagValue(ch, e2db::SDATA_FLAGS::fhide);
		else if (key == "fpid" && ch.data.count(e2db::SDATA::f))
			val = getFlagValue(ch, e2db::SDATA_FLAGS::fpid);
		else if (key == "fname" && ch.data.count(e2db::SDATA::f))
			val = getFlagValue(ch, e2db::SDATA_FLAGS::fname);
		else if (key == "fnew" && ch.data.count(e2db::SDATA::f))
			val = getFlagValue(ch, e2db::SDATA_FLAGS::fnew);
		else if (key == "raw_data")
		{
			if (! ch.data.empty())
			{
				auto z = std::prev(ch.data.cend());
				for (auto & q : ch.data)
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
					for (size_t i = 0; i < q.second.size(); i++)
					{
						val += d;
						val += ':' + q.second[i];
						if (! q.second[i].empty() && (i != q.second.size() - 1 || q.first != (*z).first))
							val += ',';
					}
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
			int index = field->findData(QString::fromStdString(val), Qt::UserRole);
			field->setCurrentIndex(index);
		}
		else if (QCheckBox* field = qobject_cast<QCheckBox*>(item))
		{
			field->setChecked(!! std::atoi(val.data()));
		}
	}
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
	vector<string> data = ch.data[e2db::SDATA::c];
	string cpx = (x > 9 ? "" : "0") + to_string(x);
	int cval = std::atoi(val.data());

	char pid[7];
	std::snprintf(pid, 7, "%2s%04x", cpx.c_str(), cval);

	for (auto it = data.begin(); it != data.end(); it++)
	{
		if ((*it).substr(0, 2) == cpx)
		{
			if (val.empty())
				*it = "";
			else
				*it = pid;
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
	long flags = std::strtol(ch.data[e2db::SDATA::f][0].data(), NULL, 16);

	if (flags & x)
		return "1";
	return "";
}

vector<string> editService::computeFlags(e2db::service ch, e2db::SDATA_FLAGS x, string val)
{
	vector<string> data = ch.data[e2db::SDATA::f];
	long flags = data.empty() ? 0 : std::strtol(data[0].data(), NULL, 16);

	if (flags & x)
		flags -= x;
	if (val == "1")
		flags += x;

	char cflags[3];
	std::snprintf(cflags, 3, "%02x", int (flags));

	data.clear();

	if (flags)
		data.emplace_back(cflags);

	return data;
}

void editService::setEditId(string chid)
{
	debug("setEditId");

	this->state.edit = true;
	this->chid = chid;
}

void editService::setEditId(string chid, string bname, bool stream)
{
	debug("setEditId");

	this->state.edit = true;
	this->chid = chid;
	this->bname = bname;
	this->state.is_fav = ! bname.empty();
	this->state.is_stream = stream;
}

string editService::getEditId()
{
	debug("getEditId");

	return this->chid;
}

void editService::setAddId(string bname)
{
	debug("setAddId");

	this->bname = bname;
	this->state.is_fav = ! bname.empty();
}

string editService::getTransponderId()
{
	debug("getTransponderId");

	return this->txid;
}

bool editService::isFavourite()
{
	return this->state.is_fav;
}

bool editService::isStream()
{
	return this->state.is_stream;
}

bool editService::getReload()
{
	return this->state.transponder && ! this->txid.empty();
}

bool editService::destroy()
{
	debug("destroy");

	if (this->edittxp != nullptr)
		delete this->edittxp;
	if (this->editfav != nullptr)
		delete this->editfav;
	if (this->dial != nullptr)
		delete this->dial;
	if (this->theme != nullptr)
		delete this->theme;

	bool _changes = this->changes;

	delete this;

	return ! _changes;
}

}
