#include "Krystal.Booey/MathML/MathMLRootElement.hpp"
#include "Krystal.Booey/Rendering/MathML/RenderMathMLRoot.hpp"
#include "Krystal.Booey/Rendering/RenderPtr.hpp"

namespace krys::boo::mathml
{
  RenderPtr<RenderElement> MathMLRootElement::CreateElementRenderer(RenderStyle &&style) noexcept
  {
    krys_debug_assert(HasTagName(html::TagName::msqrt) || HasTagName(html::TagName::mroot));
    return CreateRenderer<RenderMathMLRoot>(*this, krys::move(style));
  }

  void MathMLRootElement::OnChildrenChanged() noexcept
  {
    MathMLRowElement::OnChildrenChanged();

    if (auto *rootRenderer = DynamicDowncast<RenderMathMLRoot>(Renderer()))
    {
      rootRenderer->ResetRadicalOperator();
      rootRenderer->SetNeedsLayoutAndPreferredWidthsUpdate();
    }
  }
}
