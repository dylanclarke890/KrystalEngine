#include "Krystal.Booey/CSS/Calc/CalcType.hpp"
#include "Krystal.Booey/CSS/Values/Primitives/NumericCategory.hpp"
#include "Krystal.Booey/CSS/Values/Units.hpp"

namespace krys::boo::css::calc
{
  Maybe<Type> Type::Add(Type type1, Type type2) noexcept
  {
    // https://drafts.css-houdini.org/css-typed-om-1/#cssnumericvalue-add-two-types

    // To add two types type1 and type2, perform the following steps:

    // 1.
    // Replace type1 with a fresh copy of type1, and type2 with a fresh copy of type2. Let finalType be a
    // new type with an initially empty ordered map and an initially null percent hint.
    auto finalType = Type {};

    // 2.
    if (type1.PercentHint && type2.PercentHint)
    {
      // If both type1 and type2 have non-null percent hints with different values. The types can’t be added.
      // Return failure.
      if (*type1.PercentHint != *type2.PercentHint)
      {
        return null;
      }
    }
    else if (type1.PercentHint && !type2.PercentHint)
    {
      // If type1 has a non-null percent hint hint and type2 doesn’t, apply the percent hint hint to type2.
      type2.PercentHint = *type1.PercentHint;
    }
    else if (!type1.PercentHint && type2.PercentHint)
    {
      // Vice versa if type2 has a non-null percent hint and type1 doesn’t.
      type1.PercentHint = *type2.PercentHint;
    }

    // 3.
    // - If all the entries of type1 with non-zero values are contained in type2 with the same value, and
    // vice-versa
    if (type1.AllNonZeroValuesEqual(type2) && type2.AllNonZeroValuesEqual(type1))
    {
      // Copy all of type1’s entries to finalType, and then copy all of type2’s entries to finalType that
      // finalType doesn’t already contain. Set finalType’s percent hint to type1’s percent hint. Return
      // finalType.
      finalType = type1;
      for (auto baseType : Type::AllBaseTypes())
      {
        if (type2[baseType] && !finalType[baseType])
        {
          finalType[baseType] = type2[baseType];
        }
      }
      return finalType;
    }

    // - If type1 and/or type2 contain "percent" with a non-zero value, and type1 and/or type2 contain a key
    // other than "percent" with a non-zero value
    if ((type1[BaseType::Percent] || type2[BaseType::Percent])
        && (type1.HasNonPercentEntry() || type2.HasNonPercentEntry()))
    {
      // For each base type other than "percent" hint:
      for (auto hint : Type::AllPotentialPercentHintTypes())
      {
        auto type1Copy = type1;
        auto type2Copy = type2;

        // 1. Provisionally apply the percent hint `hint` to both type1 and type2.
        type1.ApplyPercentHint(hint);
        type2.ApplyPercentHint(hint);

        // 2. If, afterwards, all the entries of type1 with non-zero values are contained in type2 with the
        // same value, and vice versa, then copy all of type1’s entries to finalType, and then copy all of
        // type2’s entries to finalType that finalType doesn’t already contain. Set finalType’s percent hint
        // to hint. Return finalType.
        if (type1.AllNonZeroValuesEqual(type2) && type2.AllNonZeroValuesEqual(type1))
        {
          finalType = type1;
          for (auto baseType : Type::AllBaseTypes())
          {
            if (type2[baseType] && !finalType[baseType])
            {
              finalType[baseType] = type2[baseType];
            }
          }
          finalType.PercentHint = hint;
          return finalType;
        }

        // 3. Otherwise, revert type1 and type2 to their state at the start of this loop.
        type1 = type1Copy;
        type2 = type2Copy;
      }

      // If the loop finishes without returning finalType, then the types can’t be added. Return failure.
      return null;
    }

    // - The types can’t be added. Return failure.
    return null;
  }

