/*!
 * e2-sat-editor/src/gui/editTunersets.h
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

#ifndef editTunersets_h
#define editTunersets_h
#include <QApplication>
#include <QWidget>
#include <QDialog>
#include <QGridLayout>

#include "../logger/logger.h"
#include "e2db_gui.h"

namespace e2se_gui
{
class editTunersets : protected e2se::log_factory
{
	Q_DECLARE_TR_FUNCTIONS(editTunersets)

	public:
		editTunersets(e2db* dbih, int ty, e2se::logger::session* log);
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
			// tools tunersets current type
			int ty;
		} state;
		e2db* dbih;
};
}
#endif /* editTunersets_h */
