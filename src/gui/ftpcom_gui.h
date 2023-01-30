/*!
 * e2-sat-editor/src/gui/ftpcom_gui.h
 * 
 * @link https://github.com/ctlcltd/e2-sat-editor
 * @copyright e2 SAT Editor Team
 * @author Leonardo Laureti
 * @version 0.3
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
		void didChange();

	protected:
		void setup();
		string trs(string str) override;
		string trw(string str, string param) override;
		void error(string tmsg, string rmsg) override;
};
}
#endif /* ftpcom_gui_h */