  // https://drafts.css-houdini.org/css-typed-om-1/#cssnumericvalue-multiply-two-types
  Maybe<Type> Type::Multiply(Type type1, Type type2) noexcept
  {
    // To multiply two types type1 and type2, perform the following steps:

    // 1.
    // Replace type1 with a fresh copy of type1, and type2 with a fresh copy of type2. Let finalType be a
    // new type with an initially empty ordered map and an initially null percent hint.

    auto finalType = Type {};

    // 2/3.
    // If both type1 and type2 have non-null percent hints with different values, the types can’t be
    // multiplied. Return failure.
    if (type1.PercentHint && type2.PercentHint)
    {
      if (*type1.PercentHint != *type2.PercentHint)
      {
        // The types can’t be multiplied. Return failure.
        return null;
      }
    }
    else if (type1.PercentHint && !type2.PercentHint)
    {
      // If type1 has a non-null percent hint hint and type2 doesn’t, apply the percent hint hint to type2.
      type2.PercentHint = *type1.PercentHint;
    }
    else if (!type1.PercentHint && type2.PercentHint)
    {
      // Vice versa if type2 has a non-null percent hint and type1 doesn’t.
      type1.PercentHint = *type2.PercentHint;
    }

    // 4. Copy all of type1’s entries to finalType, then for each `baseType` → `power` of type2:
    for (auto baseType : Type::AllBaseTypes())
    {
      // If finalType[baseType] exists, increment its value by power. Otherwise, set finalType[baseType] to
      // power.
      finalType[baseType] = static_cast<int8>(type1[baseType] + type2[baseType]);

      // TODO: revisit this later, for now accept that overflow may happen.
      // NOTE: This is amended in our implementation to return `failure` if exponent would overflow.
      // auto checkedExponent = checkedSum<int8>(type1[baseType], type2[baseType]);
      // if (checkedExponent.hasOverflowed())
      // {
      //   return null;
      // }
      // finalType[baseType] = checkedExponent.value();
    }

    // Set finalType’s percent hint to type1’s percent hint.
    finalType.PercentHint = type1.PercentHint;

    // 5. Return finalType.
    return finalType;
  }

  // https://drafts.css-houdini.org/css-typed-om-1/#cssnumericvalue-invert-a-type
  Type Type::Invert(Type type) noexcept
  {
    // To invert a type `type`, perform the following steps:

    // 1. Let result be a new type with an initially empty ordered map and a percent hint matching that of
    // type.
    auto result = Type {};
    result.PercentHint = type.PercentHint;

    // 2. For each unit → exponent of type, set result[unit] to (-1 * exponent).
    for (auto unit : Type::AllBaseTypes())
    {
      result[unit] = -1 * type[unit];
    }

    // 3. Return result.
    return result;
  }

  Maybe<Type> Type::SameType(Type a, Type b) noexcept
  {
    if (a != b)
    {
      return null;
    }

    return a;
  }

  // https://drafts.csswg.org/css-values-4/#css-consistent-type
  Maybe<Type> Type::ConsistentType(Type a, Type b) noexcept
  {
    // Two or more calculations have a consistent type if adding the types doesn’t result in failure. The
    // consistent type is the result of the type addition.
    return Add(a, b);
  }

  // https://drafts.csswg.org/css-values-4/#css-make-a-type-consistent
  Maybe<Type> Type::MadeConsistent(Type base, Type input) noexcept
  {
    // 1. If both base and input have different non-null percent hints, they can’t be made consistent.
    // Return failure.
    if (base.PercentHint && input.PercentHint && *base.PercentHint != *input.PercentHint)
    {
      return null;
    }

    // 2. If base has a null percent hint set base’s percent hint to input’s percent hint.
    if (!base.PercentHint)
    {
      base.PercentHint = input.PercentHint;
    }

    // 3. Return base.
    return base;
  }

  // Part of https://drafts.csswg.org/css-values-4/#determine-the-type-of-a-calculation
  Type Type::DetermineType(UnitType unitType) noexcept
  {
    switch (unitType)
    {
      case UnitType::Number:
      case UnitType::Integer:
      {
        // the type is «[ ]» (empty map)
        return Type {};
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
        // the type is «[ "length" → 1 ]»
        return Type {.Length = 1};
      }
      case UnitType::deg:
      case UnitType::rad:
      case UnitType::grad:
      case UnitType::turn:
      {
        // the type is «[ "angle" → 1 ]»
        return Type {.Angle = 1};
      }
      case UnitType::ms:
      case UnitType::s:
      {
        // the type is «[ "time" → 1 ]»
        return Type {.Time = 1};
      }
      case UnitType::hz:
      case UnitType::khz:
      {
        // the type is «[ "frequency" → 1 ]»
        return Type {.Frequency = 1};
      }
      case UnitType::dppx:
      case UnitType::x:
      case UnitType::dpi:
      case UnitType::dpcm:
      {
        // the type is «[ "resolution" → 1 ]»
        return Type {.Resolution = 1};
      }
      case UnitType::fr:
      {
        // the type is «[ "flex" → 1 ]»
        return Type {.Flex = 1};
      }
      case UnitType::Percentage:
      {
        // NOTE: This is the context-independent type. When context is available, this may be transformed
        // into a percent-hint.
        // the type is «[ "percent" → 1 ]».
        return Type {.Percent = 1};
      }
      case UnitType::Attr:
      case UnitType::Calc:
      case UnitType::CalcPercentageWithAngle:
      case UnitType::CalcPercentageWithLength:
      case UnitType::Dimension:
      case UnitType::FontFamily:
      case UnitType::Ident:
      case UnitType::PropertyId:
      case UnitType::QuirkyEm:
      case UnitType::String:
      case UnitType::Unknown:
      case UnitType::ValueId:
      case UnitType::CustomIdent:
      {
        break;
      }
    }

    krys_unreachable();
  }

