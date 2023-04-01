/*!
 * e2-sat-editor/src/logger/logger.cpp
 *
 * @link https://github.com/ctlcltd/e2-sat-editor
 * @copyright e2 SAT Editor Team
 * @author Leonardo Laureti
 * @version 0.6
 * @license MIT License
 * @license GNU GPLv3 License
 */

#include <cstdio>
#include <ctime>
#include <iostream>

#include "logger.h"

namespace e2se
{

logger::logger(string ns, string cn)
{
	this->obj = OBJECT;
	this->buf = new std::stringbuf;
	this->ns = PREFIX + '.' + ns;
	this->cn = cn;

	debug(cn);
}

logger::logger(data* obj, string ns, string cn)
{
	this->obj = obj;
	this->buf = new std::stringbuf;
	this->ns = PREFIX + '.' + ns;
	this->cn = cn;
}

void logger::debug(string fn)
{
	std::ostream out (buf);
	out << timestamp();
	out << ' ' << "<Debug>";
	out << ' ' << '[' << ns << ']';
	out << ' ' << cn << "::" << fn;
	out << std::endl;
	if (this->obj->debug)
		std::cout << out.rdbuf();
	this->obj->log.append(buf->str());
	buf->str("");
}

void logger::debug(string fn, string optk, string optv)
{
	std::ostream out (buf);
	out << timestamp();
	out << ' ' << "<Debug>";
	out << ' ' << '[' << ns << ']';
	out << ' ' << cn << "::" << fn;
	out << ' ';
	out << ' ' << optk << ':';
	out << ' ' << optv;
	out << std::endl;
	if (this->obj->debug)
		std::cout << out.rdbuf();
	this->obj->log.append(buf->str());
	buf->str("");
}

void logger::debug(string fn, string optk, int optv)
{
	this->logger::debug(fn, optk, std::to_string(optv));
}

void logger::info(string fn)
{
	std::ostream out (buf);
	out << timestamp();
	out << ' ' << "<Info>";
	out << ' ' << '[' << ns << ']';
	out << ' ' << cn << "::" << fn;
	out << std::endl;
	if (this->obj->debug)
		std::cout << out.rdbuf();
	this->obj->log.append(buf->str());
	buf->str("");
}

void logger::info(string fn, string optk, string optv)
{
	std::ostream out (buf);
	out << timestamp();
	out << ' ' << "<Info>";
	out << ' ' << '[' << ns << ']';
	out << ' ' << cn << "::" << fn;
	out << ' ';
	out << ' ' << optk << ':';
	out << ' ' << optv;
	out << std::endl;
	if (this->obj->debug)
		std::cout << out.rdbuf();
	this->obj->log.append(buf->str());
	buf->str("");
}

void logger::info(string fn, string optk, int optv)
{
	this->logger::info(fn, optk, std::to_string(optv));
}

void logger::error(string fn)
{
	std::ostream err (buf);
	err << timestamp();
	err << ' ' << "<Error>";
	err << ' ' << '[' << ns << ']';
	err << ' ' << cn << "::" << fn;
	err << std::endl;
	std::cerr << err.rdbuf();
	this->obj->log.append(buf->str());
	buf->str("");
}

void logger::error(string fn, string optk, string optv)
{
	std::ostream err (buf);
	err << timestamp();
	err << ' ' << "<Error>";
	err << ' ' << '[' << ns << ']';
	err << ' ' << cn << "::" << fn;
	err << ' ';
	err << ' ' << optk << ':';
	err << ' ' << optv;
	err << std::endl;
	std::cerr << err.rdbuf();
	this->obj->log.append(buf->str());
	buf->str("");
}

void logger::error(string fn, string optk, int optv)
{
	this->logger::error(fn, optk, std::to_string(optv));
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
	char c[8];
	std::snprintf(c, 8, ".%06d", int (float (ct.tv_nsec) / 1e9 * 1e6));
	return string (t) + string (c);
}

string logger::str()
{
	return this->obj->log;
}

size_t logger::size()
{
	return this->obj->log.size();
}

}
