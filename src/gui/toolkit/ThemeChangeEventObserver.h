/*!
 * e2-sat-editor/src/gui/toolkit/ThemeChangeEventObserver.h
 *
 * @link https://github.com/ctlcltd/e2-sat-editor
 * @copyright e2 SAT Editor Team
 * @author Leonardo Laureti
 * @version 1.2.0
 * @license MIT License
 * @license GNU GPLv3 License
 */

#include <functional>

#ifndef ThemeChangeEventObserver_h
#define ThemeChangeEventObserver_h
#include <QObject>
#include <QEvent>

namespace e2se_gui
{
class ThemeChangeEventObserver : public QObject
{
	public:
		void setEventCallback(std::function<void()> func)
		{
			this->eventCallback = func;
		}

	protected:
		bool eventFilter(QObject* object, QEvent* event)
		{
			//TODO TEST call once
			if (event->type() == QEvent::ThemeChange/* || event->type() == QEvent::ApplicationPaletteChange*/)
				return eventThemeChange(object, event);

			return QObject::eventFilter(object, event);
		}
		bool eventThemeChange(QObject* object, QEvent* event)
		{
			callEventCallback();
			return QObject::eventFilter(object, event);
		}
		void callEventCallback()
		{
			if (this->eventCallback)
				this->eventCallback();
		}

	private:
		std::function<void()> eventCallback;
};
}
#endif /* ThemeChangeEventObserver_h */
