#pragma once

#include "Krystal.Booey/Core/BoxSides.hpp"
#include "Krystal.Booey/Core/WritingMode.hpp"
#include "Krystal.Booey/CSS/Properties/Property.hpp"
#include "Krystal.Booey/CSS/Values/ValueId.hpp"
#include "Krystal.Booey/Graphics/LayoutPoint.hpp"
#include "Krystal.Booey/Graphics/LayoutRect.hpp"
#include "Krystal.Booey/Graphics/LayoutSize.hpp"
#include "Krystal.Booey/Style/AnchorSizeDimension.hpp"
#include "Krystal.Booey/Style/PseudoElementIdentifier.hpp"
#include "Krystal.Booey/Style/ResolvedScopedName.hpp"
#include "Krystal.Booey/Style/ScopedName.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Hash.hpp"
#include "Krystal.Core/Types/CheckedPtr.hpp"
#include "Krystal.Core/Types/HashMap.hpp"
#include "Krystal.Core/Types/HashSet.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/Pair.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include "Krystal.Core/Types/UniquePtr.hpp"
#include "Krystal.Core/Types/Variant.hpp"
#include "Krystal.Core/Types/WeakPtr.hpp"

namespace krys::boo::dom
{
  class Document;
  class Element;
}

namespace krys::boo
{
  class RenderBox;
  class RenderBoxModelObject;
  class RenderElement;
  class RenderStyle;
  class RenderView;

  /// @brief A scroller's scroll position, or the viewport's (without a scroller), when an anchor-positioned
  /// box was laid out.
  struct AnchorScrollSnapshot
  {
    WeakPtr<const RenderBox> Scroller;
    LayoutPoint ScrollSnapshot;

    AnchorScrollSnapshot(const RenderBox &scroller, LayoutPoint snapshot) noexcept;
    explicit AnchorScrollSnapshot(LayoutPoint snapshot) noexcept;

    KRYS_NODISCARD LayoutSize AdjustmentForCurrentScrollPosition() const noexcept;
    KRYS_NODISCARD bool operator==(const AnchorScrollSnapshot &) const noexcept;
  };

  /// @brief Shifts an anchor-positioned box by how far the scrollers between it and its default anchor have
  /// scrolled since it was laid out: its default scroll shift.
  /// @see https://drafts.csswg.org/css-anchor-position-1/#scroll
  class AnchorScrollAdjuster
  {
  public:
    enum class Diff : uint8
    {
      New,
      SnapshotsDiffer,
      SnapshotsMatch
    };

  private:
    CheckedRef<RenderBox> _anchored;
    List<AnchorScrollSnapshot> _scrollSnapshots;
    bool _needsXAdjustment : 1 {false};
    bool _needsYAdjustment : 1 {false};
    bool _adjustForViewport : 1 {false};
    bool _hasChainedAnchor : 1 {false};
    bool _hasStickyAnchor : 1 {false};
    bool _isHidden : 1 {false};
    bool _hasFallback : 1 {false};
    LayoutSize _stickySnapshot;
    LayoutSizeLimits _fallbackLimits;

  public:
    AnchorScrollAdjuster(RenderBox &anchored, const RenderBoxModelObject &defaultAnchor) noexcept;
    AnchorScrollAdjuster(const AnchorScrollAdjuster &) noexcept;
    AnchorScrollAdjuster(AnchorScrollAdjuster &&) noexcept;
    ~AnchorScrollAdjuster() noexcept;

    AnchorScrollAdjuster &operator=(const AnchorScrollAdjuster &) noexcept;
    AnchorScrollAdjuster &operator=(AnchorScrollAdjuster &&) noexcept;

    KRYS_NODISCARD RenderBox *Anchored() const noexcept;

    KRYS_NODISCARD bool IsEmpty() const noexcept;
    KRYS_NODISCARD bool MayNeedAdjustment() const noexcept
    {
      return _needsXAdjustment || _needsYAdjustment;
    }
    KRYS_NODISCARD bool MayNeedXAdjustment() const noexcept
    {
      return _needsXAdjustment;
    }
    KRYS_NODISCARD bool MayNeedYAdjustment() const noexcept
    {
      return _needsYAdjustment;
    }

    KRYS_NODISCARD bool IsHidden() const noexcept
    {
      return _isHidden;
    }
    void SetHidden(bool hide) noexcept
    {
      _isHidden = hide;
    }

