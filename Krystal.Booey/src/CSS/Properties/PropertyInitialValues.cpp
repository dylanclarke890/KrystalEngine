#include "Krystal.Booey/CSS/Properties/PropertyInitialValues.hpp"
#include "Krystal.Booey/CSS/Properties/InitialValues.hpp"
#include "Krystal.Booey/CSS/Values/BorderImageSliceValue.hpp"
#include "Krystal.Booey/CSS/Values/BorderImageWidthValue.hpp"
#include "Krystal.Booey/CSS/Values/OffsetRotateValue.hpp"
#include "Krystal.Booey/CSS/Values/PrimitiveValue.hpp"
#include "Krystal.Booey/CSS/Values/QuadValue.hpp"
#include "Krystal.Booey/CSS/Values/RectBase.hpp"
#include "Krystal.Booey/CSS/Values/ValuePair.hpp"
#include "Krystal.Core/Visitor.hpp"

namespace krys::boo::css
{
  namespace
  {
    KRYS_NODISCARD bool IsValueIdPair(const Value &value, ValueId valueId) noexcept
    {
      return value.IsValuePairValue() && IsValueId(value.First(), valueId)
             && IsValueId(value.Second(), valueId);
    }

    KRYS_NODISCARD bool IsNumber(const PrimitiveValue &value, double number, UnitType type) noexcept
    {
      return value.PrimitiveType() == type && !value.IsCalculated()
             && value.ValueNoConversionDataRequired<double>() == number;
    }

    KRYS_NODISCARD bool IsNumber(const PrimitiveValue *value, double number, UnitType type) noexcept
    {
      return value && IsNumber(*value, number, type);
    }

    KRYS_NODISCARD bool IsNumber(const Value &value, double number, UnitType type) noexcept
    {
      return IsNumber(DynamicDowncast<PrimitiveValue>(value), number, type);
    }

    KRYS_NODISCARD bool IsNumber(const RectBase &quad, double number, UnitType type) noexcept
    {
      return IsNumber(quad.Top(), number, type) && IsNumber(quad.Right(), number, type)
             && IsNumber(quad.Bottom(), number, type) && IsNumber(quad.Left(), number, type);
    }

    KRYS_NODISCARD bool IsValueId(const RectBase &quad, ValueId valueId) noexcept
    {
      return css::IsValueId(quad.Top(), valueId) && css::IsValueId(quad.Right(), valueId)
             && css::IsValueId(quad.Bottom(), valueId) && css::IsValueId(quad.Left(), valueId);
    }

    KRYS_NODISCARD bool IsNumericQuad(const Value &value, double number, UnitType type) noexcept
    {
      return value.IsQuadValue() && IsNumber(value.GetQuad(), number, type);
    }
  }

  bool IsInitialValueForLonghand(PropertyId longhand, const Value &value) noexcept
  {
    if (value.IsImplicitInitialValue())
    {
      return true;
    }

    switch (longhand)
    {
      case PropertyId::BackgroundSize:
      case PropertyId::MaskSize:
      {
        if (IsValueIdPair(value, ValueId::Auto))
        {
          return true;
        }
        break;
      }
      case PropertyId::BorderImageOutset:
      case PropertyId::MaskBorderOutset:
      {
        if (IsNumericQuad(value, 0, UnitType::Number))
        {
          return true;
        }
        break;
      }
      case PropertyId::BorderImageRepeat:
      case PropertyId::MaskBorderRepeat:
      {
        if (IsValueIdPair(value, ValueId::Stretch))
        {
          return true;
        }
        break;
      }
      case PropertyId::BorderImageSlice:
      {
        if (auto *sliceValue = DynamicDowncast<BorderImageSliceValue>(value))
        {
          if (!sliceValue->Fill() && IsNumber(sliceValue->Slices(), 100, UnitType::Percentage))
          {
            return true;
          }
        }
        break;
      }
      case PropertyId::BorderImageWidth:
      {
        if (auto *widthValue = DynamicDowncast<BorderImageWidthValue>(value))
        {
          if (!widthValue->OverridesBorderWidths() && IsNumber(widthValue->Widths(), 1, UnitType::Number))
          {
            return true;
          }
        }
        break;
      }
      case PropertyId::OffsetRotate:
      {
        if (auto *rotateValue = DynamicDowncast<OffsetRotateValue>(value))
        {
          if (rotateValue->IsInitialValue())
          {
            return true;
          }
        }
        break;
      }
      case PropertyId::MaskBorderSlice:
      {
        if (auto *sliceValue = DynamicDowncast<BorderImageSliceValue>(value))
        {
          if (!sliceValue->Fill() && IsNumber(sliceValue->Slices(), 0, UnitType::Number))
          {
            return true;
          }
        }
        return false;
      }
      case PropertyId::MaskBorderWidth:
      {
        if (auto *widthValue = DynamicDowncast<BorderImageWidthValue>(value))
        {
          if (!widthValue->OverridesBorderWidths() && IsValueId(widthValue->Widths(), ValueId::Auto))
          {
            return true;
          }
        }
        break;
      }
      default: break;
    }

    return SwitchOn(
      InitialValueForLonghand(longhand), [&](ValueId initialValue)
      { return css::IsValueId(value, initialValue); }, [&](InitialNumericValue initialValue)
      { return IsNumber(value, initialValue.Number, initialValue.Type); });
  }

  CSSOMStringView InitialValueTextForLonghand(PropertyId longhand) noexcept
  {
    return SwitchOn(
      InitialValueForLonghand(longhand), [](ValueId value) { return ToString(value); },
      [](InitialNumericValue initialValue) -> CSSOMStringView
      {
        switch (initialValue.Type)
        {
          case UnitType::Number:
          {
            if (initialValue.Number == 0.0)
            {
              return u8"0";
            }
            if (initialValue.Number == 1.0)
            {
              return u8"1";
            }
            if (initialValue.Number == 2.0)
            {
              return u8"2";
            }
            if (initialValue.Number == 4.0)
            {
              return u8"4";
            }
            if (initialValue.Number == 8.0)
            {
              return u8"8";
            }
            break;
          }
          case UnitType::Percentage:
          {
            if (initialValue.Number == 0.0)
            {
              return u8"0%";
            }
            if (initialValue.Number == 50.0)
            {
              return u8"50%";
            }
            if (initialValue.Number == 100.0)
            {
              return u8"100%";
            }
            break;
          }
          case UnitType::px:
          {
            if (initialValue.Number == 0.0)
            {
              return u8"0px";
            }
            if (initialValue.Number == 1.0)
            {
              return u8"1px";
            }
            break;
          }
          case UnitType::s:
          {
            if (initialValue.Number == 0.0)
            {
              return u8"0s";
            }
            break;
          }
          case UnitType::deg:
          {
            if (initialValue.Number == 0.0)
            {
              return u8"0deg";
            }
            break;
          }
          default: break;
        }

        krys_debug_assert_msg(false, "unexpected initial value");
        return u8"";
      });
  }

  ValueId InitialValueIdForLonghand(PropertyId longhand) noexcept
  {
    return SwitchOn(
      InitialValueForLonghand(longhand), [](ValueId value) { return value; },
      [](InitialNumericValue) { return ValueId::Invalid; });
  }
}
