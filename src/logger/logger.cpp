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

using std::cout, std::endl;

namespace e2se
{

logger::logger(string ns)
{
	this->ns = "e2se." + ns;
}

void logger::debug()
{
	cout << '[' << ns << ']';
	cout << endl;
}

void logger::debug(string cmsg)
{
	cout << '[' << ns << ']';
	cout << ' ' << cmsg;
	cout << endl;
}

void logger::debug(string cmsg, string optk, string optv)
{
	cout << '[' << ns << ']';
	cout << ' ' << cmsg;
	cout << ' ' << optk << ':';
	cout << ' ' << optv;
	cout << endl;
}

void logger::error(string cmsg)
{
	cout << '[' << ns << ']';
	cout << ' ' << cmsg;
	cout << endl;
}

void logger::error(string cmsg, string optk, string optv)
{
	cout << '[' << ns << ']';
	cout << ' ' << cmsg;
	cout << ' ' << optk << ':';
	cout << ' ' << optv;
	cout << endl;
}

}
