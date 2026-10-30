#include "Krystal.Booey/Core/LocaleToScriptMapping.hpp"
#include "Krystal.Core/Types/Array.hpp"
#include "Krystal.Core/Types/Pair.hpp"
#include <algorithm>

namespace krys::boo
{
  namespace
  {
    using Entry = Pair<utf8_stringview, UScriptCode>;

    // Looks up an ASCII-case-insensitive key in a table sorted by key. Keys longer than `maxLength` or
    // holding non-ASCII characters never match.
    template <size_t N>
    KRYS_NODISCARD Maybe<UScriptCode> Find(const Array<Entry, N> &table, utf8_stringview key,
                                           size_t maxLength) noexcept
    {
      if (key.size() > maxLength)
      {
        return std::nullopt;
      }
      utf8_string lowered;
      lowered.reserve(key.size());
      for (auto c : key)
      {
        if (static_cast<uint8>(c) >= 0x80)
        {
          return std::nullopt;
        }
        lowered.push_back(c >= u8'A' && c <= u8'Z' ? static_cast<char8>(c - u8'A' + u8'a') : c);
      }
      auto it =
        std::lower_bound(table.begin(), table.end(), utf8_stringview {lowered},
                         [](const Entry &entry, utf8_stringview value) { return entry.first < value; });
      if (it == table.end() || it->first != utf8_stringview {lowered})
      {
        return std::nullopt;
      }
      return it->second;
    }
  }

