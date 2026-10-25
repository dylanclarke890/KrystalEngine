#include "Krystal.Booey/CSS/TypedOM/Numeric/CSSNumericType.hpp"
#include "Krystal.Booey/CSS/TypedOM/CSSNumericValue.hpp"
#include "Krystal.Booey/CSS/Values/Units.hpp"
#include "Krystal.Core/Text/StringBuilder.hpp"
#include <utility>

namespace krys::boo::css
{
  Maybe<CSSNumericType> CSSNumericType::Create(UnitType unit, int32 exponent) noexcept
  {
    // https://drafts.css-houdini.org/css-typed-om/#cssnumericvalue-create-a-type
    CSSNumericType type;
    switch (UnitCategoryForType(unit))
    {
      case UnitCategory::Number:
      {
        return type;
      }
      case UnitCategory::Percent:
      {
        type.Percent = exponent;
        return type;
      }
      case UnitCategory::AbsoluteLength:
      case UnitCategory::FontRelativeLength:
      case UnitCategory::ViewportPercentageLength:
      {
        type.Length = exponent;
        return type;
      }
      case UnitCategory::Angle:
      {
        type.Angle = exponent;
        return type;
      }
      case UnitCategory::Time:
      {
        type.Time = exponent;
        return type;
      }
      case UnitCategory::Frequency:
      {
        type.Frequency = exponent;
        return type;
      }
      case UnitCategory::Resolution:
      {
        type.Resolution = exponent;
        return type;
      }
      case UnitCategory::Flex:
      {
        type.Flex = exponent;
        return type;
      }
      case UnitCategory::Other:
      {
        break;
      }
    }

    return null;
  }

  Maybe<CSSNumericType> CSSNumericType::AddTypes(CSSNumericType a, CSSNumericType b) noexcept
  {
    // https://drafts.css-houdini.org/css-typed-om/#cssnumericvalue-add-two-types
    if (a.PercentHint && b.PercentHint && *a.PercentHint != *b.PercentHint)
    {
      return null;
    }

    if (a.PercentHint)
    {
      b.ApplyPercentHint(*a.PercentHint);
    }
    if (b.PercentHint)
    {
      a.ApplyPercentHint(*b.PercentHint);
    }

    if (a == b)
    {
      return a;
    }

    for (auto type : EachBaseType())
    {
      if (type == CSSNumericBaseType::Percent)
      {
        continue;
      }
      if (!a.ValueForType(type) && !b.ValueForType(type))
      {
        continue;
      }
      a.ApplyPercentHint(type);
      b.ApplyPercentHint(type);
      if (a.ValueForType(type) != b.ValueForType(type))
      {
        return null;
      }
    }

    return a;
  }

  template <typename TArgument>
  KRYS_NODISCARD static Maybe<CSSNumericType>
    TypeFromList(const List<Ref<CSSNumericValue>> &values,
                 Maybe<CSSNumericType> (*function)(TArgument, TArgument) noexcept) noexcept
  {
    if (values.empty())
    {
      return null;
    }
    Maybe<CSSNumericType> type = values[0]->Type();
    for (size_t i = 1; i < values.size(); ++i)
    {
      type = function(*type, values[i]->Type());
      if (!type)
      {
        return null;
      }
    }
    return type;
  }

  Maybe<CSSNumericType> CSSNumericType::AddTypes(const List<Ref<CSSNumericValue>> &values) noexcept
  {
    return TypeFromList<CSSNumericType>(values, &CSSNumericType::AddTypes);
  }

  Maybe<CSSNumericType> CSSNumericType::MultiplyTypes(const CSSNumericType &a,
                                                      const CSSNumericType &b) noexcept
  {
    // https://drafts.css-houdini.org/css-typed-om/#cssnumericvalue-multiply-two-types
    if (a.PercentHint && b.PercentHint && *a.PercentHint != *b.PercentHint)
    {
      return null;
    }

    auto add = [](BaseTypeStorage left, BaseTypeStorage right) -> BaseTypeStorage
    {
      if (!left)
      {
        return right;
      }
      if (!right)
      {
        return left;
      }
      return *left + *right;
    };

    return CSSNumericType {
      .Length = add(a.Length, b.Length),
      .Angle = add(a.Angle, b.Angle),
      .Time = add(a.Time, b.Time),
      .Frequency = add(a.Frequency, b.Frequency),
      .Resolution = add(a.Resolution, b.Resolution),
      .Flex = add(a.Flex, b.Flex),
      .Percent = add(a.Percent, b.Percent),
      .PercentHint = a.PercentHint ? a.PercentHint : b.PercentHint,
    };
  }

  Maybe<CSSNumericType> CSSNumericType::MultiplyTypes(const List<Ref<CSSNumericValue>> &values) noexcept
  {
    return TypeFromList<const CSSNumericType &>(values, &CSSNumericType::MultiplyTypes);
  }

  utf8_string CSSNumericType::DebugString() const noexcept
  {
    StringBuilder builder;
    builder.Append(u8'{');
    auto appendEntry = [&](utf8_stringview name, const BaseTypeStorage &value)
    {
      if (value)
      {
        builder.Append(u8' ', name, u8':', *value);
      }
    };
    appendEntry(u8"length", Length);
    appendEntry(u8"angle", Angle);
    appendEntry(u8"time", Time);
    appendEntry(u8"frequency", Frequency);
    appendEntry(u8"resolution", Resolution);
    appendEntry(u8"flex", Flex);
    appendEntry(u8"percent", Percent);
    if (PercentHint)
    {
      builder.Append(u8" percentHint:", css::DebugString(*PercentHint));
    }
    builder.Append(u8" }");
    return krys::move(builder).ToString();
  }

  auto CSSNumericType::ValueForType(CSSNumericBaseType type) noexcept -> BaseTypeStorage &
  {
    switch (type)
    {
      case CSSNumericBaseType::Length:     return Length;
      case CSSNumericBaseType::Angle:      return Angle;
      case CSSNumericBaseType::Time:       return Time;
      case CSSNumericBaseType::Frequency:  return Frequency;
      case CSSNumericBaseType::Resolution: return Resolution;
      case CSSNumericBaseType::Flex:       return Flex;
      case CSSNumericBaseType::Percent:    return Percent;
    }
    krys_unreachable();
  }

  void CSSNumericType::ApplyPercentHint(CSSNumericBaseType hint) noexcept
  {
    // https://drafts.css-houdini.org/css-typed-om/#apply-the-percent-hint
    auto &optional = ValueForType(hint);
    if (!optional)
    {
      optional = 0;
    }
    if (Percent)
    {
      *optional += *std::exchange(Percent, 0);
    }
    PercentHint = hint;
  }

  size_t CSSNumericType::NonZeroEntryCount() const noexcept
  {
    size_t count {0};
    count += Length && *Length;
    count += Angle && *Angle;
    count += Time && *Time;
    count += Frequency && *Frequency;
    count += Resolution && *Resolution;
    count += Flex && *Flex;
    count += Percent && *Percent;
    return count;
  }
}
