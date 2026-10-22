#pragma once

#include "Krystal.Booey/DOM/HTMLCollection.hpp"
#include "Krystal.Core/Types/Variant.hpp"

namespace krys::boo::html
{
  class HTMLElement;
  class HTMLFormElement;
  class RadioNodeList;

  /// @brief A form's `elements`: its listed elements, wherever they are in the tree.
  /// @see https://html.spec.whatwg.org/#htmlformcontrolscollection
  class HTMLFormControlsCollection final : public dom::HTMLCollection
  {
  private:
    Ref<HTMLFormElement> _ownerNode;

  public:
    explicit HTMLFormControlsCollection(HTMLFormElement &ownerNode) noexcept;

    KRYS_NODISCARD RefPtr<dom::Element> Item(size_t index) noexcept override;
    KRYS_NODISCARD RefPtr<const dom::Element> Item(size_t index) const noexcept override;
    KRYS_NODISCARD RefPtr<dom::Element> operator[](size_t index) noexcept override;
    KRYS_NODISCARD RefPtr<const dom::Element> operator[](size_t index) const noexcept override;
    KRYS_NODISCARD RefPtr<dom::Element> NamedItem(dom::DOMStringView name) noexcept override;
    KRYS_NODISCARD RefPtr<const dom::Element> NamedItem(dom::DOMStringView name) const noexcept override;
    KRYS_NODISCARD RefPtr<dom::Element> operator[](dom::DOMStringView name) noexcept override;
    KRYS_NODISCARD RefPtr<const dom::Element> operator[](dom::DOMStringView name) const noexcept override;
    KRYS_NODISCARD size_t Length() const noexcept override;

    /// @brief The elements whose ID or name is @p name: those by ID first, then those by name.
    KRYS_NODISCARD List<Ref<dom::Element>> NamedItems(dom::DOMStringView name) const noexcept;

    /// @brief The element whose ID or name is @p name, or a RadioNodeList of them if there are several.
    /// @see https://html.spec.whatwg.org/#dom-htmlformcontrolscollection-nameditem
    KRYS_NODISCARD Maybe<Variant<RefPtr<RadioNodeList>, RefPtr<dom::Element>>>
      NamedItemOrItems(dom::DOMStringView name) const noexcept;

    KRYS_NODISCARD HTMLFormElement &OwnerNode() const noexcept
    {
      return *_ownerNode;
    }

  protected:
    KRYS_NODISCARD List<dom::DOMString> SupportedPropertyNames() const noexcept override;

  private:
    /// @brief The enumeratable listed elements, in order.
    KRYS_NODISCARD List<Ref<HTMLElement>> Elements() const noexcept;
  };
}
