#pragma once

#include "Krystal.Booey/Core/BoxExtents.hpp"
#include "Krystal.Booey/Rendering/PaintPhase.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyleProperties.hpp"
#include "Krystal.Booey/Style/Values/Color/ColorOptions.hpp"

namespace krys::boo::style
{
  class CustomPropertyRegistry;
}

namespace krys::boo
{
  class IntSize;
  class RenderElement;

  class RenderStyle final : public RenderStyleProperties
  {
  public:
    RenderStyle(RenderStyle &&) noexcept;
    RenderStyle &operator=(RenderStyle &&) noexcept;

    explicit RenderStyle(CreateDefaultStyleTag) noexcept;
    RenderStyle(const RenderStyle &, CloneTag) noexcept;

    KRYS_NODISCARD RenderStyle Replace(RenderStyle &&) noexcept;

    static RenderStyle &DefaultStyleSingleton() noexcept;

    // MARK: - Initialization

    KRYS_NODISCARD static RenderStyle Create() noexcept;
    KRYS_NODISCARD static UniquePtr<RenderStyle> CreatePtr() noexcept;
    KRYS_NODISCARD static UniquePtr<RenderStyle>
      CreatePtrWithRegisteredInitialValues(const style::CustomPropertyRegistry &) noexcept;

    KRYS_NODISCARD static RenderStyle Clone(const RenderStyle &) noexcept;
    KRYS_NODISCARD static RenderStyle CloneIncludingPseudoElements(const RenderStyle &) noexcept;
    KRYS_NODISCARD static UniquePtr<RenderStyle> ClonePtr(const RenderStyle &) noexcept;

    KRYS_NODISCARD static RenderStyle CreateAnonymousStyleWithDisplay(const RenderStyle &parentStyle,
                                                                      DisplayType) noexcept;
    KRYS_NODISCARD static RenderStyle
      CreateStyleInheritingFromPseudoStyle(const RenderStyle &pseudoStyle) noexcept;

    /// @return The logical left/right float of the renderer (block-relative).
    KRYS_NODISCARD static UsedFloat GetUsedFloat(const RenderElement &renderer) noexcept;
    /// @return The logical left/right clear of the renderer (block-relative).
    KRYS_NODISCARD static UsedClear GetUsedClear(const RenderElement &renderer) noexcept;

    void InheritFrom(const RenderStyle &) noexcept;
    void InheritIgnoringCustomPropertiesFrom(const RenderStyle &) noexcept;
    void InheritUnicodeBidiFrom(const RenderStyle &) noexcept;
    void InheritColumnPropertiesFrom(const RenderStyle &) noexcept;
    void FastPathInheritFrom(const RenderStyle &) noexcept;
    void CopyNonInheritedFrom(const RenderStyle &) noexcept;
    void CopyContentFrom(const RenderStyle &) noexcept;
    void CopyPseudoElementsFrom(const RenderStyle &) noexcept;
    void CopyPseudoElementBitsFrom(const RenderStyle &) noexcept;

    // MARK: - Specific style change queries

    KRYS_NODISCARD bool ScrollAnchoringSuppressionStyleDidChange(const RenderStyle *) const noexcept;
    KRYS_NODISCARD bool OutOfFlowPositionStyleDidChange(const RenderStyle *) const noexcept;

    // MARK: - Comparisons

    bool operator==(const RenderStyle &) const noexcept;

    KRYS_NODISCARD bool InheritedEqual(const RenderStyle &) const noexcept;
    KRYS_NODISCARD bool NonInheritedEqual(const RenderStyle &) const noexcept;
    KRYS_NODISCARD bool FastPathInheritedEqual(const RenderStyle &) const noexcept;
    KRYS_NODISCARD bool NonFastPathInheritedEqual(const RenderStyle &) const noexcept;
    KRYS_NODISCARD bool DescendantAffectingNonInheritedPropertiesEqual(const RenderStyle &) const noexcept;
    KRYS_NODISCARD bool BorderAndBackgroundEqual(const RenderStyle &) const noexcept;
    KRYS_NODISCARD bool ContainerTypeAndNamesEqual(const RenderStyle &) const noexcept;
    KRYS_NODISCARD bool ColumnSpanEqual(const RenderStyle &) const noexcept;
    KRYS_NODISCARD bool ScrollPaddingEqual(const RenderStyle &) const noexcept;
    KRYS_NODISCARD bool FontCascadeEqual(const RenderStyle &) const noexcept;
    KRYS_NODISCARD bool ScrollSnapDataEquivalent(const RenderStyle &) const noexcept;

