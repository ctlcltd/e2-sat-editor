/*!
 * e2-sat-editor/src/gui/gui.h
 *
 * @link https://github.com/ctlcltd/e2-sat-editor
 * @copyright e2 SAT Editor Team
 * @author Leonardo Laureti
 * @version 0.1
 * @license MIT License
 * @license GNU GPLv3 License
 */

#include <string>
#include <vector>
#include <unordered_map>

using std::string, std::vector, std::unordered_map;

#ifndef gui_h
#define gui_h
#include <QApplication>
#include <QWidget>
#include <QSettings>
#include <QMenuBar>
#include <QStatusBar>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QTabWidget>
#include <QActionGroup>
#include <QLabel>

#include "../logger.h"

namespace e2se_gui
{
class tab;

class gui : protected e2se::log_factory
{
	Q_DECLARE_TR_FUNCTIONS(gui)

	public:
		enum TAB_EDIT_ATS {
			Cut,
			Copy,
			Paste,
			Delete,
			SelectAll
		};

		gui(int argc, char* argv[]);
		void root();
		void menuCtl();
		void tabCtl();
		void statusCtl();
		int newTab(string filename);
		void closeTab(int index);
		void closeAllTabs();
		void tabChanged(int index);
		void tabClicked(int index);
		void tabMoved(int from, int to);
		string openFileDialog();
		string saveFileDialog(string filename);
		vector<string> importFileDialog();
		string exportFileDialog(string filename);
		void tabChangeName(int ttid, string filename = "");
		void loaded(int counters[5]);
		void reset();
		void open();
		void save();
		void tabEditAction(TAB_EDIT_ATS action);
		void settings();
		void about();
		tab* getCurrentTabHandler();
		void setDefaultSets();
		QSettings* sets;
		QString icopx;
	private:
		QApplication* mroot;
		QWidget* mwid;
		QGridLayout* mfrm;
		QHBoxLayout* mcnt;
		QHBoxLayout* mstatusb;
		QStatusBar* sbwid;
		QLabel* sbwidl;
		QLabel* sbwidr;
		QTabWidget* twid;
		QMenuBar* menu;
		QMenu* mwind;
		QActionGroup* mwtabs;
		int ttidx;
		unordered_map<int, tab*> ttabs;
		unordered_map<int, QAction*> ttmenu;
};
}
#endif /* gui_h */
