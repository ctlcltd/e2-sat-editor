/*!
 * e2-sat-editor/src/gui/DropEventHandler.h
 *
 * @link https://github.com/ctlcltd/e2-sat-editor
 * @copyright e2 SAT Editor Team
 * @author Leonardo Laureti
 * @version 0.1
 * @license MIT License
 * @license GNU GPLv3 License
 */

#include <functional>

#ifndef DropEventHandler_h
#define DropEventHandler_h
#include <QEvent>
#include <QList>
#include <QTreeWidget>
#include <QDropEvent>

namespace e2se_gui
{
//TODO optional switch to current (drop) bouquets tree item in settings
//TODO FIX persistent glitches with event reject, force a repaint on bouquets tree or his viewport
class DropEventHandler : public QObject
{
	public:
		void setEventCallback(std::function<void()> func)
		{
			this->eventCallback = func;
		}
	protected:
		bool eventFilter(QObject* o, QEvent* e)
		{
			if (e->type() == QEvent::Drop)
			{
				QDropEvent* evt = static_cast<QDropEvent*>(e);
				QTreeWidget* tree0 = qobject_cast<QTreeWidget*>(o->parent());
				QTreeWidget* tree1 = qobject_cast<QTreeWidget*>(evt->source());
				QPoint pos = evt->position().toPoint();

				QTreeWidgetItem* current = tree0->itemAt(pos);
				int ti = tree0->indexOfTopLevelItem(current);

				evt->setDropAction(Qt::DropAction::IgnoreAction);

				if (current == nullptr || ti != -1)
				{
				}
				else if (tree0 == tree1)
				{
					dropFromTree(current, tree0);
				}
				else
				{
					dropFromList(current, tree0, tree1);
				}

				return QObject::eventFilter(o, evt);
			}

			return QObject::eventFilter(o, e);
		}
		void dropFromTree(QTreeWidgetItem* current, QTreeWidget* tree0)
		{
			QTreeWidgetItem* parent = current->parent();
			QList<QTreeWidgetItem*> selected = tree0->selectedItems();

			if (parent == nullptr || selected.empty())
				return;

			QList<QTreeWidgetItem*> caches;

			for (auto & item : selected)
			{
				caches.append(item->clone());
				delete item;
			}
			parent->insertChildren(parent->indexOfChild(current), caches);
		}
		void dropFromList(QTreeWidgetItem* current, QTreeWidget* tree0, QTreeWidget* tree1)
		{
			QList<QTreeWidgetItem*> selected = tree1->selectedItems();

			if (selected.empty())
				return;

			QList<QTreeWidgetItem*> caches;

			for (auto & item : selected)
				caches.append(item->clone());

			tree0->setCurrentItem(current);
			tree1->addTopLevelItems(caches);

			eventCallback();
		}
		void callEventCallback()
		{
			this->eventCallback();
		}
	private:
		std::function<void()> eventCallback;
};
}
#endif /* DropEventHandler_h */
