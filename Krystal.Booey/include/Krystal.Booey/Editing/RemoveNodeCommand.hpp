#pragma once

#include "Krystal.Booey/Editing/EditCommand.hpp"

namespace krys::boo::dom
{
  class ContainerNode;
  class Node;
}

namespace krys::boo
{
  /// @brief Removes a node, remembering where it was.
  class RemoveNodeCommand final : public SimpleEditCommand
  {
  private:
    const Ref<dom::Node> _node;
    RefPtr<dom::ContainerNode> _parent;
    RefPtr<dom::Node> _refChild;
    ShouldAssumeContentIsAlwaysEditable _shouldAssumeContentIsAlwaysEditable;

  public:
    KRYS_NODISCARD static Ref<RemoveNodeCommand>
      Create(dom::Node &node, ShouldAssumeContentIsAlwaysEditable shouldAssumeContentIsAlwaysEditable,
             EditAction editingAction = EditAction::Unspecified) noexcept;

    RemoveNodeCommand(dom::Node &node,
                      ShouldAssumeContentIsAlwaysEditable shouldAssumeContentIsAlwaysEditable,
                      EditAction editingAction) noexcept;

  private:
    void DoApply() noexcept override;
    void DoUnapply() noexcept override;
  };
}
