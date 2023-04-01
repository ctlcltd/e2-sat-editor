/*!
 * e2-sat-editor/src/gui/editTunersets.h
 *
 * @link https://github.com/ctlcltd/e2-sat-editor
 * @copyright e2 SAT Editor Team
 * @author Leonardo Laureti
 * @version 0.6
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
		struct __state
		{
			// edit { edit: true, add: false }
			bool edit = false;
			// tunersets tvid type
			int yx;
		};

		editTunersets(dataHandler* data, int yx);
		~editTunersets();
		void display(QWidget* cwid);
		void setEditId(int tvid);
		int getEditId();
		int getAddId();

	protected:
		void layout(QWidget* cwid);
		void store();
		void retrieve();

		__state state;
		__action action;

	private:
		int tvid;
		string iname;
};
}
#endif /* editTunersets_h */
