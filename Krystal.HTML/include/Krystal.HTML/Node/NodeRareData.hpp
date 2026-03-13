#pragma once

#include "Krystal.HTML/MutationObserver/MutationObserver.hpp"
#include "Krystal.HTML/MutationObserver/RegisteredObserver.hpp"
#include "Krystal.HTML/MutationObserver/TransientRegisteredObserver.hpp"
#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Lib/Pointers/RefPtr.hpp"
#include "Krystal.Lib/Pointers/WeakPtr.hpp"
#include "Krystal.Lib/Types/List.hpp"
#include "Krystal.Lib/Types/Maybe.hpp"
#include "Krystal.Lib/Types/Variant.hpp"

namespace Krys::HTML
{
  class Node;
  class ChildNodeList;
  class NodeList;

  class NodeRareData
  {
  private:
    WeakPtr<ChildNodeList> _childNodeList;
    Maybe<List<Ref<RegisteredObserver>>> _registeredObserverList;
    Maybe<List<Ref<TransientRegisteredObserver>>> _transientRegisteredObservers;

  public:
    KRYS_NODISCARD Ref<NodeList> ChildNodes(Node &node) noexcept;

    void InvalidateChildNodes() noexcept;

    KRYS_NODISCARD List<Ref<RegisteredObserver>> &RegisteredObserverList() noexcept;

    KRYS_NODISCARD List<Ref<TransientRegisteredObserver>> &TransientRegisteredObservers() noexcept;
  };
}