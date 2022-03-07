/*!
 * e2-sat-editor/src/gui/e2db.h
 * 
 * @link https://github.com/ctlcltd/e2-sat-editor
 * @copyright e2 SAT Editor Team
 * @author Leonardo Laureti
 * @version 0.1
 * @license MIT License
 * @license GNU GPLv3 License
 */

#ifndef e2db_gui_h
#define e2db_gui_h
#include "../commons.h"
#include "../e2db.h"

namespace e2se_gui
{
class e2db : public ::e2db
{
	public:
		void options();
};
}
#endif /* e2db_gui_h */
