#include "Krystal.Booey/CSS/Values/Motion/RayFunction.hpp"
#include "Krystal.Booey/CSS/Values/Primitives/NumericTypesSerialisation.hpp"
#include "Krystal.Booey/CSS/Values/ValueId.hpp"

namespace krys::boo::css
{
  void Serialiser<Ray>::operator()(CSSOMString &builder, const SerialisationContext &context,
                                   const Ray &value) noexcept
  {
    // ray() = ray( <angle> && <ray-size>? && contain? && [at <position>]? )
    // https://drafts.fxtf.org/motion-1/#ray-function

    Serialise(builder, context, value.Angle);

    if (!std::holds_alternative<keywords::ClosestSide>(value.Size))
    {
      builder.push_back(u8' ');
      Serialise(builder, context, value.Size);
    }

    if (value.Contain)
    {
      builder.push_back(u8' ');
      Serialise(builder, context, *value.Contain);
    }

    if (value.Position)
    {
      builder.push_back(u8' ');
      builder.append(ToLower(ValueId::At));
      builder.push_back(u8' ');
      Serialise(builder, context, *value.Position);
    }
  }
}
