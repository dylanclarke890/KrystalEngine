#pragma once

#include "Krystal.Booey/DOM/Types/DOMString.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Base.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::dom
{
  class Node;

  /// @brief What can change a live collection's items, other than the descendants of its root changing.
  /// @note A filter that reads anything else must have its collection invalidated when that changes.
  enum class LiveCollectionInvalidation : uint8
  {
    TreeOnly,
    Class,
    IdOrName,
    Name,
    Href,
    Count
  };

  /// @brief Which nodes a live collection looks at.
  enum class LiveCollectionTraversal : uint8
  {
    Descendants,
    Children
  };

  /// @brief Whether a change to the attribute called @p localName can change the items of a collection with
  /// @p invalidation.
  KRYS_NODISCARD bool LiveCollectionDependsOnAttribute(LiveCollectionInvalidation invalidation,
                                                       DOMStringAtom localName) noexcept;

  /// @brief The part of a live collection that keeps its cache valid: while it has a cache it's registered
  /// with its root, which clears it on a change to the root's descendants, and counted by the document, which
  /// spares changes the walk to the root if nothing could need it.
  class LiveCollection
  {
    KRYS_NON_COPY_MOVABLE(LiveCollection);

  private:
    Ref<Node> _root;
    LiveCollectionInvalidation _invalidation;
    LiveCollectionTraversal _traversal;
    mutable bool _isRegistered {false};

  public:
    virtual ~LiveCollection() noexcept;

    /// @brief Forgets the collection's items.
    virtual void InvalidateCache() noexcept = 0;

    /// @brief Forgets what depends on the attribute called @p localName.
    virtual void InvalidateCacheForAttribute(DOMStringAtom localName) noexcept;

  protected:
    LiveCollection(Node &root, LiveCollectionInvalidation invalidation,
                   LiveCollectionTraversal traversal) noexcept;

    /// @brief The node to look at first, if any.
    KRYS_NODISCARD Node *FirstNode() const noexcept;

    /// @brief The node to look at after @p node, if any.
    KRYS_NODISCARD Node *NextNode(const Node &node) const noexcept;

    KRYS_NODISCARD Node &Root() const noexcept;

    void RegisterCache() const noexcept;
    void UnregisterCache() const noexcept;
  };
}
