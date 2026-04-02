/*!
 * e2-sat-editor/src/gui/e2db_console_gui.cpp
 * 
 * @link https://github.com/ctlcltd/e2-sat-editor
 * @copyright e2 SAT Editor Team
 * @author Leonardo Laureti
 * @version 1.9.0
 * @license MIT License
 * @license GNU GPLv3 License
 */

#include <cstdio>
#include <ctime>
#include <clocale>

#include "e2db_console_gui.h"

namespace e2se_gui
{

e2db_console_gui::e2db_console_gui()
{
	std::setlocale(LC_NUMERIC, "C");

	if (__objio.out == OBJIO::byline)
		__objio.hrn = false;
	else if (__objio.out == OBJIO::json)
		__objio.hrn = false;

	command_version();
}

e2db_console_gui::~e2db_console_gui()
{
}

}
