/*!
 * e2-sat-editor/src/gui/gui.h
 *
 * @author Leonardo Laureti
 * @version 0.1
 * @license MIT License
 */

#include <iostream>
#include <string>
#include <QApplication>
#include <QWidget>
#include <QStatusBar>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QHeaderView>
#include <QTreeWidget>
#include <QTabWidget>

using namespace std;

#ifndef gui_h
namespace e2se_gui
{
class gui
{
	public:
		gui(int argc, char* argv[]);
		void root();
		void tabCtl();
		void statusCtl();
		int newTab(string filename);
		void closeTab(int index);
		void tabChanged(int index);
		void open();
		string openFileDialog();
		void tabChangeName(int index, string filename);
		void settings();
	private:
		QApplication* mroot;
		QWidget* mwid;
		QGridLayout* mfrm;
		QHBoxLayout* mcnt;
		QHBoxLayout* mstatusb;
		QStatusBar* sbwid;
		QTabWidget* twid;
};
}
#define gui_h
#endif /* gui_hpp */
