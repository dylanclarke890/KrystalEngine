#pragma once

#include "Krystal.HTML/DOM/MutationRecord.hpp"

namespace Krys::HTML
{
  class ContainerNode;

  class ChildListMutationRecord final : public MutationRecord
  {
  private:
    Ref<Node> _target;
    Ref<NodeList> _addedNodes;
    Ref<NodeList> _removedNodes;
    RefPtr<Node> _previousSibling;
    RefPtr<Node> _nextSibling;

  public:
    ChildListMutationRecord(Ref<Node> &&target, Ref<NodeList> &&added, Ref<NodeList> &&removed,
                            RefPtr<Node> &&previousSibling, RefPtr<Node> &&nextSibling) noexcept;

    KRYS_NODISCARD DOMStringAtom Type() const noexcept override
    {
      return u8"childList";
    }

    KRYS_NODISCARD Ref<Node> Target() const noexcept override
    {
      return _target;
    }

    KRYS_NODISCARD RefPtr<NodeList> AddedNodes() const noexcept override
    {
      return _addedNodes;
    }

    KRYS_NODISCARD RefPtr<NodeList> RemovedNodes() const noexcept override
    {
      return _removedNodes;
    }

    KRYS_NODISCARD RefPtr<Node> PreviousSibling() const noexcept override
    {
      return _previousSibling;
    }

    KRYS_NODISCARD RefPtr<Node> NextSibling() const noexcept override
    {
      return _nextSibling;
    }
  };
}