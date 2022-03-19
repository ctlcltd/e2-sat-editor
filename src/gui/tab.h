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

using namespace std;

#ifndef tab_h
#define tab_h
#include <Qt>
#include <QWidget>
#include <QHeaderView>
#include <QTreeWidget>
#include <QList>
#include <QAction>
#include <QLabel>
#include "e2db_gui.h"
#include "TreeEventObserver.h"

namespace e2se_gui
{
class gui;

class tab
{
	public:
		tab(gui* gid, QWidget* wid, string filename);
		~tab();
		void newFile();
		void openFile();
		bool readFile(string filename = "");
		void saveFile(bool saveas);
		void addChannel();
		void load();
		void populate();
		void bouquetsItemChanged(QTreeWidgetItem* current);
		void listItemChanged();
		void visualReindexList();
		void trickySortByColumn(int column);
		void allowDnD();
		void disallowDnD();
		void putChannels(vector<QString>);
		void updateListIndex();
		void setCounters(bool userbouquet = false);
		void setTabId(int ttid);
		void initialize();
		void destroy();
		void loadSeeds();
		QWidget* widget;
	protected:
		unordered_map<string, vector<pair<int, string>>> index;
		unordered_map<string, QList<QTreeWidgetItem*>> cache;
	private:
		struct actions {
			QAction* bouquets_newbs;
			QAction* list_addch;
			QAction* list_newch;
			QLabel* list_dnd;
		};
		gui* gid;
		QWidget* cwid;
		int ttid;
		e2db* dbih = nullptr;
		TreeEventObserver* list_evt;
		QTreeWidget* bouquets_tree;
		QTreeWidget* list_tree;
		QWidget* list_wrap;
		QHeaderView* lheaderv;
		actions* ats;
		string filename;
		bool _state_nwwr;
		bool _state_ovwr;
		bool _state_dnd;
		bool _state_changed;
		string _state_curr;
		pair<int, Qt::SortOrder> _state_sort;
};
}
#endif /* tab_h */
