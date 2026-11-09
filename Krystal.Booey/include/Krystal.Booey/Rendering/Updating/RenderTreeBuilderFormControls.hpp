#pragma once

#include "Krystal.Booey/Rendering/Updating/RenderTreeBuilder.hpp"

namespace krys::boo
{
  class RenderBlock;
  class RenderButton;
  class RenderMenuList;

  /// @brief Attaches and detaches the children of form control renderers, which wrap them in an anonymous
  /// inner block.
  // FIXME: Checkmarks of checkboxes and radios (appearance: base).
  class RenderTreeBuilder::FormControls
  {
    KRYS_NON_COPY_MOVABLE(FormControls);

  private:
    RenderTreeBuilder &_builder;

  public:
    explicit FormControls(RenderTreeBuilder &builder) noexcept;

    /// @brief Updates the generated boxes of @p renderer's element, if it's a select or option rendered with
    /// base appearance: an option's checkmark and a select's picker icon.
    void UpdateAfterDescendants(RenderElement &renderer) noexcept;

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
