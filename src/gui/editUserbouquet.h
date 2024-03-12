/*!
 * e2-sat-editor/src/gui/editUserbouquet.h
 *
 * @link https://github.com/ctlcltd/e2-sat-editor
 * @copyright e2 SAT Editor Team
 * @author Leonardo Laureti
 * @version 1.2.0
 * @license MIT License
 * @license GNU GPLv3 License
 */

#include <string>

using std::string;

#ifndef editUserbouquet_h
#define editUserbouquet_h
#include <QWidget>

#include "dialAbstract.h"

namespace e2se_gui
{
class editUserbouquet : public dialAbstract
{
	Q_DECLARE_TR_FUNCTIONS(editUserbouquet)

	public:
		struct __state
		{
			// edit { edit: true, add: false }
			bool edit = false;
			// tree top level index
			int ti;
		};

		editUserbouquet(dataHandler* data, int ti);
		~editUserbouquet();
		void display(QWidget* cwid);
		void setEditId(string bname);
		string getEditId();

	protected:
		void layout(QWidget* cwid);
		void store();
		void retrieve();
		void save();

		__state state;
		__action action;

	private:
		string bname;
};
}
#endif /* editUserbouquet_h */
