#pragma once

#include "Krystal.Booey/Rendering/Style/RenderStyleBase.hpp"
#include "Krystal.Booey/Style/Computed/ComputedStyle+GettersInlines.hpp"

namespace krys::boo
{
  // MARK: - Non-property getters

  inline bool RenderStyleBase::UsesViewportUnits() const noexcept
  {
    return _computedStyle.UsesViewportUnits();
  }

  inline bool RenderStyleBase::UsesContainerUnits() const noexcept
  {
    return _computedStyle.UsesContainerUnits();
  }

  inline bool RenderStyleBase::UseTreeCountingFunctions() const noexcept
  {
    return _computedStyle.UseTreeCountingFunctions();
  }

  inline InsideLink RenderStyleBase::GetInsideLink() const noexcept
  {
    return _computedStyle.GetInsideLink();
  }

  inline bool RenderStyleBase::IsLink() const noexcept
  {
    return _computedStyle.IsLink();
  }

  inline bool RenderStyleBase::EmptyState() const noexcept
  {
    return _computedStyle.EmptyState();
  }

  inline bool RenderStyleBase::FirstChildState() const noexcept
  {
    return _computedStyle.FirstChildState();
  }

  inline bool RenderStyleBase::LastChildState() const noexcept
  {
    return _computedStyle.LastChildState();
  }

  inline bool RenderStyleBase::HasExplicitlyInheritedProperties() const noexcept
  {
    return _computedStyle.HasExplicitlyInheritedProperties();
  }

  inline bool RenderStyleBase::DisallowsFastPathInheritance() const noexcept
  {
    return _computedStyle.DisallowsFastPathInheritance();
  }

  inline bool RenderStyleBase::EffectiveInert() const noexcept
  {
    return _computedStyle.EffectiveInert();
  }

  inline bool RenderStyleBase::IsEffectivelyTransparent() const noexcept
  {
    return _computedStyle.IsEffectivelyTransparent();
  }

  inline bool RenderStyleBase::InsideDefaultButton() const noexcept
  {
    return _computedStyle.InsideDefaultButton();
  }

  inline bool RenderStyleBase::InsideSubmitButton() const noexcept
  {
    return _computedStyle.InsideSubmitButton();
  }

  inline bool RenderStyleBase::IsInSubtreeWithBlendMode() const noexcept
  {
    return _computedStyle.IsInSubtreeWithBlendMode();
  }

  inline bool RenderStyleBase::IsForceHidden() const noexcept
  {
    return _computedStyle.IsForceHidden();
  }

  inline bool RenderStyleBase::HasDisplayAffectedByAnimations() const noexcept
  {
    return _computedStyle.HasDisplayAffectedByAnimations();
  }

  inline bool RenderStyleBase::TransformStyleForcedToFlat() const noexcept
  {
    return _computedStyle.TransformStyleForcedToFlat();
  }

  inline bool RenderStyleBase::UsesAnchorFunctions() const noexcept
  {
    return _computedStyle.UsesAnchorFunctions();
  }

  inline EnumSet<BoxAxis> RenderStyleBase::AnchorFunctionScrollCompensatedAxes() const noexcept
  {
    return _computedStyle.AnchorFunctionScrollCompensatedAxes();
  }

  inline bool RenderStyleBase::IsPopoverInvoker() const noexcept
  {
    return _computedStyle.IsPopoverInvoker();
  }

  inline bool RenderStyleBase::AutoRevealsWhenFound() const noexcept
  {
    return _computedStyle.AutoRevealsWhenFound();
  }

  inline bool RenderStyleBase::NativeAppearanceDisabled() const noexcept
  {
    return _computedStyle.NativeAppearanceDisabled();
  }

  inline EnumSet<EventListenerRegionType> RenderStyleBase::EventListenerRegionTypes() const noexcept
  {
    return _computedStyle.EventListenerRegionTypes();
  }

  inline bool RenderStyleBase::HasAttrContent() const noexcept
  {
    return _computedStyle.HasAttrContent();
  }

  inline Maybe<size_t> RenderStyleBase::UsedPositionOptionIndex() const noexcept
  {
    return _computedStyle.UsedPositionOptionIndex();
  }

