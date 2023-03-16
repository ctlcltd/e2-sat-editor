/*!
 * e2-sat-editor/src/gui/about.h
 *
 * @link https://github.com/ctlcltd/e2-sat-editor
 * @copyright e2 SAT Editor Team
 * @author Leonardo Laureti
 * @version 0.5
 * @license MIT License
 * @license GNU GPLv3 License
 */

#ifndef about_h
#define about_h
#include <QWidget>
#include <QDialog>

#include "../logger/logger.h"

namespace e2se_gui_dialog
{
class about : protected e2se::log_factory
{
	public:
		about();
		virtual ~about() = default;
		void display();
		void destroy();

	protected:
		void layout();

		QDialog* dial = nullptr;
};
}
#endif /* about_h */
