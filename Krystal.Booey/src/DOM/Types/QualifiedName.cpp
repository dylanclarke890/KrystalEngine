#include "Krystal.Booey/DOM/Types/QualifiedName.hpp"
#include "Krystal.Booey/DOM/Internals/QualifiedNameCache.hpp"
#include <cassert>

namespace krys::boo::dom
{
  QualifiedNameStorage::QualifiedNameStorage(DOMStringAtom namespaceURI, DOMStringAtom prefix,
                                             DOMStringAtom localName, html::TagName tagName,
                                             html::Namespace tagNamespace) noexcept
      : _namespaceURI(namespaceURI), _namespacePrefix(prefix), _localName(localName), _tagName(tagName),
        _namespace(tagNamespace)
  {
    assert(_localName != DOMStringAtom::Null() || _namespacePrefix != DOMStringAtom::Empty());

    if (_localName != DOMStringAtom::Null())
    {
      _localNameLower = DOMStringAtom(krys::text::ToASCIILower(_localName.View()));
    }
  }

  QualifiedNameStorage::~QualifiedNameStorage() noexcept
  {
    QualifiedNameCache::ThreadLocalCache().Remove(*this);
  }

  QualifiedName::QualifiedName(DOMStringAtom namespaceURI, DOMStringAtom prefix,
                               DOMStringAtom localName) noexcept
      : _storage(QualifiedNameCache::ThreadLocalCache().GetOrAdd(namespaceURI, prefix, localName))
  {
  }

  QualifiedName::QualifiedName(DOMStringAtom namespaceURI, DOMStringAtom prefix, DOMStringAtom localName,
                               html::TagName tagName, html::Namespace tagNamespace) noexcept
      : _storage(QualifiedNameCache::ThreadLocalCache().GetOrAdd(namespaceURI, prefix, localName, tagName,
                                                                 tagNamespace))
  {
  }

  KRYS_NODISCARD const DOMStringAtom &QualifiedName::LocalNameUpper() const noexcept
  {
    if (_storage->_localNameUpper == DOMStringAtom::Null())
    {
      _storage->_localNameUpper = DOMStringAtom(krys::text::ToASCIIUpper(_storage->_localName.View()));
    }

    return _storage->_localNameUpper;
  }
}