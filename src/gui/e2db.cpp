/*!
 * e2-sat-editor/src/gui/e2db.cpp
 * 
 * @link https://github.com/ctlcltd/e2-sat-editor
 * @copyright e2 SAT Editor Team
 * @author Leonardo Laureti
 * @version 0.1
 * @license MIT License
 * @license GNU GPLv3 License
 */

#include "e2db.h"

using namespace std;

namespace e2se_gui
{
void e2db::options()
{
	e2db::DEBUG = DEBUG;
	e2db::PARSE_TUNERSETS = PARSE_TUNERSETS;
	e2db::PARSE_LAMEDB5_PRIOR = PARSE_LAMEDB5;
}
}
