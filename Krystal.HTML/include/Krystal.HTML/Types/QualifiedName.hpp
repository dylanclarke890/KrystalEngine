#pragma once

#include "Krystal.HTML/HTML/Enums/Namespace.hpp"
#include "Krystal.HTML/HTML/Enums/TagName.hpp"
#include "Krystal.HTML/Types/DOMString.hpp"
#include <cassert>

namespace Krys::HTML
{
  /// @see https://dom.spec.whatwg.org/#concept-element-qualified-name
  class QualifiedName
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
    QualifiedName(DOMStringAtom namespaceURI, DOMStringAtom prefix, DOMStringAtom localName) noexcept
        : _namespaceURI(namespaceURI), _namespacePrefix(prefix), _localName(localName)
    {
      assert(_namespaceURI == DOMStringAtom::Null() || _namespaceURI != DOMStringAtom::Empty());
      assert(_namespacePrefix == DOMStringAtom::Null() || _namespacePrefix != DOMStringAtom::Empty());
      assert(_localName != DOMStringAtom::Null());
    }

    KRYS_NODISCARD const DOMStringAtom &NamespaceURI() const noexcept
    {
      return _namespaceURI;
    }

    KRYS_NODISCARD const DOMStringAtom &NamespacePrefix() const noexcept
    {
      return _namespacePrefix;
    }

    KRYS_NODISCARD const DOMStringAtom &LocalName() const noexcept
    {
      return _localName;
    }

    KRYS_NODISCARD const DOMStringAtom &LocalNameLower() const noexcept
    {
      return _localNameLower;
    }

    KRYS_NODISCARD const DOMStringAtom &LocalNameUpper() const noexcept
    {
      return _localNameLower;
    }

    KRYS_NODISCARD TagName TagName() const noexcept
    {
      return _tagName;
    }

    KRYS_NODISCARD Namespace Namespace() const noexcept
    {
      return _namespace;
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

    KRYS_NODISCARD friend bool operator==(const QualifiedName &a, const QualifiedName &b) noexcept
    {
      return a._namespaceURI == b._namespaceURI && a._namespacePrefix == b._namespacePrefix
             && a._localName == b._localName;
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