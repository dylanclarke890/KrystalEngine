#pragma once

#include "Krystal.Booey/Editing/CompositeEditCommand.hpp"

namespace krys::boo
{
  /// @brief Removes the inline elements (and the redundant `div`s) in a run of inserted nodes that don't
  /// change how it looks.
  class SimplifyMarkupCommand final : public CompositeEditCommand
  {
  private:
    RefPtr<dom::Node> _firstNode;
    RefPtr<dom::Node> _nodeAfterLast;

  public:
    SimplifyMarkupCommand(dom::Document &document, dom::Node *firstNode, dom::Node *nodeAfterLast) noexcept;

    KRYS_NODISCARD static Ref<SimplifyMarkupCommand> Create(dom::Document &document, dom::Node *firstNode,
                                                            dom::Node *nodeAfterLast) noexcept;

  private:
    void DoApply() noexcept override;
    int32 PruneSubsequentAncestorsToRemove(List<Ref<dom::Node>> &nodesToRemove,
                                           size_t startNodeIndex) noexcept;
  };
}
