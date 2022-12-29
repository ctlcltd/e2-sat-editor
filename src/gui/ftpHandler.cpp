/*!
 * e2-sat-editor/src/gui/ftpHandler.cpp
 *
 * @link https://github.com/ctlcltd/e2-sat-editor
 * @copyright e2 SAT Editor Team
 * @author Leonardo Laureti
 * @version 0.1
 * @license MIT License
 * @license GNU GPLv3 License
 */

#include "ftpHandler.h"

using namespace e2se;

namespace e2se_gui
{

ftpHandler::ftpHandler(e2se::logger::session* log)
{
	this->log = new logger(log, "ftpHandler");
	debug("ftpHandler()");
}

ftpHandler::~ftpHandler()
{
	if (this->ftih != nullptr)
		this->ftih->disconnect();
	delete this->ftih;
}

void ftpHandler::openConnection()
{
	if (this->ftih == nullptr)
		this->ftih = new ftpcom(this->log->log);
	this->ftih->connect();
}

void ftpHandler::closeConnection()
{
	if (this->ftih != nullptr)
		this->ftih->disconnect();
	delete this->ftih;
	this->ftih = nullptr;
}

//TODO FIX settings changed
bool ftpHandler::handleConnection()
{
	if (this->ftih == nullptr)
		this->ftih = new ftpcom(this->log->log);
	if (this->ftih->connect())
		return true;
	return false;
}

}
