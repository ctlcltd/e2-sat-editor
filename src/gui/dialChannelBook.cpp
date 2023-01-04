/*!
 * e2-sat-editor/src/gui/dialChannelBook.cpp
 *
 * @link https://github.com/ctlcltd/e2-sat-editor
 * @copyright e2 SAT Editor Team
 * @author Leonardo Laureti
 * @version 0.1
 * @license MIT License
 * @license GNU GPLv3 License
 */

#include <QWidget>

#include "dialChannelBook.h"
#include "theme.h"

using namespace e2se;

namespace e2se_gui
{

dialChannelBook::dialChannelBook(dataHandler* data, int stype, e2se::logger::session* log)
{
	this->log = new logger(log, "dialChannelBook");
	debug("dialChannelBook()");

	this->data = data;
	this->stype = stype;

	this->frameMargins = QMargins (0, 0, 0, 0);
	this->frameFixed = false;
}

void dialChannelBook::display(QWidget* cwid)
{
	debug("display()");

	this->dbih = this->data->dbih;

	layout(cwid);

	dial->open();
}

void dialChannelBook::layout(QWidget* cwid)
{
	this->dialAbstract::layout(cwid);

	dial->setParent(nullptr);
	dial->setWindowFlags(Qt::Dialog);
	dial->setMinimumSize(760, 420);
	dial->setWindowTitle("Add Channel");
	dial->connect(dial, &QDialog::finished, [=]() { delete cbv; delete dial; });

	this->cbv = new e2se_gui::channelBookView(this->data, this->stype, this->log->log);

	dtform->setContentsMargins(0, 0, 0, 0);
	dtform->addWidget(cbv->widget, 1, 0);

	this->cbv->load();
}

void dialChannelBook::toolbar()
{
	debug("toolbar()");

	QWidget* dtspacer = new QWidget;
	dtspacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

	this->action.filter = new QCheckBox(tr("Filters for service type"));
	this->action.filter->setChecked(true);
	this->action.filter->connect(this->action.filter, &QCheckBox::stateChanged, [=](int checked) { this->cbv->filterChanged(checked); });
	dtbar->addWidget(this->action.filter);

	dtbar->addWidget(dtspacer);
	
	//TODO default focused
	this->action.add = dtbar->addAction(theme::icon("add"), tr("Add"), [=]() { this->sender(); });
}

void dialChannelBook::sender()
{
	debug("sender()");

	callEventCallback(cbv->getSelected());
}

}
