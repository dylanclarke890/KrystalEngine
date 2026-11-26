#pragma once

#include "Krystal.Booey/Css/Geometry/DOMRect.hpp"
#include "Krystal.Booey/Dom/Internals/SmallNodeList.hpp"
#include "Krystal.Booey/Dom/Ranges/AbstractRange.hpp"
#include "Krystal.Booey/Dom/Ranges/BoundaryPoint.hpp"
#include "Krystal.Core/Enum.hpp"
#include <compare>

namespace krys::boo::dom
{
  /// @see https://dom.spec.whatwg.org/#dom-range-compareboundarypoints
  enum class BoundaryPointComparator
  {
    StartToStart,
    StartToEnd,
    EndToEnd,
    EndToStart,
  };

  consteval BoundaryPointComparator MaxEnumValue(BoundaryPointComparator) noexcept
  {
    return BoundaryPointComparator::EndToStart;
  }
}

namespace krys::boo
{
  class DOMRectList;
  class DocumentFragment;
}

namespace krys::boo::dom
{
  /// @brief A live range, which follows changes to the tree. Its root is its start node's root.
  /// @see https://dom.spec.whatwg.org/#interface-range
  /// @see https://dom.spec.whatwg.org/#concept-live-range
  /// @see https://dom.spec.whatwg.org/#concept-range-root
  class Range : public AbstractRange
  {
    friend class LiveRangeUpdater;

    enum class DeleteClonedContent : bool
    {
      No,
      Yes,
    };

  public:
    ~Range() noexcept override;

    KRYS_NODISCARD bool IsLiveRange() const noexcept final
    {
      return true;
    }

#pragma region Range - https://dom.spec.whatwg.org/#range

    /// @brief A live range from @p start to @p end; `Document::CreateRange` is `new Range()`.
    /// @see https://dom.spec.whatwg.org/#dom-range-range
    Range(BoundaryPoint start, BoundaryPoint end) noexcept;

    /// @see https://dom.spec.whatwg.org/#dom-range-commonancestorcontainer
    /// @see https://dom.spec.whatwg.org/#get-the-common-ancestor
    KRYS_NODISCARD Node *CommonAncestorContainer() const noexcept;

    /// @see https://dom.spec.whatwg.org/#dom-range-setstart
    ExceptionOr<void> SetStart(Node &node, uint64 offset) noexcept;

    /// @see https://dom.spec.whatwg.org/#dom-range-setend
    ExceptionOr<void> SetEnd(Node &node, uint64 offset) noexcept;

    /// @see https://dom.spec.whatwg.org/#dom-range-setstartbefore
    ExceptionOr<void> SetStartBefore(Node &node) noexcept;

    /// @see https://dom.spec.whatwg.org/#dom-range-setstartafter
    ExceptionOr<void> SetStartAfter(Node &node) noexcept;

    /// @see https://dom.spec.whatwg.org/#dom-range-setendbefore
    ExceptionOr<void> SetEndBefore(Node &node) noexcept;

    /// @see https://dom.spec.whatwg.org/#dom-range-setendafter
    ExceptionOr<void> SetEndAfter(Node &node) noexcept;

    /// @see https://dom.spec.whatwg.org/#dom-range-collapse
    void Collapse(bool toStart = false) noexcept;

    /// @see https://dom.spec.whatwg.org/#dom-range-selectnode
    /// @see https://dom.spec.whatwg.org/#concept-range-select
    ExceptionOr<void> SelectNode(Node &node) noexcept;

    /// @see https://dom.spec.whatwg.org/#dom-range-selectnodecontents
    ExceptionOr<void> SelectNodeContents(Node &node) noexcept;

    /// @see https://dom.spec.whatwg.org/#dom-range-compareboundarypoints
    KRYS_NODISCARD ExceptionOr<std::strong_ordering> CompareBoundaryPoints(BoundaryPointComparator how,
                                                                           const Range &other) const noexcept;

    /// @see https://dom.spec.whatwg.org/#dom-range-deletecontents
    ExceptionOr<void> DeleteContents() noexcept;

    /// @see https://dom.spec.whatwg.org/#dom-range-extractcontents
    /// @see https://dom.spec.whatwg.org/#concept-range-extract
    KRYS_NODISCARD ExceptionOr<Ref<DocumentFragment>> ExtractContents() noexcept;

    /// @see https://dom.spec.whatwg.org/#dom-range-clonecontents
    /// @see https://dom.spec.whatwg.org/#concept-range-clone
    KRYS_NODISCARD ExceptionOr<Ref<DocumentFragment>> CloneContents() const noexcept;

    /// @see https://dom.spec.whatwg.org/#dom-range-insertnode
    /// @see https://dom.spec.whatwg.org/#concept-range-insert
    ExceptionOr<void> InsertNode(Node &node) noexcept;

