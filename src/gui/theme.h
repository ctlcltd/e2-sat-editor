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
#include <QSettings>
#include <QString>
#include <QIcon>

namespace e2se_gui
{
//TODO on change QEvent::PaletteChange()
class theme
{
	public:
		theme();
		static QIcon icon(QString icon);
		static QString getIcon(QString icon);
		static int getDefaultFontSize();
};

class style
{
	public:
		static void light();
		static void dark();
};

}
#endif /* theme_h */
