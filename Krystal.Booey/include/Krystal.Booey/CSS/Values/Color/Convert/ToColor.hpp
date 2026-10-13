#pragma once

#include "Krystal.Booey/CSS/Values/Color/ColorDescriptors.hpp"
#include "Krystal.Booey/Graphics/Color/Color.hpp"
#include "Krystal.Booey/Graphics/Color/ColorConversion.hpp"
#include "Krystal.Booey/Graphics/Color/ColorTypes.hpp"
#include "Krystal.Core/Types/Maybe.hpp"

namespace krys::boo::css
{
  // This file implements support for converting "typed colors" (e.g. `SRGBA<float>`)
  // into the type erased `Color`, allowing for compaction and optional flags.

  KRYS_NODISCARD constexpr bool OutsideSRGBGamut(HSLA<float> hsla) noexcept
  {
    auto unresolved = hsla.Unresolved();
    return unresolved.Saturation > 100.0 || unresolved.Lightness < 0.0 || unresolved.Lightness > 100.0;
  }

  KRYS_NODISCARD constexpr bool OutsideSRGBGamut(HWBA<float> hwba) noexcept
  {
    auto unresolved = hwba.Unresolved();
    return unresolved.Whiteness < 0.0 || unresolved.Whiteness > 100.0 || unresolved.Blackness < 0.0
           || unresolved.Blackness > 100.0;
  }

  KRYS_NODISCARD constexpr bool OutsideSRGBGamut(SRGBA<float>) noexcept
  {
    return false;
  }

  template <typename Descriptor, ColorFunctionForm Form>
  struct ColorConverter;

  template <typename Descriptor>
  struct ColorConverter<Descriptor, ColorFunctionForm::Absolute>
  {
    KRYS_NODISCARD static boo::Color Convert(GetColorType<Descriptor> color, uint32 nestingLevel) noexcept
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

        // The explicit conversion to SRGBA<uint8> is an intentional performance optimization that allows
        // storing the color with no extra allocation for an extended color object. This is permissible in
        // some case due to the historical requirement that some syntaxes serialize using the legacy color
        // syntax (rgb()/rgba()) and historically have used the 8-bit rgba internal representation in engines.
        return {ConvertColor<SRGBA<uint8>>(color), Descriptor::FlagsForAbsolute};
      }
      else
      {
        return {color, Descriptor::FlagsForAbsolute};
      }
    }
  };

  template <typename Descriptor>
  struct ColorConverter<Descriptor, ColorFunctionForm::Relative>
  {
    KRYS_NODISCARD static boo::Color Convert(GetColorType<Descriptor> color) noexcept
    {
      return {color, Descriptor::FlagsForRelative};
    }
  };

  template <typename Descriptor, ColorFunctionForm Form>
  KRYS_NODISCARD boo::Color ConvertToColor(GetColorType<Descriptor> color, uint32 nestingLevel) noexcept
  {
    static_assert(Form == ColorFunctionForm::Absolute);
    return ColorConverter<Descriptor, Form>::Convert(color, nestingLevel);
  }

  template <typename Descriptor, ColorFunctionForm Form>
  KRYS_NODISCARD boo::Color ConvertToColor(GetColorType<Descriptor> color) noexcept
  {
    static_assert(Form == ColorFunctionForm::Relative);
    return ColorConverter<Descriptor, Form>::Convert(color);
  }

  template <typename Descriptor, ColorFunctionForm Form>
  KRYS_NODISCARD boo::Color ConvertToColor(Maybe<GetColorType<Descriptor>> color,
                                           uint32 nestingLevel) noexcept
  {
    static_assert(Form == ColorFunctionForm::Absolute);

    if (!color)
    {
      return {};
    }
    return ConvertToColor<Descriptor, Form>(*color, nestingLevel);
  }

  template <typename Descriptor, ColorFunctionForm Form>
  KRYS_NODISCARD boo::Color ConvertToColor(Maybe<GetColorType<Descriptor>> color) noexcept
  {
    static_assert(Form == ColorFunctionForm::Relative);

    if (!color)
    {
      return {};
    }
    return ConvertToColor<Descriptor, Form>(*color);
  }
}
