/*!
 * e2-sat-editor/src/gui/dataHandler.cpp
 *
 * @link https://github.com/ctlcltd/e2-sat-editor
 * @copyright e2 SAT Editor Team
 * @author Leonardo Laureti
 * @version 1.3.0
 * @license MIT License
 * @license GNU GPLv3 License
 */

#include "../e2se_defs.h"

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

bool dataHandler::readFile(string path)
{
	debug("readFile");

	delete this->dbih;

	this->dbih = new e2db;
	this->newfile = false;
	this->changed = false;

	if (this->dbih->prepare(path))
	{
		this->path = path;
		this->newfile = false;

		return true;
	}

	return false;
}

//TODO TEST sort bss TV, Radio
bool dataHandler::readBlob(string path, unordered_map<string, e2db::e2db_file> files)
{
	debug("readBlob");

	delete this->dbih;

	this->dbih = new e2db;
	this->newfile = false;
	this->changed = false;

	try
	{
		this->dbih->importBlob(files);

		if (! this->path.empty())
			this->path = path;
	}
	catch (...)
	{
		return false;
	}

	return true;
}

bool dataHandler::writeFile(string path)
{
	debug("writeFile");

	if (this->dbih == nullptr)
		return false;

	if (this->dbih->write(path))
	{
		this->path = path;

		return true;
	}

	return false;
}

bool dataHandler::haveErrors()
{
	if (this->dbih != nullptr)
		return this->dbih->haveErrors();

	return false;
}

vector<string> dataHandler::getErrors()
{
	if (this->dbih != nullptr)
		return this->dbih->getErrors();

	return {};
}

void dataHandler::clearErrors()
{
	if (this->dbih != nullptr)
		return this->dbih->clearErrors();
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

string dataHandler::getPath()
{
	return this->path;
}

void dataHandler::settingsChanged()
{
	debug("settingsChanged");

	if (this->dbih != nullptr)
		this->dbih->didChange();
}

}
