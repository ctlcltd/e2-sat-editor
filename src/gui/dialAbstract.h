/*!
 * e2-sat-editor/src/gui/dialAbstract.h
 *
 * @link https://github.com/ctlcltd/e2-sat-editor
 * @copyright e2 SAT Editor Team
 * @author Leonardo Laureti
 * @version 0.8
 * @license MIT License
 * @license GNU GPLv3 License
 */

#include <vector>

using std::pair, std::vector;

#ifndef dialAbstract_h
#define dialAbstract_h
#include <QApplication>
#include <QWidget>
#include <QDialog>
#include <QGridLayout>
#include <QToolBar>
#include <QToolButton>

#include "../logger/logger.h"
#include "theme.h"
#include "dataHandler.h"

namespace e2se_gui
{
class dialAbstract : protected e2se::log_factory
{
	Q_DECLARE_TR_FUNCTIONS(dialAbstract)

	public:
		struct __state
		{
		};

		struct __action
		{
			QAction* cancel;
			QAction* save;
		};

		virtual ~dialAbstract() = default;
		virtual void display(QWidget* cwid) = 0;
		virtual void destroy();
		void themeChanged();

		QWidget* widget;

	protected:
		void layout(QWidget* cwid);
		virtual void toolbarLayout();
		virtual void collapsibleLayout();
		virtual void store() = 0;
		virtual void retrieve() = 0;
		void cancel();
		void save();
		void expand();
		void collapse();
		void toggle();

		static QToolBar* toolBar();
		static QAction* toolBarAction(QToolBar* toolbar, QString text, std::function<void()> trigger);
		static QAction* toolBarAction(QToolBar* toolbar, QString text, pair<e2se_gui::theme*, QString> icon, std::function<void()> trigger);
		static QAction* toolBarAction(QToolBar* toolbar, QString text, std::function<void()> trigger, QKeySequence shortcut);
		static QAction* toolBarAction(QToolBar* toolbar, QString text, pair<e2se_gui::theme*, QString> icon, std::function<void()> trigger, QKeySequence shortcut);
		static QWidget* toolBarWidget(QToolBar* toolbar, QString text, QWidget* widget);
		static QWidget* toolBarSeparator(QToolBar* toolbar);
		static QWidget* toolBarSpacer(QToolBar* toolbar);
		void toolBarStyleSheet();

		QDialog* dial = nullptr;
		e2se_gui::theme* theme;
		dataHandler* data = nullptr;
		QToolBar* dtbar;
		QToolButton* dttoggler;
		QGridLayout* dtform = nullptr;
		bool collapsible = false;
		QMargins frameMargins = QMargins (10, 12, 10, 12);
		bool frameFixed = true;
		vector<QWidget*> fields;

		__state state;
		__action action;
};
}
#endif /* dialAbstract_h */
