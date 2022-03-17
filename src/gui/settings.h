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

#include <vector>
#include <map>
#include <unordered_map>

using namespace std;

#ifndef settings_h
#define settings_h
#include <QCoreApplication>
#include <QWidget>
#include <QDialog>
#include <QSettings>
#include <QTabWidget>
#include <QListWidget>
#include <QTableWidget>

#include "WidgetWithBackdrop.h"

using e2se_gui::WidgetWithBackdrop;

namespace e2se_gui_dialog
{
class settings
{
	Q_DECLARE_TR_FUNCTIONS(settings)

	public:
		enum PREF_SECTIONS {
			Connections,
			Preferences,
			Advanced
		};

		settings(QWidget* mwid);
		void preferences();
		void connections();
		void advanced();
		QListWidgetItem* addProfile(int id = 0);
		void delProfile();
		void renameProfile(bool enabled = true);
		void profileNameChanged(QString text);
		void currentProfileChanged(QListWidgetItem* previous);
		void tabChanged(int index);
		void store();
		void retrieve();
		void retrieve(QListWidgetItem* item);
		void retrieve(QTableWidget* adtbl);
		QSettings* sets;
	protected:
		QTabWidget* dtwid;
		QDialog* dial;
		WidgetWithBackdrop* rppage;
		QListWidget* rplist;
		QWidget* adntc;
		QTableWidget* adtbl;
		map<int, unordered_map<QString, QVariant>> tmpps;
		map<int, vector<QWidget*>> prefs;
	private:
		int _state_prev;
};
}
#endif /* settings_h */
