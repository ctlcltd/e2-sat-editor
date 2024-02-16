/*!
 * e2-sat-editor/src/main.cpp
 * 
 * @link https://github.com/ctlcltd/e2-sat-editor
 * @copyright e2 SAT Editor Team
 * @author Leonardo Laureti
 * @version 1.2.0
 * @license MIT License
 * @license GNU GPLv3 License
 */

#include <cstdlib>
#include <string>

#include "e2se_defs.h"
#include "logger/logger.h"
#include "gui/gui.h"

int main(int argc, char* argv[])
{
	bool DEBUG = false;
#if E2SE_BUILD == E2SE_TARGET_DEBUG
	DEBUG = true;
#endif

	const char* envp = std::getenv("DEBUG");
	if (envp)
	{
		DEBUG = true;
	}

	for (int i = 0; i < argc; i++)
	{
		std::string arg = argv[i];

		if (arg == "-debug" || arg == "--debug")
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
