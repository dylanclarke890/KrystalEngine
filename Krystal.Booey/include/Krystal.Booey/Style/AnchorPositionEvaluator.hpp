#pragma once

#include "Krystal.Booey/Core/WritingMode.hpp"
#include "Krystal.Booey/CSS/Properties/Property.hpp"
#include "Krystal.Booey/CSS/Values/ValueId.hpp"
#include "Krystal.Booey/Graphics/LayoutRect.hpp"
#include "Krystal.Booey/Style/AnchorSizeDimension.hpp"
#include "Krystal.Booey/Style/PseudoElementIdentifier.hpp"
#include "Krystal.Booey/Style/ResolvedScopedName.hpp"
#include "Krystal.Booey/Style/ScopedName.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Hash.hpp"
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
  // FIXME: The anchors' scroll adjustment (AnchorScrollAdjuster, captureScrollSnapshots,
  // updateScrollAdjustments) and the last successful position options (recordLastSuccessfulPositionOptions).
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
    static void
      UpdateAnchorPositionedStateForDefaultAnchorAndPositionVisibility(dom::Element &, const RenderStyle &,
                                                                       AnchorPositionedStates &) noexcept;

    KRYS_NODISCARD static LayoutRect
      ComputeAnchorRectRelativeToContainingBlock(const RenderBoxModelObject &anchorBox,
                                                 const RenderElement &containingBlock,
                                                 const RenderBox &anchoredBox) noexcept;

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
  };
}
