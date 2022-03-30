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

#include <QGuiApplication>
#include <QPalette>

#include "theme.h"

namespace e2se_gui
{

theme::theme()
{
	QString icons = ":/icons/";

	//experimental theme color detect
	QColor bgcolor = QGuiApplication::palette().color(QPalette::Window).toHsl();
	QString colors = bgcolor.lightness() > 128 ? "light" : "dark";

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
