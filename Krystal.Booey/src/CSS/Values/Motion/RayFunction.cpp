#include "Krystal.Booey/CSS/Values/Motion/RayFunction.hpp"
#include "Krystal.Booey/CSS/Values/Primitives/NumericTypesSerialisation.hpp"
#include "Krystal.Booey/CSS/Values/ValueId.hpp"
#include "Krystal.Core/Text/StringBuilder.hpp"

namespace krys::boo::css
{
  void Serialiser<Ray>::operator()(StringBuilder &builder, const SerialisationContext &context,
                                   const Ray &value) noexcept
  {
    // ray() = ray( <angle> && <ray-size>? && contain? && [at <position>]? )
    // https://drafts.fxtf.org/motion-1/#ray-function

    Serialise(builder, context, value.Angle);

    if (!std::holds_alternative<keywords::ClosestSide>(value.Size))
    {
      builder.Append(u8' ');
      Serialise(builder, context, value.Size);
    }

    if (value.Contain)
    {
      builder.Append(u8' ');
      Serialise(builder, context, *value.Contain);
    }

    if (value.Position)
    {
      builder.Append(u8' ');
      builder.Append(ToLower(ValueId::At));
      builder.Append(u8' ');
      Serialise(builder, context, *value.Position);
    }
  }
}
