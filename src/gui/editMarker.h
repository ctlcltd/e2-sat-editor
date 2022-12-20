/*!
 * e2-sat-editor/src/gui/editMarker.h
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

#ifndef editMarker_h
#define editMarker_h
#include <QWidget>

#include "dialAbstract.h"

namespace e2se_gui
{
class editMarker : public dialAbstract
{
	Q_DECLARE_TR_FUNCTIONS(editMarker)

	public:
		editMarker(dataHandler* data, e2se::logger::session* log);
		void display(QWidget* cwid);
		void layout();
		void store();
		void retrieve();
		void setEditId(string chid, string bname);
		string getEditId();
		void setAddId(string bname);
		string getAddId();

		struct sts
		{
			// editable
			bool edit = false;
		} state;
	private:
		string bname;
		string chid;
};
}
#endif /* editMarker_h */
