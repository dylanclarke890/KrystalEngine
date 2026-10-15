#pragma once

#include "Krystal.Booey/Rendering/Style/RenderStyleBase+GettersInlines.hpp"
#include "Krystal.Booey/Style/Computed/ComputedStyle+SettersInlines.hpp"

namespace krys::boo
{
  // MARK: - Non-property setters

  inline void RenderStyleBase::SetUsesViewportUnits() noexcept
  {
    _computedStyle.SetUsesViewportUnits();
  }

  inline void RenderStyleBase::SetUsesContainerUnits() noexcept
  {
    _computedStyle.SetUsesContainerUnits();
  }

  inline void RenderStyleBase::SetUsesTreeCountingFunctions() noexcept
  {
    _computedStyle.SetUsesTreeCountingFunctions();
  }

  inline void RenderStyleBase::SetInsideLink(InsideLink insideLink) noexcept
  {
    _computedStyle.SetInsideLink(insideLink);
  }

  inline void RenderStyleBase::SetIsLink(bool isLink) noexcept
  {
    _computedStyle.SetIsLink(isLink);
  }

  inline void RenderStyleBase::SetEmptyState(bool emptyState) noexcept
  {
    _computedStyle.SetEmptyState(emptyState);
  }

  inline void RenderStyleBase::SetFirstChildState() noexcept
  {
    _computedStyle.SetFirstChildState();
  }

  inline void RenderStyleBase::SetLastChildState() noexcept
  {
    _computedStyle.SetLastChildState();
  }

  inline void RenderStyleBase::SetHasExplicitlyInheritedProperties() noexcept
  {
    _computedStyle.SetHasExplicitlyInheritedProperties();
  }

  inline void RenderStyleBase::SetDisallowsFastPathInheritance() noexcept
  {
    _computedStyle.SetDisallowsFastPathInheritance();
  }

  inline void RenderStyleBase::SetEffectiveInert(bool effectiveInert) noexcept
  {
    _computedStyle.SetEffectiveInert(effectiveInert);
  }

  inline void RenderStyleBase::SetIsEffectivelyTransparent(bool effectivelyTransparent) noexcept
  {
    _computedStyle.SetIsEffectivelyTransparent(effectivelyTransparent);
  }

  inline void
    RenderStyleBase::SetEventListenerRegionTypes(EnumSet<EventListenerRegionType> eventListenerTypes) noexcept
  {
    _computedStyle.SetEventListenerRegionTypes(eventListenerTypes);
  }

  inline void RenderStyleBase::SetHasAttrContent() noexcept
  {
    _computedStyle.SetHasAttrContent();
  }

  inline void RenderStyleBase::SetHasDisplayAffectedByAnimations() noexcept
  {
    _computedStyle.SetHasDisplayAffectedByAnimations();
  }

  inline void RenderStyleBase::SetTransformStyleForcedToFlat(bool value) noexcept
  {
    _computedStyle.SetTransformStyleForcedToFlat(value);
  }

  inline void RenderStyleBase::SetUsesAnchorFunctions() noexcept
  {
    _computedStyle.SetUsesAnchorFunctions();
  }

  inline void RenderStyleBase::SetAnchorFunctionScrollCompensatedAxes(EnumSet<BoxAxis> axes) noexcept
  {
    _computedStyle.SetAnchorFunctionScrollCompensatedAxes(axes);
  }

  inline void RenderStyleBase::SetIsPopoverInvoker() noexcept
  {
    _computedStyle.SetIsPopoverInvoker();
  }

  inline void RenderStyleBase::SetNativeAppearanceDisabled(bool value) noexcept
  {
    _computedStyle.SetNativeAppearanceDisabled(value);
  }

  inline void RenderStyleBase::SetIsForceHidden() noexcept
  {
    _computedStyle.SetIsForceHidden();
  }

  inline void RenderStyleBase::SetAutoRevealsWhenFound() noexcept
  {
    _computedStyle.SetAutoRevealsWhenFound();
  }

