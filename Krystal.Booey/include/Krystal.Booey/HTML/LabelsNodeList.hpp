#pragma once

#include "Krystal.Booey/DOM/NodeList.hpp"
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
  class LabelsNodeList final : public dom::NodeList
  {
  private:
    Ref<HTMLElement> _ownerNode;

  public:
    explicit LabelsNodeList(HTMLElement &ownerNode) noexcept;

    KRYS_NODISCARD RefPtr<dom::Node> Item(size_t index) noexcept override;
    KRYS_NODISCARD RefPtr<const dom::Node> Item(size_t index) const noexcept override;
    KRYS_NODISCARD RefPtr<dom::Node> operator[](size_t index) noexcept override;
    KRYS_NODISCARD RefPtr<const dom::Node> operator[](size_t index) const noexcept override;
    KRYS_NODISCARD size_t Length() const noexcept override;

    KRYS_NODISCARD bool ElementMatches(const dom::Element &testNode) const noexcept;

  private:
    KRYS_NODISCARD List<Ref<dom::Element>> Elements() const noexcept;
  };
}
