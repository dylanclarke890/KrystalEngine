#pragma once

#include "Krystal.Booey/Core/WritingMode.hpp"
#include "Krystal.Booey/DOM/BoundaryPoint.hpp"
#include "Krystal.Booey/DOM/ContainerNode.hpp"
#include "Krystal.Booey/Editing/EditingBoundary.hpp"
#include "Krystal.Booey/Editing/TextAffinity.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include <compare>

namespace krys::boo
{
  class RenderElement;
  struct InlineBoxAndOffset;
}

namespace krys::boo::dom
{
  class Text;
  struct SimpleRange;

  enum class PositionMoveType : uint8
  {
    CodePoint,       // Move by a single code point.
    Character,       // Move to the next Unicode character break.
    BackwardDeletion // Subject to platform conventions.
  };

  /// @brief A position in the DOM for editing: anchored at an offset in a node, or before or after a node or
  /// its children.
  /// @note Offsets in text are bytes of its UTF-8 data, as the DOM's are.
  class Position
  {
  public:
    enum class AnchorType : uint8
    {
      PositionIsOffsetInAnchor,
      PositionIsBeforeAnchor,
      PositionIsAfterAnchor,
      PositionIsBeforeChildren,
      PositionIsAfterChildren,
    };

  private:
    friend Position MakeDeprecatedLegacyPosition(RefPtr<Node> &&, uint32 offset) noexcept;

    // For creating legacy editing positions: (Anchor type will be determined from
    // editingIgnoresContent(node))
    enum class LegacyEditingPositionFlag
    {
      On
    };

    RefPtr<Node> _anchorNode;
    // _offset can be the offset inside _anchorNode, or if editingIgnoresContent(_anchorNode)
    // returns true, then other places in editing will treat _offset == 0 as "before the anchor"
    // and _offset > 0 as "after the anchor node".  See parentAnchoredEquivalent for more info.
    uint32 _offset {0};
    AnchorType _anchorType {AnchorType::PositionIsOffsetInAnchor};
    bool _isLegacyEditingPosition {false};

  public:
    Position() noexcept = default;

    // For creating before/after positions:
    Position(RefPtr<Node> &&anchorNode, AnchorType anchorType) noexcept;
    Position(RefPtr<Text> &&textNode, uint32 offset) noexcept;

    // For creating offset positions:
    // FIXME: This constructor should eventually go away. See bug 63040.
    Position(RefPtr<Node> &&anchorNode, uint32 offset, AnchorType anchorType) noexcept;

    KRYS_NODISCARD AnchorType GetAnchorType() const noexcept
    {
      return _anchorType;
    }

    void Clear() noexcept
    {
      _anchorNode = nullptr;
      _offset = 0;
      _anchorType = AnchorType::PositionIsOffsetInAnchor;
      _isLegacyEditingPosition = false;
    }

    // These are always DOM compliant values.  Editing positions like [img, 0] (aka [img, before])
    // will return img->parentNode() and img->computeNodeIndex() from these functions.
    /// @brief Null for a before/after position anchored to a node with no parent.
    KRYS_NODISCARD Node *GetContainerNode() const noexcept;
    KRYS_NODISCARD Text *ContainerText() const noexcept;
    KRYS_NODISCARD Element *ContainerOrParentElement() const noexcept;

    /// @brief O(n) for before/after-anchored positions, O(1) for parent-anchored positions.
    KRYS_NODISCARD int32 ComputeOffsetInContainerNode() const noexcept;
    /// @brief Convenience method for DOM positions that also fixes up some positions for editing.
    KRYS_NODISCARD Position ParentAnchoredEquivalent() const noexcept;

    // Inline O(1) access for Positions which callers know to be parent-anchored
    KRYS_NODISCARD int32 OffsetInContainerNode() const noexcept
    {
      krys_debug_assert(GetAnchorType() == AnchorType::PositionIsOffsetInAnchor);
      return static_cast<int32>(_offset);
    }

    // New code should not use this function.
    KRYS_NODISCARD int32 DeprecatedEditingOffset() const noexcept
    {
      if (_isLegacyEditingPosition
          || (_anchorType != AnchorType::PositionIsAfterAnchor
              && _anchorType != AnchorType::PositionIsAfterChildren))
      {
        return static_cast<int32>(_offset);
      }
      return OffsetForPositionAfterAnchor();
    }

    KRYS_NODISCARD RefPtr<Node> FirstNode() const noexcept;

    // These are convenience methods which are smart about whether the position is neighbor anchored or parent
    // anchored
    KRYS_NODISCARD Node *ComputeNodeBeforePosition() const noexcept;
    KRYS_NODISCARD Node *ComputeNodeAfterPosition() const noexcept;

    KRYS_NODISCARD Node *AnchorNode() const noexcept
    {
      return _anchorNode.get();
    }

