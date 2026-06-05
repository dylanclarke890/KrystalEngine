#pragma once

#include "Krystal.HTML/DOM/HTMLCollection.hpp"
#include "Krystal.Lib/Types/Func.hpp"

namespace Krys::HTML
{
  class ContainerNode;

  using LiveHTMLCollectionFilterFunc = Func<bool(const Element &)>;

  class LiveHTMLCollection : public HTMLCollection
  {
  private:
    Ref<ContainerNode> _root;
    LiveHTMLCollectionFilterFunc _filter;

  public:
    LiveHTMLCollection(ContainerNode &root, LiveHTMLCollectionFilterFunc &&filter) noexcept;

    KRYS_NODISCARD RefPtr<Element> Item(size_t index) noexcept override;
    KRYS_NODISCARD RefPtr<const Element> Item(size_t index) const noexcept override;

    KRYS_NODISCARD RefPtr<Element> operator[](size_t index) noexcept override;
    KRYS_NODISCARD RefPtr<const Element> operator[](size_t index) const noexcept override;

    KRYS_NODISCARD RefPtr<Element> NamedItem(DOMStringView name) noexcept override;
    KRYS_NODISCARD RefPtr<const Element> NamedItem(DOMStringView name) const noexcept override;

    KRYS_NODISCARD RefPtr<Element> operator[](DOMStringView name) noexcept override;
    KRYS_NODISCARD RefPtr<const Element> operator[](DOMStringView name) const noexcept override;

    KRYS_NODISCARD size_t Length() const noexcept override;

    KRYS_NODISCARD List<DOMString> SupportedPropertyNames() const noexcept override;
  };
}