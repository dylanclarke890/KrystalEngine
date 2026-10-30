#include "Krystal.Booey/CSS/Values/Shapes/PolygonFunction.hpp"
#include "Krystal.Booey/CSS/Values/Primitives/NumericTypesSerialisation.hpp"
#include "Krystal.Core/Text/StringBuilder.hpp"

namespace krys::boo::css
{
  void Serialiser<Polygon>::operator()(StringBuilder &builder, const SerialisationContext &context,
                                       const Polygon &value) noexcept
  {
    // <polygon()> = polygon( <'fill-rule'>? [ round <length> ]? ,
    //                        [<length-percentage> <length-percentage>]# )

    // FIXME: Add support the "round" clause.

    if (value.FillRule && std::holds_alternative<keywords::Evenodd>(*value.FillRule))
    {
      Serialise(builder, context, *value.FillRule);
      builder.Append(u8", ");
    }

    Serialise(builder, context, value.Vertices);
  }
}
