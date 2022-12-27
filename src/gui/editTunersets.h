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
		editTunersets(dataHandler* data, int yx, e2se::logger::session* log);
		void display(QWidget* cwid);
		void layout();
		void store();
		void retrieve();
		void setEditId(int tvid);
		int getEditId();
		int getAddId();

		struct __state
		{
			// edit { edit: true, add: false }
			bool edit = false;
			// tunersets tvid type
			int yx;
		} state;
	private:
		int tvid;
		string iname;
};
}
#endif /* editTunersets_h */
