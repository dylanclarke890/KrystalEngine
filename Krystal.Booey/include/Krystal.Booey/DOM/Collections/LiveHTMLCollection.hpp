#pragma once

#include "Krystal.Booey/DOM/Collections/CollectionIndexCache.hpp"
#include "Krystal.Booey/DOM/Collections/HTMLCollection.hpp"
#include "Krystal.Booey/DOM/Collections/LiveCollection.hpp"
#include "Krystal.Core/Types/Func.hpp"
#include "Krystal.Core/Types/HashMap.hpp"

namespace krys::boo::dom
{
  class ContainerNode;

  using LiveHTMLCollectionFilterFunc = Func<bool(const Element &)>;

  /// @brief A live element collection: a view of the root's descendant elements that match the filter, in
  /// tree order, cached until something it depends on changes.
  /// @see https://dom.spec.whatwg.org/#concept-collection
  /// @see https://dom.spec.whatwg.org/#concept-collection-live
  /// @see https://dom.spec.whatwg.org/#represented-by-the-collection
  class LiveHTMLCollection : public HTMLCollection, public LiveCollection
  {
    friend class CollectionIndexCache<LiveHTMLCollection, Element>;

  private:
    LiveHTMLCollectionFilterFunc _filter;
    mutable CollectionIndexCache<LiveHTMLCollection, Element> _indexCache;
    /// @brief The first element with each ID or HTML `name` value, once a name has been looked up. The
    /// document counts it, as it depends on those attributes whatever the collection's invalidation.
    mutable HashMap<DOMString, Element *> _namedItems;
    mutable bool _hasNamedItems {false};

  public:
    LiveHTMLCollection(ContainerNode &root, LiveHTMLCollectionFilterFunc &&filter,
                       LiveCollectionInvalidation invalidation = LiveCollectionInvalidation::TreeOnly,
                       LiveCollectionTraversal traversal = LiveCollectionTraversal::Descendants) noexcept;
    ~LiveHTMLCollection() noexcept override;

    KRYS_NODISCARD RefPtr<Element> Item(size_t index) noexcept override;
    KRYS_NODISCARD RefPtr<const Element> Item(size_t index) const noexcept override;

    KRYS_NODISCARD RefPtr<Element> operator[](size_t index) noexcept override;
    KRYS_NODISCARD RefPtr<const Element> operator[](size_t index) const noexcept override;

    KRYS_NODISCARD RefPtr<Element> NamedItem(DOMStringView name) noexcept override;
    KRYS_NODISCARD RefPtr<const Element> NamedItem(DOMStringView name) const noexcept override;

    KRYS_NODISCARD RefPtr<Element> operator[](DOMStringView name) noexcept override;
    KRYS_NODISCARD RefPtr<const Element> operator[](DOMStringView name) const noexcept override;

    KRYS_NODISCARD size_t Length() const noexcept override;

    KRYS_NODISCARD List<DOMString> SupportedPropertyNames() const noexcept override;

    void InvalidateCache() noexcept override;
    void InvalidateCacheForAttribute(DOMStringAtom localName) noexcept override;

  protected:
    /// @brief Called before the collection caches its items, for a subclass to bring up to date what its
    /// filter reads: anything that would invalidate the cache must be done by now.
    virtual void WillValidateIndexCache() const noexcept;

    /// @brief The first item, for a subclass whose items aren't found by filtering nodes.
    KRYS_NODISCARD virtual Element *CollectionBegin() const noexcept;

    /// @brief The item after @p element.
    KRYS_NODISCARD virtual Element *CollectionNext(Element &element) const noexcept;

  private:
    /// @brief The first element to match the filter from @p node on, if any.
    KRYS_NODISCARD Element *FirstMatchFrom(Node *node) const noexcept;

    void ForgetNamedItems() noexcept;
  };
}