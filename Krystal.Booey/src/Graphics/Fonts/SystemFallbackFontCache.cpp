/*
 * Copyright (C) 2005-2023 Apple Inc. All rights reserved.
 * Copyright (C) 2006 Alexey Proskuryakov
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1.  Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 * 2.  Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 * 3.  Neither the name of Apple Inc. ("Apple") nor the names of
 *     its contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE AND ITS CONTRIBUTORS "AS IS" AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL APPLE OR ITS CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "Krystal.Booey/Graphics/Fonts/SystemFallbackFontCache.hpp"
#include "Krystal.Booey/Graphics/Fonts/Font.hpp"
#include "Krystal.Booey/Graphics/Fonts/FontCache.hpp"
#include "Krystal.Booey/Graphics/Fonts/FontCascade.hpp"
#include "Krystal.Booey/Graphics/Fonts/FontDescription.hpp"
#include "Krystal.Core/Hash.hpp"
#include "Krystal.Core/Text/CodePointIterator.hpp"
#include "Krystal.Core/Text/Unicode.hpp"
#include <unicode/utf8.h>

namespace krys::boo
{
  size_t CharacterFallbackMapKeyHash::operator()(const CharacterFallbackMapKey &key) const noexcept
  {
    return Hash::Combine(std::hash<utf8_stringview> {}(key.Locale.View()),
                         std::hash<utf8_string> {}(key.String), key.IsForPlatformFont,
                         static_cast<uint8>(key.ResolvedEmojiPolicy));
  }

  SystemFallbackFontCache &SystemFallbackFontCache::Singleton() noexcept
  {
    return FontCache::Singleton().GetSystemFallbackFontCache();
  }

  static void AppendCodePoint(utf8_string &string, char32 codePoint) noexcept
  {
    char8 buffer[U8_MAX_LENGTH];
    int32_t length = 0;
    U8_APPEND_UNSAFE(buffer, length, codePoint);
    string.append(buffer, static_cast<size_t>(length));
  }

  RefPtr<Font> SystemFallbackFontCache::SystemFallbackFontForCharacterCluster(
    const Font *font, utf8_stringview characterCluster, const FontDescription &description,
    boo::ResolvedEmojiPolicy resolvedEmojiPolicy, boo::IsForPlatformFont isForPlatformFont) noexcept
  {
    auto &characterMap = _characterFallbackMaps[font];

    auto key = CharacterFallbackMapKey {description.ComputedLocale(), utf8_string {characterCluster},
                                        isForPlatformFont != IsForPlatformFont::No, resolvedEmojiPolicy};
    if (auto it = characterMap.find(key); it != characterMap.end())
    {
      return ShareRefPtr(it->second);
    }

    utf8_string normalized;
    for (text::CodePointIterator it {characterCluster}; !it.AtEnd(); ++it)
    {
      AppendCodePoint(normalized, FontCascade::NormalizeSpaces(*it));
    }

    // FIXME: Is this the right place to add the variation selectors?
    // Should this be done in platform-specific code instead?
    // The fact that Core Text accepts this information in the form of variation selectors
    // seems like a platform-specific quirk.
    // However, if we do this later in platform-specific code, we'd have to reallocate
    // the array and copy its contents, which seems wasteful.
    switch (resolvedEmojiPolicy)
    {
      case ResolvedEmojiPolicy::NoPreference: break;
      case ResolvedEmojiPolicy::RequireText:  AppendCodePoint(normalized, text::TextVariationSelector); break;
      case ResolvedEmojiPolicy::RequireEmoji:
        AppendCodePoint(normalized, text::EmojiVariationSelector);
        break;
    }

    RefPtr<Font> fallbackFont = FontCache::Singleton().SystemFallbackForCharacterCluster(
      description, *font, isForPlatformFont, FontCache::PreferColoredFont::No, normalized);
    if (fallbackFont)
    {
      fallbackFont->SetIsUsedInSystemFallbackFontCache();
    }
    characterMap.emplace(krys::move(key), fallbackFont.get());
    return fallbackFont;
  }

  void SystemFallbackFontCache::Remove(Font *font) noexcept
  {
    _characterFallbackMaps.erase(font);

    if (!font->IsUsedInSystemFallbackFontCache())
    {
      return;
    }

    for (auto &[owner, characterMap] : _characterFallbackMaps)
    {
      std::erase_if(characterMap, [&](auto &entry) { return entry.second == font; });
    }
  }
}
