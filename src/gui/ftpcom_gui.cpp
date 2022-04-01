/*!
 * e2-sat-editor/src/gui/ftpcom_gui.cpp
 * 
 * @link https://github.com/ctlcltd/e2-sat-editor
 * @copyright e2 SAT Editor Team
 * @author Leonardo Laureti
 * @version 0.1
 * @license MIT License
 * @license GNU GPLv3 License
 */

#include <QMessageBox>
#include <QString>

#include "ftpcom_gui.h"

namespace e2se_gui
{

ftpcom::ftpcom()
{
	debug("gui.ftpcom()");

	this->sets = new QSettings;
	options();
}

void ftpcom::options()
{
	debug("options()");
}

void ftpcom::error(string cmsg, string rmsg)
{
	debug("gui.error()");

	this->::e2se_ftpcom::ftpcom::error(cmsg, rmsg);
	QMessageBox::critical(nullptr, NULL, QString::fromStdString(rmsg));
}

string ftpcom::trs(string str)
{
	return tr(str.data(), "ftpcom").toStdString();
}

string ftpcom::trw(string str, string param)
{
	string trstr = tr(str.data(), "ftpcom").toStdString();
	string trparam = tr(param.data(), "ftpcom").toStdString();
	size_t tsize = trstr.length() + trparam.length();
	char tstr[tsize];
	std::sprintf(tstr, trstr.c_str(), trparam.c_str());
	return string (tstr);
}

}
