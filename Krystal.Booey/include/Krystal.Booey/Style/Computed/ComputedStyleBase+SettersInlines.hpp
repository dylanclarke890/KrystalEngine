#pragma once

#include "Krystal.Booey/Style/Computed/ComputedStyleBase+GettersInlines.hpp"
#include "Krystal.Core/Concepts.hpp"

#define SET_STYLE_PROPERTY_BASE(read, value, write)                                                          \
  do                                                                                                         \
  {                                                                                                          \
    if (!CompareEqual(read, value))                                                                          \
    {                                                                                                        \
      write;                                                                                                 \
    }                                                                                                        \
  } while (0)
#define SET_STYLE_PROPERTY(read, write, value) SET_STYLE_PROPERTY_BASE(read, value, write = value)
#define SET(group, variable, value) SET_STYLE_PROPERTY(group->variable, group.Access().variable, value)
#define SET_NESTED(group, parent, variable, value)                                                           \
  SET_STYLE_PROPERTY(group->parent->variable, group.Access().parent.Access().variable, value)
#define SET_DOUBLY_NESTED(group, grandparent, parent, variable, value)                                       \
  SET_STYLE_PROPERTY(group->grandparent->parent->variable,                                                   \
                     group.Access().grandparent.Access().parent.Access().variable, value)
#define SET_NESTED_STRUCT(group, parent, variable, value)                                                    \
  SET_STYLE_PROPERTY(group->parent.variable, group.Access().parent.variable, value)
#define SET_STYLE_PROPERTY_PAIR(read, write, variable1, value1, variable2, value2)                           \
  do                                                                                                         \
  {                                                                                                          \
    auto &readable = *read;                                                                                  \
    if (!CompareEqual(readable.variable1, value1) || !CompareEqual(readable.variable2, value2))              \
    {                                                                                                        \
      auto &writable = write;                                                                                \
      writable.variable1 = value1;                                                                           \
      writable.variable2 = value2;                                                                           \
    }                                                                                                        \
  } while (0)
#define SET_PAIR(group, variable1, value1, variable2, value2)                                                \
  SET_STYLE_PROPERTY_PAIR(group, group.Access(), variable1, value1, variable2, value2)
#define SET_NESTED_PAIR(group, parent, variable1, value1, variable2, value2)                                 \
  SET_STYLE_PROPERTY_PAIR(group->parent, group.Access().parent.Access(), variable1, value1, variable2, value2)
#define SET_DOUBLY_NESTED_PAIR(group, grandparent, parent, variable1, value1, variable2, value2)             \
  SET_STYLE_PROPERTY_PAIR(group->grandparent->parent, group.Access().grandparent.Access().parent.Access(),   \
                          variable1, value1, variable2, value2)

namespace krys::boo::style
{
  template <typename T, typename U>
  KRYS_NODISCARD inline bool CompareEqual(const T &a, const U &b) noexcept
  {
    // Flags stored in integer bit-fields are set from `bool`s.
    if constexpr (SameType<U, bool> && !SameType<T, bool>)
    {
      return static_cast<bool>(a) == b;
    }
    else
    {
      return a == b;
    }
  }

  // MARK: - ComputedStyleBase::NonInheritedFlags

  inline void ComputedStyleBase::NonInheritedFlags::SetHasPseudoStyles(
    EnumSet<boo::PseudoElementType> pseudoElementSet) noexcept
  {
    krys_debug_assert(pseudoElementSet);
    krys_debug_assert(pseudoElementSet.ContainsOnly(AllPublicPseudoElementTypes));
    PseudoBits = pseudoElementSet.ToRaw();
  }

  // MARK: - Non-property setters

  inline void ComputedStyleBase::SetUsesViewportUnits() noexcept
  {
    _nonInheritedFlags.UsesViewportUnits = true;
  }

  inline void ComputedStyleBase::SetUsesContainerUnits() noexcept
  {
    _nonInheritedFlags.UsesContainerUnits = true;
  }

  inline void ComputedStyleBase::SetUsesTreeCountingFunctions() noexcept
  {
    _nonInheritedFlags.UseTreeCountingFunctions = true;
  }

  inline void ComputedStyleBase::SetInsideLink(InsideLink InsideLink) noexcept
  {
    _inheritedFlags.InsideLink = static_cast<unsigned>(InsideLink);
  }

