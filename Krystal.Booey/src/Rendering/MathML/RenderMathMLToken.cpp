#include "Krystal.Booey/Rendering/MathML/RenderMathMLToken.hpp"
#include "Krystal.Booey/MathML/MathMLTokenElement.hpp"

namespace krys::boo
{
  RenderMathMLToken::RenderMathMLToken(Type type, mathml::MathMLTokenElement &element,
                                       RenderStyle &&style) noexcept
      : RenderMathMLBlock(type, element, krys::move(style))
  {
  }

  RenderMathMLToken::RenderMathMLToken(Type type, dom::Document &document, RenderStyle &&style) noexcept
      : RenderMathMLBlock(type, document, krys::move(style))
  {
  }

  RenderMathMLToken::~RenderMathMLToken() noexcept = default;

  mathml::MathMLTokenElement &RenderMathMLToken::GetElement() const noexcept
  {
    return static_cast<mathml::MathMLTokenElement &>(NodeForNonAnonymous());
  }

  void RenderMathMLToken::UpdateTokenContent() noexcept
  {
    RenderMathMLBlock::UpdateFromElement();
    SetNeedsLayoutAndPreferredWidthsUpdate();
  }

  void RenderMathMLToken::UpdateFromElement() noexcept
  {
    RenderMathMLBlock::UpdateFromElement();
    SetNeedsLayoutAndPreferredWidthsUpdate();
  }
}
