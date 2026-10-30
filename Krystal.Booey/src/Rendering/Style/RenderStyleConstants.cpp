#include "Krystal.Booey/Rendering/Style/RenderStyleConstants.hpp"
#include "Krystal.Core/Assert.hpp"
#include "Krystal.Core/Text/TextStream.hpp"

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

  TextStream &operator<<(TextStream &ts, ImageRendering imageRendering) noexcept
  {
    switch (imageRendering)
    {
      case ImageRendering::Auto:            ts << u8"auto"; break;
      case ImageRendering::OptimizeSpeed:   ts << u8"optimizeSpeed"; break;
      case ImageRendering::OptimizeQuality: ts << u8"optimizeQuality"; break;
      case ImageRendering::CrispEdges:      ts << u8"crispEdges"; break;
      case ImageRendering::Pixelated:       ts << u8"pixelated"; break;
    }
    return ts;
  }
}
