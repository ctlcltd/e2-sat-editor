/*!
 * e2-sat-editor/src/gui/toolkit/TreeDragDropEventHandler.h
 *
 * @link https://github.com/ctlcltd/e2-sat-editor
 * @copyright e2 SAT Editor Team
 * @author Leonardo Laureti
 * @version 0.5
 * @license MIT License
 * @license GNU GPLv3 License
 */

#include <functional>

#ifndef TreeDragDropEventHandler_h
#define TreeDragDropEventHandler_h
#include <QObject>
#include <QEvent>

#include "TreeDropIndicatorEventPainter.h"

namespace e2se_gui
{
class TreeDragDropEventHandler : public TreeDropIndicatorEventPainter
{
	public:
		void setEventCallback(std::function<void(QTreeWidget* tw)> func)
		{
			this->eventCallback = func;
		}
		void disallowInternalMove()
		{
			this->dnd = false;
		}
		void allowInternalMove()
		{
			this->dnd = true;
		}

	protected:
		bool eventFilter(QObject* object, QEvent* event);
		bool eventDragMove(QObject* object, QEvent* event);
		bool eventDrop(QObject* object, QEvent* event);
		void callEventCallback(QTreeWidget* tw)
		{
			if (this->eventCallback != nullptr)
				this->eventCallback(tw);
		}

	private:
		std::function<void(QTreeWidget* tw)> eventCallback;
		bool dnd = true;
};
}
#endif /* TreeDragDropEventHandler_h */
