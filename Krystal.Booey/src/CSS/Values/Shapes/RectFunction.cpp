#include "Krystal.Booey/CSS/Values/Shapes/RectFunction.hpp"
#include "Krystal.Booey/CSS/Values/Primitives/NumericTypesSerialisation.hpp"
#include "Krystal.Core/Text/StringBuilder.hpp"

namespace krys::boo::css
{
  void Serialiser<RectShape>::operator()(StringBuilder &builder, const SerialisationContext &context,
                                         const RectShape &value) noexcept
  {
    // <rect()> = rect( [ <length-percentage> | auto ]{4} [ round <'border-radius'> ]? )

    Serialise(builder, context, value.Edges);

    if (!HasDefaultValue(value.Radii))
    {
      builder.Append(u8" ");
      builder.Append(ToLower(ValueId::Round));
      builder.Append(u8" ");
      Serialise(builder, context, value.Radii);
    }
  }
}
