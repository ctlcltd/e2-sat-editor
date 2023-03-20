/*!
 * e2-sat-editor/src/gui/toolkit/ListEventHandler.cpp
 *
 * @link https://github.com/ctlcltd/e2-sat-editor
 * @copyright e2 SAT Editor Team
 * @author Leonardo Laureti
 * @version 0.5
 * @license MIT License
 * @license GNU GPLv3 License
 */

#include <QDropEvent>
#include <QTreeWidget>

#include "ListEventHandler.h"

namespace e2se_gui
{

bool ListEventHandler::eventFilter(QObject* object, QEvent* event)
{
	if (event->type() == QEvent::Drop)
	{
		QDropEvent* e = static_cast<QDropEvent*>(event);

		if (this->dnd)
		{
			QTreeWidget* list = qobject_cast<QTreeWidget*>(object->parent());
			callEventCallback(list);
		}
		else
		{
			e->setDropAction(Qt::DropAction::IgnoreAction);
		}

		return QObject::eventFilter(object, e);
	}

	return QObject::eventFilter(object, event);
}

}
