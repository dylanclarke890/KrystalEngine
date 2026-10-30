#pragma once

#include "Krystal.Booey/DOM/NodeList.hpp"
#include "Krystal.Booey/DOM/Types/DOMString.hpp"
#include "Krystal.Core/Types/List.hpp"

namespace krys::boo::dom
{
  class ContainerNode;
  class Element;
}

namespace krys::boo::html
{
  /// @brief The listed elements of a form with an ID or name, as its named getter returns them when there are
  /// several.
  /// @see https://html.spec.whatwg.org/#radionodelist
  class RadioNodeList final : public dom::NodeList
  {
  private:
    Ref<dom::ContainerNode> _ownerNode;
    dom::DOMString _name;
    bool _isRootedAtTreeScope;

  public:
    RadioNodeList(dom::ContainerNode &ownerNode, dom::DOMStringView name) noexcept;

    KRYS_NODISCARD RefPtr<dom::Node> Item(size_t index) noexcept override;
    KRYS_NODISCARD RefPtr<const dom::Node> Item(size_t index) const noexcept override;
    KRYS_NODISCARD RefPtr<dom::Node> operator[](size_t index) noexcept override;
    KRYS_NODISCARD RefPtr<const dom::Node> operator[](size_t index) const noexcept override;
    KRYS_NODISCARD size_t Length() const noexcept override;

    /// @see https://html.spec.whatwg.org/#dom-radionodelist-value
    KRYS_NODISCARD dom::DOMString Value() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-radionodelist-value
    void SetValue(dom::DOMStringView value) noexcept;

    KRYS_NODISCARD bool ElementMatches(const dom::Element &element) const noexcept;

  private:
    KRYS_NODISCARD List<Ref<dom::Element>> Elements() const noexcept;
  };
}
