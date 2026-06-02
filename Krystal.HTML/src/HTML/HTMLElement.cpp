#include "Krystal.HTML/HTML/HTMLElement.hpp"
#include "Krystal.HTML/DOM/AbortSignal.hpp"
#include "Krystal.HTML/DOM/Algorithms/ElementAlgorithms.hpp"
#include "Krystal.HTML/DOM/Attr.hpp"
#include "Krystal.HTML/DOM/ShadowRoot.hpp"
#include "Krystal.HTML/HTML/Attributes/EnumeratedAttributes.hpp"
#include "Krystal.HTML/HTML/Attributes/Reflection.hpp"
#include "Krystal.HTML/HTML/CustomElement/CustomElementRegistry.hpp"
#include "Krystal.HTML/HTML/ElementInternals.hpp"
#include "Krystal.HTML/HTML/HTMLSlotElement.hpp"
#include "Krystal.HTML/Infra/StringAlgorithms.hpp"

namespace Krys::HTML
{
  HTMLElement::HTMLElement(Document &document, DOMInterface interface, NodeFlags flags) noexcept
      : Element(document, flags | NodeFlags::IsHTMLElement), _interface(interface)
  {
  }

  HTMLElement::HTMLElement(Document &document) noexcept : HTMLElement(document, DOMInterface::None)
  {
  }

#pragma region HTMLElement

  DOMString HTMLElement::Title() const noexcept
  {
    return Attributes::Reflection::Reflect<DOMString>(*this, u8"title").value_or(u8"");
  }

  void HTMLElement::Title(DOMString &&value) noexcept
  {
    Attributes::Reflection::Reflect<DOMString>(*this, u8"title", Krys::Move(value));
  }

  DOMString HTMLElement::Lang() const noexcept
  {
    return Attributes::Reflection::Reflect<DOMString>(*this, u8"lang").value_or(u8"");
  }

  void HTMLElement::Lang(DOMString &&value) noexcept
  {
    Attributes::Reflection::Reflect<DOMString>(*this, u8"lang", Krys::Move(value));
  }

  bool HTMLElement::Translate() const noexcept
  {
    auto value = Attributes::Reflection::Reflect<DOMString>(*this, u8"translate");
    auto state = Attributes::EnumeratedAttribute<"translate", HTMLElement>::ResolveState(value);
    return state == Attributes::AttributeState::Yes;
  }

  void HTMLElement::Translate(bool value) noexcept
  {
    Attributes::Reflection::Reflect<DOMString>(*this, u8"translate", value ? u8"yes" : u8"no");
  }

  DOMString HTMLElement::Dir() const noexcept
  {
    auto value = Attributes::Reflection::Reflect<DOMString>(*this, u8"dir");
    return Attributes::EnumeratedAttribute<"dir", HTMLElement>::ResolveCanonicalKeyword<DOMString>(
      Krys::Move(value));
  }

  void HTMLElement::Dir(DOMString &&value) noexcept
  {
    Attributes::Reflection::Reflect<DOMString>(*this, u8"dir", Krys::Move(value));
  }

  BoolOr<DOMString> HTMLElement::Hidden() const noexcept
  {
    auto value = Attributes::Reflection::Reflect<DOMString>(*this, u8"hidden");

    auto state = Attributes::EnumeratedAttribute<"hidden", HTMLElement>::ResolveState(value);
    if (state == Attributes::AttributeState::UntilFound)
    {
      return u8"until-found";
    }

    return state == Attributes::AttributeState::Hidden;
  }

  void HTMLElement::Hidden(DOMString &&value) noexcept
  {
    if (value == u8"until-found")
    {
      ElementAlgorithms::SetAttributeValue(*this, u8"hidden", u8"until-found");
    }
    else if (value.empty())
    {
      ElementAlgorithms::RemoveAttributeByName(u8"hidden", *this);
    }
    else
    {
      ElementAlgorithms::SetAttributeValue(*this, u8"hidden", u8"");
    }
  }

  void HTMLElement::Hidden(bool value) noexcept
  {
    if (value)
    {
      ElementAlgorithms::SetAttributeValue(*this, u8"hidden", u8"");
    }
    else
    {
      ElementAlgorithms::RemoveAttributeByName(u8"hidden", *this);
    }
  }

  void HTMLElement::Hidden(double value) noexcept
  {
    if (value == 0)
    {
      ElementAlgorithms::RemoveAttributeByName(u8"hidden", *this);
    }
    else if (std::isnan(value))
    {
      ElementAlgorithms::RemoveAttributeByName(u8"hidden", *this);
    }
    else
    {
      ElementAlgorithms::SetAttributeValue(*this, u8"hidden", u8"");
    }
  }

  bool HTMLElement::Inert() const noexcept
  {
    return Attributes::Reflection::Reflect<bool>(*this, u8"inert");
  }

