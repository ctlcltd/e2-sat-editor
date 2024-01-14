/*!
 * e2-sat-editor/src/gui/transpondersView.h
 *
 * @link https://github.com/ctlcltd/e2-sat-editor
 * @copyright e2 SAT Editor Team
 * @author Leonardo Laureti
 * @version 1.1
 * @license MIT License
 * @license GNU GPLv3 License
 */

#ifndef transpondersView_h
#define transpondersView_h
#include <QWidget>
#include <QTreeWidget>
#include <QPushButton>

#include "toolkit/TreeDropIndicatorEventPainter.h"
#include "toolkit/TreeItemChangedEventObserver.h"
#include "viewAbstract.h"

namespace e2se_gui
{
class transpondersView : public viewAbstract
{
	Q_DECLARE_TR_FUNCTIONS(transpondersView)

	public:

		static const int TSV_TABS = 17;

		struct __state
		{
			// list sort (default sort 0|asc)
			pair<int, Qt::SortOrder> sort;
			// pending update list index
			bool txx_pending = false;
		};

		struct __action
		{
			QAction* list_newtx;
			QPushButton* list_search;
		};

		enum ITEM_DATA_ROLE {
			idx,
			txid,
		};

		enum ITEM_ROW_ROLE {
			x,
			debug_txid,
			combo,
			sys,
			pos,
			tsid,
			dvbns,
			onid,
			freq,
			pol,
			sr,
			fec,
			mod,
			band,
			pil,
			rol,
			inv,
			tmx,
			guard,
			hier
		};

		transpondersView(tab* twid, QWidget* cwid, dataHandler* data);
		~transpondersView();
		void load();
		void reset();
		void listItemCopy(bool cut = false);
		void listItemPaste();
		void listItemDelete();
		void updateIndex();

		__state currentState() { return this->state; }

	protected:
		void layout();
		void searchLayout();
		void populate();
		void listItemChanged();
		void listItemSelectionChanged();
		void listItemDoubleClicked();
		void listUpdate();
		void addTransponder();
		void editTransponder();
		void putListItems(vector<QString> items);
		void showListEditContextMenu(QPoint& pos);
		void updateFlags();
		void updateStatusBar(bool current = false);
		void updateListIndex();

		TreeDropIndicatorEventPainter* list_evth;
		TreeItemChangedEventObserver* list_evto;

		__state state;
		__action action;

	private:
		QWidget* cwid;
};
}
#endif /* transpondersView_h */
