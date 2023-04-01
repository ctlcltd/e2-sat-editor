/*!
 * e2-sat-editor/src/gui/platforms/platform.h
 *
 * @link https://github.com/ctlcltd/e2-sat-editor
 * @copyright e2 SAT Editor Team
 * @author Leonardo Laureti
 * @version 0.6
 * @license MIT License
 * @license GNU GPLv3 License
 */

#ifndef platform_h
#define platform_h
#include <QtGlobal>
#include <QSettings>
#include <QWidget>
#include <QMenu>
#include <QLineEdit>
#include <QComboBox>
#include <QTextEdit>

class _platform
{
	public:
		static QWidget* osWindowBlend(QWidget* widget)
		{
			bool experiment = QSettings().value("preference/osExperiment", false).toBool();
			if (QSettings().value("preference/osTranslucency", experiment).toBool())
				return _osWindowBlend(widget);
			else
				return widget;
		};
		static QWidget* osWidgetBlend(QWidget* widget)
		{
			bool experiment = QSettings().value("preference/osExperiment", false).toBool();
			if (QSettings().value("preference/osTranslucency", experiment).toBool())
				return _osWidgetBlend(widget);
			else
				return widget;
		};
		static QWidget* osWidgetOpaque(QWidget* widget)
		{
			bool experiment = QSettings().value("preference/osExperiment", false).toBool();
			if (QSettings().value("preference/osTranslucency", experiment).toBool())
				return _osWidgetOpaque(widget);
			else
				return widget;
		};
		static void osContextMenuPopup(QMenu* menu, QWidget* widget, QPoint pos)
		{
			menu->popup(widget->mapToGlobal(pos));
		}
		static QLineEdit* osLineEdit(QLineEdit* input)
		{
			return input;
		}
		static QComboBox* osComboBox(QComboBox* select)
		{
			return select;
		}
		static QTextEdit* osTextEdit(QTextEdit* input)
		{
			return input;
		}
		static QWidget* osPersistentEditor(QWidget* widget)
		{
			return widget;
		}

	protected:
		static QWidget* _osWindowBlend(QWidget* widget)
		{
			return widget;
		};
		static QWidget* _osWidgetBlend(QWidget* widget)
		{
			widget->setAttribute(Qt::WA_TranslucentBackground);
			return widget;
		};
		static QWidget* _osWidgetOpaque(QWidget* widget)
		{
			widget->setAttribute(Qt::WA_TintedBackground);
			return widget;
		}
};

#if defined Q_OS_WIN
	class platform : public _platform {};
#elif defined Q_OS_MAC
	#include "platform_macx.h"
	class platform : public _platform_macx {};
#elif defined Q_OS_LINUX
	class platform : public _platform {};
#endif

#endif /* platform_h */
