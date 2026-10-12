#include "Krystal.Booey/Graphics/Color/ColorSerialisation.hpp"
#include "Krystal.Booey/Core/MathExtras.hpp"
#include "Krystal.Booey/Core/NumberFormatting.hpp"
#include "Krystal.Booey/Graphics/Color/Color.hpp"
#include "Krystal.Booey/Graphics/Color/ColorNormalization.hpp"
#include "Krystal.Core/Assert.hpp"
#include "Krystal.Core/Text/ASCII.hpp"
#include "Krystal.Core/Types/Array.hpp"
#include <charconv>
#include <cmath>

namespace krys::boo
{
  namespace
  {
    void AppendNumericComponent(utf8_string &builder, float value) noexcept
    {
      if (std::isnan(value))
      {
        builder.append(u8"none");
        return;
      }

      if (std::isfinite(value))
      {
        FormatCSSNumber(builder, value);
        return;
      }

      builder.append(u8"calc(");
      FormatCSSNumber(builder, value);
      builder.push_back(u8')');
    }

    void AppendInteger(utf8_string &builder, uint32 value) noexcept
    {
      Array<char, 16uz> buffer;
      auto result = std::to_chars(buffer.data(), buffer.data() + buffer.size(), value);
      krys_debug_assert(result.ec == std::errc {});
      builder.append(buffer.data(), result.ptr);
    }

    enum class HexCase : uint8
    {
      Uppercase,
      Lowercase
    };

    void AppendHexByte(utf8_string &builder, uint8 value, HexCase hexCase = HexCase::Uppercase) noexcept
    {
      if (hexCase == HexCase::Lowercase)
      {
        builder.push_back(static_cast<char8>(text::UpperNibbleToLowercaseASCIIHexDigit(value)));
        builder.push_back(static_cast<char8>(text::LowerNibbleToLowercaseASCIIHexDigit(value)));
      }
      else
      {
        builder.push_back(static_cast<char8>(text::UpperNibbleToASCIIHexDigit(value)));
        builder.push_back(static_cast<char8>(text::LowerNibbleToASCIIHexDigit(value)));
      }
    }

    /// @brief Appends " / <alpha>" unless the alpha is (essentially) opaque, then the closing parenthesis.
    void AppendAlphaAndClose(utf8_string &builder, float alpha) noexcept
    {
      if (!AreEssentiallyEqual(alpha, 1.0f))
      {
        builder.append(u8" / ");
        AppendNumericComponent(builder, alpha);
      }

      builder.push_back(u8')');
    }

    template <typename ColorType>
    KRYS_NODISCARD utf8_string SerialisationUsingColorFunction(const ColorType &color) noexcept
    {
      static_assert(SameType<typename ColorType::ComponentType, float>);

      auto [c1, c2, c3, alpha] = color.Unresolved();

      utf8_string builder = u8"color(";
      builder.append(Serialisation(ColorSpaceFor<ColorType>));
      builder.push_back(u8' ');
      AppendNumericComponent(builder, c1);
      builder.push_back(u8' ');
      AppendNumericComponent(builder, c2);
      builder.push_back(u8' ');
      AppendNumericComponent(builder, c3);
      AppendAlphaAndClose(builder, alpha);
      return builder;
    }

    KRYS_NODISCARD utf8_string SerialisationUsingColorFunction(const SRGBA<uint8> &color) noexcept
    {
      return SerialisationUsingColorFunction(ConvertColor<SRGBA<float>>(color));
    }

    template <typename ColorType>
    KRYS_NODISCARD utf8_string SerialisationOfLabLikeColorsForCSS(const ColorType &color) noexcept
    {
      static_assert(SameType<typename ColorType::ComponentType, float>);

      // https://www.w3.org/TR/css-color-4/#serializing-lab-lch
      auto [c1, c2, c3, alpha] = color.Unresolved();

      utf8_string builder {Serialisation(ColorSpaceFor<ColorType>)};
      builder.push_back(u8'(');
      AppendNumericComponent(builder, c1);
      builder.push_back(u8' ');
      AppendNumericComponent(builder, c2);
      builder.push_back(u8' ');
      AppendNumericComponent(builder, c3);
      AppendAlphaAndClose(builder, alpha);
      return builder;
    }

