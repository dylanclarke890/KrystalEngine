#pragma once

#include "Krystal.HTML/DOMString.hpp"
#include "Krystal.HTML/Node/Enums/NodeCollectionLiveness.hpp"
#include "Krystal.HTML/Utils/SmallNodeList.hpp"
#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Lib/Mixins/CanMakeWeakPtr.hpp"
#include "Krystal.Lib/Mixins/RefCounted.hpp"
#include "Krystal.Lib/Pointers/RefPtr.hpp"
#include "Krystal.Lib/Types/Func.hpp"

namespace Krys::HTML
{
  class ContainerNode;
  class Element;

  using LiveHTMLCollectionFilterFunc = Func<bool(const Element &)>;

  class HTMLCollection : public RefCounted<HTMLCollection>, public CanMakeWeakPtr<HTMLCollection>
  {
  private:
    NodeCollectionLiveness _liveness;

  protected:
    explicit HTMLCollection(NodeCollectionLiveness liveness) noexcept : _liveness(liveness)
    {
    }

  public:
    virtual ~HTMLCollection() noexcept = default;

    KRYS_NODISCARD virtual RawPtr<Element> Item(size_t index) noexcept = 0;
    KRYS_NODISCARD virtual RawPtr<const Element> Item(size_t index) const noexcept = 0;

    KRYS_NODISCARD virtual RawPtr<Element> operator[](size_t index) noexcept = 0;
    KRYS_NODISCARD virtual RawPtr<const Element> operator[](size_t index) const noexcept = 0;

    KRYS_NODISCARD virtual RawPtr<Element> NamedItem(DOMStringView name) noexcept = 0;
    KRYS_NODISCARD virtual RawPtr<const Element> NamedItem(DOMStringView name) const noexcept = 0;

    KRYS_NODISCARD virtual RawPtr<Element> operator[](DOMStringView name) noexcept = 0;
    KRYS_NODISCARD virtual RawPtr<const Element> operator[](DOMStringView name) const noexcept = 0;

    KRYS_NODISCARD virtual size_t Length() const noexcept = 0;
  };

  class LiveHTMLCollection : public HTMLCollection
  {
  private:
    WeakRef<ContainerNode> _root;
    LiveHTMLCollectionFilterFunc _filter;

  public:
    LiveHTMLCollection(WeakRef<ContainerNode> &&root, LiveHTMLCollectionFilterFunc &&filter) noexcept;

    KRYS_NODISCARD RawPtr<Element> Item(size_t index) noexcept override;
    KRYS_NODISCARD RawPtr<const Element> Item(size_t index) const noexcept override;

    KRYS_NODISCARD RawPtr<Element> operator[](size_t index) noexcept override;
    KRYS_NODISCARD RawPtr<const Element> operator[](size_t index) const noexcept override;

    KRYS_NODISCARD RawPtr<Element> NamedItem(DOMStringView name) noexcept override;
    KRYS_NODISCARD RawPtr<const Element> NamedItem(DOMStringView name) const noexcept override;

    KRYS_NODISCARD RawPtr<Element> operator[](DOMStringView name) noexcept override;
    KRYS_NODISCARD RawPtr<const Element> operator[](DOMStringView name) const noexcept override;

    KRYS_NODISCARD size_t Length() const noexcept override;
  };

  class StaticHTMLCollection : public HTMLCollection
  {
  private:
    SmallElementList _elements;

  public:
    explicit StaticHTMLCollection(SmallElementList &&elements) noexcept;

    KRYS_NODISCARD RawPtr<Element> Item(size_t index) noexcept override;
    KRYS_NODISCARD RawPtr<const Element> Item(size_t index) const noexcept override;

    KRYS_NODISCARD RawPtr<Element> operator[](size_t index) noexcept override;
    KRYS_NODISCARD RawPtr<const Element> operator[](size_t index) const noexcept override;

    KRYS_NODISCARD RawPtr<Element> NamedItem(DOMStringView name) noexcept override;
    KRYS_NODISCARD RawPtr<const Element> NamedItem(DOMStringView name) const noexcept override;

    KRYS_NODISCARD RawPtr<Element> operator[](DOMStringView name) noexcept override;
    KRYS_NODISCARD RawPtr<const Element> operator[](DOMStringView name) const noexcept override;

    KRYS_NODISCARD size_t Length() const noexcept override;
  };
}