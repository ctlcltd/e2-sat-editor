/*!
 * e2-sat-editor/src/gui/platforms/platform_macx.h
 *
 * @link https://github.com/ctlcltd/e2-sat-editor
 * @copyright e2 SAT Editor Team
 * @author Leonardo Laureti
 * @version 0.2
 * @license MIT License
 * @license GNU GPLv3 License
 */

#ifndef _platform_macx_h
#define _platform_macx_h
#include <QWidget>
#include <QMenu>
#include <QLineEdit>
#include <QComboBox>
#include <QSettings>

class _platform_macx
{
	private:

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

		enum FX_MATERIAL {
			fx_translucent_background = _FX_MATERIAL::macx_fx_UnderWindowBackground,
			fx_area_background = _FX_MATERIAL::macx_fx_UnderWindowBackground,
			fx_top_background = _FX_MATERIAL::macx_fx_WindowBackground,
			fx_opaque_background = _FX_MATERIAL::macx_fx_ContentBackground
		};

		//TODO FIX ~400MB RAM
		static QWidget* osWidgetBlend(QWidget* widget, FX_MATERIAL material = fx_translucent_background)
		{
			bool experiment = QSettings().value("preference/osExperiment", true).toBool();
			if (QSettings().value("preference/osWidgetBlend", experiment).toBool())
				return _osWidgetBlend(widget, material);
			else
				return widget;
		}
		//TODO FIX wrong position and mouse release
		static void osContextMenuPopup(QMenu* menu, QWidget* widget, QPoint pos)
		{
			bool experiment = QSettings().value("preference/osExperiment", false).toBool();
			if (QSettings().value("preference/osContextMenu", experiment).toBool())
				_osContextMenuPopup(menu, widget, pos);
			else
				menu->popup(widget->mapToGlobal(pos));
		}
		//TODO FIX dialogs
		static QLineEdit* osLineEdit(QLineEdit* input)
		{
			bool isMainView = ! (input->window()->windowType() & Qt::Dialog);
			bool experiment = QSettings().value("preference/osExperiment", false).toBool();
			if (QSettings().value("preference/osContextMenu", experiment).toBool() && isMainView)
				return _osLineEdit(input);
			else
				return input;
		}
		//TODO only for non-editable
		static QComboBox* osComboBox(QComboBox* select)
		{
			bool experiment = QSettings().value("preference/osExperiment", true).toBool();
			if (QSettings().value("preference/osContextMenu", experiment).toBool())
				return _osComboBox(select);
			else
				return select;
		}

	protected:

		static QWidget* _osWidgetBlend(QWidget* widget, FX_MATERIAL material = fx_translucent_background);
		static void _osContextMenuPopup(QMenu* menu, QWidget* widget, QPoint pos);
		static QLineEdit* _osLineEdit(QLineEdit* input);
		static QComboBox* _osComboBox(QComboBox* select);

};
#endif /* _platform_macx_h */