    void AddSnapshot(const RenderBox &scroller) noexcept;
    void AddViewportSnapshot(const RenderView &) noexcept;
    KRYS_NODISCARD bool HasViewportSnapshot() const noexcept
    {
      return _adjustForViewport;
    }

    // Snapshot differences can require invalidation.
    KRYS_NODISCARD bool RecaptureDiffers(const AnchorScrollAdjuster &) const noexcept;

    void SetFallbackLimits(const RenderBox &anchored) noexcept;
    KRYS_NODISCARD bool HasFallbackLimits() const noexcept
    {
      return _hasFallback;
    }
    KRYS_NODISCARD bool ExceedsFallbackLimits(LayoutSize adjustment) const noexcept
    {
      return !_fallbackLimits.Fits(adjustment);
    }

    KRYS_NODISCARD LayoutSize AccumulateAdjustments(const RenderView &,
                                                    const RenderBox &anchored) const noexcept;

    bool InvalidateForScroller(const RenderBox &scroller) noexcept;

  private:
    KRYS_NODISCARD LayoutSize AdjustmentForViewport(const RenderView &) const noexcept;
  };
}

namespace krys::boo::style
{
  class BuilderState;
  struct BuilderPositionTryFallback;

  enum class AnchorPositionResolutionStage : uint8
  {
    FindAnchors,
    ResolveAnchorFunctions,
    Resolved,
    Positioned,
  };

  using AnchorElements = HashMap<ResolvedScopedName, WeakPtr<dom::Element>>;

  struct AnchorPositionedState
  {
    style::AnchorElements AnchorElements;
    HashSet<ResolvedScopedName> AnchorNames;
    AnchorPositionResolutionStage Stage {AnchorPositionResolutionStage::FindAnchors};
  };

  using AnchorPositionedKey = Pair<RefPtr<const dom::Element>, Maybe<PseudoElementIdentifier>>;

  /// @brief Hashes an anchor-positioned element or pseudo-element, for maps keyed by it.
  struct AnchorPositionedKeyHash
  {
    KRYS_NODISCARD size_t operator()(const AnchorPositionedKey &key) const noexcept
    {
      auto pseudoHash = key.second ? DefaultHash<PseudoElementIdentifier>::Hash(*key.second) : 0uz;
      return UnsignedIntegerPairHash(std::hash<const dom::Element *> {}(key.first.get()), pseudoHash);
    }
  };

  using AnchorPositionedStates =
    HashMap<AnchorPositionedKey, UniquePtr<AnchorPositionedState>, AnchorPositionedKeyHash>;

  using AnchorsForAnchorName = HashMap<ResolvedScopedName, List<const RenderBoxModelObject *>>;

  struct ResolvedAnchor
  {
    WeakPtr<RenderBoxModelObject> Renderer;
    ResolvedScopedName Name;
  };

  struct AnchorPositionedToAnchorEntry
  {
    // The anchor-positioned element: the map is keyed by its address, and this tells a destroyed element from
    // a new one at the same address.
    WeakPtr<dom::Element> Element;

    // The pseudo-element identifier can be used to access the AnchorPositionedState struct of the current
    // element in an AnchorPositionedStates map, in combination with the relevant Element object.
    Maybe<style::PseudoElementIdentifier> PseudoElementIdentifier;

    List<ResolvedAnchor> Anchors;
  };

  using AnchorPositionedToAnchorMap = HashMap<const dom::Element *, AnchorPositionedToAnchorEntry>;
  using AnchorToAnchorPositionedMap = HashMap<const RenderBoxModelObject *, List<Ref<dom::Element>>>;

  /// @brief Resolves anchor positioning: finds the anchors of anchor-positioned boxes and evaluates the
  /// `anchor()` and `anchor-size()` functions against their geometry.
  /// @see https://drafts.csswg.org/css-anchor-position-1/
  class AnchorPositionEvaluator
  {
  public:
    using Side = Variant<css::ValueId, double>;

    AnchorPositionEvaluator() = delete;

    /// @brief Whether @p property takes `anchor()`: the inset properties.
    KRYS_NODISCARD static bool PropertyAllowsAnchorFunction(css::PropertyId property) noexcept
    {
      return css::Property::IsInsetProperty(property);
    }

    KRYS_NODISCARD static Maybe<double> Evaluate(BuilderState &, Maybe<ScopedName> elementName,
                                                 Side) noexcept;

