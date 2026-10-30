#include "Krystal.Booey/Graphics/SystemFallbackFontCache.hpp"
#include "Krystal.Booey/Graphics/Font.hpp"
#include "Krystal.Booey/Graphics/FontCache.hpp"
#include "Krystal.Booey/Graphics/FontCascade.hpp"
#include "Krystal.Booey/Graphics/FontDescription.hpp"
#include "Krystal.Core/Hash.hpp"
#include "Krystal.Core/Text/CodePointIterator.hpp"
#include <unicode/utf8.h>

namespace krys::boo
{
  // Variation selectors for text and emoji presentation.
  constexpr char32 TextVariationSelector = 0xFE0E;
  constexpr char32 EmojiVariationSelector = 0xFE0F;

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
      case ResolvedEmojiPolicy::RequireText:  AppendCodePoint(normalized, TextVariationSelector); break;
      case ResolvedEmojiPolicy::RequireEmoji: AppendCodePoint(normalized, EmojiVariationSelector); break;
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
