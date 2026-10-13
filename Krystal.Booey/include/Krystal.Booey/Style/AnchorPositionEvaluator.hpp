#pragma once

#include "Krystal.Booey/Core/WritingMode.hpp"
#include "Krystal.Booey/CSS/Values/ValueId.hpp"
#include "Krystal.Booey/DOM/EventTarget.hpp"
#include "Krystal.Booey/Style/AnchorSizeDimension.hpp"
#include "Krystal.Core/Types/CheckedPtr.hpp"
#include "Krystal.Core/Types/HashMap.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/SmallList.hpp"
#include "Krystal.Core/Types/UniquePtr.hpp"
#include "Krystal.Core/Types/WeakPtr.hpp"
#include <WebCore/LayoutPoint.h>
#include <WebCore/LayoutSize.h>
#include <WebCore/LayoutUnit.h>
#include <WebCore/PositionTryOrder.h>
#include <WebCore/PseudoElementIdentifier.h>
#include <WebCore/ResolvedScopedName.h>
#include <WebCore/ScopedName.h>
#include <wtf/WeakHashMap.h>
#include <wtf/WeakHashSet.h>

namespace krys::boo::dom
{
  class Document;
  class Element;
}

namespace krys::boo::css
{
  enum class PropertyId : uint16;
}

namespace krys::boo
{
  class LayoutPoint;
  class LayoutRect;
  class LayoutSize;
  class RenderBlock;
  class RenderBox;
  class RenderBoxModelObject;
  class RenderElement;
  class RenderStyle;
  class RenderView;

  struct AnchorScrollSnapshot
  {
    WeakPtr<const RenderBox> _scroller;
    LayoutPoint _scrollSnapshot {};

    AnchorScrollSnapshot(const RenderBox &scroller, LayoutPoint snapshot) noexcept;
    AnchorScrollSnapshot(LayoutPoint snapshot) noexcept;

    inline LayoutSize AdjustmentForCurrentScrollPosition() const noexcept;

    bool operator==(const AnchorScrollSnapshot &) const noexcept = default;
  };

  class AnchorScrollAdjuster
  {
  private:
    CheckedRef<RenderBox> _anchored;
    SmallList<AnchorScrollSnapshot, 1> _scrollSnapshots;
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

    RenderBox *Anchored() const noexcept;

    inline bool IsEmpty() const noexcept;

    bool MayNeedAdjustment() const noexcept
    {
      return _needsXAdjustment || _needsYAdjustment;
    }

    bool MayNeedXAdjustment() const noexcept
    {
      return _needsXAdjustment;
    }

    bool MayNeedYAdjustment() const noexcept
    {
      return _needsYAdjustment;
    }

    bool IsHidden() const noexcept
    {
      return _isHidden;
    }

    void SetHidden(bool hide) noexcept
    {
      _isHidden = hide;
    }

    inline void AddSnapshot(const RenderBox &scroller) noexcept;

    inline void AddViewportSnapshot(const RenderView &) noexcept;

    bool HasViewportSnapshot() const noexcept
    {
      return _adjustForViewport;
    }

    enum Diff : uint8
    {
      New,
      SnapshotsDiffer,
      SnapshotsMatch
    };

    // Snapshot differences can require invalidation.
    bool RecaptureDiffers(const AnchorScrollAdjuster &) const noexcept;

    void SetFallbackLimits(const RenderBox &anchored) noexcept;

    bool HasFallbackLimits() const noexcept
    {
      return _hasFallback;
    }

    bool ExceedsFallbackLimits(LayoutSize adjustment) noexcept
    {
      return !_fallbackLimits.fits(adjustment);
    }

    LayoutSize AccumulateAdjustments(const RenderView &, const RenderBox &anchored) const noexcept;

    bool InvalidateForScroller(const RenderBox &scroller) noexcept;

  private:
    LayoutSize AdjustmentForViewport(const RenderView &) const noexcept;
  };
}

namespace krys::boo::style
{
  class BuilderState;
  struct BuilderPositionTryFallback;

  enum class AnchorPositionResolutionStage : uint8_t
  {
    FindAnchors,
    ResolveAnchorFunctions,
    Resolved,
    Positioned,
  };

  using AnchorElements = HashMap<ResolvedScopedName, WeakPtr<dom::Element>>;

  struct AnchorPositionedState
  {
    AnchorElements anchorElements;
    HashSet<ResolvedScopedName> anchorNames;
    AnchorPositionResolutionStage stage;
  };

  using AnchorPositionedKey = std::pair<RefPtr<const dom::Element>, Maybe<PseudoElementIdentifier>>;
  using AnchorPositionedStates = HashMap<AnchorPositionedKey, UniquePtr<AnchorPositionedState>>;

