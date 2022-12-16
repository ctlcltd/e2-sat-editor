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
#include "e2db_gui.h"
#include "ftpcom_gui.h"
#include "mainView.h"
#include "viewAbstract.h"
#include "tools.h"

namespace e2se_gui
{
class gui;

class tab : protected e2se::log_factory
{
	public:
		tab(gui* gid, QWidget* wid, e2se::logger::session* log);
		~tab();
		bool isChild();
		bool hasChildren();
		vector<tab*> children();
		void addChild(tab* child);
		void removeChild(tab* child);
		void setTabId(int ttid);
		int getTabId();
		void setTabName(string ttname);
		string getTabName();
		int getTabView();
		string getFilename();
		void tabSwitched();
		void tabChangeName(string filename = "");
		void viewMain();
		void viewTunersets(tab* parent, int ytype);
		void viewChannelBook(tab* parent);
		void load();
		void reset();
		void reset(e2db* dbih);
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
		mainView* main;
		viewAbstract* view;
		e2db* dbih = nullptr;
		e2se_gui_tools::tools* tools = nullptr;

		struct sts
		{
			// new file
			bool nwwr;
			// overwrite file
			bool ovwr;
			// file name
			string filename;
			// stored gui connector flags
			bitset<256> gxe;
			// tools tunersets current type
			int ty;
		} state;
	protected:
		bool child = false;
		vector<tab*> childs;
	private:
		gui* gid;
		QWidget* cwid;
		tab* parent = nullptr;
		int ttid = -1;
		string ttname;
		ftpcom* ftph = nullptr;
		// tab view
		int ttv; // relation with gui::TAB_VIEW
};
}
#endif /* tab_h */
