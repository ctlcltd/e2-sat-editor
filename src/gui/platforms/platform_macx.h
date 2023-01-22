/*!
 * e2-sat-editor/src/gui/platforms/platform_macx.h
 *
 * @link https://github.com/ctlcltd/e2-sat-editor
 * @copyright e2 SAT Editor Team
 * @author Leonardo Laureti
 * @version 0.3
 * @license MIT License
 * @license GNU GPLv3 License
 */

#ifndef _platform_macx_h
#define _platform_macx_h
#include <QSettings>
#include <QWidget>
#include <QMenu>
#include <QLineEdit>
#include <QComboBox>

class _platform_macx
{
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

		static const bool TESTING = false;

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
			bool experiment = QSettings().value("preference/osExperiment", true).toBool();
			if (QSettings().value("preference/osTranslucency", experiment).toBool())
				return _osWindowBlend(widget);
			else
				return widget;
		}
		static QWidget* osWidgetBlend(QWidget* widget, FX_MATERIAL material = fx_translucent_background, FX_BLENDING blending = fx_translucent)
		{
			bool experiment = QSettings().value("preference/osExperiment", true).toBool();
			if (QSettings().value("preference/osTranslucency", experiment).toBool())
				return _osWidgetBlend(widget, material, blending);
			else
				return widget;
		}
		static QWidget* osWidgetOpaque(QWidget* widget)
		{
			return widget;
		}
		static void osContextMenuPopup(QMenu* menu, QWidget* widget, QPoint pos)
		{
			//TODO
			if (! TESTING)
			{
				menu->popup(widget->mapToGlobal(pos));
				return;
			}

			bool experiment = QSettings().value("preference/osExperiment", false).toBool();
			if (QSettings().value("preference/osContextMenu", experiment).toBool())
				_osContextMenuPopup(menu, widget, pos);
			else
				menu->popup(widget->mapToGlobal(pos));
		}
		static QLineEdit* osLineEdit(QLineEdit* input)
		{
			//TODO
			if (! TESTING)
			{
				return input;
			}

			bool experiment = QSettings().value("preference/osExperiment", false).toBool();
			if (QSettings().value("preference/osContextMenu", experiment).toBool())
				return _osLineEdit(input);
			else
				return input;
		}
		static QComboBox* osComboBox(QComboBox* select)
		{
			bool experiment = QSettings().value("preference/osExperiment", true).toBool();
			if (QSettings().value("preference/osContextMenu", experiment).toBool())
				return _osComboBox(select);
			else
				return select;
		}

	protected:
		static QWidget* _osWindowBlend(QWidget* widget);
		static QWidget* _osWidgetBlend(QWidget* widget, FX_MATERIAL material, FX_BLENDING blending);
		static void _osContextMenuPopup(QMenu* menu, QWidget* widget, QPoint pos);
		static QLineEdit* _osLineEdit(QLineEdit* input);
		static QComboBox* _osComboBox(QComboBox* select);
};
#endif /* _platform_macx_h */
