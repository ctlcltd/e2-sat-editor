/*!
 * e2-sat-editor/src/gui/transpondersView.h
 *
 * @link https://github.com/ctlcltd/e2-sat-editor
 * @copyright e2 SAT Editor Team
 * @author Leonardo Laureti
 * @version 0.3
 * @license MIT License
 * @license GNU GPLv3 License
 */

#ifndef transpondersView_h
#define transpondersView_h
#include <QWidget>
#include <QTreeWidget>
#include <QAction>
#include <QPushButton>

#include "toolkit/ListEventObserver.h"
#include "viewAbstract.h"

namespace e2se_gui
{
class transpondersView : public viewAbstract
{
	public:
		struct __state
		{
			// list sort (default sort 0|asc)
			pair<int, Qt::SortOrder> sort;
			// pending update list index
			bool txx_pending;
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
		void load();
		void reset();
		void listItemCopy(bool cut = false);
		void listItemPaste();
		void listItemDelete();
		void updateIndex();

		__state currentState() { return this->state; };

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
		void showListEditContextMenu(QPoint &pos);
		void updateFlags();
		void updateStatusBar(bool current = false);
		void updateListIndex();

		ListEventObserver* list_evto;

		__state state;
		__action action;

	private:
		QWidget* cwid;
};
}
#endif /* transpondersView_h */
