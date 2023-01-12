/*!
 * e2-sat-editor/src/gui/ftpcom_gui.h
 * 
 * @link https://github.com/ctlcltd/e2-sat-editor
 * @copyright e2 SAT Editor Team
 * @author Leonardo Laureti
 * @version 0.2
 * @license MIT License
 * @license GNU GPLv3 License
 */

#include <iostream>

#ifndef ftpcom_gui_h
#define ftpcom_gui_h
#include <QApplication>
#include <QSettings>

#include "../ftpcom/ftpcom.h"

namespace e2se_gui
{
class ftpcom : public ::e2se_ftpcom::ftpcom
{
	Q_DECLARE_TR_FUNCTIONS(ftpcom)

	public:
		ftpcom(e2se::logger::session* log);

	protected:
		string trs(string str) override;
		string trw(string str, string param) override;
		void error(string tmsg, string rmsg) override;
		QSettings* sets;
};
}
#endif /* ftpcom_gui_h */
