#include "Krystal.Booey/MathML/MathMLElement.hpp"

namespace krys::boo::mathml
{
  MathMLElement::MathMLElement(dom::Document &document) noexcept
      : StyledElement(document, dom::NullQualifiedName(), dom::NodeFlags::IsMathMLElement)
  {
  }
}
