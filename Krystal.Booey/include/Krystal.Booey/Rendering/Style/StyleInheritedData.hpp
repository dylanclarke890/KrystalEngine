#pragma once

#include "Krystal.Booey/Graphics/FontCascade.hpp"
#include "Krystal.Booey/Rendering/Style/StyleFontData.hpp"
#include "Krystal.Booey/Style/Values/Color/Color.hpp"
#include "Krystal.Booey/Style/Values/Inline/LineHeight.hpp"
#include "Krystal.Booey/Style/Values/NonStandard/WebKitBorderSpacing.hpp"
#include "Krystal.Core/Types/DataRef.hpp"

namespace krys::boo
{
  class StyleInheritedData : public RefCounted<StyleInheritedData>
  {
  public:
    KRYS_NODISCARD static Ref<StyleInheritedData> Create() noexcept
    {
      return AdoptRef(*new StyleInheritedData);
    }
    KRYS_NODISCARD Ref<StyleInheritedData> Copy() const noexcept;

    bool operator==(const StyleInheritedData &) const noexcept;

    KRYS_NODISCARD bool FastPathInheritedEqual(const StyleInheritedData &) const noexcept;
    KRYS_NODISCARD bool NonFastPathInheritedEqual(const StyleInheritedData &) const noexcept;
    void FastPathInheritFrom(const StyleInheritedData &) noexcept;

    style::WebkitBorderSpacing BorderHorizontalSpacing;
    style::WebkitBorderSpacing BorderVerticalSpacing;

    style::LineHeight LineHeight;

    DataRef<StyleFontData> FontData;
    boo::Color Color;
    boo::Color VisitedLinkColor;

  private:
    StyleInheritedData() noexcept;
    StyleInheritedData(const StyleInheritedData &) noexcept;
    void operator=(const StyleInheritedData &) noexcept = delete;
  };
}
