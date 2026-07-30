#pragma once

#include "Krystal.HTML/DOM/Types/DOMString.hpp"
#include "Krystal.HTML/HTML/Enums/Namespace.hpp"
#include "Krystal.HTML/HTML/Enums/TagName.hpp"
#include "Krystal.Lib/Mixins/RefCounted.hpp"
#include "Krystal.Lib/Pointers/RefPtr.hpp"

namespace Krys::HTML
{
  struct QualifiedNameStorage : public RefCounted<QualifiedNameStorage>
  {
    TagName _tagName {TagName::Unknown};
    Namespace _namespace {Namespace::Unknown};
    DOMStringAtom _namespaceURI {DOMStringAtom::Null()};
    DOMStringAtom _namespacePrefix {DOMStringAtom::Null()};
    DOMStringAtom _localName {DOMStringAtom::Null()};
    DOMStringAtom _localNameLower {DOMStringAtom::Null()};
    mutable DOMStringAtom _localNameUpper {DOMStringAtom::Null()};

  public:
    QualifiedNameStorage(DOMStringAtom namespaceURI, DOMStringAtom prefix, DOMStringAtom localName,
                         HTML::TagName tagName, HTML::Namespace tagNamespace) noexcept;

    ~QualifiedNameStorage() noexcept;

    KRYS_NODISCARD friend bool operator==(const QualifiedNameStorage &a,
                                          const QualifiedNameStorage &b) noexcept
    {
      return a._namespaceURI == b._namespaceURI && a._namespacePrefix == b._namespacePrefix
             && a._localName == b._localName;
    }

    KRYS_NODISCARD friend bool operator!=(const QualifiedNameStorage &a,
                                          const QualifiedNameStorage &b) noexcept
    {
      return !(a == b);
    }
  };

  /// @see https://dom.spec.whatwg.org/#concept-element-qualified-name
  class QualifiedName
  {
  private:
    RefPtr<QualifiedNameStorage> _storage {nullptr};

  public:
    QualifiedName(DOMStringAtom namespaceURI, DOMStringAtom prefix, DOMStringAtom localName) noexcept;

    QualifiedName(DOMStringAtom namespaceURI, DOMStringAtom prefix, DOMStringAtom localName,
                  HTML::TagName tagName, HTML::Namespace tagNamespace) noexcept;

    QualifiedName(const QualifiedName &other) noexcept = default;

    KRYS_NODISCARD TagName TagName() const noexcept
    {
      return _storage->_tagName;
    }

    KRYS_NODISCARD Namespace Namespace() const noexcept
    {
      return _storage->_namespace;
    }

    KRYS_NODISCARD const DOMStringAtom &NamespaceURI() const noexcept
    {
      return _storage->_namespaceURI;
    }

    KRYS_NODISCARD const DOMStringAtom &NamespacePrefix() const noexcept
    {
      return _storage->_namespacePrefix;
    }

    KRYS_NODISCARD const DOMStringAtom &LocalName() const noexcept
    {
      return _storage->_localName;
    }

    KRYS_NODISCARD const DOMStringAtom &LocalNameLower() const noexcept
    {
      return _storage->_localNameLower;
    }

    KRYS_NODISCARD const DOMStringAtom &LocalNameUpper() const noexcept;

    /// @see https://dom.spec.whatwg.org/#concept-attribute-qualified-name
    KRYS_NODISCARD DOMString Name() const noexcept
    {
      if (NamespacePrefix() == DOMStringAtom::Null())
      {
        return DOMString(LocalName().View());
      }
      else
      {
        return DOMString {NamespacePrefix().View()} + u8":" + DOMString {LocalName().View()};
      }
    }

    KRYS_NODISCARD RawPtr<QualifiedNameStorage> get() const noexcept
    {
      return _storage.get();
    }

    KRYS_NODISCARD friend bool operator==(const QualifiedName &a, const QualifiedName &b) noexcept
    {
      return a._storage == b._storage;
    }

    KRYS_NODISCARD friend bool operator!=(const QualifiedName &a, const QualifiedName &b) noexcept
    {
      return !(a == b);
    }
  };

  KRYS_NODISCARD inline QualifiedName NullQualifiedName() noexcept
  {
    return QualifiedName(DOMStringAtom::Null(), DOMStringAtom::Null(), DOMStringAtom::Null(),
                         TagName::Unknown, Namespace::None);
  }

  KRYS_NODISCARD inline QualifiedName AnyQualifiedName() noexcept
  {
    return QualifiedName(DOMStringAtom::Null(), StarAtom(), StarAtom(), TagName::Unknown, Namespace::Unknown);
  }
}