#pragma once

#include "Krystal.HTML/MutationObserver/MutationRecord.hpp"

namespace Krys::HTML::detail
{
  class EmptyNodeMutationRecord : public MutationRecord
  {
  private:
    Ref<Node> _target;
    mutable RefPtr<NodeList> _addedNodes;
    mutable RefPtr<NodeList> _removedNodes;
    DOMString _oldValue;

  protected:
    EmptyNodeMutationRecord(Node &target, const DOMString &oldValue) noexcept;

  public:
    KRYS_NODISCARD RawPtr<Node> Target() const noexcept final
    {
      return _target.get();
    }

    KRYS_NODISCARD RawPtr<NodeList> AddedNodes() const noexcept final;

    KRYS_NODISCARD RawPtr<NodeList> RemovedNodes() const noexcept final;

    KRYS_NODISCARD DOMString OldValue() const noexcept final
    {
      return _oldValue;
    }
  };
}
