#pragma once

#include "Krystal.Booey/Rendering/RenderTreeAsText.hpp"

namespace krys::boo
{
  class LegacyRenderSVGContainer;
  class LegacyRenderSVGImage;
  class LegacyRenderSVGResourceContainer;
  class LegacyRenderSVGRoot;
  class LegacyRenderSVGShape;
  class RenderElement;
  class RenderObject;
  class RenderSVGGradientStop;
  class RenderSVGInlineText;
  class RenderSVGText;

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
  void WriteSVGResourceContainer(TextStream &, const LegacyRenderSVGResourceContainer &,
                                 EnumSet<RenderAsTextFlag>) noexcept;
  void WriteSVGGradientStop(TextStream &, const RenderSVGGradientStop &, EnumSet<RenderAsTextFlag>) noexcept;
  void WriteSVGText(TextStream &, const RenderSVGText &, EnumSet<RenderAsTextFlag>) noexcept;
  void WriteSVGInlineText(TextStream &, const RenderSVGInlineText &, EnumSet<RenderAsTextFlag>) noexcept;
  void WriteSVGGraphicsElement(TextStream &, const svg::SVGGraphicsElement &) noexcept;
}
