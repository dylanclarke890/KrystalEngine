#include "Krystal.Booey/Rendering/Style/StyleBackgroundData.hpp"

#include "Krystal.Booey/Style/Computed/ComputedStyle+InitialInlines.hpp"

namespace krys::boo
{

  StyleBackgroundData::StyleBackgroundData() noexcept
      : Background(css::keywords::None {}), BackgroundColor(style::ComputedStyle::InitialBackgroundColor())
  {
  }

  inline StyleBackgroundData::StyleBackgroundData(const StyleBackgroundData &other) noexcept
      : RefCounted<StyleBackgroundData>(), Background(other.Background),
        BackgroundColor(other.BackgroundColor), Outline(other.Outline)
  {
  }

  Ref<StyleBackgroundData> StyleBackgroundData::Copy() const noexcept
  {
    return AdoptRef(*new StyleBackgroundData(*this));
  }

  bool StyleBackgroundData::operator==(const StyleBackgroundData &other) const noexcept
  {
    return Background == other.Background && BackgroundColor == other.BackgroundColor
           && Outline == other.Outline;
  }

  bool StyleBackgroundData::ContainsCurrentColor() const noexcept
  {
    return BackgroundColor.ContainsCurrentColor() || Outline.OutlineColor.ContainsCurrentColor();
  }
}
