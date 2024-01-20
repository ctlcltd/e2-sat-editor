/*!
 * e2-sat-editor/src/gui/tab.h
 *
 * @link https://github.com/ctlcltd/e2-sat-editor
 * @copyright e2 SAT Editor Team
 * @author Leonardo Laureti
 * @version 1.1.1
 * @license MIT License
 * @license GNU GPLv3 License
 */

#include <functional>
#include <string>
#include <vector>
#include <map>
#include <unordered_map>
#include <bitset>

using std::string, std::pair, std::vector, std::map, std::unordered_map, std::bitset;

#ifndef tab_h
#define tab_h
#include <Qt>
#include <QTimer>
#include <QApplication>
#include <QWidget>
#include <QGridLayout>
#include <QToolBar>
#include <QList>

#include "../logger/logger.h"
#include "theme.h"
#include "gui.h"
#include "dataHandler.h"
#include "ftpHandler.h"
#include "tools.h"
#include "mainView.h"
#include "viewAbstract.h"

namespace e2se_gui
{
class tab : protected e2se::log_factory
{
	Q_DECLARE_TR_FUNCTIONS(tab)

	public:

		const int STATUSBAR_MESSAGE_DELAY = 1500;
		const int STATUSBAR_MESSAGE_TIMEOUT = 5000;

		enum TOOLS_FILE {
			tools_csv,
			tools_html
		};

		tab(gui* gid, QWidget* cwid);
		virtual ~tab();
		bool isChild();
		bool hasChildren();
		vector<tab*> children();
		void addChild(tab* child);
		void removeChild(tab* child);
		int getTabId();
		void setTabName(string ttname);
		string getTabName();
		gui::TAB_VIEW getTabView();
		string getFilename();
		void settingsChanged();
		void themeChanged();
		void clipboardDataChanged();
		void tabSwitched();
		void tabChangeName(string path = "");
		void setFlag(gui::GUI_CXE bit, bool flag);
		bool getFlag(gui::GUI_CXE bit);
		void setFlags(int preset);
		void storeFlags();
		void retrieveFlags();
		void updateToolBars();
		bool statusBarIsVisible();
		bool statusBarIsHidden();
		void setStatusBar(gui::status msg);
		void resetStatusBar(bool messsage = false);
		void viewMain();
		void viewTransponders(tab* parent);
		void viewTunersets(tab* parent, int ytype);
		void viewPicons(tab* parent);
		void viewChannelBook(tab* parent);
		void load();
		void reset();
		void actionCall(int bit);
		void newFile();
		void openFile();
		bool readFile(string path = "");
		void saveFile(bool saveas);
		void importFile();
		void exportFile();
		void printFile(bool all);
		void infoFile();
		void settingsDialog();
		void linkToRepository(int page = 0);
		void linkToWebsite(int page = 0);
		void linkToOnlineHelp(int page = 0);
		void updateIndex();
		QTimer* statusBarMessage(QString title);
		void statusBarMessage(QTimer* timer);
		bool saveQuestion(QString title, QString text);
		bool removeQuestion(QString title, QString text);
		void infoMessage(QString title);
		void infoMessage(QString title, QString text);
		void errorMessage(QString title, QString text);
		void demoMessage();

		QWidget* widget = nullptr;
		dataHandler* data = nullptr;
		ftpHandler* ftph = nullptr;
		e2se_gui::tools* tools = nullptr;

	protected:
		void layout();
		void toolsInspector();
		void toolsImportFromFile(TOOLS_FILE ftype, e2db::FCONVS fci);
		void toolsExportToFile(TOOLS_FILE ftype, e2db::FCONVS fco);
		void ftpComboItems();
		void ftpComboChanged(int index);
		void ftpConnect();
		void ftpDisconnect();
		void ftpUpload();
		void ftpDownload();
		void ftpReloadStb();
		void ftpStbConnectingNotify();
		void ftpStbDisconnectingNotify();
		void ftpStbHandlingNotify();
		void ftpStbReloadingNotify();
		void ftpStbDownloadNotify(int files_count);
		void ftpStbDownloadNotify(string filename);
		void ftpStbUploadNotify(int files_count);
		void ftpStbUploadNotify(string filename);
		void ftpStbReloadSuccessNotify();
		void ftpStbReloadErrorNotify();
		void ftpStbConnectSuccessNotify();
		void ftpStbConnectErrorNotify();
		void ftpStbDisconnectSuccessNotify();
		void ftpStbDisconnectErrorNotify();
		void ftpGenericError(string context);
		void loadSeeds();

		static QToolBar* toolBar(int type);
		static QAction* toolBarAction(QToolBar* toolbar, QString text, std::function<void()> trigger);
		static QAction* toolBarAction(QToolBar* toolbar, QString text, pair<e2se_gui::theme*, QString> icon, std::function<void()> trigger);
		static QAction* toolBarAction(QToolBar* toolbar, QString text, std::function<void()> trigger, QKeySequence shortcut);
		static QAction* toolBarAction(QToolBar* toolbar, QString text, pair<e2se_gui::theme*, QString> icon, std::function<void()> trigger, QKeySequence shortcut);
		static QWidget* toolBarWidget(QToolBar* toolbar, QString text, QWidget* widget);
		static QWidget* toolBarWidget(QToolBar* toolbar, QString text, QWidget* widget, std::function<void()> trigger);
		static QAction* toolBarSeparator(QToolBar* toolbar);
		static QWidget* toolBarSpacer(QToolBar* toolbar);
		void toolBarStyleSheet();

		e2se_gui::theme* theme;
		// tab view
		gui::TAB_VIEW ttv;
		viewAbstract* view;
		QGridLayout* root;
		QToolBar* top_toolbar;
		QToolBar* bottom_toolbar;
		QComboBox* ftp_combo;

		unordered_map<string, e2se_ftpcom::ftpcom::ftpcom_file> ftp_files;
		unordered_map<string, string> ftp_errors;
		unordered_map<string, e2db::e2db_file> files;

		// stored gui bit flags
		bitset<256> gxe;

	private:
		gui* gid;
		QWidget* cwid;
		tab* parent = nullptr;
		bool child = false;
		vector<tab*> childs;
		unordered_map<int, QAction*> tbars;
		int ttid = -1;
		string ttname;
};
}
#endif /* tab_h */
