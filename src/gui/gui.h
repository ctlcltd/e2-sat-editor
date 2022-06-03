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
		enum GUI_CXE {
			FileNew = 0x00000001,
			FileOpen = 0x00000002,
			FileSave = 0x00000004,
			FileImport = 0x00000008,
			FileExport = 0x00000010,
			WindowMinimize = 0x10000000,
			NewTab = 0x00000020,
			CloseTab = 0x00000040,
			CloseAllTabs = 0x0000080,
			TabListEditService = 0x00000100,
			TabListEditMarker = 0x00000200,
			TabListSelectAll = 0x00000800,
			TabListDelete = 0x00001000,
			TabListCut = 0x00002000,
			TabListCopy = 0x00004000,
			TabListPaste = 0x00008000,
			TabBouquetsEdit = 0x00010000,
			TabBouquetsDelete = 0x00020000,
			TabBouquetsFind = 0x00100000,
			TabListFind = 0x00200000,
			FindNext = 0x00400000,
			FindPrevious = 0x00800000,
			ToolsTunersetsSat = 0x01000000,
			init = GUI_CXE::FileNew | GUI_CXE::FileOpen | GUI_CXE::FileSave | GUI_CXE::FileImport | GUI_CXE::FileExport |  GUI_CXE::WindowMinimize | GUI_CXE::NewTab | GUI_CXE::CloseTab | GUI_CXE::CloseAllTabs,
			deactivated = GUI_CXE::FileNew | GUI_CXE::FileOpen
		};

		enum TAB_ATS {
			ListCut,
			ListCopy,
			ListPaste,
			ListDelete,
			ListSelectAll,
			BouquetsFind,
			ListFind,
			EditTunerSat
		};

		enum COUNTER {
			data,
			tv,
			radio,
			all,
			current
		};

		gui(int argc, char* argv[]);
		~gui();
		void root();
		void menuCtl();
		void tabCtl();
		void statusCtl();
		void windowFocusChanged();
		int newTab(string filename = "");
		void closeTab(int index);
		void closeAllTabs();
		void tabChanged(int index);
		void tabMoved(int from, int to);
		string openFileDialog();
		string saveFileDialog(string filename);
		vector<string> importFileDialog();
		string exportFileDialog(string filename);
		void tabChangeName(int ttid, string filename = "");
		void setStatus(int counters[5]);
		void resetStatus();
		void open();
		void save();
		void tabAction(TAB_ATS action);
		void windowMinimize();
		void settings();
		void about();
		int getActionFlags();
		void setActionFlags(int connector, bool flag);
		int getCurrentTabID();
		int getCurrentTabID(int index);
		tab* getCurrentTabHandler();
		void update();
		void update(int connector, bool flag);
		void initialize();
		void setDefaultSets();
		QSettings* sets;
	private:
		struct sts
		{
			// tab id increment
			int tt = 0;
			// gui previous connector flags
			int ex;
		} state;
		// gui connector flags
		int xe;
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
		unordered_map<int, QAction*> gmenu;
		unordered_map<int, QAction*> ttmenu;
		unordered_map<int, tab*> ttabs;
};
}
#endif /* gui_h */
