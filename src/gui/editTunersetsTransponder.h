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

using std::string;

#ifndef editTunersetsTransponder_h
#define editTunersetsTransponder_h
#include <QWidget>

#include "dialAbstract.h"

namespace e2se_gui
{
class editTunersetsTransponder : public dialAbstract
{
	Q_DECLARE_TR_FUNCTIONS(editTunersetsTransponder)

	public:
		editTunersetsTransponder(e2db* dbih, e2se::logger::session* log);
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
		} state;
};
}
#endif /* editTunersetsTransponder_h */
