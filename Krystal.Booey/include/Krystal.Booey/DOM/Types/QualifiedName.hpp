#pragma once

#include "Krystal.Booey/DOM/Types/DOMString.hpp"
#include "Krystal.Booey/HTML/Types/Namespace.hpp"
#include "Krystal.Booey/HTML/Types/TagName.hpp"
#include "Krystal.Core/Memory/RefCounted.hpp"
#include "Krystal.Core/Memory/RefPtr.hpp"

namespace krys::boo
{
  /// @brief What a `QualifiedName` names, shared by all the equal names through `QualifiedNameCache`, which
  /// forgets it when the last one goes.
  struct QualifiedNameStorage : public RefCounted<QualifiedNameStorage>
  {
    html::TagName _tagName {html::TagName::Unknown};
    html::Namespace _namespace {html::Namespace::Unknown};
    DOMStringAtom _namespaceURI {null};
    DOMStringAtom _namespacePrefix {null};
    DOMStringAtom _localName {null};
    DOMStringAtom _localNameLower {null};

  public:
    QualifiedNameStorage(DOMStringAtom namespaceURI, DOMStringAtom prefix, DOMStringAtom localName,
                         html::TagName tagName, html::Namespace tagNamespace) noexcept;

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

  /// @brief A namespace, prefix and local name, as elements and attributes are named. Equal names share one
  /// storage, so comparing two is comparing pointers.
  /// @see https://dom.spec.whatwg.org/#concept-element-qualified-name
  class QualifiedName
  {
  private:
    RefPtr<QualifiedNameStorage> _storage {nullptr};

  public:
    /// @brief The name for @p namespaceURI, @p prefix and @p localName (an empty namespace is none); a new
    /// name's tag and namespace are worked out from them.
    QualifiedName(DOMStringAtom namespaceURI, DOMStringAtom prefix, DOMStringAtom localName) noexcept;

    /// @brief The name with its tag and namespace given rather than worked out.
    QualifiedName(DOMStringAtom namespaceURI, DOMStringAtom prefix, DOMStringAtom localName,
                  html::TagName tagName, html::Namespace tagNamespace) noexcept;

    QualifiedName(const QualifiedName &other) noexcept = default;

    /// @brief The known element this names, for fast checks (`Unknown` for others).
    KRYS_NODISCARD html::TagName TagName() const noexcept
    {
      return _storage->_tagName;
    }

    /// @brief The namespace as an enum.
    KRYS_NODISCARD html::Namespace Namespace() const noexcept
    {
      return _storage->_namespace;
    }

    KRYS_NODISCARD const DOMStringAtom &NamespaceURI() const noexcept KRYS_LIFETIME_BOUND
    {
      return _storage->_namespaceURI;
    }

    KRYS_NODISCARD const DOMStringAtom &NamespacePrefix() const noexcept KRYS_LIFETIME_BOUND
    {
      return _storage->_namespacePrefix;
    }

    KRYS_NODISCARD const DOMStringAtom &LocalName() const noexcept KRYS_LIFETIME_BOUND
    {
      return _storage->_localName;
    }

    /// @brief The local name in ASCII lowercase, as HTML matches names case-insensitively.
    KRYS_NODISCARD const DOMStringAtom &LocalNameLower() const noexcept
    {
      return _storage->_localNameLower;
    }

    /// @see https://dom.spec.whatwg.org/#concept-attribute-qualified-name
    KRYS_NODISCARD DOMString Name() const noexcept
    {
      if (NamespacePrefix() == null)
      {
        return LocalName().ToString();
      }

      return NamespacePrefix().ToString() + u8":" + LocalName().ToString();
    }

    /// @brief The shared storage, which identifies the name (for hashing).
    KRYS_NODISCARD QualifiedNameStorage *get() const noexcept
    {
      return _storage.get();
    }

    /// @brief Whether this and @p other have the same local name and namespace, whatever their prefixes.
    KRYS_NODISCARD bool Matches(const QualifiedName &other) const noexcept
    {
      return _storage == other._storage
             || (LocalName() == other.LocalName() && NamespaceURI() == other.NamespaceURI());
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
}

namespace krys::boo::dom
{
  /// @brief The empty name, for no name.
  KRYS_NODISCARD inline QualifiedName NullQualifiedName() noexcept
  {
    return QualifiedName(null, null, null, html::TagName::Unknown, html::Namespace::None);
  }

  /// @brief `*|*`, which a selector uses to match any name.
  KRYS_NODISCARD inline QualifiedName AnyQualifiedName() noexcept
  {
    return QualifiedName(StarAtom(), null, StarAtom(), html::TagName::Unknown, html::Namespace::Unknown);
  }
}
