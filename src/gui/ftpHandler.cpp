/*!
 * e2-sat-editor/src/gui/ftpHandler.cpp
 *
 * @link https://github.com/ctlcltd/e2-sat-editor
 * @copyright e2 SAT Editor Team
 * @author Leonardo Laureti
 * @version 1.8.0
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
	return handleConnection();
}

bool ftpHandler::closeConnection()
{
	bool ret = false;

	if (this->ftih != nullptr)
		ret = this->ftih->disconnect();

	delete this->ftih;

	this->ftih = nullptr;
	this->connected = false;

	return ret;
}

bool ftpHandler::handleConnection()
{
	bool ret = false;

	if (this->ftih == nullptr)
		this->ftih = new ftpcom;

	ret = this->ftih->connect();
	this->connected = ret;

	return ret;
}

bool ftpHandler::isConnected()
{
	return this->connected;
}

bool ftpHandler::isDisconnected()
{
	return ! this->connected;
}

void ftpHandler::settingsChanged()
{
	debug("settingsChanged");

	if (this->ftih != nullptr)
		this->ftih->disconnect();

	delete this->ftih;

	this->ftih = nullptr;
	this->connected = false;
}

string ftpHandler::getServerHostname()
{
	if (this->ftih != nullptr)
		return this->ftih->get_server_hostname();

	return "";
}

}
