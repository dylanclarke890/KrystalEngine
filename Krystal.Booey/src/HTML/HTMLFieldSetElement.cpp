#include "Krystal.Booey/HTML/HTMLFieldSetElement.hpp"
#include "Krystal.Booey/HTML/Attributes/Reflection.hpp"
#include "Krystal.Booey/Rendering/RenderElement.hpp"

namespace krys::boo::html
{
  HTMLFieldSetElement::HTMLFieldSetElement(dom::Document &document) noexcept
      : HTMLElement(document, HTMLElementInterface::FieldSet)
  {
  }

  RenderPtr<RenderElement> HTMLFieldSetElement::CreateElementRenderer(RenderStyle &&style) noexcept
  {
    // Fieldsets should make a block flow if display: inline or table types are set.
    return RenderElement::CreateFor(*this, krys::move(style),
                                    {RenderElement::ConstructBlockLevelRendererFor::Inline,
                                     RenderElement::ConstructBlockLevelRendererFor::TableOrTablePart});
  }

#pragma region HTMLFieldSetElement

  bool HTMLFieldSetElement::Disabled() const noexcept
  {
    return Reflection::Reflect<bool>(*this, u8"disabled");
  }

  void HTMLFieldSetElement::Disabled(bool value) noexcept
  {
    Reflection::Reflect<bool>(*this, u8"disabled", krys::move(value));
  }

  dom::DOMString HTMLFieldSetElement::Name() const noexcept
  {
    return Reflection::Reflect<dom::DOMString>(*this, u8"name").value_or(u8"");
  }

  void HTMLFieldSetElement::Name(dom::DOMString &&value) noexcept
  {
    return Reflection::Reflect<dom::DOMString>(*this, u8"name", krys::move(value));
  }

#pragma endregion
}