/*!
 * e2-sat-editor/src/gui/tab.h
 *
 * @link https://github.com/ctlcltd/e2-sat-editor
 * @copyright e2 SAT Editor Team
 * @author Leonardo Laureti
 * @version 0.1
 * @license MIT License
 * @license GNU GPLv3 License
 */

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <unordered_map>
#include <bitset>

using std::string, std::pair, std::vector, std::map, std::unordered_map, std::bitset;

#ifndef tab_h
#define tab_h
#include <Qt>
#include <QWidget>
#include <QGridLayout>
#include <QList>

#include "../logger/logger.h"
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
		void tabSwitched();
		void tabChangeName(string filename = "");
		void setFlag(gui::GUI_CXE bit, bool flag);
		void setFlag(gui::GUI_CXE bit);
		bool getFlag(gui::GUI_CXE bit);
		void storeFlags();
		void retrieveFlags();
		void setStatus(gui::STATUS status);
		void resetStatus();
		void viewMain();
		void viewTunersets(tab* parent, int ytype);
		void viewChannelBook(tab* parent);
		void load();
		void reset();
		void layout();
		void newFile();
		void openFile();
		bool readFile(string filename = "");
		void saveFile(bool saveas);
		void importFile();
		void exportFile();
		void exportFile(QTreeWidgetItem* item);
		void printFile(bool all);
		void toolsInspector();
		void toolsImportFromFile(TOOLS_FILE ftype, e2db::FCONVS fci);
		void toolsExportToFile(TOOLS_FILE ftype, e2db::FCONVS fco);
		void actionCall(int action);
		void updateBouquetsIndex();
		void updateChannelsIndex();
		void setPendingUpdateChannelsIndex();
		void unsetPendingUpdateChannelsIndex();
		void profileComboChanged(int index);
		bool ftpHandle();
		void ftpConnect();
		void ftpUpload();
		void ftpDownload();
		void loadSeeds();
		QWidget* widget;
		dataHandler* data = nullptr;
		ftpHandler* ftph = nullptr;
		e2se_gui::tools* tools = nullptr;
	protected:
		QGridLayout* root;
		// tab view
		gui::TAB_VIEW ttv;
		// stored gui bit flags
		bitset<256> gxe;
		// tunersets current type
		int ty;
		// pending update channels index
		bool chx_pending;
		mainView* main;
		viewAbstract* view;
	private:
		gui* gid;
		QWidget* cwid;
		tab* parent = nullptr;
		bool child = false;
		vector<tab*> childs;
		int ttid = -1;
		string ttname;
		e2db* dbih = nullptr;
};
}
#endif /* tab_h */
