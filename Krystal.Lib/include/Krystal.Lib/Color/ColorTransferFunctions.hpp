#pragma once

#include "Krystal.Lib/Maths.hpp"

// Transfer functions for colors that can be gamma encoded.
namespace Krys
{
  enum class TransferFunctionMode : uint8
  {
    Clamped,
    Unclamped
  };

  template <typename T, TransferFunctionMode mode>
  struct A98RGBTransferFunction
  {
    KRYS_NODISCARD static T ToGammaEncoded(T) noexcept;
    KRYS_NODISCARD static T ToLinear(T) noexcept;
  };

  template <typename T, TransferFunctionMode mode>
  struct ProPhotoRGBTransferFunction
  {
    constexpr static T gamma = 1.8;

    KRYS_NODISCARD static T ToGammaEncoded(T) noexcept;
    KRYS_NODISCARD static T ToLinear(T) noexcept;
  };

  template <typename T, TransferFunctionMode mode>
  struct Rec2020TransferFunction
  {
    constexpr static T alpha = 1.09929682680944;
    constexpr static T beta = 0.018053968510807;
    constexpr static T gamma = 0.45;

    KRYS_NODISCARD static T ToGammaEncoded(T) noexcept;
    KRYS_NODISCARD static T ToLinear(T) noexcept;
  };

  template <typename T, TransferFunctionMode mode>
  struct SRGBTransferFunction
  {
    KRYS_NODISCARD static T ToGammaEncoded(T) noexcept;
    KRYS_NODISCARD static T ToLinear(T) noexcept;
  };

#pragma region A98RGBTransferFunction

  template <typename T, TransferFunctionMode mode>
  T A98RGBTransferFunction<T, mode>::ToGammaEncoded(T c) noexcept
  {
    auto sign = std::signbit(c) ? -1.0f : 1.0f;
    auto result = std::pow(std::abs(c), 256.0f / 563.0f) * sign;
    if constexpr (mode == TransferFunctionMode::Clamped)
    {
      return ClampTo<T>(result, 0, 1);
    }

    return result;
  }

  template <typename T, TransferFunctionMode mode>
  T A98RGBTransferFunction<T, mode>::ToLinear(T c) noexcept
  {
    auto sign = std::signbit(c) ? -1.0f : 1.0f;
    auto result = std::pow(std::abs(c), 563.0f / 256.0f) * sign;
    if constexpr (mode == TransferFunctionMode::Clamped)
    {
      return ClampTo<T>(result, 0, 1);
    }

    return result;
  }

#pragma endregion

#pragma region ProPhotoRGBTransferFunction

  template <typename T, TransferFunctionMode mode>
  T ProPhotoRGBTransferFunction<T, mode>::ToGammaEncoded(T c) noexcept
  {
    if constexpr (mode == TransferFunctionMode::Clamped)
    {
      if (c < 1.0 / 512.0)
      {
        return 16.0 * c;
      }

      return ClampTo<T>(std::pow(c, 1.0 / gamma), 0, 1);
    }
    else
    {
      if (std::abs(c) < 1.0 / 512.0)
      {
        return 16.0 * c;
      }

      float sign = std::signbit(c) ? -1.0 : 1.0;
      return std::pow(c, 1.0 / gamma) * sign;
    }
  }

  template <typename T, TransferFunctionMode mode>
  T ProPhotoRGBTransferFunction<T, mode>::ToLinear(T c) noexcept
  {
    if constexpr (mode == TransferFunctionMode::Clamped)
    {
      if (c <= 16.0 / 512.0)
      {
        return c / 16.0;
      }

      return ClampTo<T>(std::pow(c, gamma), 0, 1);
    }
    else
    {
      if (std::abs(c) <= 16.0 / 512.0)
      {
        return c / 16.0;
      }

      float sign = std::signbit(c) ? -1.0 : 1.0;
      return std::pow(c, gamma) * sign;
    }
  }

#pragma endregion

#pragma region Rec2020TransferFunction

  template <typename T, TransferFunctionMode mode>
  T Rec2020TransferFunction<T, mode>::ToGammaEncoded(T c) noexcept
  {
    if constexpr (mode == TransferFunctionMode::Clamped)
    {
      if (c <= beta)
      {
        return 4.5f * c;
      }

      return ClampTo<T>(alpha * std::pow(c, gamma) - (alpha - 1.0f), 0, 1);
    }
    else
    {
      if (std::abs(c) <= beta)
      {
        return 4.5f * c;
      }

      float sign = std::signbit(c) ? -1.0 : 1.0;
      return (alpha * std::pow(c, gamma) - (alpha - 1.0)) * sign;
    }
  }

  template <typename T, TransferFunctionMode mode>
  T Rec2020TransferFunction<T, mode>::ToLinear(T c) noexcept
  {
    if constexpr (mode == TransferFunctionMode::Clamped)
    {
      if (c < beta * 4.5f)
      {
        return c / 4.5f;
      }

      return ClampTo<T>(std::pow((c + alpha - 1.0) / alpha, 1.0 / gamma), 0, 1);
    }
    else
    {
      if (std::abs(c) < beta * 4.5f)
      {
        return c / 4.5f;
      }

      float sign = std::signbit(c) ? -1.0 : 1.0;
      return std::pow((c + alpha - 1.0) / alpha, 1.0 / gamma) * sign;
    }
  }

#pragma endregion

#pragma region SRGBTransferFunction

  template <typename T, TransferFunctionMode mode>
  T SRGBTransferFunction<T, mode>::ToGammaEncoded(T c) noexcept
  {
    if constexpr (mode == TransferFunctionMode::Clamped)
    {
      if (c < 0.0031308f)
      {
        return std::max<T>(12.92f * c, 0);
      }

      return ClampTo<T>(1.055f * std::pow(c, 1.0f / 2.4f) - 0.055f, 0, 1);
    }
    else
    {
      auto sign = std::signbit(c) ? -1.0f : 1.0f;
      c = std::abs(c);

      if (c < 0.0031308f)
      {
        return 12.92f * c * sign;
      }

      return (1.055f * std::pow(c, 1.0f / 2.4f) - 0.055f) * sign;
    }
  }

  template <typename T, TransferFunctionMode mode>
  T SRGBTransferFunction<T, mode>::ToLinear(T c) noexcept
  {
    if constexpr (mode == TransferFunctionMode::Clamped)
    {
      if (c <= 0.04045f)
      {
        return std::max<float>(c / 12.92f, 0);
      }

      return ClampTo<float>(std::pow((c + 0.055f) / 1.055f, 2.4f), 0, 1);
    }
    else
    {
      auto sign = std::signbit(c) ? -1.0f : 1.0f;
      c = std::abs(c);

      if (c <= 0.04045f)
      {
        return c / 12.92f * sign;
      }

      return std::pow((c + 0.055f) / 1.055f, 2.4f) * sign;
    }
  }

#pragma endregion
}