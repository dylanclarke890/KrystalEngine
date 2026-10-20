#pragma once

#include "Krystal.Booey/CSS/Font/FontFace.hpp"
#include "Krystal.Booey/Graphics/FontCascadeCache.hpp"
#include "Krystal.Booey/Graphics/FontPaletteValues.hpp"
#include "Krystal.Booey/Graphics/FontRanges.hpp"
#include "Krystal.Core/Types/HashMap.hpp"
#include "Krystal.Core/Types/SmallList.hpp"

namespace krys::boo::css
{
  /// @brief The font faces of a family that match a font selection request, best first, each covering its
  /// `unicode-range`.
  class CSSSegmentedFontFace final : public RefCounted<CSSSegmentedFontFace>, public CSSFontFaceClient
  {
    KRYS_NON_COPY_MOVABLE(CSSSegmentedFontFace);

  private:
    struct CacheKey
    {
      FontDescriptionKey DescriptionKey;
      FontPaletteValues PaletteValues;

      KRYS_NODISCARD bool operator==(const CacheKey &) const noexcept = default;
    };

    struct CacheKeyHash
    {
      KRYS_NODISCARD size_t operator()(const CacheKey &key) const noexcept
      {
        return FontDescriptionKeyHash {}(key.DescriptionKey);
      }
    };

    // FIXME: Add support for font-feature-values in the key for this cache.
    HashMap<CacheKey, FontRanges, CacheKeyHash> _cache;
    SmallList<Ref<CSSFontFace>, 1> _fontFaces;

    CSSSegmentedFontFace() noexcept = default;

  public:
    KRYS_NODISCARD static Ref<CSSSegmentedFontFace> Create() noexcept
    {
      return AdoptRef(*new CSSSegmentedFontFace());
    }

    ~CSSSegmentedFontFace() noexcept override;

    void AddRef() const noexcept final
    {
      RefCounted::AddRef();
    }
    void SubRef() const noexcept final
    {
      RefCounted::SubRef();
    }
    KRYS_NODISCARD uint32 GetRefCount() const noexcept final
    {
      return RefCounted::GetRefCount();
    }

    void AppendFontFace(Ref<CSSFontFace> &&fontFace) noexcept;

    KRYS_NODISCARD FontRanges GetFontRanges(const FontDescription &fontDescription,
                                            const FontPaletteValues &fontPaletteValues,
                                            RefPtr<FontFeatureValues> fontFeatureValues) noexcept;

    KRYS_NODISCARD SmallList<Ref<CSSFontFace>, 1> &ConstituentFaces() noexcept
    {
      return _fontFaces;
    }

  private:
    void FontLoaded(CSSFontFace &fontFace) noexcept final;
  };
}
