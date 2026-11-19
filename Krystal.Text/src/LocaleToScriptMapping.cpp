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

#include "Krystal.Text/LocaleToScriptMapping.hpp"
#include "Krystal.Core/Types/Array.hpp"
#include "Krystal.Core/Types/Pair.hpp"
#include "Krystal.Text/Script.hpp"
#include <algorithm>

namespace krys::text
{
  namespace
  {
    using Entry = Pair<utf8_stringview, Script>;

    // Looks up an ASCII-case-insensitive key in a table sorted by key. Keys longer than `maxLength` or
    // holding non-ASCII characters never match.
    template <size_t N>
    KRYS_NODISCARD Maybe<Script> Find(const Array<Entry, N> &table, utf8_stringview key,
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

  Script ScriptNameToCode(utf8_stringview scriptName) noexcept
  {
    // This generally maps an ISO 15924 script code to its Script, but certain families of script codes
    // are treated as a single script for assigning a per-script font in Settings. For example, "hira" is
    // mapped to Script::KatakanaOrHiragana instead of Script::Hiragana, since we want all Japanese scripts
    // to be rendered using the same font setting.
    constexpr static Array<Entry, 106> ScriptNameCodeList {{
      {u8"arab", Script::Arabic},
      {u8"armn", Script::Armenian},
      {u8"bali", Script::Balinese},
      {u8"batk", Script::Batak},
      {u8"beng", Script::Bengali},
      {u8"blis", Script::Blissymbols},
      {u8"bopo", Script::Bopomofo},
      {u8"brah", Script::Brahmi},
      {u8"brai", Script::Braille},
      {u8"bugi", Script::Buginese},
      {u8"buhd", Script::Buhid},
      {u8"cans", Script::CanadianAboriginal},
      {u8"cham", Script::Cham},
      {u8"cher", Script::Cherokee},
      {u8"cirt", Script::Cirth},
      {u8"copt", Script::Coptic},
      {u8"cprt", Script::Cypriot},
      {u8"cyrl", Script::Cyrillic},
      {u8"cyrs", Script::OldChurchSlavonicCyrillic},
      {u8"deva", Script::Devanagari},
      {u8"dsrt", Script::Deseret},
      {u8"egyd", Script::DemoticEgyptian},
      {u8"egyh", Script::HieraticEgyptian},
      {u8"egyp", Script::EgyptianHieroglyphs},
      {u8"ethi", Script::Ethiopic},
      {u8"geok", Script::Khutsuri},
      {u8"geor", Script::Georgian},
      {u8"glag", Script::Glagolitic},
      {u8"goth", Script::Gothic},
      {u8"grek", Script::Greek},
      {u8"gujr", Script::Gujarati},
      {u8"guru", Script::Gurmukhi},
      {u8"hang", Script::Hangul},
      {u8"hani", Script::Han},
      {u8"hano", Script::Hanunoo},
      {u8"hans", Script::SimplifiedHan},
      {u8"hant", Script::TraditionalHan},
      {u8"hebr", Script::Hebrew},
      {u8"hira", Script::KatakanaOrHiragana},
      {u8"hmng", Script::PahawhHmong},
      {u8"hrkt", Script::KatakanaOrHiragana},
      {u8"hung", Script::OldHungarian},
      {u8"inds", Script::HarappanIndus},
      {u8"ital", Script::OldItalic},
      {u8"java", Script::Javanese},
      {u8"jpan", Script::KatakanaOrHiragana},
      {u8"kali", Script::KayahLi},
      {u8"kana", Script::KatakanaOrHiragana},
      {u8"khar", Script::Kharoshthi},
      {u8"khmr", Script::Khmer},
      {u8"knda", Script::Kannada},
      {u8"kore", Script::Hangul},
      {u8"laoo", Script::Lao},
      {u8"latf", Script::LatinFraktur},
      {u8"latg", Script::LatinGaelic},
      {u8"latn", Script::Latin},
      {u8"lepc", Script::Lepcha},
      {u8"limb", Script::Limbu},
      {u8"lina", Script::LinearA},
      {u8"linb", Script::LinearB},
      {u8"mand", Script::Mandaean},
      {u8"maya", Script::MayanHieroglyphs},
      {u8"mero", Script::Meroitic},
      {u8"mlym", Script::Malayalam},
      {u8"mong", Script::Mongolian},
      {u8"mymr", Script::Myanmar},
      {u8"nkoo", Script::Nko},
      {u8"ogam", Script::Ogham},
      {u8"orkh", Script::Orkhon},
      {u8"orya", Script::Oriya},
      {u8"osma", Script::Osmanya},
      {u8"perm", Script::OldPermic},
      {u8"phag", Script::PhagsPa},
      {u8"phnx", Script::Phoenician},
      {u8"plrd", Script::PhoneticPollard},
      {u8"qaai", Script::Inherited},
      {u8"roro", Script::Rongorongo},
      {u8"runr", Script::Runic},
      {u8"sara", Script::Sarati},
      {u8"shaw", Script::Shavian},
      {u8"sinh", Script::Sinhala},
      {u8"sylo", Script::SylotiNagri},
      {u8"syrc", Script::Syriac},
      {u8"syre", Script::EstrangeloSyriac},
      {u8"syrj", Script::WesternSyriac},
      {u8"syrn", Script::EasternSyriac},
      {u8"tagb", Script::Tagbanwa},
      {u8"tale", Script::TaiLe},
      {u8"talu", Script::NewTaiLue},
      {u8"taml", Script::Tamil},
      {u8"telu", Script::Telugu},
      {u8"teng", Script::Tengwar},
      {u8"tfng", Script::Tifinagh},
      {u8"tglg", Script::Tagalog},
      {u8"thaa", Script::Thaana},
      {u8"thai", Script::Thai},
      {u8"tibt", Script::Tibetan},
      {u8"ugar", Script::Ugaritic},
      {u8"vaii", Script::Vai},
      {u8"visp", Script::VisibleSpeech},
      {u8"xpeo", Script::OldPersian},
      {u8"xsux", Script::Cuneiform},
      {u8"yiii", Script::Yi},
      {u8"zxxx", Script::UnwrittenLanguages},
      {u8"zyyy", Script::Common},
      {u8"zzzz", Script::Unknown},
    }};
    // Script names are made of letters only.
    if (!std::all_of(scriptName.begin(), scriptName.end(),
                     [](char8 c) { return (c >= u8'a' && c <= u8'z') || (c >= u8'A' && c <= u8'Z'); }))
    {
      return Script::Invalid;
    }
    return Find(ScriptNameCodeList, scriptName, 4).value_or(Script::Invalid);
  }

  Script LocaleToScriptCode(utf8_stringview locale) noexcept
  {
    constexpr static Array<Entry, 198> LocaleScriptList {{
      {u8"aa", Script::Latin},
      {u8"ab", Script::Cyrillic},
      {u8"ady", Script::Cyrillic},
      {u8"af", Script::Latin},
      {u8"ak", Script::Latin},
      {u8"am", Script::Ethiopic},
      {u8"ar", Script::Arabic},
      {u8"as", Script::Bengali},
      {u8"ast", Script::Latin},
      {u8"av", Script::Cyrillic},
      {u8"ay", Script::Latin},
      {u8"az", Script::Latin},
      {u8"ba", Script::Cyrillic},
      {u8"be", Script::Cyrillic},
      {u8"bg", Script::Cyrillic},
      {u8"bi", Script::Latin},
      {u8"bn", Script::Bengali},
      {u8"bo", Script::Tibetan},
      {u8"bs", Script::Latin},
      {u8"ca", Script::Latin},
      {u8"ce", Script::Cyrillic},
      {u8"ceb", Script::Latin},
      {u8"ch", Script::Latin},
      {u8"chk", Script::Latin},
      {u8"cs", Script::Latin},
      {u8"cy", Script::Latin},
      {u8"da", Script::Latin},
      {u8"de", Script::Latin},
      {u8"dv", Script::Thaana},
      {u8"dz", Script::Tibetan},
      {u8"ee", Script::Latin},
      {u8"efi", Script::Latin},
      {u8"el", Script::Greek},
      {u8"en", Script::Latin},
      {u8"es", Script::Latin},
      {u8"et", Script::Latin},
      {u8"eu", Script::Latin},
      {u8"fa", Script::Arabic},
      {u8"fi", Script::Latin},
      {u8"fil", Script::Latin},
      {u8"fj", Script::Latin},
      {u8"fo", Script::Latin},
      {u8"fr", Script::Latin},
      {u8"fur", Script::Latin},
      {u8"fy", Script::Latin},
      {u8"ga", Script::Latin},
      {u8"gaa", Script::Latin},
      {u8"gd", Script::Latin},
      {u8"gil", Script::Latin},
      {u8"gl", Script::Latin},
      {u8"gn", Script::Latin},
      {u8"gsw", Script::Latin},
      {u8"gu", Script::Gujarati},
      {u8"ha", Script::Latin},
      {u8"haw", Script::Latin},
      {u8"he", Script::Hebrew},
      {u8"hi", Script::Devanagari},
      {u8"hil", Script::Latin},
      {u8"ho", Script::Latin},
      {u8"hr", Script::Latin},
      {u8"ht", Script::Latin},
      {u8"hu", Script::Latin},
      {u8"hy", Script::Armenian},
      {u8"id", Script::Latin},
      {u8"ig", Script::Latin},
      {u8"ii", Script::Yi},
      {u8"ilo", Script::Latin},
      {u8"inh", Script::Cyrillic},
      {u8"is", Script::Latin},
      {u8"it", Script::Latin},
      {u8"iu", Script::CanadianAboriginal},
      {u8"ja", Script::KatakanaOrHiragana},
      {u8"jv", Script::Latin},
      {u8"ka", Script::Georgian},
      {u8"kaj", Script::Latin},
      {u8"kam", Script::Latin},
      {u8"kbd", Script::Cyrillic},
      {u8"kha", Script::Latin},
      {u8"kk", Script::Cyrillic},
      {u8"kl", Script::Latin},
      {u8"km", Script::Khmer},
      {u8"kn", Script::Kannada},
      {u8"ko", Script::Hangul},
      {u8"kok", Script::Devanagari},
      {u8"kos", Script::Latin},
      {u8"kpe", Script::Latin},
      {u8"krc", Script::Cyrillic},
      {u8"ks", Script::Arabic},
      {u8"ku", Script::Arabic},
      {u8"kum", Script::Cyrillic},
      {u8"ky", Script::Cyrillic},
      {u8"la", Script::Latin},
      {u8"lah", Script::Arabic},
      {u8"lb", Script::Latin},
      {u8"lez", Script::Cyrillic},
      {u8"ln", Script::Latin},
      {u8"lo", Script::Lao},
      {u8"lt", Script::Latin},
      {u8"lv", Script::Latin},
      {u8"mai", Script::Devanagari},
      {u8"mdf", Script::Cyrillic},
      {u8"mg", Script::Latin},
      {u8"mh", Script::Latin},
      {u8"mi", Script::Latin},
      {u8"mk", Script::Cyrillic},
      {u8"ml", Script::Malayalam},
      {u8"mn", Script::Cyrillic},
      {u8"mr", Script::Devanagari},
      {u8"ms", Script::Latin},
      {u8"mt", Script::Latin},
      {u8"my", Script::Myanmar},
      {u8"myv", Script::Cyrillic},
      {u8"na", Script::Latin},
      {u8"nb", Script::Latin},
      {u8"ne", Script::Devanagari},
      {u8"niu", Script::Latin},
      {u8"nl", Script::Latin},
      {u8"nn", Script::Latin},
      {u8"nr", Script::Latin},
      {u8"nso", Script::Latin},
      {u8"ny", Script::Latin},
      {u8"oc", Script::Latin},
      {u8"om", Script::Latin},
      {u8"or", Script::Oriya},
      {u8"os", Script::Cyrillic},
      {u8"pa", Script::Gurmukhi},
      {u8"pag", Script::Latin},
      {u8"pap", Script::Latin},
      {u8"pau", Script::Latin},
      {u8"pl", Script::Latin},
      {u8"pon", Script::Latin},
      {u8"ps", Script::Arabic},
      {u8"pt", Script::Latin},
      {u8"qu", Script::Latin},
      {u8"rm", Script::Latin},
      {u8"rn", Script::Latin},
      {u8"ro", Script::Latin},
      {u8"ru", Script::Cyrillic},
      {u8"rw", Script::Latin},
      {u8"sa", Script::Devanagari},
      {u8"sah", Script::Cyrillic},
      {u8"sat", Script::Latin},
      {u8"sd", Script::Arabic},
      {u8"se", Script::Latin},
      {u8"sg", Script::Latin},
      {u8"si", Script::Sinhala},
      {u8"sid", Script::Latin},
      {u8"sk", Script::Latin},
      {u8"sl", Script::Latin},
      {u8"sm", Script::Latin},
      {u8"so", Script::Latin},
      {u8"sq", Script::Latin},
      {u8"sr", Script::Cyrillic},
      {u8"ss", Script::Latin},
      {u8"st", Script::Latin},
      {u8"su", Script::Latin},
      {u8"sv", Script::Latin},
      {u8"sw", Script::Latin},
      {u8"ta", Script::Tamil},
      {u8"te", Script::Telugu},
      {u8"tet", Script::Latin},
      {u8"tg", Script::Cyrillic},
      {u8"th", Script::Thai},
      {u8"ti", Script::Ethiopic},
      {u8"tig", Script::Ethiopic},
      {u8"tk", Script::Latin},
      {u8"tkl", Script::Latin},
      {u8"tl", Script::Latin},
      {u8"tn", Script::Latin},
      {u8"to", Script::Latin},
      {u8"tpi", Script::Latin},
      {u8"tr", Script::Latin},
      {u8"trv", Script::Latin},
      {u8"ts", Script::Latin},
      {u8"tt", Script::Cyrillic},
      {u8"tvl", Script::Latin},
      {u8"tw", Script::Latin},
      {u8"ty", Script::Latin},
      {u8"tyv", Script::Cyrillic},
      {u8"udm", Script::Cyrillic},
      {u8"ug", Script::Arabic},
      {u8"uk", Script::Cyrillic},
      {u8"und", Script::Latin},
      {u8"ur", Script::Arabic},
      {u8"uz", Script::Cyrillic},
      {u8"ve", Script::Latin},
      {u8"vi", Script::Latin},
      {u8"wal", Script::Ethiopic},
      {u8"war", Script::Latin},
      {u8"wo", Script::Latin},
      {u8"xh", Script::Latin},
      {u8"yap", Script::Latin},
      {u8"yo", Script::Latin},
      {u8"za", Script::Latin},
      {u8"zh", Script::Han},
      {u8"zh_hk", Script::TraditionalHan},
      {u8"zh_tw", Script::TraditionalHan},
      {u8"zu", Script::Latin},
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
      Script code = ScriptNameToCode(canonicalLocale.substr(underscorePosition + 1));
      if (code != Script::Invalid && code != Script::Unknown)
      {
        return code;
      }
      canonicalLocale = canonicalLocale.substr(0, underscorePosition);
    }
    return Script::Common;
  }
}
