/*!
 * e2-sat-editor/src/commons.h
 *
 * @author Leonardo Laureti
 * @version 0.1
 * @license MIT License
 */
#include <iostream>
#include <string>
#include <unordered_map>

using namespace std;

#ifndef commons_h
#define commons_h
const bool DEBUG = true;
const bool DEBUG_E2DB = false;
const bool DEBUG_TOOLBAR = true;

const string SAT_POL[4] = {"H", "V", "L", "R"};
const string SAT_FEC[11] = {"", "Auto", "1/2", "2/3", "3/4", "5/6", "7/8", "3/5", "4/5", "8/9", "9/10"};
const string SAT_INV[3] = {"Auto", "On", "Off"};
const string SAT_SYS[2] = {"DVB-S", "DVB-S2"};
const string SAT_MOD[4] = {"Auto", "QPSK", "QAM16", "8PSK"};
const string SAT_ROL[4] = {"Auto", "QPSK", "QAM16", "8PSK"};
const string SAT_PIL[3] = {"Auto", "Off", "On"};

const string TER_BAND[4] = {"Auto", "8Mhz", "7Mhz", "6Mhz"};
const string TER_HPFEC[6] = {"Auto", "1/2", "2/3", "3/4", "5/6", "7/8"};
const string TER_LPFEC[6] = {"Auto", "1/2", "2/3", "3/4", "5/6", "7/8"};
const string TER_MOD[4] = {"Auto", "QPSK", "QAM16", "QAM64"};
const string TER_TRXMODE[5] = {"Auto", "2k", "8k"};
const string TER_GUARD[5] = {"Auto", "1/32", "1/16", "1/8", "1/4"};
const string TER_HIER[5] = {"Auto", "", "1", "2", "4"};
const string TER_INV[3] = {"Auto", "On", "Off"};

const string CAB_INV[3] = {"Auto", "On", "Off"};
const string CAB_MOD[6] = {"Auto", "QAM16", "QAM32", "QAM64", "QAM128", "QAM256"};
const string CAB_IFEC[8] = {"", "Auto", "1/2", "2/3", "3/4", "5/6", "7/8", "8/9"};

const unordered_map<char, char> PVDR_DATA = {{'p', '0'}, {'c', '1'}, {'C', '2'}, {'f', '3'}};

const unordered_map<int, string> STYPES = {{0, "Data"}, {1, "TV"}, {2, "Radio"}, {10, "Radio"}, {12, "TV"}, {17, "UHD"}, {22, "H.264"}, {25, "HD"}, {31, "UHD"}};

static void debug(string ns, string cmsg = "", string optk = "", string optv = "", string indt = " ")
{
	if (DEBUG)
	{
		cout << ns;
		if (cmsg.size()) cout << indt << cmsg;
		if (optk.size()) cout << indt << optk << ":";
		if (optv.size()) cout << " " << optv;
		cout << endl;
	}
}
static void error(string ns, string cmsg = "", string optk = "", string optv = "", string indt = " ")
{
	cout << ns;
	if (cmsg.size()) cout << indt << cmsg;
	if (optk.size()) cout << indt << optk << ":";
	if (optv.size()) cout << " " << optv;
	cout << endl;
}

//C++ 17
inline string lowCase(string str)
{
	transform(str.begin(), str.end(), str.begin(), [](unsigned char c) { return tolower(c); });
	return str;
}
//C++ 17
inline string upCase(string str)
{
	transform(str.begin(), str.end(), str.begin(), [](unsigned char c) { return toupper(c); });
	return str;
}
#endif /* commons_h */
