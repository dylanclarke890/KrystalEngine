#include "Krystal.Booey/Rendering/SVG/SVGRootInlineBox.hpp"
#include "Krystal.Booey/Rendering/SVG/RenderSVGText.hpp"

namespace krys::boo
{
  SVGRootInlineBox::SVGRootInlineBox(RenderSVGText &renderSVGText) noexcept
      : LegacyRootInlineBox(renderSVGText)
  {
  }

  RenderSVGText &SVGRootInlineBox::GetRenderSVGText() const noexcept
  {
    return Downcast<RenderSVGText>(BlockFlow());
  }
}
