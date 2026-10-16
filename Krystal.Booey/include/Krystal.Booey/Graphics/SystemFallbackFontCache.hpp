#pragma once

#include "Krystal.Booey/Core/TextFlags.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Types/HashMap.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include "Krystal.Core/Types/String.hpp"
#include "Krystal.Core/Types/StringAtom.hpp"

namespace krys::boo
{
  class Font;
  class FontDescription;

  enum class IsForPlatformFont : bool;

  struct CharacterFallbackMapKey
  {
    StringAtom Locale;
    utf8_string String;
    bool IsForPlatformFont {false};
    ResolvedEmojiPolicy ResolvedEmojiPolicy {ResolvedEmojiPolicy::NoPreference};

    KRYS_NODISCARD bool operator==(const CharacterFallbackMapKey &other) const noexcept = default;
  };

  struct CharacterFallbackMapKeyHash
  {
    KRYS_NODISCARD size_t operator()(const CharacterFallbackMapKey &key) const noexcept;
  };

  /// @brief The fallback font for each character cluster a font lacks, per font.
  class SystemFallbackFontCache
  {
    KRYS_NON_COPY_MOVABLE(SystemFallbackFontCache);

  private:
    // Fonts are not ref'd to avoid cycles.
    // FIXME: Consider changing these maps to use WeakPtr instead of raw pointers.
    using CharacterFallbackMap = HashMap<CharacterFallbackMapKey, Font *, CharacterFallbackMapKeyHash>;

    HashMap<const Font *, CharacterFallbackMap> _characterFallbackMaps;

  public:
    SystemFallbackFontCache() noexcept = default;

    KRYS_NODISCARD static SystemFallbackFontCache &Singleton() noexcept;

    /// @brief The font to draw @p characterCluster (UTF-8) with when @p font lacks it, or null if no font has
    /// it.
    KRYS_NODISCARD RefPtr<Font> SystemFallbackFontForCharacterCluster(
      const Font *font, utf8_stringview characterCluster, const FontDescription &description,
      boo::ResolvedEmojiPolicy resolvedEmojiPolicy, boo::IsForPlatformFont isForPlatformFont) noexcept;

    void Remove(Font *font) noexcept;
  };
}
