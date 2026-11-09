#include "Krystal.Booey/HTML/Elements/HTMLLegendElement.hpp"
#include "Krystal.Booey/DOM/Algorithms/TreeQueries.hpp"
#include "Krystal.Booey/HTML/Attributes/ReflectAttribute.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLAnchorElement.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLButtonElement.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLFieldSetElement.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLFormElement.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLInputElement.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLLabelElement.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLOptionElement.hpp"

namespace krys::boo::html
{
  HTMLLegendElement::HTMLLegendElement(dom::Document &document) noexcept
      : HTMLElement(document, HTMLElementInterface::Legend)
  {
  }

#pragma region HTMLLegendElement

  namespace
  {
    /// @brief Whether @p element defines a command, leaving labels and legends to the caller.
    /// @see https://html.spec.whatwg.org/#commands
    KRYS_NODISCARD bool DefinesCommand(const dom::Element &element) noexcept
    {
      if (Is<HTMLAnchorElement>(element))
      {
        return element.HasAttribute(u8"href") || !element.AssignedAccessKey().empty();
      }
      if (Is<HTMLButtonElement>(element))
      {
        return true;
      }
      if (auto *input = DynamicDowncast<HTMLInputElement>(element))
      {
        auto type = input->Type();
        return type == u8"submit" || type == u8"reset" || type == u8"image" || type == u8"button"
               || type == u8"radio" || type == u8"checkbox" || !element.AssignedAccessKey().empty();
      }
      if (auto *option = DynamicDowncast<HTMLOptionElement>(element))
      {
        auto value = element.GetAttribute(u8"value");
        return (option->OwnerSelectElement() && (!value || !value->empty()))
               || !element.AssignedAccessKey().empty();
      }
      return !element.AssignedAccessKey().empty();
    }
  }

  RefPtr<dom::Element> HTMLLegendElement::AccessKeyDelegatee() const noexcept
  {
    // It is a child of a fieldset element.
    auto *parent = ParentNode();
    if (!Is<HTMLFieldSetElement>(parent))
    {
      return nullptr;
    }

    // Its parent has a descendant that defines a command that is neither a label element nor a legend
    // element. This element, if it exists, is the legend element's accesskey delegatee.
    for (auto *node = dom::TreeQueries::Next(*parent, parent); node;
         node = dom::TreeQueries::Next(*node, parent))
    {
      auto *element = DynamicDowncast<dom::Element>(*node);
      if (element && !Is<HTMLLabelElement>(*element) && !Is<HTMLLegendElement>(*element)
          && DefinesCommand(*element))
      {
        return ShareRefPtr(element);
      }
    }
    return nullptr;
  }

  bool HTMLLegendElement::AccessKeyAction(bool sendMouseEvents) noexcept
  {
    // The Hidden State, Disabled State, and Action facets of the command are the same as the respective
    // facets of the legend element's accesskey delegatee.
    if (auto delegatee = AccessKeyDelegatee())
    {
      return delegatee->AccessKeyAction(sendMouseEvents);
    }
    return HTMLElement::AccessKeyAction(sendMouseEvents);
  }

  HTMLFormElement *HTMLLegendElement::Form() const noexcept
  {
    // According to the specification, If the legend has a fieldset element as its parent, then the form
    // attribute must return the same value as the form attribute on that fieldset element. Otherwise, it must
    // return null.
    auto *fieldset = DynamicDowncast<HTMLFieldSetElement>(ParentNode());
    return fieldset ? fieldset->Form() : nullptr;
  }

  RefPtr<HTMLFormElement> HTMLLegendElement::FormForBindings() const noexcept
  {
    // FIXME: Retargeting against the legend's tree, once there are reference targets.
    return ShareRefPtr(Form());
  }

#pragma endregion

#pragma region HTMLLegendElement Obsolete members

  dom::DOMString HTMLLegendElement::Align() const noexcept
  {
    return Reflect<dom::DOMString>(*this, u8"align").value_or(u8"");
  }

  void HTMLLegendElement::Align(dom::DOMString value) noexcept
  {
    Reflect<dom::DOMString>(*this, u8"align", krys::move(value));
  }

#pragma endregion
}