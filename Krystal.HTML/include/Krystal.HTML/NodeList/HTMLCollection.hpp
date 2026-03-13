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
    Ref<ContainerNode> _owner;
    SmallElementList _elements;
    bool _invalid;

  public:
    explicit HTMLCollection(ContainerNode &owner) noexcept;

    KRYS_NODISCARD RawPtr<Element> Item(size_t index) noexcept;

    KRYS_NODISCARD RawPtr<Element> NamedItem(const DOMString &name) noexcept;

    KRYS_NODISCARD RawPtr<Element> operator[](const DOMString &name) noexcept;

    KRYS_NODISCARD size_t Length() noexcept
    {
      return _elements.size();
    }

  private:
    void Invalidate() noexcept
    {
      _invalid = true;
    }

    void BuildCollection() noexcept;
  };
}