  UScriptCode ScriptNameToCode(utf8_stringview scriptName) noexcept
  {
    // This generally maps an ISO 15924 script code to its UScriptCode, but certain families of script codes
    // are treated as a single script for assigning a per-script font in Settings. For example, "hira" is
    // mapped to USCRIPT_KATAKANA_OR_HIRAGANA instead of USCRIPT_HIRAGANA, since we want all Japanese scripts
    // to be rendered using the same font setting.
    constexpr static Array<Entry, 106> ScriptNameCodeList {{
      {u8"arab", USCRIPT_ARABIC},
      {u8"armn", USCRIPT_ARMENIAN},
      {u8"bali", USCRIPT_BALINESE},
      {u8"batk", USCRIPT_BATAK},
      {u8"beng", USCRIPT_BENGALI},
      {u8"blis", USCRIPT_BLISSYMBOLS},
      {u8"bopo", USCRIPT_BOPOMOFO},
      {u8"brah", USCRIPT_BRAHMI},
      {u8"brai", USCRIPT_BRAILLE},
      {u8"bugi", USCRIPT_BUGINESE},
      {u8"buhd", USCRIPT_BUHID},
      {u8"cans", USCRIPT_CANADIAN_ABORIGINAL},
      {u8"cham", USCRIPT_CHAM},
      {u8"cher", USCRIPT_CHEROKEE},
      {u8"cirt", USCRIPT_CIRTH},
      {u8"copt", USCRIPT_COPTIC},
      {u8"cprt", USCRIPT_CYPRIOT},
      {u8"cyrl", USCRIPT_CYRILLIC},
      {u8"cyrs", USCRIPT_OLD_CHURCH_SLAVONIC_CYRILLIC},
      {u8"deva", USCRIPT_DEVANAGARI},
      {u8"dsrt", USCRIPT_DESERET},
      {u8"egyd", USCRIPT_DEMOTIC_EGYPTIAN},
      {u8"egyh", USCRIPT_HIERATIC_EGYPTIAN},
      {u8"egyp", USCRIPT_EGYPTIAN_HIEROGLYPHS},
      {u8"ethi", USCRIPT_ETHIOPIC},
      {u8"geok", USCRIPT_KHUTSURI},
      {u8"geor", USCRIPT_GEORGIAN},
      {u8"glag", USCRIPT_GLAGOLITIC},
      {u8"goth", USCRIPT_GOTHIC},
      {u8"grek", USCRIPT_GREEK},
      {u8"gujr", USCRIPT_GUJARATI},
      {u8"guru", USCRIPT_GURMUKHI},
      {u8"hang", USCRIPT_HANGUL},
      {u8"hani", USCRIPT_HAN},
      {u8"hano", USCRIPT_HANUNOO},
      {u8"hans", USCRIPT_SIMPLIFIED_HAN},
      {u8"hant", USCRIPT_TRADITIONAL_HAN},
      {u8"hebr", USCRIPT_HEBREW},
      {u8"hira", USCRIPT_KATAKANA_OR_HIRAGANA},
      {u8"hmng", USCRIPT_PAHAWH_HMONG},
      {u8"hrkt", USCRIPT_KATAKANA_OR_HIRAGANA},
      {u8"hung", USCRIPT_OLD_HUNGARIAN},
      {u8"inds", USCRIPT_HARAPPAN_INDUS},
      {u8"ital", USCRIPT_OLD_ITALIC},
      {u8"java", USCRIPT_JAVANESE},
      {u8"jpan", USCRIPT_KATAKANA_OR_HIRAGANA},
      {u8"kali", USCRIPT_KAYAH_LI},
      {u8"kana", USCRIPT_KATAKANA_OR_HIRAGANA},
      {u8"khar", USCRIPT_KHAROSHTHI},
      {u8"khmr", USCRIPT_KHMER},
      {u8"knda", USCRIPT_KANNADA},
      {u8"kore", USCRIPT_HANGUL},
      {u8"laoo", USCRIPT_LAO},
      {u8"latf", USCRIPT_LATIN_FRAKTUR},
      {u8"latg", USCRIPT_LATIN_GAELIC},
      {u8"latn", USCRIPT_LATIN},
      {u8"lepc", USCRIPT_LEPCHA},
      {u8"limb", USCRIPT_LIMBU},
      {u8"lina", USCRIPT_LINEAR_A},
      {u8"linb", USCRIPT_LINEAR_B},
      {u8"mand", USCRIPT_MANDAEAN},
      {u8"maya", USCRIPT_MAYAN_HIEROGLYPHS},
      {u8"mero", USCRIPT_MEROITIC},
      {u8"mlym", USCRIPT_MALAYALAM},
      {u8"mong", USCRIPT_MONGOLIAN},
      {u8"mymr", USCRIPT_MYANMAR},
      {u8"nkoo", USCRIPT_NKO},
      {u8"ogam", USCRIPT_OGHAM},
      {u8"orkh", USCRIPT_ORKHON},
      {u8"orya", USCRIPT_ORIYA},
      {u8"osma", USCRIPT_OSMANYA},
      {u8"perm", USCRIPT_OLD_PERMIC},
      {u8"phag", USCRIPT_PHAGS_PA},
      {u8"phnx", USCRIPT_PHOENICIAN},
      {u8"plrd", USCRIPT_PHONETIC_POLLARD},
      {u8"qaai", USCRIPT_INHERITED},
      {u8"roro", USCRIPT_RONGORONGO},
      {u8"runr", USCRIPT_RUNIC},
      {u8"sara", USCRIPT_SARATI},
      {u8"shaw", USCRIPT_SHAVIAN},
      {u8"sinh", USCRIPT_SINHALA},
      {u8"sylo", USCRIPT_SYLOTI_NAGRI},
      {u8"syrc", USCRIPT_SYRIAC},
      {u8"syre", USCRIPT_ESTRANGELO_SYRIAC},
      {u8"syrj", USCRIPT_WESTERN_SYRIAC},
      {u8"syrn", USCRIPT_EASTERN_SYRIAC},
      {u8"tagb", USCRIPT_TAGBANWA},
      {u8"tale", USCRIPT_TAI_LE},
      {u8"talu", USCRIPT_NEW_TAI_LUE},
      {u8"taml", USCRIPT_TAMIL},
      {u8"telu", USCRIPT_TELUGU},
      {u8"teng", USCRIPT_TENGWAR},
      {u8"tfng", USCRIPT_TIFINAGH},
      {u8"tglg", USCRIPT_TAGALOG},
      {u8"thaa", USCRIPT_THAANA},
      {u8"thai", USCRIPT_THAI},
      {u8"tibt", USCRIPT_TIBETAN},
      {u8"ugar", USCRIPT_UGARITIC},
      {u8"vaii", USCRIPT_VAI},
      {u8"visp", USCRIPT_VISIBLE_SPEECH},
      {u8"xpeo", USCRIPT_OLD_PERSIAN},
      {u8"xsux", USCRIPT_CUNEIFORM},
      {u8"yiii", USCRIPT_YI},
      {u8"zxxx", USCRIPT_UNWRITTEN_LANGUAGES},
      {u8"zyyy", USCRIPT_COMMON},
      {u8"zzzz", USCRIPT_UNKNOWN},
    }};
    // Script names are made of letters only.
    if (!std::all_of(scriptName.begin(), scriptName.end(),
                     [](char8 c) { return (c >= u8'a' && c <= u8'z') || (c >= u8'A' && c <= u8'Z'); }))
    {
      return USCRIPT_INVALID_CODE;
    }
    return Find(ScriptNameCodeList, scriptName, 4).value_or(USCRIPT_INVALID_CODE);
  }

