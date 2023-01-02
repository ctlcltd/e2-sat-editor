/*!
 * e2-sat-editor/src/gui/dialAbstract.h
 *
 * @link https://github.com/ctlcltd/e2-sat-editor
 * @copyright e2 SAT Editor Team
 * @author Leonardo Laureti
 * @version 0.1
 * @license MIT License
 * @license GNU GPLv3 License
 */

#include <vector>

using std::vector;

#ifndef dialAbstract_h
#define dialAbstract_h
#include <QApplication>
#include <QWidget>
#include <QDialog>
#include <QGridLayout>
#include <QToolBar>

#include "../logger/logger.h"
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
			QAction* save;
		};

		virtual ~dialAbstract() = default;
		void display(QWidget* cwid);
		void destroy();

		QWidget* widget;
	protected:
		void layout();
		virtual void toolbar();
		virtual void store() = 0;
		virtual void retrieve() = 0;
		void save();

		QDialog* dial;
		QToolBar* dtbar;
		QGridLayout* dtform;
		QMargins frameMargins = QMargins (12, 12, 12, 12);
		bool frameFixed = true;
		dataHandler* data = nullptr;
		e2db* dbih = nullptr;
		vector<QWidget*> fields;

		__state state;
		__action action;
};
}
#endif /* dialAbstract_h */
