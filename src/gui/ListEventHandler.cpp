/*!
 * e2-sat-editor/src/gui/ListEventHandler.cpp
 *
 * @link https://github.com/ctlcltd/e2-sat-editor
 * @copyright e2 SAT Editor Team
 * @author Leonardo Laureti
 * @version 0.1
 * @license MIT License
 * @license GNU GPLv3 License
 */

#include <QDropEvent>
#include <QTreeWidget>

#include "ListEventHandler.h"

namespace e2se_gui
{

bool ListEventHandler::eventFilter(QObject* o, QEvent* e)
{
	if (e->type() == QEvent::Drop)
	{
		QDropEvent* evt = static_cast<QDropEvent*>(e);

		if (! this->dnd)
			evt->setDropAction(Qt::DropAction::IgnoreAction);

		return QObject::eventFilter(o, evt);
	}

	return QObject::eventFilter(o, e);
}

}
