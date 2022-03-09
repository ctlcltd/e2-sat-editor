/*!
 * e2-sat-editor/src/gui/gui.h
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
#include <filesystem>

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
#include <QActionGroup>

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
		void tabClicked(int index);
		void tabMoved(int from, int to);
		void open();
		string openFileDialog();
		string saveFileDialog(string filename);
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
		QActionGroup* mwtabs;
		int ttidx;
		unordered_map<int, tab*> ttabs;
		unordered_map<int, QAction*> ttmenu;
};
}
#endif /* gui_h */
