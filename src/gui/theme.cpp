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
#include <QSettings>
#include <QPalette>
#include <QFont>

#include "theme.h"

namespace e2se_gui
{

theme::theme()
{
	QString style = theme::preference();

	if (style == "light")
		style::light();
	else if (style == "dark")
		style::dark();
	else
		style::system();

	QSettings().setValue("application/icons", theme::absLuma());
}

QString theme::preference()
{
	return QSettings().value("preference/theme").toString();
}

bool theme::absLuma()
{
	QColor bgcolor = QGuiApplication::palette().color(QPalette::Window).toHsl();
	return bgcolor.lightness() > 128 ? false : true;
}

bool theme::isDefault()
{
	return theme::preference().isEmpty();
}

QIcon theme::icon(QString icon)
{
	return QIcon(":/icons/" + QString (QSettings().value("application/icons").toBool() ? "dark" : "light") + "/" + icon + ".png");
}

QIcon theme::spacer(int width)
{
	return QIcon(":/icons/" + QString().setNum(width) + "x1.png");
}

QString theme::fontFamily()
{
	return QFont().defaultFamily();
}

//TODO FIX behaviour Qt5 [linux]
int theme::fontSize()
{
	return QFont().pointSize();
}

int theme::calcFontSize(int size)
{
	return (theme::fontSize() + size);
}


//TODO FIX behaviour Qt5 [linux]
void style::system()
{
#ifndef Q_OS_MAC
	QPalette palette = QApplication::palette();

	// light
	if (theme::absLuma())
	{
		palette.setColor(QPalette::Mid, QPalette::HighlightedText);
	}
	// dark
	else
	{
		palette.setColor(QPalette::Mid, QPalette::Dark);
	}
	QApplication::setPalette(palette);
#endif
}

void style::light()
{
	QStyle* style = QStyleFactory::create("Fusion");
	QApplication::setStyle(style);
	QPalette palette = style->standardPalette();
	palette.setColor(QPalette::Mid, QColor(249, 249, 249)); // QColor(184, 184, 184);
	palette.setColor(QPalette::HighlightedText, QColor(249, 249, 249));
	palette.setColor(QPalette::HighlightedText, Qt::white); // QColor(249, 249, 249);
	QApplication::setPalette(palette);
}

void style::dark()
{
	QApplication::setStyle("Fusion");
	QPalette palette;
	palette.setColor(QPalette::WindowText, Qt::white);
	palette.setColor(QPalette::Button, QColor(74, 73, 71).darker(175)); // QColor(66, 66, 69)
	palette.setColor(QPalette::Light, QColor(151, 151, 151));
	palette.setColor(QPalette::Midlight, QColor(94, 92, 91));
	palette.setColor(QPalette::Dark, QColor(61, 61, 61)); // QColor(48, 47, 46)
	palette.setColor(QPalette::Mid, QColor(94, 92, 91).darker(400)); // QColor(74, 73, 71)
	palette.setColor(QPalette::Text, Qt::white);
	palette.setColor(QPalette::BrightText, Qt::black);
	palette.setColor(QPalette::ButtonText, Qt::white);
	palette.setColor(QPalette::Base, QColor(48, 47, 46).darker(250)); // QColor(61, 61, 61)
	palette.setColor(QPalette::Window, QColor(34, 32, 32));
	palette.setColor(QPalette::Shadow, QColor(231, 228, 224));
	palette.setColor(QPalette::Highlight, QColor(18, 96, 138));
	palette.setColor(QPalette::HighlightedText, QColor(249, 249, 249));
	palette.setColor(QPalette::Link, QColor(9, 134, 211));
	palette.setColor(QPalette::LinkVisited, QColor(167, 11, 6));
	palette.setColor(QPalette::AlternateBase, QColor(92, 91, 90));
	palette.setColor(QPalette::NoRole, Qt::black);
	palette.setColor(QPalette::ToolTipBase, QColor(63, 63, 54));
	palette.setColor(QPalette::ToolTipText, Qt::white);
	palette.setColor(QPalette::PlaceholderText, Qt::white);
	palette.setColor(QPalette::Disabled, QPalette::WindowText, QColor(128, 128, 128));
	palette.setColor(QPalette::Disabled, QPalette::Text, QColor(128, 128, 128));
	palette.setColor(QPalette::Disabled, QPalette::ButtonText, QColor(128, 128, 128));
	palette.setColor(QPalette::Disabled, QPalette::HighlightedText, QColor(128, 128, 128));
	QApplication::setPalette(palette);
}

}
