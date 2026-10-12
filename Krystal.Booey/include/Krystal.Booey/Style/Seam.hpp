#pragma once

#include "Krystal.Booey/Core/WritingMode.hpp"
#include "Krystal.Booey/CSS/Queries/ContainerQueryAxis.hpp"
#include "Krystal.Booey/CSS/Types/CSSOMString.hpp"
#include "Krystal.Booey/CSS/Values/Primitives/Range.hpp"
#include "Krystal.Booey/CSS/Values/ValueId.hpp"
#include "Krystal.Booey/Graphics/FloatSize.hpp"
#include "Krystal.Booey/Style/AnchorSizeDimension.hpp"
#include "Krystal.Booey/Style/ScopedName.hpp"
#include "Krystal.Booey/Style/ScopeOrdinal.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/CanMakeCheckedPtr.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Numeric.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include "Krystal.Core/Types/Variant.hpp"

// FIXME: Placeholders for the rendering, font and style-resolver types that length resolution and calc
// evaluation need. Each declares only the members those callers use, under its upstream name. None of them
// can be constructed, so no caller can reach these members and every definition is unreachable. Each type
// moves to its own header, with its real implementation, when its slice is ported:
//   - `FontMetrics`, `FontCascadeDescription`, `FontCascade`: platform/graphics fonts.
//   - `RenderStyle`, `RenderView`, `RenderBox`: rendering.
//   - `style::BuilderState`, `style::AnchorPositionEvaluator`, `style::ContainerQueryEvaluator`: the style
//     resolver (`Style/BuilderState.hpp`, `Style/AnchorPositionEvaluator.hpp`, unreviewed).
// A few members collapse an upstream call chain whose types aren't ported yet; they're marked below.

namespace krys::boo::dom
{
  class Document;
  class Element;
}

namespace krys::boo::css
{
  class ToLengthConversionData;
}

namespace krys::boo::css::calc
{
  struct RandomCachingKey;
}

namespace krys::boo::style
{
  struct ZoomFactor;
}

namespace krys::boo
{
  class FontMetrics
  {
    KRYS_NON_COPY_MOVABLE(FontMetrics);

  public:
    FontMetrics() = delete;

    KRYS_NODISCARD int IntAscent() const noexcept;
    KRYS_NODISCARD int IntLineSpacing() const noexcept;
    KRYS_NODISCARD Maybe<float> XHeight() const noexcept;
    KRYS_NODISCARD Maybe<float> CapHeight() const noexcept;
    KRYS_NODISCARD Maybe<float> ZeroWidth() const noexcept;
    KRYS_NODISCARD Maybe<float> IdeogramWidth() const noexcept;
  };

  class FontCascadeDescription
  {
    KRYS_NON_COPY_MOVABLE(FontCascadeDescription);

  public:
    FontCascadeDescription() = delete;

    KRYS_NODISCARD float ComputedSize() const noexcept;
    KRYS_NODISCARD float ComputedSizeForRangeZoomOption(css::RangeZoomOptions) const noexcept;
    KRYS_NODISCARD float SpecifiedSize() const noexcept;
  };

  class FontCascade
  {
    KRYS_NON_COPY_MOVABLE(FontCascade);

  public:
    FontCascade() = delete;

    KRYS_NODISCARD const FontCascadeDescription &GetFontDescription() const noexcept;
    KRYS_NODISCARD const FontMetrics &MetricsOfPrimaryFont() const noexcept;
    KRYS_NODISCARD float ZeroWidth() const noexcept;
  };

  class RenderStyle
  {
    KRYS_NON_COPY_MOVABLE(RenderStyle);

  public:
    RenderStyle() = delete;

    KRYS_NODISCARD WritingMode GetWritingMode() const noexcept;
    KRYS_NODISCARD float Zoom() const noexcept;
    KRYS_NODISCARD float UsedZoom() const noexcept;
    KRYS_NODISCARD float DeviceScaleFactor() const noexcept;
    KRYS_NODISCARD bool EvaluationTimeZoomEnabled() const noexcept;

    KRYS_NODISCARD const FontCascade &GetFontCascade() const noexcept;
    KRYS_NODISCARD const FontCascadeDescription &GetFontDescription() const noexcept;
    KRYS_NODISCARD const FontMetrics &MetricsOfPrimaryFont() const noexcept;

    KRYS_NODISCARD float ComputedLineHeight() const noexcept;

    /// @brief Collapses `!pseudoElementType()`.
    KRYS_NODISCARD bool HasPseudoElementType() const noexcept;

    /// @brief Collapses `lineHeight().tryFixed()` evaluated to a `LayoutUnit` with `zoom`, as a float.
    KRYS_NODISCARD Maybe<float> EvaluateFixedLineHeight(const style::ZoomFactor &zoom) const noexcept;

