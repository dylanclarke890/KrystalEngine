#include "Krystal.Booey/CSS/Values/Shapes/XywhFunction.hpp"
#include "Krystal.Booey/CSS/Values/Primitives/NumericTypesSerialisation.hpp"

namespace krys::boo::css
{
  void Serialiser<Xywh>::operator()(CSSOMString &builder, const SerialisationContext &context,
                                    const Xywh &value) noexcept
  {
    // <xywh()> = xywh( <length-percentage>{2} <length-percentage [0,∞]>{2} [ round <'border-radius'> ]? )

    Serialise(builder, context, value.Location);
    builder.append(u8" ");
    Serialise(builder, context, value.Size);

    if (!HasDefaultValue(value.Radii))
    {
      builder.append(u8" ");
      builder.append(ToLower(ValueId::Round));
      builder.append(u8" ");
      Serialise(builder, context, value.Radii);
    }
  }
}
