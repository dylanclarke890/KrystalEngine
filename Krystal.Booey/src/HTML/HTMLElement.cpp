#include "Krystal.Booey/HTML/HTMLElement.hpp"
#include "Krystal.Booey/DOM/AbortSignal.hpp"
#include "Krystal.Booey/DOM/Algorithms/ElementAlgorithms.hpp"
#include "Krystal.Booey/DOM/Attr.hpp"
#include "Krystal.Booey/DOM/ShadowRoot.hpp"
#include "Krystal.Booey/HTML/Attributes/EnumeratedAttributes.hpp"
#include "Krystal.Booey/HTML/Attributes/Reflection.hpp"
#include "Krystal.Booey/HTML/CustomElement/CustomElementRegistry.hpp"
#include "Krystal.Booey/HTML/ElementInternals.hpp"
#include "Krystal.Booey/HTML/HTMLSlotElement.hpp"
#include "Krystal.Booey/Infra/StringAlgorithms.hpp"

namespace krys::boo::html
{
  HTMLElement::HTMLElement(dom::Document &document, HTMLElementInterface interface, HTMLElementFlags flags,
                           dom::NodeFlags nodeFlags) noexcept
      : Element(document, dom::NullQualifiedName(), nodeFlags | dom::NodeFlags::IsHTMLElement), _flags(flags),
        _interface(interface)
  {
  }

  HTMLElement::HTMLElement(dom::Document &document) noexcept
      : HTMLElement(document, HTMLElementInterface::None)
  {
  }

#pragma region HTMLElement

  dom::DOMString HTMLElement::Title() const noexcept
  {
    return Reflection::Reflect<dom::DOMString>(*this, u8"title").value_or(u8"");
  }

  void HTMLElement::Title(dom::DOMString &&value) noexcept
  {
    Reflection::Reflect<dom::DOMString>(*this, u8"title", krys::move(value));
  }

  dom::DOMString HTMLElement::Lang() const noexcept
  {
    return Reflection::Reflect<dom::DOMString>(*this, u8"lang").value_or(u8"");
  }

  void HTMLElement::Lang(dom::DOMString &&value) noexcept
  {
    Reflection::Reflect<dom::DOMString>(*this, u8"lang", krys::move(value));
  }

  bool HTMLElement::Translate() const noexcept
  {
    auto value = Reflection::Reflect<dom::DOMString>(*this, u8"translate");
    auto state = EnumeratedAttribute<"translate", HTMLElement>::ResolveState(value);
    return state == AttributeState::Yes;
  }

  void HTMLElement::Translate(bool value) noexcept
  {
    Reflection::Reflect<dom::DOMString>(*this, u8"translate", value ? u8"yes" : u8"no");
  }

  dom::DOMString HTMLElement::Dir() const noexcept
  {
    auto value = Reflection::Reflect<dom::DOMString>(*this, u8"dir");
    return EnumeratedAttribute<"dir", HTMLElement>::ResolveCanonicalKeyword<dom::DOMString>(
      krys::move(value));
  }

  void HTMLElement::Dir(dom::DOMString &&value) noexcept
  {
    Reflection::Reflect<dom::DOMString>(*this, u8"dir", krys::move(value));
  }

  BoolOr<dom::DOMString> HTMLElement::Hidden() const noexcept
  {
    auto value = Reflection::Reflect<dom::DOMString>(*this, u8"hidden");

    auto state = EnumeratedAttribute<"hidden", HTMLElement>::ResolveState(value);
    if (state == AttributeState::UntilFound)
    {
      return u8"until-found";
    }

    return state == AttributeState::Hidden;
  }

  void HTMLElement::Hidden(dom::DOMString &&value) noexcept
  {
    if (value == u8"until-found")
    {
      dom::ElementAlgorithms::SetAttributeValue(*this, u8"hidden", u8"until-found");
    }
    else if (value.empty())
    {
      dom::ElementAlgorithms::RemoveAttributeByName(u8"hidden", *this);
    }
    else
    {
      dom::ElementAlgorithms::SetAttributeValue(*this, u8"hidden", u8"");
    }
  }

  void HTMLElement::Hidden(bool value) noexcept
  {
    if (value)
    {
      dom::ElementAlgorithms::SetAttributeValue(*this, u8"hidden", u8"");
    }
    else
    {
      dom::ElementAlgorithms::RemoveAttributeByName(u8"hidden", *this);
    }
  }

  void HTMLElement::Hidden(double value) noexcept
  {
    if (value == 0)
    {
      dom::ElementAlgorithms::RemoveAttributeByName(u8"hidden", *this);
    }
    else if (std::isnan(value))
    {
      dom::ElementAlgorithms::RemoveAttributeByName(u8"hidden", *this);
    }
    else
    {
      dom::ElementAlgorithms::SetAttributeValue(*this, u8"hidden", u8"");
    }
  }

  bool HTMLElement::Inert() const noexcept
  {
    return Reflection::Reflect<bool>(*this, u8"inert");
  }

  void HTMLElement::Inert(bool value) noexcept
  {
    Reflection::Reflect<bool>(*this, u8"inert", krys::move(value));
  }

  void HTMLElement::Click() noexcept
  {
    // TODO(HTMLELEMENT, EVENTS, HTML): Implement Click() method.
    // If this element is a form control that is disabled, then return.
    // If this element's click in progress flag is set, then return.
    // Set this element's click in progress flag.
    // Fire a synthetic pointer event named click at this element, with the not trusted flag set.
    // Unset this element's click in progress flag.
  }

  dom::DOMString HTMLElement::AccessKey() const noexcept
  {
    return Reflection::Reflect<dom::DOMString>(*this, u8"accesskey").value_or(u8"");
  }

