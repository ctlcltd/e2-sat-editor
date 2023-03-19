/*!
 * e2-sat-editor/src/main.cpp
 * 
 * @link https://github.com/ctlcltd/e2-sat-editor
 * @copyright e2 SAT Editor Team
 * @author Leonardo Laureti
 * @version 0.5
 * @license MIT License
 * @license GNU GPLv3 License
 */

#include <cstdlib>

#include "logger/logger.h"
#include "gui/gui.h"

int main(int argc, char* argv[])
{
	bool DEBUG = true;

	if (const char* envp = std::getenv("DEBUG"))
	{
		DEBUG = true;
	}

	for (int i = 0; i < argc; i++)
	{
		if (string (argv[i]) == "debug")
		{
			DEBUG = true;
			break;
		}
	}

	e2se::logger::OBJECT = new e2se::logger::data;
	e2se::logger::OBJECT->debug = DEBUG;

	e2se_gui::gui* gui = new e2se_gui::gui(argc, argv);

	return gui->exited();
}
