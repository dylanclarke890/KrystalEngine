#include "Krystal.Booey/HTML/HTMLWBRElement.hpp"
#include "Krystal.Booey/Rendering/RenderLineBreak.hpp"

namespace krys::boo::html
{
  HTMLWBRElement::HTMLWBRElement(dom::Document &document) noexcept
      : HTMLElement(document, HTMLElementInterface::WBR)
  {
  }

  RenderLineBreak *HTMLWBRElement::Renderer() const noexcept
  {
    return Downcast<RenderLineBreak>(HTMLElement::Renderer());
  }

  RenderPtr<RenderElement> HTMLWBRElement::CreateElementRenderer(RenderStyle &&style) noexcept
  {
    return CreateRenderer<RenderLineBreak>(*this, krys::move(style));
  }
}
