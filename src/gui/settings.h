/*!
 * e2-sat-editor/src/gui/settings.h
 *
 * @link https://github.com/ctlcltd/e2-sat-editor
 * @copyright e2 SAT Editor Team
 * @author Leonardo Laureti
 * @version 0.3
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
#include "gui.h"

using e2se_gui::WidgetWithBackdrop;

namespace e2se_gui_dialog
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
			bool retr;
			// profile deleting
			bool dele;
		};

		struct __action
		{
			QPushButton* dtsave;
			QPushButton* dtcancel;
		};

		enum PREF_SECTIONS {
			Connections,
			Preferences,
			Advanced
		};

		settings(e2se_gui::gui* gid, QWidget* cwid);
		virtual ~settings() = default;
		void display(QWidget* cwid);

	protected:
		void layout(QWidget* cwid);
		void preferencesLayout();
		void connectionsLayout();
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
		e2se_gui::gui* gid;
		QDialog* dial = nullptr;
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
