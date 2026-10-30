#pragma once

#include "Krystal.Booey/Style/Computed/ComputedStyle.hpp"

namespace krys::boo
{
  class RenderStyleBase
  {
  public:
    ~RenderStyleBase() noexcept = default;

    KRYS_NODISCARD inline bool UsesViewportUnits() const noexcept;
    inline void SetUsesViewportUnits() noexcept;

    KRYS_NODISCARD inline bool UsesContainerUnits() const noexcept;
    inline void SetUsesContainerUnits() noexcept;

    KRYS_NODISCARD inline bool UseTreeCountingFunctions() const noexcept;
    inline void SetUsesTreeCountingFunctions() noexcept;

    KRYS_NODISCARD inline InsideLink GetInsideLink() const noexcept;
    inline void SetInsideLink(InsideLink) noexcept;

    KRYS_NODISCARD inline bool IsLink() const noexcept;
    inline void SetIsLink(bool) noexcept;

    KRYS_NODISCARD inline bool EmptyState() const noexcept;
    inline void SetEmptyState(bool) noexcept;

    KRYS_NODISCARD inline bool FirstChildState() const noexcept;
    inline void SetFirstChildState() noexcept;

    KRYS_NODISCARD inline bool LastChildState() const noexcept;
    inline void SetLastChildState() noexcept;

    KRYS_NODISCARD inline bool HasExplicitlyInheritedProperties() const noexcept;
    inline void SetHasExplicitlyInheritedProperties() noexcept;

    KRYS_NODISCARD inline bool DisallowsFastPathInheritance() const noexcept;
    inline void SetDisallowsFastPathInheritance() noexcept;

    KRYS_NODISCARD inline bool HasDisplayAffectedByAnimations() const noexcept;
    inline void SetHasDisplayAffectedByAnimations() noexcept;

    KRYS_NODISCARD inline bool TransformStyleForcedToFlat() const noexcept;
    inline void SetTransformStyleForcedToFlat(bool) noexcept;

    inline void SetUsesAnchorFunctions() noexcept;
    KRYS_NODISCARD inline bool UsesAnchorFunctions() const noexcept;

    inline void SetAnchorFunctionScrollCompensatedAxes(EnumSet<BoxAxis>) noexcept;
    KRYS_NODISCARD inline EnumSet<BoxAxis> AnchorFunctionScrollCompensatedAxes() const noexcept;

    inline void SetIsPopoverInvoker() noexcept;
    KRYS_NODISCARD inline bool IsPopoverInvoker() const noexcept;

    KRYS_NODISCARD inline bool NativeAppearanceDisabled() const noexcept;
    inline void SetNativeAppearanceDisabled(bool) noexcept;

    KRYS_NODISCARD inline bool InsideDefaultButton() const noexcept;
    inline void SetInsideDefaultButton(bool) noexcept;

    KRYS_NODISCARD inline bool InsideSubmitButton() const noexcept;
    inline void SetInsideSubmitButton(bool) noexcept;

    KRYS_NODISCARD inline EnumSet<EventListenerRegionType> EventListenerRegionTypes() const noexcept;
    inline void SetEventListenerRegionTypes(EnumSet<EventListenerRegionType>) noexcept;

    // No setter. Set via `RenderStyleProperties::setBlendMode()`.
    KRYS_NODISCARD inline bool IsInSubtreeWithBlendMode() const noexcept;

    KRYS_NODISCARD inline bool IsForceHidden() const noexcept;
    inline void SetIsForceHidden() noexcept;

    KRYS_NODISCARD inline bool AutoRevealsWhenFound() const noexcept;
    inline void SetAutoRevealsWhenFound() noexcept;

    KRYS_NODISCARD inline bool HasAttrContent() const noexcept;
    inline void SetHasAttrContent() noexcept;

    KRYS_NODISCARD inline Maybe<size_t> UsedPositionOptionIndex() const noexcept;
    inline void SetUsedPositionOptionIndex(Maybe<size_t>) noexcept;

    KRYS_NODISCARD inline bool EffectiveInert() const noexcept;
    inline void SetEffectiveInert(bool) noexcept;

    KRYS_NODISCARD inline bool
      IsEffectivelyTransparent() const noexcept; // This or any ancestor has opacity 0.
    inline void SetIsEffectivelyTransparent(bool) noexcept;

    // No setter. Set via `RenderStyleProperties::setDisplay()`.
    KRYS_NODISCARD constexpr inline DisplayType OriginalDisplay() const noexcept;

