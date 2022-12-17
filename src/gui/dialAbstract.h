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
#include "e2db_gui.h"

namespace e2se_gui
{
class dialAbstract : protected e2se::log_factory
{
	Q_DECLARE_TR_FUNCTIONS(dialAbstract)

	public:
		virtual ~dialAbstract() = default;
		void display(QWidget* cwid);
		void layout();
		virtual void toolbar();
		virtual void store() {};
		virtual void retrieve() {};
		void save();
		void destroy();
		QWidget* widget;

		struct ats
		{
			// QAction* add;
			QAction* save;
		} action;
	protected:
		QDialog* dial;
		QToolBar* dtbar;
		QGridLayout* dtform;
		QMargins frameMargins = QMargins (12, 12, 12, 12);
		bool frameFixed = true;
		vector<QWidget*> fields;
		e2db* dbih = nullptr;
};
}
#endif /* dialAbstract_h */