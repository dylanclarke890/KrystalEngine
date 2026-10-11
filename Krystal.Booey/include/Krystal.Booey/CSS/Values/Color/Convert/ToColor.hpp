#pragma once

#include "Krystal.Booey/CSS/Values/Color/ColorDescriptors.hpp"
#include "Krystal.Booey/Graphics/Color/Color.hpp"
#include "Krystal.Booey/Graphics/Color/ColorTypes.hpp"

namespace krys::boo::css
{
  // This file implements support for converting "typed colors" (e.g. `SRGBA<float>`)
  // into the type erased `Color`, allowing for compaction and optional flags.

  KRYS_NODISCARD constexpr bool OutsideSRGBGamut(HSLA<float> hsla) noexcept
  {
    auto unresolved = hsla.Unresolved();
    return unresolved.saturation > 100.0 || unresolved.lightness < 0.0 || unresolved.lightness > 100.0;
  }

  KRYS_NODISCARD constexpr bool OutsideSRGBGamut(HWBA<float> hwba) noexcept
  {
    auto unresolved = hwba.Unresolved();
    return unresolved.whiteness < 0.0 || unresolved.whiteness > 100.0 || unresolved.blackness < 0.0
           || unresolved.blackness > 100.0;
  }

  KRYS_NODISCARD constexpr bool OutsideSRGBGamut(SRGBA<float>) noexcept
  {
    return false;
  }

  template <typename Descriptor, ColorFunctionForm Form>
  struct ConvertToColor;

  template <typename Descriptor>
  struct ConvertToColor<Descriptor, ColorFunctionForm::Absolute>
  {
    KRYS_NODISCARD static Color ConvertToColor(GetColorType<Descriptor> color, size_t nestingLevel) noexcept
    {
      if constexpr (Descriptor::AllowConversionTo8BitSRGB)
      {
        if constexpr (Descriptor::Syntax == ColorFunctionSyntax::Modern)
        {
          if (color.Unresolved().AnyComponentIsNone())
          {
            // If any component uses "none", we store the value as is to allow for storage of the special
            // value as NaN.
            return {color, Descriptor::FlagsForAbsolute};
          }
        }

        if (OutsideSRGBGamut(color))
        {
          // If any component is outside the reference range, we store the value as is to allow for non-SRGB
          // gamut values.
          return {color, Descriptor::FlagsForAbsolute};
        }

        if (nestingLevel > 1)
        {
          // If the color is being consumed as part of a composition (relative color, color-mix, light-dark,
          // etc.), we store the value as is to allow for maximum precision.
          return {color, Descriptor::FlagsForAbsolute};
        }

        // The explicit conversion to SRGBA<uint8_t> is an intentional performance optimization that allows
        // storing the color with no extra allocation for an extended color object. This is permissible in
        // some case due to the historical requirement that some syntaxes serialize using the legacy color
        // syntax (rgb()/rgba()) and historically have used the 8-bit rgba internal representation in engines.
        return {convertColor<SRGBA<uint8_t>>(color), Descriptor::FlagsForAbsolute};
      }
      else
      {
        return {color, Descriptor::FlagsForAbsolute};
      }
    }
  };

  template <typename Descriptor>
  struct ConvertToColor<Descriptor, ColorFunctionForm::Relative>
  {
    KRYS_NODISCARD static Color ConvertToColor(GetColorType<Descriptor> color) noexcept
    {
      return {color, Descriptor::FlagsForRelative};
    }
  };

  template <typename Descriptor, ColorFunctionForm Form>
  KRYS_NODISCARD Color ConvertToColor(GetColorType<Descriptor> color, size_t nestingLevel) noexcept
  {
    static_assert(Form == ColorFunctionForm::Absolute);
    return ConvertToColor<Descriptor, Form>::ConvertToColor(color, nestingLevel);
  }

  template <typename Descriptor, ColorFunctionForm Form>
  KRYS_NODISCARD Color ConvertToColor(GetColorType<Descriptor> color) noexcept
  {
    static_assert(Form == ColorFunctionForm::Relative);
    return ConvertToColor<Descriptor, Form>::ConvertToColor(color);
  }

  template <typename Descriptor, ColorFunctionForm Form>
  KRYS_NODISCARD Color ConvertToColor(std::optional<GetColorType<Descriptor>> color,
                                      size_t nestingLevel) noexcept
  {
    static_assert(Form == ColorFunctionForm::Absolute);
    if (!color)
    {
      return {};
    }

    return ConvertToColor<Descriptor, Form>(*color, nestingLevel);
  }

  template <typename Descriptor, ColorFunctionForm Form>
  KRYS_NODISCARD Color ConvertToColor(std::optional<GetColorType<Descriptor>> color) noexcept
  {
    static_assert(Form == ColorFunctionForm::Relative);
    if (!color)
    {
      return {};
    }

    return ConvertToColor<Descriptor, Form>(*color);
  }
}