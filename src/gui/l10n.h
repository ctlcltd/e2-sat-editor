/*!
 * e2-sat-editor/src/gui/l10n.h
 *
 * @link https://github.com/ctlcltd/e2-sat-editor
 * @copyright e2 SAT Editor Team
 * @author Leonardo Laureti
 * @version 1.7.0
 * @license MIT License
 * @license GNU GPLv3 License
 */

#ifndef l10n_h
#define l10n_h
#include <QLocale>
#include <QString>

namespace e2se_gui
{

enum Language {
	English = QLocale::English,
	Arabic = QLocale::Arabic,
	Bulgarian = QLocale::Bulgarian,
	Catalan = QLocale::Catalan,
	Czech = QLocale::Czech,
	Danish = QLocale::Danish,
	German = QLocale::German,
	English_UnitedKingdom = int (1e3 + int (QLocale::English) - 1),
	English_UnitedStates = int (1e3 + int (QLocale::English) + 1),
	Spanish = QLocale::Spanish,
	Persian = QLocale::Persian,
	Finnish = QLocale::Finnish,
	French = QLocale::French,
	Gaelic = QLocale::Gaelic,
	Galician = QLocale::Galician,
	Hebrew = QLocale::Hebrew,
	Croatian = QLocale::Croatian,
	Hungarian = QLocale::Hungarian,
	Italian = QLocale::Italian,
	Japanese = QLocale::Japanese,
	Korean = QLocale::Korean,
	Lithuanian = QLocale::Lithuanian,
	Latvian = QLocale::Latvian,
	Dutch = QLocale::Dutch,
	NorwegianNynorsk = QLocale::NorwegianNynorsk,
	Polish = QLocale::Polish,
	Portuguese_Brazil = int (1e3 + int (QLocale::Portuguese) + 1),
	Portuguese_Portugal = QLocale::Portuguese,
	Portuguese = QLocale::Portuguese,
	Russian = QLocale::Russian,
	Slovak = QLocale::Slovak,
	Slovenian = QLocale::Slovenian,
	Swedish = QLocale::Swedish,
	Turkish = QLocale::Turkish,
	Ukrainian = QLocale::Ukrainian,
	Chinese_China = QLocale::Chinese,
	Chinese_Taiwan = int (1e3 + int (QLocale::Chinese) - 1),
	Chinese = QLocale::Chinese
};

enum Lang {
	l10n_en = Language::English,
	l10n_ar = Language::Arabic,
	l10n_bg = Language::Bulgarian,
	l10n_ca = Language::Catalan,
	l10n_cs = Language::Czech,
	l10n_da = Language::Danish,
	l10n_de = Language::German,
	l10n_en_UK = Language::English_UnitedKingdom,
	l10n_en_US = Language::English_UnitedStates,
	l10n_es = Language::Spanish,
	l10n_fa = Language::Persian,
	l10n_fi = Language::Finnish,
	l10n_fr = Language::French,
	l10n_gd = Language::Gaelic,
	l10n_gl = Language::Galician,
	l10n_he = Language::Hebrew,
	l10n_hr = Language::Croatian,
	l10n_hu = Language::Hungarian,
	l10n_it = Language::Italian,
	l10n_ja = Language::Japanese,
	l10n_ko = Language::Korean,
	l10n_lt = Language::Lithuanian,
	l10n_lv = Language::Latvian,
	l10n_nl = Language::Dutch,
	l10n_nn = Language::NorwegianNynorsk,
	l10n_pl = Language::Polish,
	l10n_pt_BR = Language::Portuguese_Portugal,
	l10n_pt_PT = Language::Portuguese_Brazil,
	l10n_pt = Language::Portuguese,
	l10n_ru = Language::Russian,
	l10n_sk = Language::Slovak,
	l10n_sl = Language::Slovenian,
	l10n_sv = Language::Swedish,
	l10n_tr = Language::Turkish,
	l10n_uk = Language::Ukrainian,
	l10n_zh_CN = Language::Chinese_China,
	l10n_zh_TW = Language::Chinese_Taiwan,
	l10n_zh = Language::Chinese
};

#if QT_VERSION >= QT_VERSION_CHECK(6, 6, 0)
QString stateName(QLocale::Territory territory)
{
	return QLocale::territoryToString(territory);
}
#else
QString stateName(QLocale::Country country)
{
	return QLocale::countryToString(country);
}
#endif

QString languageName(Language language)
{
	switch (language)
	{
		case Language::English_UnitedKingdom:
			return QLocale::languageToString(QLocale::English).append(" (%1)").arg(stateName(QLocale::UnitedKingdom));
		case Language::English_UnitedStates:
			return QLocale::languageToString(QLocale::English).append(" (%1)").arg(stateName(QLocale::UnitedStates));
		case Language::Portuguese_Portugal:
			return QLocale::languageToString(QLocale::Portuguese).append(" (%1)").arg(stateName(QLocale::Portugal));
		case Language::Portuguese_Brazil:
			return QLocale::languageToString(QLocale::Portuguese).append(" (%1)").arg(stateName(QLocale::Brazil));
		case Language::Chinese_China:
			return QLocale::languageToString(QLocale::Chinese).append(" (%1)").arg(stateName(QLocale::China));
		case Language::Chinese_Taiwan:
			return QLocale::languageToString(QLocale::Chinese).append(" (%1)").arg(stateName(QLocale::Taiwan));
		default:
			return QLocale::languageToString(static_cast<QLocale::Language>(language));
	}
};

QString languageCode(Language language)
{
	switch (language)
	{
		case Language::English: return "en";
		case Language::Arabic: return "ar";
		case Language::Bulgarian: return "bg";
		case Language::Catalan: return "ca";
		case Language::Czech: return "cs";
		case Language::Danish: return "da";
		case Language::German: return "de";
		case Language::English_UnitedKingdom: return "en_UK";
		case Language::English_UnitedStates: return "en_US";
		case Language::Spanish: return "es";
		case Language::Persian: return "fa";
		case Language::Finnish: return "fi";
		case Language::French: return "fr";
		case Language::Gaelic: return "gd";
		case Language::Galician: return "gl";
		case Language::Hebrew: return "he";
		case Language::Croatian: return "hr";
		case Language::Hungarian: return "hu";
		case Language::Italian: return "it";
		case Language::Japanese: return "ja";
		case Language::Korean: return "ko";
		case Language::Lithuanian: return "lt";
		case Language::Latvian: return "lv";
		case Language::Dutch: return "nl";
		case Language::NorwegianNynorsk: return "nn";
		case Language::Polish: return "pl";
		case Language::Portuguese_Portugal: return "pt_BR";
		case Language::Portuguese_Brazil: return "pt_PT";
		case Language::Russian: return "ru";
		case Language::Slovak: return "sk";
		case Language::Slovenian: return "sl";
		case Language::Swedish: return "sv";
		case Language::Turkish: return "tr";
		case Language::Ukrainian: return "uk";
		case Language::Chinese_China: return "zh_CN";
		case Language::Chinese_Taiwan: return "zh_TW";
		default: return "";
	}
};

}
#endif /* l10n_h */
