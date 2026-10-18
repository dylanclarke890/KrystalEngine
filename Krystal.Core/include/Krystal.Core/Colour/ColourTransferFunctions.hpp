#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Maths/Base.hpp"
#include "Krystal.Core/Numeric.hpp"
#include <algorithm>
#include <cmath>

// Transfer functions for colors that can be Gamma encoded.
namespace krys
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
    constexpr static T Gamma = static_cast<T>(1.8);

    KRYS_NODISCARD static T ToGammaEncoded(T) noexcept;
    KRYS_NODISCARD static T ToLinear(T) noexcept;
  };

  template <typename T, TransferFunctionMode mode>
  struct Rec2020TransferFunction
  {
    constexpr static T Alpha = static_cast<T>(1.09929682680944);
    constexpr static T Beta = static_cast<T>(0.018053968510807);
    constexpr static T Gamma = static_cast<T>(0.45);

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
    else
    {
      return result;
    }
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
    else
    {
      return result;
    }
  }

#pragma endregion

#pragma region ProPhotoRGBTransferFunction

  template <typename T, TransferFunctionMode mode>
  T ProPhotoRGBTransferFunction<T, mode>::ToGammaEncoded(T c) noexcept
  {
    if constexpr (mode == TransferFunctionMode::Clamped)
    {
      if (c < 1.0f / 512.0f)
      {
        return 16.0f * c;
      }

      return ClampTo<T>(std::pow(c, 1.0f / Gamma), 0, 1);
    }
    else
    {
      if (std::abs(c) < 1.0f / 512.0f)
      {
        return 16.0f * c;
      }

      float sign = std::signbit(c) ? -1.0f : 1.0f;
      return std::pow(std::abs(c), 1.0f / Gamma) * sign;
    }
  }

  template <typename T, TransferFunctionMode mode>
  T ProPhotoRGBTransferFunction<T, mode>::ToLinear(T c) noexcept
  {
    if constexpr (mode == TransferFunctionMode::Clamped)
    {
      if (c <= 16.0f / 512.0f)
      {
        return c / 16.0f;
      }

      return ClampTo<T>(std::pow(c, Gamma), 0, 1);
    }
    else
    {
      if (std::abs(c) <= 16.0f / 512.0f)
      {
        return c / 16.0f;
      }

      float sign = std::signbit(c) ? -1.0f : 1.0f;
      return std::pow(std::abs(c), Gamma) * sign;
    }
  }

#pragma endregion

#pragma region Rec2020TransferFunction

  template <typename T, TransferFunctionMode mode>
  T Rec2020TransferFunction<T, mode>::ToGammaEncoded(T c) noexcept
  {
    if constexpr (mode == TransferFunctionMode::Clamped)
    {
      if (c <= Beta)
      {
        return 4.5f * c;
      }

      return ClampTo<T>(Alpha * std::pow(c, Gamma) - (Alpha - 1.0f), 0, 1);
    }
    else
    {
      if (std::abs(c) <= Beta)
      {
        return 4.5f * c;
      }

      float sign = std::signbit(c) ? -1.0f : 1.0f;
      return (Alpha * std::pow(std::abs(c), Gamma) - (Alpha - 1.0f)) * sign;
    }
  }

  template <typename T, TransferFunctionMode mode>
  T Rec2020TransferFunction<T, mode>::ToLinear(T c) noexcept
  {
    if constexpr (mode == TransferFunctionMode::Clamped)
    {
      if (c < Beta * 4.5f)
      {
        return c / 4.5f;
      }

      return ClampTo<T>(std::pow((c + Alpha - 1.0f) / Alpha, 1.0f / Gamma), 0, 1);
    }
    else
    {
      if (std::abs(c) < Beta * 4.5f)
      {
        return c / 4.5f;
      }

      float sign = std::signbit(c) ? -1.0f : 1.0f;
      return std::pow((std::abs(c) + Alpha - 1.0f) / Alpha, 1.0f / Gamma) * sign;
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