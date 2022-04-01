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
#include <unordered_map>

using std::string, std::pair, std::unordered_map;

#ifndef tab_h
#define tab_h
#include <Qt>
#include <QWidget>
#include <QHeaderView>
#include <QTreeWidget>
#include <QList>
#include <QAction>
#include <QLabel>

#include "../logger.h"
#include "e2db_gui.h"
#include "BouquetsEventHandler.h"
#include "ListEventHandler.h"
#include "ListEventObserver.h"

namespace e2se_gui
{
class gui;

class tab : protected e2se::log_factory
{
	public:
		enum LIST_EDIT_ATS {
			Cut,
			Copy,
			Paste,
			Delete,
			SelectAll
		};

		tab(gui* gid, QWidget* wid, string filename);
		~tab();
		void newFile();
		void openFile();
		bool readFile(string filename = "");
		void saveFile(bool saveas);
		void addChannel();
		void addService();
		void editService();
		void delService();
		void load();
		void populate();
		void bouquetsItemChanged(QTreeWidgetItem* current);
		void listItemChanged();
		void visualReindexList();
		void trickySortByColumn(int column);
		void allowDnD();
		void disallowDnD();
		void reharmDnD();
		void listItemCut();
		void listItemCopy(bool cut = false);
		void listItemPaste();
		void listItemDelete();
		void listItemSelectAll();
		void listItemAction(int action);
		void putChannels(vector<QString> channels, string chlist);
		void updateListIndex();
		void showListEditContextMenu(QPoint &pos);
		void setCounters(bool channels = false);
		void setTabId(int ttid);
		void initialize();
		void destroy();
		void profileComboChanged(int index);
		void ftpConnect();
		void loadSeeds();
		QWidget* widget;
	protected:
		unordered_map<string, QList<QTreeWidgetItem*>> cache;
	private:
		struct ats {
			QAction* bouquets_newbs;
			QAction* list_addch;
			QAction* list_newch;
			QPushButton* list_dnd;
		} action;
		struct sts {
			bool nwwr;
			bool ovwr;
			bool dnd;
			bool changed;
			int ti;
			string curr;
			pair<int, Qt::SortOrder> sort;
		} state;
		gui* gid;
		QWidget* cwid;
		int ttid;
		e2db* dbih = nullptr;
		BouquetsEventHandler* bouquets_evth;
		ListEventHandler* list_evth;
		ListEventObserver* list_evto;
		QTreeWidget* bouquets_tree;
		QTreeWidget* list_tree;
		QWidget* list_wrap;
		QHeaderView* lheaderv;
		string filename;
};
}
#endif /* tab_h */
