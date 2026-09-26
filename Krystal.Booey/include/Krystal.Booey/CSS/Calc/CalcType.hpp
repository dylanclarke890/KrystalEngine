#pragma once

#include "Krystal.Core/Assert.hpp"
#include "Krystal.Core/Numeric.hpp"
#include "Krystal.Core/Types/Array.hpp"
#include "Krystal.Core/Types/Maybe.hpp"

namespace krys::boo::css
{
  enum class Category : uint8;
  enum class UnitType : uint8;
}

namespace krys::boo::css::calc
{
  /// @see https://drafts.css-houdini.org/css-typed-om-1/#cssnumericvalue-base-type
  enum class BaseType : uint8
  {
    Length,
    Angle,
    Time,
    Frequency,
    Resolution,
    Flex,
    Percent
  };

  /// @see https://drafts.css-houdini.org/css-typed-om-1/#cssnumericvalue-percent-hint
  enum class PercentHint : uint8
  {
    Length = 1,
    Angle,
    Time,
    Frequency,
    Resolution,
    Flex
  };

  /// @see https://drafts.css-houdini.org/css-typed-om-1/#cssnumericvalue-type
  struct Type
  {
    using Exponent = int8;

    struct PercentHintValue
    {
    private:
      enum class InternalValue : uint8
      {
        None = 0,
        Length = static_cast<uint8>(PercentHint::Length),
        Angle = static_cast<uint8>(PercentHint::Angle),
        Time = static_cast<uint8>(PercentHint::Time),
        Frequency = static_cast<uint8>(PercentHint::Frequency),
        Resolution = static_cast<uint8>(PercentHint::Resolution),
        Flex = static_cast<uint8>(PercentHint::Flex)
      };

      InternalValue _value;

    public:
      constexpr PercentHintValue() noexcept : _value {InternalValue::None}
      {
      }

      constexpr PercentHintValue(PercentHint hint) noexcept : _value {static_cast<InternalValue>(hint)}
      {
      }

      KRYS_NODISCARD constexpr bool operator==(const PercentHintValue &) const noexcept = default;

      KRYS_NODISCARD constexpr explicit operator bool() const noexcept
      {
        return _value != InternalValue::None;
      }

      KRYS_NODISCARD constexpr PercentHint operator*() const noexcept
      {
        krys_debug_assert(_value != InternalValue::None);
        return static_cast<calc::PercentHint>(_value);
      }
    };

    Exponent Length = 0;
    Exponent Angle = 0;
    Exponent Time = 0;
    Exponent Frequency = 0;
    Exponent Resolution = 0;
    Exponent Flex = 0;
    Exponent Percent = 0;
    PercentHintValue PercentHint = {};

    constexpr bool operator==(const Type &) const noexcept = default;

    KRYS_NODISCARD constexpr static Type CreateNumber() noexcept
    {
      return {};
    }

    KRYS_NODISCARD constexpr static Type CreateLength() noexcept
    {
      return {.Length = 1};
    }

    KRYS_NODISCARD constexpr static Type CreateAngle() noexcept
    {
      return {.Angle = 1};
    }

    KRYS_NODISCARD constexpr static Type CreateTime() noexcept
    {
      return {.Time = 1};
    }

    KRYS_NODISCARD constexpr static Type CreateFrequency() noexcept
    {
      return {.Frequency = 1};
    }

    KRYS_NODISCARD constexpr static Type CreateResolution() noexcept
    {
      return {.Resolution = 1};
    }

    KRYS_NODISCARD constexpr static Type CreateFlex() noexcept
    {
      return {.Flex = 1};
    }

    KRYS_NODISCARD constexpr static Type CreatePercent() noexcept
    {
      return {.Percent = 1};
    }

    KRYS_NODISCARD static Type DetermineType(UnitType type) noexcept;

    KRYS_NODISCARD static PercentHintValue DeterminePercentHint(Category category) noexcept;

    KRYS_NODISCARD static Maybe<Type> Add(Type, Type) noexcept;

    KRYS_NODISCARD static Maybe<Type> Add(Maybe<Type> a, Type b) noexcept
    {
      if (!a)
      {
        return a;
      }

      return Add(*a, b);
    }

    KRYS_NODISCARD static Maybe<Type> Multiply(Type, Type) noexcept;

    KRYS_NODISCARD static Maybe<Type> Multiply(Maybe<Type> a, Type b) noexcept
    {
      if (!a)
      {
        return a;
      }

      return Multiply(*a, b);
    }

    KRYS_NODISCARD static Type Invert(Type);

    KRYS_NODISCARD static Maybe<Type> SameType(Type, Type) noexcept;

    KRYS_NODISCARD static Maybe<Type> ConsistentType(Type, Type) noexcept;

    KRYS_NODISCARD static Maybe<Type> MadeConsistent(Type base, Type input) noexcept;

