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

#include "toolkit/BouquetsEventHandler.h"
#include "toolkit/ListEventHandler.h"
#include "toolkit/ListEventObserver.h"
#include "viewAbstract.h"
#include "tools.h"

namespace e2se_gui
{
class gui;
class tab;

class mainView : public viewAbstract
{
	public:
		enum LIST_REF {
			ReferenceID,
			ServiceID,
			Transponder,
			Userbouquets,
			Bouquets,
			Tuner
		};

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
			chfreq,
			chpol,
			chsr,
			chfec,
			chpos,
			chsys
		};

		mainView(gui* gid, tab* twid, QWidget* wid, e2se::logger::session* log);
		~mainView() {};
		void layout();
		void searchLayout();
		void refboxLayout();
		void load();
		void reset();
		void populate(QTreeWidget* side_tree);
		void treeSwitched(QTreeWidget* tree, QTreeWidgetItem* item);
		void servicesItemChanged(QTreeWidgetItem* current);
		void bouquetsItemChanged(QTreeWidgetItem* current);
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
		void bouquetItemDelete();
		void listReferenceToggle();
		void listItemCut();
		void listItemCopy(bool cut = false);
		void listItemPaste();
		void listItemDelete();
		void listItemSelectAll();
		void putListItems(vector<QString> items);
		void updateListIndex();
		void updateTreeIndex();
		void updateConnectors();
		void updateCounters(bool current = false);
		void updateRefBox();
		void showTreeEditContextMenu(QPoint &pos);
		void showListEditContextMenu(QPoint &pos);
		QTreeWidget* services_tree;

		struct sts
		{
		   // drag-and-drop (default sort 0|asc)
		   bool dnd;
		   // post update index
		   bool changed;
		   // post visual reindex list_tree
		   bool reindex;
		   // refbox shown
		   bool refbox;
		   // side tree focused { services_tree = 0, bouquets_tree = 1 }
		   int tc;
		   // tree current top level index
		   int ti;
		   // tree current bname
		   string curr;
		   // list_tree sort
		   pair<int, Qt::SortOrder> sort;
		} state;

		struct ats
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
	protected:
		map<int, QLabel*> ref_fields;
		unordered_map<string, QList<QTreeWidgetItem*>> cache;
		e2se_gui_tools::tools* tools;
		QWidget* list_wrap;
		BouquetsEventHandler* bouquets_evth;
		ListEventHandler* list_evth;
		ListEventObserver* list_evto;
		QVBoxLayout* tbox;
		QVBoxLayout* lbox;
		QWidget* list_reference;
	private:
		gui* gid;
		tab* twid;
		QWidget* cwid;
		string filename;
};
}
#endif /* mainView_h */
