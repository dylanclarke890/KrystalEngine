#include "Krystal.Booey/CSS/Values/Borders/BorderRadius.hpp"
#include "Krystal.Booey/CSS/Values/Primitives/NumericTypesSerialisation.hpp"
#include "Krystal.Core/Types/Pair.hpp"

namespace krys::boo::css
{
  namespace
  {
    using Radii = SpaceSeparatedList<LengthPercentage<NonNegative>, 4>;

    KRYS_NODISCARD bool HasDefaultValueForAxis(const BorderRadius::Axis &values) noexcept
    {
      return values.Value[3] == values.Value[1] && values.Value[2] == values.Value[0]
             && values.Value[1] == values.Value[0] && values.Value[0] == 0_css_px;
    }

    KRYS_NODISCARD Pair<Radii, bool> GatherSerialisableRadiiForAxis(const BorderRadius::Axis &values) noexcept
    {
      bool isDefaultValue = false;

      Radii::Container result;
      result.push_back(values.Value[0]);

      if (values.Value[3] != values.Value[1])
      {
        result.push_back(values.Value[1]);
        result.push_back(values.Value[2]);
        result.push_back(values.Value[3]);
      }
      else if (values.Value[2] != values.Value[0])
      {
        result.push_back(values.Value[1]);
        result.push_back(values.Value[2]);
      }
      else if (values.Value[1] != values.Value[0])
      {
        result.push_back(values.Value[1]);
      }
      else
      {
        isDefaultValue = result[0] == 0_css_px;
      }

      return {Radii {krys::move(result)}, isDefaultValue};
    }
  }

  bool HasDefaultValue(const BorderRadius &borderRadius) noexcept
  {
    return HasDefaultValueForAxis(borderRadius.Horizontal) && HasDefaultValueForAxis(borderRadius.Vertical);
  }

  void Serialiser<BorderRadius>::operator()(CSSOMString &builder, const SerialisationContext &context,
                                            const BorderRadius &value) noexcept
  {
    // <'border-radius'> = <length-percentage [0,∞]>{1,4} [ / <length-percentage [0,∞]>{1,4} ]?

    auto [horizontal, horizontalIsDefault] = GatherSerialisableRadiiForAxis(value.Horizontal);
    auto [vertical, verticalIsDefault] = GatherSerialisableRadiiForAxis(value.Vertical);

    if (!horizontalIsDefault || !verticalIsDefault)
    {
      Serialise(builder, context, horizontal);

      if (horizontal != vertical)
      {
        builder.append(u8" / ");
        Serialise(builder, context, vertical);
      }
    }
  }
}
