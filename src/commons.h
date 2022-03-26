/*!
 * e2-sat-editor/src/commons.h
 *
 * @link https://github.com/ctlcltd/e2-sat-editor
 * @copyright e2 SAT Editor Team
 * @author Leonardo Laureti
 * @version 0.1
 * @license MIT License
 * @license GNU GPLv3 License
 */

#include <string>
#include <iostream>

using std::string, std::cout, std::endl;

#ifndef commons_h
#define commons_h
namespace e2se
{
const bool DEBUG = true;
const bool DEBUG_E2DB = false;
const bool DEBUG_TOOLBAR = true;
//TODO FIX mingw32
#ifdef _WIN32
const bool PARSER_TUNERSETS = false;
#else
const bool PARSER_TUNERSETS = true;
#endif
const bool PARSER_LAMEDB5 = false;
const bool MAKER_LAMEDB5 = true;

static void debug(string ns, string cmsg = "", string optk = "", string optv = "", string indt = " ")
{
	if (! DEBUG) return;
	cout << '[' << "e2se." << ns << ']';
	if (! cmsg.empty()) cout << indt << cmsg;
	if (! optk.empty()) cout << indt << optk << ':';
	if (! optv.empty()) cout << ' ' << optv;
	cout << endl;
}
static void error(string ns, string cmsg = "", string optk = "", string optv = "", string indt = " ")
{
	cout << '[' << "e2se." << ns << ']';
	if (! cmsg.empty()) cout << indt << cmsg;
	if (! optk.empty()) cout << indt << optk << ':';
	if (! optv.empty()) cout << ' ' << optv;
	cout << endl;
}
}
#endif /* commons_h */
