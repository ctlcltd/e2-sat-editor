/*!
 * e2-sat-editor/src/gui/theme.h
 *
 * @link https://github.com/ctlcltd/e2-sat-editor
 * @copyright e2 SAT Editor Team
 * @author Leonardo Laureti
 * @version 1.3.0
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
		// style defaulted
		static void styleDefault();
		// style Fusion modified variant light
		static void styleLight();
		// style Fusion modified variant dark
		static void styleDark();
		// style win modified  light: vista  dark: fusion
		static void styleWin();
		// stylesheet win fusion dark mode  top level widget
		static QString win_fusion_DarkStyleSheet_tlw();
		// stylesheet win fusion dark mode  QApplication
		static QString win_fusion_DarkStyleSheet_root();
		// fix theme  before QApplication
		static void fix();
		// fix theme  after top level widget
		static void fix(QWidget* tlw);
		// fix theme  after QApplication
		static void fix(QApplication* mroot);

	private:
		vector<QWidget*> styled;
		vector<QObject*> imaged;
};
}
#endif /* theme_h */
