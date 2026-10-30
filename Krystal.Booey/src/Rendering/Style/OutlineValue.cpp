#include "Krystal.Booey/Rendering/Style/OutlineValue.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesEvaluation.hpp"

namespace krys::boo
{
  bool OutlineValue::NonZero() const noexcept
  {
    return OutlineWidth && static_cast<boo::OutlineStyle>(OutlineStyle) != boo::OutlineStyle::None;
  }

  bool OutlineValue::IsTransparent() const noexcept
  {
    return OutlineColor.IsResolvedColor() && OutlineColor.GetResolvedColor().IsValid()
           && !OutlineColor.GetResolvedColor().IsVisible();
  }

  bool OutlineValue::IsVisible() const noexcept
  {
    return NonZero() && !IsTransparent();
  }
}
