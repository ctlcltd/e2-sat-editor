/*!
 * e2-sat-editor/src/gui/toolkit/TabBarProxyStyle.h
 *
 * @link https://github.com/ctlcltd/e2-sat-editor
 * @copyright e2 SAT Editor Team
 * @author Leonardo Laureti
 * @version 0.3
 * @license MIT License
 * @license GNU GPLv3 License
 */

#ifndef TabBarProxyStyle_h
#define TabBarProxyStyle_h
#include <QProxyStyle>
#include <QTabWidget>

namespace e2se_gui
{
class TabBarProxyStyle : public QProxyStyle
{
	public:
		int styleHint(StyleHint hint, const QStyleOption* option = 0, const QWidget* widget = 0, QStyleHintReturn* returnData = 0) const override
		{
			if (hint == QStyle::SH_TabBar_CloseButtonPosition)
				return QTabBar::RightSide;

			return QProxyStyle::styleHint(hint, option, widget, returnData);
		}
};
}
#endif /* TabBarProxyStyle_h */
