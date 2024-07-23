/*!
 * e2-sat-editor/src/gui/theme.cpp
 *
 * @link https://github.com/ctlcltd/e2-sat-editor
 * @copyright e2 SAT Editor Team
 * @author Leonardo Laureti
 * @version 1.5.0
 * @license MIT License
 * @license GNU GPLv3 License
 */

#include <string>

#include <QtGlobal>
#include <QApplication>
#include <QGuiApplication>
#include <QOperatingSystemVersion>
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

theme::theme(QApplication* mroot)
{
#ifdef Q_OS_WIN
	stylePseudoWin();
#endif
#ifdef Q_OS_MAC
	mroot->setAttribute(Qt::AA_DontShowIconsInMenus);
#endif
#ifdef Q_OS_WASM
	QFont font = mroot->font();
	font.setPointSize(font.pointSize() - 2);
	mroot->setFont(font);
#endif
}

void theme::checkStyleOverride(int argc, char* argv[])
{
	theme::QSTYLE_OVERRIDDEN = ! qgetenv("QT_STYLE_OVERRIDE").isEmpty();

	for (int i = 0; i < argc; i++)
	{
		std::string arg = argv[i];

		if (arg == "-style" || arg == "-style=")
		{
			theme::QSTYLE_OVERRIDDEN = true;
			break;
		}
	}
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

bool theme::isOverridden()
{
	return theme::QSTYLE_OVERRIDDEN;
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
	QStyle* style = QStyleFactory::create("fusion");
	QApplication::setStyle(style);
	QPalette palette = style->standardPalette();

	palette.setColor(QPalette::WindowText, Qt::black);
	palette.setColor(QPalette::Button, QColor(239, 239, 239));
	palette.setColor(QPalette::Light, QColor(239, 239, 239).lighter(150));
	palette.setColor(QPalette::Midlight, QColor(239, 239, 239).darker(130).lighter(110));
	palette.setColor(QPalette::Dark, QColor(239, 239, 239).darker(150));
	palette.setColor(QPalette::Mid, QColor(249, 249, 249));
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
	QStyle* style = QStyleFactory::create("fusion");
	QApplication::setStyle(style);
	QPalette palette = style->standardPalette();

	palette.setColor(QPalette::WindowText, Qt::white);
	palette.setColor(QPalette::Button, QColor(74, 73, 71).darker(175));
	palette.setColor(QPalette::Light, QColor(151, 151, 151));
	palette.setColor(QPalette::Midlight, QColor(94, 92, 91));
	palette.setColor(QPalette::Dark, QColor(61, 61, 61));
	palette.setColor(QPalette::Mid, QColor(94, 92, 91).darker(400));
	palette.setColor(QPalette::Text, Qt::white);
	palette.setColor(QPalette::BrightText, Qt::black);
	palette.setColor(QPalette::ButtonText, Qt::white);
	palette.setColor(QPalette::Base, QColor(48, 47, 46).darker(250));
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

#ifdef Q_OS_WIN

#if QT_VERSION >= QT_VERSION_CHECK(6, 7, 0)
bool theme::isFluentWin()
{
	return (QOperatingSystemVersion::current() >= QOperatingSystemVersion::Windows11);
}
#else
bool theme::isFluentWin()
{
	return false;
}
#endif

bool theme::isMetroWin()
{
	return (QOperatingSystemVersion::current() >= QOperatingSystemVersion::Windows10 && QOperatingSystemVersion::current().microVersion() < 22e3);
}

void theme::stylePseudoWin()
{
	if (! theme::isOverridden())
	{
		// pseudo fluent win == 11
		if (theme::isFluentWin())
			theme::stylePseudoWinModern();
		// pseudo metro  win == 10
		else if (theme::isMetroWin())
			theme::stylePseudoWinEarly();
	}
}

void theme::stylePseudoWinModern()
{
	QStyle* style = QStyleFactory::create("windows11");
	QApplication::setStyle(style);

	QColor highlightColor;
	QSettings accentColor = QSettings ("HKEY_CURRENT_USER\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Accent", QSettings::NativeFormat);
	if (accentColor.contains("AccentColorMenu")) {
		int color = accentColor.value("AccentColorMenu").toInt();
		int r = color & 0xff;
		int g = (color >> 8) & 0xff;
		int b = (color >> 16) & 0xff;
		highlightColor.setRgb(r, g, b);
	}

	//TEST
	QPalette p1;

	if (! qgetenv("TEST1").isEmpty())
		p1 = style->standardPalette();
	else
		p1 = QGuiApplication::palette();
	//TEST

	// QPalette p1 = QGuiApplication::palette();

	// temp fusion with adjustments
	{
		QStyle* style = QStyleFactory::create("fusion");
		QApplication::setStyle(style);
	}

	QPalette p2 = QGuiApplication::palette();

	// fix light mode
	if (! theme::absLuma())
	{
		if (highlightColor.isValid())
		{
			QColor highlightedTextColor = highlightColor.toHsl().lightness() > 128 ? Qt::black : Qt::white;

			p1.setCurrentColorGroup(QPalette::Active);
			p2.setColor(QPalette::Active, QPalette::Highlight, highlightColor);
			p2.setColor(QPalette::Active, QPalette::HighlightedText, highlightedTextColor);
			p1.setCurrentColorGroup(QPalette::Disabled);
			p2.setBrush(QPalette::Disabled, QPalette::Highlight, p1.highlight().color());
			p2.setColor(QPalette::Disabled, QPalette::HighlightedText, p1.highlightedText().color());
			p1.setCurrentColorGroup(QPalette::Inactive);
			p2.setColor(QPalette::Inactive, QPalette::Highlight, p1.midlight().color().darker(108));
			p2.setColor(QPalette::Inactive, QPalette::HighlightedText, p1.buttonText().color());

			QApplication::setPalette(p2);
		}
	}
	// fix dark mode
	else
	{
		p1.setCurrentColorGroup(QPalette::Disabled);
		p2.setBrush(QPalette::Disabled, QPalette::Highlight, p1.highlight().color());
		p2.setColor(QPalette::Disabled, QPalette::HighlightedText, p1.highlightedText().color());
		p1.setCurrentColorGroup(QPalette::Inactive);
		p2.setColor(QPalette::Inactive, QPalette::Highlight, p1.mid().color().darker(307));
		p2.setColor(QPalette::Inactive, QPalette::HighlightedText, p1.highlightedText().color());

		QApplication::setPalette(p2);
	}
}

void theme::stylePseudoWinEarly()
{
	QStyle* style = QStyleFactory::create("windows");
	QApplication::setStyle(style);

	QColor highlightColor;
	QSettings accentColor = QSettings ("HKEY_CURRENT_USER\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Accent", QSettings::NativeFormat);
	if (accentColor.contains("AccentColorMenu")) {
		int color = accentColor.value("AccentColorMenu").toInt();
		int r = color & 0xff;
		int g = (color >> 8) & 0xff;
		int b = (color >> 16) & 0xff;
		highlightColor.setRgb(r, g, b);
	}

	//TODO improve win32 Qt5 not has fusion dark standard palette

/*#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
	QSettings personalize = QSettings ("HKEY_CURRENT_USER\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize", QSettings::NativeFormat);
	if (personalize.contains("AppsUseLightTheme") && ! personalize.value("AppsUseLightTheme").toBool())
	{
		QPalette p = QGuiApplication::palette();
		p.setColor(QPalette::All, QPalette::Window, Qt::black);
		QApplication::setPalette(p);
	}
#endif*/

	// dark mode: fusion
	if (theme::absLuma())
	{
		QPalette p1 = QGuiApplication::palette();

		QStyle* style = QStyleFactory::create("fusion");
		QApplication::setStyle(style);

		QPalette p2 = style->standardPalette();

		p1.setCurrentColorGroup(QPalette::Normal);
		p2.setColor(QPalette::WindowText, p1.windowText().color());
		p2.setColor(QPalette::Button, p1.button().color());
		p2.setColor(QPalette::Light, p1.light().color());
		p2.setColor(QPalette::Midlight, p1.midlight().color());
		p2.setColor(QPalette::Dark, p1.dark().color());
		p2.setColor(QPalette::Mid, p1.mid().color());
		p2.setColor(QPalette::Text, p1.text().color());
		p2.setColor(QPalette::BrightText, p1.brightText().color()); // accent
		p2.setColor(QPalette::ButtonText, p1.buttonText().color());
		p2.setColor(QPalette::Base, p1.base().color());
		p2.setColor(QPalette::Window, p1.window().color());
		p2.setColor(QPalette::Shadow, p1.shadow().color());

		if (highlightColor.isValid())
		{
			QColor highlightedTextColor = highlightColor.toHsl().lightness() > 128 ? Qt::black : Qt::white;

			p1.setCurrentColorGroup(QPalette::Active);
			p2.setColor(QPalette::Active, QPalette::Highlight, highlightColor);
			p2.setColor(QPalette::Active, QPalette::HighlightedText, highlightedTextColor);
			p1.setCurrentColorGroup(QPalette::Disabled);
			p2.setBrush(QPalette::Disabled, QPalette::Highlight, p1.highlight().color());
			p2.setColor(QPalette::Disabled, QPalette::HighlightedText, p1.highlightedText().color());
			p1.setCurrentColorGroup(QPalette::Inactive);
			p2.setColor(QPalette::Inactive, QPalette::Highlight, p1.highlight().color());
			p2.setColor(QPalette::Inactive, QPalette::HighlightedText, p1.highlightedText().color());
		}
		else
		{
			p1.setCurrentColorGroup(QPalette::Active);
			p2.setColor(QPalette::Active, QPalette::Highlight, p1.highlight().color());
			p2.setColor(QPalette::Active, QPalette::HighlightedText, p1.highlightedText().color());
			p1.setCurrentColorGroup(QPalette::Disabled);
			p2.setBrush(QPalette::Disabled, QPalette::Highlight, p1.highlight().color());
			p2.setColor(QPalette::Disabled, QPalette::HighlightedText, p1.highlightedText().color());
			p1.setCurrentColorGroup(QPalette::Inactive);
			p2.setColor(QPalette::Inactive, QPalette::Highlight, p1.highlight().color());
			p2.setColor(QPalette::Inactive, QPalette::HighlightedText, p1.highlightedText().color());
		}

		p1.setCurrentColorGroup(QPalette::Normal);
		p2.setColor(QPalette::AlternateBase, p1.alternateBase().color());
		p2.setColor(QPalette::NoRole, Qt::black);
		p2.setColor(QPalette::ToolTipBase, p1.toolTipBase().color());
		p2.setColor(QPalette::ToolTipText, p1.toolTipText().color());
		p2.setColor(QPalette::PlaceholderText, p1.placeholderText().color());
		p1.setCurrentColorGroup(QPalette::Disabled);
		p2.setColor(QPalette::Disabled, QPalette::WindowText, p1.windowText().color());
		p2.setBrush(QPalette::Disabled, QPalette::Dark, p1.dark().color());
		p2.setColor(QPalette::Disabled, QPalette::Text, p1.text().color());
		p2.setColor(QPalette::Disabled, QPalette::ButtonText, p1.buttonText().color());
		p2.setBrush(QPalette::Disabled, QPalette::Base, p1.base().color());
		p2.setBrush(QPalette::Disabled, QPalette::Shadow, p1.shadow().color());

		QApplication::setPalette(p2);
	}
	// light mode: vista
	else
	{
		QStyle* style = QStyleFactory::create("windowsvista");
		QApplication::setStyle(style);

		if (highlightColor.isValid())
		{
			QPalette p2 = QGuiApplication::palette();

			QColor highlightedTextColor = highlightColor.toHsl().lightness() > 128 ? Qt::black : Qt::white;

			p2.setColor(QPalette::Active, QPalette::Highlight, highlightColor);
			p2.setColor(QPalette::Active, QPalette::HighlightedText, highlightedTextColor);

			QApplication::setPalette(p2);
		}
	}
}

QString theme::qss_early_win_PseudoMetroDark_tlw()
{
	return "QMenuBar { background: palette(base) } QMenuBar:active { background: palette(shadow) } QMenu { border: 1px solid palette(light) } QPushButton, QToolButton, QLineEdit, QComboBox, QCheckBox, QRadioButton { border-radius: 0 } QPushButton, QComboBox { padding: 1px } QToolBar QPushButton, QDialog QPushButton { padding: 2px 3ex } QToolBar QComboBox, QDialog QComboBox { padding: 2px 4px } QToolButton { padding: 5px 0 } QLineEdit { padding: 1px } QHeaderView:section { padding: 4px 5px 6px } #dial_toggler { padding: 0 } QPushButton { background: transparent; border: 2px solid palette(midlight) } QPushButton:hover { background: palette(button) } QPushButton:pressed { border-color: palette(button-text) } QToolButton { background: transparent; border: 1px solid transparent } QToolButton:hover { background: palette(mid) } QToolButton:pressed { background: palette(midlight); border-color: palette(button-text) } QLineEdit, QComboBox { background: transparent; border: 1px solid palette(midlight) } QLineEdit:hover, QComboBox:hover, QComboBox:focus { border-color: palette(light) } QLineEdit:focus { border-color: palette(button-text) } QPushButton, QCheckBox, QRadioButton { outline: none } QCheckBox:focus:!pressed:!hover, QRadioButton:focus:!pressed:!hover { outline: 1px solid palette(button-text) } QCheckBox:focus:pressed { outline: none } QPushButton:focus:!pressed:!hover:!open { border-color: palette(light) } QToolBox:tab { background: palette(base); border: 2px solid palette(midlight) } QToolBox QWidget { background: palette(base) } QTabWidget, QTabBar { background: palette(window) } QTreeWidget, QHeaderView:section { background: palette(window) } QTabWidget:pane QSplitter { background: palette(mid) } #tree_search QPushButton, #list_search QPushButton { border-color: transparent } #tree_search QPushButton:pressed, #list_search QPushButton:pressed { background: palette(light) } ";
}
QString theme::qss_early_win_PseudoMetroDark_root()
{
	return "QMenu { border: 1px solid palette(light) }";
}

// before QApplication
void theme::early_win_flavor_fix()
{
	if (! theme::isOverridden() && theme::isMetroWin())
	{
		// QApplication style override
		//
		QSettings personalize = QSettings ("HKEY_CURRENT_USER\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize", QSettings::NativeFormat);
		if (personalize.contains("AppsUseLightTheme") && ! personalize.value("AppsUseLightTheme").toBool())
			qputenv("QT_STYLE_OVERRIDE", "fusion");
	}
}

// after top level widget
void theme::early_win_flavor_fix(QWidget* tlw)
{
	if (! theme::isOverridden() && theme::isMetroWin())
	{
		if (theme::isDefault() && theme::absLuma())
			tlw->setStyleSheet(theme::qss_early_win_PseudoMetroDark_tlw());
	}
}

// after QApplication
void theme::early_win_flavor_fix(QApplication* mroot)
{
	// additional fix application wide
	// 
	// keep QApplication stylesheet available
	// when passed from command line argument
	// -stylesheet file.qss
	if (! theme::isOverridden() && theme::isMetroWin() && mroot->styleSheet().isEmpty())
	{
		if (theme::isDefault() && theme::absLuma())
			mroot->setStyleSheet(theme::qss_early_win_PseudoMetroDark_root());
	}
}

#endif

}