    /// @brief Whether @p property takes `anchor-size()`: the sizing, inset and margin properties.
    KRYS_NODISCARD static bool PropertyAllowsAnchorSizeFunction(css::PropertyId property) noexcept
    {
      return css::Property::IsSizingProperty(property) || css::Property::IsInsetProperty(property)
             || css::Property::IsMarginProperty(property);
    }

    KRYS_NODISCARD static Maybe<double> EvaluateSize(BuilderState &, Maybe<ScopedName> elementName,
                                                     Maybe<AnchorSizeDimension>) noexcept;

    static void UpdateAnchorPositioningStatesAfterInterleavedLayout(dom::Document &,
                                                                    AnchorPositionedStates &) noexcept;
    static void UpdateScrollAdjustments(RenderView &) noexcept;
    static void
      UpdateAnchorPositionedStateForDefaultAnchorAndPositionVisibility(dom::Element &, const RenderStyle &,
                                                                       AnchorPositionedStates &) noexcept;

    KRYS_NODISCARD static LayoutRect
      ComputeAnchorRectRelativeToContainingBlock(const RenderBoxModelObject &anchorBox,
                                                 const RenderElement &containingBlock,
                                                 const RenderBox &anchoredBox) noexcept;
    static void CaptureScrollSnapshots(RenderBox &anchored,
                                       bool invalidateStyleForScrollPositionChanges = true) noexcept;

    KRYS_NODISCARD static AnchorToAnchorPositionedMap
      MakeAnchorPositionedForAnchorMap(AnchorPositionedToAnchorMap &) noexcept;

    KRYS_NODISCARD static bool IsAnchorPositioned(const RenderStyle &) noexcept;
    KRYS_NODISCARD static bool IsStyleTimeAnchorPositioned(const RenderStyle &) noexcept;
    KRYS_NODISCARD static bool IsLayoutTimeAnchorPositioned(const RenderStyle &) noexcept;

    KRYS_NODISCARD static css::PropertyId
      ResolvePositionTryFallbackProperty(css::PropertyId, WritingMode,
                                         const BuilderPositionTryFallback &) noexcept;
    /// @brief Swaps a self-alignment position (`align-self`/`justify-self`) according to a position-try
    /// fallback's tactics.
    KRYS_NODISCARD static css::ValueId
      ResolvePositionTryFallbackValueForSelfPosition(css::PropertyId, css::ValueId position, WritingMode,
                                                     const BuilderPositionTryFallback &) noexcept;

    KRYS_NODISCARD static bool OverflowsInsetModifiedContainingBlock(const RenderBox &anchoredBox) noexcept;
    KRYS_NODISCARD static bool
      IsDefaultAnchorInvisibleOrClippedByInterveningBoxes(const RenderBox &anchoredBox) noexcept;

    KRYS_NODISCARD static ScopedName DefaultAnchorName(const RenderStyle &) noexcept;
    KRYS_NODISCARD static bool IsAnchor(const RenderStyle &) noexcept;
    KRYS_NODISCARD static bool IsImplicitAnchor(const RenderStyle &) noexcept;

    KRYS_NODISCARD static RenderBoxModelObject *DefaultAnchorForBox(const RenderBox &) noexcept;

    /// @brief The index of the position option each of @p positionTryBoxes is using.
    KRYS_NODISCARD static HashMap<AnchorPositionedKey, size_t, AnchorPositionedKeyHash>
      RecordLastSuccessfulPositionOptions(const HashSet<const RenderBox *> &positionTryBoxes) noexcept;

  private:
    KRYS_NODISCARD static RenderBoxModelObject *
      FindAnchorForAnchorFunctionAndAttemptResolution(BuilderState &, Maybe<ScopedName> elementName) noexcept;
    KRYS_NODISCARD static AnchorElements
      FindAnchorsForAnchorPositionedElement(const dom::Element &,
                                            const HashSet<ResolvedScopedName> &anchorNames,
                                            const AnchorsForAnchorName &) noexcept;
    KRYS_NODISCARD static RefPtr<const dom::Element>
      AnchorPositionedElementOrPseudoElement(BuilderState &) noexcept;
    KRYS_NODISCARD static AnchorPositionedKey KeyForElementOrPseudoElement(const dom::Element &) noexcept;
    static void AddAnchorFunctionScrollCompensatedAxis(RenderStyle &, const RenderBox &anchored,
                                                       const RenderBoxModelObject &anchor, BoxAxis) noexcept;
  };
}
