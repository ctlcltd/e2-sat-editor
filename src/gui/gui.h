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

#include "../logger/logger.h"

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
			FileSaveAs = 0x00000008,
			FileImport = 0x00000010,
			FileExport = 0x00000020,
			FilePrint = 0x00000040,
			FilePrintAll = 0x00000080,
			TabBouquetsEdit = 0x00000100,
			TabBouquetsDelete = 0x00000200,
			TabBouquetsFind = 0x00000400,
			TabBouquetsFindNext = 0x00000800,
			TabListEditService = 0x00001000,
			TabListEditMarker = 0x00002000,
			TabListDelete = 0x00004000,
			TabListSelectAll = 0x00008000,
			TabListCut = 0x00010000,
			TabListCopy = 0x00020000,
			TabListPaste = 0x00040000,
			TabListFind = 0x00100000,
			TabListFindNext = 0x00200000,
			TabListFindPrev = 0x00400000,
			TabListFindAll = 0x00800000,
			ToolsTunersetsSat = 0x01000000,
			ToolsTunersetsTerrestrial = 0x02000000,
			ToolsTunersetsCable = 0x04000000,
			ToolsTunersetsAtsc = 0x08000000,
			NewTab = 0x10000000,
			CloseTab = 0x20000000,
			CloseAllTabs = 0x40000000,
			WindowMinimize = 0x80000000,
			init = GUI_CXE::FileNew | GUI_CXE::FileOpen | GUI_CXE::FileSave | GUI_CXE::FileSaveAs | GUI_CXE::FileImport | GUI_CXE::FileExport | GUI_CXE::FilePrint | GUI_CXE::FilePrintAll | GUI_CXE::NewTab | GUI_CXE::CloseTab | GUI_CXE::CloseAllTabs | GUI_CXE::WindowMinimize,
			idle = GUI_CXE::FileNew | GUI_CXE::FileOpen
		};

		enum TAB_ATS {
			BouquetsFind = GUI_CXE::TabBouquetsFind,
			BouquetsFindNext = GUI_CXE::TabBouquetsFindNext,
			ListCut = GUI_CXE::TabListCut,
			ListCopy = GUI_CXE::TabListCopy,
			ListPaste = GUI_CXE::TabListPaste,
			ListDelete = GUI_CXE::TabListDelete,
			ListSelectAll = GUI_CXE::TabListSelectAll,
			ListFind = GUI_CXE::TabListFind,
			ListFindNext = GUI_CXE::TabListFindNext,
			ListFindPrev = GUI_CXE::TabListFindPrev,
			ListFindAll = GUI_CXE::TabListFindAll,
			EditTunerSat = GUI_CXE::ToolsTunersetsSat,
			EditTunerTerrestrial = GUI_CXE::ToolsTunersetsTerrestrial,
			EditTunerCable = GUI_CXE::ToolsTunersetsCable,
			EditTunerAtsc = GUI_CXE::ToolsTunersetsAtsc,
			Print = GUI_CXE::FilePrint,
			PrintAll = GUI_CXE::FilePrintAll,
			Inspector = 0x0
		};

		enum GUI_DPORTS {
			Services = 0x1,
			Bouquets = 0x2,
			Userbouquets = 0x4,
			Tunersets = 0x8
		};

		enum COUNTER {
			data,
			tv,
			radio,
			all,
			current
		};

		gui(int argc, char* argv[], e2se::logger::session* log);
		~gui();
		void layout();
		void menuCtl();
		void tabCtl();
		void statusCtl();
		void windowChanged();
		int newTab(string filename = "");
		void closeTab(int index);
		void closeAllTabs();
		void tabChanged(int index);
		void tabMoved(int from, int to);
		string openFileDialog();
		string saveFileDialog(string filename);
		vector<string> importFileDialog();
		string exportFileDialog(GUI_DPORTS gde, string filename, int& flags);
		void tabChangeName(int ttid, string filename = "");
		void setStatus(int counters[5]);
		void resetStatus();
		void fileOpen();
		void fileSave();
		void fileSaveAs();
		void fileImport();
		void fileExport();
		void filePrint();
		void filePrintAll();
		void tabAction(TAB_ATS action);
		void windowMinimize();
		void settings();
		void about();
		int getActionFlag(GUI_CXE connector);
		void setActionFlag(GUI_CXE connector, bool flag);
		int getActionFlags();
		void setActionFlags(int connectors);
		void setActionFlags(int connectors, bool flag);
		int getCurrentTabID();
		int getCurrentTabID(int index);
		tab* getCurrentTabHandler();
		void update();
		void update(GUI_CXE connector, bool flag);
		void update(int connectors, bool flag);
		void update(int connectors);
		void launcher();
		void setDefaultSets();
		QSettings* sets;
	private:
		struct sts
		{
			// tab id increment
			int tt = 0;
			// gui connector flags
			int xe;
			// gui previous connector flags
			int ex;
		} state;
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
