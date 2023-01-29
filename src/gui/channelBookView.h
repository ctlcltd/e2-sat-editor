/*!
 * e2-sat-editor/src/gui/channelBookView.h
 *
 * @link https://github.com/ctlcltd/e2-sat-editor
 * @copyright e2 SAT Editor Team
 * @author Leonardo Laureti
 * @version 0.3
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
#include <QListWidget>
#include <QTreeWidget>

#include "viewAbstract.h"

namespace e2se_gui
{
class channelBookView : public viewAbstract
{
	public:
		struct __state
		{
			// tree current
			string curr;
			// view selector
			// 0 tabv | list
			// 1 tree | list
			// 2 list
			int vx;
			// filter for stype
			int sy = -1;
			// list_tree sort
			pair<int, Qt::SortOrder> sort;
		};

		enum views {
			Services,
			Bouquets,
			Positions,
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
			chtxp,
			chpos,
			chsys
		};

		channelBookView(dataHandler* data, int stype);
		channelBookView(tab* tid, QWidget* cwid, dataHandler* data);
		void load();
		void reset();
		void filterChanged(bool enabled);
		void listItemCut() {};
		void listItemCopy(bool cut = false);
		void listItemPaste() {};
		void listItemDelete() {};
		vector<QString> getSelected();

		__state currentState() { return this->state; };

	protected:
		void layout();
		void sideLayout();
		void populate();
		void stacker(int vv);
		void sideRowChanged(int index);
		void listItemSelectionChanged();
		void showListEditContextMenu(QPoint &pos);
		void updateFlags();

		QListWidget* lwid;
		QTabBar* tabv;
		map<string, vector<pair<int, string>>> index;

		__state state;
		__action action;

	private:
		QWidget* cwid;

};
}
#endif /* channelBookView_h */
