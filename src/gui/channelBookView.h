/*!
 * e2-sat-editor/src/gui/channelBookView.h
 *
 * @link https://github.com/ctlcltd/e2-sat-editor
 * @copyright e2 SAT Editor Team
 * @author Leonardo Laureti
 * @version 1.2.0
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
	Q_DECLARE_TR_FUNCTIONS(channelBookView)

	public:

		static const int TSV_TABS = 7;

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
			// pending update from tab
			bool tab_pending = false;
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
		~channelBookView();
		void load();
		void reset();
		void filterChanged(bool enabled);
		void listItemCut() {}
		void listItemCopy(bool cut = false);
		void listItemPaste() {}
		void listItemDelete(bool cut = false) {}
		vector<QString> getSelected();
		void update();
		void updateFromTab();

		QListWidget* side = nullptr;

		__state currentState() { return this->state; }

	protected:
		void layout();
		void sideLayout();
		void populate();
		void stacker(int vv);
		void sideRowChanged(int index);
		void listItemSelectionChanged();
		void showListEditContextMenu(QPoint& pos);
		void updateFlags();

		QTabBar* tabv;
		map<string, vector<pair<int, string>>> index;

		__state state;
		__action action;

	private:
		QWidget* cwid;

};
}
#endif /* channelBookView_h */
