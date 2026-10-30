#pragma once

#include "Krystal.Booey/Style/Computed/ComputedStyleBase.hpp"

#include "Krystal.Booey/Animation/ViewTimeline.hpp"
#include "Krystal.Booey/Rendering/Style/StyleAppleColorFilterData.hpp"
#include "Krystal.Booey/Rendering/Style/StyleBackdropFilterData.hpp"
#include "Krystal.Booey/Rendering/Style/StyleBackgroundData.hpp"
#include "Krystal.Booey/Rendering/Style/StyleBorderImageData.hpp"
#include "Krystal.Booey/Rendering/Style/StyleBoxData.hpp"
#include "Krystal.Booey/Rendering/Style/StyleFilterData.hpp"
#include "Krystal.Booey/Rendering/Style/StyleFlexibleBoxData.hpp"
#include "Krystal.Booey/Rendering/Style/StyleFontData.hpp"
#include "Krystal.Booey/Rendering/Style/StyleGridData.hpp"
#include "Krystal.Booey/Rendering/Style/StyleGridItemData.hpp"
#include "Krystal.Booey/Rendering/Style/StyleInheritedData.hpp"
#include "Krystal.Booey/Rendering/Style/StyleMaskBorderData.hpp"
#include "Krystal.Booey/Rendering/Style/StyleMiscNonInheritedData.hpp"
#include "Krystal.Booey/Rendering/Style/StyleMultiColData.hpp"
#include "Krystal.Booey/Rendering/Style/StyleNonInheritedData.hpp"
#include "Krystal.Booey/Rendering/Style/StyleRareInheritedData.hpp"
#include "Krystal.Booey/Rendering/Style/StyleRareNonInheritedData.hpp"
#include "Krystal.Booey/Rendering/Style/StyleSurroundData.hpp"
#include "Krystal.Booey/Rendering/Style/StyleTransformData.hpp"
#include "Krystal.Booey/Rendering/Style/StyleVisitedLinkColorData.hpp"
#include "Krystal.Booey/Style/Values/Backgrounds/BorderImage.hpp"
#include "Krystal.Booey/Style/Values/Backgrounds/FillLayers.hpp"
#include "Krystal.Booey/Style/Values/Fonts/FontFamily.hpp"
#include "Krystal.Booey/Style/Values/Fonts/FontFeatureSettings.hpp"
#include "Krystal.Booey/Style/Values/Fonts/FontPalette.hpp"
#include "Krystal.Booey/Style/Values/Fonts/FontSizeAdjust.hpp"
#include "Krystal.Booey/Style/Values/Fonts/FontStyle.hpp"
#include "Krystal.Booey/Style/Values/Fonts/FontVariantAlternates.hpp"
#include "Krystal.Booey/Style/Values/Fonts/FontVariantEastAsian.hpp"
#include "Krystal.Booey/Style/Values/Fonts/FontVariantLigatures.hpp"
#include "Krystal.Booey/Style/Values/Fonts/FontVariantNumeric.hpp"
#include "Krystal.Booey/Style/Values/Fonts/FontVariationSettings.hpp"
#include "Krystal.Booey/Style/Values/Fonts/FontWeight.hpp"
#include "Krystal.Booey/Style/Values/Fonts/FontWidth.hpp"
#include "Krystal.Booey/Style/Values/Grid/GridTrackSizingDirection.hpp"
#include "Krystal.Booey/Style/Values/Masking/MaskBorder.hpp"
#include "Krystal.Booey/Style/Values/NonStandard/WebKitLocale.hpp"
#include "Krystal.Booey/Style/Values/Text/TextAlign.hpp"
#include "Krystal.Booey/Style/Values/Text/TextAutospace.hpp"
#include "Krystal.Booey/Style/Values/Text/TextSpacingTrim.hpp"
#include "Krystal.Booey/Style/Values/Text/TextTransform.hpp"
#include "Krystal.Booey/Style/Values/TextDecoration/TextDecorationLine.hpp"

namespace krys::boo::style
{
  // MARK: - ComputedStyleBase::NonInheritedFlags

  inline bool
    ComputedStyleBase::NonInheritedFlags::HasPseudoStyle(boo::PseudoElementType pseudo) const noexcept
  {
    krys_debug_assert(AllPublicPseudoElementTypes.Contains(pseudo));
    return EnumSet<boo::PseudoElementType>::FromRaw(PseudoBits).Contains(pseudo);
  }

