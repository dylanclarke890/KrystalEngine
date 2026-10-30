#include "Krystal.Booey/Rendering/Style/StyleTransformData.hpp"

#include "Krystal.Booey/Style/Computed/ComputedStyle+InitialInlines.hpp"

namespace krys::boo
{

  StyleTransformData::StyleTransformData() noexcept
      : Transform(style::ComputedStyle::InitialTransform()),
        Origin({style::ComputedStyle::InitialTransformOriginX(),
                style::ComputedStyle::InitialTransformOriginY(),
                style::ComputedStyle::InitialTransformOriginZ()}),
        TransformBox(static_cast<unsigned>(style::ComputedStyle::InitialTransformBox()))
  {
  }

  inline StyleTransformData::StyleTransformData(const StyleTransformData &other) noexcept
      : RefCounted<StyleTransformData>(), Transform(other.Transform), Origin(other.Origin),
        TransformBox(other.TransformBox)
  {
  }

  Ref<StyleTransformData> StyleTransformData::Copy() const noexcept
  {
    return AdoptRef(*new StyleTransformData(*this));
  }

  bool StyleTransformData::operator==(const StyleTransformData &other) const noexcept
  {
    return Origin == other.Origin && TransformBox == other.TransformBox && Transform == other.Transform;
  }
}
