/*!
 * e2-sat-editor/src/gui/tab.h
 *
 * @author Leonardo Laureti
 * @version 0.1
 * @license MIT License
 */

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <QWidget>
#include <QStatusBar>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QHeaderView>
#include <QTreeWidget>
#include "../e2db.h"
#include "gui.h"

using namespace std;

#ifndef tab_h
namespace e2se_gui
{
class tab
{
	public:
		tab(gui* gid, QWidget* wid, string filename);
		void newFile();
		void open();
		bool load(string filename = "");
		void populate();
		void trickySortByColumn(int column);
		void setIndex(int index);
		void loadSeeds();
		QWidget* widget;
	private:
		gui* gid;
		QWidget* cwid;
		int tid;
		e2db_parser* temp_parser;
		map<string, e2db_parser::transponder> temp_transponders;
		map<string, e2db_parser::service> temp_channels;
		pair<map<string, e2db_parser::bouquet>, map<string, e2db_parser::userbouquet>> temp_bouquets;
		map<string, vector<pair<int, string>>> temp_index;
		QTreeWidget* bouquets_tree;
		QTreeWidget* list_tree;
		QHeaderView* lheaderv;
		pair<int, Qt::SortOrder> _state_sort;
};
}
#define tab_h
#endif /* tab_h*/