    KRYS_NODISCARD constexpr static decltype(auto) AllBaseTypes() noexcept
    {
      return std::to_array({BaseType::Length, BaseType::Angle, BaseType::Time, BaseType::Frequency,
                            BaseType::Resolution, BaseType::Flex, BaseType::Percent});
    }

    KRYS_NODISCARD constexpr static decltype(auto) AllPotentialPercentHintTypes() noexcept
    {
      return std::to_array({PercentHint::Length, PercentHint::Angle, PercentHint::Time,
                            PercentHint::Frequency, PercentHint::Resolution, PercentHint::Flex});
    }

    KRYS_NODISCARD constexpr Exponent &operator[](BaseType type) noexcept
    {
      switch (type)
      {
        case BaseType::Length:     return Length;
        case BaseType::Angle:      return Angle;
        case BaseType::Time:       return Time;
        case BaseType::Frequency:  return Frequency;
        case BaseType::Resolution: return Resolution;
        case BaseType::Flex:       return Flex;
        case BaseType::Percent:    return Percent;
      }

      std::unreachable();
    }

    KRYS_NODISCARD constexpr Exponent operator[](BaseType type) const noexcept
    {
      switch (type)
      {
        case BaseType::Length:     return Length;
        case BaseType::Angle:      return Angle;
        case BaseType::Time:       return Time;
        case BaseType::Frequency:  return Frequency;
        case BaseType::Resolution: return Resolution;
        case BaseType::Flex:       return Flex;
        case BaseType::Percent:    return Percent;
      }

      std::unreachable();
    }

    KRYS_NODISCARD constexpr Exponent &operator[](calc::PercentHint hint) noexcept
    {
      switch (hint)
      {
        case PercentHint::Length:     return Length;
        case PercentHint::Angle:      return Angle;
        case PercentHint::Time:       return Time;
        case PercentHint::Frequency:  return Frequency;
        case PercentHint::Resolution: return Resolution;
        case PercentHint::Flex:       return Flex;
      }

      std::unreachable();
    }

    KRYS_NODISCARD constexpr Exponent operator[](calc::PercentHint hint) const noexcept
    {
      switch (hint)
      {
        case PercentHint::Length:     return Length;
        case PercentHint::Angle:      return Angle;
        case PercentHint::Time:       return Time;
        case PercentHint::Frequency:  return Frequency;
        case PercentHint::Resolution: return Resolution;
        case PercentHint::Flex:       return Flex;
      }

      std::unreachable();
    }

    constexpr void ApplyPercentHint(calc::PercentHint hint) noexcept
    {
      // https://drafts.css-houdini.org/css-typed-om-1/#apply-the-percent-hint

      // 1. If type doesn’t contain `hint`, set type[hint] to 0.
      //    No work required as we represent "doesn't contain" as `0`.

      // 2. If type contains "percent", add type["percent"] to type[hint], then set type["percent"] to 0.
      (*this)[hint] += (*this)[BaseType::Percent];
      (*this)[BaseType::Percent] = 0;

      // 3. Set type’s percent hint to `hint`.
      this->PercentHint = hint;
    }

    KRYS_NODISCARD constexpr bool HasNonPercentEntry() const noexcept
    {
      return Length || Angle || Time || Frequency || Resolution || Flex;
    }

    KRYS_NODISCARD constexpr bool AllNonZeroValuesEqual(Type otherType) const noexcept
    {
      for (auto baseType : AllBaseTypes())
      {
        auto value = (*this)[baseType];
        if (!value)
        {
          continue;
        }

        if (value != otherType[baseType])
        {
          return false;
        }
      }
      return true;
    }

    struct MatchingContext
    {
      bool AllowsPercentHint;
    };

    enum class Match : uint8
    {
      Number,
      Percent,
      Length,
      Angle,
      Time,
      Frequency,
      Resolution,
      Flex
    };

    template <Match... M>
    KRYS_NODISCARD constexpr bool
      MatchesAny(MatchingContext context = {.AllowsPercentHint = false}) const noexcept
    {
      if (!TypeMatcher<M...>::MatchesAny(*this))
      {
        return false;
      }

      // If the context in which the value is used does not allow <percentage> values, then the type must
      // additionally have a null percent hint to be considered matching.
      if (PercentHint && !context.AllowsPercentHint)
      {
        return false;
      }

      return true;
    }

    KRYS_NODISCARD bool Matches(Category category) const noexcept;

    /// @brief Returns the Category for Type, if there is one.
    KRYS_NODISCARD Maybe<Category> CalculationCategory() const noexcept;
  };

  static_assert(sizeof(Type) == 8);

  template <Type::Match... M>
  struct TypeMatcher
  {
    enum class Check
    {
      MustBeZero,
      MustBeOneOrZero,
      MustBeOne
    };

