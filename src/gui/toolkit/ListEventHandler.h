/*!
 * e2-sat-editor/src/gui/toolkit/ListEventHandler.h
 *
 * @link https://github.com/ctlcltd/e2-sat-editor
 * @copyright e2 SAT Editor Team
 * @author Leonardo Laureti
 * @version 0.2
 * @license MIT License
 * @license GNU GPLv3 License
 */

#include <functional>

#ifndef ListEventHandler_h
#define ListEventHandler_h
#include <QObject>
#include <QEvent>

namespace e2se_gui
{
class ListEventHandler : public QObject
{
	public:
		void setEventCallback(std::function<void(QTreeWidget* tw)> func)
		{
			this->eventCallback = func;
		}
		void disallowInternalMove()
		{
			this->dnd = false;
		}
		void allowInternalMove()
		{
			this->dnd = true;
		}

	protected:
		bool eventFilter(QObject* o, QEvent* e);
		void callEventCallback(QTreeWidget* tw)
		{
			if (this->eventCallback != nullptr)
				this->eventCallback(tw);
		}

	private:
		std::function<void(QTreeWidget* tw)> eventCallback;
		bool dnd = true;
};
}
#endif /* ListEventHandler_h */
