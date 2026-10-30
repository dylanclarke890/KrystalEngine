#include "Krystal.Booey/HTML/EmailInputType.hpp"
#include "Krystal.Booey/HTML/HTMLInputElement.hpp"
#include "Krystal.Booey/HTML/Parser/HTMLParserIdioms.hpp"
#include "Krystal.Booey/Infra/StringAlgorithms.hpp"
#include "Krystal.Booey/Platform/LocalizedStrings.hpp"
#include <regex>

namespace krys::boo::html
{
  namespace
  {
    // From https://html.spec.whatwg.org/#valid-e-mail-address.
    constexpr const char *EmailPattern =
      "^[a-zA-Z0-9.!#$%&'*+\\/=?^_`{|}~-]+@[a-zA-Z0-9](?:[a-zA-Z0-9-]{0,61}[a-zA-Z0-9])?(?:\\.[a-zA-Z0-9]"
      "(?:[a-zA-Z0-9-]{0,61}[a-zA-Z0-9])?)*$";

    KRYS_NODISCARD bool IsValidEmailAddress(dom::DOMStringView address) noexcept
    {
      if (address.empty())
      {
        return false;
      }

      static const std::regex regExp(EmailPattern,
                                     std::regex_constants::ECMAScript | std::regex_constants::icase);
      auto first = reinterpret_cast<const char *>(address.data());
      return std::regex_match(first, first + address.size(), regExp);
    }
  }

  bool EmailInputType::TypeMismatchFor(dom::DOMStringView value) const noexcept
  {
    krys_debug_assert(Element());
    if (value.empty())
    {
      return false;
    }
    if (!Element()->Multiple())
    {
      return !IsValidEmailAddress(value);
    }
    for (auto address : infra::StringAlgorithms::StrictlySplit(value, u8','))
    {
      if (!IsValidEmailAddress(infra::StringAlgorithms::StripLeadingAndTrailingWhitespace(address)))
      {
        return true;
      }
    }
    return false;
  }

  bool EmailInputType::TypeMismatch() const noexcept
  {
    krys_debug_assert(Element());
    return TypeMismatchFor(Element()->Value());
  }

  dom::DOMString EmailInputType::TypeMismatchText() const noexcept
  {
    krys_debug_assert(Element());
    return Element()->Multiple() ? ValidationMessageTypeMismatchForMultipleEmailText()
                                 : ValidationMessageTypeMismatchForEmailText();
  }

  bool EmailInputType::SupportsSelectionAPI() const noexcept
  {
    return false;
  }

  void EmailInputType::AttributeChanged(dom::DOMStringAtom localName) noexcept
  {
    if (localName == u8"multiple")
    {
      auto &element = *Element();
      element.SetValueInternal(SanitizeValue(element.Value()), TextFieldEventBehavior::DispatchNoEvent);
    }
    BaseTextInputType::AttributeChanged(localName);
  }

  dom::DOMString EmailInputType::SanitizeValue(dom::DOMStringView proposedValue) const noexcept
  {
    dom::DOMString noLineBreakValue;
    for (char8 character : proposedValue)
    {
      if (!IsHTMLLineBreak(character))
      {
        noLineBreakValue += character;
      }
    }

    krys_debug_assert(Element());
    if (!Element()->Multiple())
    {
      return dom::DOMString(infra::StringAlgorithms::StripLeadingAndTrailingWhitespace(noLineBreakValue));
    }

    dom::DOMString strippedValue;
    bool first = true;
    for (auto address : infra::StringAlgorithms::StrictlySplit(noLineBreakValue, u8','))
    {
      if (!first)
      {
        strippedValue += u8',';
      }
      first = false;
      strippedValue += infra::StringAlgorithms::StripLeadingAndTrailingWhitespace(address);
    }
    return strippedValue;
  }
}
