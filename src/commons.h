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
const string SAT_POL[4] = {"H", "V", "L", "R"};
const string SAT_FEC[11] = {"", "Auto", "1/2", "2/3", "3/4", "5/6", "7/8", "3/5", "4/5", "8/9", "9/10"};
const string SAT_INV[3] = {"Auto", "On", "Off"};
const string SAT_SYS[2] = {"DVB-S", "DVB-S2"};
const string SAT_MOD[4] = {"Auto", "QPSK", "QAM16", "8PSK"};
const string SAT_ROL[4] = {"Auto", "QPSK", "QAM16", "8PSK"};
const string SAT_PIL[3] = {"Auto", "Off", "On"};

const unordered_map<int, string> STYPES = {{0, "Data"}, {1, "TV"}, {2, "Radio"}, {10, "Radio"}, {12, "TV"}, {17, "UHD"}, {22, "H.264"}, {25, "HD"}, {31, "UHD"}};
#endif /* commons_h */
