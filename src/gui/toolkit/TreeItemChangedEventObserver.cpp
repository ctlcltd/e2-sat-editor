/*!
 * e2-sat-editor/src/gui/toolkit/TreeItemChangedEventObserver.cpp
 *
 * @link https://github.com/ctlcltd/e2-sat-editor
 * @copyright e2 SAT Editor Team
 * @author Leonardo Laureti
 * @version 1.3.0
 * @license MIT License
 * @license GNU GPLv3 License
 */

#include "TreeItemChangedEventObserver.h"

namespace e2se_gui
{

bool TreeItemChangedEventObserver::eventFilter(QObject* object, QEvent* event)
{
	if (event->type() == QEvent::ChildRemoved)
		this->changed = this->changed ? false : true;
	else if (event->type() == QEvent::ChildAdded)
		this->changed = this->changed ? false : true;

	return QObject::eventFilter(object, event);
}

}
