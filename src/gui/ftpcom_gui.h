/*!
 * e2-sat-editor/src/gui/ftpcom_gui.h
 * 
 * @link https://github.com/ctlcltd/e2-sat-editor
 * @copyright e2 SAT Editor Team
 * @author Leonardo Laureti
 * @version 0.5
 * @license MIT License
 * @license GNU GPLv3 License
 */

#ifndef ftpcom_gui_h
#define ftpcom_gui_h
#include <QApplication>

#include "../ftpcom/ftpcom.h"

namespace e2se_gui
{
class ftpcom : public ::e2se_ftpcom::ftpcom
{
	Q_DECLARE_TR_FUNCTIONS(ftpcom)

	public:
		ftpcom();
		~ftpcom(); // final destructor
		void didChange();

	protected:
		void setup();
		void error(string fn, string optk, string optv) override;
		void error(string fn, string optk, int optv) override;
		string trs(string str) override;
		string trw(string str, string param) override;
};
}
#endif /* ftpcom_gui_h */
