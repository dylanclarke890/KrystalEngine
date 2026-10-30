#pragma once

#include "Krystal.Booey/Rendering/Style/RenderStyleConstants.hpp"
#include "Krystal.Booey/Style/Values/Transforms/Transform.hpp"
#include "Krystal.Booey/Style/Values/Transforms/TransformOrigin.hpp"
#include "Krystal.Core/RefCounted.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo
{
  class StyleTransformData : public RefCounted<StyleTransformData>
  {
  public:
    KRYS_NODISCARD static Ref<StyleTransformData> Create() noexcept
    {
      return AdoptRef(*new StyleTransformData);
    }
    KRYS_NODISCARD Ref<StyleTransformData> Copy() const noexcept;

    bool operator==(const StyleTransformData &) const noexcept;

    KRYS_NODISCARD bool HasTransform() const noexcept
    {
      return !Transform.IsNone();
    }

    style::Transform Transform;
    style::TransformOrigin Origin;
    unsigned TransformBox : 3;

  private:
    StyleTransformData() noexcept;
    StyleTransformData(const StyleTransformData &) noexcept;
  };
}
