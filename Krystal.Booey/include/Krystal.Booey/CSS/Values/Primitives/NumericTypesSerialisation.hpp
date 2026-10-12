#pragma once

#include "Krystal.Booey/CSS/Types/CSSOMString.hpp"
#include "Krystal.Booey/CSS/Values/Primitives/NumericTypes.hpp"
#include "Krystal.Booey/CSS/Values/Types.hpp"
#include "Krystal.Core/Attributes.hpp"

namespace krys::boo::css
{
  // MARK: - Serialisation

  struct SerialisableNumber
  {
    double Value;
    CSSOMStringView Suffix;
  };

  /// @brief Appends a non-finite number as `infinity`, `-infinity` or `NaN`, multiplied by 1 of the suffix's
  /// unit if there is one (e.g. `infinity * 1px`).
  void FormatNonfiniteCSSNumberValue(CSSOMString &builder, const SerialisableNumber &number) noexcept;
  KRYS_NODISCARD CSSOMString FormatNonfiniteCSSNumberValue(const SerialisableNumber &number) noexcept;

  /// @brief Appends a number rounded to at most 6 decimal places, without trailing zeros, followed by its
  /// suffix.
  void FormatCSSNumberValue(CSSOMString &builder, const SerialisableNumber &number) noexcept;
  KRYS_NODISCARD CSSOMString FormatCSSNumberValue(const SerialisableNumber &number) noexcept;

  template <>
  struct Serialiser<SerialisableNumber>
  {
    void operator()(CSSOMString &builder, const SerialisationContext &,
                    const SerialisableNumber &number) noexcept
    {
      FormatCSSNumberValue(builder, number);
    }
  };

  template <NumericRaw RawType>
  struct Serialiser<RawType>
  {
    void operator()(CSSOMString &builder, const SerialisationContext &context, const RawType &value) noexcept
    {
      Serialise(builder, context, SerialisableNumber {value.Value, UnitString(value.Unit)});
    }
  };

  template <Range nR, Range pR, typename V>
  struct Serialiser<NumberOrPercentageResolvedToNumber<nR, pR, V>>
  {
    void operator()(CSSOMString &builder, const SerialisationContext &context,
                    const NumberOrPercentageResolvedToNumber<nR, pR, V> &value) noexcept
    {
      value.SwitchOn([&](const typename NumberOrPercentageResolvedToNumber<nR, pR, V>::Number &number)
                     { Serialise(builder, context, number); },
                     [&](const typename NumberOrPercentageResolvedToNumber<nR, pR, V>::Percentage &percentage)
                     {
                       if (auto raw = percentage.GetRaw())
                       {
                         Serialise(builder, context, NumberRaw<nR, V> {raw->Value / 100.0});
                       }
                       else
                       {
                         Serialise(builder, context, percentage);
                       }
                     });
    }
  };
}
