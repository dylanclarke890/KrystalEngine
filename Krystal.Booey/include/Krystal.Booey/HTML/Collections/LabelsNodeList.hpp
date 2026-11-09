#pragma once

#include "Krystal.Booey/DOM/Collections/LiveNodeList.hpp"
#include "Krystal.Core/Types/List.hpp"

namespace krys::boo::dom
{
  class Element;
}

namespace krys::boo::html
{
  class HTMLElement;

  /// @brief The labels of a labelable element: the `label` elements in its tree whose labeled control it is,
  /// in tree order.
  /// @see https://html.spec.whatwg.org/#dom-lfe-labels
  class LabelsNodeList final : public dom::LiveNodeList
  {
  private:
    Ref<HTMLElement> _ownerNode;

  public:
    explicit LabelsNodeList(HTMLElement &ownerNode) noexcept;

    KRYS_NODISCARD bool ElementMatches(const dom::Element &testNode) const noexcept;
  };
}
