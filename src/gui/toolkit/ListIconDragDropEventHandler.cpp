/*!
 * e2-sat-editor/src/gui/toolkit/ListIconDragDropEventHandler.cpp
 *
 * @link https://github.com/ctlcltd/e2-sat-editor
 * @copyright e2 SAT Editor Team
 * @author Leonardo Laureti
 * @version 1.8.0
 * @license MIT License
 * @license GNU GPLv3 License
 */

#include <Qt>
#include <QtGlobal>
#include <QDragEnterEvent>
#include <QDragMoveEvent>
#include <QDropEvent>
#include <QMimeData>

#include "ListIconDragDropEventHandler.h"

namespace e2se_gui
{

bool ListIconDragDropEventHandler::eventFilter(QObject* object, QEvent* event)
{
	if (event->type() == QEvent::DragEnter)
		return eventDragEnter(object, event);
	else if (event->type() == QEvent::DragMove)
		return eventDragMove(object, event);
	else if (event->type() == QEvent::DragLeave)
		return eventDragLeave(object, event);
	else if (event->type() == QEvent::Drop)
		return eventDrop(object, event);

	return QObject::eventFilter(object, event);
}

bool ListIconDragDropEventHandler::eventDragEnter(QObject* object, QEvent* event)
{
	QDragEnterEvent* e = static_cast<QDragEnterEvent*>(event);

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
	QPoint pos = e->position().toPoint();
#else
	QPoint pos = e->pos();
#endif

	if (e->mimeData()->hasUrls())
	{
		raiseWindow();

		QModelIndex index = list->indexAt(pos);

		if (index.isValid())
		{
			list->setCurrentIndex(index);
		}

		e->acceptProposedAction();
		e->accept();
	}
	else
	{
		e->ignore();
	}

	return QObject::eventFilter(object, e);
}

bool ListIconDragDropEventHandler::eventDragMove(QObject* object, QEvent* event)
{
	QDragMoveEvent* e = static_cast<QDragMoveEvent*>(event);

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
	QPoint pos = e->position().toPoint();
#else
	QPoint pos = e->pos();
#endif

	if (e->mimeData()->hasUrls())
	{
		raiseWindow();

		QModelIndex index = list->indexAt(pos);

		if (index.isValid())
		{
			list->setCurrentIndex(index);
		}

		e->acceptProposedAction();
		e->accept();
	}
	else if (e->mimeData()->formats().contains("application/x-qt-mime-type-name"))
	{
	}
	else
	{
		e->ignore();
	}

	return QObject::eventFilter(object, e);
}

bool ListIconDragDropEventHandler::eventDragLeave(QObject* object, QEvent* event)
{
	QDragLeaveEvent* e = static_cast<QDragLeaveEvent*>(event);

	//TODO list deselect

	return QObject::eventFilter(object, e);
}

bool ListIconDragDropEventHandler::eventDrop(QObject* object, QEvent* event)
{
	QDropEvent* e = static_cast<QDropEvent*>(event);

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
	QPoint pos = e->position().toPoint();
#else
	QPoint pos = e->pos();
#endif

	if (e->mimeData()->hasUrls())
	{
		e->setDropAction(Qt::CopyAction);
		e->accept();

		QModelIndex index = list->indexAt(pos);

		if (index.isValid())
		{
			QListWidgetItem* item = list->itemAt(pos);
			callEventCallback(item, e->mimeData()->urls().first().toLocalFile());
			list->setCurrentIndex(index);
		}
	}
	else
	{
		e->setDropAction(Qt::DropAction::IgnoreAction);
	}

	return QObject::eventFilter(object, e);
}

void ListIconDragDropEventHandler::raiseWindow()
{
	list->raise();
	list->window()->activateWindow();
	list->window()->raise();
}


bool ListIconDragDropEventFilter::eventFilter(QObject* object, QEvent* event)
{
	if (event->type() == QEvent::DragEnter)
	{
		QDragEnterEvent* e = static_cast<QDragEnterEvent*>(event);
		e->setDropAction(Qt::DropAction::IgnoreAction);
		e->ignore();
		return QObject::eventFilter(object, e);
	}
	else if (event->type() == QEvent::DragMove)
	{
		QDragMoveEvent* e = static_cast<QDragMoveEvent*>(event);
		e->setDropAction(Qt::DropAction::IgnoreAction);
		e->ignore();
		return QObject::eventFilter(object, e);
	}
	else if (event->type() == QEvent::DragLeave)
	{
	}
	else if (event->type() == QEvent::Drop)
	{
		QDropEvent* e = static_cast<QDropEvent*>(event);
		e->setDropAction(Qt::DropAction::IgnoreAction);
		e->ignore();
		return QObject::eventFilter(object, e);
	}

	return QObject::eventFilter(object, event);
}

}
