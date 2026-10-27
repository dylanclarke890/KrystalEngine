#pragma once

#include "Krystal.Booey/Rendering/Updating/RenderTreeBuilder.hpp"

namespace krys::boo
{
  class LegacyRenderSVGContainer;
  class LegacyRenderSVGRoot;
  class RenderSVGInline;
  class RenderSVGText;

  /// @brief Attaches and detaches the children of SVG renderers.
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
    void Attach(RenderSVGInline &parent, RenderPtr<RenderObject> child, RenderObject *beforeChild) noexcept;
    void Attach(RenderSVGText &parent, RenderPtr<RenderObject> child, RenderObject *beforeChild) noexcept;

    KRYS_NODISCARD RenderPtr<RenderObject> Detach(LegacyRenderSVGRoot &parent, RenderObject &child,
                                                  WillBeDestroyed willBeDestroyed) noexcept;
    KRYS_NODISCARD RenderPtr<RenderObject> Detach(LegacyRenderSVGContainer &parent, RenderObject &child,
                                                  WillBeDestroyed willBeDestroyed) noexcept;
    KRYS_NODISCARD RenderPtr<RenderObject> Detach(RenderSVGText &parent, RenderObject &child,
                                                  WillBeDestroyed willBeDestroyed) noexcept;
    KRYS_NODISCARD RenderPtr<RenderObject> Detach(RenderSVGInline &parent, RenderObject &child,
                                                  WillBeDestroyed willBeDestroyed) noexcept;
  };
}