  inline void RenderStyleBase::SetInsideDefaultButton(bool value) noexcept
  {
    _computedStyle.SetInsideDefaultButton(value);
  }

  inline void RenderStyleBase::SetInsideSubmitButton(bool value) noexcept
  {
    _computedStyle.SetInsideSubmitButton(value);
  }

  inline void RenderStyleBase::SetUsedPositionOptionIndex(Maybe<size_t> index) noexcept
  {
    _computedStyle.SetUsedPositionOptionIndex(index);
  }

  inline void RenderStyleBase::SetEffectiveDisplay(DisplayType effectiveDisplay) noexcept
  {
    _computedStyle.SetEffectiveDisplay(effectiveDisplay);
  }

  // MARK: - Cache used values

  inline void RenderStyleBase::SetUsedAppearance(StyleAppearance appearance) noexcept
  {
    _computedStyle.SetUsedAppearance(appearance);
  }

  inline void RenderStyleBase::SetUsedTouchAction(style::TouchAction touchAction) noexcept
  {
    _computedStyle.SetUsedTouchAction(touchAction);
  }

  inline void RenderStyleBase::SetUsedContentVisibility(ContentVisibility usedContentVisibility) noexcept
  {
    _computedStyle.SetUsedContentVisibility(usedContentVisibility);
  }

  inline void RenderStyleBase::SetUsedZIndex(style::ZIndex index) noexcept
  {
    _computedStyle.SetUsedZIndex(index);
  }

  // MARK: - Pseudo element/style

  inline void RenderStyleBase::SetHasPseudoStyles(EnumSet<PseudoElementType> set) noexcept
  {
    _computedStyle.SetHasPseudoStyles(set);
  }

  inline void
    RenderStyleBase::SetPseudoElementIdentifier(Maybe<style::PseudoElementIdentifier> &&identifier) noexcept
  {
    _computedStyle.SetPseudoElementIdentifier(krys::move(identifier));
  }

  inline RenderStyle *RenderStyleBase::AddCachedPseudoStyle(UniquePtr<RenderStyle> pseudo) noexcept
  {
    return _computedStyle.AddCachedPseudoStyle(krys::move(pseudo));
  }

  // MARK: - Custom properties

  inline void RenderStyleBase::SetCustomPropertyValue(Ref<const style::CustomProperty> &&value,
                                                      bool isInherited) noexcept
  {
    _computedStyle.SetCustomPropertyValue(krys::move(value), isInherited);
  }

  // MARK: - Fonts

  inline void RenderStyleBase::SetLetterSpacingFromAnimation(style::LetterSpacing &&value) noexcept
  {
    _computedStyle.SetLetterSpacingFromAnimation(krys::move(value));
  }

  inline void RenderStyleBase::SetWordSpacingFromAnimation(style::WordSpacing &&value) noexcept
  {
    _computedStyle.SetWordSpacingFromAnimation(krys::move(value));
  }

  // MARK: - Zoom

  inline void RenderStyleBase::SetEvaluationTimeZoomEnabled(bool value) noexcept
  {
    _computedStyle.SetEvaluationTimeZoomEnabled(value);
  }

  inline void RenderStyleBase::SetDeviceScaleFactor(float value) noexcept
  {
    _computedStyle.SetDeviceScaleFactor(value);
  }

  inline void RenderStyleBase::SetUseSVGZoomRulesForLength(bool value) noexcept
  {
    _computedStyle.SetUseSVGZoomRulesForLength(value);
  }

  inline bool RenderStyleBase::SetUsedZoom(float zoomLevel) noexcept
  {
    return _computedStyle.SetUsedZoom(zoomLevel);
  }

  // MARK: - Aggregates

  inline style::Animations &RenderStyleBase::EnsureAnimations() noexcept
  {
    return _computedStyle.EnsureAnimations();
  }

  inline style::Transitions &RenderStyleBase::EnsureTransitions() noexcept
  {
    return _computedStyle.EnsureTransitions();
  }

