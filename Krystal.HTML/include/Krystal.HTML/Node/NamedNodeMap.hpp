#pragma once

#include "Krystal.HTML/Types/DOMStringAtom.hpp"
#include "Krystal.HTML/Types/ExceptionOr.hpp"
#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Lib/Pointers/RawPtr.hpp"
#include "Krystal.Lib/Pointers/RefPtr.hpp"
#include "Krystal.Lib/Pointers/WeakPtr.hpp"
#include "Krystal.Lib/Types/Numeric.hpp"

namespace Krys::HTML
{
  class Attr;
  class Element;

  class NamedNodeMap
  {
  private:
    CheckedRef<Element> _associatedElement;

  public:
    explicit NamedNodeMap(Element &associatedElement) noexcept
        : _associatedElement(ShareCheckedRef(associatedElement))
    {
    }

    KRYS_NODISCARD size_t Length() noexcept;

    KRYS_NODISCARD RefPtr<Attr> Item(size_t index) noexcept;

    KRYS_NODISCARD RefPtr<Attr> GetNamedItem(DOMStringAtom qualifiedName) noexcept;

    KRYS_NODISCARD RefPtr<Attr> GetNamedItemNS(DOMStringAtom attrNamespace, DOMStringAtom localName) noexcept;

    ExceptionOr<RefPtr<Attr>> SetNamedItem(Attr &attr) noexcept;

    ExceptionOr<RefPtr<Attr>> SetNamedItemNS(Attr &attr) noexcept;

    ExceptionOr<Ref<Attr>> RemoveNamedItem(DOMStringAtom qualifiedName) noexcept;

    ExceptionOr<Ref<Attr>> RemoveNamedItemNS(DOMStringAtom attrNamespace, DOMStringAtom localName) noexcept;

    KRYS_NODISCARD RefPtr<Attr> operator[](size_t index) noexcept;
    KRYS_NODISCARD RefPtr<Attr> operator[](DOMStringAtom qualifiedName) noexcept;
  };
}