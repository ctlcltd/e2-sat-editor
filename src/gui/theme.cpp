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

#include <QApplication>
#include <QGuiApplication>
#include <QStyle>
#include <QStyleFactory>
#include <QPalette>
#include <QFont>

#include "theme.h"

namespace e2se_gui
{

theme::theme()
{
	QString icons = ":/icons/";
	QString style = QSettings().value("preference/theme").toString();

	if (style == "light")
		style::light();
	else if (style == "dark")
		style::dark();

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

QString theme::getDefaultFontFamily()
{
	return QFont().defaultFamily();
}

int theme::getDefaultFontSize()
{
	return QFont().pointSize();
}


void style::light()
{
	QStyle* style = QStyleFactory::create("Fusion");
	QApplication::setStyle(style);
	QApplication::setPalette(style->standardPalette());
}

void style::dark()
{
	QApplication::setStyle("Fusion");
	QPalette palette;
	palette.setColor(QPalette::Window, QColor(53, 53, 53));
	palette.setColor(QPalette::WindowText, Qt::white);
	palette.setColor(QPalette::Base, QColor(42, 42, 42));
	palette.setColor(QPalette::AlternateBase, QColor(66, 66, 69));
	palette.setColor(QPalette::ToolTipBase, Qt::white);
	palette.setColor(QPalette::ToolTipText, QColor(53, 53, 53));
	palette.setColor(QPalette::Text, Qt::white);
	palette.setColor(QPalette::Button, QColor(53, 53, 53));
	palette.setColor(QPalette::ButtonText, Qt::white);
	palette.setColor(QPalette::BrightText, QColor(167, 11, 6));
	palette.setColor(QPalette::Link, QColor(9, 134, 211));
	palette.setColor(QPalette::Highlight, QColor(18, 96, 138));
	palette.setColor(QPalette::HighlightedText, Qt::white);
	palette.setColor(QPalette::Disabled, QPalette::WindowText, QColor(128, 128, 128));
	palette.setColor(QPalette::Disabled, QPalette::Text, QColor(128, 128, 128));
	palette.setColor(QPalette::Disabled, QPalette::ButtonText, QColor(128, 128, 128));
	palette.setColor(QPalette::Disabled, QPalette::Highlight, QColor(80, 80, 80));
	palette.setColor(QPalette::Disabled, QPalette::HighlightedText, QColor(128, 128, 128));
	QApplication::setPalette(palette);
}

}
