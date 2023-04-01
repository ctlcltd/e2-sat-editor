/*!
 * e2-sat-editor/src/gui/editTunersetsTable.h
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

#ifndef editTunersetsTable_h
#define editTunersetsTable_h
#include <QWidget>

#include "dialAbstract.h"

namespace e2se_gui
{
class editTunersetsTable : public dialAbstract
{
	Q_DECLARE_TR_FUNCTIONS(editTunersetsTable)

	public:
		struct __state
		{
			// edit { edit: true, add: false }
			bool edit = false;
			// tunersets tvid type
			int yx;
		};

		editTunersetsTable(dataHandler* data, int yx);
		~editTunersetsTable();
		void display(QWidget* cwid);
		void setEditId(string tnid, int tvid);
		string getEditId();
		void setAddId(int tvid);
		string getAddId();

	protected:
		void layout(QWidget* cwid);
		void tableSatLayout();
		void tableTerrestrialLayout();
		void tableCableLayout();
		void tableAtscLayout();
		void store();
		void retrieve();

		__state state;
		__action action;

	private:
		int tvid;
		string tnid;
};
}
#endif /* editTunersetsTable_h */
