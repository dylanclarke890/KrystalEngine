#pragma once

#include "Krystal.Booey/HTML/InputType.hpp"

namespace krys::boo::html
{
  /// @brief The base of the input types that Space and Enter click: the buttons, color and file.
  class BaseClickableWithKeyInputType : public InputType
  {
  public:
    static ShouldCallBaseEventHandler HandleKeydownEvent(HTMLInputElement &element,
                                                         dom::KeyboardEvent &event) noexcept;
    static void HandleKeypressEvent(HTMLInputElement &element, dom::KeyboardEvent &event) noexcept;
    static void HandleKeyupEvent(InputType &inputType, dom::KeyboardEvent &event) noexcept;

  protected:
    BaseClickableWithKeyInputType(Type type, HTMLInputElement &element) noexcept : InputType(type, element)
    {
    }

  private:
    ShouldCallBaseEventHandler HandleKeydownEvent(dom::KeyboardEvent &event) noexcept override;
    void HandleKeypressEvent(dom::KeyboardEvent &event) noexcept override;
    void HandleKeyupEvent(dom::KeyboardEvent &event) noexcept override;
  };
}
