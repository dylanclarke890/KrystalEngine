#pragma once

#include "Krystal.HTML/DOM/AbstractRange.hpp"
#include "Krystal.HTML/DOM/BoundaryPoint.hpp"
#include "Krystal.HTML/DOM/Enums/BoundaryPointComparator.hpp"
#include "Krystal.HTML/DOM/Types/SmallNodeList.hpp"
#include "Krystal.Lib/Core/Enum.hpp"
#include "Krystal.Lib/Types/StronglyTypedValue.hpp"
#include <compare>

namespace Krys::HTML
{
  class DocumentFragment;

  /// @see https://dom.spec.whatwg.org/#interface-range
  class Range : public AbstractRange
  {
    friend class LiveRangeUpdater;

    struct DeleteClonedContent : public StronglyTypedBool<DeleteClonedContent>
    {
      using Base::Base;
    };

  public:
    ~Range() noexcept override;

#pragma region Range - https://dom.spec.whatwg.org/#range

    /// @see https://dom.spec.whatwg.org/#dom-range-range
    Range(BoundaryPoint start, BoundaryPoint end) noexcept;

    /// @see https://dom.spec.whatwg.org/#dom-range-startcontainer
    KRYS_NODISCARD RawPtr<Node> CommonAncestorContainer() const noexcept;

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
    ExceptionOr<void> SelectNode(Node &node) noexcept;

    /// @see https://dom.spec.whatwg.org/#dom-range-selectnodecontents
    ExceptionOr<void> SelectNodeContents(Node &node) noexcept;

    /// @see https://dom.spec.whatwg.org/#dom-range-compareboundarypoints
    KRYS_NODISCARD ExceptionOr<std::strong_ordering> CompareBoundaryPoints(BoundaryPointComparator how,
                                                                           const Range &other) const noexcept;

    /// @see https://dom.spec.whatwg.org/#dom-range-deletecontents
    ExceptionOr<void> DeleteContents() noexcept;

    /// @see https://dom.spec.whatwg.org/#dom-range-extractcontents
    KRYS_NODISCARD ExceptionOr<Ref<DocumentFragment>> ExtractContents() noexcept;

    /// @see https://dom.spec.whatwg.org/#dom-range-clonecontents
    KRYS_NODISCARD ExceptionOr<Ref<DocumentFragment>> CloneContents() const noexcept;

    /// @see https://dom.spec.whatwg.org/#dom-range-insertnode
    ExceptionOr<void> InsertNode(Node &node) noexcept;

    /// @see https://dom.spec.whatwg.org/#dom-range-surroundcontents
    ExceptionOr<void> SurroundContents(ContainerNode &newParent) noexcept;

    /// @see https://dom.spec.whatwg.org/#dom-range-clonerange
    KRYS_NODISCARD Ref<Range> CloneRange() const noexcept;

    /// @see https://dom.spec.whatwg.org/#dom-range-detach
    void Detach() noexcept;

    /// @see https://dom.spec.whatwg.org/#dom-range-ispointinrange
    KRYS_NODISCARD ExceptionOr<bool> IsPointInRange(Node &node, uint64 offset) const noexcept;

    /// @see https://dom.spec.whatwg.org/#dom-range-comparepoint
    KRYS_NODISCARD ExceptionOr<std::strong_ordering> ComparePoint(Node &node, uint64 offset) const noexcept;

    /// @see https://dom.spec.whatwg.org/#dom-range-intersectsnode
    KRYS_NODISCARD bool IntersectsNode(const Node &node) const noexcept;

    /// @see https://dom.spec.whatwg.org/#dom-range-stringifier
    KRYS_NODISCARD ExceptionOr<DOMString> ToString() const noexcept;

#pragma endregion

  private:
    KRYS_NODISCARD ExceptionOr<void> SetStartBoundaryPoint(Node &node, uint64 offset) noexcept;
    KRYS_NODISCARD ExceptionOr<void> SetEndBoundaryPoint(Node &node, uint64 offset) noexcept;

    KRYS_NODISCARD bool IsContained(Node &node) const noexcept;
    KRYS_NODISCARD bool IsPartiallyContained(const Node &node) const noexcept;

    KRYS_NODISCARD RawPtr<Node> FirstPartiallyContainedChild(Node &commonAncestor, Node &startContainer,
                                                             Node &endContainer) const noexcept;
    KRYS_NODISCARD RawPtr<Node> LastPartiallyContainedChild(Node &commonAncestor, Node &startContainer,
                                                            Node &endContainer) const noexcept;
    KRYS_NODISCARD ExceptionOr<SmallNodeList>
      GetContainedChildren(RawPtr<Node> commonAncestor) const noexcept;

    KRYS_NODISCARD ExceptionOr<void>
      CloneCharacterDataContents(DocumentFragment &fragment, Node &container, size_t offset, size_t length,
                                 DeleteClonedContent deleteClonedContent) const noexcept;
  };
}