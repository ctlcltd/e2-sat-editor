/*!
 * e2-sat-editor/src/gui/toolkit/TreeItemChangedEventObserver.h
 *
 * @link https://github.com/ctlcltd/e2-sat-editor
 * @copyright e2 SAT Editor Team
 * @author Leonardo Laureti
 * @version 1.8.0
 * @license MIT License
 * @license GNU GPLv3 License
 */

#ifndef TreeItemChangedEventObserver_h
#define TreeItemChangedEventObserver_h
#include <QObject>
#include <QEvent>

namespace e2se_gui
{
class TreeItemChangedEventObserver : public QObject
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
		bool eventFilter(QObject* object, QEvent* event);

	private:
		bool changed = false;
};
}
#endif /* TreeItemChangedEventObserver_h */
