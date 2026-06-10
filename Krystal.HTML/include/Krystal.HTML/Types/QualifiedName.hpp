#pragma once

#include "Krystal.HTML/HTML/Enums/Namespace.hpp"
#include "Krystal.HTML/HTML/Enums/TagName.hpp"
#include "Krystal.HTML/Types/DOMString.hpp"
#include <cassert>

namespace Krys::HTML
{
  /// @see https://dom.spec.whatwg.org/#concept-element-qualified-name
  class QName
  {
  private:
    uint64 _hash {0ull};
    TagName _tagName {TagName::Unknown};
    Namespace _namespace {Namespace::Unknown};
    DOMStringAtom _namespaceURI {DOMStringAtom::Null()};
    DOMStringAtom _namespacePrefix {DOMStringAtom::Null()};
    DOMStringAtom _localName {DOMStringAtom::Null()};
    DOMStringAtom _localNameLower {DOMStringAtom::Null()};
    DOMStringAtom _localNameUpper {DOMStringAtom::Null()};

  public:
    QName(DOMStringAtom namespaceURI, DOMStringAtom prefix, DOMStringAtom localName) noexcept
        : _namespaceURI(namespaceURI), _namespacePrefix(prefix), _localName(localName)
    {
      assert(_namespaceURI == DOMStringAtom::Null() || _namespaceURI != DOMStringAtom::Empty());
      assert(_namespacePrefix == DOMStringAtom::Null() || _namespacePrefix != DOMStringAtom::Empty());
      assert(_localName != DOMStringAtom::Null());
    }

    /// @see https://dom.spec.whatwg.org/#concept-attribute-qualified-name
    KRYS_NODISCARD DOMString Name() const noexcept
    {
      if (_namespacePrefix == DOMStringAtom::Null())
      {
        return DOMString(_localName.View());
      }
      else
      {
        return DOMString {_namespacePrefix.View()} + u8":" + DOMString {_localName.View()};
      }
    }

    
    KRYS_NODISCARD friend bool operator==(const QName &a, const QName &b) noexcept
    {
      return a._namespaceURI == b._namespaceURI && a._namespacePrefix == b._namespacePrefix
             && a._localName == b._localName;
    }

    KRYS_NODISCARD friend bool operator!=(const QName &a, const QName &b) noexcept
    {
      return !(a == b);
    }
  };

  /// @see https://dom.spec.whatwg.org/#concept-element-qualified-name
  struct QualifiedName
  {
    DOMStringAtom NamespaceURI {DOMStringAtom::Null()};
    DOMStringAtom Prefix {DOMStringAtom::Null()};
    DOMStringAtom LocalName {DOMStringAtom::Null()};

    QualifiedName(DOMStringAtom namespaceURI, DOMStringAtom prefix, DOMStringAtom localName) noexcept
        : NamespaceURI(namespaceURI), Prefix(prefix), LocalName(localName)
    {
      assert(NamespaceURI == DOMStringAtom::Null() || NamespaceURI != DOMStringAtom::Empty());
      assert(Prefix == DOMStringAtom::Null() || Prefix != DOMStringAtom::Empty());
      assert(LocalName != DOMStringAtom::Null());
    }

    /// @see https://dom.spec.whatwg.org/#concept-attribute-qualified-name
    KRYS_NODISCARD DOMString Name() const noexcept
    {
      if (Prefix == DOMStringAtom::Null())
      {
        return DOMString(LocalName.View());
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

  KRYS_NODISCARD inline QualifiedName NullQualifiedName() noexcept
  {
    return QualifiedName(DOMStringAtom::Null(), DOMStringAtom::Null(), DOMStringAtom::Empty());
  }
}