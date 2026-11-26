#include "Krystal.Web/Dom/Types/QualifiedName.hpp"
#include "Krystal.Core/Move.hpp"
#include "Krystal.Web/Dom/Internals/QualifiedNameCache.hpp"

namespace krys::web
{
  QualifiedNameStorage::QualifiedNameStorage(DOMStringAtom namespaceURI, DOMStringAtom prefix,
                                             DOMStringAtom localName, html::TagName tagName,
                                             html::Namespace tagNamespace) noexcept
      : _namespaceURI(krys::move(namespaceURI)), _namespacePrefix(krys::move(prefix)),
        _localName(krys::move(localName)), _tagName(tagName), _namespace(tagNamespace)
  {
    // Only the null name lacks a local name.
    krys_debug_assert(_localName != null || _namespacePrefix != DOMStringAtom::Empty());

    if (_localName != null)
    {
      _localNameLower = _localName.ToASCIILower();
    }
  }

  QualifiedNameStorage::~QualifiedNameStorage() noexcept
  {
    dom::QualifiedNameCache::ThreadLocalCache().Remove(*this);
  }

  QualifiedName::QualifiedName(DOMStringAtom namespaceURI, DOMStringAtom prefix,
                               DOMStringAtom localName) noexcept
      : _storage(dom::QualifiedNameCache::ThreadLocalCache().GetOrAdd(
          krys::move(namespaceURI), krys::move(prefix), krys::move(localName)))
  {
  }

  QualifiedName::QualifiedName(DOMStringAtom namespaceURI, DOMStringAtom prefix, DOMStringAtom localName,
                               html::TagName tagName, html::Namespace tagNamespace) noexcept
      : _storage(dom::QualifiedNameCache::ThreadLocalCache().GetOrAdd(
          krys::move(namespaceURI), krys::move(prefix), krys::move(localName), tagName, tagNamespace))
  {
  }
}
