#pragma once

#include "Krystal.Booey/HTML/BaseTextInputType.hpp"
#include "Krystal.Booey/HTML/InputTypeNames.hpp"

namespace krys::boo::html
{
  /// @brief The search input type.
  // FIXME: The results and cancel buttons, recent searches and the incremental search event come with text
  // fields.
  class SearchInputType final : public BaseTextInputType
  {
  public:
    explicit SearchInputType(HTMLInputElement &element) noexcept : BaseTextInputType(Type::Search, element)
    {
    }

    KRYS_NODISCARD dom::DOMStringView FormControlType() const noexcept final
    {
      return InputTypeNames::Search;
    }

    ShouldCallBaseEventHandler HandleKeydownEvent(dom::KeyboardEvent &event) noexcept final;
  };
}
