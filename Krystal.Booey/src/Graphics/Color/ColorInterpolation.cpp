#include "Krystal.Booey/Graphics/Color/ColorInterpolation.hpp"
#include "Krystal.Booey/Graphics/Color/Color.hpp"
#include "Krystal.Core/Assert.hpp"
#include "Krystal.Core/Visitor.hpp"

namespace krys::boo
{
  Color InterpolateColors(ColourInterpolationMethod colorInterpolationMethod, Color color1,
                          double color1Multiplier, Color color2, double color2Multiplier) noexcept
  {
    return krys::SwitchOn(
      colorInterpolationMethod.ColourSpace,
      [&]<typename MethodColourSpace>(const MethodColourSpace &colorSpace) -> Color
      {
        using ColorType = typename MethodColourSpace::ColourType;
        switch (colorInterpolationMethod.AlphaPremultiplication)
        {
          case AlphaPremultiplication::Premultiplied:
          {
            return InterpolateColourComponents<AlphaPremultiplication::Premultiplied>(
              colorSpace, color1.ToColorTypeLossyCarryingForwardMissing<ColorType>(), color1Multiplier,
              color2.ToColorTypeLossyCarryingForwardMissing<ColorType>(), color2Multiplier);
          }
          case AlphaPremultiplication::Unpremultiplied:
          {
            return InterpolateColourComponents<AlphaPremultiplication::Unpremultiplied>(
              colorSpace, color1.ToColorTypeLossyCarryingForwardMissing<ColorType>(), color1Multiplier,
              color2.ToColorTypeLossyCarryingForwardMissing<ColorType>(), color2Multiplier);
          }
        }

        krys_unreachable();
      });
  }
}