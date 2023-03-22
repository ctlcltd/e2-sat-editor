/*!
 * e2-sat-editor/src/gui/dataHandler.cpp
 *
 * @link https://github.com/ctlcltd/e2-sat-editor
 * @copyright e2 SAT Editor Team
 * @author Leonardo Laureti
 * @version 0.5
 * @license MIT License
 * @license GNU GPLv3 License
 */

#include "dataHandler.h"

using namespace e2se;

namespace e2se_gui
{

dataHandler::dataHandler()
{
	this->log = new logger("gui", "dataHandler");
}

dataHandler::~dataHandler()
{
	debug("~dataHandler");

	delete this->dbih;
	delete this->log;
}

void dataHandler::newFile()
{
	debug("newFile");

	delete this->dbih;

	this->dbih = new e2db;
	this->newfile = true;
	this->changed = false;
}

bool dataHandler::readFile(string filename)
{
	debug("readFile");

	delete this->dbih;

	this->dbih = new e2db;
	this->newfile = false;
	this->changed = false;

	if (this->dbih->prepare(filename))
	{
		this->filename = filename;
		this->newfile = false;
		return true;
	}

	return false;
}

bool dataHandler::writeFile(string path)
{
	debug("writeFile");

	if (this->dbih == nullptr)
		return false;

	if (this->dbih->write(path))
	{
		this->filename = path;
		return true;
	}

	return false;
}

void dataHandler::setChanged(bool changed)
{
	this->changed = changed;
}

bool dataHandler::hasChanged()
{
	return this->changed;
}

void dataHandler::setNewfile(bool newfile)
{
	this->newfile = newfile;
}

bool dataHandler::isNewfile()
{
	return this->newfile;
}

string dataHandler::getFilename()
{
	return this->filename;
}

void dataHandler::settingsChanged()
{
	debug("settingsChanged");

	if (this->dbih != nullptr)
		this->dbih->didChange();
}

}
