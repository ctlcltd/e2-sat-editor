/*!
 * e2-sat-editor/src/gui/toolkit/ListEventObserver.h
 *
 * @link https://github.com/ctlcltd/e2-sat-editor
 * @copyright e2 SAT Editor Team
 * @author Leonardo Laureti
 * @version 0.1
 * @license MIT License
 * @license GNU GPLv3 License
 */

#ifndef ListEventObserver_h
#define ListEventObserver_h
#include <QObject>
#include <QEvent>

namespace e2se_gui
{
class ListEventObserver : public QObject
{
	public:
		bool isChanged()
		{
			return this->changed;
		}
		void reset()
		{
			this->changed = false;
		}
	protected:
		bool eventFilter(QObject* o, QEvent* e);
	private:
		bool changed = false;
};
}
#endif /* ListEventObserver_h */
