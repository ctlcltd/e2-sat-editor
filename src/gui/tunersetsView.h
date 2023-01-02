/*!
 * e2-sat-editor/src/gui/tunersetsView.h
 *
 * @link https://github.com/ctlcltd/e2-sat-editor
 * @copyright e2 SAT Editor Team
 * @author Leonardo Laureti
 * @version 0.1
 * @license MIT License
 * @license GNU GPLv3 License
 */

#ifndef tunersetsView_h
#define tunersetsView_h
#include <QWidget>
#include <QTreeWidget>
#include <QAction>
#include <QPushButton>

#include "toolkit/ListEventObserver.h"
#include "viewAbstract.h"

namespace e2se_gui
{
class tunersetsView : public viewAbstract
{
	public:
		struct __state
		{
			// tunersets tvid type
			int yx;
			// current tnid
			string curr;
			// list sort (default sort 0|asc)
			pair<int, Qt::SortOrder> sort;
			// pending update list index
			bool tvx_pending;
		};

		struct __action
		{
			QAction* list_newtr;
			QPushButton* tree_search;
			QPushButton* list_search;
		};

		enum TREE_ROW_ROLE {
			tnid,
			trow1,
			trow2,
			name = TREE_ROW_ROLE::trow1,
			pos = TREE_ROW_ROLE::trow2,
			country = TREE_ROW_ROLE::trow2
		};

		enum ITEM_DATA_ROLE {
			idx,
			trid,
		};

		enum ITEM_ROW_ROLE {
			x,
			debug_trid,
			combo,
			row3,
			row4,
			row5,
			row6,
			row7,
			row8,
			row9,
			rowA,
			rowB,
			rowC,
			s_freq = ITEM_ROW_ROLE::row3,
			s_pol = ITEM_ROW_ROLE::row4,
			s_sr = ITEM_ROW_ROLE::row5,
			s_fec = ITEM_ROW_ROLE::row6,
			s_sys = ITEM_ROW_ROLE::row7,
			s_mod = ITEM_ROW_ROLE::row8,
			s_inv = ITEM_ROW_ROLE::row9,
			s_pil = ITEM_ROW_ROLE::rowA,
			s_rol = ITEM_ROW_ROLE::rowB,
			t_freq = ITEM_ROW_ROLE::row3,
			t_tmod = ITEM_ROW_ROLE::row4,
			t_band = ITEM_ROW_ROLE::row5,
			t_sys = ITEM_ROW_ROLE::row6,
			t_tmx = ITEM_ROW_ROLE::row7,
			t_hpfec = ITEM_ROW_ROLE::row8,
			t_lpfec = ITEM_ROW_ROLE::row9,
			t_inv = ITEM_ROW_ROLE::rowA,
			t_guard = ITEM_ROW_ROLE::rowB,
			t_hier = ITEM_ROW_ROLE::rowC,
			c_freq = ITEM_ROW_ROLE::row3,
			c_cmod = ITEM_ROW_ROLE::row4,
			c_sr = ITEM_ROW_ROLE::row5,
			c_cfec = ITEM_ROW_ROLE::row6,
			c_inv = ITEM_ROW_ROLE::row7,
			c_sys = ITEM_ROW_ROLE::row8,
			a_freq = ITEM_ROW_ROLE::row3,
			a_amod = ITEM_ROW_ROLE::row4,
			a_sys = ITEM_ROW_ROLE::row5
		};

		tunersetsView(tab* twid, QWidget* cwid, dataHandler* data, int ytype, e2se::logger::session* log);
		void load();
		void reset();
		void treeItemDelete();
		void listItemCopy(bool cut = false);
		void listItemPaste();
		void listItemDelete();
		void updateIndex();

		__state currentState() { return this->state; };
	protected:
		void layout();
		void searchLayout();
		void populate();
		void treeItemChanged(QTreeWidgetItem* current);
		void listItemChanged();
		void listItemSelectionChanged();
		void listItemDoubleClicked();
		void listPendingUpdate();
		void addSettings();
		void editSettings();
		void addPosition();
		void editPosition();
		void addTransponder();
		void editTransponder();
		void putListItems(vector<QString> items);
		void showTreeEditContextMenu(QPoint &pos);
		void showListEditContextMenu(QPoint &pos);
		void updateFlags();
		void updateStatus(bool current = false);
		void updateTreeIndex();
		void updateListIndex();
		void setPendingUpdateListIndex();
		void unsetPendingUpdateListIndex();

		ListEventObserver* tree_evto;
		ListEventObserver* list_evto;

		__state state;
		__action action;
	private:
		QWidget* cwid;
};
}
#endif /* tunersetsView_h */