  inline style::BackgroundLayers &RenderStyleBase::EnsureBackgroundLayers() noexcept
  {
    return _computedStyle.EnsureBackgroundLayers();
  }

  inline style::MaskLayers &RenderStyleBase::EnsureMaskLayers() noexcept
  {
    return _computedStyle.EnsureMaskLayers();
  }

  inline void RenderStyleBase::SetBackgroundLayers(style::BackgroundLayers &&layers) noexcept
  {
    _computedStyle.SetBackgroundLayers(krys::move(layers));
  }

  inline void RenderStyleBase::SetMaskLayers(style::MaskLayers &&layers) noexcept
  {
    _computedStyle.SetMaskLayers(krys::move(layers));
  }

  inline void RenderStyleBase::SetMaskBorder(style::MaskBorder &&image) noexcept
  {
    _computedStyle.SetMaskBorder(krys::move(image));
  }

  inline void RenderStyleBase::SetBorderImage(style::BorderImage &&image) noexcept
  {
    _computedStyle.SetBorderImage(krys::move(image));
  }

  inline void RenderStyleBase::SetPerspectiveOrigin(style::PerspectiveOrigin &&origin) noexcept
  {
    _computedStyle.SetPerspectiveOrigin(krys::move(origin));
  }

  inline void RenderStyleBase::SetTransformOrigin(style::TransformOrigin &&origin) noexcept
  {
    _computedStyle.SetTransformOrigin(krys::move(origin));
  }

  inline void RenderStyleBase::SetInsetBox(style::InsetBox &&box) noexcept
  {
    _computedStyle.SetInsetBox(krys::move(box));
  }

  inline void RenderStyleBase::SetMarginBox(style::MarginBox &&box) noexcept
  {
    _computedStyle.SetMarginBox(krys::move(box));
  }

  inline void RenderStyleBase::SetPaddingBox(style::PaddingBox &&box) noexcept
  {
    _computedStyle.SetPaddingBox(krys::move(box));
  }

  inline void RenderStyleBase::SetBorderRadius(style::BorderRadiusValue &&size) noexcept
  {
    _computedStyle.SetBorderRadius(krys::move(size));
  }

  inline void RenderStyleBase::SetBorderTop(BorderValue &&value) noexcept
  {
    _computedStyle.SetBorderTop(krys::move(value));
  }

  inline void RenderStyleBase::SetBorderRight(BorderValue &&value) noexcept
  {
    _computedStyle.SetBorderRight(krys::move(value));
  }

  inline void RenderStyleBase::SetBorderBottom(BorderValue &&value) noexcept
  {
    _computedStyle.SetBorderBottom(krys::move(value));
  }

  inline void RenderStyleBase::SetBorderLeft(BorderValue &&value) noexcept
  {
    _computedStyle.SetBorderLeft(krys::move(value));
  }

  // MARK: - Properties/descriptors that are not yet generated

  // FIXME: Support descriptors

  inline void RenderStyleBase::SetPageSize(style::PageSize &&pageSize) noexcept
  {
    _computedStyle.SetPageSize(krys::move(pageSize));
  }

  // FIXME: Add a type that encapsulates both caretColor() and hasAutoCaretColor().

  inline void RenderStyleBase::SetCaretColor(style::Color &&color) noexcept
  {
    _computedStyle.SetCaretColor(krys::move(color));
  }

  inline void RenderStyleBase::SetHasAutoCaretColor() noexcept
  {
    _computedStyle.SetHasAutoCaretColor();
  }

  inline void RenderStyleBase::SetVisitedLinkCaretColor(style::Color &&value) noexcept
  {
    _computedStyle.SetVisitedLinkCaretColor(krys::move(value));
  }

  inline void RenderStyleBase::SetHasVisitedLinkAutoCaretColor() noexcept
  {
    _computedStyle.SetHasVisitedLinkAutoCaretColor();
  }
}