  UScriptCode LocaleToScriptCode(utf8_stringview locale) noexcept
  {
    constexpr static Array<Entry, 198> LocaleScriptList {{
      {u8"aa", USCRIPT_LATIN},
      {u8"ab", USCRIPT_CYRILLIC},
      {u8"ady", USCRIPT_CYRILLIC},
      {u8"af", USCRIPT_LATIN},
      {u8"ak", USCRIPT_LATIN},
      {u8"am", USCRIPT_ETHIOPIC},
      {u8"ar", USCRIPT_ARABIC},
      {u8"as", USCRIPT_BENGALI},
      {u8"ast", USCRIPT_LATIN},
      {u8"av", USCRIPT_CYRILLIC},
      {u8"ay", USCRIPT_LATIN},
      {u8"az", USCRIPT_LATIN},
      {u8"ba", USCRIPT_CYRILLIC},
      {u8"be", USCRIPT_CYRILLIC},
      {u8"bg", USCRIPT_CYRILLIC},
      {u8"bi", USCRIPT_LATIN},
      {u8"bn", USCRIPT_BENGALI},
      {u8"bo", USCRIPT_TIBETAN},
      {u8"bs", USCRIPT_LATIN},
      {u8"ca", USCRIPT_LATIN},
      {u8"ce", USCRIPT_CYRILLIC},
      {u8"ceb", USCRIPT_LATIN},
      {u8"ch", USCRIPT_LATIN},
      {u8"chk", USCRIPT_LATIN},
      {u8"cs", USCRIPT_LATIN},
      {u8"cy", USCRIPT_LATIN},
      {u8"da", USCRIPT_LATIN},
      {u8"de", USCRIPT_LATIN},
      {u8"dv", USCRIPT_THAANA},
      {u8"dz", USCRIPT_TIBETAN},
      {u8"ee", USCRIPT_LATIN},
      {u8"efi", USCRIPT_LATIN},
      {u8"el", USCRIPT_GREEK},
      {u8"en", USCRIPT_LATIN},
      {u8"es", USCRIPT_LATIN},
      {u8"et", USCRIPT_LATIN},
      {u8"eu", USCRIPT_LATIN},
      {u8"fa", USCRIPT_ARABIC},
      {u8"fi", USCRIPT_LATIN},
      {u8"fil", USCRIPT_LATIN},
      {u8"fj", USCRIPT_LATIN},
      {u8"fo", USCRIPT_LATIN},
      {u8"fr", USCRIPT_LATIN},
      {u8"fur", USCRIPT_LATIN},
      {u8"fy", USCRIPT_LATIN},
      {u8"ga", USCRIPT_LATIN},
      {u8"gaa", USCRIPT_LATIN},
      {u8"gd", USCRIPT_LATIN},
      {u8"gil", USCRIPT_LATIN},
      {u8"gl", USCRIPT_LATIN},
      {u8"gn", USCRIPT_LATIN},
      {u8"gsw", USCRIPT_LATIN},
      {u8"gu", USCRIPT_GUJARATI},
      {u8"ha", USCRIPT_LATIN},
      {u8"haw", USCRIPT_LATIN},
      {u8"he", USCRIPT_HEBREW},
      {u8"hi", USCRIPT_DEVANAGARI},
      {u8"hil", USCRIPT_LATIN},
      {u8"ho", USCRIPT_LATIN},
      {u8"hr", USCRIPT_LATIN},
      {u8"ht", USCRIPT_LATIN},
      {u8"hu", USCRIPT_LATIN},
      {u8"hy", USCRIPT_ARMENIAN},
      {u8"id", USCRIPT_LATIN},
      {u8"ig", USCRIPT_LATIN},
      {u8"ii", USCRIPT_YI},
      {u8"ilo", USCRIPT_LATIN},
      {u8"inh", USCRIPT_CYRILLIC},
      {u8"is", USCRIPT_LATIN},
      {u8"it", USCRIPT_LATIN},
      {u8"iu", USCRIPT_CANADIAN_ABORIGINAL},
      {u8"ja", USCRIPT_KATAKANA_OR_HIRAGANA},
      {u8"jv", USCRIPT_LATIN},
      {u8"ka", USCRIPT_GEORGIAN},
      {u8"kaj", USCRIPT_LATIN},
      {u8"kam", USCRIPT_LATIN},
      {u8"kbd", USCRIPT_CYRILLIC},
      {u8"kha", USCRIPT_LATIN},
      {u8"kk", USCRIPT_CYRILLIC},
      {u8"kl", USCRIPT_LATIN},
      {u8"km", USCRIPT_KHMER},
      {u8"kn", USCRIPT_KANNADA},
      {u8"ko", USCRIPT_HANGUL},
      {u8"kok", USCRIPT_DEVANAGARI},
      {u8"kos", USCRIPT_LATIN},
      {u8"kpe", USCRIPT_LATIN},
      {u8"krc", USCRIPT_CYRILLIC},
      {u8"ks", USCRIPT_ARABIC},
      {u8"ku", USCRIPT_ARABIC},
      {u8"kum", USCRIPT_CYRILLIC},
      {u8"ky", USCRIPT_CYRILLIC},
      {u8"la", USCRIPT_LATIN},
      {u8"lah", USCRIPT_ARABIC},
      {u8"lb", USCRIPT_LATIN},
      {u8"lez", USCRIPT_CYRILLIC},
      {u8"ln", USCRIPT_LATIN},
      {u8"lo", USCRIPT_LAO},
      {u8"lt", USCRIPT_LATIN},
      {u8"lv", USCRIPT_LATIN},
      {u8"mai", USCRIPT_DEVANAGARI},
      {u8"mdf", USCRIPT_CYRILLIC},
      {u8"mg", USCRIPT_LATIN},
      {u8"mh", USCRIPT_LATIN},
      {u8"mi", USCRIPT_LATIN},
      {u8"mk", USCRIPT_CYRILLIC},
      {u8"ml", USCRIPT_MALAYALAM},
      {u8"mn", USCRIPT_CYRILLIC},
      {u8"mr", USCRIPT_DEVANAGARI},
      {u8"ms", USCRIPT_LATIN},
      {u8"mt", USCRIPT_LATIN},
      {u8"my", USCRIPT_MYANMAR},
      {u8"myv", USCRIPT_CYRILLIC},
      {u8"na", USCRIPT_LATIN},
      {u8"nb", USCRIPT_LATIN},
      {u8"ne", USCRIPT_DEVANAGARI},
      {u8"niu", USCRIPT_LATIN},
      {u8"nl", USCRIPT_LATIN},
      {u8"nn", USCRIPT_LATIN},
      {u8"nr", USCRIPT_LATIN},
      {u8"nso", USCRIPT_LATIN},
      {u8"ny", USCRIPT_LATIN},
      {u8"oc", USCRIPT_LATIN},
      {u8"om", USCRIPT_LATIN},
      {u8"or", USCRIPT_ORIYA},
      {u8"os", USCRIPT_CYRILLIC},
      {u8"pa", USCRIPT_GURMUKHI},
      {u8"pag", USCRIPT_LATIN},
      {u8"pap", USCRIPT_LATIN},
      {u8"pau", USCRIPT_LATIN},
      {u8"pl", USCRIPT_LATIN},
      {u8"pon", USCRIPT_LATIN},
      {u8"ps", USCRIPT_ARABIC},
      {u8"pt", USCRIPT_LATIN},
      {u8"qu", USCRIPT_LATIN},
      {u8"rm", USCRIPT_LATIN},
      {u8"rn", USCRIPT_LATIN},
      {u8"ro", USCRIPT_LATIN},
      {u8"ru", USCRIPT_CYRILLIC},
      {u8"rw", USCRIPT_LATIN},
      {u8"sa", USCRIPT_DEVANAGARI},
      {u8"sah", USCRIPT_CYRILLIC},
      {u8"sat", USCRIPT_LATIN},
      {u8"sd", USCRIPT_ARABIC},
      {u8"se", USCRIPT_LATIN},
      {u8"sg", USCRIPT_LATIN},
      {u8"si", USCRIPT_SINHALA},
      {u8"sid", USCRIPT_LATIN},
      {u8"sk", USCRIPT_LATIN},
      {u8"sl", USCRIPT_LATIN},
      {u8"sm", USCRIPT_LATIN},
      {u8"so", USCRIPT_LATIN},
      {u8"sq", USCRIPT_LATIN},
      {u8"sr", USCRIPT_CYRILLIC},
      {u8"ss", USCRIPT_LATIN},
      {u8"st", USCRIPT_LATIN},
      {u8"su", USCRIPT_LATIN},
      {u8"sv", USCRIPT_LATIN},
      {u8"sw", USCRIPT_LATIN},
      {u8"ta", USCRIPT_TAMIL},
      {u8"te", USCRIPT_TELUGU},
      {u8"tet", USCRIPT_LATIN},
      {u8"tg", USCRIPT_CYRILLIC},
      {u8"th", USCRIPT_THAI},
      {u8"ti", USCRIPT_ETHIOPIC},
      {u8"tig", USCRIPT_ETHIOPIC},
      {u8"tk", USCRIPT_LATIN},
      {u8"tkl", USCRIPT_LATIN},
      {u8"tl", USCRIPT_LATIN},
      {u8"tn", USCRIPT_LATIN},
      {u8"to", USCRIPT_LATIN},
      {u8"tpi", USCRIPT_LATIN},
      {u8"tr", USCRIPT_LATIN},
      {u8"trv", USCRIPT_LATIN},
      {u8"ts", USCRIPT_LATIN},
      {u8"tt", USCRIPT_CYRILLIC},
      {u8"tvl", USCRIPT_LATIN},
      {u8"tw", USCRIPT_LATIN},
      {u8"ty", USCRIPT_LATIN},
      {u8"tyv", USCRIPT_CYRILLIC},
      {u8"udm", USCRIPT_CYRILLIC},
      {u8"ug", USCRIPT_ARABIC},
      {u8"uk", USCRIPT_CYRILLIC},
      {u8"und", USCRIPT_LATIN},
      {u8"ur", USCRIPT_ARABIC},
      {u8"uz", USCRIPT_CYRILLIC},
      {u8"ve", USCRIPT_LATIN},
      {u8"vi", USCRIPT_LATIN},
      {u8"wal", USCRIPT_ETHIOPIC},
      {u8"war", USCRIPT_LATIN},
      {u8"wo", USCRIPT_LATIN},
      {u8"xh", USCRIPT_LATIN},
      {u8"yap", USCRIPT_LATIN},
      {u8"yo", USCRIPT_LATIN},
      {u8"za", USCRIPT_LATIN},
      {u8"zh", USCRIPT_HAN},
      {u8"zh_hk", USCRIPT_TRADITIONAL_HAN},
      {u8"zh_tw", USCRIPT_TRADITIONAL_HAN},
      {u8"zu", USCRIPT_LATIN},
    }};
    utf8_string canonicalLocaleString {locale};
    std::replace(canonicalLocaleString.begin(), canonicalLocaleString.end(), u8'-', u8'_');
    for (utf8_stringview canonicalLocale = canonicalLocaleString; !canonicalLocale.empty();)
    {
      if (auto scriptCode = Find(LocaleScriptList, canonicalLocale, 8))
      {
        return *scriptCode;
      }
      auto underscorePosition = canonicalLocale.rfind(u8'_');
      if (underscorePosition == utf8_stringview::npos)
      {
        break;
      }
      UScriptCode code = ScriptNameToCode(canonicalLocale.substr(underscorePosition + 1));
      if (code != USCRIPT_INVALID_CODE && code != USCRIPT_UNKNOWN)
      {
        return code;
      }
      canonicalLocale = canonicalLocale.substr(0, underscorePosition);
    }
    return USCRIPT_COMMON;
  }
}
