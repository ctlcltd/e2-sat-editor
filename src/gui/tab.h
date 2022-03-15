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

#include <string>
#include <unordered_map>

using namespace std;

#ifndef tab_h
#define tab_h
#include <Qt>
#include <QWidget>
#include <QHeaderView>
#include <QTreeWidget>
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
		void newFile();
		void open();
		void addChannel();
		bool load(string filename = "");
		void populate();
		void listItemChanged();
		void visualReindexList();
		void trickySortByColumn(int column);
		void setTabId(int ttid);
		void updateListIndex();
		void save(bool saveas);
		void loadSeeds();
		void allowDnD();
		void disallowDnD();
		QWidget* widget;
	protected:
		unordered_map<string, vector<pair<int, string>>> index;
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
		e2db* dbih;
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