    // MARK: - Style adjustment utilities

    inline void SetPageScaleTransform(float) noexcept;
    inline void AddToTextDecorationLineInEffect(style::TextDecorationLine) noexcept;
    inline void ContainIntrinsicWidthAddAuto() noexcept;
    inline void ContainIntrinsicHeightAddAuto() noexcept;
    inline void SetGridAutoFlowDirection(style::GridAutoFlow::Direction) noexcept;

    void AdjustAnimations() noexcept;
    void AdjustTransitions() noexcept;
    void AdjustBackgroundLayers() noexcept;
    void AdjustMaskLayers() noexcept;
    void AdjustScrollTimelines() noexcept;
    void AdjustViewTimelines() noexcept;

    inline void ResetBorder() noexcept;
    inline void ResetBorderExceptRadius() noexcept;
    inline void ResetBorderTop() noexcept;
    inline void ResetBorderRight() noexcept;
    inline void ResetBorderBottom() noexcept;
    inline void ResetBorderLeft() noexcept;
    inline void ResetBorderRadius() noexcept;
    inline void ResetMargin() noexcept;
    inline void ResetPadding() noexcept;

    // MARK: - Logical Values

    // Logical Inset aliases
    KRYS_NODISCARD inline const style::InsetEdge &LogicalLeft() const noexcept;
    KRYS_NODISCARD inline const style::InsetEdge &LogicalRight() const noexcept;
    KRYS_NODISCARD inline const style::InsetEdge &LogicalTop() const noexcept;
    KRYS_NODISCARD inline const style::InsetEdge &LogicalBottom() const noexcept;

    // Logical Border (aggregate)
    KRYS_NODISCARD const BorderValue &BorderBefore(const WritingMode) const noexcept;
    KRYS_NODISCARD const BorderValue &BorderAfter(const WritingMode) const noexcept;
    KRYS_NODISCARD const BorderValue &BorderStart(const WritingMode) const noexcept;
    KRYS_NODISCARD const BorderValue &BorderEnd(const WritingMode) const noexcept;
    KRYS_NODISCARD inline const BorderValue &BorderBefore() const noexcept;
    KRYS_NODISCARD inline const BorderValue &BorderAfter() const noexcept;
    KRYS_NODISCARD inline const BorderValue &BorderStart() const noexcept;
    KRYS_NODISCARD inline const BorderValue &BorderEnd() const noexcept;

    // Logical Aspect Ratio
    KRYS_NODISCARD inline style::Number<css::NonNegative> AspectRatioLogicalWidth() const noexcept;
    KRYS_NODISCARD inline style::Number<css::NonNegative> AspectRatioLogicalHeight() const noexcept;
    KRYS_NODISCARD inline double LogicalAspectRatio() const noexcept;
    KRYS_NODISCARD inline BoxSizing BoxSizingForAspectRatio() const noexcept;

    // Logical Grid
    KRYS_NODISCARD inline const style::GridTrackSizes &
      GridAutoList(style::GridTrackSizingDirection) const noexcept;
    KRYS_NODISCARD inline const style::GridTemplateList &
      GridTemplateList(style::GridTrackSizingDirection) const noexcept;
    KRYS_NODISCARD inline const style::GridPosition &
      GridItemStart(style::GridTrackSizingDirection) const noexcept;
    KRYS_NODISCARD inline const style::GridPosition &
      GridItemEnd(style::GridTrackSizingDirection) const noexcept;
    KRYS_NODISCARD inline const style::GapGutter &Gap(style::GridTrackSizingDirection) const noexcept;

    // MARK: - Derived Values

    KRYS_NODISCARD inline float ComputedLineHeight() const noexcept;
    KRYS_NODISCARD inline float ComputeLineHeight(const style::LineHeight &) const noexcept;
    KRYS_NODISCARD LayoutBoxExtent ImageOutsets(const style::BorderImage &) const noexcept;
    KRYS_NODISCARD LayoutBoxExtent ImageOutsets(const style::MaskBorder &) const noexcept;
    KRYS_NODISCARD LayoutBoxExtent BorderImageOutsets() const noexcept;
    KRYS_NODISCARD LayoutBoxExtent MaskBorderOutsets() const noexcept;
    KRYS_NODISCARD inline bool HasBorderImageOutsets() const noexcept;

    // MARK: - Used Values

