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

#include <Qt>
#include <QtGlobal>
#include <QDropEvent>
#include <QDragMoveEvent>
#include <QPainter>
#include <QTreeWidget>

#include "ListEventHandler.h"

namespace e2se_gui
{

bool ListEventHandler::eventFilter(QObject* object, QEvent* event)
{
	if (event->type() == QEvent::DragMove)
		return eventDragMove(object, event);
	else if (event->type() == QEvent::Drop)
		return eventDrop(object, event);
	else if (event->type() == QEvent::Paint)
		return eventPaint(object, event);

	return QObject::eventFilter(object, event);
}

bool ListEventHandler::eventDragMove(QObject* object, QEvent* event)
{
	QDragMoveEvent* e = static_cast<QDragMoveEvent*>(event);

	if (this->dnd)
	{
		this->TreeDropIndicatorEventPainter::eventDragMove(object, event);
	}
	else
	{
		e->ignore();
	}

	return QObject::eventFilter(object, e);
}

bool ListEventHandler::eventDrop(QObject* object, QEvent* event)
{
	QDropEvent* e = static_cast<QDropEvent*>(event);

	if (this->dnd)
	{
		QTreeWidget* list = qobject_cast<QTreeWidget*>(object->parent());
		this->TreeDropIndicatorEventPainter::eventDrop(object, event);
		callEventCallback(list);
	}
	else
	{
		e->setDropAction(Qt::DropAction::IgnoreAction);
	}

	return QObject::eventFilter(object, e);
}

}
