/*!
 * e2-sat-editor/src/gui/toolkit/ListEventHandler.cpp
 *
 * @link https://github.com/ctlcltd/e2-sat-editor
 * @copyright e2 SAT Editor Team
 * @author Leonardo Laureti
 * @version 0.3
 * @license MIT License
 * @license GNU GPLv3 License
 */

#include <QDropEvent>
#include <QTreeWidget>

#include "ListEventHandler.h"

namespace e2se_gui
{

bool ListEventHandler::eventFilter(QObject* o, QEvent* e)
{
	if (e->type() == QEvent::Drop)
	{
		QDropEvent* evt = static_cast<QDropEvent*>(e);

		if (this->dnd)
		{
			QTreeWidget* list = qobject_cast<QTreeWidget*>(o->parent());
			callEventCallback(list);
		}
		else
		{
			evt->setDropAction(Qt::DropAction::IgnoreAction);
		}

		return QObject::eventFilter(o, evt);
	}

	return QObject::eventFilter(o, e);
}

}