    /// @see https://dom.spec.whatwg.org/#dom-range-surroundcontents
    ExceptionOr<void> SurroundContents(ContainerNode &newParent) noexcept;

    /// @see https://dom.spec.whatwg.org/#dom-range-clonerange
    KRYS_NODISCARD Ref<Range> CloneRange() const noexcept;

    /// @see https://dom.spec.whatwg.org/#dom-range-ispointinrange
    KRYS_NODISCARD ExceptionOr<bool> IsPointInRange(Node &node, uint64 offset) const noexcept;

    /// @see https://dom.spec.whatwg.org/#dom-range-comparepoint
    KRYS_NODISCARD ExceptionOr<std::strong_ordering> ComparePoint(Node &node, uint64 offset) const noexcept;

    /// @see https://dom.spec.whatwg.org/#dom-range-intersectsnode
    KRYS_NODISCARD bool IntersectsNode(const Node &node) const noexcept;

    /// @see https://dom.spec.whatwg.org/#dom-range-stringifier
    KRYS_NODISCARD ExceptionOr<DOMString> ToString() const noexcept;

#pragma endregion

#pragma region Range (DOM Parsing) - https://html.spec.whatwg.org/multipage/dynamic-markup-insertion.html#dom-range-createcontextualfragment

    /// @brief @p markup parsed as the children of the start node (or its parent element, or a `body`).
    /// @see
    /// https://html.spec.whatwg.org/multipage/dynamic-markup-insertion.html#dom-range-createcontextualfragment
    KRYS_NODISCARD ExceptionOr<Ref<DocumentFragment>>
      CreateContextualFragment(DOMStringView markup) const noexcept;

#pragma endregion

#pragma region Range Extensions (CSSOM View) - https://drafts.csswg.org/cssom-view/#extensions-to-the-range-interface

    /// @brief The border boxes of the elements the range selects, and the boxes of its text.
    /// @see https://drafts.csswg.org/cssom-view/#dom-range-getclientrects
    KRYS_NODISCARD Ref<DOMRectList> GetClientRects() const noexcept;

    /// @brief The union of the client rects, ignoring empty ones.
    /// @see https://drafts.csswg.org/cssom-view/#dom-range-getboundingclientrect
    KRYS_NODISCARD Ref<DOMRect> GetBoundingClientRect() const noexcept;

#pragma endregion

    /// @brief The union of @p range's client rects, ignoring empty ones.
    KRYS_NODISCARD static Ref<DOMRect> BoundingClientRect(const SimpleRange &range) noexcept;

    /// @brief Whether this is the range associated with its document's selection, so changing its boundary
    /// points changes the selection too.
    /// @see https://w3c.github.io/selection-api/#definition
    KRYS_NODISCARD bool IsAssociatedWithSelection() const noexcept
    {
      return _isAssociatedWithSelection;
    }

    /// @brief Called by the selection as it takes this range as its own, or lets it go. A range is let go of
    /// before it's destroyed.
    void DidAssociateWithSelection() noexcept
    {
      _isAssociatedWithSelection = true;
    }

    void DidDisassociateFromSelection() noexcept
    {
      _isAssociatedWithSelection = false;
    }

  private:
    bool _isAssociatedWithSelection {false};

    /// @brief Brings the document's selection up to date with the range's new boundary points, if it's the
    /// selection's range.
    void UpdateAssociatedSelection() noexcept;

    /// @see https://dom.spec.whatwg.org/#concept-range-bp-set
    KRYS_NODISCARD ExceptionOr<void> SetStartBoundaryPoint(Node &node, uint64 offset) noexcept;

    /// @see https://dom.spec.whatwg.org/#concept-range-bp-set
    KRYS_NODISCARD ExceptionOr<void> SetEndBoundaryPoint(Node &node, uint64 offset) noexcept;

    /// @see https://dom.spec.whatwg.org/#contained
    KRYS_NODISCARD bool IsContained(Node &node) const noexcept;

    /// @see https://dom.spec.whatwg.org/#partially-contained
    KRYS_NODISCARD bool IsPartiallyContained(const Node &node) const noexcept;

    KRYS_NODISCARD Node *FirstPartiallyContainedChild(Node &commonAncestor, Node &startContainer,
                                                      Node &endContainer) const noexcept;

    KRYS_NODISCARD Node *LastPartiallyContainedChild(Node &commonAncestor, Node &startContainer,
                                                     Node &endContainer) const noexcept;

    KRYS_NODISCARD ExceptionOr<SmallNodeList> GetContainedChildren(Node *commonAncestor) const noexcept;

    KRYS_NODISCARD static ExceptionOr<void>
      CloneCharacterDataContents(DocumentFragment &fragment, Node &container, size_t offset, size_t length,
                                 DeleteClonedContent deleteClonedContent) noexcept;
  };
}
