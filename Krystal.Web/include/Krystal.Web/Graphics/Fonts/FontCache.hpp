/*
 * Copyright (C) 2006-2025 Apple Inc. All rights reserved.
 * Copyright (C) 2007-2008 Torch Mobile, Inc.
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

#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Containers/EnumSet.hpp"
#include "Krystal.Core/Containers/HashMap.hpp"
#include "Krystal.Core/Containers/HashSet.hpp"
#include "Krystal.Core/Containers/List.hpp"
#include "Krystal.Core/Enum.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Memory/RefPtr.hpp"
#include "Krystal.Core/Memory/UniquePtr.hpp"
#include "Krystal.Core/Memory/WeakPtr.hpp"
#include "Krystal.Core/Strings/String.hpp"
#include "Krystal.Core/Strings/StringAtom.hpp"
#include "Krystal.Core/Time/Timer.hpp"
#include "Krystal.Web/Graphics/Fonts/Font.hpp"
#include "Krystal.Web/Graphics/Fonts/FontCascadeCache.hpp"
#include "Krystal.Web/Graphics/Fonts/FontCreationContext.hpp"
#include "Krystal.Web/Graphics/Fonts/FontPlatformData.hpp"
#include "Krystal.Web/Graphics/Fonts/SystemFallbackFontCache.hpp"

namespace krys::web
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

  consteval FontLookupOptions MaxEnumValue(FontLookupOptions) noexcept
  {
    return FontLookupOptions::DisallowObliqueSynthesis;
  }

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
      Yes,
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

    /// @brief The font of @p family for @p description, or null if there's no such family. @p context gives
    /// the family's `@font-feature-values` and `@font-palette-values`.
    KRYS_NODISCARD RefPtr<Font> FontForFamily(const FontDescription &description, const StringAtom &family,
                                              EnumSet<FontLookupOptions> options = {},
                                              const FontCreationContext &context = {}) noexcept;

    /// @brief The font used when no family has a font: the bundled Liberation Sans.
    KRYS_NODISCARD Ref<Font> LastResortFallbackFont(const FontDescription &description) noexcept;

    KRYS_NODISCARD Ref<Font> FontForPlatformData(const FontPlatformData &platformData) noexcept;

    KRYS_NODISCARD static RefPtr<Font> SimilarFont(const FontDescription &description,
                                                   const StringAtom &family) noexcept;

    /// @brief A registered font that has every character of @p characterCluster (UTF-8), the best match for
    /// @p description, or null if none has.
    KRYS_NODISCARD RefPtr<Font> SystemFallbackForCharacterCluster(const FontDescription &description,
                                                                  const Font &originalFontData,
                                                                  IsForPlatformFont isForPlatformFont,
                                                                  PreferColoredFont preferColoredFont,
                                                                  utf8_stringview characterCluster) noexcept;

    /// @brief The names of every registered family.
    KRYS_NODISCARD static List<utf8_string> SystemFontFamilies() noexcept;

    /// @brief The capabilities of each registered face of @p familyName; empty if there's no such family.
    KRYS_NODISCARD static List<FontSelectionCapabilities>
      GetFontSelectionCapabilitiesInFamily(const StringAtom &familyName,
                                           AllowUserInstalledFonts allowUserInstalledFonts) noexcept;

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

    KRYS_NODISCARD web::SystemFallbackFontCache &GetSystemFallbackFontCache() noexcept
    {
      return _systemFallbackFontCache;
    }

    KRYS_NODISCARD bool UseBackslashAsYenSignForFamily(const StringAtom &family) noexcept;

    void Invalidate() noexcept;

    /// @brief Whether editing must not write @p fontFamily, a system font's private name, into the content.
    KRYS_NODISCARD static bool IsSystemFontForbiddenForEditing(utf8_stringview fontFamily) noexcept
    {
      (void)fontFamily;
      return false;
    }

  private:
    KRYS_NODISCARD FontPlatformData *CachedFontPlatformData(const FontDescription &description,
                                                            const StringAtom &family,
                                                            EnumSet<FontLookupOptions> options) noexcept;

    KRYS_NODISCARD static UniquePtr<FontPlatformData>
      CreateFontPlatformData(const FontDescription &description, const StringAtom &family,
                             EnumSet<FontLookupOptions> options) noexcept;

    KRYS_NODISCARD static utf8_stringview AlternateFamilyName(utf8_stringview familyName) noexcept;
  };
}
