#include "Krystal.Booey/HTML/InputTypes/EmailInputType.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLInputElement.hpp"
#include "Krystal.Booey/HTML/Parser/ParserIdioms.hpp"
#include "Krystal.Booey/Infra/StringAlgorithms.hpp"
#include "Krystal.Booey/Platform/Locale/LocalizedStrings.hpp"
#include "Krystal.Core/Text/ASCII.hpp"

namespace krys::boo::html
{
  namespace
  {
    /// @brief Whether @p c is one of the ABNF's `atext` characters, which make up the part before the `@`.
    KRYS_NODISCARD constexpr bool IsEmailLocalPartCharacter(char8_t c) noexcept
    {
      return text::IsASCIIAlphanumeric(c)
             || dom::DOMStringView(u8".!#$%&'*+/=?^_`{|}~-").find(c) != dom::DOMStringView::npos;
    }

    /// @brief Whether @p label is a domain label: 1 to 63 ASCII alphanumerics and hyphens, starting and
    /// ending with an alphanumeric.
    KRYS_NODISCARD constexpr bool IsValidEmailLabel(dom::DOMStringView label) noexcept
    {
      if (label.empty() || label.size() > 63 || !text::IsASCIIAlphanumeric(label.front())
          || !text::IsASCIIAlphanumeric(label.back()))
      {
        return false;
      }
      for (auto c : label)
      {
        if (!text::IsASCIIAlphanumeric(c) && c != u8'-')
        {
          return false;
        }
      }
      return true;
    }

    /// @brief Whether @p address is a [valid email
    /// address](https://html.spec.whatwg.org/#valid-e-mail-address).
    KRYS_NODISCARD constexpr bool IsValidEmailAddress(dom::DOMStringView address) noexcept
    {
      auto at = address.find(u8'@');
      if (at == 0 || at == dom::DOMStringView::npos)
      {
        return false;
      }
      for (auto c : address.substr(0, at))
      {
        if (!IsEmailLocalPartCharacter(c))
        {
          return false;
        }
      }

      auto domain = address.substr(at + 1);
      while (true)
      {
        auto dot = domain.find(u8'.');
        if (!IsValidEmailLabel(domain.substr(0, dot)))
        {
          return false;
        }
        if (dot == dom::DOMStringView::npos)
        {
          return true;
        }
        domain.remove_prefix(dot + 1);
      }
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
