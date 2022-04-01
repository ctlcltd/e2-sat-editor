/*!
 * e2-sat-editor/src/gui/BouquetsEventHandler.h
 *
 * @link https://github.com/ctlcltd/e2-sat-editor
 * @copyright e2 SAT Editor Team
 * @author Leonardo Laureti
 * @version 0.1
 * @license MIT License
 * @license GNU GPLv3 License
 */

#include <functional>

#ifndef BouquetsEventHandler_h
#define BouquetsEventHandler_h
#include <QObject>
#include <QEvent>
#include <QTreeWidget>

namespace e2se_gui
{
//TODO optional switch to current (drop) bouquets tree item in settings
//TODO FIX persistent glitches with event reject, force a repaint on bouquets tree or his viewport
class BouquetsEventHandler : public QObject
{
	public:
		void setEventCallback(std::function<void()> func)
		{
			this->eventCallback = func;
		}
	protected:
		bool eventFilter(QObject* o, QEvent* e);
		void dropFromBouquets(QTreeWidgetItem* current, QTreeWidget* tree0);
		void dropFromList(QTreeWidgetItem* current, QTreeWidget* tree0, QTreeWidget* tree1);
		void callEventCallback()
		{
			this->eventCallback();
		}
	private:
		std::function<void()> eventCallback;
};
}
#endif /* BouquetsEventHandler_h */
