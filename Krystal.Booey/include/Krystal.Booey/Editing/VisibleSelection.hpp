#pragma once

#include "Krystal.Booey/DOM/Position.hpp"
#include "Krystal.Booey/DOM/SimpleRange.hpp"
#include "Krystal.Booey/Editing/EditingBoundary.hpp"
#include "Krystal.Booey/Editing/TextAffinity.hpp"
#include "Krystal.Booey/Editing/TextGranularity.hpp"
#include "Krystal.Booey/Editing/VisiblePosition.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/Maybe.hpp"

namespace krys::boo
{
  /// @brief Whether a selection keeps its anchor when extended.
  /// On Mac, Shift-arrow keys move the anchor in a strongly directional selection
  /// and moves either end to always extend in a weakly directional or non-directional selection.
  // FIXME: Add Weak.
  enum class Directionality : uint8
  {
    None,
    Strong
  };

  /// @brief Which way a selection is modified.
  enum class SelectionDirection : uint8
  {
    Forward,
    Backward,
    Right,
    Left
  };

  /// @brief A selection: an anchor and a focus, canonicalised to candidate positions and expanded to a
  /// granularity, as none, a caret or a range.
  // FIXME: Writing suggestions (canEnableWritingSuggestions), with writing suggestions.
  class VisibleSelection
  {
  private:
    enum class Type : uint8
    {
      None,
      Caret,
      Range
    };

    // We store only Positions because VisibleSelection is used to store values in
    // editing commands for use when undoing the command. We need to be able to store
    // a selection that, while currently invalid, will be valid once the changes are undone.

    // FIXME: Consider doing canonicalization only as part of editing operations, and keeping all selection
    // endpoints non-canonical outside of that code.
    // FIXME: Rename _base to _canonicalAnchor.
    // FIXME: Rename _extent to _canonicalFocus.
    dom::Position _anchor; // Where the first click happened.
    dom::Position _focus;  // Where the end click/release happened.
    dom::Position _base;   // Anchor, canonical, not yet expended to respect granularity.
    dom::Position _extent; // Focus, canonical, but not expended to respect granularity.
    dom::Position _start;  // First of anchor and focus, caonicalized and expanded to respect granularity.
    dom::Position _end;    // Last of anchor and focus, caonicalized and expanded to respect granularity.

    Affinity _affinity {DefaultAffinity};

    // These are cached, can be recalculated by Validate()
    Type _type {Type::None};
    // True if the anchor is before the focus.
    // FIXME: Rename to _anchorIsBeforeFocus since that's what the comment says.
    bool _anchorIsFirst {true};
    boo::Directionality _directionality {boo::Directionality::None};

  public:
    static constexpr auto DefaultAffinity = VisiblePosition::DefaultAffinity;

    VisibleSelection() noexcept = default;
    VisibleSelection(const dom::Position &anchor, const dom::Position &focus,
                     Affinity affinity = DefaultAffinity,
                     boo::Directionality directionality = boo::Directionality::None) noexcept;
    // FIXME: A caret selection never has direction so we should remove the Directionality argument from this
    // function.
    VisibleSelection(const dom::Position &position, Affinity affinity,
                     boo::Directionality directionality = boo::Directionality::None) noexcept;
    explicit VisibleSelection(const dom::SimpleRange &range, Affinity affinity = DefaultAffinity,
                              boo::Directionality directionality = boo::Directionality::None) noexcept;
    // FIXME: A caret selection never has direction so we should remove the Directionality argument from this
    // function.
    explicit VisibleSelection(const VisiblePosition &position,
                              boo::Directionality directionality = boo::Directionality::None) noexcept;
    VisibleSelection(const VisiblePosition &anchor, const VisiblePosition &focus,
                     boo::Directionality directionality = boo::Directionality::None) noexcept;

    KRYS_NODISCARD static const VisibleSelection &EmptySelection() noexcept;
    KRYS_NODISCARD static VisibleSelection SelectionFromContentsOfNode(dom::Node *node) noexcept;

    void SetAffinity(Affinity affinity) noexcept
    {
      _affinity = affinity;
    }
    KRYS_NODISCARD Affinity GetAffinity() const noexcept
    {
      return _affinity;
    }

    // FIXME: Move to the terms "focus" and "anchor" instead of "base" and "extent".

    void SetBase(const dom::Position &position) noexcept;
    void SetBase(const VisiblePosition &visiblePosition) noexcept;
    void SetExtent(const dom::Position &position) noexcept;
    void SetExtent(const VisiblePosition &visiblePosition) noexcept;

    // These functions return the values that were passed in, without the canonicalization done by
    // VisiblePosition.
    // FIXME: When we expand granularity, we canonicalize as a side effect, so expanded values have been made
    // canonical.
    // FIXME: Replace start/range/base/end/firstRange with these, renaming these to the shorter names.
    KRYS_NODISCARD const dom::Position &UncanonicalizedStart() const noexcept;
    KRYS_NODISCARD const dom::Position &UncanonicalizedEnd() const noexcept;
    KRYS_NODISCARD const dom::Position &Anchor() const noexcept
    {
      return _anchor;
    }
    KRYS_NODISCARD const dom::Position &Focus() const noexcept
    {
      return _focus;
    }
    KRYS_NODISCARD Maybe<dom::SimpleRange> Range() const noexcept;

