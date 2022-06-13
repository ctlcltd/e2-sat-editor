/*!
 * e2-sat-editor/src/gui/toolkit/ListEventObserver.cpp
 *
 * @link https://github.com/ctlcltd/e2-sat-editor
 * @copyright e2 SAT Editor Team
 * @author Leonardo Laureti
 * @version 0.1
 * @license MIT License
 * @license GNU GPLv3 License
 */

#include "ListEventObserver.h"

namespace e2se_gui
{

bool ListEventObserver::eventFilter(QObject* o, QEvent* e)
{
	if (e->type() == QEvent::ChildRemoved)
		this->changed = this->changed ? false : true;
	else if (e->type() == QEvent::ChildAdded)
		this->changed = this->changed ? false : true;

	return false;
}

}
