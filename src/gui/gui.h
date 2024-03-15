/*!
 * e2-sat-editor/src/gui/gui.h
 *
 * @link https://github.com/ctlcltd/e2-sat-editor
 * @copyright e2 SAT Editor Team
 * @author Leonardo Laureti
 * @version 1.3.0
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
#include <QPushButton>

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
			FileInfo = 14,
			FilePrint = 12,
			FilePrintAll = 13,
			EditUndo = 21,
			EditRedo = 20,
			EditDelete = 26,
			EditSelectAll = 25,
			EditCut = 27,
			EditCopy = 28,
			EditPaste = 29,
			TabTreeEdit = 30,
			TabTreeDelete = 42,
			TabTreeFind = 53,
			TabTreeFindNext = 54,
			TabListEditTransponder = 31,
			TabListEditService = 32,
			TabListEditMarker = 33,
			TabListEditFavourite = 34,
			TabListEditPicon = 38,
			TabListDelete = 46,
			TabListSelectAll = 45,
			TabListCut = 47,
			TabListCopy = 48,
			TabListPaste = 49,
			TabListFind = 52,
			TabListFindNext = 57,
			TabListFindPrev = 56,
			TabListFindAll = 59,
			Transponders = 60,
			TunersetsSat = 61,
			TunersetsTerrestrial = 62,
			TunersetsCable = 63,
			TunersetsAtsc = 64,
			Picons = 69,
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
			ToolsImportM3U = 120,
			ToolsExportM3U = 130,
			ToolsExportHTML_current = 136,
			ToolsExportHTML_all = 137,
			ToolsExportHTML_index = 138,
			ToolsExportHTML_services = 139,
			ToolsExportHTML_bouquets = 140,
			ToolsExportHTML_userbouquets = 141,
			ToolsExportHTML_tunersets = 142,
			ToolsInspector = 0xfe,
			init = 0, // preset
			idle = -1 // preset
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
			GUI_CXE::FileInfo,
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
			GUI_CXE::ToolsImportM3U,
			GUI_CXE::ToolsExportM3U,
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
			Info = GUI_CXE::FileInfo,
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
			EditPicons = GUI_CXE::Picons,
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
			ImportM3U = GUI_CXE::ToolsImportM3U,
			ExportM3U = GUI_CXE::ToolsExportM3U,
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
			picons,
			channelBook
		};

		enum GUI_DPORTS {
			AllFiles = 0x0,
			Services = 0x1,
			Bouquets = 0x2,
			Userbouquets = 0x4,
			Tunersets = 0x8,
			CSV = 0x10,
			HTML = 0x20,
			M3U = 0x80,
			ConnectionProfile = 0x100
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
			bool info = false;
			bool update = false;
			int version = 0;
			int convert = 0;
		};

#if defined Q_OS_WASM || defined E2SE_DEMO
		struct gui_file
		{
			string path;
			string filename;
			string data;
			string mime;
			size_t size;
		};

		vector<gui_file> blobs;
#endif

		gui(int argc, char* argv[]);
		~gui();
		int exited();
		void settingsChanged();
		int newTab(string path = "");
		int openTab(TAB_VIEW ttv);
		int openTab(TAB_VIEW ttv, int arg);
		void closeTab(int index = -1);
		void closeAllTabs();
		string openFileDialog();
		string saveFileDialog(string path);
		vector<string> importFileDialog(gui::GUI_DPORTS gde);
		string exportFileDialog(GUI_DPORTS gde, string path, int& bit);
		string exportFileDialog(GUI_DPORTS gde, string path);
		void changeTabName(int ttid, string path = "");
		bool statusBarIsVisible();
		bool statusBarIsHidden();
		void statusBarShow();
		void statusBarHide();
		void statusBarToggle();
		void setStatusBar(status msg);
		void resetStatusBar(bool message = false);
		void fileOpen();
		void fileSave();
		void fileSaveAs();
		void fileImport();
		void fileExport();
		void fileInfo();
		void filePrint();
		void filePrintAll();
		void windowMinimize();
		void settingsDialog();
		void aboutDialog();
		void linkToRepository(int page = 0);
		void linkToWebsite(int page = 0);
		void linkToOnlineHelp(int page = 0);
		void checkUpdate();
		virtual void autoCheckUpdate();
		bool getFlag(GUI_CXE bit);
		void setFlag(GUI_CXE bit, bool flag);
		bitset<256> getFlags();
		void setFlags(bitset<256> bits);
		void setFlags(vector<int> bits);
		void setFlags(vector<int> bits, bool flag);
		void setFlags(int preset);
		int getTabEditActionFlag(GUI_CXE bit);
		void setTabEditActionFlag(GUI_CXE bit, bool flag);
		void updateMenu();
		QLocale getLocale();
		void errorMessage(QString title, QString message);
		void demoMessage();

		static QString getFileFormatName(int ver);

		e2se_gui::theme* theme;

	protected:
		void layout();
		void menuBarLayout();
		void tabStackerLayout();
		void statusBarLayout();
		void themeChanged();
		void clipboardDataChanged();
		void initSettings();
		void updateSettings();
		void resetSettings();
		void editAction(GUI_CXE bit);
		void tabChanged(int index);
		void tabMoved(int from, int to);
		void tabViewChanged(TAB_VIEW ttv);
		void tabViewChanged(TAB_VIEW ttv, int arg);
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
		QPushButton* sbwidi;
		QTabWidget* twid;
		QMenuBar* menu;
		QMenu* mwind;
		QActionGroup* mwtabs;
		string ifp;
		unordered_map<int, QAction*> gmenu;
		unordered_map<int, QAction*> ttmenu;
		unordered_map<int, tab*> ttabs;
};
}
#endif /* gui_h */
