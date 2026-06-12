#pragma once

#include "Krystal.HTML/HTML/Enums/Namespace.hpp"
#include "Krystal.HTML/HTML/Enums/TagName.hpp"
#include "Krystal.HTML/QualifiedName.hpp"
#include "Krystal.HTML/Types/DOMString.hpp"
#include "Krystal.Lib/Core/Hash.hpp"
#include "Krystal.Lib/Pointers/RawPtr.hpp"
#include "Krystal.Lib/Types/List.hpp"
#include <utility>

namespace Krys::HTML
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

    bool operator==(RawPtr<QualifiedNameStorage> ptr) const noexcept
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
      return Krys::Hash::Combine(key.NamespaceURI, key.NamespacePrefix, key.LocalName);
    }

    size_t operator()(RawPtr<QualifiedNameStorage> ptr) const
    {
      return Krys::Hash::Combine(ptr->_namespaceURI, ptr->_namespacePrefix, ptr->_localName);
    }
  };

  struct QualifiedNameCacheKeyEqual
  {
    using is_transparent = void;

    bool operator()(const QualifiedNameCacheKey &lhs, const QualifiedNameCacheKey &rhs) const
    {
      return lhs == rhs;
    }

    bool operator()(const QualifiedNameCacheKey &key, RawPtr<QualifiedNameStorage> ptr) const
    {
      return key == ptr;
    }

    bool operator()(RawPtr<QualifiedNameStorage> ptr, const QualifiedNameCacheKey &key) const
    {
      return key == ptr;
    }

    bool operator()(RawPtr<QualifiedNameStorage> lhs, RawPtr<QualifiedNameStorage> rhs) const
    {
      return lhs == rhs;
    }
  };

  class QualifiedNameCache
  {
    using QNameSet = Set<RawPtr<QualifiedNameStorage>, QualifiedNameCacheKeyHash, QualifiedNameCacheKeyEqual>;

  private:
    QNameSet _cache;

  public:
    KRYS_NODISCARD static QualifiedNameCache &ThreadLocalCache() noexcept;

    KRYS_NODISCARD Ref<QualifiedNameStorage> GetOrAdd(DOMStringAtom namespaceURI, DOMStringAtom prefix,
                                                         DOMStringAtom localName) noexcept;

    KRYS_NODISCARD Ref<QualifiedNameStorage> GetOrAdd(DOMStringAtom namespaceURI, DOMStringAtom prefix,
                                                         DOMStringAtom localName, TagName tagName,
                                                         Namespace tagNamespace) noexcept;

    void Remove(QualifiedNameStorage &storage) noexcept;
  };
}