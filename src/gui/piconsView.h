/*!
 * e2-sat-editor/src/gui/piconsView.h
 *
 * @link https://github.com/ctlcltd/e2-sat-editor
 * @copyright e2 SAT Editor Team
 * @author Leonardo Laureti
 * @version 0.7
 * @license MIT License
 * @license GNU GPLv3 License
 */

#ifndef piconsView_h
#define piconsView_h
#include <QWidget>
#include <QListWidget>
#include <QPushButton>

#include "toolkit/ListIconDragDropEventHandler.h"
#include "viewAbstract.h"

namespace e2se_gui
{
class piconsView : public viewAbstract
{
	Q_DECLARE_TR_FUNCTIONS(piconsView)

	public:

		struct __state
		{
			string picons_dir;
		};

		struct __action
		{
			QWidget* list_browse;
			QPushButton* list_search;
		};

		enum ITEM_DATA_ROLE {
			x = 0,
			chid = 2,
			txid = 3,
			chname = 1,
			chtype = 7,
			chcas = 8,
			chpname = 9,
			filename = 10,
			txtname = 13,
			txpos = 12,
			txsys = 11
		};

		piconsView(tab* twid, QWidget* cwid, dataHandler* data);
		~piconsView();
		void load();
		void reset();
		void listItemCopy(bool cut = false);
		void listItemPaste();
		void listItemDelete();
		void listFindPerform(const QString& value, LIST_FIND flag);
		void listFindClear(bool hidden = true);

		QListWidget* list = nullptr;

		__state currentState() { return this->state; }

	protected:
		void layout();
		void browseLayout();
		void searchLayout();
		void populate();
		void listChangedPiconsPath();
		void listItemChanged();
		void listItemSelectionChanged();
		void listItemDoubleClicked();
		void editPicon();
		void changePicon(QListWidgetItem* item, string path);
		void showListEditContextMenu(QPoint& pos);
		void updateFlags();
		void updateStatusBar(bool current = false);

		static QString piconPathname(string chname);

		ListIconDragDropEventHandler* list_evth;
		ListIconDragDropEventFilter* list_evte;
		QWidget* list_browse;

		__state state;
		__action action;

	private:
		static string browseFileDialog(string path);
		static string importFileDialog(string path);

		QWidget* cwid;
};
}
#endif /* piconsView_h */
