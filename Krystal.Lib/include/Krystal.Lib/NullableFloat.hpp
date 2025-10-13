#pragma once

#include <limits>

namespace Krys
{
  struct NullableFloat
  {
  private:
    float _value = std::numeric_limits<float>::quiet_NaN();

  public:
    explicit constexpr NullableFloat(float value) : _value(value)
    {
    }
    constexpr NullableFloat() = default;

    constexpr float Value() const
    {
      return _value;
    }

    constexpr float ValueOrDefault(float defaultValue) const
    {
      return HasValue() ? _value : defaultValue;
    }

    constexpr bool HasValue() const
    {
      return _value == _value; // NaN is the only value that is not equal to itself
    }
  };

  constexpr bool operator==(NullableFloat lhs, NullableFloat rhs)
  {
    return lhs.Value() == rhs.Value() || (!lhs.HasValue() && !rhs.HasValue());
  }
  
  constexpr bool operator!=(NullableFloat lhs, NullableFloat rhs)
  {
    return !(lhs == rhs);
  }

  constexpr bool operator==(NullableFloat lhs, float rhs)
  {
    return lhs == NullableFloat {rhs};
  }

  constexpr bool operator!=(NullableFloat lhs, float rhs)
  {
    return !(lhs == rhs);
  }

  constexpr bool operator==(float lhs, NullableFloat rhs)
  {
    return rhs == lhs;
  }

  constexpr bool operator!=(float lhs, NullableFloat rhs)
  {
    return !(lhs == rhs);
  }

  constexpr NullableFloat operator+(NullableFloat lhs, NullableFloat rhs)
  {
    return NullableFloat {lhs.Value() + rhs.Value()};
  }

  constexpr bool operator>(NullableFloat lhs, NullableFloat rhs)
  {
    return lhs.Value() > rhs.Value();
  }

  constexpr bool operator<(NullableFloat lhs, NullableFloat rhs)
  {
    return lhs.Value() < rhs.Value();
  }

  constexpr bool operator>=(NullableFloat lhs, NullableFloat rhs)
  {
    return lhs > rhs || lhs == rhs;
  }

  constexpr bool operator<=(NullableFloat lhs, NullableFloat rhs)
  {
    return lhs < rhs || lhs == rhs;
  }
}
