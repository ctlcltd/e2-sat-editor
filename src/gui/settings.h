/*!
 * e2-sat-editor/src/gui/settings.h
 *
 * @link https://github.com/ctlcltd/e2-sat-editor
 * @copyright e2 SAT Editor Team
 * @author Leonardo Laureti
 * @version 1.1
 * @license MIT License
 * @license GNU GPLv3 License
 */

#include <vector>
#include <map>

#include "../logger/logger.h"

using std::vector, std::map;

#ifndef settings_h
#define settings_h
#include <QSettings>
#include <QWidget>
#include <QDialog>
#include <QTabWidget>
#include <QListWidget>
#include <QTableWidget>
#include <QPushButton>

#include "toolkit/WidgetWithBackdrop.h"
#include "theme.h"
#include "gui.h"

namespace e2se_gui
{
class settings : protected e2se::log_factory
{
	Q_DECLARE_TR_FUNCTIONS(settings)

	public:
		struct __state
		{
			// previous tab index
			int prev;
			// profile retrieving
			bool retr = false;
			// profile deleting
			bool dele = false;
		};

		struct __action
		{
			QPushButton* dtsave;
			QPushButton* dtcancel;
		};

		enum PREF_SECTIONS {
			Connections,
			Preferences,
			Engine,
			Advanced
		};

		settings(gui* gid);
		virtual ~settings();
		void display(QWidget* cwid);
		void destroy();
		void themeChanged();

	protected:
		void layout(QWidget* cwid);
		void connectionsLayout();
		void preferencesLayout();
		void engineLayout();
		void advancedLayout();
		QListWidgetItem* addProfile(int i = -1);
		void deleteProfile();
		void renameProfile(bool enabled = true);
		void updateProfile(QListWidgetItem* item);
		void profileNameChanged(QString text);
		void currentProfileChanged(QListWidgetItem* current, QListWidgetItem* previous);
		void tabChanged(int index);
		void store();
		void store(QTableWidget* adtbl);
		void retrieve();
		void retrieve(QListWidgetItem* item);
		void retrieve(QTableWidget* adtbl);
		void save();
		void cancel();

	private:
		gui* gid;
		QDialog* dial = nullptr;
		e2se_gui::theme* theme;
		QSettings* sets;
		QTabWidget* dtwid;
		WidgetWithBackdrop* rppage;
		QListWidget* rplist;
		QWidget* adntc;
		QTableWidget* adtbl;
		map<int, map<QString, QVariant>> tmpps; //Qt5
		map<int, vector<QWidget*>> prefs;

		__state state;
		__action action;
};
}
#endif /* settings_h */
