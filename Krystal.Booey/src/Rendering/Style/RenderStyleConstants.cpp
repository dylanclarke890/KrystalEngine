#include "Krystal.Booey/Rendering/Style/RenderStyleConstants.hpp"
#include "Krystal.Core/Assert.hpp"

namespace krys::boo
{
  bool AlwaysPageBreak(BreakBetween between) noexcept
  {
    return between >= BreakBetween::Page;
  }

  CSSBoxType TransformBoxToCSSBoxType(TransformBox transformBox) noexcept
  {
    switch (transformBox)
    {
      case TransformBox::StrokeBox:  return CSSBoxType::StrokeBox;
      case TransformBox::ContentBox: return CSSBoxType::ContentBox;
      case TransformBox::BorderBox:  return CSSBoxType::BorderBox;
      case TransformBox::FillBox:    return CSSBoxType::FillBox;
      case TransformBox::ViewBox:    return CSSBoxType::ViewBox;
      default:                       krys_debug_assert_msg(false, "Unexpected transform box"); return CSSBoxType::BorderBox;
    }
  }
}
