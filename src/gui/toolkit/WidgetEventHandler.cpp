/*!
 * e2-sat-editor/src/gui/toolkit/WidgetEventHandler.cpp
 *
 * @link https://github.com/ctlcltd/e2-sat-editor
 * @copyright e2 SAT Editor Team
 * @author Leonardo Laureti
 * @version 1.7.0
 * @license MIT License
 * @license GNU GPLv3 License
 */

#include <QWidget>
#include <QDragEnterEvent>
#include <QDragMoveEvent>
#include <QDropEvent>
#include <QMimeData>

#include "WidgetEventHandler.h"

namespace e2se_gui
{

bool WidgetEventHandler::eventFilter(QObject* object, QEvent* event)
{
	if (event->type() == QEvent::ThemeChange)
		return eventThemeChange(object, event);
	//TODO call once
	// else if (event->type() == QEvent::ApplicationPaletteChange)
		// return eventThemeChange(object, event);
	else if (event->type() == QEvent::DragEnter)
		return eventDragEnter(object, event);
	else if (event->type() == QEvent::DragMove)
		return eventDragMove(object, event);
	else if (event->type() == QEvent::DragLeave)
		return eventDragLeave(object, event);
	else if (event->type() == QEvent::Drop)
		return eventDrop(object, event);

	return QObject::eventFilter(object, event);
}

bool WidgetEventHandler::eventThemeChange(QObject* object, QEvent* event)
{
	callEventCallback(CALLEVENT::themeChanged);

	return QObject::eventFilter(object, event);
}

bool WidgetEventHandler::eventDragEnter(QObject* object, QEvent* event)
{
	QDragEnterEvent* e = static_cast<QDragEnterEvent*>(event);

	if (e->mimeData()->hasUrls())
	{
		raiseWindow(object);

		e->acceptProposedAction();
		e->accept();
	}
	else
	{
		e->ignore();
	}

	return QObject::eventFilter(object, e);
}

bool WidgetEventHandler::eventDragMove(QObject* object, QEvent* event)
{
	QDragMoveEvent* e = static_cast<QDragMoveEvent*>(event);

	if (e->mimeData()->hasUrls())
	{
		raiseWindow(object);

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

bool WidgetEventHandler::eventDragLeave(QObject* object, QEvent* event)
{
	QDragLeaveEvent* e = static_cast<QDragLeaveEvent*>(event);

	return QObject::eventFilter(object, e);
}

bool WidgetEventHandler::eventDrop(QObject* object, QEvent* event)
{
	QDropEvent* e = static_cast<QDropEvent*>(event);

	if (e->mimeData()->hasUrls())
	{
		e->setDropAction(Qt::CopyAction);
		e->accept();

		callEventCallback(CALLEVENT::fileDropped, e->mimeData()->urls().first().toLocalFile());
	}
	else
	{
		e->setDropAction(Qt::DropAction::IgnoreAction);
	}

	return QObject::eventFilter(object, e);
}

void WidgetEventHandler::raiseWindow(QObject* object)
{
	if (QWidget* widget = qobject_cast<QWidget*>(object))
	{
		widget->raise();
		widget->window()->activateWindow();
		widget->window()->raise();
	}
}

}
