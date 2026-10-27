#pragma once

#include "Krystal.Booey/Rendering/Updating/RenderTreeBuilder.hpp"

namespace krys::boo
{
  class LegacyRenderSVGContainer;
  class LegacyRenderSVGRoot;

  /// @brief Attaches and detaches the children of SVG renderers.
  // FIXME: SVG text and inlines (RenderSVGText, RenderSVGInline), with text (B11g).
  class RenderTreeBuilder::SVG
  {
    KRYS_NON_COPY_MOVABLE(SVG);

  private:
    RenderTreeBuilder &_builder;

  public:
    explicit SVG(RenderTreeBuilder &builder) noexcept;

    void Attach(LegacyRenderSVGRoot &parent, RenderPtr<RenderObject> child,
                RenderObject *beforeChild) noexcept;
    void Attach(LegacyRenderSVGContainer &parent, RenderPtr<RenderObject> child,
                RenderObject *beforeChild) noexcept;

    KRYS_NODISCARD RenderPtr<RenderObject> Detach(LegacyRenderSVGRoot &parent, RenderObject &child,
                                                  WillBeDestroyed willBeDestroyed) noexcept;
    KRYS_NODISCARD RenderPtr<RenderObject> Detach(LegacyRenderSVGContainer &parent, RenderObject &child,
                                                  WillBeDestroyed willBeDestroyed) noexcept;
  };
}
