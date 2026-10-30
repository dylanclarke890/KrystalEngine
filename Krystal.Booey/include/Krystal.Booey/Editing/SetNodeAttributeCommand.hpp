#pragma once

#include "Krystal.Booey/DOM/Types/DOMString.hpp"
#include "Krystal.Booey/Editing/EditCommand.hpp"
#include "Krystal.Core/Types/Maybe.hpp"

namespace krys::boo::dom
{
  class Element;
}

namespace krys::boo
{
  /// @brief Sets an attribute of an element, or removes it when the value is null.
  class SetNodeAttributeCommand final : public SimpleEditCommand
  {
  private:
    const Ref<dom::Element> _element;
    dom::DOMStringAtom _attribute;
    Maybe<dom::DOMString> _value;
    Maybe<dom::DOMString> _oldValue;

  public:
    KRYS_NODISCARD static Ref<SetNodeAttributeCommand>
      Create(dom::Element &element, dom::DOMStringAtom attribute, Maybe<dom::DOMString> value) noexcept;

    SetNodeAttributeCommand(dom::Element &element, dom::DOMStringAtom attribute,
                            Maybe<dom::DOMString> value) noexcept;

  private:
    void DoApply() noexcept override;
    void DoUnapply() noexcept override;
  };
}
