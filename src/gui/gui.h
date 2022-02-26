/*!
 * e2-sat-editor/src/gui/gui.h
 *
 * @author Leonardo Laureti
 * @version 0.1
 * @license MIT License
 */

#include <filesystem>
#include <string>
#include <unordered_map>

using namespace std;

#ifndef gui_h
#define gui_h
#include <QApplication>
#include <QWidget>
#include <QMenuBar>
#include <QStatusBar>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QTabWidget>

namespace e2se_gui
{
class tab;

class gui
{
	public:
		gui(int argc, char* argv[]);
		void root();
		void menuCtl();
		void tabCtl();
		void statusCtl();
		int newTab(string filename);
		void closeTab(int index);
		void closeAllTabs();
		void tabChanged(int index);
		void open();
		string openFileDialog();
		void tabChangeName(int ttid, string filename);
		void save();
		void settings();
		void about();
	private:
		QApplication* mroot;
		QWidget* mwid;
		QGridLayout* mfrm;
		QHBoxLayout* mcnt;
		QHBoxLayout* mstatusb;
		QStatusBar* sbwid;
		QTabWidget* twid;
		QMenuBar* menu;
		QMenu* mwind;
		int ttidx;
		unordered_map<int, tab*> ttabs;
};
}
#endif /* gui_h */
