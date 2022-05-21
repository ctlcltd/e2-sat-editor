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
#include <unordered_map>

using std::string, std::pair, std::vector, std::unordered_map;

#ifndef tab_h
#define tab_h
#include <Qt>
#include <QWidget>
#include <QHeaderView>
#include <QTreeWidget>
#include <QList>
#include <QAction>
#include <QLabel>

#include "../logger.h"
#include "e2db_gui.h"
#include "ftpcom_gui.h"
#include "BouquetsEventHandler.h"
#include "ListEventHandler.h"
#include "ListEventObserver.h"

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
		void visualReindexList();
		void trickySortByColumn(int column);
		void allowDnD();
		void disallowDnD();
		void reharmDnD();
		void bouquetItemDelete();
		void listItemCut();
		void listItemCopy(bool cut = false);
		void listItemPaste();
		void listItemDelete();
		void listItemSelectAll();
		void listItemAction(int action);
		void putChannels(vector<QString> channels);
		void updateListIndex();
		void updateBouquetsIndex();
		void showBouquetEditContextMenu(QPoint &pos);
		void showListEditContextMenu(QPoint &pos);
		void setCounters(bool channels = false);
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
		QWidget* widget;
	protected:
		unordered_map<string, QList<QTreeWidgetItem*>> cache;
	private:
		struct ats {
			QAction* bouquets_newbs;
			QAction* list_addch;
			QAction* list_newch;
			QPushButton* list_dnd;
		} action;
		struct sts {
			bool nwwr;
			bool ovwr;
			bool dnd;
			bool init;
			bool changed;
			int tc;
			int ti;
			string curr;
			pair<int, Qt::SortOrder> sort;
		} state;
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
		string filename;
};
}
#endif /* tab_h */
