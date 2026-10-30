#include "Krystal.Booey/CSS/Values/Types.hpp"
#include "Krystal.Booey/CSS/Markup.hpp"
#include "Krystal.Booey/CSS/Properties/PropertyId.hpp"
#include "Krystal.Core/Text/StringBuilder.hpp"

namespace krys::boo::css
{
  void SerialiseCustomIdentifier(StringBuilder &builder, const SerialisationContext &,
                                 const CustomIdentifier &value) noexcept
  {
    SerialiseIdentifier(CSSOMString {value.Value.View()}, builder);
  }

  void SerialisePropertyIdentifier(StringBuilder &builder, const SerialisationContext &,
                                   const PropertyIdentifier &value) noexcept
  {
    builder.Append(ToString(value.Value));
  }

  void SerialiseString(StringBuilder &builder, const SerialisationContext &,
                       const CSSOMStringAtom &value) noexcept
  {
    css::SerialiseString(CSSOMString {value.View()}, builder);
  }

  void SerialiseString(StringBuilder &builder, const SerialisationContext &,
                       const CSSOMString &value) noexcept
  {
    css::SerialiseString(value, builder);
  }
}
