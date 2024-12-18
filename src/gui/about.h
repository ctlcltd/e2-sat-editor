/*!
 * e2-sat-editor/src/gui/about.h
 *
 * @link https://github.com/ctlcltd/e2-sat-editor
 * @copyright e2 SAT Editor Team
 * @author Leonardo Laureti
 * @version 1.8.0
 * @license MIT License
 * @license GNU GPLv3 License
 */

#ifndef about_h
#define about_h
#include <QApplication>
#include <QWidget>
#include <QDialog>

#include "../logger/logger.h"
#include "theme.h"

namespace e2se_gui
{
class about : protected e2se::log_factory
{
	Q_DECLARE_TR_FUNCTIONS(about)

	public:
		about();
		virtual ~about();
		void display();
		void destroy();

	protected:
		void layout();

		QDialog* dial = nullptr;
		e2se_gui::theme* theme;
};
}
#endif /* about_h */
