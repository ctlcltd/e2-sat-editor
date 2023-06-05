/*!
 * e2-sat-editor/src/gui/toolkit/InputControlEditEventObserver.h
 *
 * @link https://github.com/ctlcltd/e2-sat-editor
 * @copyright e2 SAT Editor Team
 * @author Leonardo Laureti
 * @version 0.8
 * @license MIT License
 * @license GNU GPLv3 License
 */

#ifndef InputControlEditEventObserver_h
#define InputControlEditEventObserver_h
#include <QObject>
#include <QEvent>

namespace e2se_gui
{
class tab;

class InputControlEditEventObserver : public QObject
{
	public:
		InputControlEditEventObserver(tab* tid)
		{
			this->tid = tid;
		}

	protected:
		bool eventFilter(QObject* object, QEvent* event);
		bool eventFocusIn(QObject* object, QEvent* event);
		bool eventFocusOut(QObject* object, QEvent* event);

	private:
		tab* tid;
};
}
#endif /* InputControlEditEventObserver_h */
