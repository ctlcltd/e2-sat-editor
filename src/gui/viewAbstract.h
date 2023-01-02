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
#include <QTreeWidget>
#include <QAction>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QComboBox>
#include <QElapsedTimer>

#include "../logger/logger.h"
#include "gui.h"
#include "dataHandler.h"
#include "ftpHandler.h"
#include "tools.h"

namespace e2se_gui
{
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

		struct __state {
		};

		struct __action
		{
			QAction* list_newtr;
			QPushButton* tree_search;
			QPushButton* list_search;
		};

		virtual ~viewAbstract() = default;
		virtual void load() = 0;
		virtual void reset() = 0;
		virtual void treeItemDelete();
		virtual void listItemCut();
		virtual void listItemCopy(bool cut = false) = 0;
		virtual void listItemPaste() = 0;
		virtual void listItemDelete() = 0;
		virtual void listItemSelectAll();
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
		virtual void updateFlags()
		{
			tabUpdateFlags();
		};
		//TODO refact
		virtual void updateStatus(bool current = false)
		{
			tabResetStatus();
			// suppress lot of annoying warning
			if (current) return;
		};
		virtual void updateIndex() {};

		QWidget* widget;
		QTreeWidget* list;
		QTreeWidget* tree;
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

		virtual void layout() {};
		void searchLayout();
		void treeItemChanged();
		void sortByColumn(int column);
		void tabSetFlag(gui::GUI_CXE bit, bool flag);
		void tabSetFlag(gui::GUI_CXE bit);
		bool tabGetFlag(gui::GUI_CXE bit);
		void tabUpdateFlags();
		void tabUpdateFlags(gui::GUI_CXE bit);
		virtual void tabSetStatus(gui::STATUS status);
		virtual void tabResetStatus();
		void tabNewFile();
		void tabOpenFile();
		bool tabReadFile(string filename = "");
		void tabSaveFile(bool saveas);
		void tabImportFile();
		void tabExportFile();
		void tabPrintFile(bool all);

		QSettings* sets;
		tab* tid;
		dataHandler* data = nullptr;
		ftpHandler* ftph = nullptr;
		e2se_gui::tools* tools;
		e2db* dbih = nullptr;
		QWidget* tree_search;
		QWidget* list_search;
		search tsr_search;
		search lsr_search;
		find lsr_find;

		__state state;
		__action action;
};
}
#endif /* viewAbstract_h */
