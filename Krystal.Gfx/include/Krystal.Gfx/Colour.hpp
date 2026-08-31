#pragma once

#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Lib/Types/Numeric.hpp"
#include "Krystal.Maths/Vector.hpp"

namespace Krys::Gfx
{
  template <typename ColourType, ColourType AlphaDefault, bool PremultipliedAlpha>
  class Colour
  {
    using Type = Colour<ColourType, AlphaDefault, PremultipliedAlpha>;

  public:
    ColourType red, green, blue, alpha;

  public:
    constexpr Colour() noexcept : red(0), green(0), blue(0), alpha(AlphaDefault)
    {
    }

    /// @param rgb Initial red, green and blue value of the colour.
    /// @param alpha Initial alpha value of the colour.
    constexpr Colour(ColourType rgb, ColourType alpha = AlphaDefault) noexcept
        : red(rgb), green(rgb), blue(rgb), alpha(alpha)
    {
    }

    /// @param red Initial red value of the colour.
    /// @param green Initial green value of the colour.
    /// @param blue Initial blue value of the colour.
    /// @param alpha Initial alpha value of the colour.
    constexpr Colour(ColourType red, ColourType green, ColourType blue,
                     ColourType alpha = AlphaDefault) noexcept
        : red(red), green(green), blue(blue), alpha(alpha)
    {
    }

    constexpr Colour(uchar red, uchar green, uchar blue, uchar alpha) noexcept
    requires SameType<ColourType, byte>
        : red(static_cast<ColourType>(red)), green(static_cast<ColourType>(green)),
          blue(static_cast<ColourType>(blue)), alpha(static_cast<ColourType>(alpha))
    {
    }

    /// @brief Returns the sum of this colour and another. This does not saturate the channels.
    /// @param rhs The colour to add this to.
    /// @return The sum of the two colours.
    KRYS_NODISCARD constexpr Type operator+(const Type &rhs) const noexcept
    {
      return Type(red + rhs.red, green + rhs.green, blue + rhs.blue, alpha + rhs.alpha);
    }

    /// @brief Returns the result of subtracting another colour from this colour.
    /// @param rhs The colour to subtract from this colour.
    /// @return The result of the subtraction.
    KRYS_NODISCARD constexpr Type operator-(const Type &rhs) const noexcept
    {
      return Type(red - rhs.red, green - rhs.green, blue - rhs.blue, alpha - rhs.alpha);
    }

    /// @brief Returns the result of multiplying this colour component-wise by a scalar.
    /// @param rhs The scalar value to multiply by.
    /// @return The result of the scale.
    KRYS_NODISCARD constexpr Type operator*(float rhs) const noexcept
    {
      return Type((ColourType)(red * rhs), (ColourType)(green * rhs), (ColourType)(blue * rhs),
                  (ColourType)(alpha * rhs));
    }

    /// @brief Returns the result of dividing this colour component-wise by a scalar.
    /// @param rhs The scalar value to divide by.
    /// @return The result of the scale.
    KRYS_NODISCARD constexpr Type operator/(float rhs) const noexcept
    {
      return Type((ColourType)(red / rhs), (ColourType)(green / rhs), (ColourType)(blue / rhs),
                  (ColourType)(alpha / rhs));
    }

    /// @brief Adds another colour to this in-place. This does not saturate the channels.
    /// @param rhs The colour to add.
    constexpr void operator+=(const Colour rhs) noexcept
    {
      red += rhs.red;
      green += rhs.green;
      blue += rhs.blue;
      alpha += rhs.alpha;
    }

    /// @brief Subtracts another colour from this in-place.
    /// @param rhs The colour to subtract.
    constexpr void operator-=(const Colour rhs) noexcept
    {
      red -= rhs.red;
      green -= rhs.green;
      blue -= rhs.blue;
      alpha -= rhs.alpha;
    }

    /// @brief Scales this colour component-wise in-place.
    /// @param rhs The value to scale this colour's components by.
    constexpr void operator*=(float rhs) noexcept
    {
      red = (ColourType)(red * rhs);
      green = (ColourType)(green * rhs);
      blue = (ColourType)(blue * rhs);
      alpha = (ColourType)(alpha * rhs);
    }

    /// @brief Scales this colour component-wise in-place by the inverse of a value.
    /// @param rhs The value to divide this colour's components by.
    constexpr void operator/=(float rhs) noexcept
    {
      *this *= (1.0f / rhs);
    }

    /// @brief Equality operator.
    /// @param rhs The colour to compare this against.
    /// @return True if the two colours are equal, false otherwise.
    KRYS_NODISCARD constexpr bool operator==(Colour rhs) const noexcept
    {
      return red == rhs.red && green == rhs.green && blue == rhs.blue && alpha == rhs.alpha;
    }

    /// @brief Inequality operator.
    /// @param rhs The colour to compare this against.
    /// @return True if the two colours are not equal, false otherwise.
    KRYS_NODISCARD constexpr bool operator!=(Colour rhs) const noexcept
    {
      return !(*this == rhs);
    }

