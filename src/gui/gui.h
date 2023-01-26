/*!
 * e2-sat-editor/src/gui/gui.h
 *
 * @link https://github.com/ctlcltd/e2-sat-editor
 * @copyright e2 SAT Editor Team
 * @author Leonardo Laureti
 * @version 0.3
 * @license MIT License
 * @license GNU GPLv3 License
 */

#include <string>
#include <vector>
#include <unordered_map>
#include <bitset>
#include <functional>

using std::string, std::vector, std::unordered_map, std::bitset;

#ifndef gui_h
#define gui_h
#include <QApplication>
#include <QWidget>
#include <QMenuBar>
#include <QStatusBar>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QTabWidget>
#include <QActionGroup>
#include <QLabel>

#include "../logger/logger.h"
#include "theme.h"

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
			CloseAllTabs = 5,
			WindowMinimize = 6,
			StatusBar = 8,
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
			TabListEditTransponder = 31,
			TabListDelete = 34,
			TabListSelectAll = 35,
			TabListCut = 44,
			TabListCopy = 42,
			TabListPaste = 43,
			TabListFind = 49,
			TabListFindNext = 47,
			TabListFindPrev = 45,
			TabListFindAll = 48,
			Transponders = 50,
			TunersetsSat = 51,
			TunersetsTerrestrial = 52,
			TunersetsCable = 53,
			TunersetsAtsc = 54,
			OpenChannelBook = 70,
			ToolsServicesOrder = 88,
			ToolsServicesCache = 83,
			ToolsBouquetsOrder = 98,
			ToolsBouquetsDelete = 92,
			ToolsImportCSV_services = 101,
			ToolsImportCSV_bouquet = 102,
			ToolsImportCSV_userbouquet = 103,
			ToolsImportCSV_tunersets = 104,
			ToolsExportCSV_current = 106,
			ToolsExportCSV_all = 107,
			ToolsExportCSV_services = 109,
			ToolsExportCSV_bouquets = 110,
			ToolsExportCSV_userbouquets = 111,
			ToolsExportCSV_tunersets = 112,
			ToolsExportHTML_current = 116,
			ToolsExportHTML_all = 117,
			ToolsExportHTML_index = 118,
			ToolsExportHTML_services = 119,
			ToolsExportHTML_bouquets = 120,
			ToolsExportHTML_userbouquets = 121,
			ToolsExportHTML_tunersets = 122,
			ToolsInspector = 0xfe,
			init = 0,
			idle = -1
		};

		inline static const vector<int> GUI_CXE__init = {
			GUI_CXE::NewTab,
			GUI_CXE::CloseTab,
			GUI_CXE::CloseAllTabs,
			GUI_CXE::WindowMinimize,
			GUI_CXE::StatusBar,
			GUI_CXE::FileNew,
			GUI_CXE::FileOpen,
			GUI_CXE::FileSave,
			GUI_CXE::FileSaveAs,
			GUI_CXE::FileImport,
			GUI_CXE::FileExport,
			GUI_CXE::FilePrint,
			GUI_CXE::FilePrintAll,
			GUI_CXE::Transponders,
			GUI_CXE::TunersetsSat,
			GUI_CXE::TunersetsTerrestrial,
			GUI_CXE::TunersetsCable,
			GUI_CXE::TunersetsAtsc,
			GUI_CXE::ToolsImportCSV_services,
			GUI_CXE::ToolsImportCSV_bouquet,
			GUI_CXE::ToolsImportCSV_userbouquet,
			GUI_CXE::ToolsImportCSV_tunersets,
			GUI_CXE::ToolsExportCSV_current,
			GUI_CXE::ToolsExportCSV_all,
			GUI_CXE::ToolsExportCSV_services,
			GUI_CXE::ToolsExportCSV_bouquets,
			GUI_CXE::ToolsExportCSV_userbouquets,
			GUI_CXE::ToolsExportCSV_tunersets,
			GUI_CXE::ToolsExportHTML_current,
			GUI_CXE::ToolsExportHTML_all,
			GUI_CXE::ToolsExportHTML_index,
			GUI_CXE::ToolsExportHTML_services,
			GUI_CXE::ToolsExportHTML_bouquets,
			GUI_CXE::ToolsExportHTML_userbouquets,
			GUI_CXE::ToolsExportHTML_tunersets,
			GUI_CXE::ToolsInspector
		};

		inline static const vector<int> GUI_CXE__idle = {
			GUI_CXE::FileNew,
			GUI_CXE::FileOpen
		};

		enum TAB_ATS {
			Print = GUI_CXE::FilePrint,
			PrintAll = GUI_CXE::FilePrintAll,
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
			EditTransponders = GUI_CXE::Transponders,
			EditTunersetsSat = GUI_CXE::TunersetsSat,
			EditTunersetsTerrestrial = GUI_CXE::TunersetsTerrestrial,
			EditTunersetsCable = GUI_CXE::TunersetsCable,
			EditTunersetsAtsc = GUI_CXE::TunersetsAtsc,
			ShowChannelBook = GUI_CXE::OpenChannelBook,
			ImportCSV_services = GUI_CXE::ToolsImportCSV_services,
			ImportCSV_bouquet = GUI_CXE::ToolsImportCSV_bouquet,
			ImportCSV_userbouquet = GUI_CXE::ToolsImportCSV_userbouquet,
			ImportCSV_tunersets = GUI_CXE::ToolsImportCSV_tunersets,
			ExportCSV_current = GUI_CXE::ToolsExportCSV_current,
			ExportCSV_all = GUI_CXE::ToolsExportCSV_all,
			ExportCSV_services = GUI_CXE::ToolsExportCSV_services,
			ExportCSV_bouquets = GUI_CXE::ToolsExportCSV_bouquets,
			ExportCSV_userbouquets = GUI_CXE::ToolsExportCSV_userbouquets,
			ExportCSV_tunersets = GUI_CXE::ToolsExportCSV_tunersets,
			ExportHTML_current = GUI_CXE::ToolsExportHTML_current,
			ExportHTML_all = GUI_CXE::ToolsExportHTML_all,
			ExportHTML_index = GUI_CXE::ToolsExportHTML_index,
			ExportHTML_services = GUI_CXE::ToolsExportHTML_services,
			ExportHTML_bouquets = GUI_CXE::ToolsExportHTML_bouquets,
			ExportHTML_userbouquets = GUI_CXE::ToolsExportHTML_userbouquets,
			ExportHTML_tunersets = GUI_CXE::ToolsExportHTML_tunersets,
			Inspector = GUI_CXE::ToolsInspector
		};

		enum TAB_VIEW {
			main,
			transponders,
			tunersets,
			channelBook
		};

		enum GUI_DPORTS {
			AllFiles = 0x0,
			Services = 0x1,
			Bouquets = 0x2,
			Userbouquets = 0x4,
			Tunersets = 0x8,
			CSV = 0x10,
			HTML = 0x20
		};

		enum COUNTER {
			n_data = 0,
			n_tv = 1,
			n_radio = 2,
			n_services = 3,
			n_transponders = 3,
			n_bouquet = 4,
			n_position = 0
		};

		struct status
		{
			TAB_VIEW view;
			string curr;
			string message;
			int counters[5] = {0, 0, 0, 0, 0};
			bool info;
			bool update;
		};

		gui(int argc, char* argv[], e2se::logger::session* log);
		virtual ~gui() = default;
		int exec();
		void settingsChanged();
		int newTab(string filename = "");
		int openTab(TAB_VIEW view);
		int openTab(TAB_VIEW view, int arg);
		void closeTab(int index = -1);
		void closeAllTabs();
		string openFileDialog();
		string saveFileDialog(string filename);
		vector<string> importFileDialog(gui::GUI_DPORTS gde);
		string exportFileDialog(GUI_DPORTS gde, string filename, int& bit);
		string exportFileDialog(GUI_DPORTS gde, string filename);
		void tabChangeName(int ttid, string filename = "");
		bool statusBarIsVisible();
		bool statusBarIsHidden();
		void statusBarShow();
		void statusBarHide();
		void statusBarToggle();
		void setStatusBar(status msg);
		void resetStatusBar();
		void fileOpen();
		void fileSave();
		void fileSaveAs();
		void fileImport();
		void fileExport();
		void filePrint();
		void filePrintAll();
		void windowMinimize();
		void settingsDialog();
		void aboutDialog();
		bool getFlag(GUI_CXE bit);
		bool getFlag(int bit);
		void setFlag(GUI_CXE bit, bool flag);
		bitset<256> getFlags();
		void setFlags(bitset<256> bits);
		//TODO improve bits sequence bitwise or
		void setFlags(vector<int> bits, bool flag);
		void update();
		void update(int bit, bool flag);
		void update(vector<int> bits, bool flag);
		void update(vector<int> bits);
		void update(int bit);

		e2se_gui::theme* theme;

	protected:
		void layout();
		void menuBarLayout();
		void tabStackerLayout();
		void statusBarLayout();
		void themeChanged();
		void initSettings();
		void updateSettings();
		void resetSettings();
		void tabViewSwitch(TAB_VIEW ttv);
		void tabViewSwitch(TAB_VIEW ttv, int arg);
		void tabChanged(int index);
		void tabMoved(int from, int to);
		void tabAction(TAB_ATS action);
		void windowChanged();
		int getTabId(int index);
		int getCurrentTabId();
		tab* getCurrentTabHandler();
		void launcher();
	
		static QMenuBar* menuBar(QLayout* layout);
		static QMenu* menuBarMenu(QMenuBar* menubar, QString title);
		static QMenu* menuBarMenu(QMenu* menu, QString title);
		static QAction* menuBarAction(QMenu* menu, QString text, std::function<void()> trigger);
		static QAction* menuBarAction(QMenu* menu, QString text, std::function<void()> trigger, QKeySequence shortcut);
		static QAction* menuBarSeparator(QMenu* menu);
		static QActionGroup* menuBarActionGroup(QMenu* menu, bool exclusive = true);

		// gui current bit flags
		bitset<256> gxe;
		// gui previous bit flags
		bitset<256> gex;

	private:
		QApplication* mroot;
		QWidget* mwid = nullptr;
		QGridLayout* mfrm;
		QHBoxLayout* mcnt;
		QHBoxLayout* mstatusb;
		QStatusBar* sbwid;
		QLabel* sbwidl;
		QWidget* sbwidc;
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
