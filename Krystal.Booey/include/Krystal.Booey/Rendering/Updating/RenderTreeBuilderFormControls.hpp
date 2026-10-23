#pragma once

#include "Krystal.Booey/Rendering/Updating/RenderTreeBuilder.hpp"

namespace krys::boo
{
  class RenderBlock;
  class RenderButton;
  class RenderMenuList;

  /// @brief Attaches and detaches the children of form control renderers, which wrap them in an anonymous
  /// inner block.
  // FIXME: Checkmarks (appearance: base checkboxes and radios).
  class RenderTreeBuilder::FormControls
  {
    KRYS_NON_COPY_MOVABLE(FormControls);

  private:
    RenderTreeBuilder &_builder;

  public:
    explicit FormControls(RenderTreeBuilder &builder) noexcept;

    void Attach(RenderButton &parent, RenderPtr<RenderObject> child, RenderObject *beforeChild) noexcept;
    void Attach(RenderMenuList &parent, RenderPtr<RenderObject> child, RenderObject *beforeChild) noexcept;

    KRYS_NODISCARD RenderPtr<RenderObject> Detach(RenderMenuList &parent, RenderObject &child,
                                                  WillBeDestroyed willBeDestroyed) noexcept;
    KRYS_NODISCARD RenderPtr<RenderObject> Detach(RenderButton &parent, RenderObject &child,
                                                  WillBeDestroyed willBeDestroyed) noexcept;

  private:
    KRYS_NODISCARD RenderBlock &FindOrCreateParentForChild(RenderButton &parent) noexcept;
    KRYS_NODISCARD RenderBlock &FindOrCreateParentForChild(RenderMenuList &parent) noexcept;
  };
}