    /// @brief Auto-cast operator.
    /// @return A pointer to the first value.
    constexpr operator const ColourType *() const noexcept
    {
      return &red;
    }

    /// @brief Constant auto-cast operator.
    /// @return A constant pointer to the first value.
    constexpr operator ColourType *() noexcept
    {
      return &red;
    }

    /// @brief Convert color to premultiplied alpha.
    KRYS_NODISCARD constexpr Colour<ColourType, AlphaDefault, true> ToPremultiplied() const noexcept
    requires(!PremultipliedAlpha && SameType<ColourType, uchar>)
    {
      return {
        ColourType((red * alpha) / 255),
        ColourType((green * alpha) / 255),
        ColourType((blue * alpha) / 255),
        alpha,
      };
    }

    /// @brief Convert color to premultiplied alpha, after multiplying alpha by opacity.
    KRYS_NODISCARD constexpr Colour<ColourType, AlphaDefault, true>
      ToPremultiplied(float opacity) const noexcept
    requires(!PremultipliedAlpha && SameType<ColourType, uchar>)
    {
      const float new_alpha = alpha * opacity;
      return {
        ColourType(red * (new_alpha / 255.f)),
        ColourType(green * (new_alpha / 255.f)),
        ColourType(blue * (new_alpha / 255.f)),
        ColourType(new_alpha),
      };
    }

    /// @brief Convert color to non-premultiplied alpha.
    KRYS_NODISCARD constexpr Colour<ColourType, AlphaDefault, false> ToNonPremultiplied() const noexcept
    requires(PremultipliedAlpha && SameType<ColourType, uchar>)
    {
      return {
        ColourType(alpha > 0 ? (red * 255) / alpha : 0),
        ColourType(alpha > 0 ? (green * 255) / alpha : 0),
        ColourType(alpha > 0 ? (blue * 255) / alpha : 0),
        ColourType(alpha),
      };
    }

    KRYS_NODISCARD constexpr Maths::Vec3 ToVec3() const noexcept
    {
      if constexpr (SameType<ColourType, float>)
      {
        return Maths::Vec3 {red, green, blue};
      }
      else if constexpr (SameType<ColourType, uchar>)
      {
        return Maths::Vec3 {(float)red / 255.f, (float)green / 255.f, (float)blue / 255.f};
      }
    }

    KRYS_NODISCARD constexpr Maths::Vec4 ToVec4() const noexcept
    {
      if constexpr (SameType<ColourType, float>)
      {
        return Maths::Vec4 {red, green, blue, alpha};
      }
      else if constexpr (SameType<ColourType, uchar>)
      {
        return Maths::Vec4 {(float)red / 255.f, (float)green / 255.f, (float)blue / 255.f,
                            (float)alpha / 255.f};
      }
    }
  };

  using Colourf = Colour<float, 1.f, false>;
  using Colourb = Colour<uchar, uchar {255}, false>;
  using ColourbPremultiplied = Colour<uchar, uchar {255}, true>;

  namespace Colours
  {
    constexpr static ColourbPremultiplied White {255, 255, 255, 255};
    constexpr static ColourbPremultiplied Black {0, 0, 0, 255};
    constexpr static ColourbPremultiplied Transparent {0, 0, 0, 0};
    constexpr static ColourbPremultiplied Red {255, 0, 0, 255};
    constexpr static ColourbPremultiplied Green {0, 128, 0, 255};
    constexpr static ColourbPremultiplied Blue {0, 0, 255, 255};
    constexpr static ColourbPremultiplied Gray25 {64, 64, 64, 255};
    constexpr static ColourbPremultiplied Gray50 {128, 128, 128, 255};
    constexpr static ColourbPremultiplied Gray75 {192, 192, 192, 255};
    constexpr static ColourbPremultiplied Coral {255, 127, 79, 255};
    constexpr static ColourbPremultiplied Yellow {255, 255, 0, 255};
    constexpr static ColourbPremultiplied Orange {255, 165, 0, 255};
    constexpr static ColourbPremultiplied Purple {128, 0, 128, 255};
    constexpr static ColourbPremultiplied Pink {255, 191, 204, 255};
    constexpr static ColourbPremultiplied Brown {165, 42, 42, 255};
    constexpr static ColourbPremultiplied Cyan {0, 255, 255, 255};
    constexpr static ColourbPremultiplied Magenta {255, 0, 255, 255};
    constexpr static ColourbPremultiplied Lime {0, 255, 0, 255};
    constexpr static ColourbPremultiplied Teal {0, 128, 128, 255};
    constexpr static ColourbPremultiplied Indigo {74, 0, 130, 255};
    constexpr static ColourbPremultiplied Maroon {128, 0, 0, 255};
    constexpr static ColourbPremultiplied Olive {128, 128, 0, 255};
    constexpr static ColourbPremultiplied Navy {0, 0, 128, 255};
    constexpr static ColourbPremultiplied Beige {245, 245, 220, 255};
    constexpr static ColourbPremultiplied Ivory {255, 255, 240, 255};
  }
}
