#include "Krystal.Booey/CSS/CSSOM/DOMCSSNamespace.hpp"
#include "Krystal.Booey/CSS/CSSOM/Markup.hpp"

namespace krys::boo::css
{
  CSSOMString DOMCSSNamespace::Escape(const CSSOMString &ident) noexcept
  {
    StringBuilder builder;
    SerialiseIdentifier(ident, builder);
    return builder.ToString();
  }
}
