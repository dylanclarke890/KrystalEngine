#pragma once

#include "Krystal.HTML/MutationObserver/MutationRecord.hpp"

namespace Krys::HTML
{
  class TreeMutationRecord : public MutationRecord
  {
  private:
    Ref<Node> _target;
    Ref<NodeList> _addedNodes;
    Ref<NodeList> _removedNodes;
    Ref<Node> _previousSibling;
    Ref<Node> _nextSibling;

  public:
    TreeMutationRecord(ContainerNode &target, Ref<NodeList> &&added, Ref<NodeList> &&removed,
                       RefPtr<Node> &&previousSibling, RefPtr<Node> &&nextSibling) noexcept;

    KRYS_NODISCARD const StringAtom &Type() const noexcept override
    {
      static const StringAtom type = u8"childList";
      return type;
    }

    KRYS_NODISCARD RawPtr<Node> Target() const noexcept override
    {
      return _target.get();
    }

    KRYS_NODISCARD RawPtr<NodeList> AddedNodes() const noexcept override
    {
      return _addedNodes.get();
    }

    KRYS_NODISCARD RawPtr<NodeList> RemovedNodes() const noexcept override
    {
      return _removedNodes.get();
    }

    KRYS_NODISCARD RawPtr<Node> PreviousSibling() const noexcept override
    {
      return _previousSibling.get();
    }

    KRYS_NODISCARD RawPtr<Node> NextSibling() const noexcept override
    {
      return _nextSibling.get();
    }
  };
}