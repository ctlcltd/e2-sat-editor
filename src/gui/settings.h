/*!
 * e2-sat-editor/src/gui/settings.h
 *
 * @link https://github.com/ctlcltd/e2-sat-editor
 * @copyright e2 SAT Editor Team
 * @author Leonardo Laureti
 * @version 0.1
 * @license MIT License
 * @license GNU GPLv3 License
 */

#ifndef settings_h
#define settings_h
#include <QWidget>
#include <QDialog>
#include <QTabWidget>
#include <QListWidget>

#include "WidgetWithBackdrop.h"

namespace e2se_gui_dialog
{
class settings
{
	public:
		settings(QWidget* mwid);
		void preferences();
		void connections();
		void todo();
		void newProfile(QListWidget* list, WidgetWithBackdrop* cnt);
		void delProfile(QListWidget* list, WidgetWithBackdrop* cnt);
		void renameProfile(QListWidget* list, bool dismiss, WidgetWithBackdrop* cnt);
	protected:
		QTabWidget* dtwid;
		QDialog* dial;
};
}
#endif /* settings_h */