  using AnchorsForAnchorName = HashMap<ResolvedScopedName, SmallList<WeakRef<const RenderBoxModelObject>>>;

  struct ResolvedAnchor
  {
    WeakPtr<RenderBoxModelObject> renderer;
    ResolvedScopedName name;
  };

  struct AnchorPositionedToAnchorEntry
  {
    // The pseudo-element identifier can be used to access the AnchorPositionedState struct
    // of the current element in an AnchorPositionedStates map, in combination with the relevant
    // Element object.
    std::optional<PseudoElementIdentifier> pseudoElementIdentifier;

    SmallList<ResolvedAnchor> anchors;
  };

  using AnchorPositionedToAnchorMap =
    WeakHashMap<Element, AnchorPositionedToAnchorEntry, WeakPtrImplWithEventTargetData>;
  using AnchorToAnchorPositionedMap =
    SingleThreadWeakHashMap<const RenderBoxModelObject, SmallList<Ref<dom::Element>>>;

  class AnchorPositionEvaluator
  {
  public:
    using Side = Variant<css::ValueId, double>;
    static bool propertyAllowsAnchorFunction(css::PropertyId);
    static std::optional<double> evaluate(BuilderState &, std::optional<ScopedName> elementName, Side);

    static bool propertyAllowsAnchorSizeFunction(css::PropertyId);
    static std::optional<double> evaluateSize(BuilderState &, std::optional<ScopedName> elementName,
                                              std::optional<AnchorSizeDimension>);

    static void updateAnchorPositioningStatesAfterInterleavedLayout(dom::Document &,
                                                                    AnchorPositionedStates &);

    static void updateScrollAdjustments(RenderView &);

    static void updateAnchorPositionedStateForDefaultAnchorAndPositionVisibility(dom::Element &,
                                                                                 const RenderStyle &,
                                                                                 AnchorPositionedStates &);

    static LayoutRect
      computeAnchorRectRelativeToContainingBlock(CheckedRef<const RenderBoxModelObject> anchorBox,
                                                 const RenderElement &containingBlock,
                                                 const RenderBox &anchoredBox);

    static void captureScrollSnapshots(RenderBox &anchored,
                                       bool invalidateStyleForScrollPositionChanges = true);

    static AnchorToAnchorPositionedMap makeAnchorPositionedForAnchorMap(AnchorPositionedToAnchorMap &);

    static bool isAnchorPositioned(const RenderStyle &);

    static bool isStyleTimeAnchorPositioned(const RenderStyle &);

    static bool isLayoutTimeAnchorPositioned(const RenderStyle &);

    static css::PropertyId resolvePositionTryFallbackProperty(css::PropertyId, WritingMode,
                                                              const BuilderPositionTryFallback &);

    static css::ValueId resolvePositionTryFallbackValueForSelfPosition(css::PropertyId, css::ValueId,
                                                                       WritingMode,
                                                                       const BuilderPositionTryFallback &);

    static bool overflowsInsetModifiedContainingBlock(const RenderBox &anchoredBox);

    static bool isDefaultAnchorInvisibleOrClippedByInterveningBoxes(const RenderBox &anchoredBox);

    static ScopedName defaultAnchorName(const RenderStyle &);

    static bool isAnchor(const RenderStyle &);

    static bool isImplicitAnchor(const RenderStyle &);

    static CheckedPtr<RenderBoxModelObject> defaultAnchorForBox(const RenderBox &);

    static HashMap<AnchorPositionedKey, size_t>
      recordLastSuccessfulPositionOptions(const SingleThreadWeakHashSet<const RenderBox> &positionTryBoxes);

  private:
    static CheckedPtr<RenderBoxModelObject>
      findAnchorForAnchorFunctionAndAttemptResolution(BuilderState &, std::optional<ScopedName> elementName);

    static AnchorElements findAnchorsForAnchorPositionedElement(
      const dom::Element &, const HashSet<ResolvedScopedName> &anchorNames, const AnchorsForAnchorName &);

    static RefPtr<const dom::Element> anchorPositionedElementOrPseudoElement(BuilderState &);

    static AnchorPositionedKey keyForElementOrPseudoElement(const dom::Element &);

    static void addAnchorFunctionScrollCompensatedAxis(RenderStyle &, const RenderBox &anchored,
                                                       const RenderBoxModelObject &anchor, BoxAxis);

    static LayoutSize scrollOffsetFromAnchor(const RenderBoxModelObject &anchor, const RenderBox &anchored);
  };
}
