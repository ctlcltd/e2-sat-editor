/*!
 * e2-sat-editor/src/gui/editTunersets.cpp
 *
 * @link https://github.com/ctlcltd/e2-sat-editor
 * @copyright e2 SAT Editor Team
 * @author Leonardo Laureti
 * @version 0.1
 * @license MIT License
 * @license GNU GPLv3 License
 */

#include <QtGlobal>
#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QComboBox>

#include "editTunersets.h"

using namespace e2se;

namespace e2se_gui
{

editTunersets::editTunersets(e2db* dbih, int ty, e2se::logger::session* log)
{
	this->log = new logger(log, "editTunersets");
	debug("editTunersets()");

	this->state.ty = ty;
	this->dbih = dbih;
}

void editTunersets::display(QWidget* cwid)
{
	layout();

	if (this->state.edit)
		retrieve();

	this->dialAbstract::display(cwid);
}

void editTunersets::layout()
{
	this->dialAbstract::layout();
}

void editTunersets::store()
{
	debug("store()");
}

void editTunersets::retrieve()
{
	debug("retrieve()");
}

void editTunersets::setEditID(string todo)
{
	debug("setEditID()");

	this->state.edit = true;
}

string editTunersets::getEditID()
{
	debug("getEditID()");

	return "";
}

}
