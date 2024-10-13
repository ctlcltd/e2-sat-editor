/*!
 * e2-sat-editor/src/gui/ftpcom_gui.cpp
 * 
 * @link https://github.com/ctlcltd/e2-sat-editor
 * @copyright e2 SAT Editor Team
 * @author Leonardo Laureti
 * @version 1.7.0
 * @license MIT License
 * @license GNU GPLv3 License
 */

#include <cstdio>

// note: std::runtime_error behaviour
#include <stdexcept>

#include <QSettings>
#include <QString>
#include <QByteArray>
#include <QMessageBox>

#include "ftpcom_gui.h"

using std::string;

namespace e2se_gui
{

ftpcom::ftpcom()
{
	this->log = new e2se::logger("gui", "ftpcom");

	setup();
}

ftpcom::~ftpcom()
{
	debug("~ftpcom");

	delete this->log;
}

void ftpcom::setup()
{
	debug("setup");

	QSettings settings;

	settings.beginReadArray("ftpcom");
	ftpcom::VERBOSE = settings.value("debug", false).toBool();
	ftpcom::FTP_CONNECT_TIMEOUT = settings.value("ftpConnectTimeout", 10).toInt();
	ftpcom::HTTP_TIMEOUT = settings.value("httpTimeout", 15).toInt();
	ftpcom::TELNET_TIMEOUT = settings.value("telnetTimeout", 15).toInt();
	ftpcom::MAX_RESUME_ATTEMPTS = settings.value("maxResumeAttempts", 5).toInt();
	ftpcom::FILENAME_CHECK = settings.value("filenameCheck", true).toBool();
	ftpcom::FIX_CRLF = settings.value("fixCrlf", false).toBool();
	settings.endArray();

	if (! settings.value("profile/selected").isValid() || settings.value("profile/selected").isNull())
	{
		return error("setup", "FTP Error", "Please select a connection profile then connect.");
	}

	int profile_sel = settings.value("profile/selected", -1).toInt();
	int profile_i = -1;

	int idx = 0;
	int size = settings.beginReadArray("profile");
	for (int i = 0; i < size; i++)
	{
		settings.setArrayIndex(i);
		idx = settings.group().section('/', 1).toInt();
		if (profile_sel == idx)
		{
			profile_i = i;
			break;
		}
	}
	settings.endArray();

	if (profile_i == -1)
	{
		return error("setup", "FTP Error", "Please select a connection profile then connect.");
	}

	if (! checkProfileIntegrity())
	{
		return error("setup", "FTP Error", "Check connection profile and settings integrity.\n\nPreference values are not valid.");
	}

	settings.beginReadArray("profile");
	settings.setArrayIndex(profile_i);

	ftpcom::ftp_params params;
	params.host = settings.value("ipAddress").toString().toStdString();
	params.ftport = settings.value("ftpPort", 21).toInt();
	params.htport = settings.value("httpPort", 80).toInt();
	params.tnport = settings.value("telnetPort", 23).toInt();
	params.user = settings.value("username").toString().toStdString();

	QString pwrd;
	QByteArray ba (settings.value("password").toString().toUtf8());
	pwrd = QByteArray::fromBase64(ba, QByteArray::AbortOnBase64DecodingErrors);
	params.pwrd = pwrd.toStdString();

	params.tpath = settings.value("pathTransponders").toString().toStdString();
	params.spath = settings.value("pathServices").toString().toStdString();
	params.bpath = settings.value("pathBouquets").toString().toStdString();
	params.ifreload = settings.value("customWebifReloadUrl").toString().toStdString();
	params.tnreload = settings.value("customTelnetReloadCmd").toString().toStdString();
	settings.endArray();

	setParameters(params);
}

bool ftpcom::checkProfileIntegrity(bool strict)
{
	debug("checkProfileIntegrity");

	QSettings settings;

	if (! settings.value("profile/selected").isValid() || settings.value("profile/selected").isNull())
		return false;

	int profile_sel = settings.value("profile/selected", -1).toInt();
	int profile_i = -1;

	int idx = 0;
	int size = settings.beginReadArray("profile");
	for (int i = 0; i < size; i++)
	{
		settings.setArrayIndex(i);
		idx = settings.group().section('/', 1).toInt();
		if (profile_sel == idx)
		{
			profile_i = i;
			break;
		}
	}

	if (profile_i == -1)
		return false;

	settings.setArrayIndex(profile_i);

	idx = settings.group().section('/', 1).toInt();

	if (idx != profile_sel)
		return false;

	if (strict)
	{
		// version 1.4

		QStringList keys = {
			"customTelnetReloadCmd",
			"customWebifReloadUrl",
			"ftpActive",
			"ftpPort",
			"httpPort",
			"ipAddress",
			"password",
			"pathBouquets",
			"pathPicons",
			"pathServices",
			"pathTransponders",
			"profileName",
			"telnetPort",
			"username"
		};

		if (keys != settings.allKeys())
			return false;
	}
	else
	{
		if (! settings.contains("profileName"))
			return false;

		if (settings.allKeys().size() < 6)
			return false;
	}

	for (auto & key : settings.allKeys())
	{
		if (! settings.value(key).isValid())
			return false;
	}

	return true;
}

string ftpcom::msg(string str, string param)
{
	string trstr = tr(str.data()).toStdString();
	string trparam = tr(param.data()).toStdString();
	size_t tsize = trstr.length() + trparam.length();
	char tstr[tsize];
	std::snprintf(tstr, tsize, trstr.c_str(), trparam.c_str());

	return string (tstr);
}

string ftpcom::msg(string str)
{
	return tr(str.data()).toStdString();
}

void ftpcom::error(string fn, string optk, string optv)
{
	this->::e2se_ftpcom::ftpcom::error(fn, tr(optk.data(), "error").toStdString(), tr(optv.data(), "error").toStdString());

	throw std::runtime_error(tr(optk.data(), "error").toStdString() + '\t' + tr(optv.data(), "error").toStdString() + '\t' + fn);
}

}
