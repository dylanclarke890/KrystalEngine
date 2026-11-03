#pragma once

#include "Krystal.Booey/DOM/MutationObservers/MutationObserver.hpp"
#include "Krystal.Booey/DOM/MutationObservers/RegisteredObserver.hpp"
#include "Krystal.Booey/DOM/MutationObservers/TransientRegisteredObserver.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include "Krystal.Core/Types/WeakPtr.hpp"

namespace krys::boo::dom
{
  class ContainerNode;
  class HTMLCollection;
  class Node;
  class NodeList;

  /// @brief The data a node needs only some of the time, made when it's first needed; an element's is an
  /// `ElementRareData`.
  class NodeRareData
  {
  private:
    WeakPtr<NodeList> _childNodeList;
    WeakPtr<HTMLCollection> _children;
    WeakPtr<NodeList> _labelsNodeList;

    /// @see https://dom.spec.whatwg.org/#registered-observer-list
    List<Ref<RegisteredObserver>> _registeredObservers;

    /// @brief We track transient registered observers separately despite the spec having them in the same
    /// list as it makes it easier to inspect them later.
    /// @see https://dom.spec.whatwg.org/#registered-observer-list
    List<Ref<TransientRegisteredObserver>> _transientRegisteredObservers;

  public:
    virtual ~NodeRareData() noexcept;

    KRYS_NODISCARD Ref<NodeList> ChildNodes(Node &node) noexcept;

    KRYS_NODISCARD Ref<HTMLCollection> Children(ContainerNode &parent) noexcept;

    /// @brief The node's labels list, if it has made one; held weakly, as the list holds the node.
    KRYS_NODISCARD WeakPtr<NodeList> &LabelsNodeList() noexcept;

    KRYS_NODISCARD List<Ref<RegisteredObserver>> &RegisteredObservers() noexcept;

    KRYS_NODISCARD List<Ref<TransientRegisteredObserver>> &TransientRegisteredObservers() noexcept;
  };
}