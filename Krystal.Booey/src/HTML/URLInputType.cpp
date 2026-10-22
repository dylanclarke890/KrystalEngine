#include "Krystal.Booey/HTML/URLInputType.hpp"
#include "Krystal.Booey/HTML/HTMLInputElement.hpp"
#include "Krystal.Booey/Infra/StringAlgorithms.hpp"
#include "Krystal.Booey/Platform/LocalizedStrings.hpp"
#include "Krystal.Booey/URL/URL.hpp"

namespace krys::boo::html
{
  bool URLInputType::TypeMismatchFor(dom::DOMStringView value) const noexcept
  {
    return !value.empty() && !url::URL(value).IsValid();
  }

  bool URLInputType::TypeMismatch() const noexcept
  {
    krys_debug_assert(Element());
    return TypeMismatchFor(Element()->Value());
  }

  dom::DOMString URLInputType::TypeMismatchText() const noexcept
  {
    return ValidationMessageTypeMismatchForURLText();
  }

  dom::DOMString URLInputType::SanitizeValue(dom::DOMStringView proposedValue) const noexcept
  {
    return dom::DOMString(infra::StringAlgorithms::StripLeadingAndTrailingWhitespace(
      BaseTextInputType::SanitizeValue(proposedValue)));
  }
}