  constexpr inline DisplayType RenderStyleBase::OriginalDisplay() const noexcept
  {
    return _computedStyle.OriginalDisplay();
  }

  inline DisplayType RenderStyleBase::EffectiveDisplay() const noexcept
  {
    return _computedStyle.EffectiveDisplay();
  }

  inline StyleAppearance RenderStyleBase::UsedAppearance() const noexcept
  {
    return _computedStyle.UsedAppearance();
  }

  inline ContentVisibility RenderStyleBase::UsedContentVisibility() const noexcept
  {
    return _computedStyle.UsedContentVisibility();
  }

  inline style::TouchAction RenderStyleBase::UsedTouchAction() const noexcept
  {
    return _computedStyle.UsedTouchAction();
  }

  inline style::ZIndex RenderStyleBase::UsedZIndex() const noexcept
  {
    return _computedStyle.UsedZIndex();
  }

  // MARK: - Pseudo element/style

  inline bool RenderStyleBase::HasAnyPublicPseudoStyles() const noexcept
  {
    return _computedStyle.HasAnyPublicPseudoStyles();
  }

  inline bool RenderStyleBase::HasPseudoStyle(PseudoElementType pseudo) const noexcept
  {
    return _computedStyle.HasPseudoStyle(pseudo);
  }

  inline Maybe<PseudoElementType> RenderStyleBase::GetPseudoElementType() const noexcept
  {
    return _computedStyle.GetPseudoElementType();
  }

  inline const css::CSSOMStringAtom &RenderStyleBase::PseudoElementNameArgument() const noexcept
  {
    return _computedStyle.PseudoElementNameArgument();
  }

  inline Maybe<style::PseudoElementIdentifier> RenderStyleBase::GetPseudoElementIdentifier() const noexcept
  {
    return _computedStyle.GetPseudoElementIdentifier();
  }

  inline RenderStyle *RenderStyleBase::GetCachedPseudoStyle(
    const style::PseudoElementIdentifier &pseudoElementIdentifier) const noexcept
  {
    return _computedStyle.GetCachedPseudoStyle(pseudoElementIdentifier);
  }

  // MARK: - Custom properties

  inline const style::CustomPropertyData &RenderStyleBase::InheritedCustomProperties() const noexcept
  {
    return _computedStyle.InheritedCustomProperties();
  }

  inline const style::CustomPropertyData &RenderStyleBase::NonInheritedCustomProperties() const noexcept
  {
    return _computedStyle.NonInheritedCustomProperties();
  }

  inline const style::CustomProperty *
    RenderStyleBase::CustomPropertyValue(const css::CSSOMStringAtom &property) const noexcept
  {
    return _computedStyle.CustomPropertyValue(property);
  }

  inline bool RenderStyleBase::CustomPropertyValueEqual(const RenderStyleBase &other,
                                                        const css::CSSOMStringAtom &property) const noexcept
  {
    return _computedStyle.CustomPropertyValueEqual(other._computedStyle, property);
  }

  inline bool RenderStyleBase::CustomPropertiesEqual(const RenderStyleBase &other) const noexcept
  {
    return _computedStyle.CustomPropertiesEqual(other._computedStyle);
  }

  inline void RenderStyleBase::DeduplicateCustomProperties(const RenderStyleBase &other) noexcept
  {
    _computedStyle.DeduplicateCustomProperties(other._computedStyle);
  }

  // MARK: - Custom paint

  inline void RenderStyleBase::AddCustomPaintWatchProperty(const css::CSSOMStringAtom &property) noexcept
  {
    _computedStyle.AddCustomPaintWatchProperty(property);
  }

  // MARK: - Zoom

  inline bool RenderStyleBase::EvaluationTimeZoomEnabled() const noexcept
  {
    return _computedStyle.EvaluationTimeZoomEnabled();
  }

  inline float RenderStyleBase::DeviceScaleFactor() const noexcept
  {
    return _computedStyle.DeviceScaleFactor();
  }

  inline bool RenderStyleBase::UseSVGZoomRulesForLength() const noexcept
  {
    return _computedStyle.UseSVGZoomRulesForLength();
  }

  inline float RenderStyleBase::UsedZoom() const noexcept
  {
    return _computedStyle.UsedZoom();
  }

