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

//TODO FIX init after declaration
namespace e2se_gui
{
class e2db : public ::e2db
{
	public:
		e2db()
		{
			debug("e2se_gui::e2db");

			options();
		}
		void options()
		{
			debug("e2se_gui::e2db", "options()");

			e2db::DEBUG = DEBUG;
			e2db_parser::PARSE_TUNERSETS = PARSE_TUNERSETS;
			e2db_parser::PARSE_LAMEDB5_PRIOR = PARSE_LAMEDB5;
		}
};
}
#endif /* e2db_gui_h */
