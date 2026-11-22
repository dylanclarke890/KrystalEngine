/*
 * Copyright (C) 2019 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above
 *    copyright notice, this list of conditions and the following
 *    disclaimer.
 * 2. Redistributions in binary form must reproduce the above
 *    copyright notice, this list of conditions and the following
 *    disclaimer in the documentation and/or other materials
 *    provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDER “AS IS” AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY,
 * OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
 * TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF
 * THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include "Krystal.Booey/CSS/TypedOM/CSSUnitValue.hpp"
#include "Krystal.Booey/CSS/Calc/Tree.hpp"
#include "Krystal.Booey/CSS/Calc/Value.hpp"
#include "Krystal.Booey/CSS/Parser/ParserFastPaths.hpp"
#include "Krystal.Booey/CSS/Values/NumberFormatting.hpp"
#include "Krystal.Booey/CSS/Values/PrimitiveValue.hpp"
#include "Krystal.Booey/CSS/Values/Units.hpp"
#include <cmath>

namespace krys::boo
{
  css::UnitType CSSUnitValue::ParseUnit(const CSSOMString &unit) noexcept
  {
    if (unit == u8"number")
    {
      return css::UnitType::Number;
    }
    if (unit == u8"percent")
    {
      return css::UnitType::Percentage;
    }
    return css::ParseUnitType(unit);
  }

  CSSOMStringView CSSUnitValue::Unit() const noexcept
  {
    switch (_unit)
    {
      case css::UnitType::Number:
      {
        return u8"number";
      }
      case css::UnitType::Percentage:
      {
        return u8"percent";
      }
      default:
      {
        break;
      }
    }
    return UnitSerialisation();
  }

  CSSOMStringView CSSUnitValue::UnitSerialisation() const noexcept
  {
    return css::UnitTypeString(_unit);
  }

  void CSSUnitValue::Serialise(StringBuilder &builder, EnumSet<SerialisationArguments>) const noexcept
  {
    FormatCSSNumber(builder, _value);
    builder.Append(UnitSerialisation());
  }

  ExceptionOr<Ref<CSSUnitValue>> CSSUnitValue::Create(double value, const CSSOMString &unit) noexcept
  {
    auto parsedUnit = ParseUnit(unit);
    if (parsedUnit == css::UnitType::Unknown)
    {
      return Exception {ExceptionCode::TypeError};
    }
    auto type = CSSNumericType::Create(parsedUnit);
    if (!type)
    {
      return Exception {ExceptionCode::TypeError};
    }
    auto unitValue = AdoptRef(*new CSSUnitValue(value, parsedUnit));
    unitValue->_type = krys::move(*type);
    return unitValue;
  }

  CSSUnitValue::CSSUnitValue(double value, css::UnitType unit) noexcept
      : CSSNumericValue(CSSNumericType::Create(unit).value_or(CSSNumericType())), _value(value), _unit(unit)
  {
  }

  RefPtr<CSSUnitValue> CSSUnitValue::ConvertTo(css::UnitType unit) const noexcept
  {
    // https://drafts.css-houdini.org/css-typed-om/#convert-a-cssunitvalue
    auto converted = css::ConvertUnit(_value, UnitEnum(), unit);
    if (!converted)
    {
      return nullptr;
    }

    return Create(*converted, unit);
  }

  auto CSSUnitValue::ToSumValue() const noexcept -> Maybe<SumValue>
  {
    // https://drafts.css-houdini.org/css-typed-om/#create-a-sum-value
    auto canonicalUnit = css::CanonicalUnitTypeForUnitType(_unit);
    if (canonicalUnit == css::UnitType::Unknown)
    {
      canonicalUnit = _unit;
    }

    auto convertedValue = _value * css::ConversionToCanonicalUnitsScaleFactor(UnitEnum()).value_or(1.0)
                          / css::ConversionToCanonicalUnitsScaleFactor(canonicalUnit).value_or(1.0);

    if (_unit == css::UnitType::Number)
    {
      return SumValue {Addend {convertedValue, {}}};
    }
    return SumValue {Addend {convertedValue, {{canonicalUnit, 1}}}};
  }

  bool CSSUnitValue::Equals(const CSSNumericValue &other) const noexcept
  {
    // https://drafts.css-houdini.org/css-typed-om/#equal-numeric-value
    auto *otherUnitValue = DynamicDowncast<CSSUnitValue>(other);
    if (!otherUnitValue)
    {
      return false;
    }
    return _value == otherUnitValue->_value && _unit == otherUnitValue->_unit;
  }

  RefPtr<css::Value> CSSUnitValue::ToCSSValue() const noexcept
  {
    return css::PrimitiveValue::Create(_value, _unit);
  }

  // FIXME: This function could be mostly generated from the property definitions.
  KRYS_NODISCARD static bool IsValueOutOfRangeForProperty(css::PropertyId propertyId, double value,
                                                          css::UnitType unit) noexcept
  {
    auto valueRange = css::ParserFastPaths::LengthValueRangeForPropertiesSupportingSimpleLengths(propertyId);
    if (valueRange && (value < valueRange->Min || value > valueRange->Max))
    {
      return true;
    }

    switch (propertyId)
    {
      case css::PropertyId::Order:
      case css::PropertyId::ZIndex:
      {
        return std::round(value) != value;
      }
      case css::PropertyId::TabSize:
      {
        return value < 0 || (unit == css::UnitType::Number && std::round(value) != value);
      }
      case css::PropertyId::Orphans:
      case css::PropertyId::Widows:
      case css::PropertyId::ColumnCount:
      {
        return std::round(value) != value || value < 1;
      }
      case css::PropertyId::AnimationDuration:
      case css::PropertyId::AnimationIterationCount:
      case css::PropertyId::BackgroundSize:
      case css::PropertyId::BlockSize:
      case css::PropertyId::BorderBlockEndWidth:
      case css::PropertyId::BorderBlockStartWidth:
      case css::PropertyId::BorderBottomLeftRadius:
      case css::PropertyId::BorderBottomRightRadius:
      case css::PropertyId::BorderBottomWidth:
      case css::PropertyId::BorderImageOutset:
      case css::PropertyId::BorderImageSlice:
      case css::PropertyId::BorderImageWidth:
      case css::PropertyId::BorderInlineEndWidth:
      case css::PropertyId::BorderInlineStartWidth:
      case css::PropertyId::BorderLeftWidth:
      case css::PropertyId::BorderRightWidth:
      case css::PropertyId::BorderTopLeftRadius:
      case css::PropertyId::BorderTopRightRadius:
      case css::PropertyId::BorderTopWidth:
      case css::PropertyId::ColumnGap:
      case css::PropertyId::ColumnRuleWidth:
      case css::PropertyId::ColumnWidth:
      case css::PropertyId::FlexBasis:
      case css::PropertyId::FlexGrow:
      case css::PropertyId::FlexShrink:
      case css::PropertyId::FontSize:
      case css::PropertyId::FontSizeAdjust:
      case css::PropertyId::FontWidth:
      case css::PropertyId::GridAutoColumns:
      case css::PropertyId::GridAutoRows:
      case css::PropertyId::GridTemplateColumns:
      case css::PropertyId::GridTemplateRows:
      case css::PropertyId::InlineSize:
      case css::PropertyId::LineHeight:
      case css::PropertyId::MaxBlockSize:
      case css::PropertyId::MaxInlineSize:
      case css::PropertyId::MaxHeight:
      case css::PropertyId::MaxWidth:
      case css::PropertyId::MinBlockSize:
      case css::PropertyId::MinInlineSize:
      case css::PropertyId::OutlineWidth:
      case css::PropertyId::Perspective:
      case css::PropertyId::R:
      case css::PropertyId::RowGap:
      case css::PropertyId::Rx:
      case css::PropertyId::Ry:
      case css::PropertyId::ScrollPaddingBlockEnd:
      case css::PropertyId::ScrollPaddingBlockStart:
      case css::PropertyId::ScrollPaddingBottom:
      case css::PropertyId::ScrollPaddingInlineEnd:
      case css::PropertyId::ScrollPaddingInlineStart:
      case css::PropertyId::ScrollPaddingLeft:
      case css::PropertyId::ScrollPaddingRight:
      case css::PropertyId::ScrollPaddingTop:
      case css::PropertyId::StrokeDasharray:
      case css::PropertyId::StrokeMiterlimit:
      case css::PropertyId::StrokeWidth:
      case css::PropertyId::TransitionDuration:
      {
        return value < 0;
      }
      case css::PropertyId::FontWeight:
      {
        return value < 1 || value > 1'000;
      }
      default:
      {
        return false;
      }
    }
  }

  KRYS_NODISCARD static css::Range RangeForProperty(css::PropertyId propertyId, css::UnitType) noexcept
  {
    // FIXME: Merge with IsValueOutOfRangeForProperty.

    if (auto valueRange =
          css::ParserFastPaths::LengthValueRangeForPropertiesSupportingSimpleLengths(propertyId))
    {
      return *valueRange;
    }

    switch (propertyId)
    {
      case css::PropertyId::AnimationDuration:
      case css::PropertyId::AnimationIterationCount:
      case css::PropertyId::BackgroundSize:
      case css::PropertyId::BlockSize:
      case css::PropertyId::BorderBlockEndWidth:
      case css::PropertyId::BorderBlockStartWidth:
      case css::PropertyId::BorderBottomLeftRadius:
      case css::PropertyId::BorderBottomRightRadius:
      case css::PropertyId::BorderBottomWidth:
      case css::PropertyId::BorderImageOutset:
      case css::PropertyId::BorderImageSlice:
      case css::PropertyId::BorderImageWidth:
      case css::PropertyId::BorderInlineEndWidth:
      case css::PropertyId::BorderInlineStartWidth:
      case css::PropertyId::BorderLeftWidth:
      case css::PropertyId::BorderRightWidth:
      case css::PropertyId::BorderTopLeftRadius:
      case css::PropertyId::BorderTopRightRadius:
      case css::PropertyId::BorderTopWidth:
      case css::PropertyId::ColumnGap:
      case css::PropertyId::ColumnRuleWidth:
      case css::PropertyId::ColumnWidth:
      case css::PropertyId::FlexBasis:
      case css::PropertyId::FlexGrow:
      case css::PropertyId::FlexShrink:
      case css::PropertyId::FontSize:
      case css::PropertyId::FontSizeAdjust:
      case css::PropertyId::FontWidth:
      case css::PropertyId::GridAutoColumns:
      case css::PropertyId::GridAutoRows:
      case css::PropertyId::GridTemplateColumns:
      case css::PropertyId::GridTemplateRows:
      case css::PropertyId::InlineSize:
      case css::PropertyId::LineHeight:
      case css::PropertyId::MaxBlockSize:
      case css::PropertyId::MaxInlineSize:
      case css::PropertyId::MaxHeight:
      case css::PropertyId::MaxWidth:
      case css::PropertyId::MinBlockSize:
      case css::PropertyId::MinInlineSize:
      case css::PropertyId::OutlineWidth:
      case css::PropertyId::Perspective:
      case css::PropertyId::R:
      case css::PropertyId::RowGap:
      case css::PropertyId::Rx:
      case css::PropertyId::Ry:
      case css::PropertyId::ScrollPaddingBlockEnd:
      case css::PropertyId::ScrollPaddingBlockStart:
      case css::PropertyId::ScrollPaddingBottom:
      case css::PropertyId::ScrollPaddingInlineEnd:
      case css::PropertyId::ScrollPaddingInlineStart:
      case css::PropertyId::ScrollPaddingLeft:
      case css::PropertyId::ScrollPaddingRight:
      case css::PropertyId::ScrollPaddingTop:
      case css::PropertyId::StrokeDasharray:
      case css::PropertyId::StrokeMiterlimit:
      case css::PropertyId::StrokeWidth:
      case css::PropertyId::TransitionDuration:
      case css::PropertyId::TabSize:
      case css::PropertyId::FontWeight:  // FIXME: Support more fine-grain ranges: `<number [1,1000]>`
      case css::PropertyId::Orphans:     // FIXME: Support more fine-grain ranges: `<integer [1,∞]>`
      case css::PropertyId::Widows:      // FIXME: Support more fine-grain ranges: `<integer [1,∞]>`
      case css::PropertyId::ColumnCount: // FIXME: Support more fine-grain ranges: `<integer [1,∞]>`
      {
        return css::NonNegative;
      }

      case css::PropertyId::Order:  // FIXME: Support more fine-grain ranges: `<integer>`
      case css::PropertyId::ZIndex: // FIXME: Support more fine-grain ranges: `<integer>`
      default:
      {
        return css::All;
      }
    }
  }

  KRYS_NODISCARD static css::Category CalculationCategoryForProperty(css::PropertyId,
                                                                     css::UnitType unit) noexcept
  {
    // FIXME: This should be looking up the supported calculation categories for the PropertyId and picking
    // the one that best matches the unit.

    switch (unit)
    {
      case css::UnitType::Number:
      case css::UnitType::Integer:
      {
        return css::Category::Number;
      }
      case css::UnitType::em:
      case css::UnitType::ex:
      case css::UnitType::px:
      case css::UnitType::cm:
      case css::UnitType::mm:
      case css::UnitType::in:
      case css::UnitType::pt:
      case css::UnitType::pc:
      case css::UnitType::Q:
      case css::UnitType::lh:
      case css::UnitType::cap:
      case css::UnitType::ch:
      case css::UnitType::ic:
      case css::UnitType::rcap:
      case css::UnitType::rch:
      case css::UnitType::rem:
      case css::UnitType::rex:
      case css::UnitType::ric:
      case css::UnitType::rlh:
      case css::UnitType::vw:
      case css::UnitType::vh:
      case css::UnitType::vmin:
      case css::UnitType::vmax:
      case css::UnitType::vb:
      case css::UnitType::vi:
      case css::UnitType::svw:
      case css::UnitType::svh:
      case css::UnitType::svmin:
      case css::UnitType::svmax:
      case css::UnitType::svb:
      case css::UnitType::svi:
      case css::UnitType::lvw:
      case css::UnitType::lvh:
      case css::UnitType::lvmin:
      case css::UnitType::lvmax:
      case css::UnitType::lvb:
      case css::UnitType::lvi:
      case css::UnitType::dvw:
      case css::UnitType::dvh:
      case css::UnitType::dvmin:
      case css::UnitType::dvmax:
      case css::UnitType::dvb:
      case css::UnitType::dvi:
      case css::UnitType::cqw:
      case css::UnitType::cqh:
      case css::UnitType::cqi:
      case css::UnitType::cqb:
      case css::UnitType::cqmin:
      case css::UnitType::cqmax:
      {
        return css::Category::Length;
      }
      case css::UnitType::Percentage:
      {
        return css::Category::Percentage;
      }
      case css::UnitType::deg:
      case css::UnitType::rad:
      case css::UnitType::grad:
      case css::UnitType::turn:
      {
        return css::Category::Angle;
      }
      case css::UnitType::ms:
      case css::UnitType::s:
      {
        return css::Category::Time;
      }
      case css::UnitType::hz:
      case css::UnitType::khz:
      {
        return css::Category::Frequency;
      }
      case css::UnitType::dppx:
      case css::UnitType::x:
      case css::UnitType::dpi:
      case css::UnitType::dpcm:
      {
        return css::Category::Resolution;
      }
      case css::UnitType::fr:
      {
        return css::Category::Flex;
      }
      default:
      {
        break;
      }
    }

    krys_debug_assert(false);
    return css::Category::Number;
  }

  RefPtr<css::Value> CSSUnitValue::ToCSSValueWithProperty(css::PropertyId propertyId) const noexcept
  {
    if (IsValueOutOfRangeForProperty(propertyId, _value, _unit))
    {
      // Wrap out of range values with a calc.

      auto node = ToCalcTreeNode();
      krys_debug_assert(node);
      auto type = css::calc::GetType(*node);

      auto range = RangeForProperty(propertyId, _unit);
      auto category = CalculationCategoryForProperty(propertyId, _unit);

      if (!type.Matches(category))
      {
        return nullptr;
      }

      List<css::calc::Child> sumChildren;
      sumChildren.push_back(krys::move(*node));
      auto sum = css::calc::CreateChild(css::calc::Sum {.Children = krys::move(sumChildren)}, type);

      return css::PrimitiveValue::Create(css::calc::Value::Create(category, range,
                                                                  css::calc::Tree {
                                                                    .Root = krys::move(sum),
                                                                    .Type = type,
                                                                    .Stage = css::calc::Stage::Specified,
                                                                  }));
    }
    return ToCSSValue();
  }

  Maybe<css::calc::Child> CSSUnitValue::ToCalcTreeNode() const noexcept
  {
    return css::calc::CreateNumeric(_value, _unit);
  }
}
