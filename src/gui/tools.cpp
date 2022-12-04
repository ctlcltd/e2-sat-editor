/*!
 * e2-sat-editor/src/gui/tools.cpp
 *
 * @link https://github.com/ctlcltd/e2-sat-editor
 * @copyright e2 SAT Editor Team
 * @author Leonardo Laureti
 * @version 0.1
 * @license MIT License
 * @license GNU GPLv3 License
 */

#include <QDialog>
#include <QTextEdit>

#include "tools.h"

using namespace e2se;

namespace e2se_gui_tools
{

tools::tools(QGridLayout* root, e2se::logger::session* log)
{
	this->log = new logger(log, "tools");
	debug("tools()");

	this->root = root;
}

void tools::inspector()
{
	QDialog* dial = new QDialog;
	dial->setWindowTitle("Inspector Log");
	dial->setMinimumSize(450, 520);
	dial->connect(dial, &QDialog::finished, [=]() { delete dial; });

	QGridLayout* dfrm = new QGridLayout(dial);
	QTextEdit* dcnt = new QTextEdit;
	dcnt->setReadOnly(true);
	dcnt->setHtml("<pre style=\"font-size: 11px\">" + QString::fromStdString(this->log->str()).toHtmlEscaped() + "</pre>");

	dfrm->setContentsMargins(0, 0, 0, 0);
	dfrm->addWidget(dcnt);
	dial->setLayout(dfrm);
	dial->exec();
}

void tools::editTunersets(e2se_gui::e2db* dbih, int ytype)
{
	debug("editTunersets()", "ytype", ytype);

	this->tns = new e2se_gui::editTunersets(dbih, ytype, this->log->log);
	root->addWidget(tns->widget, 1, 0);
}

void tools::closeTunersets()
{
	debug("closeTunersets()");

	if (this->tns)
	{
		tns->widget->hide();
		root->removeWidget(tns->widget);
		delete this->tns;
		this->tns = nullptr;
	}
}

void tools::destroy()
{
	debug("destroy()");

	if (this->tns)
	{
		if (this->tns->widget && this->tns->widget->isVisible())
			closeTunersets();
		else
			delete this->tns;
	}
}

}