  Type::PercentHintValue Type::DeterminePercentHint(css::Category category) noexcept
  {
    switch (category)
    {
      case css::Category::Integer:
      case css::Category::Number:
      case css::Category::Percentage:
      case css::Category::Length:
      case css::Category::Angle:
      case css::Category::Time:
      case css::Category::Frequency:
      case css::Category::Resolution:
      case css::Category::Flex:
      {
        return {};
      }
      case css::Category::LengthPercentage:
      {
        return PercentHint::Length;
      }
      case css::Category::AnglePercentage:
      {
        return PercentHint::Angle;
      }
    }

    krys_unreachable();
  }

  bool Type::Matches(css::Category category) const noexcept
  {
    switch (category)
    {
      case css::Category::Integer:
      case css::Category::Number:
      {
        return MatchesAny<Match::Number>();
      }
      case css::Category::Percentage:
      {
        return MatchesAny<Match::Percent>();
      }
      case css::Category::Length:
      {
        return MatchesAny<Match::Length>();
      }
      case css::Category::Angle:
      {
        return MatchesAny<Match::Angle>();
      }
      case css::Category::Time:
      {
        return MatchesAny<Match::Time>();
      }
      case css::Category::Frequency:
      {
        return MatchesAny<Match::Frequency>();
      }
      case css::Category::Resolution:
      {
        return MatchesAny<Match::Resolution>();
      }
      case css::Category::Flex:
      {
        return MatchesAny<Match::Flex>();
      }
      case css::Category::LengthPercentage:
      {
        return MatchesAny<Match::Length, Match::Percent>({.AllowsPercentHint = true});
      }
      case css::Category::AnglePercentage:
      {
        return MatchesAny<Match::Angle, Match::Percent>({.AllowsPercentHint = true});
      }
    }

    krys_unreachable();
  }

  Maybe<css::Category> Type::CalculationCategory() const noexcept
  {
    Maybe<BaseType> matchingUnit;
    for (auto unit : Type::AllBaseTypes())
    {
      if ((*this)[unit] == 0)
      {
        continue;
      }

      if ((*this)[unit] == 1)
      {
        if (matchingUnit)
        {
          // If `matchingUnit` has already been set, that means this type has a mixed base.
          return null;
        }
        matchingUnit = unit;
        continue;
      }

      // If `unit` is not 0 or 1, that means this type has a base type of a higher or negative power, which
      // no calculation categories match.
      return null;
    }

    if (!matchingUnit)
    {
      return css::Category::Number;
    }

    switch (*matchingUnit)
    {
      case BaseType::Length:
      {
        if (PercentHint)
        {
          return css::Category::LengthPercentage;
        }

        return css::Category::Length;
      }
      case BaseType::Angle:
      {
        if (PercentHint)
        {
          return css::Category::AnglePercentage;
        }
        return css::Category::Angle;
      }
      case BaseType::Time:
      {
        krys_assert(!PercentHint);
        return css::Category::Time;
      }
      case BaseType::Frequency:
      {
        krys_assert(!PercentHint);
        return css::Category::Frequency;
      }
      case BaseType::Resolution:
      {
        krys_assert(!PercentHint);
        return css::Category::Resolution;
      }
      case BaseType::Flex:
      {
        krys_assert(!PercentHint);
        return css::Category::Flex;
      }
      case BaseType::Percent:
      {
        krys_assert(!PercentHint);
        return css::Category::Percentage;
      }
    }

    krys_unreachable();
  }
}