    KRYS_NODISCARD float
      OutlineSize() const noexcept; // used value combining `outline-width` and `outline-offset`
    KRYS_NODISCARD css::CSSOMString AltFromContent() const noexcept;
    KRYS_NODISCARD const css::CSSOMStringAtom &HyphenString() const noexcept;
    KRYS_NODISCARD float UsedStrokeWidth(const IntSize &viewportSize) const noexcept;
    KRYS_NODISCARD Color UsedStrokeColor() const noexcept;
    KRYS_NODISCARD inline PointerEvents UsedPointerEvents() const noexcept;
    KRYS_NODISCARD inline Visibility UsedVisibility() const noexcept;
    KRYS_NODISCARD inline UserModify UsedUserModify() const noexcept;
    KRYS_NODISCARD UserSelect UsedUserSelect() const noexcept;
    KRYS_NODISCARD style::Contain UsedContain() const noexcept;
    KRYS_NODISCARD inline TransformStyle3D UsedTransformStyle3D() const noexcept;
    KRYS_NODISCARD inline float UsedPerspective() const noexcept;
    KRYS_NODISCARD Color UsedScrollbarThumbColor() const noexcept;
    KRYS_NODISCARD Color UsedScrollbarTrackColor() const noexcept;
    KRYS_NODISCARD Color UsedAccentColor(style::ColorOptions) const noexcept;
    // FIXME: `usedFloat()` and `usedClear()` (logical left/right, block-relative) come with the renderers.

    // MARK: - has*()

    KRYS_NODISCARD inline bool HasAnimations() const noexcept;
    KRYS_NODISCARD inline bool HasAnimationsOrTransitions() const noexcept;
    KRYS_NODISCARD inline bool HasAppearance() const noexcept;
    KRYS_NODISCARD inline bool HasAppleColorFilter() const noexcept;
    KRYS_NODISCARD inline bool HasAspectRatio() const noexcept;
    KRYS_NODISCARD inline bool HasAutoLengthContainIntrinsicSize() const noexcept;
    KRYS_NODISCARD inline bool HasBackdropFilter() const noexcept;
    KRYS_NODISCARD inline bool HasBackground() const noexcept;
    KRYS_NODISCARD inline bool HasBackgroundImage() const noexcept;
    KRYS_NODISCARD inline bool HasBlendMode() const noexcept;
    KRYS_NODISCARD inline bool HasBorder() const noexcept;
    KRYS_NODISCARD inline bool HasBorderImage() const noexcept;
    KRYS_NODISCARD inline bool HasBorderRadius() const noexcept;
    KRYS_NODISCARD inline bool HasBoxShadow() const noexcept;
    KRYS_NODISCARD inline bool HasClip() const noexcept;
    KRYS_NODISCARD inline bool HasClipPath() const noexcept;
    KRYS_NODISCARD inline bool HasContent() const noexcept;
    KRYS_NODISCARD inline bool HasFill() const noexcept;
    KRYS_NODISCARD inline bool HasFilter() const noexcept;
    KRYS_NODISCARD inline bool HasIsolation() const noexcept;
    KRYS_NODISCARD inline bool HasMarkers() const noexcept;
    KRYS_NODISCARD inline bool HasMask() const noexcept;
    KRYS_NODISCARD inline bool HasOffsetPath() const noexcept;
    KRYS_NODISCARD inline bool HasOpacity() const noexcept;
    KRYS_NODISCARD inline bool HasOutline() const noexcept;
    KRYS_NODISCARD inline bool HasOutlineInVisualOverflow() const noexcept;
    KRYS_NODISCARD inline bool HasPerspective() const noexcept;
    KRYS_NODISCARD inline bool HasPositionedMask() const noexcept;
    KRYS_NODISCARD inline bool HasRotate() const noexcept;
    KRYS_NODISCARD inline bool HasScale() const noexcept;
    KRYS_NODISCARD inline bool HasScrollTimelines() const noexcept;
    KRYS_NODISCARD inline bool HasSnapPosition() const noexcept;
    KRYS_NODISCARD inline bool HasStroke() const noexcept;
    KRYS_NODISCARD inline bool HasTextCombine() const noexcept;
    KRYS_NODISCARD inline bool HasTextShadow() const noexcept;
    KRYS_NODISCARD inline bool HasTransform() const noexcept;
    KRYS_NODISCARD inline bool HasTransitions() const noexcept;
    KRYS_NODISCARD inline bool HasTranslate() const noexcept;
    KRYS_NODISCARD inline bool HasUsedAppearance() const noexcept;
    KRYS_NODISCARD inline bool HasUsedContentNone() const noexcept;
    KRYS_NODISCARD inline bool HasViewTimelines() const noexcept;
    KRYS_NODISCARD inline bool HasVisibleBorder() const noexcept;
    KRYS_NODISCARD inline bool HasVisibleBorderDecoration() const noexcept;
    KRYS_NODISCARD inline bool HasExplicitlySetBorderRadius() const noexcept;
    KRYS_NODISCARD inline bool HasPositiveStrokeWidth() const noexcept;

