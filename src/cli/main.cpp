/*!
 * e2-sat-editor/src/cli/main.cpp
 * 
 * @link https://github.com/ctlcltd/e2-sat-editor
 * @copyright e2 SAT Editor Team
 * @author Leonardo Laureti
 * @version 1.3.0
 * @license MIT License
 * @license GNU GPLv3 License
 */

#include "../logger/logger.h"
#include "e2db_cli.h"

int main(int argc, char* argv[])
{
	e2se::logger::OBJECT = new e2se::logger::data;
	e2se::logger::OBJECT->debug = false;
	e2se::logger::OBJECT->cli = true;

	e2se_cli::e2db_cli* cli = new e2se_cli::e2db_cli(argc, argv);

	return cli->exited();
}
