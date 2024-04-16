/*!
 * e2-sat-editor/src/gui/editMarker.h
 *
 * @link https://github.com/ctlcltd/e2-sat-editor
 * @copyright e2 SAT Editor Team
 * @author Leonardo Laureti
 * @version 1.4.0
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
		struct __state
		{
			// edit { edit: true, add: false }
			bool edit = false;
		};

		editMarker(dataHandler* data);
		~editMarker();
		void display(QWidget* cwid);
		void setEditId(string chid, string bname);
		string getEditId();
		void setAddId(string bname);

	protected:
		void layout(QWidget* cwid);
		void store();
		void retrieve();

		__state state;
		__action action;

	private:
		string bname;
		string chid;
};
}
#endif /* editMarker_h */
