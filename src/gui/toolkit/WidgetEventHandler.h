/*!
 * e2-sat-editor/src/gui/toolkit/WidgetEventHandler.h
 *
 * @link https://github.com/ctlcltd/e2-sat-editor
 * @copyright e2 SAT Editor Team
 * @author Leonardo Laureti
 * @version 1.9.0
 * @license MIT License
 * @license GNU GPLv3 License
 */

#include <functional>

#ifndef WidgetEventHandler_h
#define WidgetEventHandler_h
#include <QObject>
#include <QEvent>

namespace e2se_gui
{
class WidgetEventHandler : public QObject
{
	public:
		enum CALLEVENT {
			closeEvent,
			themeChanged,
			fileDropped
		};

		void setEventCallback(std::function<void(CALLEVENT event, const QString path)> func)
		{
			this->eventCallback = func;
		}

	protected:
		bool eventFilter(QObject* object, QEvent* event);
		bool eventClose(QObject* object, QEvent* event);
		bool eventThemeChange(QObject* object, QEvent* event);
		bool eventDragEnter(QObject* object, QEvent* event);
		bool eventDragMove(QObject* object, QEvent* event);
		bool eventDragLeave(QObject* object, QEvent* event);
		bool eventDrop(QObject* object, QEvent* event);
		void raiseWindow(QObject* object);
		void callEventCallback(CALLEVENT event, const QString path = nullptr)
		{
			if (this->eventCallback)
				this->eventCallback(event, path);
		}

	private:
		std::function<void(CALLEVENT event, const QString path)> eventCallback;
};
}
#endif /* WidgetEventHandler_h */
