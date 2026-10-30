#include "Krystal.Booey/MathML/MathMLScriptsElement.hpp"
#include "Krystal.Booey/Rendering/MathML/RenderMathMLScripts.hpp"
#include "Krystal.Booey/Rendering/RenderPtr.hpp"

namespace krys::boo::mathml
{
  RenderPtr<RenderElement> MathMLScriptsElement::CreateElementRenderer(RenderStyle &&style) noexcept
  {
    krys_debug_assert(HasTagName(html::TagName::msub) || HasTagName(html::TagName::msup)
                      || HasTagName(html::TagName::msubsup) || HasTagName(html::TagName::mmultiscripts));
    return CreateRenderer<RenderMathMLScripts>(RenderObject::Type::MathMLScripts, *this, krys::move(style));
  }
}