    template <typename ColorType>
    KRYS_NODISCARD utf8_string SerialisationOfLCHLikeColorsForCSS(const ColorType &color) noexcept
    {
      static_assert(SameType<typename ColorType::ComponentType, float>);

      // https://www.w3.org/TR/css-color-4/#serializing-lab-lch

      // NOTE: It is unclear whether normalizing the hue component for serialization is required. The question
      // has been raised with the editors as https://github.com/w3c/csswg-drafts/issues/7782.

      auto [c1, c2, c3, alpha] = color.Unresolved();

      utf8_string builder {Serialisation(ColorSpaceFor<ColorType>)};
      builder.push_back(u8'(');
      AppendNumericComponent(builder, c1);
      builder.push_back(u8' ');
      AppendNumericComponent(builder, c2);
      builder.push_back(u8' ');
      AppendNumericComponent(builder, NormalizeHue(c3));
      AppendAlphaAndClose(builder, alpha);
      return builder;
    }

    // MARK: SRGBA<uint8> overloads

    KRYS_NODISCARD char8 DecimalDigit(uint32 number) noexcept
    {
      krys_debug_assert(number < 10);
      return static_cast<char8>(u8'0' + number);
    }

    KRYS_NODISCARD Array<char8, 4> FractionDigitsForFractionalAlphaValue(uint8 alpha) noexcept
    {
      krys_debug_assert(alpha > 0);
      krys_debug_assert(alpha < 0xFF);

      if (((alpha * 100 + 0x7F) / 0xFF * 0xFF + 50) / 100 != alpha)
      {
        return {{DecimalDigit(alpha * 10 / 0xFF % 10), DecimalDigit(alpha * 100 / 0xFF % 10),
                 DecimalDigit((alpha * 1'000 + 0x7F) / 0xFF % 10), u8'\0'}};
      }

      if (uint32 thirdDigit = (alpha * 100 + 0x7F) / 0xFF % 10)
      {
        return {{DecimalDigit(alpha * 10 / 0xFF), DecimalDigit(thirdDigit), u8'\0', u8'\0'}};
      }

      return {{DecimalDigit((alpha * 10 + 0x7F) / 0xFF), u8'\0', u8'\0', u8'\0'}};
    }

    KRYS_NODISCARD utf8_string SerialisationForCSS(SRGBA<uint8> color,
                                                   bool useColorFunctionSerialization) noexcept
    {
      if (useColorFunctionSerialization)
      {
        return SerialisationUsingColorFunction(color);
      }

      auto [red, green, blue, alpha] = color.Resolved();

      utf8_string builder = alpha == 0xFF ? u8"rgb(" : u8"rgba(";
      AppendInteger(builder, red);
      builder.append(u8", ");
      AppendInteger(builder, green);
      builder.append(u8", ");
      AppendInteger(builder, blue);

      switch (alpha)
      {
        case 0:
        {
          builder.append(u8", 0)");
          break;
        }
        case 0xFF:
        {
          builder.push_back(u8')');
          break;
        }
        default:
        {
          builder.append(u8", 0.");
          builder.append(FractionDigitsForFractionalAlphaValue(alpha).data());
          builder.push_back(u8')');
          break;
        }
      }

      return builder;
    }

    KRYS_NODISCARD utf8_string SerialisationForHTML(SRGBA<uint8> color,
                                                    bool useColorFunctionSerialization) noexcept
    {
      if (useColorFunctionSerialization)
      {
        return SerialisationUsingColorFunction(color);
      }

      auto [red, green, blue, alpha] = color.Resolved();
      if (alpha == 0xFF)
      {
        utf8_string builder = u8"#";
        AppendHexByte(builder, red, HexCase::Lowercase);
        AppendHexByte(builder, green, HexCase::Lowercase);
        AppendHexByte(builder, blue, HexCase::Lowercase);
        return builder;
      }

      return boo::SerialisationForCSS(Color {color});
    }

    KRYS_NODISCARD utf8_string SerialisationForRenderTreeAsText(SRGBA<uint8> color,
                                                                bool useColorFunctionSerialization) noexcept
    {
      if (useColorFunctionSerialization)
      {
        return SerialisationUsingColorFunction(color);
      }

      auto [red, green, blue, alpha] = color.Resolved();

      utf8_string builder = u8"#";
      AppendHexByte(builder, red);
      AppendHexByte(builder, green);
      AppendHexByte(builder, blue);
      if (alpha < 0xFF)
      {
        AppendHexByte(builder, alpha);
      }

      return builder;
    }

    // MARK: SRGBA<float> overloads

    KRYS_NODISCARD utf8_string SerialisationForCSS(const SRGBA<float> &color,
                                                   bool useColorFunctionSerialization) noexcept
    {
      if (useColorFunctionSerialization)
      {
        return SerialisationUsingColorFunction(color);
      }

      return SerialisationForCSS(ConvertColor<SRGBA<uint8>>(color), false);
    }

    KRYS_NODISCARD utf8_string SerialisationForHTML(const SRGBA<float> &color,
                                                    bool useColorFunctionSerialization) noexcept
    {
      if (useColorFunctionSerialization)
      {
        return SerialisationUsingColorFunction(color);
      }

      return SerialisationForHTML(ConvertColor<SRGBA<uint8>>(color), false);
    }

    KRYS_NODISCARD utf8_string SerialisationForRenderTreeAsText(const SRGBA<float> &color,
                                                                bool useColorFunctionSerialization) noexcept
    {
      return SerialisationForCSS(color, useColorFunctionSerialization);
    }

    // MARK: Color types always serialised with color()

    // A98RGB, DisplayP3, ExtendedA98RGB, ExtendedDisplayP3, ExtendedLinearDisplayP3, ExtendedLinearSRGBA,
    // ExtendedProPhotoRGB, ExtendedRec2020, ExtendedSRGBA, LinearDisplayP3, LinearSRGBA, ProPhotoRGB,
    // Rec2020, XYZA<D50> and XYZA<D65> serialise with color() in every context.
    template <typename ColorType>
    concept SerialisedUsingColorFunction =
      OneOf<ColorType, A98RGB<float>, DisplayP3<float>, ExtendedA98RGB<float>, ExtendedDisplayP3<float>,
            ExtendedLinearDisplayP3<float>, ExtendedLinearSRGBA<float>, ExtendedProPhotoRGB<float>,
            ExtendedRec2020<float>, ExtendedSRGBA<float>, LinearDisplayP3<float>, LinearSRGBA<float>,
            ProPhotoRGB<float>, Rec2020<float>, XYZA<float, WhitePoint::D50>, XYZA<float, WhitePoint::D65>>;

    template <SerialisedUsingColorFunction ColorType>
    KRYS_NODISCARD utf8_string SerialisationForCSS(const ColorType &color, bool) noexcept
    {
      return SerialisationUsingColorFunction(color);
    }

    template <SerialisedUsingColorFunction ColorType>
    KRYS_NODISCARD utf8_string SerialisationForHTML(const ColorType &color, bool) noexcept
    {
      return SerialisationUsingColorFunction(color);
    }

    template <SerialisedUsingColorFunction ColorType>
    KRYS_NODISCARD utf8_string SerialisationForRenderTreeAsText(const ColorType &color, bool) noexcept
    {
      return SerialisationUsingColorFunction(color);
    }

    // MARK: HSLA<float> and HWBA<float> overloads

    template <typename ColorType>
    concept SerialisedAsLegacyRGB = OneOf<ColorType, HSLA<float>, HWBA<float>>;

    template <SerialisedAsLegacyRGB ColorType>
    KRYS_NODISCARD utf8_string SerialisationForCSS(const ColorType &color,
                                                   bool useColorFunctionSerialization) noexcept
    {
      // FIXME: The spec is not completely clear on whether missing components should be
      // carried forward here, but it seems like people are leaning toward thinking they
      // should be. See https://github.com/w3c/csswg-drafts/issues/10254.

      if (useColorFunctionSerialization)
      {
        return SerialisationUsingColorFunction(
          ConvertColorCarryingForwardMissing<ExtendedSRGBA<float>>(color));
      }

      return SerialisationForCSS(ConvertColor<SRGBA<uint8>>(color), false);
    }

    template <SerialisedAsLegacyRGB ColorType>
    KRYS_NODISCARD utf8_string SerialisationForHTML(const ColorType &color,
                                                    bool useColorFunctionSerialization) noexcept
    {
      return SerialisationForHTML(ConvertColor<SRGBA<uint8>>(color), useColorFunctionSerialization);
    }

    template <SerialisedAsLegacyRGB ColorType>
    KRYS_NODISCARD utf8_string SerialisationForRenderTreeAsText(const ColorType &color,
                                                                bool useColorFunctionSerialization) noexcept
    {
      return SerialisationForRenderTreeAsText(ConvertColor<SRGBA<uint8>>(color),
                                              useColorFunctionSerialization);
    }

    // MARK: Lab<float>, LCHA<float>, OKLab<float> and OKLCHA<float> overloads

    template <typename ColorType>
    concept SerialisedAsLabLike = OneOf<ColorType, Lab<float>, OKLab<float>>;

    template <typename ColorType>
    concept SerialisedAsLCHLike = OneOf<ColorType, LCHA<float>, OKLCHA<float>>;

    template <typename ColorType>
    concept SerialisedWithOwnFunction = SerialisedAsLabLike<ColorType> || SerialisedAsLCHLike<ColorType>;

    template <SerialisedWithOwnFunction ColorType>
    KRYS_NODISCARD utf8_string SerialisationForCSS(const ColorType &color, bool) noexcept
    {
      if constexpr (SerialisedAsLabLike<ColorType>)
      {
        return SerialisationOfLabLikeColorsForCSS(color);
      }
      else
      {
        return SerialisationOfLCHLikeColorsForCSS(color);
      }
    }

    template <SerialisedWithOwnFunction ColorType>
    KRYS_NODISCARD utf8_string SerialisationForHTML(const ColorType &color,
                                                    bool useColorFunctionSerialization) noexcept
    {
      return SerialisationForCSS(color, useColorFunctionSerialization);
    }

    template <SerialisedWithOwnFunction ColorType>
    KRYS_NODISCARD utf8_string SerialisationForRenderTreeAsText(const ColorType &color,
                                                                bool useColorFunctionSerialization) noexcept
    {
      return SerialisationForCSS(color, useColorFunctionSerialization);
    }
  }

  utf8_string SerialisationForCSS(const Color &color) noexcept
  {
    return color.CallOnUnderlyingType(
      [&](const auto &underlyingColor)
      { return SerialisationForCSS(underlyingColor, color.UsesColorFunctionSerialization()); });
  }

  utf8_string SerialisationForHTML(const Color &color) noexcept
  {
    return color.CallOnUnderlyingType(
      [&](const auto &underlyingColor)
      { return SerialisationForHTML(underlyingColor, color.UsesColorFunctionSerialization()); });
  }

  utf8_string SerialisationForRenderTreeAsText(const Color &color) noexcept
  {
    return color.CallOnUnderlyingType(
      [&](const auto &underlyingColor)
      { return SerialisationForRenderTreeAsText(underlyingColor, color.UsesColorFunctionSerialization()); });
  }

  utf8_stringview Serialisation(ColorSpace colorSpace) noexcept
  {
    switch (colorSpace)
    {
      case ColorSpace::A98RGB:
      case ColorSpace::ExtendedA98RGB:
      {
        return u8"a98-rgb";
      }
      case ColorSpace::DisplayP3:
      case ColorSpace::ExtendedDisplayP3:
      {
        return u8"display-p3";
      }
      case ColorSpace::HSL:
      {
        return u8"hsl";
      }
      case ColorSpace::HWB:
      {
        return u8"hwb";
      }
      case ColorSpace::LCH:
      {
        return u8"lch";
      }
      case ColorSpace::Lab:
      {
        return u8"lab";
      }
      case ColorSpace::LinearDisplayP3:
      case ColorSpace::ExtendedLinearDisplayP3:
      {
        return u8"display-p3-linear";
      }
      case ColorSpace::LinearSRGB:
      case ColorSpace::ExtendedLinearSRGB:
      {
        return u8"srgb-linear";
      }
      case ColorSpace::OKLCH:
      {
        return u8"oklch";
      }
      case ColorSpace::OKLab:
      {
        return u8"oklab";
      }
      case ColorSpace::ProPhotoRGB:
      case ColorSpace::ExtendedProPhotoRGB:
      {
        return u8"prophoto-rgb";
      }
      case ColorSpace::Rec2020:
      case ColorSpace::ExtendedRec2020:
      {
        return u8"rec2020";
      }
      case ColorSpace::SRGB:
      case ColorSpace::ExtendedSRGB:
      {
        return u8"srgb";
      }
      case ColorSpace::XYZ_D50:
      {
        return u8"xyz-d50";
      }
      case ColorSpace::XYZ_D65:
      {
        return u8"xyz-d65";
      }
    }

    krys_unreachable();
  }
}
