/*!
 * e2-sat-editor/src/gui/tab.h
 *
 * @link https://github.com/ctlcltd/e2-sat-editor
 * @copyright e2 SAT Editor Team
 * @author Leonardo Laureti
 * @version 0.2
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
	public:

		const int STATUSBAR_MESSAGE_DELAY = 1500;
		const int STATUSBAR_MESSAGE_TIMEOUT = 5000;

		enum TOOLS_FILE {
			tools_csv,
			tools_html
		};

		tab(gui* gid, QWidget* cwid, e2se::logger::session* log);
		~tab();
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
		void tabSwitched();
		void tabChangeName(string filename = "");
		void setFlag(gui::GUI_CXE bit, bool flag);
		void setFlag(gui::GUI_CXE bit);
		bool getFlag(gui::GUI_CXE bit);
		void storeFlags();
		void retrieveFlags();
		void updateToolBars();
		bool statusBarIsVisible();
		bool statusBarIsHidden();
		void setStatusBar(gui::status msg);
		void resetStatusBar();
		void viewMain();
		void viewTunersets(tab* parent, int ytype);
		void viewChannelBook(tab* parent);
		void load();
		void reset();
		void actionCall(int action);
		void newFile();
		void openFile();
		bool readFile(string filename = "");
		void saveFile(bool saveas);
		void importFile();
		void exportFile();
		void exportFile(QTreeWidgetItem* item);
		void printFile(bool all);
		void updateIndex();
		QTimer* statusBarMessage(string title);
		void statusBarMessage(QTimer* timer);
		bool saveQuestion(QString title, QString text);
		void infoMessage(QString title);
		void infoMessage(QString title, QString text);
		void errorMessage(QString title, QString text);

		QWidget* widget;
		dataHandler* data = nullptr;
		ftpHandler* ftph = nullptr;
		e2se_gui::tools* tools = nullptr;

	protected:
		void layout();
		void toolsInspector();
		void toolsImportFromFile(TOOLS_FILE ftype, e2db::FCONVS fci);
		void toolsExportToFile(TOOLS_FILE ftype, e2db::FCONVS fco);
		void profileComboChanged(int index);
		bool ftpHandle();
		void ftpConnect();
		void ftpDisconnect();
		void ftpUpload();
		void ftpDownload();
		void loadSeeds();

		static QToolBar* toolBar(int type);
		static QAction* toolBarAction(QToolBar* toolbar, QString text, std::function<void()> trigger);
		static QAction* toolBarAction(QToolBar* toolbar, QString text, pair<e2se_gui::theme*, QString> icon, std::function<void()> trigger);
		static QAction* toolBarAction(QToolBar* toolbar, QString text, std::function<void()> trigger, QKeySequence shortcut);
		static QAction* toolBarAction(QToolBar* toolbar, QString text, pair<e2se_gui::theme*, QString> icon, std::function<void()> trigger, QKeySequence shortcut);
		static QWidget* toolBarWidget(QToolBar* toolbar, QWidget* widget);
		static QAction* toolBarSeparator(QToolBar* toolbar);
		static QWidget* toolBarSpacer(QToolBar* toolbar);
		void toolBarStyleSheet();

		QGridLayout* root;
		QToolBar* top_toolbar;
		QToolBar* bottom_toolbar;
		e2se_gui::theme* theme;
		// tab view
		gui::TAB_VIEW ttv;
		viewAbstract* view;

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