  void HTMLElement::Inert(bool value) noexcept
  {
    Attributes::Reflection::Reflect<bool>(*this, u8"inert", Krys::Move(value));
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

  DOMString HTMLElement::AccessKey() const noexcept
  {
    return Attributes::Reflection::Reflect<DOMString>(*this, u8"accesskey").value_or(u8"");
  }

  void HTMLElement::AccessKey(DOMString &&value) noexcept
  {
    Attributes::Reflection::Reflect<DOMString>(*this, u8"accesskey", Krys::Move(value));
  }

  DOMString HTMLElement::AccessKeyLabel() const noexcept
  {
    // TODO(HTMLELEMENT, HTML): Implement AccessKeyLabel() method.
    return {};
  }

  bool HTMLElement::Draggable() const noexcept
  {
    auto value = Attributes::Reflection::Reflect<DOMString>(*this, u8"draggable");
    switch (Attributes::EnumeratedAttribute<"draggable", HTMLElement>::ResolveState(value))
    {
      case Attributes::AttributeState::True:  return true;
      case Attributes::AttributeState::False: return false;
      case Attributes::AttributeState::Auto:
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
    ElementAlgorithms::SetAttributeValue(*this, u8"draggable", value ? u8"true" : u8"false");
  }

  bool HTMLElement::SpellCheck() const noexcept
  {
    auto value = Attributes::Reflection::Reflect<DOMString>(*this, u8"spellcheck");
    switch (Attributes::EnumeratedAttribute<"spellcheck", HTMLElement>::ResolveState(value))
    {
      case Attributes::AttributeState::True:  return true;
      case Attributes::AttributeState::False: return false;
      case Attributes::AttributeState::Default:
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
    ElementAlgorithms::SetAttributeValue(*this, u8"spellcheck", value ? u8"true" : u8"false");
  }

  DOMString HTMLElement::WritingSuggestions() const noexcept
  {
    // TODO(HTMLELEMENT, WRITINGSUGGESTIONS, HTML): Implement WritingSuggestions() method.
    // Return this's computed writing suggestions value.
    return {};
  }

  void HTMLElement::WritingSuggestions(DOMString &&value) noexcept
  {
    Attributes::Reflection::ReflectSetter(*this, u8"writingsuggestions", Krys::Move(value));
  }

  DOMString HTMLElement::Autocapitalize() const noexcept
  {
    // TODO(HTMLELEMENT, AUTOCAPITALIZE, HTML): Implement Autocapitalize() method.
    // Let state be the own autocapitalization hint of this.
    // If state is Default, then return the empty string.
    // If state is None, then return "none".
    // If state is Sentences, then return "sentences".
    // Return the keyword value corresponding to state.
    return {};
  }

  void HTMLElement::Autocapitalize(DOMString &&value) noexcept
  {
    Attributes::Reflection::ReflectSetter(*this, u8"autocapitalize", Krys::Move(value));
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
    ElementAlgorithms::SetAttributeValue(*this, u8"autocorrect", value ? u8"on" : u8"off");
  }

  DOMString HTMLElement::InnerText() const noexcept
  {
    // TODO(HTMLELEMENT, HTML): Implement InnerText() setter method.
    return {};
  }

  void HTMLElement::InnerText(DOMString &&value) noexcept
  {
    // TODO(HTMLELEMENT, HTML): Implement InnerText() getter method.
  }

  DOMString HTMLElement::OuterText() const noexcept
  {
    // TODO(HTMLELEMENT, HTML): Implement OuterText() method.
    return {};
  }

  void HTMLElement::OuterText(DOMString &&value) noexcept
  {
    // TODO(HTMLELEMENT, HTML): Implement OuterText() getter method.
  }

  ExceptionOr<Ref<ElementInternals>> HTMLElement::AttachInternals() noexcept
  {
    // TODO(HTMLELEMENT, ELEMENTINTERNALS, HTML): Implement AttachInternals() method.
    return ExceptionCode::NotSupportedError;
  }

  uint32 HTMLElement::HeadingOffset() const noexcept
  {
    return Attributes::Reflection::Reflect<uint32, Attributes::ReflectRange<uint32>(0u, 8u)>(
      *this, u8"headingoffset");
  }

  void HTMLElement::HeadingOffset(uint32 value) noexcept
  {
    Attributes::Reflection::Reflect<uint32>(*this, u8"headingoffset", Krys::Move(value));
  }

  bool HTMLElement::HeadingReset() const noexcept
  {
    return Attributes::Reflection::Reflect<bool>(*this, u8"headingreset");
  }

  void HTMLElement::HeadingReset(bool value) noexcept
  {
    Attributes::Reflection::Reflect<bool>(*this, u8"headingreset", Krys::Move(value));
  }

#pragma endregion
}