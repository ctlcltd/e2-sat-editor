/*!
 * e2-sat-editor/src/gui/editTunersetsTable.cpp
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

#include "editTunersetsTable.h"

using namespace e2se;

namespace e2se_gui
{

editTunersetsTable::editTunersetsTable(e2db* dbih, e2se::logger::session* log)
{
	this->log = new logger(log, "editTunersetsTable");
	debug("editTunersetsTable()");

	this->dbih = dbih;
}

void editTunersetsTable::display(QWidget* cwid)
{
	layout();

	if (this->state.edit)
		retrieve();

	this->dialAbstract::display(cwid);
}

void editTunersetsTable::layout()
{
	this->dialAbstract::layout();
}

void editTunersetsTable::store()
{
	debug("store()");
}

void editTunersetsTable::retrieve()
{
	debug("retrieve()");
}

void editTunersetsTable::setEditID(string todo)
{
	debug("setEditID()");

	this->state.edit = true;
}

string editTunersetsTable::getEditID()
{
	debug("getEditID()");

	return "";
}

}
