/*!
 * e2-sat-editor/src/gui/editTransponder.h
 *
 * @link https://github.com/ctlcltd/e2-sat-editor
 * @copyright e2 SAT Editor Team
 * @author Leonardo Laureti
 * @version 1.8.0
 * @license MIT License
 * @license GNU GPLv3 License
 */

#include <vector>
#include <string>

using std::vector, std::string;

#ifndef editTransponder_h
#define editTransponder_h
#include <QWidget>
#include <QComboBox>

#include "dialAbstract.h"

namespace e2se_gui
{
class editTransponder : public dialAbstract
{
	Q_DECLARE_TR_FUNCTIONS(editTransponder)

	public:
		struct __state
		{
			// edit { edit: true, add: false }
			bool edit = false;
			// current transponder ytype
			int yx;
		};

		editTransponder(dataHandler* data);
		~editTransponder();
		void display(QWidget* cwid);
		void show(bool retr = false);
		void reset();
		void layout(QWidget* cwid);
		void layoutChange(int vx);
		void store();
		void setEditId(string txid);
		string getEditId();
		void setAddId();

	protected:
		void leadSatLayout();
		void leadTerrestrialLayout();
		void leadCableLayout();
		void leadAtscLayout();
		void sideSatLayout();
		void sideTerrestrialLayout();
		void sideCableLayout();
		void thirdSatLayout();
		void thirdTerrestrialLayout();
		void thirdCableLayout();
		void thirdAtscLayout();
		void typeComboChanged(int index);
		void computeDvbns();
		void retrieve();
		void retrieve(string txid);

		QComboBox* dtf0yx;
		vector<QWidget*> afields;

		__state state;
		__action action;

	private:
		string txid;
};
}
#endif /* editTransponder_h */
