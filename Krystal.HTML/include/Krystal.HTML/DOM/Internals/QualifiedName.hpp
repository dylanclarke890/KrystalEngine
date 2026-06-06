#pragma once

#include "Krystal.HTML/Types/DOMString.hpp"
#include "Krystal.HTML/Types/DOMString.hpp"
#include <cassert>

namespace Krys::HTML
{
  /// @see https://dom.spec.whatwg.org/#concept-element-qualified-name
  struct QualifiedName
  {
    DOMStringAtom NamespaceURI {DOMStringAtom::Null()};
    DOMStringAtom Prefix {DOMStringAtom::Null()};
    DOMStringAtom LocalName {DOMStringAtom::Null()};

    QualifiedName() noexcept = default;

    QualifiedName(DOMStringAtom namespaceURI, DOMStringAtom prefix, DOMStringAtom localName) noexcept
        : NamespaceURI(namespaceURI), Prefix(prefix), LocalName(localName)
    {
      assert(NamespaceURI == DOMStringAtom::Null() || NamespaceURI != DOMStringAtom::Empty());
      assert(Prefix == DOMStringAtom::Null() || Prefix != DOMStringAtom::Empty());
      assert(LocalName != DOMStringAtom::Null() && Prefix != DOMStringAtom::Empty());
    }

    /// @see https://dom.spec.whatwg.org/#concept-attribute-qualified-name
    KRYS_NODISCARD DOMString Name() const noexcept
    {
      if (Prefix == DOMStringAtom::Null())
      {
        return LocalName == DOMStringAtom::Null() ? DOMString {} : DOMString {LocalName.View()};
      }
      else
      {
        return DOMString {Prefix.View()} + u8":" + DOMString {LocalName.View()};
      }
    }

    KRYS_NODISCARD friend bool operator==(const QualifiedName &a, const QualifiedName &b) noexcept
    {
      return a.NamespaceURI == b.NamespaceURI && a.Prefix == b.Prefix && a.LocalName == b.LocalName;
    }

    KRYS_NODISCARD friend bool operator!=(const QualifiedName &a, const QualifiedName &b) noexcept
    {
      return !(a == b);
    }
  };
}