#include "Krystal.Booey/MathML/Elements/MathMLMathElement.hpp"
#include "Krystal.Booey/Rendering/MathML/RenderMathMLMath.hpp"

namespace krys::boo::mathml
{
  RenderPtr<RenderElement> MathMLMathElement::CreateElementRenderer(RenderStyle &&style) noexcept
  {
    return CreateRenderer<RenderMathMLMath>(*this, krys::move(style));
  }
}
