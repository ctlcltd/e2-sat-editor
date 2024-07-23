/*!
 * e2-sat-editor/src/gui/platforms/platform_macx.h
 *
 * @link https://github.com/ctlcltd/e2-sat-editor
 * @copyright e2 SAT Editor Team
 * @author Leonardo Laureti
 * @version 1.6.0
 * @license MIT License
 * @license GNU GPLv3 License
 */

#include "../theme.h"

#ifndef _platform_macx_h
#define _platform_macx_h
#include <QApplication>
#include <QSettings>
#include <QWidget>
#include <QMenu>
#include <QLineEdit>
#include <QComboBox>
#include <QTextEdit>
#include <QLabel>

class _platform_macx
{
	Q_DECLARE_TR_FUNCTIONS(_platform_macx)

	private:

		enum _FX_BLENDING {
			macx_fx_BehindWindow = 0,
			macx_fx_WithinWindow = 1
		};

		enum _FX_MATERIAL {
			macx_fx_TitleBar = 3,
			macx_fx_Selection = 4,
			macx_fx_Menu = 5,
			macx_fx_Popover = 6,
			macx_fx_Sidebar = 7,
			macx_fx_HeaderView = 10,
			macx_fx_Sheet = 11,
			macx_fx_WindowBackground = 12,
			macx_fx_HUDwindow = 13,
			macx_fx_FullscreenUI = 15,
			macx_fx_ToolTip = 17,
			macx_fx_ContentBackground = 18,
			macx_fx_UnderWindowBackground = 21,
			macx_fx_UnderPageBackground = 22
		};

	public:

		enum FX_BLENDING {
			fx_translucent = _FX_BLENDING::macx_fx_BehindWindow,
			fx_opaque = _FX_BLENDING::macx_fx_WithinWindow
		};

		enum FX_MATERIAL {
			fx_translucent_background = _FX_MATERIAL::macx_fx_UnderWindowBackground,
			fx_opaque_background = _FX_MATERIAL::macx_fx_WindowBackground,
			fx_area_background = _FX_MATERIAL::macx_fx_ContentBackground,
			fx_page_background = _FX_MATERIAL::macx_fx_UnderPageBackground,
			fx_header_background = _FX_MATERIAL::macx_fx_HeaderView,
			fx_side_background = _FX_MATERIAL::macx_fx_Sidebar,
			fx_titlebar_background = _FX_MATERIAL::macx_fx_TitleBar
		};

		static QWidget* osWindowBlend(QWidget* widget)
		{
			if (osExperiment() && e2se_gui::theme::isDefault())
				return _osWindowBlend(widget);
			else
				return widget;
		}
		static QWidget* osWidgetBlend(QWidget* widget, FX_MATERIAL material = fx_translucent_background, FX_BLENDING blending = fx_translucent)
		{
			if (osExperiment() && e2se_gui::theme::isDefault())
				return _osWidgetBlend(widget, material, blending);
			else
				return widget;
		}
		static QWidget* osWidgetOpaque(QWidget* widget)
		{
			return widget;
		}
		static void osMenuPopup(QMenu* menu, QWidget* widget, QPoint pos)
		{
			if (osExperiment())
				_osMenuPopup(menu, widget, pos);
			else
				menu->popup(widget->mapToGlobal(pos));
		}
		static QLineEdit* osLineEdit(QLineEdit* input, bool destroy = true)
		{
			if (osExperiment())
				return _osLineEdit(input, destroy);
			else
				return input;
		}
		static QComboBox* osComboBox(QComboBox* select)
		{
			if (osExperiment())
				return _osComboBox(select);
			else
				return select;
		}
		static QTextEdit* osTextEdit(QTextEdit* input, bool destroy = true)
		{
			if (osExperiment())
				return _osTextEdit(input, destroy);
			else
				return input;
		}
		static QLabel* osLabel(QLabel* label, bool destroy = true)
		{
			if (osExperiment())
				return _osLabel(label, destroy);
			else
				return label;
		}
		static QWidget* osPersistentEditor(QWidget* widget)
		{
			if (osExperiment())
				return _osPersistentEditor(widget);
			else
				return widget;
		}

	protected:
		static QWidget* _osWindowBlend(QWidget* widget);
		static QWidget* _osWidgetBlend(QWidget* widget, FX_MATERIAL material, FX_BLENDING blending);
		static void _osMenuPopup(QMenu* menu, QWidget* widget, QPoint pos);
		static QLineEdit* _osLineEdit(QLineEdit* input, bool destroy);
		static QComboBox* _osComboBox(QComboBox* select);
		static QTextEdit* _osTextEdit(QTextEdit* input, bool destroy);
		static QLabel* _osLabel(QLabel* label, bool destroy);
		static QWidget* _osPersistentEditor(QWidget* widget);
		// note: platform minimal causes crash at startup even with osExperiment=false
		// due to cocoa plug-in needs, as workaround use fusion style
		static bool osExperiment()
		{
			if (QGuiApplication::platformName() != "cocoa")
				return false;
			else
				return QSettings().value("preference/osExperiment", true).toBool();
		}
};
#endif /* _platform_macx_h */
