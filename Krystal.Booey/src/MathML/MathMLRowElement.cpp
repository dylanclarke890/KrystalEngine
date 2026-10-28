#include "Krystal.Booey/MathML/MathMLRowElement.hpp"
#include "Krystal.Booey/MathML/MathMLOperatorElement.hpp"

namespace krys::boo::mathml
{
  MathMLRowElement::MathMLRowElement(dom::Document &document, MathMLElementInterface interface,
                                     dom::NodeFlags nodeFlags) noexcept
      : MathMLPresentationElement(document, interface, nodeFlags)
  {
  }

  void MathMLRowElement::OnChildrenChanged() noexcept
  {
    // FIXME: Avoid this invalidation for valid MathMLFractionElement/MathMLScriptsElement.
    // See https://bugs.webkit.org/show_bug.cgi?id=276828.
    for (auto *child = FirstChild(); child; child = child->NextSibling())
    {
      if (auto *mo = DynamicDowncast<MathMLOperatorElement>(child))
      {
        mo->SetOperatorFormDirty();
      }
    }

    MathMLPresentationElement::OnChildrenChanged();
  }
}
