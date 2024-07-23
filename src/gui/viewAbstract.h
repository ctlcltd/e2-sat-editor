/*!
 * e2-sat-editor/src/gui/viewAbstract.h
 *
 * @link https://github.com/ctlcltd/e2-sat-editor
 * @copyright e2 SAT Editor Team
 * @author Leonardo Laureti
 * @version 1.6.0
 * @license MIT License
 * @license GNU GPLv3 License
 */

#include <functional>
#include <string>
#include <vector>

using std::string, std::pair, std::vector;

#ifndef viewAbstract_h
#define viewAbstract_h
#include <QApplication>
#include <QWidget>
#include <QTreeWidget>
#include <QToolBar>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QComboBox>
#include <QElapsedTimer>

#include "../logger/logger.h"
#include "theme.h"
#include "gui.h"
#include "dataHandler.h"
#include "ftpHandler.h"
#include "tools.h"

namespace e2se_gui
{
class tab;

class viewAbstract : protected e2se::log_factory
{
	Q_DECLARE_TR_FUNCTIONS(viewAbstract)

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
		virtual void didChange() {}
		virtual void themeChanged();
		virtual void clipboardDataChanged();
		virtual void actionCall(int bit) = 0;
		virtual void treeItemDelete();
		virtual void listItemCut();
		virtual void listItemCopy(bool cut = false) = 0;
		virtual void listItemPaste() = 0;
		virtual void listItemDelete(bool cut = false) = 0;
		virtual void listItemSelectAll();
		void treeSearchHide();
		void treeSearchShow();
		void treeSearchToggle();
		void treeSearchClose();
		void listSearchShow();
		void listSearchHide();
		void listSearchToggle();
		void listSearchClose();
		virtual void treeFindPerform();
		virtual void treeFindPerform(const QString& value);
		virtual void listFindPerform(LIST_FIND flag);
		virtual void listFindPerform(const QString& value, LIST_FIND flag);
		virtual void listFindHighlightToggle();
		virtual void listFindClearSelection(bool hidden = true);
		virtual void listFindReset();
		virtual void listFindClear();
		virtual void updateFlags()
		{
			tabUpdateFlags();
		};
		virtual bool statusBarIsVisible()
		{
			return tabStatusBarIsVisible();
		}
		virtual bool statusBarIsHidden()
		{
			return tabStatusBarIsHidden();
		}
		virtual void updateStatusBar(bool current = false)
		{
			tabResetStatusBar();
			// suppress lot of annoying warning
			if (current) return;
		};
		virtual void resetStatusBar()
		{
			tabResetStatusBar();
		}
		virtual void update() {}
		virtual void updateIndex() {}
		virtual void updateFromTab() {}

		QWidget* widget = nullptr;
		QTreeWidget* list = nullptr;
		QTreeWidget* tree = nullptr;

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

		virtual void layout() = 0;
		void searchLayout();
		void sortByColumn(int column);
		bool tabGetFlag(gui::GUI_CXE bit);
		void tabSetFlag(gui::GUI_CXE bit, bool flag);
		void tabSetFlags(int preset);
		void tabUpdateFlags();
		void tabUpdateFlags(int preset);
		bool tabRemoveQuestion();
		bool tabRemoveQuestion(QString title, QString text);
		void tabUpdateToolBars();
		virtual bool tabStatusBarIsVisible();
		virtual bool tabStatusBarIsHidden();
		virtual void tabSetStatusBar(gui::status msg);
		virtual void tabResetStatusBar();
		void tabNewFile();
		void tabOpenFile();
		bool tabReadFile(string filename = "");
		void tabSaveFile(bool saveas);
		void tabImportFile();
		void tabExportFile();
		void tabPrintFile(bool all);
		void tabPropagateChanges();

		static QToolBar* toolBar();
		static QAction* toolBarAction(QToolBar* toolbar, QString text, std::function<void()> trigger);
		static QAction* toolBarAction(QToolBar* toolbar, QString text, pair<e2se_gui::theme*, QString> icon, std::function<void()> trigger);
		static QAction* toolBarAction(QToolBar* toolbar, QString text, std::function<void()> trigger, QKeySequence shortcut);
		static QAction* toolBarAction(QToolBar* toolbar, QString text, pair<e2se_gui::theme*, QString> icon, std::function<void()> trigger, QKeySequence shortcut);
		static QPushButton* toolBarButton(QToolBar* toolbar, QString text, std::function<void()> trigger);
		static QPushButton* toolBarButton(QToolBar* toolbar, QString text, pair<e2se_gui::theme*, QString> icon, std::function<void()> trigger);
		static QPushButton* toolBarButton(QToolBar* toolbar, QString text, std::function<void()> trigger, QKeySequence shortcut);
		static QPushButton* toolBarButton(QToolBar* toolbar, QString text, pair<e2se_gui::theme*, QString> icon, std::function<void()> trigger, QKeySequence shortcut);
		static QWidget* toolBarWidget(QToolBar* toolbar, QString text, QWidget* widget);
		static QAction* toolBarSeparator(QToolBar* toolbar);
		static QWidget* toolBarSpacer(QToolBar* toolbar);
		void toolBarStyleSheet();
		static QMenu* contextMenu();
		static QMenu* contextMenu(QMenu* menu);
		static QAction* contextMenuAction(QMenu* menu, QString text, std::function<void()> trigger);
		static QAction* contextMenuAction(QMenu* menu, QString text, std::function<void()> trigger, bool enabled);
		static QAction* contextMenuAction(QMenu* menu, QString text, std::function<void()> trigger, QKeySequence shortcut);
		static QAction* contextMenuAction(QMenu* menu, QString text, std::function<void()> trigger, bool enabled, QKeySequence shortcut);
		static QAction* contextMenuSeparator(QMenu* menu);
		static void treeSortItems(QTreeWidget* tw, int column, Qt::SortOrder order);

		e2se_gui::theme* theme;
		tab* tid = nullptr;
		dataHandler* data = nullptr;
		ftpHandler* ftph = nullptr;
		e2se_gui::tools* tools;
		QWidget* tree_search;
		QWidget* list_search;
		search tsr_search;
		search lsr_search;
		find lsr_find;

		__state state;
		__action action;

	private:
		static bool treeItemNumericLessThan(const QPair<QTreeWidgetItem*, int>& left, const QPair<QTreeWidgetItem*, int>& right)
		{
			int column = left.first->treeWidget()->sortColumn();
			return left.first->text(column).toInt() < right.first->text(column).toInt();
		}
		static bool treeItemNumericGreaterThan(const QPair<QTreeWidgetItem*, int>& left, const QPair<QTreeWidgetItem*, int>& right)
		{
			int column = left.first->treeWidget()->sortColumn();
			return right.first->text(column).toInt() < left.first->text(column).toInt();
		}
};
}
#endif /* viewAbstract_h */
