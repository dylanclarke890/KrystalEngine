#pragma once

#include "Krystal.Booey/Style/Computed/ComputedStyle+GettersInlines.hpp"
#include "Krystal.Booey/Style/Computed/ComputedStyle+InitialInlines.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesEvaluation.hpp"

#define COMPUTED_STYLE_PROPERTIES_SETTERS_INLINES_INCLUDE_TRAP 1
#include "Krystal.Booey/Style/Computed/ComputedStyleProperties+SettersInlines.hpp"
#undef COMPUTED_STYLE_PROPERTIES_SETTERS_INLINES_INCLUDE_TRAP

namespace krys::boo::style
{
  // MARK: - Non-property setters

  inline void ComputedStyle::InheritColumnPropertiesFrom(const ComputedStyle &parent) noexcept
  {
    _nonInheritedData.Access().MiscData.Access().MultiCol = parent._nonInheritedData->MiscData->MultiCol;
  }

  // MARK: - Style adjustment utilities

  inline void ComputedStyle::AddToTextDecorationLineInEffect(TextDecorationLine value) noexcept
  {
    _inheritedFlags.TextDecorationLineInEffect = TextDecorationLineInEffect().AddOrReplaceIfNotNone(value);
  }

  inline void ComputedStyle::ContainIntrinsicWidthAddAuto() noexcept
  {
    SetContainIntrinsicWidth(ContainIntrinsicWidth().AddingAuto());
  }

  inline void ComputedStyle::ContainIntrinsicHeightAddAuto() noexcept
  {
    SetContainIntrinsicHeight(ContainIntrinsicHeight().AddingAuto());
  }

  inline void ComputedStyle::SetGridAutoFlowDirection(GridAutoFlow::Direction direction) noexcept
  {
    if (_nonInheritedData->RareData->Grid->GridAutoFlow.GetDirection() != direction)
    {
      _nonInheritedData.Access().RareData.Access().Grid.Access().GridAutoFlow.SetDirection(direction);
    }
  }

  inline void ComputedStyle::ResetBorderBottom() noexcept
  {
    SetBorderBottom(BorderValue {});
  }

  inline void ComputedStyle::ResetBorderLeft() noexcept
  {
    SetBorderLeft(BorderValue {});
  }

  inline void ComputedStyle::ResetBorderRight() noexcept
  {
    SetBorderRight(BorderValue {});
  }

  inline void ComputedStyle::ResetBorderTop() noexcept
  {
    SetBorderTop(BorderValue {});
  }

  inline void ComputedStyle::ResetMargin() noexcept
  {
    SetMarginBox(MarginBox {0_css_px});
  }

  inline void ComputedStyle::ResetPadding() noexcept
  {
    SetPaddingBox(PaddingBox {0_css_px});
  }

  inline void ComputedStyle::ResetBorder() noexcept
  {
    ResetBorderExceptRadius();
    ResetBorderRadius();
  }

  inline void ComputedStyle::ResetBorderExceptRadius() noexcept
  {
    SetBorderImage(BorderImage {});
    ResetBorderTop();
    ResetBorderRight();
    ResetBorderBottom();
    ResetBorderLeft();
  }

  inline void ComputedStyle::ResetBorderRadius() noexcept
  {
    SetBorderTopLeftRadius(InitialBorderTopLeftRadius());
    SetBorderTopRightRadius(InitialBorderTopRightRadius());
    SetBorderBottomLeftRadius(InitialBorderBottomLeftRadius());
    SetBorderBottomRightRadius(InitialBorderBottomRightRadius());
  }
}
