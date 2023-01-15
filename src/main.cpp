/*!
 * e2-sat-editor/src/main.cpp
 * 
 * @link https://github.com/ctlcltd/e2-sat-editor
 * @copyright e2 SAT Editor Team
 * @author Leonardo Laureti
 * @version 0.2
 * @license MIT License
 * @license GNU GPLv3 License
 */

#include "logger/logger.h"
#include "gui/gui.h"

#ifndef __MINGW32__
int main(int argc, char* argv[], char* envp[])
{
	bool DEBUG = false;

	for (int i = 0; envp[i] != NULL; i++)
	{
		if (string (envp[i]).substr(0, 6) == "DEBUG=")
		{
			DEBUG = true;
			break;
		}
	}
	
	for (int i = 0; i < argc; i++)
	{
		if (string (argv[i]) == "debug")
		{
			DEBUG = true;
			break;
		}
	}

	e2se::logger::session* log = new e2se::logger::session;
	log->debug = DEBUG;

	e2se_gui::gui* gui = new e2se_gui::gui(argc, argv, log);

	return gui->exec();
}
#else
int main(int argc, char* argv[])
{
	bool DEBUG = true;

	e2se::logger::session* log = new e2se::logger::session;
	log->debug = DEBUG;

	e2se_gui::gui* gui = new e2se_gui::gui(argc, argv, log);

	return gui->exec();
}
#endif
