#pragma once

#include "Krystal.Booey/DOM/MutationObserver/MutationObserver.hpp"
#include "Krystal.Booey/DOM/MutationObserver/RegisteredObserver.hpp"
#include "Krystal.Booey/DOM/Types/DOMString.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Containers/List.hpp"
#include "Krystal.Core/Memory/RefPtr.hpp"
#include "Krystal.Core/Memory/WeakPtr.hpp"

namespace krys::boo
{
  class ContainerNode;
  class HTMLCollection;
  class Node;
  class NodeList;
}

namespace krys::boo::dom
{
  class LiveCollection;
  class TransientRegisteredObserver;

  /// @brief The data a node needs only some of the time, made when it's first needed; an element's is an
  /// `ElementRareData`.
  class NodeRareData
  {
  private:
    WeakPtr<NodeList> _childNodeList;
    WeakPtr<HTMLCollection> _children;
    WeakPtr<NodeList> _labelsNodeList;
    /// @brief The live collections rooted at the node that have cached their items. They hold the node, and
    /// remove themselves as they forget the items or are destroyed.
    List<LiveCollection *> _liveCollectionCaches;

    /// @see https://dom.spec.whatwg.org/#registered-observer-list
    List<Ref<RegisteredObserver>> _registeredObservers;

    /// @brief The transient registered observers, which the specification keeps in the registered observer
    /// list; apart, they're easier to find and remove.
    /// @see https://dom.spec.whatwg.org/#registered-observer-list
    List<Ref<TransientRegisteredObserver>> _transientRegisteredObservers;

    bool _hasEverPaintedImages {false};

  public:
    virtual ~NodeRareData() noexcept;

    /// @brief @p node's `childNodes`, made the first time; held weakly, as the list holds the node.
    KRYS_NODISCARD Ref<NodeList> ChildNodes(Node &node) noexcept;

    /// @brief Whether the node has painted an image. Until it has, its images may decode asynchronously, as
    /// there's nothing painted before them to flicker.
    KRYS_NODISCARD bool HasEverPaintedImages() const noexcept
    {
      return _hasEverPaintedImages;
    }

    void SetHasEverPaintedImages(bool hasEverPaintedImages) noexcept
    {
      _hasEverPaintedImages = hasEverPaintedImages;
    }

    /// @brief @p parent's `children`, made the first time; held weakly, as the collection holds the node.
    KRYS_NODISCARD Ref<HTMLCollection> Children(ContainerNode &parent) noexcept;

    /// @brief The node's labels list, if it has made one; held weakly, as the list holds the node.
    KRYS_NODISCARD WeakPtr<NodeList> &LabelsNodeList() noexcept;

    /// @brief Adds or removes @p collection from those rooted at the node that cache their items.
    void AddLiveCollectionCache(LiveCollection &collection) noexcept;

    void RemoveLiveCollectionCache(LiveCollection &collection) noexcept;

    /// @brief Clears the caches of the live collections rooted at the node.
    void InvalidateLiveCollectionCaches() noexcept;

    /// @brief Clears what those collections cached that depends on the attribute called @p localName.
    void InvalidateLiveCollectionCachesForAttribute(DOMStringView localName) noexcept;

    /// @brief Clears the cache of the node's `childNodes`, the only collection to hold nodes but elements.
    void InvalidateChildNodeListCache() noexcept;

    /// @see https://dom.spec.whatwg.org/#registered-observer-list
    KRYS_NODISCARD List<Ref<RegisteredObserver>> &RegisteredObservers() noexcept;

    KRYS_NODISCARD List<Ref<TransientRegisteredObserver>> &TransientRegisteredObservers() noexcept;
  };
}
