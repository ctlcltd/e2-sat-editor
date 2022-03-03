/*!
 * e2-sat-editor/src/gui/channelBook.h
 *
 * @author Leonardo Laureti
 * @version 0.1
 * @license MIT License
 */

using namespace std;

#ifndef channelBook_h
#define channelBook_h
#include <QWidget>
#include <QListWidget>
#include <QStackedWidget>
#include <QTabWidget>

namespace e2se_gui
{
class channelBook
{
    public:
    	channelBook();
		void side();
		void stacked();
		void listView(int vv, QLayout* layout = nullptr, QTabWidget* tw = nullptr, string tn = "");
		void treeView(int vv);
		void vtabView(int vv);
		void sideRowChanged(int index);
		void populate();
		QWidget* widget;
	protected:
		QListWidget* lwid;
		QStackedWidget* swid;
		enum views {
			Services,
			Bouquets,
			Satellites,
			Providers,
			Resolutions,
			Encryptions,
			A_Z
		};
	};
}
#endif
