/*!
 * e2-sat-editor/src/gui/dataHandler.cpp
 *
 * @link https://github.com/ctlcltd/e2-sat-editor
 * @copyright e2 SAT Editor Team
 * @author Leonardo Laureti
 * @version 0.1
 * @license MIT License
 * @license GNU GPLv3 License
 */

#include "dataHandler.h"

using namespace e2se;

namespace e2se_gui
{

dataHandler::dataHandler(e2se::logger::session* log)
{
	this->log = new logger(log, "dataHandler");
	debug("dataHandler()");
}

dataHandler::~dataHandler()
{
	delete this->dbih;
}

void dataHandler::newFile()
{
	debug("newFile()");

	delete this->dbih;
	this->dbih = new e2db(this->log->log);
	this->newfile = true;
}

bool dataHandler::readFile(string filename)
{
	debug("readFile()");

	delete this->dbih;
	this->dbih = new e2db(this->log->log);
	this->newfile = true;
	this->filename = filename;
	return this->dbih->prepare(filename);
}

bool dataHandler::writeFile(string path, bool overwrite)
{
	debug("writeFile()");

	return this->dbih->write(path, overwrite);
}

}
