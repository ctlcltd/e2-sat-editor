/*!
 * e2-sat-editor/src/gui/platforms/platform.h
 *
 * @link https://github.com/ctlcltd/e2-sat-editor
 * @copyright e2 SAT Editor Team
 * @author Leonardo Laureti
 * @version 0.2
 * @license MIT License
 * @license GNU GPLv3 License
 */

#ifndef platform_h
#define platform_h
#include <QtGlobal>
#include <QWidget>
#include <QMenu>
#include <QLineEdit>
#include <QComboBox>
#include <QSettings>

class _platform
{
	public:

		static QWidget* osWidgetBlend(QWidget* widget) {
			return widget;
		};
		static void osContextMenuPopup(QMenu* menu, QWidget* widget, QPoint pos) {
			menu->popup(widget->mapToGlobal(pos));
		}
		static QLineEdit* osLineEdit(QLineEdit* input) {
			return input;
		}
		static QComboBox* osComboBox(QComboBox* select) {
			return select;
		}

};

#ifdef Q_OS_WIN
	class platform : public _platform {};
#elifdef Q_OS_MAC
	#include "platform_macx.h"
	class platform : public _platform_macx {};
#elifdef Q_OS_LINUX
	class platform : public _platform {};
#endif

#endif /* platform_h */
