/*!
 * e2-sat-editor/src/gui/toolkit/TreeEventHandler.cpp
 *
 * @link https://github.com/ctlcltd/e2-sat-editor
 * @copyright e2 SAT Editor Team
 * @author Leonardo Laureti
 * @version 0.2
 * @license MIT License
 * @license GNU GPLv3 License
 */

#include <Qt>
#include <QtGlobal>
#include <QDropEvent>
#include <QList>

#include "TreeEventHandler.h"

namespace e2se_gui
{

bool TreeEventHandler::eventFilter(QObject* o, QEvent* e)
{
	if (e->type() == QEvent::Drop)
	{
		QDropEvent* evt = static_cast<QDropEvent*>(e);
		QTreeWidget* tree = qobject_cast<QTreeWidget*>(o->parent());
		QTreeWidget* list = qobject_cast<QTreeWidget*>(evt->source());
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
		QPoint pos = evt->position().toPoint();
#else
		QPoint pos = evt->pos();
#endif

		QTreeWidgetItem* current = tree->itemAt(pos);
		int ti = tree->indexOfTopLevelItem(current);

		evt->setDropAction(Qt::DropAction::IgnoreAction);

		// discard drop
		// all | tv | radio
		if (current == nullptr || ti != -1)
		{
		}
		// drop from tree
		// userbouquets
		else if (tree == list)
		{
			dropFromTree(current, tree);
		}
		// drop from list
		// userbouquets
		else
		{
			dropFromList(current, tree, tree);
		}

		return QObject::eventFilter(o, evt);
	}

	return QObject::eventFilter(o, e);
}

void TreeEventHandler::dropFromTree(QTreeWidgetItem* current, QTreeWidget* tree)
{
	QTreeWidgetItem* parent = current->parent();
	QList<QTreeWidgetItem*> selected = tree->selectedItems();

	if (parent == nullptr || selected.empty())
		return;

	QList<QTreeWidgetItem*> items;

	for (auto & item : selected)
	{
		if (item->parent() != parent)
			return;

		items.append(item->clone());
	}
	int index = parent->indexOfChild(current);
	for (auto & item : selected)
		parent->removeChild(item);

	parent->insertChildren(index, items);
	tree->setCurrentItem(items.first());

	callEventCallback(tree);
}

void TreeEventHandler::dropFromList(QTreeWidgetItem* current, QTreeWidget* tree, QTreeWidget* list)
{
	QList<QTreeWidgetItem*> selected = list->selectedItems();

	if (selected.empty())
		return;

	QList<QTreeWidgetItem*> items;

	for (auto & item : selected)
		items.append(item->clone());

	tree->setCurrentItem(current);
	list->addTopLevelItems(items);
	list->scrollToBottom();

	callEventCallback(list);
}

}
