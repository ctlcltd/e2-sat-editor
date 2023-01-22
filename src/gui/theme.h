/*!
 * e2-sat-editor/src/gui/theme.h
 *
 * @link https://github.com/ctlcltd/e2-sat-editor
 * @copyright e2 SAT Editor Team
 * @author Leonardo Laureti
 * @version 0.3
 * @license MIT License
 * @license GNU GPLv3 License
 */

#include <functional>
#include <vector>
#include <unordered_map>

using std::pair, std::vector, std::unordered_map;

#ifndef theme_h
#define theme_h
#include <QEvent>
#include <QWidget>
#include <QString>
#include <QIcon>

namespace e2se_gui
{

class themeChangedEventFilter : public QObject
{
	public:
		void setEventCallback(std::function<void()> func)
		{
			this->eventCallback = func;
		}

	protected:
		bool eventFilter(QObject* o, QEvent* e)
		{
			if (e->type() == QEvent::ThemeChange/* || e->type() == QEvent::ApplicationPaletteChange*/)
			{
				//TODO improve call at once
				callEventCallback();
			}

			return QObject::eventFilter(o, e);
		}
		void callEventCallback()
		{
			if (this->eventCallback != nullptr)
				this->eventCallback();
		}

	private:
		std::function<void()> eventCallback;
};


class theme
{
	public:
		enum STYLE {
			light,
			dark
		};

		theme();
		~theme() = default;
		// initialize style
		static void initStyle();
		// preference/theme setting { empty, "light", "dark" }
		static QString preference();
		// absolute lightness { false: light, true: dark }
		static bool absLuma();
		// is default | system native theme
		static bool isDefault();
		// absolute lightness shorthand for dark
		static bool isDarkMode();
		// absolute lightness shorthand for light
		static bool isLightMode();
		// prefixed QIcon { white: dark, black: light }
		static QIcon icon(QString icon);
		// spacer QIcon { 2, 3, 4, 5 } px
		static QIcon spacer(int width);
		// default fontFamily
		static QString fontFamily();
		// default fontSize
		static int fontSize();
		// calculating fontSize increment
		static int calcFontSize(int size);
		// dynamic StyleSheet
		void dynamicStyleSheet(QWidget* widget, QString stylesheet);
		void dynamicStyleSheet(QWidget* widget, QString stylesheet, STYLE style);
		// dynamic QIcon
		pair<theme*, QString> dynamicIcon(QString icon);
		QIcon dynamicIcon(QString icon, QObject* object);
		// theme changed
		void changed();

	private:
		vector<QWidget*> styled;
		vector<QObject*> imaged;
};

class style
{
	public:
		static void system();
		static void light();
		static void dark();
};

}
#endif /* theme_h */
