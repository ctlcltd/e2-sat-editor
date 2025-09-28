/*!
 * e2-sat-editor/src/gui/editBouquet.h
 *
 * @link https://github.com/ctlcltd/e2-sat-editor
 * @copyright e2 SAT Editor Team
 * @author Leonardo Laureti
 * @version 1.8.1
 * @license MIT License
 * @license GNU GPLv3 License
 */

#include <string>

using std::string;

#ifndef editBouquet_h
#define editBouquet_h
#include <QWidget>

#include "dialAbstract.h"

namespace e2se_gui
{
class editBouquet : public dialAbstract
{
	Q_DECLARE_TR_FUNCTIONS(editBouquet)

	public:
		struct __state
		{
			// edit { edit: true, add: false }
			bool edit = false;
		};

		editBouquet(dataHandler* data);
		~editBouquet();
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
#endif /* editBouquet_h */