    // Whether or not a positioned element requires normal flow x/y to be computed to determine its position.
    KRYS_NODISCARD inline bool HasStaticInlinePosition(bool horizontal) const noexcept;
    KRYS_NODISCARD inline bool HasStaticBlockPosition(bool horizontal) const noexcept;
    KRYS_NODISCARD inline bool HasOutOfFlowPosition() const noexcept;
    KRYS_NODISCARD inline bool HasInFlowPosition() const noexcept;
    KRYS_NODISCARD inline bool HasViewportConstrainedPosition() const noexcept;

    // MARK: - Other predicates

    KRYS_NODISCARD inline bool IsColumnFlexDirection() const noexcept;
    KRYS_NODISCARD inline bool IsFixedTableLayout() const noexcept;
    KRYS_NODISCARD inline bool IsFloating() const noexcept;
    KRYS_NODISCARD inline bool IsInterCharacterRubyPosition() const noexcept;
    KRYS_NODISCARD inline bool IsOverflowVisible() const noexcept;
    KRYS_NODISCARD inline bool IsReverseFlexDirection() const noexcept;
    KRYS_NODISCARD inline bool IsRowFlexDirection() const noexcept;
    KRYS_NODISCARD inline bool IsSkippedRootOrSkippedContent() const noexcept;
    KRYS_NODISCARD constexpr bool IsDisplayInlineType() const noexcept;
    KRYS_NODISCARD constexpr bool IsOriginalDisplayInlineType() const noexcept;
    KRYS_NODISCARD constexpr bool IsDisplayFlexibleOrGridFormattingContextBox() const noexcept;
    KRYS_NODISCARD constexpr bool IsDisplayDeprecatedFlexibleBox() const noexcept;
    KRYS_NODISCARD constexpr bool
      IsDisplayFlexibleBoxIncludingDeprecatedOrGridFormattingContextBox() const noexcept;
    KRYS_NODISCARD constexpr bool IsDisplayRegionType() const noexcept;
    KRYS_NODISCARD constexpr bool IsDisplayBlockLevel() const noexcept;
    KRYS_NODISCARD constexpr bool IsOriginalDisplayBlockType() const noexcept;
    KRYS_NODISCARD constexpr bool IsDisplayTableOrTablePart() const noexcept;
    KRYS_NODISCARD constexpr bool IsInternalTableBox() const noexcept;
    KRYS_NODISCARD constexpr bool IsRubyContainerOrInternalRubyBox() const noexcept;
    KRYS_NODISCARD constexpr bool IsOriginalDisplayListItemType() const noexcept;

    KRYS_NODISCARD constexpr bool DoesDisplayGenerateBlockContainer() const noexcept;

    KRYS_NODISCARD inline bool SpecifiesColumns() const noexcept;

    KRYS_NODISCARD inline bool ColumnRuleIsTransparent() const noexcept;
    KRYS_NODISCARD inline bool BorderLeftIsTransparent() const noexcept;
    KRYS_NODISCARD inline bool BorderRightIsTransparent() const noexcept;
    KRYS_NODISCARD inline bool BorderTopIsTransparent() const noexcept;
    KRYS_NODISCARD inline bool BorderBottomIsTransparent() const noexcept;

    KRYS_NODISCARD inline bool UsesStandardScrollbarStyle() const noexcept;
    KRYS_NODISCARD inline bool ShouldPlaceVerticalScrollbarOnLeft() const noexcept;

    KRYS_NODISCARD inline bool AutoWrap() const noexcept;
    KRYS_NODISCARD inline bool PreserveNewline() const noexcept;
    KRYS_NODISCARD inline bool CollapseWhiteSpace() const noexcept;
    KRYS_NODISCARD inline bool IsCollapsibleWhiteSpace(char16_t) const noexcept;
    KRYS_NODISCARD inline bool BreakOnlyAfterWhiteSpace() const noexcept;
    KRYS_NODISCARD inline bool BreakWords() const noexcept;
    KRYS_NODISCARD constexpr static bool PreserveNewline(WhiteSpaceCollapse) noexcept;
    KRYS_NODISCARD constexpr static bool CollapseWhiteSpace(WhiteSpaceCollapse) noexcept;