    // FIXME: Callers should be moved off of node(), node() is not always the container for this position.
    // For nodes which editingIgnoresContent(node()) returns true, positions like [ignoredNode, 0]
    // will be treated as before ignoredNode (thus node() is really after the position, not containing it).
    KRYS_NODISCARD Node *DeprecatedNode() const noexcept
    {
      return _anchorNode.get();
    }

    KRYS_NODISCARD Document *GetDocument() const noexcept
    {
      return _anchorNode ? &_anchorNode->NodeDocument() : nullptr;
    }

    KRYS_NODISCARD Element *RootEditableElement() const noexcept
    {
      auto *container = GetContainerNode();
      return container ? container->RootEditableElement() : nullptr;
    }

    // These should only be used for PositionIsOffsetInAnchor positions, unless
    // the position is a legacy editing position.
    void MoveToPosition(Node *anchorNode, uint32 offset) noexcept;
    void MoveToOffset(uint32 offset) noexcept;

    KRYS_NODISCARD bool IsNull() const noexcept
    {
      return !_anchorNode;
    }
    KRYS_NODISCARD bool IsNotNull() const noexcept
    {
      return _anchorNode != nullptr;
    }
    KRYS_NODISCARD bool IsOrphan() const noexcept
    {
      return _anchorNode && !_anchorNode->IsConnected();
    }

    KRYS_NODISCARD RefPtr<Element> AnchorElementAncestor() const noexcept;

    // Move up or down the DOM by one position.
    // Offsets are computed using render text for nodes that have renderers - but note that even when
    // using composed characters, the result may be inside a single user-visible character if a ligature is
    // formed.
    KRYS_NODISCARD Position Previous(PositionMoveType moveType = PositionMoveType::CodePoint) const noexcept;
    KRYS_NODISCARD Position Next(PositionMoveType moveType = PositionMoveType::CodePoint) const noexcept;
    KRYS_NODISCARD static int32 UncheckedPreviousOffset(const Node *node, uint32 current) noexcept;
    KRYS_NODISCARD static int32 UncheckedPreviousOffsetForBackwardDeletion(const Node *node,
                                                                           uint32 current) noexcept;
    KRYS_NODISCARD static int32 UncheckedNextOffset(const Node *node, uint32 current) noexcept;

    // These can be either inside or just before/after the node, depending on
    // if the node is ignored by editing or not.
    // FIXME: These should go away. They only make sense for legacy positions.
    KRYS_NODISCARD bool AtFirstEditingPositionForNode() const noexcept;
    KRYS_NODISCARD bool AtLastEditingPositionForNode() const noexcept;

    // Returns true if the visually equivalent positions around have different editability
    KRYS_NODISCARD bool AtEditingBoundary() const noexcept;
    KRYS_NODISCARD RefPtr<Node> ParentEditingBoundary() const noexcept;

    KRYS_NODISCARD bool AtStartOfTree() const noexcept;
    KRYS_NODISCARD bool AtEndOfTree() const noexcept;

    // FIXME: Make these non-member functions and put them somewhere in the editing directory.
    // These aren't really basic "position" operations. More high level editing helper functions.
    KRYS_NODISCARD Position LeadingWhitespacePosition(
      Affinity affinity, bool considerNonCollapsibleWhitespace = false) const noexcept;
    KRYS_NODISCARD Position TrailingWhitespacePosition(
      Affinity affinity, bool considerNonCollapsibleWhitespace = false) const noexcept;

    // These return useful visually equivalent positions.
    KRYS_NODISCARD Position
      Upstream(EditingBoundaryCrossingRule rule =
                 EditingBoundaryCrossingRule::CannotCrossEditingBoundary) const noexcept;
    KRYS_NODISCARD Position
      Downstream(EditingBoundaryCrossingRule rule =
                   EditingBoundaryCrossingRule::CannotCrossEditingBoundary) const noexcept;

    KRYS_NODISCARD bool IsCandidate() const noexcept;
    KRYS_NODISCARD bool IsRenderedCharacter() const noexcept;
    KRYS_NODISCARD bool RendersInDifferentPosition(const Position &position) const noexcept;

    KRYS_NODISCARD InlineBoxAndOffset GetInlineBoxAndOffset(Affinity affinity) const noexcept;
    KRYS_NODISCARD InlineBoxAndOffset GetInlineBoxAndOffset(Affinity affinity,
                                                            TextDirection primaryDirection) const noexcept;

    KRYS_NODISCARD TextDirection PrimaryDirection() const noexcept;

    // Returns the number of positions that exist between two positions.
    KRYS_NODISCARD static uint32 PositionCountBetweenPositions(const Position &a, const Position &b) noexcept;

