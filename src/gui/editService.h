/*!
 * e2-sat-editor/src/gui/editService.h
 *
 * @link https://github.com/ctlcltd/e2-sat-editor
 * @copyright e2 SAT Editor Team
 * @author Leonardo Laureti
 * @version 0.1
 * @license MIT License
 * @license GNU GPLv3 License
 */

using namespace std;

#ifndef editService_h
#define editService_h
#include <QApplication>
#include <QWidget>
#include <QGridLayout>
#include "e2db_gui.h"

namespace e2se_gui
{
class editService
{
	Q_DECLARE_TR_FUNCTIONS(editService)

	public:
		editService(e2db* dbih, QWidget* cwid, bool add = false);
		void serviceLayout();
		void transponderLayout();
		void paramsLayout();
		QWidget* widget;
	protected:
		QGridLayout* dtform;
	private:
		e2db* dbih;
};
}
#endif /* editService_h */
