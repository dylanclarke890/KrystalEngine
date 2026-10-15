#include "Krystal.Booey/Rendering/Style/BorderValue.hpp"

namespace krys::boo
{
  bool BorderValue::IsTransparent() const noexcept
  {
    return Color.IsResolvedColor() && Color.GetResolvedColor().IsValid()
           && !Color.GetResolvedColor().IsVisible();
  }

  bool BorderValue::IsVisible() const noexcept
  {
    return NonZero() && !IsTransparent() && static_cast<BorderStyle>(Style) != BorderStyle::Hidden;
  }
}
