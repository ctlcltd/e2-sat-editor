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
#include <QVBoxLayout>
#include <QTreeWidget>
#include <QAction>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QComboBox>
#include <QElapsedTimer>

#include "viewAbstract.h"

namespace e2se_gui
{
class gui;
class tab;

class tunersetsView : public viewAbstract
{
	public:
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

		struct sts
		{
			// post update index
			bool changed;
			// tree current top level index
			int ti;
			// tree current bname
			string curr;
			// list_tree sort
			pair<int, Qt::SortOrder> sort;
			// tools tunersets current type
			int ty;
		} state;

		struct ats
		{
			QAction* list_newtr;
			QPushButton* tree_search;
			QPushButton* list_search;
		} action;

		tunersetsView(tab* twid, QWidget* wid, dataHandler* data, int ytype, e2se::logger::session* log);
		void load();
		void reset();
		void treeItemDelete();
		void listItemCut();
		void listItemCopy(bool cut = false);
		void listItemPaste();
		void listItemDelete();
		void listItemSelectAll();
	protected:
		void layout();
		void searchLayout();
		void populate();
		void treeItemChanged();
		void listItemDoubleClicked();
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

		QVBoxLayout* tbox;
		QVBoxLayout* lbox;
	private:
		QWidget* cwid;
		// tunersets type
		int yx;
};
}
#endif /* tunersetsView_h */