    KRYS_NODISCARD static bool
      HasRenderedNonAnonymousDescendantsWithHeight(const RenderElement &renderer) noexcept;
    KRYS_NODISCARD static bool NodeIsUserSelectNone(const Node *node) noexcept;
    KRYS_NODISCARD static bool NodeIsUserSelectAll(const Node *node) noexcept;
    KRYS_NODISCARD static RefPtr<Node> RootUserSelectAllForNode(Node *node) noexcept;

    // This is a tentative enhancement of operator== to account for different position types.
    // FIXME: Combine this function with operator==
    KRYS_NODISCARD bool Equals(const Position &other) const noexcept;

    // FIXME: Positions at the same document location with different anchoring will return false; that's
    // unlike <= and >=.
    // FIXME: This ignores differences in _isLegacyEditingPosition in a subtle way.
    // FIXME: For legacy editing positions, <div><img></div> [div, 0] != [img, 0] even though most of editing
    // code will treat them as identical.
    KRYS_NODISCARD friend bool operator==(const Position &a, const Position &b) noexcept
    {
      return a.AnchorNode() == b.AnchorNode() && a.DeprecatedEditingOffset() == b.DeprecatedEditingOffset()
             && a.GetAnchorType() == b.GetAnchorType();
    }

  private:
    Position(RefPtr<Node> &&anchorNode, uint32 offset, LegacyEditingPositionFlag) noexcept;

    KRYS_NODISCARD int32 OffsetForPositionAfterAnchor() const noexcept;

    KRYS_NODISCARD Position PreviousCharacterPosition(Affinity affinity) const noexcept;

    KRYS_NODISCARD static AnchorType AnchorTypeForLegacyEditingPosition(Node *anchorNode,
                                                                        uint32 offset) noexcept;
  };

  template <TreeType = TreeType::Tree>
  KRYS_NODISCARD std::partial_ordering TreeOrder(const Position &a, const Position &b) noexcept;
  KRYS_NODISCARD std::partial_ordering operator<=>(const Position &a, const Position &b) noexcept;

  KRYS_NODISCARD inline Position MakeContainerOffsetPosition(RefPtr<Node> &&node, uint32 offset) noexcept
  {
    return {krys::move(node), offset, Position::AnchorType::PositionIsOffsetInAnchor};
  }
  KRYS_NODISCARD Position MakeContainerOffsetPosition(const BoundaryPoint &point) noexcept;

  KRYS_NODISCARD Position MakeDeprecatedLegacyPosition(RefPtr<Node> &&node, uint32 offset) noexcept;
  KRYS_NODISCARD Position MakeDeprecatedLegacyPosition(const BoundaryPoint &point) noexcept;

  KRYS_NODISCARD Maybe<BoundaryPoint> MakeBoundaryPoint(const Position &position) noexcept;

  KRYS_NODISCARD Position PositionInParentBeforeNode(Node *node) noexcept;
  KRYS_NODISCARD Position PositionInParentAfterNode(Node *node) noexcept;

  // PositionBeforeNode and PositionAfterNode return neighbor-anchored positions, construction is O(1)
  KRYS_NODISCARD inline Position PositionBeforeNode(Node *anchorNode) noexcept
  {
    krys_debug_assert(anchorNode);
    return {ShareRefPtr(anchorNode), Position::AnchorType::PositionIsBeforeAnchor};
  }

  KRYS_NODISCARD inline Position PositionAfterNode(Node *anchorNode) noexcept
  {
    krys_debug_assert(anchorNode);
    return {ShareRefPtr(anchorNode), Position::AnchorType::PositionIsAfterAnchor};
  }

  // FirstPositionInNode and LastPositionInNode return parent-anchored positions, LastPositionInNode
  // construction is O(n) due to CountChildNodes()
  KRYS_NODISCARD Position FirstPositionInNode(Node *anchorNode) noexcept;
  KRYS_NODISCARD Position LastPositionInNode(Node *anchorNode) noexcept;

  KRYS_NODISCARD bool OffsetIsBeforeLastNodeOffset(uint32 offset, Node *anchorNode) noexcept;

  KRYS_NODISCARD Node *CommonInclusiveAncestor(const Position &a, const Position &b) noexcept;

  /// @brief A pair of positions, not necessarily in order.
  struct PositionRange
  {
    Position Start;
    Position End;
  };

  KRYS_NODISCARD Maybe<SimpleRange> MakeSimpleRange(const PositionRange &range) noexcept;

  /// @brief A position and which side of a line break it belongs to.
  class PositionWithAffinity
  {
  private:
    Position _position;
    Affinity _affinity {Affinity::Downstream};

  public:
    PositionWithAffinity() noexcept = default;
    PositionWithAffinity(const Position &position, Affinity affinity = Affinity::Downstream) noexcept
        : _position(position), _affinity(affinity)
    {
    }

    KRYS_NODISCARD const Position &GetPosition() const noexcept
    {
      return _position;
    }
    KRYS_NODISCARD Affinity GetAffinity() const noexcept
    {
      return _affinity;
    }
  };
}
