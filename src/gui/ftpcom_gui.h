/*!
 * e2-sat-editor/src/gui/ftpcom_gui.h
 * 
 * @link https://github.com/ctlcltd/e2-sat-editor
 * @copyright e2 SAT Editor Team
 * @author Leonardo Laureti
 * @version 1.7.0
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

	protected:
		void setup();
		bool checkProfileIntegrity(bool strict = false);

		string msg(string str, string param) override;
		string msg(string str) override;

		void error(string fn, string optk, string optv) override;
};
}
#endif /* ftpcom_gui_h */