  inline bool ComputedStyleBase::NonInheritedFlags::HasAnyPublicPseudoStyles() const noexcept
  {
    return !!PseudoBits;
  }

  // MARK: - Non-property getters

  inline bool ComputedStyleBase::UsesViewportUnits() const noexcept
  {
    return _nonInheritedFlags.UsesViewportUnits;
  }

  inline bool ComputedStyleBase::UsesContainerUnits() const noexcept
  {
    return _nonInheritedFlags.UsesContainerUnits;
  }

  inline bool ComputedStyleBase::UseTreeCountingFunctions() const noexcept
  {
    return _nonInheritedFlags.UseTreeCountingFunctions;
  }

  inline InsideLink ComputedStyleBase::GetInsideLink() const noexcept
  {
    return static_cast<boo::InsideLink>(_inheritedFlags.InsideLink);
  }

  inline bool ComputedStyleBase::IsLink() const noexcept
  {
    return _nonInheritedFlags.IsLink;
  }

  inline bool ComputedStyleBase::EmptyState() const noexcept
  {
    return _nonInheritedFlags.EmptyState;
  }

  inline bool ComputedStyleBase::FirstChildState() const noexcept
  {
    return _nonInheritedFlags.FirstChildState;
  }

  inline bool ComputedStyleBase::LastChildState() const noexcept
  {
    return _nonInheritedFlags.LastChildState;
  }

  inline bool ComputedStyleBase::HasExplicitlyInheritedProperties() const noexcept
  {
    return _nonInheritedFlags.HasExplicitlyInheritedProperties;
  }

  inline bool ComputedStyleBase::DisallowsFastPathInheritance() const noexcept
  {
    return _nonInheritedFlags.DisallowsFastPathInheritance;
  }

  inline bool ComputedStyleBase::EffectiveInert() const noexcept
  {
    return _rareInheritedData->EffectiveInert;
  }

  inline bool ComputedStyleBase::IsEffectivelyTransparent() const noexcept
  {
    return _rareInheritedData->EffectivelyTransparent;
  }

  inline bool ComputedStyleBase::InsideDefaultButton() const noexcept
  {
    return _rareInheritedData->InsideDefaultButton;
  }

  inline bool ComputedStyleBase::InsideSubmitButton() const noexcept
  {
    return _rareInheritedData->InsideSubmitButton;
  }

  inline bool ComputedStyleBase::IsInSubtreeWithBlendMode() const noexcept
  {
    return _rareInheritedData->IsInSubtreeWithBlendMode;
  }

  inline bool ComputedStyleBase::IsForceHidden() const noexcept
  {
    return _rareInheritedData->IsForceHidden;
  }

  inline bool ComputedStyleBase::HasDisplayAffectedByAnimations() const noexcept
  {
    return _nonInheritedData->MiscData->HasDisplayAffectedByAnimations;
  }

  inline bool ComputedStyleBase::TransformStyleForcedToFlat() const noexcept
  {
    return static_cast<bool>(_nonInheritedData->RareData->TransformStyleForcedToFlat);
  }

  inline bool ComputedStyleBase::UsesAnchorFunctions() const noexcept
  {
    return _nonInheritedData->RareData->UsesAnchorFunctions;
  }

  inline EnumSet<BoxAxis> ComputedStyleBase::AnchorFunctionScrollCompensatedAxes() const noexcept
  {
    return EnumSet<BoxAxis>::FromRaw(_nonInheritedData->RareData->AnchorFunctionScrollCompensatedAxes);
  }

  inline bool ComputedStyleBase::IsPopoverInvoker() const noexcept
  {
    return _nonInheritedData->RareData->IsPopoverInvoker;
  }

  inline bool ComputedStyleBase::AutoRevealsWhenFound() const noexcept
  {
    return _rareInheritedData->AutoRevealsWhenFound;
  }

  inline bool ComputedStyleBase::NativeAppearanceDisabled() const noexcept
  {
    return _nonInheritedData->RareData->NativeAppearanceDisabled;
  }

  inline EnumSet<EventListenerRegionType> ComputedStyleBase::EventListenerRegionTypes() const noexcept
  {
    return _rareInheritedData->EventListenerRegionTypes;
  }

  inline bool ComputedStyleBase::HasAttrContent() const noexcept
  {
    return _nonInheritedData->MiscData->HasAttrContent;
  }

