#include "Krystal.Booey/CSS/Values/Shapes/InsetFunction.hpp"
#include "Krystal.Booey/CSS/Values/Primitives/NumericTypesSerialisation.hpp"
#include "Krystal.Core/Text/StringBuilder.hpp"

namespace krys::boo::css
{
  void Serialiser<Inset>::operator()(StringBuilder &builder, const SerialisationContext &context,
                                     const Inset &value) noexcept
  {
    // <inset()> = inset( <length-percentage>{1,4} [ round <'border-radius'> ]? )

    Serialise(builder, context, value.Insets);

    if (!HasDefaultValue(value.Radii))
    {
      builder.Append(u8" ");
      builder.Append(ToLower(ValueId::Round));
      builder.Append(u8" ");
      Serialise(builder, context, value.Radii);
    }
  }
}
