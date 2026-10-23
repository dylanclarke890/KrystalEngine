#pragma once

#include "Krystal.Booey/DOM/Types/DOMString.hpp"
#include "Krystal.Booey/HTML/DOMFormData.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include "Krystal.Core/Types/Variant.hpp"

namespace krys::boo::html
{
  /// @brief A form-associated custom element's submission value or state: none, a string, or entries.
  /// FIXME: Files, once there are any.
  /// @see https://html.spec.whatwg.org/#dom-elementinternals-setformvalue
  using CustomElementFormValue = Variant<std::nullptr_t, dom::DOMString, RefPtr<DOMFormData>>;
}
