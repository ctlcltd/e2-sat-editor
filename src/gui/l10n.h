/*!
 * e2-sat-editor/src/gui/l10n.h
 *
 * @link https://github.com/ctlcltd/e2-sat-editor
 * @copyright e2 SAT Editor Team
 * @author Leonardo Laureti
 * @version 0.9
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
	Chinese_Taiwan = int (1e3 + int (QLocale::Chinese) + 1),
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
		case Language::Portuguese_Portugal:
			return QLocale::languageToString(QLocale::Portuguese).append(" (%1)").arg(stateName(QLocale::Portugal));
		break;
		case Language::Portuguese_Brazil:
			return QLocale::languageToString(QLocale::Portuguese).append(" (%1)").arg(stateName(QLocale::Brazil));
		break;
		case Language::Chinese_China:
			return QLocale::languageToString(QLocale::Chinese).append(" (%1)").arg(stateName(QLocale::China));
		break;
		case Language::Chinese_Taiwan:
			return QLocale::languageToString(QLocale::Chinese).append(" (%1)").arg(stateName(QLocale::Taiwan));
		break;
		default:
			return QLocale::languageToString(static_cast<QLocale::Language>(language));
	}
};

QString languageCode(Language language)
{
	switch (language)
	{
		case Language::English: return "en"; break;
		case Language::Arabic: return "ar"; break;
		case Language::Bulgarian: return "bg"; break;
		case Language::Catalan: return "ca"; break;
		case Language::Czech: return "cs"; break;
		case Language::Danish: return "da"; break;
		case Language::German: return "de"; break;
		case Language::Spanish: return "es"; break;
		case Language::Persian: return "fa"; break;
		case Language::Finnish: return "fi"; break;
		case Language::French: return "fr"; break;
		case Language::Gaelic: return "gd"; break;
		case Language::Galician: return "gl"; break;
		case Language::Hebrew: return "he"; break;
		case Language::Croatian: return "hr"; break;
		case Language::Hungarian: return "hu"; break;
		case Language::Italian: return "it"; break;
		case Language::Japanese: return "ja"; break;
		case Language::Korean: return "ko"; break;
		case Language::Lithuanian: return "lt"; break;
		case Language::Latvian: return "lv"; break;
		case Language::Dutch: return "nl"; break;
		case Language::NorwegianNynorsk: return "nn"; break;
		case Language::Polish: return "pl"; break;
		case Language::Portuguese_Portugal: return "pt_BR"; break;
		case Language::Portuguese_Brazil: return "pt_PT"; break;
		case Language::Russian: return "ru"; break;
		case Language::Slovak: return "sk"; break;
		case Language::Slovenian: return "sl"; break;
		case Language::Swedish: return "sv"; break;
		case Language::Turkish: return "tr"; break;
		case Language::Ukrainian: return "uk"; break;
		case Language::Chinese_China: return "zh_CN"; break;
		case Language::Chinese_Taiwan: return "zh_TW"; break;
		default: return "";
	}
};

}
#endif /* l10n_h */
