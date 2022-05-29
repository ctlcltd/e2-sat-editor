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

using std::string, std::pair, std::vector, std::map;

#ifndef channelBook_h
#define channelBook_h
#include <QWidget>
#include <QString>
#include <QHBoxLayout>
#include <QListWidget>

#include "../logger.h"
#include "e2db_gui.h"

namespace e2se_gui
{
class channelBook : protected e2se::log_factory
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
		QSettings* sets;
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
		// view selector
		// 0 tabv | list
		// 1 tree | list
		// 2 list
		int vx;
};
}
#endif /* channelBook_h */
