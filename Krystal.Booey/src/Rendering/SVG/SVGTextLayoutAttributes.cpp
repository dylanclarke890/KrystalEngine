#include "Krystal.Booey/Rendering/SVG/SVGTextLayoutAttributes.hpp"
#include "Krystal.Booey/Rendering/SVG/RenderSVGInlineText.hpp"

namespace krys::boo
{
  SVGTextLayoutAttributes::SVGTextLayoutAttributes(RenderSVGInlineText &context) noexcept : _context(context)
  {
  }

  void SVGTextLayoutAttributes::Clear() noexcept
  {
    _characterDataMap.clear();
    _textMetricsValues.clear();
  }
}
