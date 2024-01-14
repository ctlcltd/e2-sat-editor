/*!
 * e2-sat-editor/src/logger/logger.cpp
 *
 * @link https://github.com/ctlcltd/e2-sat-editor
 * @copyright e2 SAT Editor Team
 * @author Leonardo Laureti
 * @version 1.1
 * @license MIT License
 * @license GNU GPLv3 License
 */

#include <cstdio>
#include <ctime>

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
	std::ostream os (buf);
	os << timestamp();
	os << ' ' << "<Debug>";
	os << ' ' << '[' << ns << ']';
	os << ' ' << cn << "::" << fn;
	os << std::endl;
	if (this->obj->debug)
		std::cout << os.rdbuf();
	this->obj->log.append(buf->str());
	buf->str("");
}

void logger::debug(string fn, string optk, string optv)
{
	std::ostream os (buf);
	os << timestamp();
	os << ' ' << "<Debug>";
	os << ' ' << '[' << ns << ']';
	os << ' ' << cn << "::" << fn;
	os << ' ';
	os << ' ' << optk << ':';
	os << ' ' << optv;
	os << std::endl;
	if (this->obj->debug)
		std::cout << os.rdbuf();
	this->obj->log.append(buf->str());
	buf->str("");
}

void logger::info(string fn, string optk, string optv)
{
	std::ostream os (buf);
	os << timestamp();
	os << ' ' << "<Info>";
	os << ' ' << '[' << ns << ']';
	os << ' ' << cn << "::" << fn;
	os << ' ';
	os << ' ' << optk << ':';
	os << ' ' << optv;
	os << std::endl;
	if (this->obj->debug)
		std::cout << os.rdbuf();
	this->obj->log.append(buf->str());
	buf->str("");
}

void logger::error(string fn, string optk, string optv)
{
	std::ostream os (buf);
	os << timestamp();
	os << ' ' << "<Error>";
	os << ' ' << '[' << ns << ']';
	os << ' ' << cn << "::" << fn;
	os << ' ';
	os << ' ' << optk << ':';
	os << ' ' << optv;
	os << std::endl;
	if (! this->obj->cli)
		std::cerr << os.rdbuf();
	this->obj->log.append(buf->str());
	buf->str("");
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

string logger::msg(string str, string param)
{
	size_t csize = str.size() + param.size();
	char cstr[csize];
	std::snprintf(cstr, csize, str.c_str(), param.c_str());

	return cstr;
}

string logger::msg(MSG msg, const char* param)
{
	string str = logger::msg(msg);
	str.append(" %s");
	return logger::msg(str, param);
}

string logger::msg(MSG msg)
{
	switch (msg)
	{
		case MSG::except_invalid_argument:
			return "Invalid argument exception";
		case MSG::except_out_of_range:
			return "Out of range exception";
		case MSG::except_bad_any_cast:
			return "Bad any cast exception";
		case MSG::except_filesystem:
			return "Filesystem exception";
		default:
			return "Uncaught exception";
	}
}

string logger::msg(string str)
{
	return str;
}

}
