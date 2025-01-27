/*!
 * e2-sat-editor/src/gui/checkUpdate.cpp
 *
 * @link https://github.com/ctlcltd/e2-sat-editor
 * @copyright e2 SAT Editor Team
 * @author Leonardo Laureti
 * @version 1.8.0
 * @license MIT License
 * @license GNU GPLv3 License
 */

#include <string>
#include <sstream>

#include <QtGlobal>
#include <QTimer>
#include <QSettings>
#include <QDateTime>
#include <QUrl>

#include "../e2se_defs.h"

#ifdef E2SE_CHECKUPDATE
#define ALLOW_UPDATE_CHECK

#include <curl/curl.h>
#endif

#include "checkUpdate.h"
#include "theme.h"

using std::string, std::stringstream;

using namespace e2se;

namespace e2se_gui
{

checkUpdate::checkUpdate(QWidget* cwid)
{
	this->log = new logger("gui", "checkUpdate");

	this->cwid = cwid;

	checkUpdate::VERBOSE = e2se::logger::OBJECT->debug;

#ifdef E2SE_MEDIUM
#if E2SE_MEDIUM == 6
	this->medium = MEDIUM::aur;
#elif E2SE_MEDIUM == 3
	this->medium = MEDIUM::flathub;
#elif E2SE_MEDIUM == 1
	this->medium = MEDIUM::github;
#elif E2SE_MEDIUM == 2
	this->medium = MEDIUM::snapstore;
#endif
#endif
}

checkUpdate::~checkUpdate()
{
	debug("~checkUpdate");

	delete this->log;
}

void checkUpdate::check()
{
	debug("check");

	string relver = "v1.8.0";
	string current_url = "https://github.com/ctlcltd/e2-sat-editor/releases/tag/";
	current_url.append(relver);

	this->state.checked = true;
	this->state.current_url = current_url;

	fetch();

	if (this->state.connected || this->state.fetched)
		QSettings().setValue("application/lastCheckUpdate", QDateTime::currentDateTime());

	if (this->state.fetched)
	{
		QString latest_url = QString::fromStdString(this->state.latest_url);
		QUrl url = QUrl (latest_url);

		this->state.fetched = url.isValid();

		if (url.isValid())
		{
			QString pver = QString (latest_url);
			pver.remove(0, latest_url.indexOf("/tag/"));
			pver.remove(0, 6);
			this->state.version = pver.toStdString();
		}
	}

	if (! this->autocheck)
		show();
}

void checkUpdate::autoCheck()
{
	QString periodval = QSettings().value("application/periodCheckUpdate", "weekly").toString();
	QString lastval = QSettings().value("application/lastCheckUpdate", 0).toString();

	debug("autoCheck", "period", periodval.toStdString());
	debug("autoCheck", "last", lastval.toStdString());

	this->autocheck = true;

	QDateTime ts = QSettings().value("application/lastCheckUpdate", 0).toDateTime();
	QDateTime tn;

	if (ts.isValid())
	{
		if (periodval == "weekly")
			tn = ts.addDays(7);
		else if (periodval == "montly")
			tn = ts.addMonths(1);
		else if (periodval == "daily")
			tn = ts.addDays(1);

		if (QDateTime::currentDateTime() >= tn)
		{
			debug("autoCheck", "time", "greater-equal");
		}
		else
		{
			debug("autoCheck", "time", "less-not-equal");

			return;
		}
	}
	else
	{
		debug("autoCheck", "time", "not-valid");
	}

	check();

	if (this->state.fetched && this->state.current_url != this->state.latest_url)
	{
		this->state.dialog = DIAL::dial_haveupdate;

		QString version;
		QUrl url;

		if (! this->state.version.empty())
			version = QString::fromStdString(this->state.version);
		if (! this->state.latest_url.empty())
			url = QUrl (QString::fromStdString(this->state.latest_url));

		QMetaObject::invokeMethod(this->cwid, [=]() { this->prompt(this->state.dialog, this->medium, version, url); });
	}
}

void checkUpdate::show()
{
	debug("show");

	if (! this->state.checked)
	{
		error("show", "checked", 0);

		return;
	}

	this->state.dialog = DIAL::dial_connerror;

	if (this->state.fetched)
	{
		if (this->state.current_url == this->state.latest_url)
			this->state.dialog = DIAL::dial_noupdate;
		else
			this->state.dialog = DIAL::dial_haveupdate;
	}
	else if (this->state.connected)
	{
		this->state.dialog = DIAL::dial_fetcherror;
	}

	QString version;
	QUrl url;

	if (! this->state.version.empty())
		version = QString::fromStdString(this->state.version);
	if (! this->state.latest_url.empty())
		url = QUrl (QString::fromStdString(this->state.latest_url));

	// note: this->state broken strings with QMetaObject::invokeMethod
	QMetaObject::invokeMethod(this->cwid, [=]() { this->prompt(this->state.dialog, this->medium, version, url); });
}

void checkUpdate::prompt(DIAL dialog, MEDIUM medium, QString version, QUrl url)
{
	// note: SEGFAULT with QMetaObject::invokeMethod
	// debug("prompt");

	QMessageBox::Icon icon = QMessageBox::NoIcon;
	QString title, message;

	if (dialog == dial_noupdate)
	{
		title = tr("Software is Up-To-Date", "message");
		message = tr("e2 SAT Editor %1 is the latest version available.", "message").arg(version);
	}
	else if (dialog == dial_haveupdate)
	{
		title = tr("New Update Available", "message");

		if (medium == MEDIUM::unknown)
		{
			message = tr("e2 SAT Editor %1 is available to download.", "message").arg(version);
			message = message.replace("<", "&lt;").replace(">", "&gt;");
		}
		else if (medium == MEDIUM::snapstore)
		{
			message = tr("e2 SAT Editor %1 is available to update.", "message").arg(version);
			message.append(QString("\n%1").arg(tr("Please run Snap update to get the latest version.", "message")));
			message = message.replace("<", "&lt;").replace(">", "&gt;");
		}
		else if (medium == MEDIUM::flathub)
		{
			message = tr("e2 SAT Editor %1 is available to update.", "message").arg(version);
			message.append(QString("\n%1").arg(tr("Please run Flatpak update to get the latest version.", "message")));
			message = message.replace("<", "&lt;").replace(">", "&gt;");
		}
		else if (medium == MEDIUM::aur)
		{
			message = tr("e2 SAT Editor %1 is available to update.", "message").arg(version);
			message.append(QString("\n%1").arg(tr("Please fetch the AUR repository and build the latest version.", "message")));
			message = message.replace("<", "&lt;").replace(">", "&gt;");
		}
		else
		{
			message = tr("e2 SAT Editor %1 is available to download.", "message").arg(version);
			message = message.replace("<", "&lt;").replace(">", "&gt;");

			if (url.isValid())
			{
				QString purl = QString("<p style=\"margin-top: 5px\"><a href=\"%1\">%2</a></p>").arg(url.toString()).arg(url.toString());
				message.append(purl);
			}
		}
	}
	else if (dialog == dial_connerror)
	{
		title = tr("Connection Error", "message");
		message = tr("There was an error during connection.\nPlease check network settings and try again.", "message");
		icon = QMessageBox::Critical;
	}
	else if (dialog == dial_fetcherror)
	{
		title = tr("Service Not Available", "message");
		message = tr("Service seems to be unavailable right now.\nPlease wait few minutes and try again.", "message");
		icon = QMessageBox::Critical;
	}

	title = title.toHtmlEscaped();

	if (dialog != dial_haveupdate)
		message = message.replace("<", "&lt;").replace(">", "&gt;");

	// note: rand SEGFAULT with cwid in thread
	QMessageBox msg = QMessageBox(nullptr);

	msg.setIcon(icon);
	msg.setTextFormat(Qt::PlainText);
	msg.setText(title);
	msg.setInformativeText(message);
	msg.setStandardButtons(QMessageBox::Ok);
	msg.setDefaultButton(QMessageBox::Ok);

	msg.exec();
}

void checkUpdate::destroy()
{
	debug("destroy");

	delete this;
}

#ifdef ALLOW_UPDATE_CHECK
void checkUpdate::fetch()
{
	debug("fetch");

	curl_global_init(CURL_GLOBAL_DEFAULT);

	CURL* cph = curl_easy_init();

	if (! cph)
		return;

	CURLU* rph = curl_url();
	curl_url_set(rph, CURLUPART_SCHEME, "https", 0);
	curl_url_set(rph, CURLUPART_URL, "https://github.com/ctlcltd/e2-sat-editor/releases/latest", 0);

#if LIBCURL_VERSION_NUM < 0x075500
	curl_easy_setopt(cph, CURLOPT_PROTOCOLS, CURLPROTO_HTTPS);
#else
	curl_easy_setopt(cph, CURLOPT_PROTOCOLS_STR, "https");
#endif

	struct curl_slist* list = NULL;
	list = curl_slist_append(list, "DNT: 1");

	if (this->medium == MEDIUM::github || this->medium == MEDIUM::unknown)
		curl_easy_setopt(cph, CURLOPT_CAINFO, cabundle_path().c_str());

	curl_easy_setopt(cph, CURLOPT_CONNECT_ONLY, true);
	curl_easy_setopt(cph, CURLOPT_HTTPGET, true);
	curl_easy_setopt(cph, CURLOPT_FOLLOWLOCATION, false);
	curl_easy_setopt(cph, CURLOPT_HTTPHEADER, list);
	curl_easy_setopt(cph, CURLOPT_CURLU, rph);
	curl_easy_setopt(cph, CURLOPT_CONNECTTIMEOUT, 0);
	if (checkUpdate::VERBOSE)
		curl_easy_setopt(cph, CURLOPT_VERBOSE, true);

	CURLcode res = curl_easy_perform(cph);

	curl_slist_free_all(list);

	if (res != CURLE_OK)
	{
		error("fetch", "Error", e2se::logger::msg("%s (connect)", curl_easy_strerror(res)));

		curl_url_cleanup(rph);
		curl_easy_cleanup(cph);
		curl_global_cleanup();

		return;
	}
	else
	{
		this->state.connected = true;

		curl_easy_cleanup(cph);

		CURL* cph = curl_easy_init();

		if (! cph)
			return;

		stringstream data;

#if LIBCURL_VERSION_NUM < 0x075500
		curl_easy_setopt(cph, CURLOPT_PROTOCOLS, CURLPROTO_HTTPS);
#else
		curl_easy_setopt(cph, CURLOPT_PROTOCOLS_STR, "https");
#endif

		struct curl_slist* list = NULL;
		list = curl_slist_append(list, "DNT: 1");

		if (this->medium == MEDIUM::github || this->medium == MEDIUM::unknown)
			curl_easy_setopt(cph, CURLOPT_CAINFO, cabundle_path().c_str());

		curl_easy_setopt(cph, CURLOPT_HTTPGET, true);
		curl_easy_setopt(cph, CURLOPT_FOLLOWLOCATION, false);
		curl_easy_setopt(cph, CURLOPT_HTTPHEADER, list);
		curl_easy_setopt(cph, CURLOPT_CURLU, rph);
		curl_easy_setopt(cph, CURLOPT_HEADERFUNCTION, data_write_func);
		curl_easy_setopt(cph, CURLOPT_HEADERDATA, &data);
		curl_easy_setopt(cph, CURLOPT_WRITEFUNCTION, data_discard_func);
		curl_easy_setopt(cph, CURLOPT_CONNECTTIMEOUT, checkUpdate::HTTP_CONNECT_TIMEOUT);
		curl_easy_setopt(cph, CURLOPT_TIMEOUT, checkUpdate::HTTP_TIMEOUT);
		if (checkUpdate::VERBOSE)
			curl_easy_setopt(cph, CURLOPT_VERBOSE, true);

		CURLcode res = curl_easy_perform(cph);

		curl_slist_free_all(list);

		if (res != CURLE_OK)
		{
			error("fetch", "Error", e2se::logger::msg("%s (check)", curl_easy_strerror(res)));

			curl_url_cleanup(rph);
			curl_easy_cleanup(cph);
			curl_global_cleanup();

			return;
		}

		string latest_url;
		string line;
		bool httpchecked = false;
		bool http2 = false;

		while (std::getline(data, line))
		{
			if (httpchecked)
			{
				if (line.find(http2 ? "location:" : "Location:") != string::npos)
				{
					latest_url = line.substr(10);
					latest_url = latest_url.substr(0, latest_url.size() - 1);

					break;
				}
			}
			else
			{
				if (line.find("HTTP/2") != string::npos)
				{
					httpchecked = true;
					http2 = true;
				}
				else if (line.find("HTTP/1") != string::npos)
				{
					httpchecked = true;
					http2 = false;
				}
			}
		}

		if (! latest_url.empty() && latest_url.find("https://github.com/ctlcltd/e2-sat-editor/releases/tag/") != string::npos)
		{
			this->state.fetched = true;
			this->state.latest_url = latest_url;
		}

		debug("fetch", "current url", this->state.current_url);
		debug("fetch", "latest url", latest_url);
	}

	curl_global_cleanup();
}

size_t checkUpdate::data_write_func(void* csi, size_t size, size_t nmemb, void* pso)
{
	size_t relsize = size * nmemb;
	string data ((const char*) csi, relsize);
	*((stringstream*) pso) << data;
	return relsize;
}

size_t checkUpdate::data_discard_func(void* csi, size_t size, size_t nmemb, void* pso)
{
	(void) csi;
	(void) pso;
	return size * nmemb;
}

string checkUpdate::cabundle_path()
{
#if E2SE_BUILD == E2SE_TARGET_DEBUG
	QString path = QApplication::applicationDirPath();
	if (path.contains("/src"))
		path.truncate(path.indexOf("/src"));
	path.append("/workflow/curl-cacert.pem");
	return path.toStdString();
#else
	QString path = QApplication::applicationDirPath();
#ifndef Q_OS_MAC
	if (path.contains("/bin"))
	{
		path.truncate(path.indexOf("/bin"));
		path.append("/share/e2-sat-editor/curl-cacert.pem");
	}
	else
	{
		path.append("/ca-certificates/curl-cacert.pem");
	}
	return path.toStdString();
#else
	if (path.contains("/Contents/MacOS"))
	{
		path.truncate(path.indexOf("/Contents/MacOS"));
		path.append("/Contents/Resources/curl-cacert.pem");
	}
	return path.toStdString();
#endif
#endif
}
#endif

}
