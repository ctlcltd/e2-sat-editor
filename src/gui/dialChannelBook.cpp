/*!
 * e2-sat-editor/src/gui/dialChannelBook.cpp
 *
 * @link https://github.com/ctlcltd/e2-sat-editor
 * @copyright e2 SAT Editor Team
 * @author Leonardo Laureti
 * @version 0.2
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

	this->collapsible = true;

	this->frameMargins = QMargins (0, 0, 0, 0);
	this->frameFixed = false;
}

void dialChannelBook::display(QWidget* cwid)
{
	debug("display()");

	this->dbih = this->data->dbih;

	layout(cwid);

	dial->show();
}

void dialChannelBook::layout(QWidget* cwid)
{
	this->dialAbstract::layout(cwid);

	dial->setWindowFlag(Qt::WindowStaysOnTopHint);
	dial->setAttribute(Qt::WA_MacAlwaysShowToolWindow);

	dial->setMinimumSize(760, 420);
	dial->setWindowTitle(tr("Add Channel"));
	dial->connect(dial, &QDialog::finished, [=]() { this->destroy(); });

	this->cbv = new e2se_gui::channelBookView(this->data, this->stype, this->log->log);

	dtform->setContentsMargins(0, 0, 0, 0);
	dtform->addWidget(cbv->widget, 1, 0);

	this->cbv->load();
}

void dialChannelBook::toolbarLayout()
{
	debug("toolbarLayout()");

	this->dtbar = toolBar();

	this->action.filter = new QCheckBox(tr("Filters for service type"));
	this->action.filter->setChecked(true);
	this->action.filter->connect(this->action.filter, &QCheckBox::stateChanged, [=](int checked) { this->cbv->filterChanged(checked); });

	toolBarWidget(dtbar, this->action.filter);
	toolBarSpacer(dtbar);
	this->action.cancel = toolBarAction(dtbar, tr("Cancel"), [=]() { this->cancel(); });
	toolBarSeparator(dtbar);
	this->action.add = toolBarAction(dtbar, tr("Add"), theme->dynamicIcon("add"), [=]() { this->sender(); });

	dtbar->widgetForAction(this->action.add)->setFocus();
}

void dialChannelBook::sender()
{
	debug("sender()");

	callEventCallback(cbv->getSelected());
}

void dialChannelBook::destroy()
{
	debug("destroy()");

	delete this->cbv;
	delete this->dial;
	delete this;
}

}
