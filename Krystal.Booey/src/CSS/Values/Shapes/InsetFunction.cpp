#include "Krystal.Booey/CSS/Values/Shapes/InsetFunction.hpp"
#include "Krystal.Booey/CSS/Values/Primitives/NumericTypesSerialisation.hpp"

namespace krys::boo::css
{
  void Serialiser<Inset>::operator()(CSSOMString &builder, const SerialisationContext &context,
                                     const Inset &value) noexcept
  {
    // <inset()> = inset( <length-percentage>{1,4} [ round <'border-radius'> ]? )

    Serialise(builder, context, value.Insets);

    if (!HasDefaultValue(value.Radii))
    {
      builder.append(u8" ");
      builder.append(ToLower(ValueId::Round));
      builder.append(u8" ");
      Serialise(builder, context, value.Radii);
    }
  }
}