  void HTMLElement::AccessKey(dom::DOMString &&value) noexcept
  {
    Reflection::Reflect<dom::DOMString>(*this, u8"accesskey", krys::move(value));
  }

  dom::DOMString HTMLElement::AccessKeyLabel() const noexcept
  {
    // TODO(HTMLELEMENT, HTML): Implement AccessKeyLabel() method.
    return {};
  }

  bool HTMLElement::Draggable() const noexcept
  {
    auto value = Reflection::Reflect<dom::DOMString>(*this, u8"draggable");
    switch (EnumeratedAttribute<"draggable", HTMLElement>::ResolveState(value))
    {
      case AttributeState::True:  return true;
      case AttributeState::False: return false;
      case AttributeState::Auto:
      {
        // TODO(HTMLELEMENT, DRAGGABLE, HTML): Implement auto behavior for Draggable() getter.
        //  If the element is an img element, an object element that represents an image, or an a element with
        //  an href content attribute, the draggable IDL attribute must return true; otherwise, the draggable
        //  IDL attribute must return false.
        return false;
      }
    }

    return false;
  }

  void HTMLElement::Draggable(bool value) noexcept
  {
    dom::ElementAlgorithms::SetAttributeValue(*this, u8"draggable", value ? u8"true" : u8"false");
  }

  bool HTMLElement::SpellCheck() const noexcept
  {
    auto value = Reflection::Reflect<dom::DOMString>(*this, u8"spellcheck");
    switch (EnumeratedAttribute<"spellcheck", HTMLElement>::ResolveState(value))
    {
      case AttributeState::True:  return true;
      case AttributeState::False: return false;
      case AttributeState::Default:
      {
        // TODO(HTMLELEMENT, SPELLCHECK, HTML): Implement default behavior for SpellCheck() getter.
        // The spellcheck IDL attribute, on getting, must return true if the element's spellcheck content
        // attribute is in the True state, or if the element's spellcheck content attribute is in the Default
        // state and the element's default behavior is true-by-default, or if the element's spellcheck content
        // attribute is in the Default state and the element's default behavior is inherit-by-default and the
        // element's parent element's spellcheck IDL attribute would return true; otherwise, if none of those
        // conditions applies, then the attribute must instead return false.
        return false;
      }
    }

    return false;
  }

  void HTMLElement::SpellCheck(bool value) noexcept
  {
    dom::ElementAlgorithms::SetAttributeValue(*this, u8"spellcheck", value ? u8"true" : u8"false");
  }

  dom::DOMString HTMLElement::WritingSuggestions() const noexcept
  {
    // TODO(HTMLELEMENT, WRITINGSUGGESTIONS, HTML): Implement WritingSuggestions() method.
    // Return this's computed writing suggestions value.
    return {};
  }

  void HTMLElement::WritingSuggestions(dom::DOMString &&value) noexcept
  {
    Reflection::ReflectSetter(*this, u8"writingsuggestions", krys::move(value));
  }

  dom::DOMString HTMLElement::Autocapitalize() const noexcept
  {
    // TODO(HTMLELEMENT, AUTOCAPITALIZE, HTML): Implement Autocapitalize() method.
    // Let state be the own autocapitalization hint of this.
    // If state is Default, then return the empty string.
    // If state is None, then return "none".
    // If state is Sentences, then return "sentences".
    // Return the keyword value corresponding to state.
    return {};
  }

  void HTMLElement::Autocapitalize(dom::DOMString &&value) noexcept
  {
    Reflection::ReflectSetter(*this, u8"autocapitalize", krys::move(value));
  }

  bool HTMLElement::Autocorrect() const noexcept
  {
    // TODO(HTMLELEMENT, AUTOCORRECT, HTML): Implement Autocorrect() method.
    // The autocorrect getter steps are: return true if the element's used autocorrection state is On and
    // false if the element's used autocorrection state is Off
    return false;
  }

  void HTMLElement::Autocorrect(bool value) noexcept
  {
    dom::ElementAlgorithms::SetAttributeValue(*this, u8"autocorrect", value ? u8"on" : u8"off");
  }

  dom::DOMString HTMLElement::InnerText() const noexcept
  {
    // TODO(HTMLELEMENT, HTML): Implement InnerText() setter method.
    return {};
  }

  void HTMLElement::InnerText(dom::DOMString &&value) noexcept
  {
    // TODO(HTMLELEMENT, HTML): Implement InnerText() getter method.
  }

  dom::DOMString HTMLElement::OuterText() const noexcept
  {
    // TODO(HTMLELEMENT, HTML): Implement OuterText() method.
    return {};
  }

  void HTMLElement::OuterText(dom::DOMString &&value) noexcept
  {
    // TODO(HTMLELEMENT, HTML): Implement OuterText() getter method.
  }

  dom::ExceptionOr<Ref<ElementInternals>> HTMLElement::AttachInternals() noexcept
  {
    // TODO(HTMLELEMENT, ELEMENTINTERNALS, HTML): Implement AttachInternals() method.
    return dom::ExceptionCode::NotSupportedError;
  }

  uint32 HTMLElement::HeadingOffset() const noexcept
  {
    return Reflection::Reflect<uint32, ReflectRange<uint32>(0u, 8u)>(*this, u8"headingoffset");
  }

  void HTMLElement::HeadingOffset(uint32 value) noexcept
  {
    Reflection::Reflect<uint32>(*this, u8"headingoffset", krys::move(value));
  }

  bool HTMLElement::HeadingReset() const noexcept
  {
    return Reflection::Reflect<bool>(*this, u8"headingreset");
  }

  void HTMLElement::HeadingReset(bool value) noexcept
  {
    Reflection::Reflect<bool>(*this, u8"headingreset", krys::move(value));
  }

#pragma endregion
}