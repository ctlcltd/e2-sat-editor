/*!
 * e2-sat-editor/src/gui/TreeEventObserver.h
 *
 * @link https://github.com/ctlcltd/e2-sat-editor
 * @copyright e2 SAT Editor Team
 * @author Leonardo Laureti
 * @version 0.1
 * @license MIT License
 * @license GNU GPLv3 License
 */

using namespace std;

#ifndef TreeEventObserver_h
#define TreeEventObserver_h
#include <QEvent>

namespace e2se_gui
{
class TreeEventObserver : public QObject
{
	public:
		bool isChanged()
		{
			return changed;
		}
		void reset()
		{
			changed = false;
		}
	protected:
		bool eventFilter(QObject* o, QEvent* e)
		{
			if (e->type() == QEvent::ChildRemoved)
				changed = changed ? false : true;
			else if (e->type() == QEvent::ChildAdded)
				changed = changed ? false : true;

			return false;
		}
	private:
		bool changed = false;
};
}
#endif /* TreeEventObserver_h */
