/*!
 * e2-sat-editor/src/gui/tunersets.h
 *
 * @link https://github.com/ctlcltd/e2-sat-editor
 * @copyright e2 SAT Editor Team
 * @author Leonardo Laureti
 * @version 0.1
 * @license MIT License
 * @license GNU GPLv3 License
 */

#ifndef tunersets_h
#define tunersets_h
#include <QWidget>
#include <QVBoxLayout>
#include <QTreeWidget>
#include <QAction>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QComboBox>
#include <QElapsedTimer>

#include "../logger/logger.h"
#include "e2db_gui.h"

namespace e2se_gui
{
class tunersets : protected e2se::log_factory
{
	public:
		enum LIST_FIND {
			fast,
			next,
			prev,
			all
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

		tunersets(e2db* dbih, int ytype, e2se::logger::session* log);
		void layout();
		void treeItemChanged();
		void load();
		void populate();
		void trickySortByColumn(int column);
		void treeSearchHide();
		void treeSearchShow();
		void treeSearchToggle();
		void treeSearchClose();
		void listSearchShow();
		void listSearchHide();
		void listSearchToggle();
		void listSearchClose();
		void treeFindPerform();
		void treeFindPerform(const QString& value);
		void listFindPerform(LIST_FIND flag);
		void listFindPerform(const QString& value, LIST_FIND flag);
		void listFindHighlightToggle();
		void listFindClear(bool hidden = true);
		void listFindReset();
		QWidget* widget;
		QSettings* sets;
	protected:
		QVBoxLayout* tbox;
		QVBoxLayout* lbox;
		QTreeWidget* list;
		QTreeWidget* tree;
	private:
		struct ats
		{
			QAction* list_newtr;
			QPushButton* tree_search;
			QPushButton* list_search;
		} action;
		struct search
		{
			QComboBox* filter;
			QLineEdit* input;
			QPushButton* next;
			QPushButton* prev;
			QPushButton* all;
			QPushButton* highlight;
			QPushButton* close;
		};
		struct find
		{
			LIST_FIND flag;
			int filter;
			bool highlight = true;
			int curr = -1;
			QString input;
			QModelIndexList match;
			QElapsedTimer timer;
		};
		e2db* dbih;
		// tunersets type
		int yx;
		QWidget* tree_search;
		QWidget* list_search;
		search tsr_search;
		search lsr_search;
		find lsr_find;
};
}
#endif /* tunersets_h */
