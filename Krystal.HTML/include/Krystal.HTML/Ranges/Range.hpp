#pragma once

#include "Krystal.HTML/Ranges/AbstractRange.hpp"
#include "Krystal.HTML/Ranges/BoundaryPoint.hpp"
#include "Krystal.Lib/Core/Enum.hpp"
#include "Krystal.Lib/Types/StronglyTypedValue.hpp"
#include <compare>

namespace Krys::HTML
{
  enum class BoundaryPointComparator
  {
    StartToStart,
    StartToEnd,
    EndToEnd,
    EndToStart,
  };
}

KRYS_DEFINE_CONTIGUOUS_ENUM_TRAITS(Krys::HTML::BoundaryPointComparator, 4u)

namespace Krys::HTML
{
  class DocumentFragment;

  class Range : public AbstractRange
  {
    struct DeleteClonedContent : public StronglyTypedBool<DeleteClonedContent>
    {
      using Base::Base;
    };

  public:
    Range(BoundaryPoint start, BoundaryPoint end) noexcept;

    ~Range() noexcept override = default;

    KRYS_NODISCARD RawPtr<Node> CommonAncestorContainer() const noexcept;

    ExceptionOr<void> SetStart(Node &node, uint64 offset) noexcept;
    ExceptionOr<void> SetEnd(Node &node, uint64 offset) noexcept;
    ExceptionOr<void> SetStartBefore(Node &node) noexcept;
    ExceptionOr<void> SetStartAfter(Node &node) noexcept;
    ExceptionOr<void> SetEndBefore(Node &node) noexcept;
    ExceptionOr<void> SetEndAfter(Node &node) noexcept;
    void Collapse(bool toStart = false) noexcept;
    ExceptionOr<void> SelectNode(Node &node) noexcept;
    ExceptionOr<void> SelectNodeContents(Node &node) noexcept;

    KRYS_NODISCARD ExceptionOr<std::strong_ordering> CompareBoundaryPoints(BoundaryPointComparator how,
                                                                           const Range &other) const noexcept;

    void DeleteContents() noexcept;
    KRYS_NODISCARD ExceptionOr<Ref<DocumentFragment>> ExtractContents() noexcept;
    KRYS_NODISCARD ExceptionOr<Ref<DocumentFragment>> CloneContents() const noexcept;
    ExceptionOr<void> InsertNode(Node &node) noexcept;
    ExceptionOr<void> SurroundContents(ContainerNode &newParent) noexcept;

    KRYS_NODISCARD Ref<Range> CloneRange() const noexcept;
    void Detach() noexcept;

    KRYS_NODISCARD ExceptionOr<bool> IsPointInRange(Node &node, uint64 offset) const noexcept;
    KRYS_NODISCARD ExceptionOr<std::strong_ordering> ComparePoint(Node &node, uint64 offset) const noexcept;

    KRYS_NODISCARD bool IntersectsNode(const Node &node) const noexcept;

    KRYS_NODISCARD ExceptionOr<DOMString> ToString() const noexcept;

  private:
    ExceptionOr<void> SetStartBoundaryPoint(Node &node, uint64 offset) noexcept;
    ExceptionOr<void> SetEndBoundaryPoint(Node &node, uint64 offset) noexcept;

    ExceptionOr<void> CloneCharacterDataContents(DocumentFragment &fragment, Node &container, size_t offset,
                                                 size_t length,
                                                 DeleteClonedContent deleteClonedContent) const noexcept;

    KRYS_NODISCARD bool IsPartiallyContained(const Node &node) const noexcept;
    KRYS_NODISCARD RawPtr<Node> GetFirstPartiallyContainedChild(RawPtr<Node> commonAncestor) const noexcept;
    KRYS_NODISCARD RawPtr<Node> GetLastPartiallyContainedChild(RawPtr<Node> commonAncestor) const noexcept;
    KRYS_NODISCARD List<Ref<Node>> GetContainedChildren(RawPtr<Node> commonAncestor) const noexcept;

    ExceptionOr<void>
      CloneFirstPartiallyContainedChildContents(RawPtr<Node> child, DocumentFragment &fragment,
                                                DeleteClonedContent deleteClonedContent) const noexcept;
    ExceptionOr<void>
      CloneLastPartiallyContainedChildContents(RawPtr<Node> child, DocumentFragment &fragment,
                                               DeleteClonedContent deleteClonedContent) const noexcept;
  };
}