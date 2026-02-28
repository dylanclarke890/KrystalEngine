#pragma once

#include "Krystal.HTML/DOMString.hpp"
#include "Krystal.HTML/Utils/SmallNodeList.hpp"
#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Lib/Mixins/CanMakeWeakPtr.hpp"
#include "Krystal.Lib/Mixins/RefCounted.hpp"
#include "Krystal.Lib/Pointers/RefPtr.hpp"

namespace Krys::HTML
{
  class ContainerNode;
  class Element;
  class ParentNodeRareData;

  class HTMLCollection : public RefCounted<HTMLCollection>, public CanMakeWeakPtr<HTMLCollection>
  {
    friend class ParentNodeRareData;

  private:
    Ref<const ContainerNode> _owner;
    mutable SmallElementList _elements;
    mutable bool _invalid;

  public:
    explicit HTMLCollection(const ContainerNode &owner) noexcept;

    KRYS_NODISCARD RawPtr<Element> Item(size_t index) const noexcept;

    KRYS_NODISCARD RawPtr<Element> NamedItem(const DOMString &name) const noexcept;

    KRYS_NODISCARD RawPtr<Element> operator[](const DOMString &name) const noexcept;

    KRYS_NODISCARD size_t Length() const noexcept
    {
      return _elements.size();
    }

  private:
    void Invalidate() noexcept
    {
      _invalid = true;
    }

    void BuildCollection() const noexcept;
  };
}