  inline void ComputedStyleBase::SetIsLink(bool IsLink) noexcept
  {
    _nonInheritedFlags.IsLink = IsLink;
  }

  inline void ComputedStyleBase::SetEmptyState(bool EmptyState) noexcept
  {
    _nonInheritedFlags.EmptyState = EmptyState;
  }

  inline void ComputedStyleBase::SetFirstChildState() noexcept
  {
    _nonInheritedFlags.FirstChildState = true;
  }

  inline void ComputedStyleBase::SetLastChildState() noexcept
  {
    _nonInheritedFlags.LastChildState = true;
  }

  inline void ComputedStyleBase::SetHasExplicitlyInheritedProperties() noexcept
  {
    _nonInheritedFlags.HasExplicitlyInheritedProperties = true;
  }

  inline void ComputedStyleBase::SetDisallowsFastPathInheritance() noexcept
  {
    _nonInheritedFlags.DisallowsFastPathInheritance = true;
  }

  inline void ComputedStyleBase::SetEffectiveInert(bool effectiveInert) noexcept
  {
    SET(_rareInheritedData, EffectiveInert, effectiveInert);
  }

  inline void ComputedStyleBase::SetIsEffectivelyTransparent(bool effectivelyTransparent) noexcept
  {
    SET(_rareInheritedData, EffectivelyTransparent, effectivelyTransparent);
  }

  inline void ComputedStyleBase::SetEventListenerRegionTypes(
    EnumSet<EventListenerRegionType> eventListenerTypes) noexcept
  {
    SET(_rareInheritedData, EventListenerRegionTypes, eventListenerTypes);
  }

  inline void ComputedStyleBase::SetHasAttrContent() noexcept
  {
    SET_NESTED(_nonInheritedData, MiscData, HasAttrContent, true);
  }

  inline void ComputedStyleBase::SetHasDisplayAffectedByAnimations() noexcept
  {
    SET_NESTED(_nonInheritedData, MiscData, HasDisplayAffectedByAnimations, true);
  }

  inline void ComputedStyleBase::SetTransformStyleForcedToFlat(bool b) noexcept
  {
    SET_NESTED(_nonInheritedData, RareData, TransformStyleForcedToFlat, static_cast<unsigned>(b));
  }

  inline void ComputedStyleBase::SetUsesAnchorFunctions() noexcept
  {
    SET_NESTED(_nonInheritedData, RareData, UsesAnchorFunctions, true);
  }

  inline void ComputedStyleBase::SetAnchorFunctionScrollCompensatedAxes(EnumSet<BoxAxis> axes) noexcept
  {
    SET_NESTED(_nonInheritedData, RareData, AnchorFunctionScrollCompensatedAxes, axes.ToRaw());
  }

  inline void ComputedStyleBase::SetIsPopoverInvoker() noexcept
  {
    SET_NESTED(_nonInheritedData, RareData, IsPopoverInvoker, true);
  }

  inline void ComputedStyleBase::SetNativeAppearanceDisabled(bool value) noexcept
  {
    SET_NESTED(_nonInheritedData, RareData, NativeAppearanceDisabled, value);
  }

  inline void ComputedStyleBase::SetIsForceHidden() noexcept
  {
    SET(_rareInheritedData, IsForceHidden, true);
  }

  inline void ComputedStyleBase::SetAutoRevealsWhenFound() noexcept
  {
    SET(_rareInheritedData, AutoRevealsWhenFound, true);
  }

  inline void ComputedStyleBase::SetInsideDefaultButton(bool value) noexcept
  {
    SET(_rareInheritedData, InsideDefaultButton, value);
  }

  inline void ComputedStyleBase::SetInsideSubmitButton(bool value) noexcept
  {
    SET(_rareInheritedData, InsideSubmitButton, value);
  }

  inline void ComputedStyleBase::SetUsedPositionOptionIndex(Maybe<size_t> index) noexcept
  {
    SET_NESTED(_nonInheritedData, RareData, UsedPositionOptionIndex, index);
  }

  inline void ComputedStyleBase::SetEffectiveDisplay(DisplayType EffectiveDisplay) noexcept
  {
    _nonInheritedFlags.EffectiveDisplay = static_cast<unsigned>(EffectiveDisplay);
  }

