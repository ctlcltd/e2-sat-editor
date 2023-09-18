/*!
 * e2-sat-editor/src/gui/toolkit/TreeDropIndicatorEventPainter.cpp
 *
 * @link https://github.com/ctlcltd/e2-sat-editor
 * @copyright e2 SAT Editor Team
 * @author Leonardo Laureti
 * @version 0.9
 * @license MIT License
 * @license GNU GPLv3 License
 */

#include <Qt>
#include <QtGlobal>
#include <QDragMoveEvent>
#include <QDropEvent>
#include <QPaintEvent>
#include <QPainter>
#include <QTreeWidget>

#include "TreeDropIndicatorEventPainter.h"

namespace e2se_gui
{

bool TreeDropIndicatorEventPainter::eventFilter(QObject* object, QEvent* event)
{
	if (event->type() == QEvent::DragMove)
		return eventDragMove(object, event);
	else if (event->type() == QEvent::DragLeave)
		return eventDragLeave(object, event);
	else if (event->type() == QEvent::Drop)
		return eventDrop(object, event);
	else if (event->type() == QEvent::Paint)
		return eventPaint(object, event);

	return QObject::eventFilter(object, event);
}

bool TreeDropIndicatorEventPainter::eventDragMove(QObject* object, QEvent* event)
{
	QDragMoveEvent* e = static_cast<QDragMoveEvent*>(event);
	QTreeWidget* tree = qobject_cast<QTreeWidget*>(object->parent());
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
	QPoint pos = e->position().toPoint();
#else
	QPoint pos = e->pos();
#endif
	QModelIndex index = tree->indexAt(pos);

	if (index.isValid())
	{
		// Qt bug Qt::RightToLeft visualItemRect

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
		QRect rect = tree->visualItemRect(tree->itemFromIndex(index)); //Qt5 protected member
#else
		QRect rect = tree->visualItemRect(tree->itemAt(pos));
#endif

		rect.setX(tree->viewport()->pos().x() - 1);
		rect.setWidth(tree->viewport()->width());

		dropIndicatorRect = rect;

		e->acceptProposedAction();
	}

	return QObject::eventFilter(object, e);
}

bool TreeDropIndicatorEventPainter::eventDragLeave(QObject* object, QEvent* event)
{
	dropIndicatorRect = QRect();
	return QObject::eventFilter(object, event);
}

bool TreeDropIndicatorEventPainter::eventDrop(QObject* object, QEvent* event)
{
	dropIndicatorRect = QRect();
	return QObject::eventFilter(object, event);
}

bool TreeDropIndicatorEventPainter::eventPaint(QObject* object, QEvent* event)
{
	QTreeWidget* tree = qobject_cast<QTreeWidget*>(object->parent());

	QPainter painter (tree->viewport());
	painter.drawRect(dropIndicatorRect);

	return QObject::eventFilter(object, event);
}

}
