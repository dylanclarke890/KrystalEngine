#include "Krystal.Booey/Editing/Commands/SetNodeAttributeCommand.hpp"
#include "Krystal.Booey/DOM/Mixins/ChildNode.hpp"
#include "Krystal.Booey/DOM/Nodes/ContainerNode.hpp"
#include "Krystal.Booey/DOM/Nodes/Document.hpp"
#include "Krystal.Booey/DOM/Nodes/Element.hpp"
#include "Krystal.Booey/DOM/Nodes/Text.hpp"
#include "Krystal.Booey/Editing/Commands/ApplyStyleCommand.hpp"
#include "Krystal.Booey/Editing/Editing.hpp"

namespace krys::boo
{
  namespace
  {
    void SetOrRemoveAttribute(dom::Element &element, dom::DOMStringAtom attribute,
                              const Maybe<dom::DOMString> &value) noexcept
    {
      if (value)
      {
        (void)element.SetAttribute(attribute, *value);
      }
      else
      {
        element.RemoveAttribute(attribute);
      }
    }
  }

  Ref<SetNodeAttributeCommand> SetNodeAttributeCommand::Create(dom::Element &element,
                                                               dom::DOMStringAtom attribute,
                                                               Maybe<dom::DOMString> value) noexcept
  {
    return CreateRef<SetNodeAttributeCommand>(element, attribute, krys::move(value));
  }

  SetNodeAttributeCommand::SetNodeAttributeCommand(dom::Element &element, dom::DOMStringAtom attribute,
                                                   Maybe<dom::DOMString> value) noexcept
      : SimpleEditCommand(element.NodeDocument()), _element(ShareRef(element)), _attribute(attribute),
        _value(krys::move(value))
  {
  }

  void SetNodeAttributeCommand::DoApply() noexcept
  {
    _oldValue = _element->GetAttribute(_attribute);
    SetOrRemoveAttribute(*_element, _attribute, _value);
  }

  void SetNodeAttributeCommand::DoUnapply() noexcept
  {
    SetOrRemoveAttribute(*_element, _attribute, _oldValue);
    _oldValue = null;
  }
}
