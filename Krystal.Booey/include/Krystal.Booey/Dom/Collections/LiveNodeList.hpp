#pragma once

#include "Krystal.Booey/Dom/Collections/CollectionIndexCache.hpp"
#include "Krystal.Booey/Dom/Collections/LiveCollection.hpp"
#include "Krystal.Booey/Dom/Collections/NodeList.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/Func.hpp"

namespace krys::boo
{
  class Node;

  using LiveNodeListFilterFunc = Func<bool(const Node &node)>;

  /// @brief A live node list: a view of the root's descendants that match the filter, in tree order, cached
  /// until something it depends on changes.
  /// @see https://dom.spec.whatwg.org/#concept-collection
  /// @see https://dom.spec.whatwg.org/#concept-collection-live
  /// @see https://dom.spec.whatwg.org/#represented-by-the-collection
  class LiveNodeList : public NodeList, public dom::LiveCollection
  {
    friend class dom::CollectionIndexCache<LiveNodeList, Node>;

  private:
    LiveNodeListFilterFunc _filter;
    mutable dom::CollectionIndexCache<LiveNodeList, Node> _indexCache;

  public:
    LiveNodeList(Node &root, LiveNodeListFilterFunc filter,
                 dom::LiveCollectionInvalidation invalidation = dom::LiveCollectionInvalidation::TreeOnly,
                 dom::LiveCollectionTraversal traversal = dom::LiveCollectionTraversal::Descendants) noexcept;

#pragma region NodeList - https://dom.spec.whatwg.org/#nodelist

    /// @see https://dom.spec.whatwg.org/#dom-nodelist-item
    KRYS_NODISCARD RefPtr<Node> Item(size_t index) noexcept override;

    /// @see https://dom.spec.whatwg.org/#dom-nodelist-item
    KRYS_NODISCARD RefPtr<const Node> Item(size_t index) const noexcept override;

    /// @see https://dom.spec.whatwg.org/#dom-nodelist-item
    KRYS_NODISCARD RefPtr<Node> operator[](size_t index) noexcept override;

    /// @see https://dom.spec.whatwg.org/#dom-nodelist-item
    KRYS_NODISCARD RefPtr<const Node> operator[](size_t index) const noexcept override;

    /// @see https://dom.spec.whatwg.org/#dom-nodelist-length
    KRYS_NODISCARD size_t Length() const noexcept override;

#pragma endregion

    void InvalidateCache() noexcept override;

  protected:
    KRYS_NODISCARD bool IsLiveNodeList() const noexcept override
    {
      return true;
    }

  private:
    void WillValidateIndexCache() const noexcept;

    KRYS_NODISCARD Node *CollectionBegin() const noexcept;

    KRYS_NODISCARD Node *CollectionNext(Node &node) const noexcept;

    /// @brief The first node to match the filter from @p node on, if any.
    KRYS_NODISCARD Node *FirstMatchFrom(Node *node) const noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::LiveNodeList)
  static bool IsType(const krys::boo::NodeList &target) noexcept
  {
    return target.IsLiveNodeList();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
