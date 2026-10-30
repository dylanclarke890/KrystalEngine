#pragma once

#include "Krystal.Booey/Rendering/Style/RenderStyle.hpp"
#include "Krystal.Booey/Rendering/Updating/RenderTreeBuilder.hpp"

namespace krys::boo
{
  /// @brief Attaches children to style-based ruby (`display: ruby`, `ruby-base` and `ruby-text`), wrapping
  /// them in the anonymous ruby containers and ruby bases the CSS ruby box model needs.
  class RenderTreeBuilder::Ruby
  {
    KRYS_NON_COPY_MOVABLE(Ruby);

  private:
    RenderTreeBuilder &_builder;

  public:
    explicit Ruby(RenderTreeBuilder &builder) noexcept;

    KRYS_NODISCARD RenderElement &
      FindOrCreateParentForStyleBasedRubyChild(RenderElement &parent, const RenderObject &child,
                                               RenderObject *&beforeChild) noexcept;
    void AttachForStyleBasedRuby(RenderElement &parent, RenderPtr<RenderObject> child,
                                 RenderObject *beforeChild) noexcept;
  };

  /// @brief The style of an anonymous ruby container or ruby base: bidi-isolated, and ruby bases don't wrap.
  KRYS_NODISCARD RenderStyle CreateAnonymousStyleForRuby(const RenderStyle &parentStyle,
                                                         DisplayType display) noexcept;
}
