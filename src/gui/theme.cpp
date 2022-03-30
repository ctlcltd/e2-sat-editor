/*!
 * e2-sat-editor/src/gui/theme.cpp
 *
 * @link https://github.com/ctlcltd/e2-sat-editor
 * @copyright e2 SAT Editor Team
 * @author Leonardo Laureti
 * @version 0.1
 * @license MIT License
 * @license GNU GPLv3 License
 */

#include <QRegularExpression>
#include <QSysInfo>

#include "theme.h"

namespace e2se_gui
{

theme::theme()
{
	QString colors = "light";
	QString icons = ":/icons/";

	//experimental theme color detect
	// windows
	if (QSysInfo::productType().contains("windows"))
	{
		QSettings settings = QSettings(R"(HKEY_CURRENT_USER\Software\Microsoft\Windows\CurrentVersion\Themes\Personalize)",  QSettings::NativeFormat);
		colors = settings.value("AppsUseLightTheme", 1).toInt() == 0 ? "dark" : "light";
	}
	// macos Qt 6
	else if (QSysInfo::productType().contains(QRegularExpression("macos|osx")))
	{
		colors = QSettings().value("AppleInterfaceStyle").toString() == "Dark" ? "dark" : "light";
	}
	// icon theme based
	else if (! QIcon::themeName().isEmpty())
	{
		colors = QIcon::themeName().contains("dark") ? "dark" : "light";
	}

	QSettings().setValue("application/icons", icons.append(colors));
}

QIcon theme::icon(QString icon)
{
	return QIcon(theme::getIcon(icon));
}

QString theme::getIcon(QString icon)
{
	return QSettings().value("application/icons").toString().append('/' + icon).append(".png");
}

}
