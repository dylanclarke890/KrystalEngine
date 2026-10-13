#include "Krystal.Booey/CSS/Values/UrlValue.hpp"
#include "Krystal.Booey/CSS/Values/Primitives/NumericTypes.hpp"
#include "Krystal.Booey/CSS/Values/Primitives/NumericTypesComputedStyleDependencies.hpp"
#include "Krystal.Booey/CSS/Values/Primitives/NumericTypesSerialisation.hpp"
#include "Krystal.Booey/CSS/Values/Primitives/NumericTypesValueVisitation.hpp"

namespace krys::boo::css
{
  CSSOMString UrlValue::CustomCSSText(const SerialisationContext &context) const noexcept
  {
    return Serialise(context, _url);
  }

  bool UrlValue::Equals(const UrlValue &other) const noexcept
  {
    return _url == other._url;
  }

  IterationStatus
    UrlValue::CustomVisitChildren(KRYS_NOESCAPE const Func<IterationStatus(Value &)> &func) const noexcept
  {
    return VisitValueChildren(func, _url);
  }
}
