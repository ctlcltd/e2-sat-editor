/*!
 * e2-sat-editor/src/gui/viewAbstract.h
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

using std::string, std::pair, std::vector;

#ifndef viewAbstract_h
#define viewAbstract_h
#include <QSettings>
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
class gui;
class tab;

class viewAbstract : protected e2se::log_factory
{
	public:
		enum LIST_FIND {
			fast,
			next,
			prev,
			all
		};

		void setDataSource(e2db* dbih);
		virtual void layout() {};
		void searchLayout();
		virtual void load() {};
		virtual void preset() {};
		void treeItemChanged();
		void sortByColumn(int column);
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
		virtual void listItemCut() {};
		virtual void listItemCopy(bool cut = false) {};
		virtual void listItemPaste() {};
		virtual void listItemDelete() {};
		virtual void listItemSelectAll() {};
		virtual void putListItems(vector<QString> items) {};
		virtual void showTreeEditContextMenu(QPoint &pos) {};
		virtual void showListEditContextMenu(QPoint &pos) {};
		virtual void updateConnectors() {};
		virtual void updateCounters(bool current = false) {};

		QWidget* widget;
		QSettings* sets;
		QTreeWidget* list;
		QTreeWidget* tree;

		struct ats
		{
			QAction* list_newtr;
			QPushButton* tree_search;
			QPushButton* list_search;
		} action;

		// stored gui connector flags
		int gxe;
	protected:
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

		QVBoxLayout* tbox;
		QVBoxLayout* lbox;
		QWidget* tree_search;
		QWidget* list_search;
		search tsr_search;
		search lsr_search;
		find lsr_find;
		e2db* dbih = nullptr;
	private:
		gui* gid;
		tab* twid;
};
}
#endif /* viewAbstract_h */
