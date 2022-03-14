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

using namespace std;

#ifndef DropEventHandler_h
#define DropEventHandler_h
#include <QEvent>

namespace e2se_gui
{
class DropEventHandler : public QObject
{
	protected:
		bool eventFilter(QObject* o, QEvent* e)
		{
			if (e->type() == QEvent::Drop)
			{
				QEvent* dropEvt = static_cast<QEvent*>(e);
				return true;
			}

//			return QObject::eventFilter(obj, event);
			return false;
		}
};
}
#endif /* DropEventHandler_h */
