#pragma once

#include "Krystal.Booey/DOM/Node.hpp"
#include "Krystal.Booey/Editing/CompositeEditCommand.hpp"

namespace krys::boo
{
  /// @brief Removes a node, moving its children to where it was.
  class RemoveNodePreservingChildrenCommand final : public CompositeEditCommand
  {
  private:
    const Ref<dom::Node> _node;
    ShouldAssumeContentIsAlwaysEditable _shouldAssumeContentIsAlwaysEditable;

  public:
    RemoveNodePreservingChildrenCommand(
      dom::Node &node, ShouldAssumeContentIsAlwaysEditable shouldAssumeContentIsAlwaysEditable,
      EditAction editingAction) noexcept;

    KRYS_NODISCARD static Ref<RemoveNodePreservingChildrenCommand>
      Create(dom::Node &node, ShouldAssumeContentIsAlwaysEditable shouldAssumeContentIsAlwaysEditable,
             EditAction editingAction) noexcept;

  private:
    void DoApply() noexcept override;
  };
}
