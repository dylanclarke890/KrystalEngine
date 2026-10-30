#pragma once

#include "Krystal.Booey/DOM/Types/DOMString.hpp"
#include "Krystal.Booey/DOM/Types/ExceptionOr.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/RefCounted.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include "Krystal.Core/Types/WeakPtr.hpp"

namespace krys::boo::dom
{
  class Document;
  class Node;
  class Position;
  class Range;
  class ShadowRoot;
  class StaticRange;
  struct SimpleRange;
}

namespace krys::boo
{
  class LocalFrame;

  /// @brief A document's `Selection`: the script's view of the frame's selection, which is the range it
  /// associates with it.
  /// @see https://w3c.github.io/selection-api/#selection-interface
  class DOMSelection final : public RefCounted<DOMSelection>
  {
    KRYS_NON_COPY_MOVABLE(DOMSelection);

  private:
    WeakPtr<dom::Document> _document;

  public:
    explicit DOMSelection(dom::Document &document) noexcept;

    KRYS_NODISCARD dom::DOMString Type() const noexcept;
    KRYS_NODISCARD dom::DOMString Direction() const noexcept;

    dom::ExceptionOr<void> SetBaseAndExtent(dom::Node &anchorNode, uint32 anchorOffset, dom::Node &focusNode,
                                            uint32 focusOffset) noexcept;
    dom::ExceptionOr<void> SetPosition(dom::Node *node, uint32 offset = 0) noexcept;
    /// @note Non-standard.
    void Modify(dom::DOMStringView alter, dom::DOMStringView direction,
                dom::DOMStringView granularity) noexcept;

    // The anchor and focus are the start and end of the selection, and
    // reflect the direction in which the selection was made by the user.
    KRYS_NODISCARD RefPtr<dom::Node> AnchorNode() const noexcept;
    KRYS_NODISCARD uint32 AnchorOffset() const noexcept;
    KRYS_NODISCARD RefPtr<dom::Node> FocusNode() const noexcept;
    KRYS_NODISCARD uint32 FocusOffset() const noexcept;
    KRYS_NODISCARD bool IsCollapsed() const noexcept;
    KRYS_NODISCARD uint32 RangeCount() const noexcept;

    dom::ExceptionOr<void> Collapse(dom::Node *node, uint32 offset = 0) noexcept;
    dom::ExceptionOr<void> CollapseToEnd() noexcept;
    dom::ExceptionOr<void> CollapseToStart() noexcept;
    dom::ExceptionOr<void> Extend(dom::Node &node, uint32 offset = 0) noexcept;
    KRYS_NODISCARD dom::ExceptionOr<Ref<dom::Range>> GetRangeAt(uint32 index) noexcept;
    void RemoveAllRanges() noexcept;
    void AddRange(dom::Range &liveRange) noexcept;
    dom::ExceptionOr<void> RemoveRange(dom::Range &liveRange) noexcept;
    KRYS_NODISCARD List<Ref<dom::StaticRange>>
      GetComposedRanges(const List<Ref<dom::ShadowRoot>> &shadowRoots = {}) noexcept;
    void DeleteFromDocument() noexcept;
    KRYS_NODISCARD bool ContainsNode(dom::Node &node, bool allowPartialContainment = false) const noexcept;
    dom::ExceptionOr<void> SelectAllChildren(dom::Node &node) noexcept;
    KRYS_NODISCARD dom::DOMString ToString() const noexcept;
    void Empty() noexcept;

  private:
    KRYS_NODISCARD LocalFrame *Frame() const noexcept;
    KRYS_NODISCARD Maybe<dom::SimpleRange> GetRange() const noexcept;
    KRYS_NODISCARD dom::Position AnchorPosition() const noexcept;
    KRYS_NODISCARD dom::Position FocusPosition() const noexcept;
    KRYS_NODISCARD RefPtr<dom::Node> ShadowAdjustedNode(const dom::Position &position) const noexcept;
    KRYS_NODISCARD uint32 ShadowAdjustedOffset(const dom::Position &position) const noexcept;
  };
}
