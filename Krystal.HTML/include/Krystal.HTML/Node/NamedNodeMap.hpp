#pragma once

#include "Krystal.HTML/DOMString.hpp"
#include "Krystal.HTML/Utils/ExceptionOr.hpp"
#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Lib/Mixins/RefCounted.hpp"
#include "Krystal.Lib/Pointers/RawPtr.hpp"
#include "Krystal.Lib/Pointers/RefPtr.hpp"
#include "Krystal.Lib/Pointers/WeakPtr.hpp"
#include "Krystal.Lib/Types/Numeric.hpp"

namespace Krys::HTML
{
  class Attr;
  class Element;

  class NamedNodeMap : public RefCounted<NamedNodeMap>
  {
  private:
    WeakRef<Element> _associatedElement;

  public:
    explicit NamedNodeMap(Element &associatedElement) noexcept
        : _associatedElement(CreateWeakRef(associatedElement))
    {
    }

    KRYS_NODISCARD size_t Length() noexcept;

    KRYS_NODISCARD RawPtr<Attr> Item(size_t index) noexcept;

    KRYS_NODISCARD RawPtr<Attr> NamedItem(const DOMString &qualifiedName) noexcept;

    KRYS_NODISCARD RawPtr<Attr> NamedItemNS(const DOMString &attrNamespace,
                                            const DOMString &localName) noexcept;

    RawPtr<Attr> SetNamedItem(Attr &attr) noexcept;

    RawPtr<Attr> SetNamedItemNS(Attr &attr) noexcept;

    ExceptionOr<Ref<Attr>> RemoveNamedItem(const DOMString &qualifiedName) noexcept;

    ExceptionOr<Ref<Attr>> RemoveNamedItemNS(const DOMString &attrNamespace,
                                             const DOMString &localName) noexcept;
  };
}