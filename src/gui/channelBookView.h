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
class channelBookView : public viewAbstract
{
	public:
		enum views {
			Services,
			Bouquets,
			Satellites,
			Providers,
			Resolution,
			Encryption,
			A_Z
		};

		enum ITEM_ROW_ROLE {
			x,
			chnum,
			chname,
			chtype,
			chpname,
			chsys,
			chtxp,
			chpos
		};

		channelBookView(dataHandler* data, e2se::logger::session* log);
		channelBookView(tab* tid, QWidget* cwid, dataHandler* data, e2se::logger::session* log);
		void load();
		void reset();
		void listItemCut() {};
		void listItemCopy(bool cut = false);
		void listItemPaste() {};
		void listItemDelete() {};
		vector<QString> getSelected();

		struct __state
		{
			// toggle tree events
			bool evt;
			// tree current
			string curr;
			// view selector
			// 0 tabv | list
			// 1 tree | list
			// 2 list
			int vx;
			// list_tree sort
			pair<int, Qt::SortOrder> sort;
		} state;
	protected:
		void layout();
		void sideLayout();
		void populate();
		void sideRowChanged(int index);
		void stacker(int vv);

		QListWidget* lwid;
		QHBoxLayout* awid;
		QTabBar* tabv;
		map<string, vector<pair<int, string>>> index;
	private:
		QWidget* cwid;

};
}
#endif /* channelBookView_h */
