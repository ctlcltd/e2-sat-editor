/*!
 * e2-sat-editor/src/gui/settings.h
 *
 * @author Leonardo Laureti
 * @version 0.1
 * @license MIT License
 */

#ifndef settings_h
#define settings_h
#include <QWidget>
#include <QDialog>
#include <QTabWidget>

namespace e2se_gui_settings
{
class settings
{
	public:
		settings(QWidget* mwid);
		void connections();
	protected:
		QTabWidget* dtwid;
		QDialog* dial;
};
}
#endif /* settings_h */
