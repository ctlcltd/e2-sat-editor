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

using namespace std;

#ifndef tab_h
#define tab_h
#include <Qt>
#include <QWidget>
#include <QHeaderView>
#include <QTreeWidget>
#include "e2db_gui.h"

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
		void trickySortByColumn(int column);
		void setTabId(int ttid);
		void save(bool saveas);
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
		string filename;
		bool _state_nwwr;
		bool _state_ovwr;
		pair<int, Qt::SortOrder> _state_sort;
};
}
#endif /* tab_h */
