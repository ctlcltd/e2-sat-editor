/*!
 * e2-sat-editor/src/gui_qt6.h
 *
 * @author Leonardo Laureti
 * @version 0.1
 * @license MIT License
 */

#include <iostream>
#include <QApplication>
#include <QWidget>
#include <QScreen>
#include <QGridLayout>
#include <QGroupBox>
#include <QTreeWidget>
#include <QToolBar>

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
		void populate(QTreeWidgetItem& item);
	private:
		QTreeWidget* bouquets_tree;
		QTreeWidget* list_tree;
};
#endif
