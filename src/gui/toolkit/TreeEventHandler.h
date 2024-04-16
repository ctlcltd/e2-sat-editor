/*!
 * e2-sat-editor/src/gui/toolkit/TreeEventHandler.h
 *
 * @link https://github.com/ctlcltd/e2-sat-editor
 * @copyright e2 SAT Editor Team
 * @author Leonardo Laureti
 * @version 1.4.0
 * @license MIT License
 * @license GNU GPLv3 License
 */

#include <functional>

#ifndef TreeEventHandler_h
#define TreeEventHandler_h
#include <QObject>
#include <QEvent>
#include <QTreeWidget>

#include "TreeDropIndicatorEventPainter.h"

namespace e2se_gui
{
class TreeEventHandler : public TreeDropIndicatorEventPainter
{
	public:
		void setEventCallback(std::function<void(QTreeWidget* tw, QTreeWidgetItem* current)> func)
		{
			this->eventCallback = func;
		}

	protected:
		bool eventFilter(QObject* object, QEvent* event);
		bool eventDrop(QObject* object, QEvent* event);
		void callEventCallback(QTreeWidget* tw, QTreeWidgetItem* current)
		{
			if (this->eventCallback)
				this->eventCallback(tw, current);
		}

	private:
		std::function<void(QTreeWidget* tw, QTreeWidgetItem* current)> eventCallback;
};
}
#endif /* TreeEventHandler_h */
