/*!
 * e2-sat-editor/src/gui_qt6.h
 *
 * @author Leonardo Laureti
 * @version 0.1
 * @license MIT License
 */

#include <iostream>
#include <QWidget>
#include <QStatusBar>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QHeaderView>
#include <QTreeWidget>
#include "commons.h"
#include "e2db.h"


using namespace std;

#ifndef gui_qt6_h
#define gui_qt6_h
namespace e2se_gui
{
class tab
{
	public:
		tab(QWidget* wid);
		void newFile();
		bool load(string filename = "");
		void populate();
		void trickySortByColumn(int column);
		void loadSeeds();
		QWidget* widget;
	private:
		QWidget* cwid;
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

class gui
{
	public:
		gui(int argc, char* argv[]);
		void root();
		void tabCtl();
		void statusCtl();
		void newTab();
		void closeTab(int index);
		void tabChanged(int index);
	private:
		QApplication* mroot;
		QWidget* mwid;
		QVBoxLayout* mfrm;
		QHBoxLayout* mcnt;
		QHBoxLayout* mstatusb;
		QStatusBar* sbwid;
		QTabWidget* twid;
};
}
#endif
