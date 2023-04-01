/*!
 * e2-sat-editor/src/gui/editTunersetsTransponder.h
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
		struct __state
		{
			// edit { edit: true, add: false }
			bool edit = false;
			// tunersets tvid type
			int yx;
		};

		editTunersetsTransponder(dataHandler* data, int yx);
		~editTunersetsTransponder();
		void display(QWidget* cwid);
		void setEditId(string trid, string tnid, int tvid);
		string getEditId();
		void setAddId(string tnid, int tvid);
		string getAddId();

	protected:
		void layout(QWidget* cwid);
		void leadSatLayout();
		void leadTerrestrialLayout();
		void leadCableLayout();
		void leadAtscLayout();
		void sideSatLayout();
		void sideTerrestrialLayout();
		void sideCableLayout();
		void thirdSatLayout();
		void store();
		void retrieve();

		__state state;
		__action action;

	private:
		int tvid;
		string tnid;
		string trid;
};
}
#endif /* editTunersetsTransponder_h */
