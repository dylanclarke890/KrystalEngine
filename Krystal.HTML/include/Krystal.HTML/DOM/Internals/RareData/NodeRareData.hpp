#pragma once

#include "Krystal.HTML/DOM/Internals/MutationObserver/RegisteredObserver.hpp"
#include "Krystal.HTML/DOM/Internals/MutationObserver/TransientRegisteredObserver.hpp"
#include "Krystal.HTML/DOM/MutationObserver.hpp"
#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Lib/Pointers/RefPtr.hpp"
#include "Krystal.Lib/Pointers/WeakPtr.hpp"
#include "Krystal.Lib/Types/List.hpp"

namespace Krys::HTML
{
  class Node;
  class NodeList;

  class NodeRareData
  {
  private:
    WeakPtr<NodeList> _childNodeList;

    /// @see https://dom.spec.whatwg.org/#registered-observer-list
    List<Ref<RegisteredObserver>> _registeredObservers;

    /// @brief We track transient registered observers separately despite the spec having them in the same
    /// list as it makes it easier to inspect them later.
    /// @see https://dom.spec.whatwg.org/#registered-observer-list
    List<Ref<TransientRegisteredObserver>> _transientRegisteredObservers;

  public:
    KRYS_NODISCARD Ref<NodeList> ChildNodes(Node &node) noexcept;

    KRYS_NODISCARD List<Ref<RegisteredObserver>> &RegisteredObservers() noexcept;

    KRYS_NODISCARD List<Ref<TransientRegisteredObserver>> &TransientRegisteredObservers() noexcept;
  };
}