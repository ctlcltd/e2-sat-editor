/*!
 * e2-sat-editor/src/gui/ftpcom_gui.cpp
 * 
 * @link https://github.com/ctlcltd/e2-sat-editor
 * @copyright e2 SAT Editor Team
 * @author Leonardo Laureti
 * @version 0.2
 * @license MIT License
 * @license GNU GPLv3 License
 */

#include <cstdio>

#include <QString>
#include <QMessageBox>

#include "ftpcom_gui.h"

namespace e2se_gui
{

ftpcom::ftpcom(e2se::logger::session* log)
{
	this->log = new e2se::logger(log, "gui.ftpcom");
	debug("ftpcom()");

	this->sets = new QSettings;

	int profile_sel = sets->value("profile/selected").toInt();
	sets->beginReadArray("profile");
	sets->setArrayIndex(profile_sel);
	ftpcom::ftp_params params;
	params.host = sets->value("ipAddress").toString().toStdString();
	params.ftport = sets->value("ftpPort").toInt();
	params.htport = sets->value("httpPort").toInt();
	params.user = sets->value("username").toString().toStdString();
	params.pass = sets->value("password").toString().toStdString();
	params.tpath = sets->value("pathTransponders").toString().toStdString();
	params.spath = sets->value("pathServices").toString().toStdString();
	params.bpath = sets->value("pathBouquets").toString().toStdString();
	params.ifreload = sets->value("customWebifReloadUrl").toString().toStdString();
	params.tnreload = sets->value("customTelnetReloadCmd").toString().toStdString();
	sets->endArray();

	this->setup(params);
}

void ftpcom::error(string tmsg, string rmsg)
{
	debug("error()");

	this->::e2se_ftpcom::ftpcom::error(tmsg, rmsg);
	QMessageBox::critical(nullptr, QString::fromStdString(tmsg), QString::fromStdString(rmsg));
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
