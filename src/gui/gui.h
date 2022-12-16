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
#include <bitset>

using std::string, std::vector, std::unordered_map, std::bitset;

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
			NewTab = 1,
			CloseTab = 4,
			CloseAllTabs = 8,
			WindowMinimize = 2,
			FileNew = 11,
			FileOpen = 10,
			FileSave = 15,
			FileSaveAs = 16,
			FileImport = 18,
			FileExport = 19,
			FilePrint = 12,
			FilePrintAll = 13,
			TabTreeEdit = 20,
			TabTreeDelete = 22,
			TabTreeFind = 29,
			TabTreeFindNext = 27,
			TabListEditService = 32,
			TabListEditMarker = 33,
			TabListDelete = 34,
			TabListSelectAll = 35,
			TabListCut = 44,
			TabListCopy = 42,
			TabListPaste = 43,
			TabListFind = 49,
			TabListFindNext = 47,
			TabListFindPrev = 45,
			TabListFindAll = 48,
			TunersetsSat = 51,
			TunersetsTerrestrial = 52,
			TunersetsCable = 53,
			TunersetsAtsc = 54,
			OpenChannelBook = 70,
			ToolsServicesOrder = 88,
			ToolsServicesCache = 83,
			ToolsBouquetsOrder = 98,
			ToolsBouquetsDelete = 92,
			ToolsInspector = 0xfe,
			init = 0,
			idle = -1
		};

		inline static const vector<int> GUI_CXE__init = {
			GUI_CXE::NewTab,
			GUI_CXE::CloseTab,
			GUI_CXE::CloseAllTabs,
			GUI_CXE::WindowMinimize,
			GUI_CXE::FileNew,
			GUI_CXE::FileOpen,
			GUI_CXE::FileSave,
			GUI_CXE::FileSaveAs,
			GUI_CXE::FileImport,
			GUI_CXE::FileExport,
			GUI_CXE::FilePrint,
			GUI_CXE::FilePrintAll,
			GUI_CXE::ToolsInspector
		};

		inline static const vector<int> GUI_CXE__idle = {
			GUI_CXE::FileNew,
			GUI_CXE::FileOpen
		};

		enum TAB_ATS {
			Print = GUI_CXE::FilePrint,
			PrintAll = GUI_CXE::FilePrintAll,
			Inspector = GUI_CXE::ToolsInspector,
			TreeFind = GUI_CXE::TabTreeFind,
			TreeFindNext = GUI_CXE::TabTreeFindNext,
			ListCut = GUI_CXE::TabListCut,
			ListCopy = GUI_CXE::TabListCopy,
			ListPaste = GUI_CXE::TabListPaste,
			ListDelete = GUI_CXE::TabListDelete,
			ListSelectAll = GUI_CXE::TabListSelectAll,
			ListFind = GUI_CXE::TabListFind,
			ListFindNext = GUI_CXE::TabListFindNext,
			ListFindPrev = GUI_CXE::TabListFindPrev,
			ListFindAll = GUI_CXE::TabListFindAll,
			EditTunersetsSat = GUI_CXE::TunersetsSat,
			EditTunersetsTerrestrial = GUI_CXE::TunersetsTerrestrial,
			EditTunersetsCable = GUI_CXE::TunersetsCable,
			EditTunersetsAtsc = GUI_CXE::TunersetsAtsc,
			ShowChannelBook = GUI_CXE::OpenChannelBook
		};

		enum TAB_VIEW {
			main,
			tunersets,
			channelBook
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
		void menuLayout();
		void tabLayout();
		void statusLayout();
		void tabViewSwitch(int v);
		void tabViewSwitch(int v, int arg);
		int newTab(string filename = "");
		int openTab(TAB_VIEW view);
		int openTab(TAB_VIEW view, int arg);
		void closeTab(int index);
		void closeAllTabs();
		void windowChanged();
		void tabChanged(int index);
		void tabMoved(int from, int to);
		void tabChangeName(int ttid, string filename = "");
		string openFileDialog();
		string saveFileDialog(string filename);
		vector<string> importFileDialog();
		string exportFileDialog(GUI_DPORTS gde, string filename, int& flags);
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
		bool getActionFlag(GUI_CXE connector);
		void setActionFlag(GUI_CXE connector, bool flag);
		bitset<256> getActionFlags();
		void setActionFlags(bitset<256> connectors);
		//TODO bits sequence bitwise or
		void setActionFlags(vector<int> connectors, bool flag);
		int getTabId(int index);
		int getCurrentTabId();
		tab* getCurrentTabHandler();
		void update();
		void update(int connector, bool flag);
		void update(vector<int> connectors, bool flag);
		void update(vector<int> connectors);
		void update(int connector);
		void launcher();
		void setDefaultSets();
		QSettings* sets;
	protected:
		struct sts
		{
			// tab id increment
			int tt = 0;
			// gui connector flags
			bitset<256> xe;
			// gui previous connector flags
			bitset<256> ex;
		} state;
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
		unordered_map<int, QAction*> gmenu;
		unordered_map<int, QAction*> ttmenu;
		unordered_map<int, tab*> ttabs;
};
}
#endif /* gui_h */
