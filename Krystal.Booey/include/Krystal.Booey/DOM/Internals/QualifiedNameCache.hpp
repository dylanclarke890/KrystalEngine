#pragma once

#include "Krystal.Booey/DOM/Types/DOMString.hpp"
#include "Krystal.Booey/DOM/Types/QualifiedName.hpp"
#include "Krystal.Booey/HTML/Enums/Namespace.hpp"
#include "Krystal.Booey/HTML/Enums/TagName.hpp"
#include "Krystal.Core/Hash.hpp"
#include "Krystal.Core/Types/HashSet.hpp"
#include <utility>

namespace krys::boo::dom
{
  struct QualifiedNameCacheKey
  {
    DOMStringAtom NamespaceURI {DOMStringAtom::Null()};
    DOMStringAtom NamespacePrefix {DOMStringAtom::Null()};
    DOMStringAtom LocalName {DOMStringAtom::Null()};

    bool operator==(const QualifiedNameCacheKey &other) const noexcept
    {
      return NamespaceURI == other.NamespaceURI && NamespacePrefix == other.NamespacePrefix
             && LocalName == other.LocalName;
    }

    bool operator==(QualifiedNameStorage *ptr) const noexcept
    {
      return NamespaceURI == ptr->_namespaceURI && NamespacePrefix == ptr->_namespacePrefix
             && LocalName == ptr->_localName;
    }
  };

  struct QualifiedNameCacheKeyHash
  {
    using is_transparent = void;

    size_t operator()(const QualifiedNameCacheKey &key) const
    {
      return krys::Hash::Combine(key.NamespaceURI, key.NamespacePrefix, key.LocalName);
    }

    size_t operator()(QualifiedNameStorage *ptr) const
    {
      return krys::Hash::Combine(ptr->_namespaceURI, ptr->_namespacePrefix, ptr->_localName);
    }
  };

  struct QualifiedNameCacheKeyEqual
  {
    using is_transparent = void;

    bool operator()(const QualifiedNameCacheKey &lhs, const QualifiedNameCacheKey &rhs) const
    {
      return lhs == rhs;
    }

    bool operator()(const QualifiedNameCacheKey &key, QualifiedNameStorage *ptr) const
    {
      return key == ptr;
    }

    bool operator()(QualifiedNameStorage *ptr, const QualifiedNameCacheKey &key) const
    {
      return key == ptr;
    }

    bool operator()(QualifiedNameStorage *lhs, QualifiedNameStorage *rhs) const
    {
      return lhs == rhs;
    }
  };

  class QualifiedNameCache
  {
    using QNameSet = HashSet<QualifiedNameStorage *, QualifiedNameCacheKeyHash, QualifiedNameCacheKeyEqual>;

  private:
    QNameSet _cache;

  public:
    KRYS_NODISCARD static QualifiedNameCache &ThreadLocalCache() noexcept;

    KRYS_NODISCARD Ref<QualifiedNameStorage> GetOrAdd(DOMStringAtom namespaceURI, DOMStringAtom prefix,
                                                      DOMStringAtom localName) noexcept;

    KRYS_NODISCARD Ref<QualifiedNameStorage> GetOrAdd(DOMStringAtom namespaceURI, DOMStringAtom prefix,
                                                      DOMStringAtom localName, html::TagName tagName,
                                                      html::Namespace tagNamespace) noexcept;

    void Remove(QualifiedNameStorage &storage) noexcept;
  };
}