/*!
 * e2-sat-editor/src/gui/editTunersetsTransponder.h
 *
 * @link https://github.com/ctlcltd/e2-sat-editor
 * @copyright e2 SAT Editor Team
 * @author Leonardo Laureti
 * @version 0.1
 * @license MIT License
 * @license GNU GPLv3 License
 */

#include <string>
#include <vector>

using std::string, std::vector;

#ifndef editTunersetsTransponder_h
#define editTunersetsTransponder_h
#include <QApplication>
#include <QWidget>
#include <QDialog>
#include <QGridLayout>

#include "../logger/logger.h"
#include "e2db_gui.h"

namespace e2se_gui
{
class editTunersetsTransponder : protected e2se::log_factory
{
	Q_DECLARE_TR_FUNCTIONS(editTunersetsTransponder)

	public:
		editTunersetsTransponder(e2db* dbih, e2se::logger::session* log);
		void display(QWidget* cwid);
		void layout();
		void store();
		void retrieve();
		void save();
		void setEditID(string todo);
		string getEditID();
		void destroy();
		QWidget* widget;
	protected:
		QDialog* dial;
		QGridLayout* dtform;
		vector<QWidget*> fields;
	private:
		struct sts
		{
			// editable
			bool edit = false;
		} state;
		e2db* dbih;
};
}
#endif /* editTunersetsTransponder_h */
