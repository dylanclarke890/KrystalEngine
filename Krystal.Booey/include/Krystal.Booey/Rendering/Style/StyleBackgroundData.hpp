#pragma once

#include "Krystal.Booey/Rendering/Style/OutlineValue.hpp"
#include "Krystal.Booey/Style/Values/Backgrounds/BackgroundLayers.hpp"
#include "Krystal.Booey/Style/Values/Color/Color.hpp"
#include "Krystal.Core/RefCounted.hpp"
#include "Krystal.Core/Types/DataRef.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo
{
  class StyleBackgroundData : public RefCounted<StyleBackgroundData>
  {
  public:
    KRYS_NODISCARD static Ref<StyleBackgroundData> Create() noexcept
    {
      return AdoptRef(*new StyleBackgroundData);
    }
    KRYS_NODISCARD Ref<StyleBackgroundData> Copy() const noexcept;

    bool operator==(const StyleBackgroundData &) const noexcept;

    style::BackgroundLayers Background;
    style::Color BackgroundColor;
    OutlineValue Outline;

    KRYS_NODISCARD bool ContainsCurrentColor() const noexcept;

  private:
    StyleBackgroundData() noexcept;
    StyleBackgroundData(const StyleBackgroundData &) noexcept;
  };
}
