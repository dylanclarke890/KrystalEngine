#pragma once

#include "Krystal.HTML/DOMString.hpp"

namespace Krys::HTML
{
  /// @see https://dom.spec.whatwg.org/#concept-element-qualified-name
  struct QualifiedName
  {
    DOMStringAtom NamespaceURI;
    DOMStringAtom Prefix;
    DOMStringAtom LocalName;

    QualifiedName() noexcept = default;

    QualifiedName(DOMStringAtom namespaceURI, DOMStringAtom prefix, DOMStringAtom localName) noexcept
        : NamespaceURI(namespaceURI), Prefix(prefix), LocalName(localName)
    {
    }

    KRYS_NODISCARD friend bool operator==(const QualifiedName &a, const QualifiedName &b) noexcept
    {
      return a.NamespaceURI == b.NamespaceURI && a.Prefix == b.Prefix && a.LocalName == b.LocalName;
    }

    KRYS_NODISCARD friend bool operator!=(const QualifiedName &a, const QualifiedName &b) noexcept
    {
      return !(a == b);
    }

    /// @see https://dom.spec.whatwg.org/#concept-attribute-qualified-name
    KRYS_NODISCARD DOMString Name() const noexcept
    {
      if (Prefix.View().empty())
      {
        return DOMString {LocalName.View()};
      }
      else
      {
        return DOMString {Prefix.View()} + u8":" + DOMString {LocalName.View()};
      }
    }
  };
}