/*!
 * e2-sat-editor/src/gui/theme.cpp
 *
 * @link https://github.com/ctlcltd/e2-sat-editor
 * @copyright e2 SAT Editor Team
 * @author Leonardo Laureti
 * @version 0.7
 * @license MIT License
 * @license GNU GPLv3 License
 */

#include <string>

#include <QApplication>
#include <QGuiApplication>
#include <QSettings>
#include <QStyle>
#include <QStyleFactory>
#include <QSettings>
#include <QPalette>
#include <QBitmap>
#include <QFont>
#include <QAction>
#include <QPushButton>

#include "theme.h"

using std::string, std::pair, std::to_string;

namespace e2se_gui
{

theme::theme()
{
}

void theme::initStyle()
{
	QString style = theme::preference();

	if (style == "light")
		styleLight();
	else if (style == "dark")
		styleDark();
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

bool theme::isLightMode()
{
	return ! theme::absLuma();
}

bool theme::isDarkMode()
{
	return theme::absLuma();
}

QIcon theme::icon(QString icon, ICON_STYLE style)
{
	QPixmap ico = QPixmap(":/icons/" + icon + ".png");

	QBitmap mask = ico.createMaskFromColor(Qt::black, Qt::MaskOutColor);
	QColor color = theme::absLuma() ? Qt::white : Qt::black;

	if (style == ICON_STYLE::icon_default)
	{
#ifndef Q_OS_MAC
		color = theme::absLuma() ? QPalette().color(QPalette::Text).darker(130) : QPalette().color(QPalette::Mid).darker();
#else
		color.setAlphaF(theme::absLuma() ? 0.73 : 0.67);
#endif
	}

	ico.fill(color);
	ico.setMask(mask);

	return ico;
}

//TODO strange behaviour [Wine]
QString theme::fontFamily()
{
	return QFont().defaultFamily();
}

int theme::fontSize()
{
	return QFont().pointSize();
}

int theme::calcFontSize(int size)
{
	return (theme::fontSize() + size);
}

void theme::setWaitCursor()
{
#ifndef Q_OS_MAC
	QGuiApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
#endif
}

void theme::unsetWaitCursor()
{
#ifndef Q_OS_MAC
	QGuiApplication::restoreOverrideCursor();
#endif
}

void theme::dynamicStyleSheet(QWidget* widget, QString stylesheet)
{
	widget->setStyleSheet(widget->styleSheet().append(stylesheet));
}

void theme::dynamicStyleSheet(QWidget* widget, QString stylesheet, STYLE style)
{
	string key = "dynqss_" + to_string(style);
	if (widget->property(key.data()).isNull())
		widget->setProperty(key.data(), stylesheet);
	else
		widget->setProperty(key.data(), widget->property(key.data()).toString().append(stylesheet));

	if (theme::absLuma() == style)
		widget->setStyleSheet(widget->styleSheet().append(stylesheet));

	styled.emplace_back(widget);
}

pair<theme*, QString> theme::dynamicIcon(QString icon)
{
	return pair (this, icon);
}

QIcon theme::dynamicIcon(QString icon, QObject* object, ICON_STYLE style)
{
	object->setProperty("dynico_name", icon);
	object->setProperty("dynico_style", style);
	imaged.emplace_back(object);

	return this->icon(icon);
}

void theme::changed()
{
	int style = theme::absLuma();

	for (auto & widget : styled)
	{
		QString stylesheet = widget->property(string ("dynqss_" + to_string(style)).data()).toString();
		//TODO improve
		widget->setStyleSheet(widget->styleSheet().append(stylesheet));
	}
	for (auto & object : imaged)
	{
		QString icon = object->property("dynico_name").toString();
		ICON_STYLE style = static_cast<ICON_STYLE>(object->property("dynico_style").toInt());

		if (QAction* action = qobject_cast<QAction*>(object))
			action->setIcon(this->icon(icon, style));
		else if (QPushButton* button = qobject_cast<QPushButton*>(object))
			button->setIcon(this->icon(icon, style));
	}
}

// custom Fusion light
void theme::styleLight()
{
	QStyle* style = QStyleFactory::create("Fusion");
	QApplication::setStyle(style);
	QPalette palette = style->standardPalette();

	palette.setColor(QPalette::WindowText, Qt::black);
	palette.setColor(QPalette::Button, QColor(239, 239, 239));
	palette.setColor(QPalette::Light, QColor(239, 239, 239).lighter(150));
	palette.setColor(QPalette::Midlight, QColor(239, 239, 239).darker(130).lighter(110));
	palette.setColor(QPalette::Dark, QColor(239, 239, 239).darker(150));
	palette.setColor(QPalette::Mid, QColor(249, 249, 249)); // QColor(239, 239, 239).darker(150)
	palette.setColor(QPalette::Text, Qt::black);
	palette.setColor(QPalette::BrightText, Qt::white);
	palette.setColor(QPalette::ButtonText, Qt::black);
	palette.setColor(QPalette::Base, Qt::white);
	palette.setColor(QPalette::Window, QColor(239, 239, 239));
	palette.setColor(QPalette::Shadow, QColor(239, 239, 239).darker(150).darker(135));
	palette.setColor(QPalette::Active, QPalette::Highlight, QColor(48, 140, 198));
	palette.setColor(QPalette::Inactive, QPalette::Highlight, QColor(48, 140, 198));
	palette.setColor(QPalette::HighlightedText, Qt::white);
	palette.setColor(QPalette::AlternateBase, QColor(239, 239, 239));
	palette.setColor(QPalette::NoRole, Qt::black);
	palette.setColor(QPalette::ToolTipBase, Qt::white);
	palette.setColor(QPalette::ToolTipText, Qt::black);

	QColor placeholder = Qt::white;
	placeholder.setAlpha(128);
	palette.setColor(QPalette::PlaceholderText, placeholder);

	palette.setColor(QPalette::Disabled, QPalette::WindowText, QColor(190, 190, 190));
	palette.setBrush(QPalette::Disabled, QPalette::Dark, QColor(209, 209, 209).darker(110));
	palette.setColor(QPalette::Disabled, QPalette::Text, QColor(190, 190, 190));
	palette.setColor(QPalette::Disabled, QPalette::ButtonText, QColor(190, 190, 190));
	palette.setBrush(QPalette::Disabled, QPalette::Base, QColor(239, 239, 239));
	palette.setBrush(QPalette::Disabled, QPalette::Shadow, QColor(239, 239, 239).darker(150).darker(135).lighter(150));
	palette.setBrush(QPalette::Disabled, QPalette::Highlight, QColor(145, 145, 145));
	palette.setColor(QPalette::Disabled, QPalette::HighlightedText, QColor(190, 190, 190));

	QApplication::setPalette(palette);
}

// custom Fusion darker
void theme::styleDark()
{
	QStyle* style = QStyleFactory::create("Fusion");
	QApplication::setStyle(style);
	QPalette palette = style->standardPalette();

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
	palette.setColor(QPalette::Active, QPalette::Highlight, QColor(18, 96, 138));
	palette.setColor(QPalette::Inactive, QPalette::Highlight, QColor(18, 96, 138));
	palette.setColor(QPalette::HighlightedText, QColor(249, 249, 249));
	palette.setColor(QPalette::AlternateBase, QColor(34, 32, 32));
	palette.setColor(QPalette::NoRole, Qt::black);
	palette.setColor(QPalette::ToolTipBase, QColor(63, 63, 54));
	palette.setColor(QPalette::ToolTipText, Qt::white);

	QColor placeholder = Qt::white;
	placeholder.setAlpha(128);
	palette.setColor(QPalette::PlaceholderText, placeholder);

	palette.setColor(QPalette::Disabled, QPalette::WindowText, QColor(128, 128, 128));
	palette.setBrush(QPalette::Disabled, QPalette::Dark, QColor(209, 209, 209).darker(110));
	palette.setColor(QPalette::Disabled, QPalette::Text, QColor(128, 128, 128));
	palette.setColor(QPalette::Disabled, QPalette::ButtonText, QColor(128, 128, 128));
	palette.setBrush(QPalette::Disabled, QPalette::Base, QColor(34, 32, 32));
	palette.setBrush(QPalette::Disabled, QPalette::Shadow, QColor(231, 228, 224).lighter(150));
	palette.setBrush(QPalette::Disabled, QPalette::Highlight, QColor(145, 145, 145));
	palette.setColor(QPalette::Disabled, QPalette::HighlightedText, QColor(128, 128, 128));

	QApplication::setPalette(palette);
}

}
