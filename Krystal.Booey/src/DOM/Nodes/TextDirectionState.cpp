#include "Krystal.Booey/DOM/Nodes/TextDirectionState.hpp"
#include "Krystal.Booey/DOM/Nodes/Element.hpp"
#include "Krystal.Booey/HTML/HTMLBDIElement.hpp"
#include "Krystal.Booey/Infra/StringAlgorithms.hpp"

namespace krys::boo::dom
{
  TextDirectionState ParseTextDirectionState(Maybe<DOMStringView> value) noexcept
  {
    if (!value)
    {
      return TextDirectionState::Undefined;
    }
    if (infra::StringAlgorithms::ASCIICaseInsensitiveMatch(*value, u8"ltr"))
    {
      return TextDirectionState::LTR;
    }
    if (infra::StringAlgorithms::ASCIICaseInsensitiveMatch(*value, u8"rtl"))
    {
      return TextDirectionState::RTL;
    }
    if (infra::StringAlgorithms::ASCIICaseInsensitiveMatch(*value, u8"auto"))
    {
      return TextDirectionState::Auto;
    }
    return TextDirectionState::Undefined;
  }

  TextDirectionState ElementTextDirectionState(const Element &element) noexcept
  {
    auto value = element.GetAttribute(u8"dir");
    return ParseTextDirectionState(value ? Maybe<DOMStringView>(*value) : null);
  }

  bool ElementHasValidTextDirectionState(const Element &element) noexcept
  {
    return ElementTextDirectionState(element) != TextDirectionState::Undefined;
  }

  bool ElementHasAutoTextDirectionState(const Element &element) noexcept
  {
    auto textDirectionState = ElementTextDirectionState(element);
    return textDirectionState == TextDirectionState::Auto
           || (Is<html::HTMLBDIElement>(element) && textDirectionState == TextDirectionState::Undefined);
  }
}