    /// @brief Collapses `computeLineHeight(specifiedLineHeight())`.
    KRYS_NODISCARD float ComputeSpecifiedLineHeight() const noexcept;
  };

  class RenderView
  {
    KRYS_NON_COPY_MOVABLE(RenderView);

  public:
    RenderView() = delete;

    KRYS_NODISCARD const dom::Document &GetDocument() const noexcept;
    KRYS_NODISCARD float ZoomFactor() const noexcept;

    KRYS_NODISCARD FloatSize SizeForCSSSmallViewportUnits() const noexcept;
    KRYS_NODISCARD FloatSize SizeForCSSLargeViewportUnits() const noexcept;
    KRYS_NODISCARD FloatSize SizeForCSSDynamicViewportUnits() const noexcept;
    KRYS_NODISCARD FloatSize SizeForCSSDefaultViewportUnits() const noexcept;
  };

  class RenderBox
  {
    KRYS_NON_COPY_MOVABLE(RenderBox);

  public:
    RenderBox() = delete;

    KRYS_NODISCARD bool HasEligibleContainmentForSizeQuery() const noexcept;

    // Upstream returns `LayoutUnit`.
    KRYS_NODISCARD float ContentBoxWidth() const noexcept;
    KRYS_NODISCARD float ContentBoxHeight() const noexcept;
  };

  KRYS_NODISCARD float AdjustFloatForAbsoluteZoom(float, const RenderStyle &) noexcept;

  /// @brief Collapses `document.renderView()`.
  KRYS_NODISCARD const RenderView *RenderViewOf(const dom::Document &) noexcept;

  /// @brief Collapses `element.renderStyle()`.
  KRYS_NODISCARD const RenderStyle *RenderStyleOf(const dom::Element &) noexcept;

  /// @brief Collapses `dynamicDowncast<RenderBox>(element.renderer())`.
  KRYS_NODISCARD const RenderBox *RenderBoxOf(const dom::Element &) noexcept;
}

namespace krys::boo::style
{
  KRYS_NODISCARD bool EvaluationTimeZoomEnabled(const RenderStyle &) noexcept;

  class BuilderState : public CanMakeCheckedPtr<BuilderState>
  {
    KRYS_NON_COPY_MOVABLE(BuilderState);
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(BuilderState);

  public:
    BuilderState() = delete;

    KRYS_NODISCARD const RenderStyle &ParentStyle() const noexcept;
    KRYS_NODISCARD const RenderStyle *RootElementStyle() const noexcept;

    KRYS_NODISCARD const dom::Document &GetDocument() const noexcept;
    KRYS_NODISCARD const dom::Element *GetElement() const noexcept;

    KRYS_NODISCARD float ZoomWithTextZoomFactor() noexcept;

    KRYS_NODISCARD style::ScopeOrdinal StyleScopeOrdinal() const noexcept;

    KRYS_NODISCARD const css::ToLengthConversionData &CssToLengthConversionData() const noexcept;

    void SetCurrentPropertyInvalidAtComputedValueTime() noexcept;

    void SetUsesViewportUnits() noexcept;
    void SetUsesContainerUnits() noexcept;

    KRYS_NODISCARD double LookupCSSRandomBaseValue(const css::calc::RandomCachingKey &,
                                                   Maybe<css::keywords::ElementShared>) const noexcept;

    // Accessors for sibling information used by the sibling-count() and sibling-index() CSS functions.
    KRYS_NODISCARD uint32 SiblingCount() noexcept;
    KRYS_NODISCARD uint32 SiblingIndex() noexcept;
  };

  class AnchorPositionEvaluator
  {
  public:
    using Side = Variant<css::ValueId, double>;

    AnchorPositionEvaluator() = delete;

    KRYS_NODISCARD static Maybe<double> Evaluate(BuilderState &, Maybe<ScopedName> elementName,
                                                 Side) noexcept;
    KRYS_NODISCARD static Maybe<double> EvaluateSize(BuilderState &, Maybe<ScopedName> elementName,
                                                     Maybe<AnchorSizeDimension>) noexcept;
  };

  class ContainerQueryEvaluator
  {
  public:
    enum class SelectionMode : uint8
    {
      Element,
      PseudoElement,
      PartPseudoElement
    };

    ContainerQueryEvaluator() = delete;

    KRYS_NODISCARD static RefPtr<const dom::Element>
      SelectContainer(css::cq::Axis, const css::CSSOMString &name, const dom::Element &,
                      SelectionMode = SelectionMode::Element) noexcept;
  };
}
