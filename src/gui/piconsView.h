/*!
 * e2-sat-editor/src/gui/piconsView.h
 *
 * @link https://github.com/ctlcltd/e2-sat-editor
 * @copyright e2 SAT Editor Team
 * @author Leonardo Laureti
 * @version 1.2.0
 * @license MIT License
 * @license GNU GPLv3 License
 */

#ifndef piconsView_h
#define piconsView_h
#include <QByteArray>
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
			// picons directory
			QString picons_dir;
			// current picon
			string curr_picon;
			// pending update from tab
			bool tab_pending = false;
		};

		struct __action
		{
			QPushButton* acrn_prefs;
			QWidget* list_browse;
			QPushButton* list_search;
			QMenu* acrn_prefs_menu = nullptr;
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
		void listItemDelete(bool cut = false);
		void listFindPerform(const QString& value, LIST_FIND flag);
		void listFindClear(bool hidden = true);
		void didChange();
		void update();
		void updateFromTab();

		QListWidget* list = nullptr;

		__state currentState() { return this->state; }

	protected:
		void layout();
		void browseLayout();
		void searchLayout();
		void populate();
		void listItemChanged();
		void listItemSelectionChanged();
		void listItemDoubleClicked();
		void listChangedPiconsPath();
		QMenu* listPrefsCornerMenu();
		void editPicon();
		void changePicon(QListWidgetItem* item, QString path);
		/*void changePicon(QListWidgetItem* item, QByteArray data);*/
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
		static QString browseFileDialog(QString path);
		static QString importFileDialog(QString path);

		QWidget* cwid;
};
}
#endif /* piconsView_h */
