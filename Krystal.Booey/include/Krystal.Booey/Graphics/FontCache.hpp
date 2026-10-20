#pragma once

#include "Krystal.Booey/Graphics/Font.hpp"
#include "Krystal.Booey/Graphics/FontCascadeCache.hpp"
#include "Krystal.Booey/Graphics/FontPlatformData.hpp"
#include "Krystal.Booey/Graphics/SystemFallbackFontCache.hpp"
#include "Krystal.Booey/Platform/Timer.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Enum.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Types/EnumSet.hpp"
#include "Krystal.Core/Types/HashMap.hpp"
#include "Krystal.Core/Types/HashSet.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include "Krystal.Core/Types/String.hpp"
#include "Krystal.Core/Types/StringAtom.hpp"
#include "Krystal.Core/Types/UniquePtr.hpp"
#include "Krystal.Core/Types/WeakPtr.hpp"

namespace krys::boo
{
  class FontCascade;
  class FontDescription;
  class FontSelector;

  enum class FontLookupOptions : uint8
  {
    ExactFamilyNameMatch,
    DisallowBoldSynthesis,
    DisallowObliqueSynthesis,
  };
}

KRYS_DEFINE_CONTIGUOUS_ENUM_TRAITS(krys::boo::FontLookupOptions, 3uz);

namespace krys::boo
{
  /// @brief Hashes `FontPlatformData`, for the font cache's maps.
  struct FontPlatformDataHash
  {
    KRYS_NODISCARD size_t operator()(const FontPlatformData &data) const noexcept
    {
      return data.Hash();
    }
  };

  /// @brief Finds fonts: a family's face for a description (in the font database), the fonts made from faces,
  /// the fonts of font cascades, and fallback fonts for characters no family has.
  class FontCache final
  {
    KRYS_NON_COPY_MOVABLE(FontCache);

  public:
    enum class PreferColoredFont : bool
    {
      No,
      Yes
    };

  private:
    struct FontPlatformDataCacheKey
    {
      FontDescriptionKey DescriptionKey;
      FontFamilyName Family;

      KRYS_NODISCARD bool operator==(const FontPlatformDataCacheKey &) const noexcept = default;
    };

    struct FontPlatformDataCacheKeyHash
    {
      KRYS_NODISCARD size_t operator()(const FontPlatformDataCacheKey &key) const noexcept;
    };

    HashMap<FontPlatformDataCacheKey, UniquePtr<FontPlatformData>, FontPlatformDataCacheKeyHash>
      _platformData;
    HashMap<FontPlatformData, Ref<Font>, FontPlatformDataHash> _data;
    FontCascadeCache _fontCascadeCache;
    SystemFallbackFontCache _systemFallbackFontCache;
    List<WeakPtr<FontSelector>> _clients;
    HashSet<utf8_string> _familiesUsingBackslashAsYenSign;
    uint16 _generation {0};
    Timer _purgeTimer;

  public:
    FontCache() noexcept;
    ~FontCache() noexcept;

    KRYS_NODISCARD static FontCache &Singleton() noexcept;

    /// @brief The font of @p family for @p description, or null if there's no such family.
    KRYS_NODISCARD RefPtr<Font> FontForFamily(const FontDescription &description, const StringAtom &family,
                                              EnumSet<FontLookupOptions> options = {}) noexcept;

    /// @brief The font used when no family has a font: the bundled Liberation Sans.
    KRYS_NODISCARD Ref<Font> LastResortFallbackFont(const FontDescription &description) noexcept;

    KRYS_NODISCARD Ref<Font> FontForPlatformData(const FontPlatformData &platformData) noexcept;

    KRYS_NODISCARD RefPtr<Font> SimilarFont(const FontDescription &description,
                                            const StringAtom &family) noexcept;

    /// @brief A registered font that has every character of @p characterCluster (UTF-8), the best match for
    /// @p description, or null if none has.
    KRYS_NODISCARD RefPtr<Font> SystemFallbackForCharacterCluster(const FontDescription &description,
                                                                  const Font &originalFontData,
                                                                  IsForPlatformFont isForPlatformFont,
                                                                  PreferColoredFont preferColoredFont,
                                                                  utf8_stringview characterCluster) noexcept;

    /// @brief The names of every registered family.
    KRYS_NODISCARD List<utf8_string> SystemFontFamilies() const noexcept;

    void AddClient(FontSelector &client) noexcept;
    void RemoveClient(FontSelector &client) noexcept;

    KRYS_NODISCARD uint16 Generation() const noexcept
    {
      return _generation;
    }

    /// @brief Forgets every font found, and tells the font selectors (after fonts were registered).
    static void InvalidateAllFontCaches() noexcept;

    KRYS_NODISCARD size_t FontCount() const noexcept;
    KRYS_NODISCARD size_t InactiveFontCount() const noexcept;
    void PurgeInactiveFontData(uint32 count = UINT32_MAX) noexcept;
    /// @brief Purges inactive fonts once there are more than a limit, down to a target.
    void PurgeInactiveFontDataIfNeeded() noexcept;
    /// @brief Purges the inactive fonts and clears the width caches.
    void ReleaseNoncriticalMemory() noexcept;

    void UpdateFontCascade(const FontCascade &fontCascade) noexcept;

    KRYS_NODISCARD FontCascadeCache &GetFontCascadeCache() noexcept
    {
      return _fontCascadeCache;
    }

    KRYS_NODISCARD boo::SystemFallbackFontCache &GetSystemFallbackFontCache() noexcept
    {
      return _systemFallbackFontCache;
    }

    KRYS_NODISCARD bool UseBackslashAsYenSignForFamily(const StringAtom &family) noexcept;

    void Invalidate() noexcept;

  private:
    KRYS_NODISCARD FontPlatformData *CachedFontPlatformData(const FontDescription &description,
                                                            const StringAtom &family,
                                                            EnumSet<FontLookupOptions> options) noexcept;
    KRYS_NODISCARD UniquePtr<FontPlatformData>
      CreateFontPlatformData(const FontDescription &description, const StringAtom &family,
                             EnumSet<FontLookupOptions> options) noexcept;

    KRYS_NODISCARD static utf8_stringview AlternateFamilyName(utf8_stringview familyName) noexcept;
  };
}
