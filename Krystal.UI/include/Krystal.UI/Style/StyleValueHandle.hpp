#pragma once

#include "Krystal.Lib/NullableFloat.hpp"
#include "Krystal.Lib/Types.hpp"
#include "Krystal.UI/Style/StyleLength.hpp"

namespace Krys::UI
{
#pragma pack(push)
#pragma pack(1)

  /// @brief StyleValueHandle is a small (16-bit) handle to a length or number in a style. The value may be
  /// embedded directly in the handle if simple, or the handle may instead point to an index within a
  /// StyleValuePool. To read or write a value from a StyleValueHandle, use `StyleValuePool::Store()`, and
  /// `StyleValuePool::GetLength()`/ `StyleValuePool::GetNumber()`.
  class StyleValueHandle
  {
    friend class StyleValuePool;

    static constexpr uint16 HandleTypeMask = 0b0'000'000'000'000'111;
    static constexpr uint16 HandleIndexedMask = 0b0'000'000'000'001'000;
    static constexpr uint16 HandleValueMask = 0b1'111'111'111'110'000;

    enum class Type : uint8
    {
      Undefined,
      Point,
      Percent,
      Number,
      Auto,
      Keyword
    };

    // Intentionally leaving out auto as a fast path
    enum class Keyword : uint8_t
    {
      MaxContent,
      FitContent,
      Stretch
    };

  private:
    uint16 _repr {0};

  public:
    static constexpr StyleValueHandle Auto()
    {
      StyleValueHandle handle;
      handle.SetType(Type::Auto);
      return handle;
    }

    constexpr bool IsUndefined() const
    {
      return GetType() == Type::Undefined;
    }

    constexpr bool IsDefined() const
    {
      return !IsUndefined();
    }

    constexpr bool IsAuto() const
    {
      return GetType() == Type::Auto;
    }

  private:
    constexpr bool IsKeyword(Keyword keyword) const
    {
      return GetType() == Type::Keyword && Value() == static_cast<uint16_t>(keyword);
    }

    constexpr Type GetType() const
    {
      return static_cast<Type>(_repr & HandleTypeMask);
    }

    constexpr void SetType(Type handleType)
    {
      _repr &= (~HandleTypeMask);
      _repr |= static_cast<uint8_t>(handleType);
    }

    constexpr uint16_t Value() const
    {
      return _repr >> 4;
    }

    constexpr void SetValue(uint16_t value)
    {
      _repr &= (~HandleValueMask);
      _repr |= (value << 4);
    }

    constexpr bool IsValueIndexed() const
    {
      return (_repr & HandleIndexedMask) != 0;
    }

    constexpr void SetValueIsIndexed()
    {
      _repr |= HandleIndexedMask;
    }
  };

#pragma pack(pop)
}