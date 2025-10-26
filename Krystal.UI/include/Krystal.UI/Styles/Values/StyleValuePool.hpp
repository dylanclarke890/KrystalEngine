#pragma once

#include "Krystal.Lib/NullableFloat.hpp"
#include "Krystal.Lib/SmallValueBuffer.hpp"
#include "Krystal.Lib/Types.hpp"
#include "Krystal.UI/Styles/Values/StyleLength.hpp"
#include "Krystal.UI/Styles/Values/StyleSizeLength.hpp"
#include "Krystal.UI/Styles/Values/StyleValueHandle.hpp"
#include <cassert>

namespace Krys::UI
{
  /// @brief StyleValuePool allows compact storage for a sparse collection of assigned
  /// lengths and numbers. Values are referred to using StyleValueHandle. In most
  /// cases StyleValueHandle can embed the value directly, but if not, the value is
  /// stored within a buffer provided by the pool. The pool contains a fixed number
  /// of inline slots before falling back to heap allocating additional slots.
  class StyleValuePool
  {
    SmallValueBuffer<4> _buffer;

  public:
    void Store(StyleValueHandle &handle, StyleLength length)
    {
      if (length.IsUndefined())
      {
        handle.SetType(StyleValueHandle::Type::Undefined);
      }
      else if (length.IsAuto())
      {
        handle.SetType(StyleValueHandle::Type::Auto);
      }
      else
      {
        auto type = length.IsPoints() ? StyleValueHandle::Type::Point : StyleValueHandle::Type::Percent;
        StoreValue(handle, length.Value().Value(), type);
      }
    }

    void Store(StyleValueHandle &handle, StyleSizeLength sizeValue)
    {
      if (sizeValue.IsUndefined())
      {
        handle.SetType(StyleValueHandle::Type::Undefined);
      }
      else if (sizeValue.IsAuto())
      {
        handle.SetType(StyleValueHandle::Type::Auto);
      }
      else if (sizeValue.IsMaxContent())
      {
        StoreKeyword(handle, StyleValueHandle::Keyword::MaxContent);
      }
      else if (sizeValue.IsStretch())
      {
        StoreKeyword(handle, StyleValueHandle::Keyword::Stretch);
      }
      else if (sizeValue.IsFitContent())
      {
        StoreKeyword(handle, StyleValueHandle::Keyword::FitContent);
      }
      else
      {
        auto type = sizeValue.IsPoints() ? StyleValueHandle::Type::Point : StyleValueHandle::Type::Percent;
        StoreValue(handle, sizeValue.Value().Value(), type);
      }
    }

    void Store(StyleValueHandle &handle, NullableFloat number)
    {
      if (!number.HasValue())
      {
        handle.SetType(StyleValueHandle::Type::Undefined);
      }
      else
      {
        StoreValue(handle, number.Value(), StyleValueHandle::Type::Number);
      }
    }

    StyleLength GetLength(StyleValueHandle handle) const
    {
      if (handle.IsUndefined())
      {
        return StyleLength::Undefined();
      }
      else if (handle.IsAuto())
      {
        return StyleLength::Auto();
      }
      else
      {
        assert(handle.GetType() == StyleValueHandle::Type::Point
               || handle.GetType() == StyleValueHandle::Type::Percent);
        float value = (handle.IsValueIndexed()) ? std::bit_cast<float>(_buffer.GetU32(handle.Value()))
                                                : UnpackInlineInteger(handle.Value());

        return handle.GetType() == StyleValueHandle::Type::Point ? StyleLength::Points(value)
                                                                 : StyleLength::Percent(value);
      }
    }

    StyleSizeLength GetSize(StyleValueHandle handle) const
    {
      if (handle.IsUndefined())
      {
        return StyleSizeLength::Undefined();
      }
      else if (handle.IsAuto())
      {
        return StyleSizeLength::Auto();
      }
      else if (handle.IsKeyword(StyleValueHandle::Keyword::MaxContent))
      {
        return StyleSizeLength::MaxContent();
      }
      else if (handle.IsKeyword(StyleValueHandle::Keyword::FitContent))
      {
        return StyleSizeLength::FitContent();
      }
      else if (handle.IsKeyword(StyleValueHandle::Keyword::Stretch))
      {
        return StyleSizeLength::Stretch();
      }
      else
      {
        assert(handle.GetType() == StyleValueHandle::Type::Point
               || handle.GetType() == StyleValueHandle::Type::Percent);
        float value = (handle.IsValueIndexed()) ? std::bit_cast<float>(_buffer.GetU32(handle.Value()))
                                                : UnpackInlineInteger(handle.Value());

        return handle.GetType() == StyleValueHandle::Type::Point ? StyleSizeLength::Points(value)
                                                                 : StyleSizeLength::Percent(value);
      }
    }

    NullableFloat GetNumber(StyleValueHandle handle) const
    {
      if (handle.IsUndefined())
      {
        return NullableFloat {};
      }
      else
      {
        assert(handle.GetType() == StyleValueHandle::Type::Number);
        float value = (handle.IsValueIndexed()) ? std::bit_cast<float>(_buffer.GetU32(handle.Value()))
                                                : UnpackInlineInteger(handle.Value());
        return NullableFloat {value};
      }
    }

  private:
    void StoreValue(StyleValueHandle &handle, float value, StyleValueHandle::Type type)
    {
      handle.SetType(type);

      if (handle.IsValueIndexed())
      {
        auto newIndex = _buffer.Replace(handle.Value(), std::bit_cast<uint32_t>(value));
        handle.SetValue(newIndex);
      }
      else if (IsIntegerPackable(value))
      {
        handle.SetValue(PackInlineInteger(value));
      }
      else
      {
        auto newIndex = _buffer.Push(std::bit_cast<uint32_t>(value));
        handle.SetValue(newIndex);
        handle.SetValueIsIndexed();
      }
    }

    void StoreKeyword(StyleValueHandle &handle, StyleValueHandle::Keyword keyword)
    {
      handle.SetType(StyleValueHandle::Type::Keyword);

      if (handle.IsValueIndexed())
      {
        auto newIndex = _buffer.Replace(handle.Value(), static_cast<uint32_t>(keyword));
        handle.SetValue(newIndex);
      }
      else
      {
        handle.SetValue(static_cast<uint16_t>(keyword));
      }
    }

    static constexpr bool IsIntegerPackable(float f)
    {
      constexpr uint16_t kMaxInlineAbsValue = (1 << 11) - 1;

      auto i = static_cast<int32_t>(f);
      return static_cast<float>(i) == f && i >= -kMaxInlineAbsValue && i <= +kMaxInlineAbsValue;
    }

    static constexpr uint16_t PackInlineInteger(float value)
    {
      uint16_t isNegative = value < 0 ? 1 : 0;
      return static_cast<uint16_t>((isNegative << 11)
                                   | (static_cast<int32_t>(value) * (isNegative != 0u ? -1 : 1)));
    }

    static constexpr float UnpackInlineInteger(uint16_t value)
    {
      constexpr uint16_t kValueSignMask = 0b0'000'100'000'000'000;
      constexpr uint16_t kValueMagnitudeMask = 0b0'000'011'111'111'111;
      const bool isNegative = (value & kValueSignMask) != 0;
      return static_cast<float>((value & kValueMagnitudeMask) * (isNegative ? -1 : 1));
    }
  };
}