  inline style::ZoomFactor RenderStyleBase::UsedZoomForLength() const noexcept
  {
    return _computedStyle.UsedZoomForLength();
  }

  // MARK: - Fonts

  inline const FontCascade &RenderStyleBase::GetFontCascade() const noexcept
  {
    return _computedStyle.GetFontCascade();
  }

  inline FontCascade &RenderStyleBase::MutableFontCascadeWithoutUpdate() noexcept
  {
    return _computedStyle.MutableFontCascadeWithoutUpdate();
  }

  inline void RenderStyleBase::SetFontCascade(FontCascade &&fontCascade) noexcept
  {
    _computedStyle.SetFontCascade(krys::move(fontCascade));
  }

  inline const FontCascadeDescription &RenderStyleBase::GetFontDescription() const noexcept
  {
    return _computedStyle.GetFontDescription();
  }

  inline FontCascadeDescription &RenderStyleBase::MutableFontDescriptionWithoutUpdate() noexcept
  {
    return _computedStyle.MutableFontDescriptionWithoutUpdate();
  }

  inline void RenderStyleBase::SetFontDescription(FontCascadeDescription &&description) noexcept
  {
    _computedStyle.SetFontDescription(krys::move(description));
  }

  inline bool RenderStyleBase::SetFontDescriptionWithoutUpdate(FontCascadeDescription &&description) noexcept
  {
    return _computedStyle.SetFontDescriptionWithoutUpdate(krys::move(description));
  }

  inline const FontMetrics &RenderStyleBase::MetricsOfPrimaryFont() const noexcept
  {
    return _computedStyle.MetricsOfPrimaryFont();
  }

  inline std::pair<FontOrientation, NonCJKGlyphOrientation>
    RenderStyleBase::FontAndGlyphOrientation() noexcept
  {
    return _computedStyle.FontAndGlyphOrientation();
  }

  inline style::WebkitLocale RenderStyleBase::ComputedLocale() const noexcept
  {
    return _computedStyle.ComputedLocale();
  }

  inline float RenderStyleBase::ComputedFontSize() const noexcept
  {
    return _computedStyle.ComputedFontSize();
  }

  inline const style::LineHeight &RenderStyleBase::SpecifiedLineHeight() const noexcept
  {
    return _computedStyle.SpecifiedLineHeight();
  }

  inline void RenderStyleBase::SynchronizeLetterSpacingWithFontCascade() noexcept
  {
    _computedStyle.SynchronizeLetterSpacingWithFontCascade();
  }

  inline void RenderStyleBase::SynchronizeLetterSpacingWithFontCascadeWithoutUpdate() noexcept
  {
    _computedStyle.SynchronizeLetterSpacingWithFontCascadeWithoutUpdate();
  }

  inline void RenderStyleBase::SynchronizeWordSpacingWithFontCascade() noexcept
  {
    _computedStyle.SynchronizeWordSpacingWithFontCascade();
  }

  inline void RenderStyleBase::SynchronizeWordSpacingWithFontCascadeWithoutUpdate() noexcept
  {
    _computedStyle.SynchronizeWordSpacingWithFontCascadeWithoutUpdate();
  }

  inline float RenderStyleBase::UsedLetterSpacing() const noexcept
  {
    return _computedStyle.UsedLetterSpacing();
  }

  inline float RenderStyleBase::UsedWordSpacing() const noexcept
  {
    return _computedStyle.UsedWordSpacing();
  }

  // MARK: - Aggregates

  inline const style::InsetBox &RenderStyleBase::GetInsetBox() const noexcept
  {
    return _computedStyle.GetInsetBox();
  }

  inline const style::MarginBox &RenderStyleBase::GetMarginBox() const noexcept
  {
    return _computedStyle.GetMarginBox();
  }

  inline const style::PaddingBox &RenderStyleBase::GetPaddingBox() const noexcept
  {
    return _computedStyle.GetPaddingBox();
  }

  inline const style::ScrollMarginBox &RenderStyleBase::GetScrollMarginBox() const noexcept
  {
    return _computedStyle.GetScrollMarginBox();
  }

  inline const style::ScrollPaddingBox &RenderStyleBase::GetScrollPaddingBox() const noexcept
  {
    return _computedStyle.GetScrollPaddingBox();
  }