  inline Maybe<size_t> ComputedStyleBase::UsedPositionOptionIndex() const noexcept
  {
    return _nonInheritedData->RareData->UsedPositionOptionIndex;
  }

  constexpr inline DisplayType ComputedStyleBase::OriginalDisplay() const noexcept
  {
    return static_cast<DisplayType>(_nonInheritedFlags.OriginalDisplay);
  }

  inline DisplayType ComputedStyleBase::EffectiveDisplay() const noexcept
  {
    return static_cast<DisplayType>(_nonInheritedFlags.EffectiveDisplay);
  }

  inline StyleAppearance ComputedStyleBase::UsedAppearance() const noexcept
  {
    return static_cast<StyleAppearance>(_nonInheritedData->MiscData->UsedAppearance);
  }

  inline ContentVisibility ComputedStyleBase::UsedContentVisibility() const noexcept
  {
    return static_cast<ContentVisibility>(_rareInheritedData->UsedContentVisibility);
  }

  inline TouchAction ComputedStyleBase::UsedTouchAction() const noexcept
  {
    return _rareInheritedData->UsedTouchAction;
  }

  inline ZIndex ComputedStyleBase::UsedZIndex() const noexcept
  {
    return _nonInheritedData->BoxData->UsedZIndex();
  }

  inline Maybe<PseudoElementType> ComputedStyleBase::GetPseudoElementType() const noexcept
  {
    return _nonInheritedFlags.PseudoElementType
             ? Maybe(static_cast<boo::PseudoElementType>(_nonInheritedFlags.PseudoElementType - 1))
             : null;
  }

  inline const css::CSSOMStringAtom &ComputedStyleBase::PseudoElementNameArgument() const
  {
    return _nonInheritedData->RareData->PseudoElementNameArgument;
  }

  inline bool ComputedStyleBase::HasPseudoStyle(PseudoElementType pseudo) const noexcept
  {
    return _nonInheritedFlags.HasPseudoStyle(pseudo);
  }

  inline bool ComputedStyleBase::HasAnyPublicPseudoStyles() const noexcept
  {
    return _nonInheritedFlags.HasAnyPublicPseudoStyles();
  }

  // MARK: - Custom properties

  inline const CustomPropertyData &ComputedStyleBase::InheritedCustomProperties() const
  {
    return _rareInheritedData->CustomProperties.Get();
  }

  inline const CustomPropertyData &ComputedStyleBase::NonInheritedCustomProperties() const
  {
    return _nonInheritedData->RareData->CustomProperties.Get();
  }

  // MARK: - Zoom

  inline bool ComputedStyleBase::EvaluationTimeZoomEnabled() const noexcept
  {
    return _rareInheritedData->EvaluationTimeZoomEnabled;
  }

  inline float ComputedStyleBase::DeviceScaleFactor() const noexcept
  {
    return _rareInheritedData->DeviceScaleFactor;
  }

  inline bool ComputedStyleBase::UseSVGZoomRulesForLength() const noexcept
  {
    return _nonInheritedData->RareData->UseSVGZoomRulesForLength;
  }

  inline float ComputedStyleBase::UsedZoom() const noexcept
  {
    return _rareInheritedData->UsedZoom;
  }

  inline ZoomFactor ComputedStyleBase::UsedZoomForLength() const noexcept
  {
    if (UseSVGZoomRulesForLength())
    {
      return ZoomFactor(1.0f, DeviceScaleFactor());
    }

    if (EvaluationTimeZoomEnabled())
    {
      return ZoomFactor(UsedZoom(), DeviceScaleFactor());
    }

    return ZoomFactor(1.0f, DeviceScaleFactor());
  }

  // MARK: - Fonts

  inline const FontCascade &ComputedStyleBase::GetFontCascade() const
  {
    return _inheritedData->FontData->FontCascade;
  }

  inline WebkitLocale ComputedStyleBase::ComputedLocale() const noexcept
  {
    return GetFontDescription().ComputedLocale();
  }

  inline float ComputedStyleBase::UsedLetterSpacing() const noexcept
  {
    return GetFontCascade().LetterSpacing();
  }

  inline float ComputedStyleBase::UsedWordSpacing() const noexcept
  {
    return GetFontCascade().WordSpacing();
  }

  // MARK: - Aggregates

  inline const InsetBox &ComputedStyleBase::GetInsetBox() const
  {
    return _nonInheritedData->SurroundData->Inset;
  }

