/*!
 * e2-sat-editor/src/gui/toolkit/TabBarProxyStyle.h
 *
 * @link https://github.com/ctlcltd/e2-sat-editor
 * @copyright e2 SAT Editor Team
 * @author Leonardo Laureti
 * @version 1.8.0
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
		void drawPrimitive(PrimitiveElement element, const QStyleOption* option, QPainter* painter, const QWidget* widget = nullptr) const override
		{
			if (element == QStyle::PE_FrameFocusRect) // 3
				return;

			QProxyStyle::drawPrimitive(element, option, painter, widget);
		}
		void drawControl(ControlElement element, const QStyleOption* option, QPainter* painter, const QWidget* widget = nullptr) const override
		{
			QProxyStyle::drawControl(element, option, painter, widget);
		}
		int styleHint(StyleHint hint, const QStyleOption* option = 0, const QWidget* widget = 0, QStyleHintReturn* returnData = 0) const override
		{
			if (hint == QStyle::SH_TabBar_CloseButtonPosition)
				return QTabBar::RightSide;

			return QProxyStyle::styleHint(hint, option, widget, returnData);
		}
};
}
#endif /* TabBarProxyStyle_h */
