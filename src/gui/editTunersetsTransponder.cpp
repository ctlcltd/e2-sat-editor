/*!
 * e2-sat-editor/src/gui/editTunersetsTransponder.cpp
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

#include "editTunersetsTransponder.h"

using namespace e2se;

namespace e2se_gui
{

editTunersetsTransponder::editTunersetsTransponder(e2db* dbih, e2se::logger::session* log)
{
	this->log = new logger(log, "editTunersetsTransponder");
	debug("editTunersetsTransponder()");

	this->dbih = dbih;
}

void editTunersetsTransponder::display(QWidget* cwid)
{
	layout();

	if (this->state.edit)
		retrieve();

	this->dialAbstract::display(cwid);
}

void editTunersetsTransponder::layout()
{
	this->dialAbstract::layout();
}

void editTunersetsTransponder::store()
{
	debug("store()");
}

void editTunersetsTransponder::retrieve()
{
	debug("retrieve()");
}

void editTunersetsTransponder::setEditID(string todo)
{
	debug("setEditID()");

	this->state.edit = true;
}

string editTunersetsTransponder::getEditID()
{
	debug("getEditID()");

	return "";
}

}
