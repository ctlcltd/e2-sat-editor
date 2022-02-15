/*!
 * e2-sat-editor/src/commons.h
 *
 * @author Leonardo Laureti
 * @version 0.1
 * @license MIT License
 */

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
#endif /* commons_h */
