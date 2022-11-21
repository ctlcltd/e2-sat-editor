/*!
 * e2-sat-editor/src/logger/logger.cpp
 *
 * @link https://github.com/ctlcltd/e2-sat-editor
 * @copyright e2 SAT Editor Team
 * @author Leonardo Laureti
 * @version 0.1
 * @license MIT License
 * @license GNU GPLv3 License
 */

#include <iostream>

#include "logger.h"

namespace e2se
{

logger::logger(string ns)
{
	this->buf = new std::stringbuf;
	this->ns = "e2se." + ns;
}

logger::logger(session* log, string ns)
{
	this->log = log;
	this->buf = new std::stringbuf;
	this->ns = "e2se." + ns;
}

//TODO FIX crash isnull
void logger::debug(string msg)
{
	std::ostream out (buf);
	out << '[' << ns << ']';
	out << ' ' << msg;
	out << std::endl;
	if (this->log->debug)
		std::cout << out.rdbuf();
	this->log->text.append(buf->str());
	buf->str("");
}

//TODO FIX crash isnull
void logger::debug(string msg, string optk, string optv)
{
	std::ostream out (buf);
	out << '[' << ns << ']';
	out <<  ' ' << msg;
	out << ' ' << optk << ':';
	out << ' ' << optv;
	out << std::endl;
	if (this->log->debug)
		std::cout << out.rdbuf();
	this->log->text.append(buf->str());
	buf->str("");
}

void logger::debug(string msg, string optk, int optv)
{
	this->logger::debug(msg, optk, std::to_string(optv));
}

void logger::debug(string msg, string optk, bool optv)
{
	this->logger::debug(msg, optk, string (optv ? "true" : "false"));
}

void logger::error(string msg)
{
	std::ostream out (buf);
	out << '[' << ns << ']';
	out << ' ' << msg;
	out << std::endl;
	std::clog << out.rdbuf();
	this->log->text.append(buf->str());
	buf->str("");
}

void logger::error(string msg, string optk, string optv)
{
	std::ostream out (buf);
	out << '[' << ns << ']';
	out << ' ' << msg;
	out << ' ' << optk << ':';
	out << ' ' << optv;
	out << std::endl;
	std::clog << out.rdbuf();
	this->log->text.append(buf->str());
	buf->str("");
}

string logger::str()
{
	return this->log->text;
}

}
