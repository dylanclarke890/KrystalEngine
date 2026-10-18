#include "Krystal.Booey/Graphics/Color/ColorBlending.hpp"
#include "Krystal.Booey/Graphics/Animation/AnimationUtilities.hpp"
#include "Krystal.Booey/Graphics/Color/Color.hpp"
#include "Krystal.Booey/Graphics/Color/ColorInterpolation.hpp"
#include "Krystal.Core/Assert.hpp"

namespace krys::boo
{
  namespace
  {
    KRYS_NODISCARD bool RequiresLegacyInterpolationRules(const Color &color) noexcept
    {
      return color.CallOnUnderlyingType(
        [&]<typename ColorType>(const ColorType &)
        {
          if constexpr (SameType<ColorType, SRGBA<uint8>>)
          {
            return true;
          }
          else if constexpr (SameType<ColorType, SRGBA<float>>)
          {
            return true;
          }
          else if constexpr (SameType<ColorType, HSLA<float>>)
          {
            return true;
          }
          else if constexpr (SameType<ColorType, HWBA<float>>)
          {
            return true;
          }
          else if constexpr (SameType<ColorType, ExtendedSRGBA<float>>)
          {
            return !color.UsesColorFunctionSerialisation();
          }
          else
          {
            return false;
          }
        });
    }
  }

  Color BlendSourceOver(const Color &backdrop, const Color &source) noexcept
  {
    if (!backdrop.IsVisible() || source.IsOpaque())
    {
      return source;
    }

    if (!source.IsVisible())
    {
      return backdrop;
    }

    auto [backdropR, backdropG, backdropB, backdropA] = backdrop.ToColorTypeLossy<SRGBA<uint8>>().Resolved();
    auto [sourceR, sourceG, sourceB, sourceA] = source.ToColorTypeLossy<SRGBA<uint8>>().Resolved();

    if (!backdropA || sourceA == 255)
    {
      return source;
    }

    if (!sourceA)
    {
      return backdrop;
    }

    int32 d = 0xFF * (backdropA + sourceA) - backdropA * sourceA;
    int32 a = d / 0xFF;
    int32 r = (backdropR * backdropA * (0xFF - sourceA) + 0xFF * sourceA * sourceR) / d;
    int32 g = (backdropG * backdropA * (0xFF - sourceA) + 0xFF * sourceA * sourceG) / d;
    int32 b = (backdropB * backdropA * (0xFF - sourceA) + 0xFF * sourceA * sourceB) / d;

    return CreateFromComponentsClamping<SRGBA<uint8>>(r, g, b, a);
  }

  Color BlendWithWhite(const Color &color) noexcept
  {
    constexpr int32 startAlpha = 153; // 60%
    constexpr int32 endAlpha = 204;   // 80%;
    constexpr int32 alphaIncrement = 17;

    auto BlendComponent = [](int32 c, int32 a) -> int32
    {
      float alpha = a / 255.0f;
      int32 whiteBlend = 255 - a;
      c -= whiteBlend;
      return static_cast<int32>(c / alpha);
    };

    // If the color contains alpha already, we leave it alone.
    if (!color.IsOpaque())
    {
      return color;
    }

    auto [existingR, existingG, existingB, existingAlpha] = color.ToColorTypeLossy<SRGBA<uint8>>().Resolved();

    SRGBA<uint8> result;
    for (int32 alpha = startAlpha; alpha <= endAlpha; alpha += alphaIncrement)
    {
      // We have a solid color.  Convert to an equivalent color that looks the same when blended with white
      // at the current alpha.  Try using less transparency if the numbers end up being negative.
      int32 r = BlendComponent(existingR, alpha);
      int32 g = BlendComponent(existingG, alpha);
      int32 b = BlendComponent(existingB, alpha);

      result = CreateFromComponentsClamping<SRGBA<uint8>>(r, g, b, alpha);

      if (r >= 0 && g >= 0 && b >= 0)
      {
        break;
      }
    }

    // FIXME: Why is preserving the semantic bit desired and/or correct here?
    if (color.IsSemantic())
    {
      return {result, ColorFlags::Semantic};
    }

    return result;
  }

  Color Blend(const Color &from, const Color &to, const BlendingContext &context) noexcept
  {
    // We need to preserve the state of the valid flag at the end of the animation
    if (context.Progress == 1 && !to.IsValid())
    {
      return {};
    }

    if (RequiresLegacyInterpolationRules(from) && RequiresLegacyInterpolationRules(to))
    {
      using InterpolationColourSpace = ColourInterpolationMethod::SRGB;

      auto fromComponents =
        from.ToColorTypeLossyCarryingForwardMissing<typename InterpolationColourSpace::ColourType>();
      auto toComponents =
        to.ToColorTypeLossyCarryingForwardMissing<typename InterpolationColourSpace::ColourType>();

      switch (context.CompositeOperation)
      {
        case CompositeOperation::Replace:
        {
          auto interpolatedColor = InterpolateColourComponents<AlphaPremultiplication::Premultiplied>(
            InterpolationColourSpace {}, fromComponents, 1.0 - context.Progress, toComponents,
            context.Progress);
          return ConvertColour<SRGBA<uint8>>(ClipToGamut<SRGBA<float>>(interpolatedColor));
        }
        case CompositeOperation::Add:
        case CompositeOperation::Accumulate:
        {
          krys_assert(context.Progress == 1.0);
          return AddColourComponents<AlphaPremultiplication::Premultiplied>(InterpolationColourSpace {},
                                                                            fromComponents, toComponents);
        }
      }
    }
    else
    {
      using InterpolationColourSpace = ColourInterpolationMethod::OKLab;

      auto fromComponents =
        from.ToColorTypeLossyCarryingForwardMissing<typename InterpolationColourSpace::ColourType>();
      auto toComponents =
        to.ToColorTypeLossyCarryingForwardMissing<typename InterpolationColourSpace::ColourType>();

      switch (context.CompositeOperation)
      {
        case CompositeOperation::Replace:
        {
          return InterpolateColourComponents<AlphaPremultiplication::Premultiplied>(
            InterpolationColourSpace {}, fromComponents, 1.0 - context.Progress, toComponents,
            context.Progress);
        }
        case CompositeOperation::Add:
        case CompositeOperation::Accumulate:
        {
          krys_assert(context.Progress == 1.0);
          return AddColourComponents<AlphaPremultiplication::Premultiplied>(InterpolationColourSpace {},
                                                                            fromComponents, toComponents);
        }
      }
    }

    krys_unreachable();
  }

  Color BlendWithoutPremultiply(const Color &from, const Color &to, const BlendingContext &context) noexcept
  {
    // FIXME: ExtendedColor - needs to handle color spaces.
    // We need to preserve the state of the valid flag at the end of the animation
    if (context.Progress == 1 && !to.IsValid())
    {
      return {};
    }

    auto [fromRed, fromGreen, fromBlue, fromAlpha] = from.ToColorTypeLossy<SRGBA<float>>().Resolved();
    auto [toRed, toGreen, toBlue, toAlpha] = to.ToColorTypeLossy<SRGBA<float>>().Resolved();

    auto blended = CreateFromComponentsClamping<SRGBA<float>>(
      boo::Blend(fromRed, toRed, context), boo::Blend(fromGreen, toGreen, context),
      boo::Blend(fromBlue, toBlue, context), boo::Blend(fromAlpha, toAlpha, context));

    return ConvertColour<SRGBA<uint8>>(blended);
  }
}
