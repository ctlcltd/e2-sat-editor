/*!
 * e2-sat-editor/src/gui/tab.h
 *
 * @author Leonardo Laureti
 * @version 0.1
 * @license MIT License
 */

#include <string>
#include <vector>
#include <map>

using namespace std;

#ifndef tab_h
#define tab_h
#include <Qt>
#include <QWidget>
#include <QHeaderView>
#include <QTreeWidget>
#include "../e2db.h"

namespace e2se_gui
{
class gui;

class tab
{
	public:
		tab(gui* gid, QWidget* wid, string filename);
		void newFile();
		void open();
		bool load(string filename = "");
		void populate();
		void trickySortByColumn(int column);
		void setTabId(int ttid);
		void save();
		void loadSeeds();
		QWidget* widget;
	private:
		gui* gid;
		QWidget* cwid;
		int ttid;
		e2db* dbih;
		QTreeWidget* bouquets_tree;
		QTreeWidget* list_tree;
		QHeaderView* lheaderv;
		pair<int, Qt::SortOrder> _state_sort;
};
}
#endif /* tab_h */
