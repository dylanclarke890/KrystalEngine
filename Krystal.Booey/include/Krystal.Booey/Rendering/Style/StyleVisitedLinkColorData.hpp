#pragma once

#include "Krystal.Booey/Core/RectEdges.hpp"
#include "Krystal.Booey/Style/Values/Color/Color.hpp"

namespace krys::boo
{
  class StyleVisitedLinkColorData : public RefCounted<StyleVisitedLinkColorData>
  {
  public:
    KRYS_NODISCARD static Ref<StyleVisitedLinkColorData> Create() noexcept
    {
      return AdoptRef(*new StyleVisitedLinkColorData);
    }
    KRYS_NODISCARD Ref<StyleVisitedLinkColorData> Copy() const noexcept;
    ~StyleVisitedLinkColorData() noexcept;

    bool operator==(const StyleVisitedLinkColorData &) const noexcept;

    style::Color VisitedLinkBackgroundColor;
    RectEdges<style::Color> VisitedLinkBorderColors;
    style::Color VisitedLinkTextDecorationColor;
    style::Color VisitedLinkOutlineColor;

  private:
    StyleVisitedLinkColorData() noexcept;
    StyleVisitedLinkColorData(const StyleVisitedLinkColorData &) noexcept;
  };
}
