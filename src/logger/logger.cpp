/*!
 * e2-sat-editor/src/logger/logger.cpp
 *
 * @link https://github.com/ctlcltd/e2-sat-editor
 * @copyright e2 SAT Editor Team
 * @author Leonardo Laureti
 * @version 0.3
 * @license MIT License
 * @license GNU GPLv3 License
 */

#include <cstdio>
#include <ctime>
#include <iostream>

#include "logger.h"

namespace e2se
{

logger::logger(string ns)
{
	this->buf = new std::stringbuf;
	this->ns = "e2se." + ns;
}

logger::logger(data* obj, string ns)
{
	this->obj = obj;
	this->buf = new std::stringbuf;
	this->ns = "e2se." + ns;
}

void logger::debug(string msg)
{
	std::ostream out (buf);
	out << timestamp();
	out << ' ' << "<Debug>";
	out << ' ' << '[' << ns << ']';
	out << ' ' << msg;
	out << std::endl;
	if (this->obj->debug)
		std::cout << out.rdbuf();
	this->obj->log.append(buf->str());
	buf->str("");
}

void logger::debug(string msg, string optk, string optv)
{
	std::ostream out (buf);
	out << timestamp();
	out << ' ' << "<Debug>";
	out << ' ' << '[' << ns << ']';
	out << ' ' << msg;
	out << ' ' << optk << ':';
	out << ' ' << optv;
	out << std::endl;
	if (this->obj->debug)
		std::cout << out.rdbuf();
	this->obj->log.append(buf->str());
	buf->str("");
}

void logger::debug(string msg, string optk, int optv)
{
	this->logger::debug(msg, optk, std::to_string(optv));
}

void logger::info(string msg)
{
	std::ostream out (buf);
	out << timestamp();
	out << ' ' << "<Info>";
	out << ' ' << '[' << ns << ']';
	out << ' ' << msg;
	out << std::endl;
	if (this->obj->debug)
		std::cout << out.rdbuf();
	this->obj->log.append(buf->str());
	buf->str("");
}

void logger::info(string msg, string optk, string optv)
{
	std::ostream out (buf);
	out << timestamp();
	out << ' ' << "<Info>";
	out << ' ' << '[' << ns << ']';
	out << ' ' << msg;
	out << ' ' << optk << ':';
	out << ' ' << optv;
	out << std::endl;
	if (this->obj->debug)
		std::cout << out.rdbuf();
	this->obj->log.append(buf->str());
	buf->str("");
}

void logger::info(string msg, string optk, int optv)
{
	this->logger::info(msg, optk, std::to_string(optv));
}

void logger::error(string msg)
{
	std::ostream err (buf);
	err << timestamp();
	err << ' ' << "<Error>";
	err << ' ' << '[' << ns << ']';
	err << ' ' << msg;
	err << std::endl;
	std::cerr << err.rdbuf();
	this->obj->log.append(buf->str());
	buf->str("");
}

void logger::error(string msg, string optk, string optv)
{
	std::ostream err (buf);
	err << timestamp();
	err << ' ' << "<Error>";
	err << ' ' << '[' << ns << ']';
	err << ' ' << msg;
	err << ' ' << optk << ':';
	err << ' ' << optv;
	err << std::endl;
	std::cerr << err.rdbuf();
	this->obj->log.append(buf->str());
	buf->str("");
}

void logger::error(string msg, string optk, int optv)
{
	this->logger::error(msg, optk, std::to_string(optv));
}

string logger::timestamp()
{
#ifdef TIME_UTC
	std::timespec ct;
	std::timespec_get(&ct, TIME_UTC);
#else
	timespec ct;
	clock_gettime(CLOCK_REALTIME, &ct);
#endif
	std::tm* lct = std::localtime(&ct.tv_sec);
	char t[80];
	std::strftime(t, 80, "%Y-%m-%d %H:%M:%S", lct);
	char c[5];
	std::sprintf(c, ".%03d", int (float (ct.tv_nsec) / 1e9 * 1e3));
	return string (t) + string (c);
}

string logger::str()
{
	this->obj->size = this->obj->log.size();
	return this->obj->log;
}

string logger::str_lend()
{
	int pos = this->obj->size;
	this->obj->size = this->obj->log.size();
	return this->obj->log.substr(pos);
}

size_t logger::pos()
{
	return this->obj->log.size();
}

size_t logger::last_pos()
{
	return this->obj->size;
}

}