  inline void ComputedStyleBase::SetUsedAppearance(StyleAppearance a) noexcept
  {
    SET_NESTED(_nonInheritedData, MiscData, UsedAppearance, static_cast<unsigned>(a));
  }

  inline void ComputedStyleBase::SetUsedContentVisibility(ContentVisibility usedContentVisibility) noexcept
  {
    SET(_rareInheritedData, UsedContentVisibility, static_cast<unsigned>(usedContentVisibility));
  }

  inline void ComputedStyleBase::SetUsedTouchAction(TouchAction touchAction) noexcept
  {
    SET(_rareInheritedData, UsedTouchAction, touchAction);
  }

  inline void ComputedStyleBase::SetUsedZIndex(ZIndex index) noexcept
  {
    SET_NESTED_PAIR(_nonInheritedData, BoxData, HasAutoUsedZIndex, static_cast<uint8>(index._isAuto),
                    UsedZIndexValue, index._value);
  }

  // MARK: - Pseudo element/style

  inline void ComputedStyleBase::SetHasPseudoStyles(EnumSet<PseudoElementType> set) noexcept
  {
    _nonInheritedFlags.SetHasPseudoStyles(set);
  }

  inline void
    ComputedStyleBase::SetPseudoElementIdentifier(Maybe<PseudoElementIdentifier> &&identifier) noexcept
  {
    if (identifier)
    {
      _nonInheritedFlags.PseudoElementType = ToUnderlying(identifier->Type) + 1;
      SET_NESTED(_nonInheritedData, RareData, PseudoElementNameArgument,
                 krys::move(identifier->NameArgument));
    }
    else
    {
      _nonInheritedFlags.PseudoElementType = 0;
      SET_NESTED(_nonInheritedData, RareData, PseudoElementNameArgument, css::CSSOMStringAtom::Null());
    }
  }

  // MARK: - Zoom

  inline void ComputedStyleBase::SetEvaluationTimeZoomEnabled(bool value) noexcept
  {
    SET(_rareInheritedData, EvaluationTimeZoomEnabled, value);
  }

  inline void ComputedStyleBase::SetDeviceScaleFactor(float value) noexcept
  {
    SET(_rareInheritedData, DeviceScaleFactor, value);
  }

  inline void ComputedStyleBase::SetUseSVGZoomRulesForLength(bool value) noexcept
  {
    SET_NESTED(_nonInheritedData, RareData, UseSVGZoomRulesForLength, value);
  }

  inline bool ComputedStyleBase::SetUsedZoom(float zoomLevel) noexcept
  {
    if (CompareEqual(_rareInheritedData->UsedZoom, zoomLevel))
    {
      return false;
    }
    _rareInheritedData.Access().UsedZoom = zoomLevel;
    return true;
  }

  // MARK: - Aggregates

  inline Animations &ComputedStyleBase::EnsureAnimations()
  {
    return _nonInheritedData.Access().MiscData.Access().Animations.Access();
  }

  inline Transitions &ComputedStyleBase::EnsureTransitions()
  {
    return _nonInheritedData.Access().MiscData.Access().Transitions.Access();
  }

  inline BackgroundLayers &ComputedStyleBase::EnsureBackgroundLayers()
  {
    return _nonInheritedData.Access().BackgroundData.Access().Background.Access();
  }

  inline MaskLayers &ComputedStyleBase::EnsureMaskLayers()
  {
    return _nonInheritedData.Access().MiscData.Access().Mask.Access();
  }

  inline void ComputedStyleBase::SetBackgroundLayers(BackgroundLayers &&layers) noexcept
  {
    SET_NESTED(_nonInheritedData, BackgroundData, Background, krys::move(layers));
  }

  inline void ComputedStyleBase::SetMaskLayers(MaskLayers &&layers) noexcept
  {
    SET_NESTED(_nonInheritedData, MiscData, Mask, krys::move(layers));
  }

  inline void ComputedStyleBase::SetMaskBorder(MaskBorder &&image) noexcept
  {
    SET_DOUBLY_NESTED(_nonInheritedData, RareData, MaskBorder, MaskBorder, krys::move(image));
  }

  inline void ComputedStyleBase::SetBorderImage(BorderImage &&image) noexcept
  {
    SET_DOUBLY_NESTED(_nonInheritedData, SurroundData, Border.BorderImage, BorderImage, krys::move(image));
  }

