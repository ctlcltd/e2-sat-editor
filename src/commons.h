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

using namespace std;

#ifndef commons_h
#define commons_h
const bool DEBUG = true;
const bool DEBUG_E2DB = false;
const bool DEBUG_TOOLBAR = true;
const bool PARSE_TUNERSETS = true;
const bool PARSE_LAMEDB5 = false;

static void debug(string ns, string cmsg = "", string optk = "", string optv = "", string indt = " ")
{
	if (DEBUG) return;
	cout << ns;
	if (! cmsg.empty()) cout << indt << cmsg;
	if (! optk.empty()) cout << indt << optk << ":";
	if (! optv.empty()) cout << " " << optv;
	cout << endl;
}
static void error(string ns, string cmsg = "", string optk = "", string optv = "", string indt = " ")
{
	cout << ns;
	if (! cmsg.empty()) cout << indt << cmsg;
	if (! optk.empty()) cout << indt << optk << ":";
	if (! optv.empty()) cout << " " << optv;
	cout << endl;
}
#endif /* commons_h */
