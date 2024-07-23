/*!
 * e2-sat-editor/src/gui/toolkit/InputControlEditEventObserver.cpp
 *
 * @link https://github.com/ctlcltd/e2-sat-editor
 * @copyright e2 SAT Editor Team
 * @author Leonardo Laureti
 * @version 1.6.0
 * @license MIT License
 * @license GNU GPLv3 License
 */

#include <QClipboard>
#include <QLineEdit>

#include "InputControlEditEventObserver.h"
#include "../tab.h"

namespace e2se_gui
{

bool InputControlEditEventObserver::eventFilter(QObject* object, QEvent* event)
{
	if (event->type() == QEvent::FocusIn || event->type() == QEvent::MouseButtonPress)
		return eventFocusIn(object, event);
	else if (event->type() == QEvent::FocusOut)
		return eventFocusOut(object, event);

	return QObject::eventFilter(object, event);
}

bool InputControlEditEventObserver::eventFocusIn(QObject* object, QEvent* event)
{
	QLineEdit* input = qobject_cast<QLineEdit*>(object);

	if (input && tid != nullptr)
	{
		tid->setFlag(gui::GUI_CXE::EditUndo, ! input->isReadOnly() && input->isUndoAvailable());
		tid->setFlag(gui::GUI_CXE::EditRedo, ! input->isReadOnly() && input->isRedoAvailable());
		tid->setFlag(gui::GUI_CXE::EditDelete, ! input->isReadOnly() && ! input->text().isEmpty() && input->isRedoAvailable());
		tid->setFlag(gui::GUI_CXE::EditSelectAll, ! input->text().isEmpty() && input->selectionLength() != input->text().length());
		tid->setFlag(gui::GUI_CXE::EditCut, ! input->isReadOnly() && input->hasSelectedText());
		tid->setFlag(gui::GUI_CXE::EditCopy, input->hasSelectedText());
		tid->setFlag(gui::GUI_CXE::EditPaste, ! input->isReadOnly() && ! QGuiApplication::clipboard()->text().isEmpty());
	}

	return QObject::eventFilter(object, event);
}

bool InputControlEditEventObserver::eventFocusOut(QObject* object, QEvent* event)
{
	QLineEdit* input = qobject_cast<QLineEdit*>(object);

	if (input && tid != nullptr)
	{
		tid->setFlag(gui::GUI_CXE::EditUndo, false);
		tid->setFlag(gui::GUI_CXE::EditRedo, false);
		tid->setFlag(gui::GUI_CXE::EditDelete, false);
		tid->setFlag(gui::GUI_CXE::EditSelectAll, false);
		tid->setFlag(gui::GUI_CXE::EditCut, false);
		tid->setFlag(gui::GUI_CXE::EditCopy, false);
		tid->setFlag(gui::GUI_CXE::EditPaste, false);
	}

	return QObject::eventFilter(object, event);
}

}
