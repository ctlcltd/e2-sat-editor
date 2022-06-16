/*!
 * e2-sat-editor/src/gui/tab.h
 *
 * @link https://github.com/ctlcltd/e2-sat-editor
 * @copyright e2 SAT Editor Team
 * @author Leonardo Laureti
 * @version 0.1
 * @license MIT License
 * @license GNU GPLv3 License
 */

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <unordered_map>

using std::string, std::pair, std::vector, std::map, std::unordered_map;

#ifndef tab_h
#define tab_h
#include <Qt>
#include <QWidget>
#include <QGridLayout>
#include <QHeaderView>
#include <QTreeWidget>
#include <QList>
#include <QAction>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QComboBox>
#include <QElapsedTimer>

#include "toolkit/BouquetsEventHandler.h"
#include "toolkit/ListEventHandler.h"
#include "toolkit/ListEventObserver.h"
#include "../logger/logger.h"
#include "e2db_gui.h"
#include "ftpcom_gui.h"
#include "tools.h"

namespace e2se_gui
{
class gui;

class tab : protected e2se::log_factory
{
	public:
		enum LIST_EDIT_ATS {
			Cut,
			Copy,
			Paste,
			Delete,
			SelectAll
		};

		enum LIST_REF {
			ReferenceID,
			ServiceID,
			Transponder,
			Userbouquets,
			Bouquets,
			Tuner
		};

		enum LIST_FIND {
			fast,
			next,
			prev,
			all
		};

		enum ITEM_DATA_ROLE {
			idx,
			marker,
			chid
		};

		tab(gui* gid, QWidget* wid);
		~tab();
		void newFile();
		void openFile();
		bool readFile(string filename = "");
		void saveFile(bool saveas);
		void importFile();
		void exportFile();
		void addUserbouquet();
		void editUserbouquet();
		void addChannel();
		void addService();
		void editService();
		void load();
		void populate(QTreeWidget* side_tree);
		void treeSwitched(QTreeWidget* tree, QTreeWidgetItem* item);
		void servicesItemChanged(QTreeWidgetItem* current);
		void bouquetsItemChanged(QTreeWidgetItem* current);
		void listItemChanged();
		void listItemSelectionChanged();
		void listPendingUpdate();
		void visualReindexList();
		void trickySortByColumn(int column);
		void allowDnD();
		void disallowDnD();
		void reharmDnD();
		void bouquetItemDelete();
		void actionCall(int action);
		void bouquetsSearchHide();
		void bouquetsSearchShow();
		void bouquetsSearchToggle();
		void bouquetsSearchClose();
		void listSearchShow();
		void listSearchHide();
		void listSearchToggle();
		void listSearchClose();
		void listReferenceToggle();
		void bouquetsFindPerform();
		void bouquetsFindPerform(const QString& value);
		void listFindPerform(LIST_FIND flag);
		void listFindPerform(const QString& value, LIST_FIND flag);
		void listFindHighlightToggle();
		void listFindClear(bool hidden = true);
		void listFindReset();
		void listItemCut();
		void listItemCopy(bool cut = false);
		void listItemPaste();
		void listItemDelete();
		void listItemSelectAll();
		void editTunersets(int ytype);
		void closeTunersets();
		void putChannels(vector<QString> channels);
		void updateListIndex();
		void updateBouquetsIndex();
		void updateConnectors();
		void updateCounters(bool current = false);
		void updateRefBox();
		void showBouquetEditContextMenu(QPoint &pos);
		void showListEditContextMenu(QPoint &pos);
		void setTabId(int ttid);
		void tabSwitched();
		void tabChangeName(string filename = "");
		void initialize();
		void profileComboChanged(int index);
		bool ftpHandle();
		void ftpConnect();
		void ftpUpload();
		void ftpDownload();
		void loadSeeds();
		QGridLayout* root;
		QWidget* widget;
	protected:
		map<int, QLabel*> ref_fields;
		unordered_map<string, QList<QTreeWidgetItem*>> cache;
		e2se_gui_tools::tools* tools;
	private:
		struct ats
		{
			QAction* bouquets_newbs;
			QAction* list_addch;
			QAction* list_newch;
			QAction* tools_close_edit;
			QPushButton* bouquets_search;
			QPushButton* list_search;
			QPushButton* list_ref;
			QPushButton* list_dnd;
		} action;
		struct sts
		{
			// new file
			bool nwwr;
			// overwrite file
			bool ovwr;
			// drag-and-drop (default sort 0|asc)
			bool dnd;
			// post update index
			bool changed;
			// post visual reindex list_tree
 			bool reindex;
			// refbox shown
			bool refbox;
			// tools tunerset shown
			bool tunersets;
			// side tree focused { services_tree = 0, bouquets_tree = 1 } 
			int tc;
			// bouquets_tree current top level index
			int ti;
			// bouquets_tree current bname
			string curr;
			// list_tree sort
			pair<int, Qt::SortOrder> sort;
		} state;
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
		gui* gid;
		QWidget* cwid;
		int ttid = -1;
		e2db* dbih = nullptr;
		ftpcom* ftph = nullptr;
		BouquetsEventHandler* bouquets_evth;
		ListEventHandler* list_evth;
		ListEventObserver* list_evto;
		QTreeWidget* services_tree;
		QTreeWidget* bouquets_tree;
		QTreeWidget* list_tree;
		QWidget* list_wrap;
		QHeaderView* lheaderv;
		QWidget* bouquets_search;
		QWidget* list_search;
		QWidget* list_reference;
		search lsr_search;
		search bsr_search;
		find lsr_find;
		string filename;
};
}
#endif /* tab_h */
