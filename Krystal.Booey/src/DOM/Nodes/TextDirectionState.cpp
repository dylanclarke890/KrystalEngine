#include "Krystal.Booey/DOM/Nodes/TextDirectionState.hpp"
#include "Krystal.Booey/DOM/Nodes/Element.hpp"
#include "Krystal.Booey/HTML/Attributes/EnumeratedAttributes.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLBDIElement.hpp"

namespace krys::boo::dom
{
  TextDirectionState ParseTextDirectionState(Maybe<DOMStringView> value) noexcept
  {
    return html::EnumeratedAttribute<"dir", html::HTMLElement>::ResolveState(value).value_or(
      TextDirectionState::Undefined);
  }

  TextDirectionState ElementTextDirectionState(const Element &element) noexcept
  {
    return ParseTextDirectionState(element.GetAttribute(u8"dir"));
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
