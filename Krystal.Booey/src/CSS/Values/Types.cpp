#include "Krystal.Booey/CSS/Values/Types.hpp"
#include "Krystal.Booey/CSS/Markup.hpp"
#include "Krystal.Booey/CSS/Properties/PropertyId.hpp"

namespace krys::boo::css
{
  void SerializeCustomIdentifier(CSSOMString &builder, const SerialisationContext &,
                                 const CustomIdentifier &value) noexcept
  {
    SerializeIdentifier(CSSOMString {value.value.View()}, builder);
  }

  void SerializePropertyIdentifier(CSSOMString &builder, const SerialisationContext &,
                                   const PropertyIdentifier &value) noexcept
  {
    builder.append(ToString(value.value));
  }

  void SerializeString(CSSOMString &builder, const SerialisationContext &, const CSSOMStringAtom &value) noexcept
  {
    css::SerializeString(CSSOMString {value.View()}, builder);
  }

  void SerializeString(CSSOMString &builder, const SerialisationContext &, const CSSOMString &value) noexcept
  {
    css::SerializeString(value, builder);
  }
}
