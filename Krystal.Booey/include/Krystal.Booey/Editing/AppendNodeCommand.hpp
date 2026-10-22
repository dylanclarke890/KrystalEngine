#pragma once

#include "Krystal.Booey/Editing/EditCommand.hpp"

namespace krys::boo::dom
{
  class ContainerNode;
  class Node;
}

namespace krys::boo
{
  /// @brief Appends a node to a parent.
  class AppendNodeCommand final : public SimpleEditCommand
  {
  private:
    const Ref<dom::ContainerNode> _parent;
    const Ref<dom::Node> _node;

  public:
    KRYS_NODISCARD static Ref<AppendNodeCommand> Create(dom::ContainerNode &parent, dom::Node &node,
                                                        EditAction editingAction) noexcept;

    AppendNodeCommand(dom::ContainerNode &parent, dom::Node &node, EditAction editingAction) noexcept;

  private:
    void DoApply() noexcept override;
    void DoUnapply() noexcept override;
  };
}
