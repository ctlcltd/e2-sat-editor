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
#include <QTableWidget>

#include "WidgetWithBackdrop.h"

using e2se_gui::WidgetWithBackdrop;

namespace e2se_gui_dialog
{
class settings
{
	public:
		settings(QWidget* mwid);
		void preferences();
		void connections();
		void advanced();
		void newProfile();
		void delProfile();
		void renameProfile(bool dismiss);
		void tabChanged(int index);
	protected:
		QTabWidget* dtwid;
		QDialog* dial;
		WidgetWithBackdrop* rppage;
		QListWidget* rplist;
		QWidget* adntc;
		QTableWidget* adtbl;
	private:
		int _state_previ;
};
}
#endif /* settings_h */
