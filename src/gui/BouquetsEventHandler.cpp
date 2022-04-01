/*!
 * e2-sat-editor/src/gui/BouquetsEventHandler.cpp
 *
 * @link https://github.com/ctlcltd/e2-sat-editor
 * @copyright e2 SAT Editor Team
 * @author Leonardo Laureti
 * @version 0.1
 * @license MIT License
 * @license GNU GPLv3 License
 */

#include <Qt>
#include <QDropEvent>
#include <QList>

#include "BouquetsEventHandler.h"

namespace e2se_gui
{

bool BouquetsEventHandler::eventFilter(QObject* o, QEvent* e)
{
	if (e->type() == QEvent::Drop)
	{
		QDropEvent* evt = static_cast<QDropEvent*>(e);
		QTreeWidget* bouquets_tree = qobject_cast<QTreeWidget*>(o->parent());
		QTreeWidget* list_tree = qobject_cast<QTreeWidget*>(evt->source());
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
		QPoint pos = evt->position().toPoint();
#else
		QPoint pos = evt->pos();
#endif

		QTreeWidgetItem* current = bouquets_tree->itemAt(pos);
		int ti = bouquets_tree->indexOfTopLevelItem(current);

		evt->setDropAction(Qt::DropAction::IgnoreAction);

		// discard drop
		// all | tv | radio
		if (current == nullptr || ti != -1)
		{
		}
		// drop from bouquets tree
		// userbouquets
		else if (bouquets_tree == list_tree)
		{
			dropFromBouquets(current, bouquets_tree);
		}
		// drop from list tree
		// userbouquets
		else
		{
			dropFromList(current, bouquets_tree, list_tree);
		}

		return QObject::eventFilter(o, evt);
	}

	return QObject::eventFilter(o, e);
}

void BouquetsEventHandler::dropFromBouquets(QTreeWidgetItem* current, QTreeWidget* bouquets_tree)
{
	QTreeWidgetItem* parent = current->parent();
	QList<QTreeWidgetItem*> selected = bouquets_tree->selectedItems();

	if (parent == nullptr || selected.empty())
		return;

	QList<QTreeWidgetItem*> caches;

	for (auto & item : selected)
	{
		if (item->parent() != parent)
			return;

		caches.append(item->clone());
	}
	int index = parent->indexOfChild(current);
	for (auto & item : selected)
		delete item;

	parent->insertChildren(index, caches);
	bouquets_tree->setCurrentItem(caches.first());
}

void BouquetsEventHandler::dropFromList(QTreeWidgetItem* current, QTreeWidget* bouquets_tree, QTreeWidget* list_tree)
{
	QList<QTreeWidgetItem*> selected = list_tree->selectedItems();

	if (selected.empty())
		return;

	QList<QTreeWidgetItem*> caches;

	for (auto & item : selected)
		caches.append(item->clone());

	bouquets_tree->setCurrentItem(current);
	list_tree->addTopLevelItems(caches);

	eventCallback();
}

}
