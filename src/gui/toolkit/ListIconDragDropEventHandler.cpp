/*!
 * e2-sat-editor/src/gui/toolkit/ListIconDragDropEventHandler.cpp
 *
 * @link https://github.com/ctlcltd/e2-sat-editor
 * @copyright e2 SAT Editor Team
 * @author Leonardo Laureti
 * @version 0.7
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

	// qDebug() << "closest event: " << event;

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

	// qDebug() << "closest drag: " << e;
	// qDebug() << " enter urls: " << e->mimeData()->hasUrls();
	// qDebug() << " enter image: " << e->mimeData()->hasImage();

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

	// qDebug() << "closest move: " << e;
	// qDebug() << " move urls: " << e->mimeData()->hasUrls();
	// qDebug() << " move image: " << e->mimeData()->hasImage();

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

	// qDebug() << "closest leave: " << e;

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

	// qDebug() << "closest drop: " << e;

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

		// qDebug() << " drop urls: " << e->mimeData()->urls();
		// qDebug() << " drop image: " << e->mimeData()->imageData();
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
		qDebug() << "list enter: " << e;
		e->setDropAction(Qt::DropAction::IgnoreAction);
		e->ignore();
		return QObject::eventFilter(object, e);
	}
	else if (event->type() == QEvent::DragMove)
	{
		QDragMoveEvent* e = static_cast<QDragMoveEvent*>(event);
		qDebug() << "list move: " << e;
		e->setDropAction(Qt::DropAction::IgnoreAction);
		e->ignore();
		return QObject::eventFilter(object, e);
	}
	else if (event->type() == QEvent::DragLeave)
	{
		qDebug() << "list leave: " << event;
	}
	else if (event->type() == QEvent::Drop)
	{
		QDropEvent* e = static_cast<QDropEvent*>(event);
		qDebug() << "list drop: " << e;
		e->setDropAction(Qt::DropAction::IgnoreAction);
		e->ignore();
		return QObject::eventFilter(object, e);
	}

	// qDebug() << "list event: " << event;

	return QObject::eventFilter(object, event);
}

}
