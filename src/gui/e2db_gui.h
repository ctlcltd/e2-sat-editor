/*!
 * e2-sat-editor/src/gui/e2db_gui.h
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
class e2db : public ::e2db::e2db
{
	public:
		e2db()
		{
			e2se::debug("e2db");

			options();
		}
		void options()
		{
			e2se::debug("e2db", "options()");

			e2db::DEBUG = e2se::DEBUG;
			e2db::PARSER_TUNERSETS = e2se::PARSER_TUNERSETS;
			e2db::PARSER_LAMEDB5_PRIOR = e2se::PARSER_LAMEDB5;
			e2db::MAKER_LAMEDB5 = e2se::MAKER_LAMEDB5;
		}
};
}
#endif /* e2db_gui_h */
