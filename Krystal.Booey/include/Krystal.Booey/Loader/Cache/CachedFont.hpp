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