    // `effectiveDisplay()` getter is an alias of `RenderStyleProperties::display()`.
    KRYS_NODISCARD inline DisplayType EffectiveDisplay() const noexcept;
    inline void SetEffectiveDisplay(DisplayType) noexcept;

    KRYS_NODISCARD inline StyleAppearance UsedAppearance() const noexcept;
    inline void SetUsedAppearance(StyleAppearance) noexcept;

    // usedContentVisibility will return ContentVisibility::Hidden in a content-visibility: hidden subtree
    // (overriding content-visibility: auto at all times), ContentVisibility::Auto in a content-visibility:
    // auto subtree (when the content is not user relevant and thus skipped), and ContentVisibility::Visible
    // otherwise.
    KRYS_NODISCARD inline ContentVisibility UsedContentVisibility() const noexcept;
    inline void SetUsedContentVisibility(ContentVisibility) noexcept;

    // 'touch-action' behavior depends on values in ancestors. We use an additional inherited property to
    // implement that.
    KRYS_NODISCARD inline style::TouchAction UsedTouchAction() const noexcept;
    inline void SetUsedTouchAction(style::TouchAction) noexcept;

    KRYS_NODISCARD inline style::ZIndex UsedZIndex() const noexcept;
    inline void SetUsedZIndex(style::ZIndex) noexcept;

    // MARK: - Pseudo element/style

    KRYS_NODISCARD inline Maybe<PseudoElementType> GetPseudoElementType() const noexcept;
    KRYS_NODISCARD inline const css::CSSOMStringAtom &PseudoElementNameArgument() const noexcept;

    KRYS_NODISCARD inline Maybe<style::PseudoElementIdentifier> GetPseudoElementIdentifier() const noexcept;
    inline void SetPseudoElementIdentifier(Maybe<style::PseudoElementIdentifier> &&) noexcept;

    KRYS_NODISCARD inline bool HasAnyPublicPseudoStyles() const noexcept;
    KRYS_NODISCARD inline bool HasPseudoStyle(PseudoElementType) const noexcept;
    inline void SetHasPseudoStyles(EnumSet<PseudoElementType>) noexcept;

    KRYS_NODISCARD RenderStyle *GetCachedPseudoStyle(const style::PseudoElementIdentifier &) const noexcept;
    RenderStyle *AddCachedPseudoStyle(UniquePtr<RenderStyle>) noexcept;

    KRYS_NODISCARD bool HasCachedPseudoStyles() const noexcept
    {
      return _computedStyle._cachedPseudoStyles && _computedStyle._cachedPseudoStyles->Styles.size();
    }
    const style::PseudoStyleCache *CachedPseudoStyles() const
    {
      return _computedStyle._cachedPseudoStyles.get();
    }

    // MARK: - Custom properties

    KRYS_NODISCARD inline const style::CustomPropertyData &InheritedCustomProperties() const noexcept;
    KRYS_NODISCARD inline const style::CustomPropertyData &NonInheritedCustomProperties() const noexcept;
    KRYS_NODISCARD const style::CustomProperty *
      CustomPropertyValue(const css::CSSOMStringAtom &) const noexcept;
    void SetCustomPropertyValue(Ref<const style::CustomProperty> &&, bool isInherited) noexcept;
    KRYS_NODISCARD bool CustomPropertyValueEqual(const RenderStyleBase &,
                                                 const css::CSSOMStringAtom &) const noexcept;
    KRYS_NODISCARD bool CustomPropertiesEqual(const RenderStyleBase &) const noexcept;
    void DeduplicateCustomProperties(const RenderStyleBase &) noexcept;

    // MARK: - Custom paint

    void AddCustomPaintWatchProperty(const css::CSSOMStringAtom &) noexcept;

    // MARK: - Zoom

    KRYS_NODISCARD inline bool EvaluationTimeZoomEnabled() const noexcept;
    inline void SetEvaluationTimeZoomEnabled(bool) noexcept;

    KRYS_NODISCARD inline float DeviceScaleFactor() const noexcept;
    inline void SetDeviceScaleFactor(float) noexcept;

    KRYS_NODISCARD inline bool UseSVGZoomRulesForLength() const noexcept;
    inline void SetUseSVGZoomRulesForLength(bool) noexcept;

    KRYS_NODISCARD inline float UsedZoom() const noexcept;
    KRYS_NODISCARD inline bool SetUsedZoom(float) noexcept;

    KRYS_NODISCARD inline style::ZoomFactor UsedZoomForLength() const noexcept;

