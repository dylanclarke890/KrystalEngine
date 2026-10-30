#include "Krystal.Booey/Style/Values/Motion/RayFunction.hpp"

#include "Krystal.Booey/Style/Values/Primitives/NumericTypesSerialisation.hpp"
#include "Krystal.Core/Text/StringBuilder.hpp"

namespace krys::boo::style
{
  void Serialiser<Ray>::operator()(StringBuilder &builder, const css::SerialisationContext &context,
                                   const RenderStyle &style, const Ray &value) noexcept
  {
    // ray() = ray( <angle> && <ray-size>? && contain? && [at <position>]? )
    // https://drafts.fxtf.org/motion-1/#ray-function

    Serialise(builder, context, style, value.Angle);

    if (!std::holds_alternative<css::keywords::ClosestSide>(value.Size))
    {
      builder.Append(u8' ');
      Serialise(builder, context, style, value.Size);
    }

    if (value.Contain)
    {
      builder.Append(u8' ');
      Serialise(builder, context, style, *value.Contain);
    }

    if (value.Position)
    {
      builder.Append(u8" at ");
      Serialise(builder, context, style, *value.Position);
    }
  }
}
