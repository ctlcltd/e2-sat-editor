/*!
 * e2-sat-editor/src/gui/theme.h
 *
 * @link https://github.com/ctlcltd/e2-sat-editor
 * @copyright e2 SAT Editor Team
 * @author Leonardo Laureti
 * @version 0.1
 * @license MIT License
 * @license GNU GPLv3 License
 */

#ifndef theme_h
#define theme_h
#include <QString>
#include <QIcon>

namespace e2se_gui
{
class theme
{
	public:
		theme();
		~theme() {};
		// preference/theme setting { empty, "light", "dark" }
		static QString preference();
		// absolute lightness { false: light, true: dark }
		static bool absLuma();
		// is default | system native theme
		static bool isDefault();
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
