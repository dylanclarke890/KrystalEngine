#include "Krystal.Booey/CSS/Values/Types.hpp"
#include "Krystal.Booey/CSS/Markup.hpp"
#include "Krystal.Booey/CSS/Properties/PropertyId.hpp"

namespace krys::boo::css
{
  void SerialiseCustomIdentifier(CSSOMString &builder, const SerialisationContext &,
                                 const CustomIdentifier &value) noexcept
  {
    SerialiseIdentifier(CSSOMString {value.Value.View()}, builder);
  }

  void SerialisePropertyIdentifier(CSSOMString &builder, const SerialisationContext &,
                                   const PropertyIdentifier &value) noexcept
  {
    builder.append(ToString(value.Value));
  }

  void SerialiseString(CSSOMString &builder, const SerialisationContext &,
                       const CSSOMStringAtom &value) noexcept
  {
    css::SerialiseString(CSSOMString {value.View()}, builder);
  }

  void SerialiseString(CSSOMString &builder, const SerialisationContext &, const CSSOMString &value) noexcept
  {
    css::SerialiseString(value, builder);
  }
}
