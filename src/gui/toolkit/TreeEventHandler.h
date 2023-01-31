/*!
 * e2-sat-editor/src/gui/toolkit/TreeEventHandler.h
 *
 * @link https://github.com/ctlcltd/e2-sat-editor
 * @copyright e2 SAT Editor Team
 * @author Leonardo Laureti
 * @version 0.3
 * @license MIT License
 * @license GNU GPLv3 License
 */

#include <functional>

#ifndef TreeEventHandler_h
#define TreeEventHandler_h
#include <QObject>
#include <QEvent>
#include <QList>
#include <QTreeWidget>

namespace e2se_gui
{
//TODO persistent glitches with event reject, force a repaint on bouquets tree or his viewport
class TreeEventHandler : public QObject
{
	public:
		void setEventCallback(std::function<void(QTreeWidget* tw, QTreeWidgetItem* current)> func)
		{
			this->eventCallback = func;
		}

	protected:
		bool eventFilter(QObject* o, QEvent* e);
		void callEventCallback(QTreeWidget* tw, QTreeWidgetItem* current)
		{
			if (this->eventCallback != nullptr)
				this->eventCallback(tw, current);
		}

	private:
		std::function<void(QTreeWidget* tw, QTreeWidgetItem* current)> eventCallback;
};
}
#endif /* TreeEventHandler_h */
