#pragma once

#include "Krystal.HTML/DOM/MutationRecord.hpp"
#include "Krystal.HTML/MutationObserver/MutationRecordType.hpp"
#include "Krystal.HTML/QualifiedName.hpp"
#include "Krystal.HTML/Types/DOMString.hpp"
#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Lib/Mixins/RefCounted.hpp"
#include "Krystal.Lib/Pointers/RefPtr.hpp"

namespace Krys::HTML
{
  class EmptyNodeMutationRecord : public MutationRecord
  {
  private:
    Ref<Node> _target;
    mutable RefPtr<NodeList> _addedNodes;
    mutable RefPtr<NodeList> _removedNodes;
    DOMString _oldValue;

  protected:
    EmptyNodeMutationRecord(MutationRecordType type, Node &target, DOMString &&oldValue) noexcept;

  public:
    KRYS_NODISCARD RefPtr<Node> Target() const noexcept final
    {
      return _target;
    }

    KRYS_NODISCARD RefPtr<NodeList> AddedNodes() const noexcept final;

    KRYS_NODISCARD RefPtr<NodeList> RemovedNodes() const noexcept final;

    KRYS_NODISCARD DOMString OldValue() const noexcept final
    {
      return _oldValue;
    }
  };
}