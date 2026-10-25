#include "Krystal.Booey/CSS/TypedOM/CSSUnitValue.hpp"
#include "Krystal.Booey/Core/NumberFormatting.hpp"
#include "Krystal.Booey/CSS/Calc/Tree.hpp"
#include "Krystal.Booey/CSS/Calc/Value.hpp"
#include "Krystal.Booey/CSS/Parser/ParserFastPaths.hpp"
#include "Krystal.Booey/CSS/Values/PrimitiveValue.hpp"
#include "Krystal.Booey/CSS/Values/Units.hpp"
#include <cmath>

namespace krys::boo::css
{
  UnitType CSSUnitValue::ParseUnit(const CSSOMString &unit) noexcept
  {
    if (unit == u8"number")
    {
      return UnitType::Number;
    }
    if (unit == u8"percent")
    {
      return UnitType::Percentage;
    }
    return ParseUnitType(unit);
  }

  CSSOMStringView CSSUnitValue::Unit() const noexcept
  {
    switch (_unit)
    {
      case UnitType::Number:
      {
        return u8"number";
      }
      case UnitType::Percentage:
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
    return PrimitiveValue::UnitTypeString(_unit);
  }

  void CSSUnitValue::Serialise(StringBuilder &builder, EnumSet<SerialisationArguments>) const noexcept
  {
    FormatCSSNumber(builder, _value);
    builder.Append(UnitSerialisation());
  }

  dom::ExceptionOr<Ref<CSSUnitValue>> CSSUnitValue::Create(double value, const CSSOMString &unit) noexcept
  {
    auto parsedUnit = ParseUnit(unit);
    if (parsedUnit == UnitType::Unknown)
    {
      return dom::Exception {dom::ExceptionCode::TypeError};
    }
    auto type = CSSNumericType::Create(parsedUnit);
    if (!type)
    {
      return dom::Exception {dom::ExceptionCode::TypeError};
    }
    auto unitValue = AdoptRef(*new CSSUnitValue(value, parsedUnit));
    unitValue->_type = krys::move(*type);
    return unitValue;
  }

  CSSUnitValue::CSSUnitValue(double value, UnitType unit) noexcept
      : CSSNumericValue(CSSNumericType::Create(unit).value_or(CSSNumericType())), _value(value), _unit(unit)
  {
  }

  RefPtr<CSSUnitValue> CSSUnitValue::ConvertTo(UnitType unit) const noexcept
  {
    // https://drafts.css-houdini.org/css-typed-om/#convert-a-cssunitvalue
    if (UnitCategoryForType(UnitEnum()) != UnitCategoryForType(unit))
    {
      return nullptr;
    }

    return Create(_value * ConversionToCanonicalUnitsScaleFactor(UnitEnum())
                    / ConversionToCanonicalUnitsScaleFactor(unit),
                  unit);
  }

  auto CSSUnitValue::ToSumValue() const noexcept -> Maybe<SumValue>
  {
    // https://drafts.css-houdini.org/css-typed-om/#create-a-sum-value
    auto canonicalUnit = CanonicalUnitTypeForUnitType(_unit);
    if (canonicalUnit == UnitType::Unknown)
    {
      canonicalUnit = _unit;
    }

    auto convertedValue = _value * ConversionToCanonicalUnitsScaleFactor(UnitEnum())
                          / ConversionToCanonicalUnitsScaleFactor(canonicalUnit);

    if (_unit == UnitType::Number)
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
    return PrimitiveValue::Create(_value, _unit);
  }

  // FIXME: This function could be mostly generated from the property definitions.
  KRYS_NODISCARD static bool IsValueOutOfRangeForProperty(PropertyId propertyId, double value,
                                                          UnitType unit) noexcept
  {
    auto valueRange = ParserFastPaths::LengthValueRangeForPropertiesSupportingSimpleLengths(propertyId);
    if (valueRange && (value < valueRange->Min || value > valueRange->Max))
    {
      return true;
    }

    switch (propertyId)
    {
      case PropertyId::Order:
      case PropertyId::ZIndex:
      {
        return std::round(value) != value;
      }
      case PropertyId::TabSize:
      {
        return value < 0 || (unit == UnitType::Number && std::round(value) != value);
      }
      case PropertyId::Orphans:
      case PropertyId::Widows:
      case PropertyId::ColumnCount:
      {
        return std::round(value) != value || value < 1;
      }
      case PropertyId::AnimationDuration:
      case PropertyId::AnimationIterationCount:
      case PropertyId::BackgroundSize:
      case PropertyId::BlockSize:
      case PropertyId::BorderBlockEndWidth:
      case PropertyId::BorderBlockStartWidth:
      case PropertyId::BorderBottomLeftRadius:
      case PropertyId::BorderBottomRightRadius:
      case PropertyId::BorderBottomWidth:
      case PropertyId::BorderImageOutset:
      case PropertyId::BorderImageSlice:
      case PropertyId::BorderImageWidth:
      case PropertyId::BorderInlineEndWidth:
      case PropertyId::BorderInlineStartWidth:
      case PropertyId::BorderLeftWidth:
      case PropertyId::BorderRightWidth:
      case PropertyId::BorderTopLeftRadius:
      case PropertyId::BorderTopRightRadius:
      case PropertyId::BorderTopWidth:
      case PropertyId::ColumnGap:
      case PropertyId::ColumnRuleWidth:
      case PropertyId::ColumnWidth:
      case PropertyId::FlexBasis:
      case PropertyId::FlexGrow:
      case PropertyId::FlexShrink:
      case PropertyId::FontSize:
      case PropertyId::FontSizeAdjust:
      case PropertyId::FontWidth:
      case PropertyId::GridAutoColumns:
      case PropertyId::GridAutoRows:
      case PropertyId::GridTemplateColumns:
      case PropertyId::GridTemplateRows:
      case PropertyId::InlineSize:
      case PropertyId::LineHeight:
      case PropertyId::MaxBlockSize:
      case PropertyId::MaxInlineSize:
      case PropertyId::MaxHeight:
      case PropertyId::MaxWidth:
      case PropertyId::MinBlockSize:
      case PropertyId::MinInlineSize:
      case PropertyId::OutlineWidth:
      case PropertyId::Perspective:
      case PropertyId::R:
      case PropertyId::RowGap:
      case PropertyId::Rx:
      case PropertyId::Ry:
      case PropertyId::ScrollPaddingBlockEnd:
      case PropertyId::ScrollPaddingBlockStart:
      case PropertyId::ScrollPaddingBottom:
      case PropertyId::ScrollPaddingInlineEnd:
      case PropertyId::ScrollPaddingInlineStart:
      case PropertyId::ScrollPaddingLeft:
      case PropertyId::ScrollPaddingRight:
      case PropertyId::ScrollPaddingTop:
      case PropertyId::StrokeDasharray:
      case PropertyId::StrokeMiterlimit:
      case PropertyId::StrokeWidth:
      case PropertyId::TransitionDuration:
      {
        return value < 0;
      }
      case PropertyId::FontWeight:
      {
        return value < 1 || value > 1'000;
      }
      default:
      {
        return false;
      }
    }
  }

  KRYS_NODISCARD static Range RangeForProperty(PropertyId propertyId, UnitType) noexcept
  {
    // FIXME: Merge with IsValueOutOfRangeForProperty.

    if (auto valueRange = ParserFastPaths::LengthValueRangeForPropertiesSupportingSimpleLengths(propertyId))
    {
      return *valueRange;
    }

    switch (propertyId)
    {
      case PropertyId::AnimationDuration:
      case PropertyId::AnimationIterationCount:
      case PropertyId::BackgroundSize:
      case PropertyId::BlockSize:
      case PropertyId::BorderBlockEndWidth:
      case PropertyId::BorderBlockStartWidth:
      case PropertyId::BorderBottomLeftRadius:
      case PropertyId::BorderBottomRightRadius:
      case PropertyId::BorderBottomWidth:
      case PropertyId::BorderImageOutset:
      case PropertyId::BorderImageSlice:
      case PropertyId::BorderImageWidth:
      case PropertyId::BorderInlineEndWidth:
      case PropertyId::BorderInlineStartWidth:
      case PropertyId::BorderLeftWidth:
      case PropertyId::BorderRightWidth:
      case PropertyId::BorderTopLeftRadius:
      case PropertyId::BorderTopRightRadius:
      case PropertyId::BorderTopWidth:
      case PropertyId::ColumnGap:
      case PropertyId::ColumnRuleWidth:
      case PropertyId::ColumnWidth:
      case PropertyId::FlexBasis:
      case PropertyId::FlexGrow:
      case PropertyId::FlexShrink:
      case PropertyId::FontSize:
      case PropertyId::FontSizeAdjust:
      case PropertyId::FontWidth:
      case PropertyId::GridAutoColumns:
      case PropertyId::GridAutoRows:
      case PropertyId::GridTemplateColumns:
      case PropertyId::GridTemplateRows:
      case PropertyId::InlineSize:
      case PropertyId::LineHeight:
      case PropertyId::MaxBlockSize:
      case PropertyId::MaxInlineSize:
      case PropertyId::MaxHeight:
      case PropertyId::MaxWidth:
      case PropertyId::MinBlockSize:
      case PropertyId::MinInlineSize:
      case PropertyId::OutlineWidth:
      case PropertyId::Perspective:
      case PropertyId::R:
      case PropertyId::RowGap:
      case PropertyId::Rx:
      case PropertyId::Ry:
      case PropertyId::ScrollPaddingBlockEnd:
      case PropertyId::ScrollPaddingBlockStart:
      case PropertyId::ScrollPaddingBottom:
      case PropertyId::ScrollPaddingInlineEnd:
      case PropertyId::ScrollPaddingInlineStart:
      case PropertyId::ScrollPaddingLeft:
      case PropertyId::ScrollPaddingRight:
      case PropertyId::ScrollPaddingTop:
      case PropertyId::StrokeDasharray:
      case PropertyId::StrokeMiterlimit:
      case PropertyId::StrokeWidth:
      case PropertyId::TransitionDuration:
      case PropertyId::TabSize:
      case PropertyId::FontWeight:  // FIXME: Support more fine-grain ranges: `<number [1,1000]>`
      case PropertyId::Orphans:     // FIXME: Support more fine-grain ranges: `<integer [1,∞]>`
      case PropertyId::Widows:      // FIXME: Support more fine-grain ranges: `<integer [1,∞]>`
      case PropertyId::ColumnCount: // FIXME: Support more fine-grain ranges: `<integer [1,∞]>`
      {
        return NonNegative;
      }

      case PropertyId::Order:  // FIXME: Support more fine-grain ranges: `<integer>`
      case PropertyId::ZIndex: // FIXME: Support more fine-grain ranges: `<integer>`
      default:
      {
        return All;
      }
    }
  }

  KRYS_NODISCARD static Category CalculationCategoryForProperty(PropertyId, UnitType unit) noexcept
  {
    // FIXME: This should be looking up the supported calculation categories for the PropertyId and picking
    // the one that best matches the unit.

    switch (unit)
    {
      case UnitType::Number:
      case UnitType::Integer:
      {
        return Category::Number;
      }
      case UnitType::em:
      case UnitType::ex:
      case UnitType::px:
      case UnitType::cm:
      case UnitType::mm:
      case UnitType::in:
      case UnitType::pt:
      case UnitType::pc:
      case UnitType::Q:
      case UnitType::lh:
      case UnitType::cap:
      case UnitType::ch:
      case UnitType::ic:
      case UnitType::rcap:
      case UnitType::rch:
      case UnitType::rem:
      case UnitType::rex:
      case UnitType::ric:
      case UnitType::rlh:
      case UnitType::vw:
      case UnitType::vh:
      case UnitType::vmin:
      case UnitType::vmax:
      case UnitType::vb:
      case UnitType::vi:
      case UnitType::svw:
      case UnitType::svh:
      case UnitType::svmin:
      case UnitType::svmax:
      case UnitType::svb:
      case UnitType::svi:
      case UnitType::lvw:
      case UnitType::lvh:
      case UnitType::lvmin:
      case UnitType::lvmax:
      case UnitType::lvb:
      case UnitType::lvi:
      case UnitType::dvw:
      case UnitType::dvh:
      case UnitType::dvmin:
      case UnitType::dvmax:
      case UnitType::dvb:
      case UnitType::dvi:
      case UnitType::cqw:
      case UnitType::cqh:
      case UnitType::cqi:
      case UnitType::cqb:
      case UnitType::cqmin:
      case UnitType::cqmax:
      {
        return Category::Length;
      }
      case UnitType::Percentage:
      {
        return Category::Percentage;
      }
      case UnitType::deg:
      case UnitType::rad:
      case UnitType::grad:
      case UnitType::turn:
      {
        return Category::Angle;
      }
      case UnitType::ms:
      case UnitType::s:
      {
        return Category::Time;
      }
      case UnitType::hz:
      case UnitType::khz:
      {
        return Category::Frequency;
      }
      case UnitType::dppx:
      case UnitType::x:
      case UnitType::dpi:
      case UnitType::dpcm:
      {
        return Category::Resolution;
      }
      case UnitType::fr:
      {
        return Category::Flex;
      }
      default:
      {
        break;
      }
    }

    krys_debug_assert(false);
    return Category::Number;
  }

  RefPtr<css::Value> CSSUnitValue::ToCSSValueWithProperty(PropertyId propertyId) const noexcept
  {
    if (IsValueOutOfRangeForProperty(propertyId, _value, _unit))
    {
      // Wrap out of range values with a calc.

      auto node = ToCalcTreeNode();
      krys_debug_assert(node);
      auto type = calc::GetType(*node);

      auto range = RangeForProperty(propertyId, _unit);
      auto category = CalculationCategoryForProperty(propertyId, _unit);

      if (!type.Matches(category))
      {
        return nullptr;
      }

      List<calc::Child> sumChildren;
      sumChildren.push_back(krys::move(*node));
      auto sum = calc::CreateChild(calc::Sum {.Children = krys::move(sumChildren)}, type);

      return PrimitiveValue::Create(calc::Value::Create(category, range,
                                                        calc::Tree {
                                                          .Root = krys::move(sum),
                                                          .Type = type,
                                                          .Stage = calc::Stage::Specified,
                                                        }));
    }
    return ToCSSValue();
  }

  Maybe<calc::Child> CSSUnitValue::ToCalcTreeNode() const noexcept
  {
    return calc::CreateNumeric(_value, _unit);
  }
}
