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

using std::string, std::pair, std::vector, std::map, std::unordered_map;

#ifndef tab_h
#define tab_h
#include <Qt>
#include <QWidget>
#include <QGridLayout>
#include <QList>

#include "../logger/logger.h"
#include "e2db_gui.h"
#include "ftpcom_gui.h"
#include "mainView.h"
#include "tunersetsView.h"
#include "viewAbstract.h"
#include "tools.h"

namespace e2se_gui
{
class gui;

class tab : protected e2se::log_factory
{
	public:
		// relation with gui::GUI_CXE
		enum LIST_EDIT_ATS {
			Cut = 0x00010000, // GUI_CXE::TabListCut
			Copy = 0x00020000, // GUI_CXE::TabListCopy
			Paste = 0x00040000, // GUI_CXE::TabListPaste
			Delete = 0x00004000, // GUI_CXE::TabListDelete
			SelectAll = 0x00008000 // GUI_CXE::TabListSelectAll
		};

		tab(gui* gid, QWidget* wid, e2se::logger::session* log);
		~tab();
		bool isChild();
		bool hasChildren();
		vector<tab*> children();
		void setTabId(int ttid);
		int getTabId();
		string getFilename();
		void tabSwitched();
		void tabChangeName(string filename = "");
		void viewMain();
		void viewTunersets(tab* parent, int ytype);
		void load();
		void layout();
		void newFile();
		void openFile();
		bool readFile(string filename = "");
		void saveFile(bool saveas);
		void importFile();
		void exportFile();
		void exportFile(QTreeWidgetItem* item);
		void printFile(bool all);
		void actionCall(int action);
		void updateBouquetsIndex();
		void updateChannelsIndex();
		void profileComboChanged(int index);
		bool ftpHandle();
		void ftpConnect();
		void ftpUpload();
		void ftpDownload();
		void loadSeeds();
		QGridLayout* root;
		QWidget* widget;
		e2se_gui_tools::tools* tools;
		mainView* main;
		viewAbstract* view;
		e2db* dbih = nullptr;

		struct sts
		{
			// new file
			bool nwwr;
			// overwrite file
			bool ovwr;
			// stored gui connector flags
			int gxe;
			// tools tunerset shown
			bool tunersets;
			// tools tunersets current type
			int ty;
		} state;
	protected:
		bool child = false;
		vector<tab*> childs;
	private:
		gui* gid;
		QWidget* cwid;
		int ttid = -1;
		ftpcom* ftph = nullptr;
		string filename;
		// tab view
		int ttv; // relation with gui::TAB_VIEW
};
}
#endif /* tab_h */
