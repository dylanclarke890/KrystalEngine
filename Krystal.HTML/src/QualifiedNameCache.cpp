#include "Krystal.HTML/QualifiedNameCache.hpp"

namespace Krys::HTML
{
  QualifiedNameCache &QualifiedNameCache::ThreadLocalCache() noexcept
  {
    thread_local QualifiedNameCache cache {};
    return cache;
  }

  Ref<QualifiedNameStorage> QualifiedNameCache::GetOrAdd(DOMStringAtom namespaceURI, DOMStringAtom prefix,
                                                         DOMStringAtom localName) noexcept
  {
    auto it = _cache.find(QualifiedNameCacheKey {namespaceURI, prefix, localName});
    if (it != _cache.end())
    {
      return ShareRef(**it);
    }

    auto tagName = ParseTagName(localName.View());
    auto tagNamespace = ParseNamespace(namespaceURI.View());

    auto storage = CreateRef<QualifiedNameStorage>(namespaceURI, prefix, localName, tagName, tagNamespace);

    _cache.insert(storage.get());

    return storage;
  }

  Ref<QualifiedNameStorage> QualifiedNameCache::GetOrAdd(DOMStringAtom namespaceURI, DOMStringAtom prefix,
                                                         DOMStringAtom localName, TagName tagName,
                                                         Namespace tagNamespace) noexcept
  {
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
