/*!
 * e2-sat-editor/src/gui/editBouquet.h
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

#ifndef editBouquet_h
#define editBouquet_h
#include <QApplication>
#include <QWidget>
#include <QDialog>
#include <QGridLayout>

#include "../logger.h"
#include "e2db_gui.h"

namespace e2se_gui
{
class editBouquet : protected e2se::log_factory
{
	Q_DECLARE_TR_FUNCTIONS(editBouquet)

	public:
		editBouquet(e2db* dbih, int ti);
		void display(QWidget* cwid);
		void layout();
		void store();
		void retrieve();
		void save();
		void setEditID(string bname);
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
			// bouquets_tree top level index 
			int ti;
		} state;
		e2db* dbih;
		string bname;
};
}
#endif /* editBouquet_h */
