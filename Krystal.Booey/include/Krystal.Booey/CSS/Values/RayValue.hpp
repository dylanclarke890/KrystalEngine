#pragma once

#include "Krystal.Booey/CSS/Values/Motion/RayFunction.hpp"
#include "Krystal.Booey/CSS/Values/Value.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyleConstants.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::css
{
  /// @brief The value of a `ray()` function, as used in `offset-path`.
  /// @note https://drafts.fxtf.org/motion-1/#funcdef-offset-path-ray
  class RayValue final : public Value
  {
  private:
    RayFunction _ray;
    CSSBoxType _coordinateBox;

    RayValue(RayFunction &&ray, CSSBoxType coordinateBox) noexcept;

  public:
    KRYS_NODISCARD static Ref<RayValue> Create(RayFunction ray,
                                               CSSBoxType coordinateBox = CSSBoxType::BoxMissing) noexcept;

    KRYS_NODISCARD const RayFunction &GetRay() const noexcept
    {
      return _ray;
    }

    KRYS_NODISCARD CSSBoxType CoordinateBox() const noexcept
    {
      return _coordinateBox;
    }

    KRYS_NODISCARD CSSOMString CustomCSSText(const SerialisationContext &context) const noexcept;
    KRYS_NODISCARD bool Equals(const RayValue &other) const noexcept;
    KRYS_NODISCARD IterationStatus
      CustomVisitChildren(KRYS_NOESCAPE const Func<IterationStatus(Value &)> &func) const noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::css::RayValue)
  KRYS_NODISCARD static bool IsType(const krys::boo::css::Value &value) noexcept
  {
    return value.IsRayValue();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
