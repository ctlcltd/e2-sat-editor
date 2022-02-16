/*!
 * e2-sat-editor/src/commons.h
 *
 * @author Leonardo Laureti
 * @version 0.1
 * @license MIT License
 */
#include <iostream>
#include <unordered_map>
#include <tuple>

using namespace std;

#ifndef commons_h
#define commons_h
const tuple SAT_POL = tuple("H", "V", "L", "R");
const tuple SAT_FEC = tuple("", "Auto", "1/2", "2/3", "3/4", "5/6", "7/8", "3/5", "4/5", "8/9", "9/10");
const tuple SAT_INV = tuple("Auto", "On", "Off");
const tuple SAT_SYS = tuple("DVB-S", "DVB-S2");
const tuple SAT_MOD = tuple("Auto", "QPSK", "QAM16", "8PSK");
const tuple SAT_ROL = tuple("Auto", "QPSK", "QAM16", "8PSK");
const tuple SAT_PIL = tuple("Auto", "Off", "On");

const unordered_map<int, string> STYPES = {{0, "Data"}, {1, "TV"}, {2, "Radio"}, {10, "Radio"}, {12, "TV"}, {17, "UHD"}, {22, "H.264"}, {25, "HD"}, {31, "UHD"}};
#endif /* commons_h */
