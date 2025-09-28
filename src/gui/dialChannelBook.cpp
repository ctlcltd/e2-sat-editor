/*!
 * e2-sat-editor/src/gui/dialChannelBook.cpp
 *
 * @link https://github.com/ctlcltd/e2-sat-editor
 * @copyright e2 SAT Editor Team
 * @author Leonardo Laureti
 * @version 1.8.1
 * @license MIT License
 * @license GNU GPLv3 License
 */

#include <QtGlobal>
#include <QWidget>
#include <QTimer>

#include "dialChannelBook.h"
#include "theme.h"

using namespace e2se;

namespace e2se_gui
{

dialChannelBook::dialChannelBook(dataHandler* data, int stype)
{
	this->log = new logger("gui", "dialChannelBook");

	this->data = data;
	this->stype = stype;

	this->collapsible = true;

	this->frameMargins = QMargins (0, 0, 0, 0);
	this->frameFixed = false;
}

dialChannelBook::~dialChannelBook()
{
	debug("~dialChannelBook");

	delete this->log;
}

void dialChannelBook::display(QWidget* cwid)
{
	debug("display");

	layout(cwid);

	dial->show();
}

void dialChannelBook::layout(QWidget* cwid)
{
	this->dialAbstract::layout(cwid);

	dial->setObjectName("dialchbook");
	dial->setWindowFlag(Qt::WindowStaysOnTopHint);
	dial->setAttribute(Qt::WA_MacAlwaysShowToolWindow);

	dial->setMinimumSize(700, 400);
	dial->setWindowTitle(tr("Add Channel", "dialog"));

#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
	dial->connect(dial, &QDialog::finished, [=]() { QTimer::singleShot(0, [=]() { this->destroy(); }); });
#else
	dial->connect(dial, &QDialog::finished, [=]() { this->destroy(); });
#endif

	this->cbv = new e2se_gui::channelBookView(this->data, this->stype);

	dtform->setContentsMargins(0, 0, 0, 0);
	dtform->addWidget(cbv->widget, 1, 0);

	this->cbv->load();
}

void dialChannelBook::toolbarLayout()
{
	debug("toolbarLayout");

	this->dtbar = toolBar();
	toolBarStyleSheet();

	this->action.filter = new QCheckBox(tr("Filters for service type", "dialog"));
	this->action.filter->setChecked(true);
#if QT_VERSION >= QT_VERSION_CHECK(6, 7, 0)
	this->action.filter->connect(this->action.filter, &QCheckBox::checkStateChanged, [=](Qt::CheckState check) {
#else
	this->action.filter->connect(this->action.filter, &QCheckBox::stateChanged, [=](int check) {
#endif
		this->cbv->filterChanged(check != 0);
	});

	toolBarWidget(dtbar, tr("Filters for service type", "dialog"), this->action.filter);
	toolBarSpacer(dtbar);
	this->action.cancel = toolBarAction(dtbar, tr("Cancel", "dialog"), [=]() { this->cancel(); });
	toolBarSeparator(dtbar);
	this->action.submit = toolBarAction(dtbar, tr("Add", "dialog"), theme->dynamicIcon("add"), [=]() { this->submit(); });

	dtbar->widgetForAction(this->action.submit)->setFocus();
}

void dialChannelBook::submit()
{
	debug("submit");

	if (this->cbv != nullptr)
		callEventCallback(this->cbv->getSelected());
}

void dialChannelBook::load()
{
	debug("load");

	if (this->cbv != nullptr)
		this->cbv->load();
}

void dialChannelBook::reset()
{
	debug("reset");

	if (this->cbv != nullptr)
		this->cbv->reset();
}

void dialChannelBook::reload()
{
	debug("reload");

	if (this->cbv != nullptr)
		this->cbv->reload();
}

bool dialChannelBook::destroy()
{
	debug("destroy");

	if (this->cbv != nullptr)
	{
		delete this->cbv;
		this->cbv = nullptr;
	}

	if (this->dial != nullptr)
	{
		delete this->dial;
		this->dial = nullptr;
	}
	if (this->theme != nullptr)
	{
		delete this->theme;
		this->theme = nullptr;
	}

	return true;
}

}
