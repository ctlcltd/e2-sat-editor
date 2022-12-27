/*!
 * e2-sat-editor/src/gui/mainView.h
 *
 * @link https://github.com/ctlcltd/e2-sat-editor
 * @copyright e2 SAT Editor Team
 * @author Leonardo Laureti
 * @version 0.1
 * @license MIT License
 * @license GNU GPLv3 License
 */

#include <iostream>
#include <map>
#include <unordered_map>

using std::map, std::unordered_map;

#ifndef mainView_h
#define mainView_h
#include <Qt>
#include <QGridLayout>
#include <QHeaderView>
#include <QList>

#include "toolkit/TreeEventHandler.h"
#include "toolkit/ListEventHandler.h"
#include "toolkit/ListEventObserver.h"
#include "viewAbstract.h"

namespace e2se_gui
{
class mainView : public viewAbstract
{
	public:
		enum ITEM_DATA_ROLE {
			idx,
			marker,
			chid
		};

		enum ITEM_ROW_ROLE {
			x,
			chnum,
			chname,
			debug_chid,
			debug_txid,
			chssid,
			chtsid,
			chtype,
			chcas,
			chpname,
			chsys,
			chpos,
			chfreq,
			chpol,
			chsr,
			chfec
		};

		enum LIST_REF {
			ReferenceID,
			ServiceID,
			Transponder,
			Userbouquets,
			Bouquets,
			Tuner
		};

		struct __state
		{
			// tree events toggle
			bool evt;
			// list drag-and-drop toggle
			bool dnd;
			// list visual refresh pending
			bool vlx_pending;
			// reference box shown
			bool refbox;
			// current tree { side = 0, tree = 1 }
			int tc;
			// tree current top level index
			int ti;
			// current bname
			string curr;
			// list sort (default sort 0|asc)
			pair<int, Qt::SortOrder> sort;
		} state;

		struct __action
		{
			QAction* bouquets_newbs;
			QAction* list_addch;
			QAction* list_addmk;
			QAction* list_newch;
			QAction* tools_close_edit;
			QPushButton* tree_search;
			QPushButton* list_search;
			QPushButton* list_ref;
			QPushButton* list_dnd;
		} action;

		mainView(tab* tid, QWidget* cwid, dataHandler* data, e2se::logger::session* log);
		void load();
		void reset();
		void treeItemDelete();
		void listReferenceToggle();
		void listItemCopy(bool cut = false);
		void listItemPaste();
		void listItemDelete();

		QTreeWidget* side;
	protected:
		void layout();
		void searchLayout();
		void referenceBoxLayout();
		void populate(QTreeWidget* side_tree);
		void treeSwitched(QTreeWidget* tree, QTreeWidgetItem* item);
		void servicesItemChanged(QTreeWidgetItem* current);
		void treeItemChanged(QTreeWidgetItem* current);
		void listItemChanged();
		void listItemSelectionChanged();
		void listItemDoubleClicked();
		void listPendingUpdate();
		void visualReindexList();
		void sortByColumn(int column);
		void allowDnD();
		void disallowDnD();
		void reharmDnD();
		void addUserbouquet();
		void editUserbouquet();
		void addChannel();
		void addService();
		void editService();
		void addMarker();
		void editMarker();
		void putListItems(vector<QString> items);
		void showTreeEditContextMenu(QPoint &pos);
		void showListEditContextMenu(QPoint &pos);
		void updateFlags();
		void updateStatus(bool current = false);
		void updateReferenceBox();
		void tabUpdateBouquetsIndex();
		void tabUpdateChannelsIndex();
		void tabSetPendingUpdateChannelsIndex();
		void tabUnsetPendingUpdateChannelsIndex();

		map<int, QLabel*> ref_fields;
		unordered_map<string, QList<QTreeWidgetItem*>> cache;
		QWidget* list_wrap;
		TreeEventHandler* tree_evth;
		ListEventHandler* list_evth;
		ListEventObserver* list_evto;
		QWidget* list_reference;
		string filename;
	private:
		QWidget* cwid;
};
}
#endif /* mainView_h */