    template <Type::Match match>
    static constexpr bool ContainsMatch() noexcept
    {
      return ((M == match) || ...);
    }

    template <Type::Match match>
    static constexpr Check ComputeCheck() noexcept
    {
      if constexpr (ContainsMatch<match>())
      {
        if constexpr (sizeof...(M) == 1)
        {
          return Check::MustBeOne;
        }
        else
        {
          return Check::MustBeOneOrZero;
        }
      }
      else
      {
        return Check::MustBeZero;
      }
    }

    static constexpr Check ComputeCheck(BaseType baseType) noexcept
    {
      switch (baseType)
      {
        case BaseType::Length:     return ComputeCheck<Type::Match::Length>();
        case BaseType::Angle:      return ComputeCheck<Type::Match::Angle>();
        case BaseType::Time:       return ComputeCheck<Type::Match::Time>();
        case BaseType::Frequency:  return ComputeCheck<Type::Match::Frequency>();
        case BaseType::Resolution: return ComputeCheck<Type::Match::Resolution>();
        case BaseType::Flex:       return ComputeCheck<Type::Match::Flex>();
        case BaseType::Percent:    return ComputeCheck<Type::Match::Percent>();
      }

      std::unreachable();
    }

    static bool MatchesAny(Type type) noexcept
    {
      bool foundMatch = false;
      for (auto baseType : Type::AllBaseTypes())
      {
        switch (ComputeCheck(baseType))
        {
          case Check::MustBeZero:
          {
            if (type[baseType] != 0)
            {
              return false;
            }
            break;
          }
          case Check::MustBeOneOrZero:
          {
            if (type[baseType] != 1 && type[baseType] != 0)
            {
              return false;
            }

            if (type[baseType] == 1)
            {
              foundMatch = true;
            }
            break;
          }
          case Check::MustBeOne:
          {
            if (type[baseType] != 1)
            {
              return false;
            }
            foundMatch = true;
            break;
          }
        }
      }

      // If any of the match requests were `Number`, its ok if no match was found.
      if constexpr (ContainsMatch<Type::Match::Number>())
      {
        return true;
      }
      else
      {
        return foundMatch;
      }
    }
  };

  // Policy to apply to validate argument types.
  enum class AllowedTypes
  {
    Any,
    Number,
    NumberOrAngle
  };

  template <AllowedTypes allowed>
  KRYS_NODISCARD bool ValidateType(Type a) noexcept
  {
    if constexpr (allowed == AllowedTypes::Any)
    {
      return true;
    }
    else if constexpr (allowed == AllowedTypes::Number)
    {
      return a.MatchesAny<Type::Match::Number>({.AllowsPercentHint = true});
    }
    else if constexpr (allowed == AllowedTypes::NumberOrAngle)
    {
      return a.MatchesAny<Type::Match::Number, Type::Match::Angle>({.AllowsPercentHint = true});
    }
  }

  // Policy used to merge argument types.
  enum class MergePolicy
  {
    Same,
    Consistent
  };

  template <MergePolicy Policy>
  KRYS_NODISCARD Maybe<Type> MergeTypes(Type a, Type b) noexcept
  {
    if constexpr (Policy == MergePolicy::Same)
    {
      return Type::SameType(a, b);
    }
    else if constexpr (Policy == MergePolicy::Consistent)
    {
      return Type::ConsistentType(a, b);
    }
  }

  template <MergePolicy Policy>
  KRYS_NODISCARD Maybe<Type> MergeTypes(Maybe<Type> a, Type b) noexcept
  {
    if (!a)
    {
      return null;
    }

    return MergeTypes<Policy>(*a, b);
  }

  template <MergePolicy Policy>
  KRYS_NODISCARD Maybe<Type> MergeTypes(Maybe<Type> a, Maybe<Type> b) noexcept
  {
    if (!a || !b)
    {
      return null;
    }

    return MergeTypes<Policy>(*a, *b);
  }

  // Transform to apply on operation type.
  enum class OutputTransform
  {
    None,
    NumberMadeConsistent,
    AngleMadeConsistent,
  };

  template <OutputTransform Transform>
  inline Maybe<Type> TransformType(Type a) noexcept
  {
    if constexpr (Transform == OutputTransform::None)
    {
      return a;
    }
    else if constexpr (Transform == OutputTransform::NumberMadeConsistent)
    {
      return Type::MadeConsistent(Type::CreateNumber(), a);
    }
    else if constexpr (Transform == OutputTransform::AngleMadeConsistent)
    {
      return Type::MadeConsistent(Type::CreateAngle(), a);
    }
  }

  template <OutputTransform Transform>
  inline Maybe<Type> TransformType(Maybe<Type> a) noexcept
  {
    if (!a)
    {
      return null;
    }

    return TransformType<Transform>(*a);
  }
}