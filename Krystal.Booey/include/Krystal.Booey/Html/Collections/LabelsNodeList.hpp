#pragma once

#include "Krystal.Booey/Dom/Collections/LiveNodeList.hpp"

namespace krys::boo
{
  class Element;
  class HTMLElement;
}

namespace krys::boo::html
{
  /// @brief The labels of a labelable element: the `label` elements in its tree whose labeled control it is,
  /// in tree order.
  /// @see https://html.spec.whatwg.org/multipage/forms.html#dom-lfe-labels
  class LabelsNodeList final : public LiveNodeList
  {
  private:
    Ref<HTMLElement> _ownerNode;

  public:
    explicit LabelsNodeList(HTMLElement &ownerNode) noexcept;

    /// @brief Whether @p testNode is a label whose labeled control is the list's element.
    KRYS_NODISCARD bool ElementMatches(const Element &testNode) const noexcept;
  };
}
