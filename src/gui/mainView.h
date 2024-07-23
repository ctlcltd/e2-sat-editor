/*!
 * e2-sat-editor/src/gui/mainView.h
 *
 * @link https://github.com/ctlcltd/e2-sat-editor
 * @copyright e2 SAT Editor Team
 * @author Leonardo Laureti
 * @version 1.6.0
 * @license MIT License
 * @license GNU GPLv3 License
 */

#include <map>
#include <unordered_map>

using std::map, std::unordered_map;

#ifndef mainView_h
#define mainView_h
#include <QWidget>
#include <QTreeWidget>
#include <QLabel>
#include <QPushButton>
#include <QList>

#include "toolkit/TreeEventHandler.h"
#include "toolkit/TreeDragDropEventHandler.h"
#include "toolkit/TreeItemChangedEventObserver.h"
#include "viewAbstract.h"

namespace e2se_gui
{
class dialChannelBook;

class mainView : public viewAbstract
{
	Q_DECLARE_TR_FUNCTIONS(mainView)

	public:

		static const int TSV_TABS = 16;

		struct __state
		{
			// list drag-and-drop toggle
			bool dnd = false;
			// list visual refresh pending
			bool vlx_pending = false;
			// reference box shown
			bool refbox = false;
			// current tree { side = 0, tree = 1 }
			int tc;
			// tree current top level index
			int ti;
			// side current top level index
			int si;
			// current bname
			string curr;
			// list sort (default sort 0|asc)
			pair<int, Qt::SortOrder> sort;
			// pending update list index
			bool chx_pending = false;
			// pending update from tab
			bool tab_pending = false;
			// preference : parental lock [QSettings] initial value
			bool q_parentalLockInvert = false;
			// engine : marker global index [QSettings] initial value
			bool q_markerGlobalIndex = false;
			// engine : favourite match service [QSettings] initial value
			bool q_favouriteMatchService = true;
		};

		struct __action
		{
			QPushButton* scrn_sets;
			QPushButton* lcrn_prefs;
			QAction* tree_newbq;
			QAction* list_addch;
			QAction* list_addfh;
			QAction* list_addmk;
			QAction* list_newch;
			QAction* tools_close_edit;
			QPushButton* tree_search;
			QPushButton* list_search;
			QPushButton* list_ref;
			QPushButton* list_dnd;
			QMenu* scrn_sets_menu = nullptr;
			QMenu* lcrn_prefs_menu = nullptr;
		};

		enum ITEM_DATA_ROLE {
			refid,
			idx,
			numbered,
			locked,
			chid,
			reftype = 8,
			uri = 13
		};

		enum ITEM_ROW_ROLE {
			x,
			chnum,
			chname,
			chlock,
			debug_chid,
			debug_txid,
			chssid,
			chtsid,
			chtype,
			chcas,
			chpname,
			chsys,
			chpos,
			chtname,
			chfreq,
			chpol,
			chsr,
			chfec
		};

		enum REF_TYPE {
			service,
			favourite,
			marker,
			stream
		};

		enum LIST_REF {
			ReferenceID,
			ServiceID,
			Transponder,
			Userbouquets,
			Bouquets,
			Tuner
		};

		mainView(tab* tid, QWidget* cwid, dataHandler* data);
		~mainView();
		void load();
		void reset();
		void treeItemDelete();
		void listReferenceToggle();
		void listItemCopy(bool cut = false);
		void listItemPaste();
		void listItemDelete(bool cut = false);
		void clipboardDataChanged();
		void didChange();
		void update();
		void updateIndex();
		void updateFromTab();

		QTreeWidget* side = nullptr;

		__state currentState() { return this->state; }

	protected:
		void layout();
		void searchLayout();
		void referenceBoxLayout();
		void populate(QTreeWidget* tw);
		void treeSwitched(QTreeWidget* tw, QTreeWidgetItem* item);
		void servicesItemChanged(QTreeWidgetItem* current);
		void treeItemChanged(QTreeWidgetItem* current);
		void treeItemSelectionChanged();
		void treeItemDoubleClicked();
		void listItemChanged();
		void listItemSelectionChanged();
		void listItemDoubleClicked();
		void listPendingUpdate();
		void visualReindexList();
		void visualReloadList();
		void sortByColumn(int column);
		void allowDnD();
		void disallowDnD();
		void reharmDnD();
		QMenu* servicesSetsCornerMenu();
		QMenu* listPrefsCornerMenu();
		void convert(int bit);
		void addBouquet();
		void editBouquet();
		void addUserbouquet();
		void editUserbouquet();
		void addChannel();
		void addService();
		void editService();
		void addFavourite();
		void editFavourite();
		void addMarker();
		void editMarker();
		void setServiceParentalLock();
		void unsetServiceParentalLock();
		void toggleServiceParentalLock();
		void setUserbouquetParentalLock();
		void unsetUserbouquetParentalLock();
		void toggleUserbouquetParentalLock();
		void putListItems(vector<QString> items);
		void showTreeEditContextMenu(QPoint& pos);
		void showListEditContextMenu(QPoint& pos);
		void actionCall(int bit);
		void treeAfterDrop(QTreeWidget* tw, QTreeWidgetItem* current);
		void listAfterDrop(QTreeWidget* tw);
		void treeDropFromTree(QTreeWidgetItem* current);
		void treeDropFromList(QTreeWidgetItem* current);
		void updateFlags();
		void updateStatusBar(bool current = false);
		void updateReferenceBox();
		void updateTreeIndex();
		void updateListIndex();
		void updateListReferences(QTreeWidgetItem* current, QList<QTreeWidgetItem*> items);
		void setPendingUpdateListIndex();
		void unsetPendingUpdateListIndex();

		map<int, QLabel*> ref_fields;
		unordered_map<string, QList<QTreeWidgetItem*>> cache;
		TreeEventHandler* tree_evth;
		TreeDragDropEventHandler* list_evth;
		TreeItemChangedEventObserver* list_evto;
		QWidget* list_reference;
		dialChannelBook* dialchbook = nullptr;
		string filename;

		__state state;
		__action action;

	private:
		QWidget* cwid;
};
}
#endif /* mainView_h */
