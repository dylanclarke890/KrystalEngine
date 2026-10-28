#pragma once

#include "Krystal.Booey/MathML/MathMLPresentationElement.hpp"

namespace krys::boo::mathml
{
  /// @brief An element laid out as a row of its children: `<mrow>`, `<merror>`, `<mphantom>`, `<mstyle>`,
  /// `<semantics>` and `<maction>` (which show their first child), and the base of the elements with
  /// more specific layouts.
  class MathMLRowElement : public MathMLPresentationElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(MathMLRowElement);

  public:
    explicit MathMLRowElement(dom::Document &document,
                              MathMLElementInterface interface = MathMLElementInterface::Row,
                              dom::NodeFlags nodeFlags = dom::NodeFlags::None) noexcept;

  protected:
    void OnChildrenChanged() noexcept override;

    // FIXME: The element is made a RenderMathMLRow, once there are MathML renderers.
  };
}