  inline const MarginBox &ComputedStyleBase::GetMarginBox() const
  {
    return _nonInheritedData->SurroundData->Margin;
  }

  inline const PaddingBox &ComputedStyleBase::GetPaddingBox() const
  {
    return _nonInheritedData->SurroundData->Padding;
  }

  inline const ScrollMarginBox &ComputedStyleBase::GetScrollMarginBox() const
  {
    return _nonInheritedData->RareData->ScrollMargin;
  }

  inline const ScrollPaddingBox &ComputedStyleBase::GetScrollPaddingBox() const
  {
    return _nonInheritedData->RareData->ScrollPadding;
  }

  inline const ScrollTimelines &ComputedStyleBase::GetScrollTimelines() const
  {
    return _nonInheritedData->RareData->ScrollTimelines;
  }

  inline const ViewTimelines &ComputedStyleBase::GetViewTimelines() const
  {
    return _nonInheritedData->RareData->ViewTimelines;
  }

  inline const Animations &ComputedStyleBase::GetAnimations() const
  {
    return _nonInheritedData->MiscData->Animations;
  }

  inline const Transitions &ComputedStyleBase::GetTransitions() const
  {
    return _nonInheritedData->MiscData->Transitions;
  }

  inline const BackgroundLayers &ComputedStyleBase::GetBackgroundLayers() const
  {
    return _nonInheritedData->BackgroundData->Background;
  }

  inline const MaskLayers &ComputedStyleBase::GetMaskLayers() const
  {
    return _nonInheritedData->MiscData->Mask;
  }

  inline const MaskBorder &ComputedStyleBase::GetMaskBorder() const
  {
    return _nonInheritedData->RareData->MaskBorder->MaskBorder;
  }

  inline const BorderImage &ComputedStyleBase::GetBorderImage() const
  {
    return _nonInheritedData->SurroundData->Border.BorderImage->BorderImage;
  }

  inline const TransformOrigin &ComputedStyleBase::GetTransformOrigin() const
  {
    return _nonInheritedData->MiscData->Transform->Origin;
  }

  inline const PerspectiveOrigin &ComputedStyleBase::GetPerspectiveOrigin() const
  {
    return _nonInheritedData->RareData->PerspectiveOrigin;
  }

  inline const OutlineValue &ComputedStyleBase::Outline() const
  {
    return _nonInheritedData->BackgroundData->Outline;
  }

  inline const BorderData &ComputedStyleBase::Border() const
  {
    return _nonInheritedData->SurroundData->Border;
  }

  inline LineWidthBox ComputedStyleBase::BorderWidth() const noexcept
  {
    return Border().BorderWidth();
  }

  inline const BorderRadius &ComputedStyleBase::BorderRadii() const
  {
    return Border().Radii;
  }

  inline const BorderValue &ComputedStyleBase::BorderBottom() const
  {
    return Border().Bottom();
  }

  inline const BorderValue &ComputedStyleBase::BorderLeft() const
  {
    return Border().Left();
  }

  inline const BorderValue &ComputedStyleBase::BorderRight() const
  {
    return Border().Right();
  }

  inline const BorderValue &ComputedStyleBase::BorderTop() const
  {
    return Border().Top();
  }

  inline const BorderValue &ComputedStyleBase::ColumnRule() const
  {
    return _nonInheritedData->MiscData->MultiCol->ColumnRule;
  }

  // MARK: - Properties/descriptors that are not yet generated

  inline CursorType ComputedStyleBase::GetCursorType() const noexcept
  {
    return static_cast<boo::CursorType>(_inheritedFlags.CursorType);
  }

  // FIXME: Support descriptors

  inline const PageSize &ComputedStyleBase::GetPageSize() const
  {
    return _nonInheritedData->RareData->PageSize;
  }

  // FIXME: Add a type that encapsulates both caretColor() and hasAutoCaretColor().

  inline const Color &ComputedStyleBase::CaretColor() const
  {
    return _rareInheritedData->CaretColor;
  }

  inline bool ComputedStyleBase::HasAutoCaretColor() const noexcept
  {
    return _rareInheritedData->HasAutoCaretColor;
  }

  inline const Color &ComputedStyleBase::VisitedLinkCaretColor() const
  {
    return _rareInheritedData->VisitedLinkCaretColor;
  }

  inline bool ComputedStyleBase::HasVisitedLinkAutoCaretColor() const noexcept
  {
    return _rareInheritedData->HasVisitedLinkAutoCaretColor;
  }
}
