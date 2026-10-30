#include "Krystal.Booey/CSS/Values/Shapes/EllipseFunction.hpp"
#include "Krystal.Booey/CSS/Values/Primitives/NumericTypesSerialisation.hpp"
#include "Krystal.Core/Text/StringBuilder.hpp"

namespace krys::boo::css
{
  namespace
  {
    KRYS_NODISCARD bool HasDefaultValueForEllipseRadius(const Ellipse::RadialSize &radius) noexcept
    {
      return krys::SwitchOn(
        radius,
        [](const Ellipse::Extent &extent)
        {
          // FIXME: The spec says that `farthest-corner` should be the default, but this does not
          // match the tests.
          return std::holds_alternative<keywords::ClosestSide>(extent);
        },
        [](const auto &) { return false; });
    }
  }

  void Serialiser<Ellipse>::operator()(StringBuilder &builder, const SerialisationContext &context,
                                       const Ellipse &value) noexcept
  {
    // <ellipse()> = ellipse( <radial-size>? [ at <position> ]? )

    auto lengthBefore = builder.Length();

    if (!HasDefaultValueForEllipseRadius(get<0>(value.Radii))
        || !HasDefaultValueForEllipseRadius(get<1>(value.Radii)))
    {
      Serialise(builder, context, value.Radii);
    }

    if (value.Position)
    {
      // FIXME: To match other serialization of Percentage, this should not serialize if equal to the default
      // value of 50% 50%, but this does not match the tests.
      bool wroteSomething = builder.Length() != lengthBefore;
      builder.Append(wroteSomething ? u8" at " : u8"at ");
      Serialise(builder, context, *value.Position);
    }
  }
}
