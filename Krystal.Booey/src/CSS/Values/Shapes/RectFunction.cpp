#include "Krystal.Booey/CSS/Values/Shapes/RectFunction.hpp"
#include "Krystal.Booey/CSS/Values/Primitives/NumericTypesSerialisation.hpp"

namespace krys::boo::css
{
  void Serialiser<RectShape>::operator()(CSSOMString &builder, const SerialisationContext &context,
                                         const RectShape &value) noexcept
  {
    // <rect()> = rect( [ <length-percentage> | auto ]{4} [ round <'border-radius'> ]? )

    Serialise(builder, context, value.Edges);

    if (!HasDefaultValue(value.Radii))
    {
      builder.append(u8" ");
      builder.append(ToLower(ValueId::Round));
      builder.append(u8" ");
      Serialise(builder, context, value.Radii);
    }
  }
}