    // FIXME: Rename these to include the word "canonical" or remove.
    KRYS_NODISCARD const dom::Position &Base() const noexcept
    {
      return _base;
    }
    KRYS_NODISCARD const dom::Position &Extent() const noexcept
    {
      return _extent;
    }
    KRYS_NODISCARD const dom::Position &Start() const noexcept
    {
      return _start;
    }
    KRYS_NODISCARD const dom::Position &End() const noexcept
    {
      return _end;
    }

    KRYS_NODISCARD VisiblePosition VisibleStart() const noexcept
    {
      return VisiblePosition(_start, IsRange() ? Affinity::Downstream : _affinity);
    }
    KRYS_NODISCARD VisiblePosition VisibleEnd() const noexcept
    {
      return VisiblePosition(_end, IsRange() ? Affinity::Upstream : _affinity);
    }
    KRYS_NODISCARD VisiblePosition VisibleBase() const noexcept
    {
      return VisiblePosition(_base, IsRange() ? (IsBaseFirst() ? Affinity::Upstream : Affinity::Downstream)
                                              : _affinity);
    }
    KRYS_NODISCARD VisiblePosition VisibleExtent() const noexcept
    {
      return VisiblePosition(_extent, IsRange() ? (IsBaseFirst() ? Affinity::Downstream : Affinity::Upstream)
                                                : _affinity);
    }

    operator VisiblePositionRange() const noexcept
    {
      return {VisibleStart(), VisibleEnd()};
    }

    KRYS_NODISCARD bool IsNone() const noexcept
    {
      return _type == Type::None;
    }
    KRYS_NODISCARD bool IsCaret() const noexcept
    {
      return _type == Type::Caret;
    }
    KRYS_NODISCARD bool IsRange() const noexcept
    {
      return _type == Type::Range;
    }
    KRYS_NODISCARD bool IsCaretOrRange() const noexcept
    {
      return !IsNone();
    }
    KRYS_NODISCARD bool IsNonOrphanedRange() const noexcept
    {
      return IsRange() && !_start.IsOrphan() && !_end.IsOrphan();
    }
    KRYS_NODISCARD bool IsNoneOrOrphaned() const noexcept
    {
      return IsNone() || _start.IsOrphan() || _end.IsOrphan();
    }
    KRYS_NODISCARD bool IsOrphan() const noexcept;
    KRYS_NODISCARD RefPtr<dom::Document> GetDocument() const noexcept;

    KRYS_NODISCARD bool IsBaseFirst() const noexcept
    {
      return _anchorIsFirst;
    }

    KRYS_NODISCARD boo::Directionality GetDirectionality() const noexcept
    {
      return _directionality;
    }
    void SetDirectionality(boo::Directionality directionality) noexcept
    {
      _directionality = directionality;
    }

    KRYS_NODISCARD bool IsAll(EditingBoundaryCrossingRule rule) const noexcept;

    void AppendTrailingWhitespace() noexcept;

    bool ExpandUsingGranularity(TextGranularity granularity) noexcept;

    // FIXME: Rename to include the word "canonical" and remove the word "first" or remove.
    // We don't yet support multi-range selections, so we only ever have one range to return.
    KRYS_NODISCARD Maybe<dom::SimpleRange> FirstRange() const noexcept;

    // FIXME: Most callers probably don't want this function, and should use range instead.
    // This function contracts the range around text and moves the caret upstream before returning the range.
    KRYS_NODISCARD Maybe<dom::SimpleRange> ToNormalizedRange() const noexcept;

    KRYS_NODISCARD dom::Element *RootEditableElement() const noexcept;

    /// @brief Whether the selection starts in a password field.
    KRYS_NODISCARD bool IsInPasswordField() const noexcept;

    // FIXME: Fields the host autofilled and lets the user see (autofilledAndViewable), with autofill.
    KRYS_NODISCARD bool IsInAutoFilledAndViewableField() const noexcept
    {
      return false;
    }
    KRYS_NODISCARD bool IsContentEditable() const noexcept;
    KRYS_NODISCARD bool HasEditableStyle() const noexcept;
    KRYS_NODISCARD bool IsContentRichlyEditable() const noexcept;

    // Returns a child of the ShadowRoot node for shadow trees, or null for non-shadow trees.
    KRYS_NODISCARD dom::Node *NonBoundaryShadowTreeRootNode() const noexcept;

    KRYS_NODISCARD static dom::Position AdjustPositionForEnd(const dom::Position &currentPosition,
                                                             dom::Node *startContainerNode) noexcept;
    KRYS_NODISCARD static dom::Position AdjustPositionForStart(const dom::Position &currentPosition,
                                                               dom::Node *endContainerNode) noexcept;

    void SetWithoutValidation(const dom::Position &anchor, const dom::Position &focus) noexcept;

    KRYS_NODISCARD friend bool operator==(const VisibleSelection &a, const VisibleSelection &b) noexcept
    {
      return a.Start() == b.Start() && a.End() == b.End() && a.GetAffinity() == b.GetAffinity()
             && a.IsBaseFirst() == b.IsBaseFirst() && a.GetDirectionality() == b.GetDirectionality();
    }

  private:
    void Validate(TextGranularity granularity = TextGranularity::CharacterGranularity) noexcept;

    // Support methods for Validate()
    void SetBaseAndExtentToDeepEquivalents() noexcept;
    void AdjustSelectionRespectingGranularity(TextGranularity granularity) noexcept;
    void AdjustSelectionToAvoidCrossingShadowBoundaries() noexcept;
    void AdjustSelectionToAvoidCrossingEditingBoundaries() noexcept;
    void UpdateSelectionType() noexcept;
  };
}
