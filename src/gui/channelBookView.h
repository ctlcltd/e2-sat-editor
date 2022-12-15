/*!
 * e2-sat-editor/src/gui/channelBookView.h
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

#ifndef channelBookView_h
#define channelBookView_h
#include <QWidget>
#include <QString>
#include <QHBoxLayout>
#include <QListWidget>
#include <QTreeWidget>

#include "viewAbstract.h"

namespace e2se_gui
{
class gui;
class tab;

class channelBookView : public viewAbstract
{
	public:
		enum ITEM_ROW_ROLE {
			x,
			chnum,
			chname,
			chtype,
			chpname,
			chtxp,
			chpos,
			chsys
		};

		channelBookView(e2db* dbih, e2se::logger::session* log);
		channelBookView(gui* gid, tab* twid, QWidget* wid, e2se::logger::session* log);
		void side();
		void layout();
		void load();
		void populate();
		void sideRowChanged(int index);
		void stacker(int vv);
		vector<QString> getSelected();
		QSettings* sets;
	protected:
		enum views {
			Services,
			Bouquets,
			Satellites,
			Providers,
			Resolution,
			Encryption,
			A_Z
		};

		QListWidget* lwid;
		QHBoxLayout* awid;
		QTabBar* tabv;
		map<string, vector<pair<int, string>>> data;
	private:
		gui* gid;
		tab* twid;
		QWidget* cwid;
		// view selector
		// 0 tabv | list
		// 1 tree | list
		// 2 list
		int vx;
};
}
#endif /* channelBookView_h */
