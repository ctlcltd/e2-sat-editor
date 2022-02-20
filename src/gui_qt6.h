/*!
 * e2-sat-editor/src/gui_qt6.h
 *
 * @author Leonardo Laureti
 * @version 0.1
 * @license MIT License
 */

#include <iostream>
#include <QWidget>
#include <QHeaderView>
#include <QTreeWidget>
#include "commons.h"
#include "e2db.h"


using namespace std;

#ifndef gui_qt6_h
#define gui_qt6_h
class gui
{
	public:
		void root(int argc, char* argv[]);
		void main(QWidget& mwid);
		void tab(QWidget& ttab);
		void newFile();
		bool load(string filename = "");
		void populate();
		void trickySortByColumn(int column);
		void loadSeeds();
	private:
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
#endif
