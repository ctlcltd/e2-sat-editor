/*!
 * e2-sat-editor/src/gui/tools.h
 *
 * @link https://github.com/ctlcltd/e2-sat-editor
 * @copyright e2 SAT Editor Team
 * @author Leonardo Laureti
 * @version 0.1
 * @license MIT License
 * @license GNU GPLv3 License
 */

#ifndef tools_h
#define tools_h
#include <QGridLayout>

#include "../logger.h"
#include "e2db_gui.h"
#include "editTunersets.h"

namespace e2se_gui_tools
{
class tools : protected e2se::log_factory
{
	public:
		tools(QGridLayout* root);
		QGridLayout* root;
        void editTunersets(e2se_gui::e2db* dbih, int ytype);
		void closeTunersets();
		void destroy();
	private:
		e2se_gui::editTunersets* tns = nullptr;
};
}
#endif /* tools_h */
