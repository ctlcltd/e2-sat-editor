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

#include "tools.h"

using std::to_string;
using namespace e2se;

namespace e2se_gui_tools
{

tools::tools(QGridLayout* root)
{
	this->log = new logger("tools");
	debug("tools()");

    this->root = root;
}

void tools::editTunersets(e2se_gui::e2db* dbih, int ytype)
{
    debug("editTunersets()", "ytype", to_string(ytype));

	this->tns = new e2se_gui::editTunersets(dbih, ytype);
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
