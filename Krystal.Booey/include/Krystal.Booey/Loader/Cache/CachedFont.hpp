/*
 * Copyright (C) 2007-2021 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include "Krystal.Booey/CSS/Types/CSSOMString.hpp"
#include "Krystal.Booey/Loader/Cache/CachedResource.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo
{
  class Font;
  class FontCreationContext;
  class FontDescription;
  class FontPlatformData;
  struct FontCustomPlatformData;

  /// @brief A cached web font: the font file a load brought, and the font made from it.
  /// @note The file isn't loaded when the font is requested, but when something first needs it
  /// (`BeginLoadIfNeeded`).
  // FIXME: SVG fonts, and a safe font parser for fonts that aren't trusted.
  class CachedFont : public CachedResource
  {
  private:
    bool _loadInitiated {false};
    bool _hasCreatedFontDataWrappingResource {false};
    RefPtr<FontCustomPlatformData> _fontCustomPlatformData;

  public:
    explicit CachedFont(CachedResourceRequest &&request, Type type = Type::FontResource) noexcept;
    ~CachedFont() noexcept override;

    void BeginLoadIfNeeded(CachedResourceLoader &loader) noexcept;
    KRYS_NODISCARD bool StillNeedsLoad() const noexcept override
    {
      return !_loadInitiated;
    }

    /// @brief Makes the font from the loaded file, if it hasn't been made yet.
    /// @return Whether there's a font.
    virtual bool EnsureCustomFontData() noexcept;
    /// @brief The font in @p bytes (a TrueType, OpenType, WOFF or WOFF 2 file), or null if it isn't one.
    /// @param wrapping Set to whether the font may use @p bytes rather than a copy of them.
    KRYS_NODISCARD static RefPtr<FontCustomPlatformData>
      CreateCustomFontData(SharedBuffer &bytes, const css::CSSOMString &itemInCollection,
                           bool &wrapping) noexcept;
    KRYS_NODISCARD static FontPlatformData
      PlatformDataFromCustomData(FontCustomPlatformData &fontCustomPlatformData,
                                 const FontDescription &fontDescription, bool bold, bool italic,
                                 const FontCreationContext &fontCreationContext) noexcept;

    KRYS_NODISCARD virtual RefPtr<Font> CreateFont(const FontDescription &fontDescription, bool syntheticBold,
                                                   bool syntheticItalic,
                                                   const FontCreationContext &fontCreationContext) noexcept;

  protected:
    KRYS_NODISCARD FontPlatformData
      PlatformDataFromCustomData(const FontDescription &fontDescription, bool bold, bool italic,
                                 const FontCreationContext &fontCreationContext) noexcept;
    bool EnsureCustomFontData(SharedBuffer *data) noexcept;

  private:
    KRYS_NODISCARD css::CSSOMString CalculateItemInCollection() const noexcept;

    void CheckNotify() noexcept override;

    void Load(CachedResourceLoader &loader) noexcept override;

    void DidAddClient(CachedResourceClient &client) noexcept override;
    void FinishLoading(const SharedBuffer *data) noexcept override;

    void AllClientsRemoved() noexcept override;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::CachedFont)
  KRYS_NODISCARD static bool IsType(const krys::boo::CachedResource &resource) noexcept
  {
    return resource.GetType() == krys::boo::CachedResource::Type::FontResource
           || resource.GetType() == krys::boo::CachedResource::Type::SVGFontResource;
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
