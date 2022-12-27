/*!
 * e2-sat-editor/src/gui/toolkit/TreeEventHandler.h
 *
 * @link https://github.com/ctlcltd/e2-sat-editor
 * @copyright e2 SAT Editor Team
 * @author Leonardo Laureti
 * @version 0.1
 * @license MIT License
 * @license GNU GPLv3 License
 */

#include <functional>

#ifndef TreeEventHandler_h
#define TreeEventHandler_h
#include <QObject>
#include <QEvent>
#include <QTreeWidget>

namespace e2se_gui
{
//TODO optional switch to current "drop" bouquets tree item in settings
//TODO FIX persistent glitches with event reject, force a repaint on bouquets tree or his viewport
class TreeEventHandler : public QObject
{
	public:
		void setEventCallback(std::function<void(QTreeWidget* tw)> func)
		{
			this->eventCallback = func;
		}
	protected:
		bool eventFilter(QObject* o, QEvent* e);
		void dropFromTree(QTreeWidgetItem* current, QTreeWidget* tree);
		void dropFromList(QTreeWidgetItem* current, QTreeWidget* tree, QTreeWidget* list);
		void callEventCallback(QTreeWidget* tw)
		{
			if (this->eventCallback != nullptr)
				this->eventCallback(tw);
		}
	private:
		std::function<void(QTreeWidget* tw)> eventCallback;
};
}
#endif /* TreeEventHandler_h */
