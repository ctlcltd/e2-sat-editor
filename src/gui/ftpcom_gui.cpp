/*!
 * e2-sat-editor/src/gui/ftpcom_gui.cpp
 * 
 * @link https://github.com/ctlcltd/e2-sat-editor
 * @copyright e2 SAT Editor Team
 * @author Leonardo Laureti
 * @version 0.3
 * @license MIT License
 * @license GNU GPLv3 License
 */

#include <cstdio>

#include <QSettings>
#include <QString>
#include <QMessageBox>

#include "ftpcom_gui.h"

namespace e2se_gui
{

ftpcom::ftpcom(e2se::logger::session* log)
{
	this->log = new e2se::logger(log, "gui.ftpcom");
	debug("ftpcom()");

	setup();
}

void ftpcom::setup()
{
	debug("setup()");

	QSettings settings;

	int profile_sel = settings.value("profile/selected").toInt();
	settings.beginReadArray("profile");
	settings.setArrayIndex(profile_sel);
	ftpcom::ftp_params params;
	params.host = settings.value("ipAddress").toString().toStdString();
	params.ftport = settings.value("ftpPort").toInt();
	params.htport = settings.value("httpPort").toInt();
	params.user = settings.value("username").toString().toStdString();
	params.pass = settings.value("password").toString().toStdString();
	params.tpath = settings.value("pathTransponders").toString().toStdString();
	params.spath = settings.value("pathServices").toString().toStdString();
	params.bpath = settings.value("pathBouquets").toString().toStdString();
	params.ifreload = settings.value("customWebifReloadUrl").toString().toStdString();
	params.tnreload = settings.value("customTelnetReloadCmd").toString().toStdString();
	settings.endArray();

	setParameters(params);
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

void ftpcom::didChange()
{
	debug("didChange()");

	setup();
}

}
