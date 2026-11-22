/*
 * Copyright (C) 2011 Google Inc. All rights reserved.
 * Copyright (C) 2021 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *     * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above
 * copyright notice, this list of conditions and the following disclaimer
 * in the documentation and/or other materials provided with the
 * distribution.
 *     * Neither the name of Google Inc. nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "Krystal.I18n/LocaleToScriptMapping.hpp"
#include "Krystal.Core/Containers/Array.hpp"
#include "Krystal.Core/Types/Pair.hpp"
#include "Krystal.Text/Script.hpp"
#include <algorithm>

namespace krys::i18n
{
  namespace
  {
    using Entry = Pair<utf8_stringview, text::Script>;

    // Looks up an ASCII-case-insensitive key in a table sorted by key. Keys longer than `maxLength` or
    // holding non-ASCII characters never match.
    template <size_t N>
    KRYS_NODISCARD Maybe<text::Script> Find(const Array<Entry, N> &table, utf8_stringview key,
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

  text::Script ScriptNameToCode(utf8_stringview scriptName) noexcept
  {
    // This generally maps an ISO 15924 script code to its text::Script, but certain families of script codes
    // are treated as a single script for assigning a per-script font in Settings. For example, "hira" is
    // mapped to text::Script::KatakanaOrHiragana instead of text::Script::Hiragana, since we want all
    // Japanese scripts to be rendered using the same font setting.
    constexpr static Array<Entry, 106> ScriptNameCodeList {{
      {u8"arab", text::Script::Arabic},
      {u8"armn", text::Script::Armenian},
      {u8"bali", text::Script::Balinese},
      {u8"batk", text::Script::Batak},
      {u8"beng", text::Script::Bengali},
      {u8"blis", text::Script::Blissymbols},
      {u8"bopo", text::Script::Bopomofo},
      {u8"brah", text::Script::Brahmi},
      {u8"brai", text::Script::Braille},
      {u8"bugi", text::Script::Buginese},
      {u8"buhd", text::Script::Buhid},
      {u8"cans", text::Script::CanadianAboriginal},
      {u8"cham", text::Script::Cham},
      {u8"cher", text::Script::Cherokee},
      {u8"cirt", text::Script::Cirth},
      {u8"copt", text::Script::Coptic},
      {u8"cprt", text::Script::Cypriot},
      {u8"cyrl", text::Script::Cyrillic},
      {u8"cyrs", text::Script::OldChurchSlavonicCyrillic},
      {u8"deva", text::Script::Devanagari},
      {u8"dsrt", text::Script::Deseret},
      {u8"egyd", text::Script::DemoticEgyptian},
      {u8"egyh", text::Script::HieraticEgyptian},
      {u8"egyp", text::Script::EgyptianHieroglyphs},
      {u8"ethi", text::Script::Ethiopic},
      {u8"geok", text::Script::Khutsuri},
      {u8"geor", text::Script::Georgian},
      {u8"glag", text::Script::Glagolitic},
      {u8"goth", text::Script::Gothic},
      {u8"grek", text::Script::Greek},
      {u8"gujr", text::Script::Gujarati},
      {u8"guru", text::Script::Gurmukhi},
      {u8"hang", text::Script::Hangul},
      {u8"hani", text::Script::Han},
      {u8"hano", text::Script::Hanunoo},
      {u8"hans", text::Script::SimplifiedHan},
      {u8"hant", text::Script::TraditionalHan},
      {u8"hebr", text::Script::Hebrew},
      {u8"hira", text::Script::KatakanaOrHiragana},
      {u8"hmng", text::Script::PahawhHmong},
      {u8"hrkt", text::Script::KatakanaOrHiragana},
      {u8"hung", text::Script::OldHungarian},
      {u8"inds", text::Script::HarappanIndus},
      {u8"ital", text::Script::OldItalic},
      {u8"java", text::Script::Javanese},
      {u8"jpan", text::Script::KatakanaOrHiragana},
      {u8"kali", text::Script::KayahLi},
      {u8"kana", text::Script::KatakanaOrHiragana},
      {u8"khar", text::Script::Kharoshthi},
      {u8"khmr", text::Script::Khmer},
      {u8"knda", text::Script::Kannada},
      {u8"kore", text::Script::Hangul},
      {u8"laoo", text::Script::Lao},
      {u8"latf", text::Script::LatinFraktur},
      {u8"latg", text::Script::LatinGaelic},
      {u8"latn", text::Script::Latin},
      {u8"lepc", text::Script::Lepcha},
      {u8"limb", text::Script::Limbu},
      {u8"lina", text::Script::LinearA},
      {u8"linb", text::Script::LinearB},
      {u8"mand", text::Script::Mandaean},
      {u8"maya", text::Script::MayanHieroglyphs},
      {u8"mero", text::Script::Meroitic},
      {u8"mlym", text::Script::Malayalam},
      {u8"mong", text::Script::Mongolian},
      {u8"mymr", text::Script::Myanmar},
      {u8"nkoo", text::Script::Nko},
      {u8"ogam", text::Script::Ogham},
      {u8"orkh", text::Script::Orkhon},
      {u8"orya", text::Script::Oriya},
      {u8"osma", text::Script::Osmanya},
      {u8"perm", text::Script::OldPermic},
      {u8"phag", text::Script::PhagsPa},
      {u8"phnx", text::Script::Phoenician},
      {u8"plrd", text::Script::PhoneticPollard},
      {u8"qaai", text::Script::Inherited},
      {u8"roro", text::Script::Rongorongo},
      {u8"runr", text::Script::Runic},
      {u8"sara", text::Script::Sarati},
      {u8"shaw", text::Script::Shavian},
      {u8"sinh", text::Script::Sinhala},
      {u8"sylo", text::Script::SylotiNagri},
      {u8"syrc", text::Script::Syriac},
      {u8"syre", text::Script::EstrangeloSyriac},
      {u8"syrj", text::Script::WesternSyriac},
      {u8"syrn", text::Script::EasternSyriac},
      {u8"tagb", text::Script::Tagbanwa},
      {u8"tale", text::Script::TaiLe},
      {u8"talu", text::Script::NewTaiLue},
      {u8"taml", text::Script::Tamil},
      {u8"telu", text::Script::Telugu},
      {u8"teng", text::Script::Tengwar},
      {u8"tfng", text::Script::Tifinagh},
      {u8"tglg", text::Script::Tagalog},
      {u8"thaa", text::Script::Thaana},
      {u8"thai", text::Script::Thai},
      {u8"tibt", text::Script::Tibetan},
      {u8"ugar", text::Script::Ugaritic},
      {u8"vaii", text::Script::Vai},
      {u8"visp", text::Script::VisibleSpeech},
      {u8"xpeo", text::Script::OldPersian},
      {u8"xsux", text::Script::Cuneiform},
      {u8"yiii", text::Script::Yi},
      {u8"zxxx", text::Script::UnwrittenLanguages},
      {u8"zyyy", text::Script::Common},
      {u8"zzzz", text::Script::Unknown},
    }};
    // Script names are made of letters only.
    if (!std::all_of(scriptName.begin(), scriptName.end(),
                     [](char8 c) { return (c >= u8'a' && c <= u8'z') || (c >= u8'A' && c <= u8'Z'); }))
    {
      return text::Script::Invalid;
    }
    return Find(ScriptNameCodeList, scriptName, 4).value_or(text::Script::Invalid);
  }

  text::Script LocaleToScriptCode(utf8_stringview locale) noexcept
  {
    constexpr static Array<Entry, 198> LocaleScriptList {{
      {u8"aa", text::Script::Latin},
      {u8"ab", text::Script::Cyrillic},
      {u8"ady", text::Script::Cyrillic},
      {u8"af", text::Script::Latin},
      {u8"ak", text::Script::Latin},
      {u8"am", text::Script::Ethiopic},
      {u8"ar", text::Script::Arabic},
      {u8"as", text::Script::Bengali},
      {u8"ast", text::Script::Latin},
      {u8"av", text::Script::Cyrillic},
      {u8"ay", text::Script::Latin},
      {u8"az", text::Script::Latin},
      {u8"ba", text::Script::Cyrillic},
      {u8"be", text::Script::Cyrillic},
      {u8"bg", text::Script::Cyrillic},
      {u8"bi", text::Script::Latin},
      {u8"bn", text::Script::Bengali},
      {u8"bo", text::Script::Tibetan},
      {u8"bs", text::Script::Latin},
      {u8"ca", text::Script::Latin},
      {u8"ce", text::Script::Cyrillic},
      {u8"ceb", text::Script::Latin},
      {u8"ch", text::Script::Latin},
      {u8"chk", text::Script::Latin},
      {u8"cs", text::Script::Latin},
      {u8"cy", text::Script::Latin},
      {u8"da", text::Script::Latin},
      {u8"de", text::Script::Latin},
      {u8"dv", text::Script::Thaana},
      {u8"dz", text::Script::Tibetan},
      {u8"ee", text::Script::Latin},
      {u8"efi", text::Script::Latin},
      {u8"el", text::Script::Greek},
      {u8"en", text::Script::Latin},
      {u8"es", text::Script::Latin},
      {u8"et", text::Script::Latin},
      {u8"eu", text::Script::Latin},
      {u8"fa", text::Script::Arabic},
      {u8"fi", text::Script::Latin},
      {u8"fil", text::Script::Latin},
      {u8"fj", text::Script::Latin},
      {u8"fo", text::Script::Latin},
      {u8"fr", text::Script::Latin},
      {u8"fur", text::Script::Latin},
      {u8"fy", text::Script::Latin},
      {u8"ga", text::Script::Latin},
      {u8"gaa", text::Script::Latin},
      {u8"gd", text::Script::Latin},
      {u8"gil", text::Script::Latin},
      {u8"gl", text::Script::Latin},
      {u8"gn", text::Script::Latin},
      {u8"gsw", text::Script::Latin},
      {u8"gu", text::Script::Gujarati},
      {u8"ha", text::Script::Latin},
      {u8"haw", text::Script::Latin},
      {u8"he", text::Script::Hebrew},
      {u8"hi", text::Script::Devanagari},
      {u8"hil", text::Script::Latin},
      {u8"ho", text::Script::Latin},
      {u8"hr", text::Script::Latin},
      {u8"ht", text::Script::Latin},
      {u8"hu", text::Script::Latin},
      {u8"hy", text::Script::Armenian},
      {u8"id", text::Script::Latin},
      {u8"ig", text::Script::Latin},
      {u8"ii", text::Script::Yi},
      {u8"ilo", text::Script::Latin},
      {u8"inh", text::Script::Cyrillic},
      {u8"is", text::Script::Latin},
      {u8"it", text::Script::Latin},
      {u8"iu", text::Script::CanadianAboriginal},
      {u8"ja", text::Script::KatakanaOrHiragana},
      {u8"jv", text::Script::Latin},
      {u8"ka", text::Script::Georgian},
      {u8"kaj", text::Script::Latin},
      {u8"kam", text::Script::Latin},
      {u8"kbd", text::Script::Cyrillic},
      {u8"kha", text::Script::Latin},
      {u8"kk", text::Script::Cyrillic},
      {u8"kl", text::Script::Latin},
      {u8"km", text::Script::Khmer},
      {u8"kn", text::Script::Kannada},
      {u8"ko", text::Script::Hangul},
      {u8"kok", text::Script::Devanagari},
      {u8"kos", text::Script::Latin},
      {u8"kpe", text::Script::Latin},
      {u8"krc", text::Script::Cyrillic},
      {u8"ks", text::Script::Arabic},
      {u8"ku", text::Script::Arabic},
      {u8"kum", text::Script::Cyrillic},
      {u8"ky", text::Script::Cyrillic},
      {u8"la", text::Script::Latin},
      {u8"lah", text::Script::Arabic},
      {u8"lb", text::Script::Latin},
      {u8"lez", text::Script::Cyrillic},
      {u8"ln", text::Script::Latin},
      {u8"lo", text::Script::Lao},
      {u8"lt", text::Script::Latin},
      {u8"lv", text::Script::Latin},
      {u8"mai", text::Script::Devanagari},
      {u8"mdf", text::Script::Cyrillic},
      {u8"mg", text::Script::Latin},
      {u8"mh", text::Script::Latin},
      {u8"mi", text::Script::Latin},
      {u8"mk", text::Script::Cyrillic},
      {u8"ml", text::Script::Malayalam},
      {u8"mn", text::Script::Cyrillic},
      {u8"mr", text::Script::Devanagari},
      {u8"ms", text::Script::Latin},
      {u8"mt", text::Script::Latin},
      {u8"my", text::Script::Myanmar},
      {u8"myv", text::Script::Cyrillic},
      {u8"na", text::Script::Latin},
      {u8"nb", text::Script::Latin},
      {u8"ne", text::Script::Devanagari},
      {u8"niu", text::Script::Latin},
      {u8"nl", text::Script::Latin},
      {u8"nn", text::Script::Latin},
      {u8"nr", text::Script::Latin},
      {u8"nso", text::Script::Latin},
      {u8"ny", text::Script::Latin},
      {u8"oc", text::Script::Latin},
      {u8"om", text::Script::Latin},
      {u8"or", text::Script::Oriya},
      {u8"os", text::Script::Cyrillic},
      {u8"pa", text::Script::Gurmukhi},
      {u8"pag", text::Script::Latin},
      {u8"pap", text::Script::Latin},
      {u8"pau", text::Script::Latin},
      {u8"pl", text::Script::Latin},
      {u8"pon", text::Script::Latin},
      {u8"ps", text::Script::Arabic},
      {u8"pt", text::Script::Latin},
      {u8"qu", text::Script::Latin},
      {u8"rm", text::Script::Latin},
      {u8"rn", text::Script::Latin},
      {u8"ro", text::Script::Latin},
      {u8"ru", text::Script::Cyrillic},
      {u8"rw", text::Script::Latin},
      {u8"sa", text::Script::Devanagari},
      {u8"sah", text::Script::Cyrillic},
      {u8"sat", text::Script::Latin},
      {u8"sd", text::Script::Arabic},
      {u8"se", text::Script::Latin},
      {u8"sg", text::Script::Latin},
      {u8"si", text::Script::Sinhala},
      {u8"sid", text::Script::Latin},
      {u8"sk", text::Script::Latin},
      {u8"sl", text::Script::Latin},
      {u8"sm", text::Script::Latin},
      {u8"so", text::Script::Latin},
      {u8"sq", text::Script::Latin},
      {u8"sr", text::Script::Cyrillic},
      {u8"ss", text::Script::Latin},
      {u8"st", text::Script::Latin},
      {u8"su", text::Script::Latin},
      {u8"sv", text::Script::Latin},
      {u8"sw", text::Script::Latin},
      {u8"ta", text::Script::Tamil},
      {u8"te", text::Script::Telugu},
      {u8"tet", text::Script::Latin},
      {u8"tg", text::Script::Cyrillic},
      {u8"th", text::Script::Thai},
      {u8"ti", text::Script::Ethiopic},
      {u8"tig", text::Script::Ethiopic},
      {u8"tk", text::Script::Latin},
      {u8"tkl", text::Script::Latin},
      {u8"tl", text::Script::Latin},
      {u8"tn", text::Script::Latin},
      {u8"to", text::Script::Latin},
      {u8"tpi", text::Script::Latin},
      {u8"tr", text::Script::Latin},
      {u8"trv", text::Script::Latin},
      {u8"ts", text::Script::Latin},
      {u8"tt", text::Script::Cyrillic},
      {u8"tvl", text::Script::Latin},
      {u8"tw", text::Script::Latin},
      {u8"ty", text::Script::Latin},
      {u8"tyv", text::Script::Cyrillic},
      {u8"udm", text::Script::Cyrillic},
      {u8"ug", text::Script::Arabic},
      {u8"uk", text::Script::Cyrillic},
      {u8"und", text::Script::Latin},
      {u8"ur", text::Script::Arabic},
      {u8"uz", text::Script::Cyrillic},
      {u8"ve", text::Script::Latin},
      {u8"vi", text::Script::Latin},
      {u8"wal", text::Script::Ethiopic},
      {u8"war", text::Script::Latin},
      {u8"wo", text::Script::Latin},
      {u8"xh", text::Script::Latin},
      {u8"yap", text::Script::Latin},
      {u8"yo", text::Script::Latin},
      {u8"za", text::Script::Latin},
      {u8"zh", text::Script::Han},
      {u8"zh_hk", text::Script::TraditionalHan},
      {u8"zh_tw", text::Script::TraditionalHan},
      {u8"zu", text::Script::Latin},
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
      text::Script code = ScriptNameToCode(canonicalLocale.substr(underscorePosition + 1));
      if (code != text::Script::Invalid && code != text::Script::Unknown)
      {
        return code;
      }
      canonicalLocale = canonicalLocale.substr(0, underscorePosition);
    }
    return text::Script::Common;
  }
}