  inline const style::ScrollTimelines &RenderStyleBase::GetScrollTimelines() const noexcept
  {
    return _computedStyle.GetScrollTimelines();
  }

  inline const style::ViewTimelines &RenderStyleBase::GetViewTimelines() const noexcept
  {
    return _computedStyle.GetViewTimelines();
  }

  inline const style::Animations &RenderStyleBase::GetAnimations() const noexcept
  {
    return _computedStyle.GetAnimations();
  }

  inline const style::Transitions &RenderStyleBase::GetTransitions() const noexcept
  {
    return _computedStyle.GetTransitions();
  }

  inline const style::BackgroundLayers &RenderStyleBase::GetBackgroundLayers() const noexcept
  {
    return _computedStyle.GetBackgroundLayers();
  }

  inline const style::MaskLayers &RenderStyleBase::GetMaskLayers() const noexcept
  {
    return _computedStyle.GetMaskLayers();
  }

  inline const style::MaskBorder &RenderStyleBase::GetMaskBorder() const noexcept
  {
    return _computedStyle.GetMaskBorder();
  }

  inline const style::BorderImage &RenderStyleBase::GetBorderImage() const noexcept
  {
    return _computedStyle.GetBorderImage();
  }

  inline const style::TransformOrigin &RenderStyleBase::GetTransformOrigin() const noexcept
  {
    return _computedStyle.GetTransformOrigin();
  }

  inline const style::PerspectiveOrigin &RenderStyleBase::GetPerspectiveOrigin() const noexcept
  {
    return _computedStyle.GetPerspectiveOrigin();
  }

  inline const OutlineValue &RenderStyleBase::Outline() const noexcept
  {
    return _computedStyle.Outline();
  }

  inline const BorderData &RenderStyleBase::Border() const noexcept
  {
    return _computedStyle.Border();
  }

  inline style::LineWidthBox RenderStyleBase::BorderWidth() const noexcept
  {
    return _computedStyle.BorderWidth();
  }

  inline const style::BorderRadius &RenderStyleBase::BorderRadii() const noexcept
  {
    return _computedStyle.BorderRadii();
  }

  inline const BorderValue &RenderStyleBase::BorderBottom() const noexcept
  {
    return _computedStyle.BorderBottom();
  }

  inline const BorderValue &RenderStyleBase::BorderLeft() const noexcept
  {
    return _computedStyle.BorderLeft();
  }

  inline const BorderValue &RenderStyleBase::BorderRight() const noexcept
  {
    return _computedStyle.BorderRight();
  }

  inline const BorderValue &RenderStyleBase::BorderTop() const noexcept
  {
    return _computedStyle.BorderTop();
  }

  inline const BorderValue &RenderStyleBase::ColumnRule() const noexcept
  {
    return _computedStyle.ColumnRule();
  }

  // MARK: - Properties/descriptors that are not yet generated

  inline CursorType RenderStyleBase::GetCursorType() const noexcept
  {
    return _computedStyle.GetCursorType();
  }

  // FIXME: Support descriptors

  inline const style::PageSize &RenderStyleBase::GetPageSize() const noexcept
  {
    return _computedStyle.GetPageSize();
  }

  // FIXME: Add a type that encapsulates both caretColor() and hasAutoCaretColor().

  inline const style::Color &RenderStyleBase::CaretColor() const noexcept
  {
    return _computedStyle.CaretColor();
  }

  inline bool RenderStyleBase::HasAutoCaretColor() const noexcept
  {
    return _computedStyle.HasAutoCaretColor();
  }

  inline const style::Color &RenderStyleBase::VisitedLinkCaretColor() const noexcept
  {
    return _computedStyle.VisitedLinkCaretColor();
  }

  inline bool RenderStyleBase::HasVisitedLinkAutoCaretColor() const noexcept
  {
    return _computedStyle.HasVisitedLinkAutoCaretColor();
  }

  // MARK: - Properties/descriptors that are not yet generated

  inline const CounterDirectiveMap &RenderStyleBase::GetCounterDirectives() const noexcept
  {
    return _computedStyle.GetCounterDirectives();
  }

  inline CounterDirectiveMap &RenderStyleBase::AccessCounterDirectives() noexcept
  {
    return _computedStyle.AccessCounterDirectives();
  }
}
