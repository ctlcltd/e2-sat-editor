/*!
 * e2-sat-editor/src/gui/theme.h
 *
 * @link https://github.com/ctlcltd/e2-sat-editor
 * @copyright e2 SAT Editor Team
 * @author Leonardo Laureti
 * @version 1.8.1
 * @license MIT License
 * @license GNU GPLv3 License
 */

#include <vector>
#include <unordered_map>

using std::pair, std::vector, std::unordered_map;

#ifndef theme_h
#define theme_h
#include <QWidget>
#include <QString>
#include <QIcon>

namespace e2se_gui
{
class theme
{
	public:

		inline static bool QSTYLE_OVERRIDDEN = false;

		enum STYLE {
			light,
			dark
		};

		enum ICON_STYLE {
			icon_default,
			icon_highlight
		};

		theme();
		theme(QApplication* mroot);
		~theme() = default;
		// check style override
		static void checkStyleOverride(int argc, char* argv[]);
		// initialize style
		static void initStyle();
		// preference/theme setting { empty, "light", "dark" }
		static QString preference();
		// absolute lightness { false: light, true: dark }
		static bool absLuma();
		// is overridden by command line argument
		static bool isOverridden();
		// is default | system native theme
		static bool isDefault();
		// absolute lightness shorthand for dark
		static bool isDarkMode();
		// absolute lightness shorthand for light
		static bool isLightMode();
		// prefixed QIcon { white: dark, black: light }
		static QIcon icon(QString icon, ICON_STYLE style = icon_default);
		// default fontFamily
		static QString fontFamily();
		// default fontSize
		static int fontSize();
		// calculating fontSize increment
		static int calcFontSize(int size);
		// set wait cursor
		static void setWaitCursor();
		// unset wait cursor
		static void unsetWaitCursor();
		// dynamic StyleSheet
		void dynamicStyleSheet(QWidget* widget, QString stylesheet);
		void dynamicStyleSheet(QWidget* widget, QString stylesheet, STYLE style);
		// dynamic QIcon
		pair<theme*, QString> dynamicIcon(QString icon);
		QIcon dynamicIcon(QString icon, QObject* object, ICON_STYLE style = icon_default);
		// theme changed
		void changed();
		// style Fusion modified variant light
		static void styleLight();
		// style Fusion modified variant dark
		static void styleDark();

#ifdef Q_OS_WIN
		static bool isFluentWin();
		static bool isMetroWin();

		// style pseudo windows
		static void stylePseudoWin();
		static void stylePseudoWinModern();
		static void stylePseudoWinEarly();

		// stylesheet fusion metro dark mode  top level widget
		static QString qss_fusion_PseudoMetroDark_tlw();
		// stylesheet fusion metro dark mode  QApplication
		static QString qss_fusion_PseudoMetroDark_root();

		// stylesheet fusion fluent dark mode  top level widget
		static QString qss_fusion_PseudoFluentDark_tlw();

		// fix theme  before QApplication
		static void win_flavor_fix();
		// fix theme  after top level widget
		static void win_flavor_fix(QWidget* tlw);
		// fix theme  after QApplication
		static void win_flavor_fix(QApplication* mroot);
#endif

	private:
		vector<QWidget*> styled;
		vector<QObject*> imaged;
};
}
#endif /* theme_h */
