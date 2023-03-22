/*!
 * e2-sat-editor/src/gui/ftpHandler.cpp
 *
 * @link https://github.com/ctlcltd/e2-sat-editor
 * @copyright e2 SAT Editor Team
 * @author Leonardo Laureti
 * @version 0.5
 * @license MIT License
 * @license GNU GPLv3 License
 */

#include "ftpHandler.h"

using namespace e2se;

namespace e2se_gui
{

ftpHandler::ftpHandler()
{
	this->log = new logger("gui", "ftpHandler");
}

ftpHandler::~ftpHandler()
{
	debug("~ftpHandler");

	if (this->ftih != nullptr)
		this->ftih->disconnect();

	delete this->ftih;
	delete this->log;
}

bool ftpHandler::openConnection()
{
	if (this->ftih == nullptr)
		this->ftih = new ftpcom;

	return this->ftih->connect();
}

bool ftpHandler::closeConnection()
{
	bool ret = false;

	if (this->ftih != nullptr)
		ret = this->ftih->disconnect();

	delete this->ftih;

	this->ftih = nullptr;

	return ret;
}

bool ftpHandler::handleConnection()
{
	if (this->ftih == nullptr)
		this->ftih = new ftpcom;

	return this->ftih->connect();
}

void ftpHandler::settingsChanged()
{
	debug("settingsChanged");

	if (this->ftih != nullptr)
		this->ftih->didChange();
}

string ftpHandler::getServerHostname()
{
	if (this->ftih != nullptr)
		return this->ftih->get_server_hostname();

	return "";
}

}
