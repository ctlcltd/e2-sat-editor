/*!
 * e2-sat-editor/src/gui/editFavourite.h
 *
 * @link https://github.com/ctlcltd/e2-sat-editor
 * @copyright e2 SAT Editor Team
 * @author Leonardo Laureti
 * @version 1.5.0
 * @license MIT License
 * @license GNU GPLv3 License
 */

#include <vector>
#include <string>

using std::vector, std::string;

#ifndef editFavourite_h
#define editFavourite_h
#include <QWidget>

#include "dialAbstract.h"

namespace e2se_gui
{
class editFavourite : public dialAbstract
{
	Q_DECLARE_TR_FUNCTIONS(editFavourite)

	public:
		struct __state
		{
			// edit { edit: true, add: false }
			bool edit = false;
		};

		editFavourite(dataHandler* data);
		~editFavourite();
		void display(QWidget* cwid);
		void show(bool retr = false);
		void reset();
		void layout(QWidget* cwid);
		void store();
		void setEditId(string chid, string bname);
		string getEditId();
		void setAddId(string bname);

	protected:
		void referenceLayout();
		void streamLayout();
		void retrieve();
		void retrieve(string txid);

		QComboBox* dtf0yx;
		vector<QWidget*> afields;

		__state state;
		__action action;

	private:
		string bname;
		string chid;
};
}
#endif /* editFavourite_h */
