#pragma once

#include "Krystal.Booey/Rendering/RenderTreeAsText.hpp"

namespace krys::boo
{
  class LegacyRenderSVGContainer;
  class LegacyRenderSVGImage;
  class LegacyRenderSVGRoot;
  class LegacyRenderSVGShape;
  class RenderElement;
  class RenderObject;

  namespace svg
  {
    class SVGGraphicsElement;
  }

  // functions used by the main RenderTreeAsText code
  void Write(TextStream &, const LegacyRenderSVGShape &, EnumSet<RenderAsTextFlag>) noexcept;
  void Write(TextStream &, const LegacyRenderSVGRoot &, EnumSet<RenderAsTextFlag>) noexcept;
  void WriteSVGContainer(TextStream &, const LegacyRenderSVGContainer &, EnumSet<RenderAsTextFlag>) noexcept;
  void WriteSVGImage(TextStream &, const LegacyRenderSVGImage &, EnumSet<RenderAsTextFlag>) noexcept;
  void WriteSVGPaintingFeatures(TextStream &, const RenderElement &, EnumSet<RenderAsTextFlag>) noexcept;
  void WriteResources(TextStream &, const RenderObject &, EnumSet<RenderAsTextFlag>) noexcept;
  // FIXME: Resource containers and gradient stops (writeSVGResourceContainer, writeSVGGradientStop), with
  // paint servers (B11e); text (writeSVGText, writeSVGInlineText), with text (B11g).
  void WriteSVGGraphicsElement(TextStream &, const svg::SVGGraphicsElement &) noexcept;
}