    // Return true if any transform related property (currently transform, translate, scale, rotate,
    // transformStyle3D or perspective) indicates that we are transforming. The usedTransformStyle3D is not
    // used here because in many cases (such as for deciding whether or not to establish a containing block),
    // the computed value is what matters.
    KRYS_NODISCARD inline bool HasTransformRelatedProperty() const noexcept;
    KRYS_NODISCARD inline bool Preserves3D() const noexcept;
    KRYS_NODISCARD inline bool AffectsTransform() const noexcept;

    // MARK: - Colors

    // Resolves the currentColor keyword, but must not be used for the "color" property which has a different
    // semantic.
    KRYS_NODISCARD Color ColorResolvingCurrentColor(css::PropertyId colorProperty,
                                                    bool visitedLink) const noexcept;
    KRYS_NODISCARD Color ColorResolvingCurrentColor(const style::Color &,
                                                    bool visitedLink = false) const noexcept;
    KRYS_NODISCARD Color VisitedDependentColor(css::PropertyId, EnumSet<PaintBehavior> = {}) const noexcept;
    KRYS_NODISCARD Color VisitedDependentColorWithColorFilter(css::PropertyId,
                                                              EnumSet<PaintBehavior> = {}) const noexcept;
    KRYS_NODISCARD Color ColorByApplyingColorFilter(const Color &) const noexcept;
    KRYS_NODISCARD Color ColorWithColorFilter(const style::Color &) const noexcept;

  private:
    // This constructor is used to implement the replace operation.
    RenderStyle(RenderStyle &, RenderStyle &&) noexcept;

    KRYS_NODISCARD const style::Color &UnresolvedColorForProperty(css::PropertyId,
                                                                  bool visitedLink = false) const noexcept;
    KRYS_NODISCARD css::PropertyId UsedStrokeColorProperty() const noexcept;

    KRYS_NODISCARD inline bool HasAutoLeftAndRight() const noexcept;
    KRYS_NODISCARD inline bool HasAutoTopAndBottom() const noexcept;

    KRYS_NODISCARD constexpr static bool IsDisplayInlineType(DisplayType) noexcept;
    KRYS_NODISCARD constexpr static bool IsDisplayBlockType(DisplayType) noexcept;
    KRYS_NODISCARD constexpr static bool IsDisplayFlexibleBox(DisplayType) noexcept;
    KRYS_NODISCARD constexpr static bool IsDisplayGridFormattingContextBox(DisplayType) noexcept;
    KRYS_NODISCARD constexpr static bool IsDisplayGridBox(DisplayType) noexcept;
    KRYS_NODISCARD constexpr static bool IsDisplayGridLanesBox(DisplayType) noexcept;
    KRYS_NODISCARD constexpr static bool IsDisplayFlexibleOrGridFormattingContextBox(DisplayType) noexcept;
    KRYS_NODISCARD constexpr static bool IsDisplayDeprecatedFlexibleBox(DisplayType) noexcept;
    KRYS_NODISCARD constexpr static bool IsDisplayListItemType(DisplayType) noexcept;
    KRYS_NODISCARD constexpr static bool IsDisplayTableOrTablePart(DisplayType) noexcept;
    KRYS_NODISCARD constexpr static bool IsInternalTableBox(DisplayType) noexcept;
    KRYS_NODISCARD constexpr static bool IsRubyContainerOrInternalRubyBox(DisplayType) noexcept;
  };

  // Map from computed style values (which take zoom into account) to web-exposed values, which are
  // zoom-independent.
  KRYS_NODISCARD inline int AdjustForAbsoluteZoom(int, const RenderStyle &) noexcept;
  KRYS_NODISCARD inline float AdjustFloatForAbsoluteZoom(float, const RenderStyle &) noexcept;
  KRYS_NODISCARD inline LayoutUnit AdjustLayoutUnitForAbsoluteZoom(LayoutUnit, const RenderStyle &) noexcept;
  KRYS_NODISCARD inline LayoutSize AdjustLayoutSizeForAbsoluteZoom(LayoutSize, const RenderStyle &) noexcept;

  // Map from zoom-independent style values to computed style values (which take zoom into account).
  KRYS_NODISCARD inline float ApplyZoom(float, const RenderStyle &) noexcept;

  KRYS_NODISCARD constexpr BorderStyle CollapsedBorderStyle(BorderStyle) noexcept;

  KRYS_NODISCARD inline bool PseudoElementRendererIsNeeded(const RenderStyle *) noexcept;
  KRYS_NODISCARD inline bool GeneratesBox(const RenderStyle &) noexcept;
  KRYS_NODISCARD inline bool IsNonVisibleOverflow(Overflow) noexcept;
}
