/*!
 * e2-sat-editor/src/gui/platforms/platform.h
 *
 * @link https://github.com/ctlcltd/e2-sat-editor
 * @copyright e2 SAT Editor Team
 * @author Leonardo Laureti
 * @version 0.7
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
			return widget;
		}
		static QWidget* osWidgetBlend(QWidget* widget)
		{
			return widget;
		}
		static QWidget* osWidgetOpaque(QWidget* widget)
		{
			return widget;
		}
		static void osMenuPopup(QMenu* menu, QWidget* widget, QPoint pos)
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
		}
		static QWidget* _osWidgetBlend(QWidget* widget)
		{
			return widget;
		}
		static QWidget* _osWidgetOpaque(QWidget* widget)
		{
			return widget;
		}
};

#ifdef Q_OS_MAC
#include "platform_macx.h"
	class platform : public _platform_macx {};
#else
	class platform : public _platform {};
#endif

#endif /* platform_h */
