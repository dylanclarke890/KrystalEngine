#include "Krystal.Booey/CSS/Values/Shapes/XywhFunction.hpp"
#include "Krystal.Booey/CSS/Values/Primitives/NumericTypesSerialisation.hpp"
#include "Krystal.Core/Text/StringBuilder.hpp"

namespace krys::boo::css
{
  void Serialiser<Xywh>::operator()(StringBuilder &builder, const SerialisationContext &context,
                                    const Xywh &value) noexcept
  {
    // <xywh()> = xywh( <length-percentage>{2} <length-percentage [0,∞]>{2} [ round <'border-radius'> ]? )

    Serialise(builder, context, value.Location);
    builder.Append(u8" ");
    Serialise(builder, context, value.Size);

    if (!HasDefaultValue(value.Radii))
    {
      builder.Append(u8" ");
      builder.Append(ToLower(ValueId::Round));
      builder.Append(u8" ");
      Serialise(builder, context, value.Radii);
    }
  }
}