  inline void ComputedStyleBase::SetPerspectiveOrigin(PerspectiveOrigin &&origin) noexcept
  {
    SET_NESTED(_nonInheritedData, RareData, PerspectiveOrigin, krys::move(origin));
  }

  inline void ComputedStyleBase::SetTransformOrigin(TransformOrigin &&origin) noexcept
  {
    SET_DOUBLY_NESTED(_nonInheritedData, MiscData, Transform, Origin, krys::move(origin));
  }

  inline void ComputedStyleBase::SetInsetBox(InsetBox &&box) noexcept
  {
    SET_NESTED(_nonInheritedData, SurroundData, Inset, krys::move(box));
  }

  inline void ComputedStyleBase::SetMarginBox(MarginBox &&box) noexcept
  {
    SET_NESTED(_nonInheritedData, SurroundData, Margin, krys::move(box));
  }

  inline void ComputedStyleBase::SetPaddingBox(PaddingBox &&box) noexcept
  {
    SET_NESTED(_nonInheritedData, SurroundData, Padding, krys::move(box));
  }

  inline void ComputedStyleBase::SetBorderRadius(BorderRadiusValue &&size) noexcept
  {
    SET_NESTED(_nonInheritedData, SurroundData, Border.TopLeftRadius(), size);
    SET_NESTED(_nonInheritedData, SurroundData, Border.TopRightRadius(), size);
    SET_NESTED(_nonInheritedData, SurroundData, Border.BottomLeftRadius(), size);
    SET_NESTED(_nonInheritedData, SurroundData, Border.BottomRightRadius(), krys::move(size));
  }

  void ComputedStyleBase::SetBorderTop(BorderValue &&value) noexcept
  {
    SET_NESTED(_nonInheritedData, SurroundData, Border.Edges.Top(), krys::move(value));
  }

  void ComputedStyleBase::SetBorderRight(BorderValue &&value) noexcept
  {
    SET_NESTED(_nonInheritedData, SurroundData, Border.Edges.Right(), krys::move(value));
  }

  void ComputedStyleBase::SetBorderBottom(BorderValue &&value) noexcept
  {
    SET_NESTED(_nonInheritedData, SurroundData, Border.Edges.Bottom(), krys::move(value));
  }

  void ComputedStyleBase::SetBorderLeft(BorderValue &&value) noexcept
  {
    SET_NESTED(_nonInheritedData, SurroundData, Border.Edges.Left(), krys::move(value));
  }

  // MARK: - Properties/descriptors that are not yet generated

  // FIXME: Support descriptors

  inline void ComputedStyleBase::SetPageSize(PageSize &&pageSize) noexcept
  {
    SET_NESTED(_nonInheritedData, RareData, PageSize, krys::move(pageSize));
  }

  // FIXME: Add a type that encapsulates both caretColor() and hasAutoCaretColor().

  inline void ComputedStyleBase::SetCaretColor(Color &&color) noexcept
  {
    SET_PAIR(_rareInheritedData, CaretColor, krys::move(color), HasAutoCaretColor, false);
  }

  inline void ComputedStyleBase::SetHasAutoCaretColor() noexcept
  {
    SET_PAIR(_rareInheritedData, HasAutoCaretColor, true, CaretColor, Color::GetCurrentColor());
  }

  inline void ComputedStyleBase::SetVisitedLinkCaretColor(Color &&value) noexcept
  {
    SET_PAIR(_rareInheritedData, VisitedLinkCaretColor, krys::move(value), HasVisitedLinkAutoCaretColor,
             false);
  }

  inline void ComputedStyleBase::SetHasVisitedLinkAutoCaretColor() noexcept
  {
    SET_PAIR(_rareInheritedData, HasVisitedLinkAutoCaretColor, true, VisitedLinkCaretColor,
             Color::GetCurrentColor());
  }
}

#undef SET
#undef SET_DOUBLY_NESTED
#undef SET_DOUBLY_NESTED_PAIR
#undef SET_NESTED
#undef SET_NESTED_PAIR
#undef SET_NESTED_STRUCT
#undef SET_PAIR
#undef SET_STYLE_PROPERTY
#undef SET_STYLE_PROPERTY_BASE
#undef SET_STYLE_PROPERTY_PAIR
