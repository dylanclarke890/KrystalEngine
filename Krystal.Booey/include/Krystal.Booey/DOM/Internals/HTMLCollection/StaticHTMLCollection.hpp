#pragma once

#include "Krystal.Booey/DOM/HTMLCollection.hpp"
#include "Krystal.Booey/DOM/Types/SmallNodeList.hpp"

namespace krys::boo::dom
{
  class Element;

  class StaticHTMLCollection : public HTMLCollection
  {
  private:
    SmallElementList _elements;

  public:
    explicit StaticHTMLCollection(SmallElementList &&elements) noexcept;

    KRYS_NODISCARD RefPtr<Element> Item(size_t index) noexcept override;
    KRYS_NODISCARD RefPtr<const Element> Item(size_t index) const noexcept override;

    KRYS_NODISCARD RefPtr<Element> operator[](size_t index) noexcept override;
    KRYS_NODISCARD RefPtr<const Element> operator[](size_t index) const noexcept override;

    KRYS_NODISCARD RefPtr<Element> NamedItem(DOMStringView name) noexcept override;
    KRYS_NODISCARD RefPtr<const Element> NamedItem(DOMStringView name) const noexcept override;

    KRYS_NODISCARD RefPtr<Element> operator[](DOMStringView name) noexcept override;
    KRYS_NODISCARD RefPtr<const Element> operator[](DOMStringView name) const noexcept override;

    KRYS_NODISCARD size_t Length() const noexcept override;

  protected:
    KRYS_NODISCARD List<DOMString> SupportedPropertyNames() const noexcept override;
  };
}