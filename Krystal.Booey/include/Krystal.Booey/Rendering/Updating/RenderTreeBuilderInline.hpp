#pragma once

#include "Krystal.Booey/Rendering/Updating/RenderTreeBuilder.hpp"

namespace krys::boo
{
  class RenderInline;

  /// @brief Attaches children to inline boxes. Block children stay children of the inline box (blocks in
  /// inline layout).
  class RenderTreeBuilder::Inline
  {
    KRYS_NON_COPY_MOVABLE(Inline);

  private:
    RenderTreeBuilder &_builder;

  public:
    explicit Inline(RenderTreeBuilder &builder) noexcept;

    void Attach(RenderInline &parent, RenderPtr<RenderObject> child, RenderObject *beforeChild) noexcept;
    void AttachIgnoringContinuation(RenderInline &parent, RenderPtr<RenderObject> child,
                                    RenderObject *beforeChild = nullptr) noexcept;

    void ChildBecameNonInline(RenderInline &parent, RenderElement &child) noexcept;
  };
}