    // MARK: - Fonts

    KRYS_NODISCARD inline const FontCascade &GetFontCascade() const noexcept;
    inline FontCascade &MutableFontCascadeWithoutUpdate() noexcept;
    inline void SetFontCascade(FontCascade &&) noexcept;

    KRYS_NODISCARD inline const FontCascadeDescription &GetFontDescription() const noexcept;
    inline FontCascadeDescription &MutableFontDescriptionWithoutUpdate() noexcept;
    inline void SetFontDescription(FontCascadeDescription &&) noexcept;
    KRYS_NODISCARD inline bool SetFontDescriptionWithoutUpdate(FontCascadeDescription &&) noexcept;

    KRYS_NODISCARD inline const FontMetrics &MetricsOfPrimaryFont() const noexcept;
    KRYS_NODISCARD inline std::pair<FontOrientation, NonCJKGlyphOrientation>
      FontAndGlyphOrientation() noexcept;
    KRYS_NODISCARD inline float ComputedFontSize() const noexcept;
    KRYS_NODISCARD inline style::WebkitLocale ComputedLocale() const noexcept;
    KRYS_NODISCARD inline const style::LineHeight &SpecifiedLineHeight() const noexcept;

    inline void SetLetterSpacingFromAnimation(style::LetterSpacing &&) noexcept;
    inline void SetWordSpacingFromAnimation(style::WordSpacing &&) noexcept;

    inline void SynchronizeLetterSpacingWithFontCascade() noexcept;
    inline void SynchronizeLetterSpacingWithFontCascadeWithoutUpdate() noexcept;
    inline void SynchronizeWordSpacingWithFontCascade() noexcept;
    inline void SynchronizeWordSpacingWithFontCascadeWithoutUpdate() noexcept;

    KRYS_NODISCARD inline float UsedLetterSpacing() const noexcept;
    KRYS_NODISCARD inline float UsedWordSpacing() const noexcept;

    // MARK: Writing Modes

    // FIXME: Rename to something that doesn't conflict with a property name.
    // Aggregates `writing-mode`, `direction` and `text-orientation`.
    KRYS_NODISCARD WritingMode GetWritingMode() const noexcept
    {
      return _computedStyle.GetWritingMode();
    }

    // FIXME: *Deprecated* Deprecated due to confusion between physical inline directions and bidi /
    // line-relative directions.
    KRYS_NODISCARD bool IsLeftToRightDirection() const noexcept
    {
      return GetWritingMode().IsBidiLTR();
    }

    // MARK: Aggregates

    inline style::Animations &EnsureAnimations() noexcept;
    inline style::BackgroundLayers &EnsureBackgroundLayers() noexcept;
    inline style::MaskLayers &EnsureMaskLayers() noexcept;
    inline style::Transitions &EnsureTransitions() noexcept;

    KRYS_NODISCARD inline const BorderData &Border() const noexcept;
    KRYS_NODISCARD inline const BorderValue &BorderBottom() const noexcept;
    KRYS_NODISCARD inline const BorderValue &BorderLeft() const noexcept;
    KRYS_NODISCARD inline const BorderValue &BorderRight() const noexcept;
    KRYS_NODISCARD inline const BorderValue &BorderTop() const noexcept;
    KRYS_NODISCARD inline const BorderValue &ColumnRule() const noexcept;
    KRYS_NODISCARD inline const OutlineValue &Outline() const noexcept;
    KRYS_NODISCARD inline const style::Animations &GetAnimations() const noexcept;
    KRYS_NODISCARD inline const style::BackgroundLayers &GetBackgroundLayers() const noexcept;
    KRYS_NODISCARD inline const style::BorderImage &GetBorderImage() const noexcept;
    KRYS_NODISCARD inline const style::BorderRadius &BorderRadii() const noexcept;
    KRYS_NODISCARD inline const style::InsetBox &GetInsetBox() const noexcept;
    KRYS_NODISCARD inline const style::MarginBox &GetMarginBox() const noexcept;
    KRYS_NODISCARD inline const style::MaskBorder &GetMaskBorder() const noexcept;
    KRYS_NODISCARD inline const style::MaskLayers &GetMaskLayers() const noexcept;
    KRYS_NODISCARD inline const style::PaddingBox &GetPaddingBox() const noexcept;
    KRYS_NODISCARD inline const style::PerspectiveOrigin &GetPerspectiveOrigin() const noexcept;
    KRYS_NODISCARD inline const style::ScrollMarginBox &GetScrollMarginBox() const noexcept;
    KRYS_NODISCARD inline const style::ScrollPaddingBox &GetScrollPaddingBox() const noexcept;
    KRYS_NODISCARD inline const style::ScrollTimelines &GetScrollTimelines() const noexcept;
    KRYS_NODISCARD inline const style::TransformOrigin &GetTransformOrigin() const noexcept;
    KRYS_NODISCARD inline const style::Transitions &GetTransitions() const noexcept;
    KRYS_NODISCARD inline const style::ViewTimelines &GetViewTimelines() const noexcept;
    KRYS_NODISCARD inline style::LineWidthBox BorderWidth() const noexcept;

