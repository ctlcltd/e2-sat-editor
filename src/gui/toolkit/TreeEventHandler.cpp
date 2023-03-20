/*!
 * e2-sat-editor/src/gui/toolkit/TreeEventHandler.cpp
 *
 * @link https://github.com/ctlcltd/e2-sat-editor
 * @copyright e2 SAT Editor Team
 * @author Leonardo Laureti
 * @version 0.5
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

bool TreeEventHandler::eventFilter(QObject* object, QEvent* event)
{
	if (event->type() == QEvent::Drop)
	{
		QDropEvent* e = static_cast<QDropEvent*>(event);
		QTreeWidget* tree = qobject_cast<QTreeWidget*>(object->parent());
		QTreeWidget* list = qobject_cast<QTreeWidget*>(e->source());
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
		QPoint pos = e->position().toPoint();
#else
		QPoint pos = e->pos();
#endif

		QTreeWidgetItem* current = tree->itemAt(pos);
		int ti = tree->indexOfTopLevelItem(current);

		e->setDropAction(Qt::DropAction::IgnoreAction);

		// discard drop
		// all | tv | radio
		if (current == nullptr || ti != -1)
		{
		}
		// drop from tree
		// userbouquets
		else if (tree == list)
		{
			callEventCallback(tree, current);
		}
		// drop from list
		// userbouquets
		else
		{
			callEventCallback(list, current);
		}

		return QObject::eventFilter(object, e);
	}

	return QObject::eventFilter(object, event);
}

}
