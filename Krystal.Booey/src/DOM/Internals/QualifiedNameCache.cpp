#include "Krystal.Booey/DOM/Internals/QualifiedNameCache.hpp"

namespace krys::boo::dom
{
  QualifiedNameCache &QualifiedNameCache::ThreadLocalCache() noexcept
  {
    thread_local QualifiedNameCache cache {};
    return cache;
  }

  Ref<QualifiedNameStorage> QualifiedNameCache::GetOrAdd(DOMStringAtom namespaceURI, DOMStringAtom prefix,
                                                         DOMStringAtom localName) noexcept
  {
    // An empty namespace is no namespace.
    if (namespaceURI.View().empty())
    {
      namespaceURI = DOMStringAtom::Null();
    }
    auto it = _cache.find(QualifiedNameCacheKey {namespaceURI, prefix, localName});
    if (it != _cache.end())
    {
      return ShareRef(**it);
    }

    auto tagName = localName.View().empty() ? html::TagName::Unknown : html::ParseTagName(localName.View());
    auto tagNamespace = html::ParseNamespace(namespaceURI.View());

    auto storage = CreateRef<QualifiedNameStorage>(namespaceURI, prefix, localName, tagName, tagNamespace);

    _cache.insert(storage.get());

    return storage;
  }

  Ref<QualifiedNameStorage> QualifiedNameCache::GetOrAdd(DOMStringAtom namespaceURI, DOMStringAtom prefix,
                                                         DOMStringAtom localName, html::TagName tagName,
                                                         html::Namespace tagNamespace) noexcept
  {
    // An empty namespace is no namespace.
    if (namespaceURI.View().empty())
    {
      namespaceURI = DOMStringAtom::Null();
    }
    auto it = _cache.find(QualifiedNameCacheKey {namespaceURI, prefix, localName});
    if (it != _cache.end())
    {
      return ShareRef(**it);
    }

    auto storage = CreateRef<QualifiedNameStorage>(namespaceURI, prefix, localName, tagName, tagNamespace);

    _cache.insert(storage.get());

    return storage;
  }

  void QualifiedNameCache::Remove(QualifiedNameStorage &storage) noexcept
  {
    _cache.erase(&storage);
  }
}
