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

using std::string;

#ifndef editTunersets_h
#define editTunersets_h
#include <QWidget>

#include "dialAbstract.h"

namespace e2se_gui
{
class editTunersets : public dialAbstract
{
	Q_DECLARE_TR_FUNCTIONS(editTunersets)

	public:
		editTunersets(e2db* dbih, int ty, e2se::logger::session* log);
		void display(QWidget* cwid);
		void layout();
		void store();
		void retrieve();
		void setEditID(string todo);
		string getEditID();

		struct sts
		{
			// editable
			bool edit = false;
			// tools tunersets current type
			int ty;
		} state;
};
}
#endif /* editTunersets_h */