    inline void SetBackgroundLayers(style::BackgroundLayers &&) noexcept;
    inline void SetBorderImage(style::BorderImage &&) noexcept;
    inline void SetBorderRadius(style::BorderRadiusValue &&) noexcept;
    inline void SetBorderTop(BorderValue &&) noexcept;
    inline void SetBorderRight(BorderValue &&) noexcept;
    inline void SetBorderBottom(BorderValue &&) noexcept;
    inline void SetBorderLeft(BorderValue &&) noexcept;
    inline void SetInsetBox(style::InsetBox &&) noexcept;
    inline void SetMarginBox(style::MarginBox &&) noexcept;
    inline void SetMaskBorder(style::MaskBorder &&) noexcept;
    inline void SetMaskLayers(style::MaskLayers &&) noexcept;
    inline void SetPaddingBox(style::PaddingBox &&) noexcept;
    inline void SetPerspectiveOrigin(style::PerspectiveOrigin &&) noexcept;
    inline void SetTransformOrigin(style::TransformOrigin &&) noexcept;

    // MARK: - Properties/descriptors that are not yet generated

    // `caret-color`
    KRYS_NODISCARD inline const style::Color &CaretColor() const noexcept;
    KRYS_NODISCARD inline const style::Color &VisitedLinkCaretColor() const noexcept;
    KRYS_NODISCARD inline bool HasAutoCaretColor() const noexcept;
    KRYS_NODISCARD inline bool HasVisitedLinkAutoCaretColor() const noexcept;
    inline void SetCaretColor(style::Color &&) noexcept;
    inline void SetVisitedLinkCaretColor(style::Color &&) noexcept;
    inline void SetHasAutoCaretColor() noexcept;
    inline void SetHasVisitedLinkAutoCaretColor() noexcept;

    // `cursor`
    KRYS_NODISCARD inline CursorType GetCursorType() const noexcept;

    // `counter-*`
    KRYS_NODISCARD inline const CounterDirectiveMap &GetCounterDirectives() const noexcept;
    inline CounterDirectiveMap &AccessCounterDirectives() noexcept;

    // `@page size`
    KRYS_NODISCARD inline const style::PageSize &GetPageSize() const noexcept;
    inline void SetPageSize(style::PageSize &&) noexcept;

    using NonInheritedFlags = style::ComputedStyle::NonInheritedFlags;
    using InheritedFlags = style::ComputedStyle::InheritedFlags;

    const StyleNonInheritedData &NonInheritedData() const
    {
      return _computedStyle.NonInheritedData();
    }
    const NonInheritedFlags &GetNonInheritedFlags() const
    {
      return _computedStyle.GetNonInheritedFlags();
    }

    const StyleRareInheritedData &RareInheritedData() const
    {
      return _computedStyle.RareInheritedData();
    }
    const StyleInheritedData &InheritedData() const
    {
      return _computedStyle.InheritedData();
    }
    const InheritedFlags &GetInheritedFlags() const
    {
      return _computedStyle.GetInheritedFlags();
    }

    const SVGRenderStyle &SvgStyle() const
    {
      return _computedStyle.SvgStyle();
    }

    const style::ComputedStyle &ComputedStyle() const
    {
      return _computedStyle;
    }

  protected:
    friend class RenderStyle;

    enum CloneTag
    {
      Clone
    };
    enum CreateDefaultStyleTag
    {
      CreateDefaultStyle
    };

    RenderStyleBase(RenderStyleBase &&) noexcept;
    RenderStyleBase &operator=(RenderStyleBase &&) noexcept;

    explicit RenderStyleBase(CreateDefaultStyleTag) noexcept;
    RenderStyleBase(const RenderStyleBase &, CloneTag) noexcept;

    RenderStyleBase(RenderStyleBase &, RenderStyleBase &&) noexcept;

    style::ComputedStyle _computedStyle;
  };
}
