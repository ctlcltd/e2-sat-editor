/*!
 * e2-sat-editor/src/gui/channelBook.h
 *
 * @link https://github.com/ctlcltd/e2-sat-editor
 * @copyright e2 SAT Editor Team
 * @author Leonardo Laureti
 * @version 0.1
 * @license MIT License
 * @license GNU GPLv3 License
 */

#include <string>
#include <vector>
#include <map>

using namespace std;

#ifndef channelBook_h
#define channelBook_h
#include <QWidget>
#include <QString>
#include <QHBoxLayout>
#include <QListWidget>
#include "e2db_gui.h"

namespace e2se_gui
{
class channelBook
{
	public:
		channelBook(e2db* dbih);
		void side();
		void layout();
		void sideRowChanged(int index);
		void stacker(int vv);
		void populate();
		void trickySortByColumn(int column);
		vector<QString> getSelected();
		QWidget* widget;
	protected:
		QListWidget* lwid;
		QHBoxLayout* awid;
		QTreeWidget* list;
		QTreeWidget* tree;
		QTabBar* tabv;
		enum views {
			Services,
			Bouquets,
			Satellites,
			Providers,
			Resolution,
			Encryption,
			A_Z
		};
		map<string, vector<pair<int, string>>> data;
	private:
		e2db* dbih;
		int vx;
};
}
#endif /* channelBook_h */
