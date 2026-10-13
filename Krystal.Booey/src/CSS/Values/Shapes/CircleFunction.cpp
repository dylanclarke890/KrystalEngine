#include "Krystal.Booey/CSS/Values/Shapes/CircleFunction.hpp"
#include "Krystal.Booey/CSS/Values/Primitives/NumericTypesSerialisation.hpp"

namespace krys::boo::css
{
  namespace
  {
    KRYS_NODISCARD bool HasDefaultValueForCircleRadius(const Circle::RadialSize &radius) noexcept
    {
      return krys::SwitchOn(
        radius,
        [](const Circle::Extent &extent)
        {
          // FIXME: The spec says that `farthest-corner` should be the default, but this does not
          // match the tests.
          return std::holds_alternative<keywords::ClosestSide>(extent);
        },
        [](const auto &) { return false; });
    }
  }

  void Serialiser<Circle>::operator()(CSSOMString &builder, const SerialisationContext &context,
                                      const Circle &value) noexcept
  {
    // <circle()> = circle( <radial-size>? [ at <position> ]? )

    auto lengthBefore = builder.size();

    if (!HasDefaultValueForCircleRadius(value.Radius))
    {
      Serialise(builder, context, value.Radius);
    }

    if (value.Position)
    {
      // FIXME: To match other serialization of Percentage, this should not serialize if equal to the default
      // value of 50% 50%, but this does not match the tests.
      bool wroteSomething = builder.size() != lengthBefore;
      builder.append(wroteSomething ? u8" at " : u8"at ");
      Serialise(builder, context, *value.Position);
    }
  }
}
