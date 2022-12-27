/*!
 * e2-sat-editor/src/gui/editBouquet.h
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
		editBouquet(dataHandler* data, int ti, e2se::logger::session* log);
		void display(QWidget* cwid);
		void layout();
		void store();
		void retrieve();
		void save();
		void setEditId(string bname);
		string getEditId();
		string getAddId();

		struct __state
		{
			// edit { edit: true, add: false }
			bool edit = false;
			// tree top level index
			int ti;
		} state;
	private:
		string bname;
};
}
#endif /* editBouquet_h */
