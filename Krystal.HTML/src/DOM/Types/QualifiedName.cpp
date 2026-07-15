#include "Krystal.HTML/DOM/Types/QualifiedName.hpp"
#include "Krystal.HTML/DOM/Internals/QualifiedNameCache.hpp"
#include "Krystal.Text/ASCII.hpp"
#include <cassert>

namespace Krys::HTML
{
  QualifiedNameStorage::QualifiedNameStorage(DOMStringAtom namespaceURI, DOMStringAtom prefix,
                                             DOMStringAtom localName, HTML::TagName tagName,
                                             HTML::Namespace tagNamespace) noexcept
      : _namespaceURI(namespaceURI), _namespacePrefix(prefix), _localName(localName), _tagName(tagName),
        _namespace(tagNamespace)
  {
    assert(_namespaceURI == DOMStringAtom::Null() || _namespaceURI != DOMStringAtom::Empty());
    assert(_namespacePrefix == DOMStringAtom::Null() || _namespacePrefix != DOMStringAtom::Empty());
    assert(_localName != DOMStringAtom::Null() || _namespacePrefix != DOMStringAtom::Empty());

    if (_localName != DOMStringAtom::Null())
    {
      _localNameLower = DOMStringAtom(Krys::Text::ToASCIILowercase(_localName.View()));
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
                               HTML::TagName tagName, HTML::Namespace tagNamespace) noexcept
      : _storage(QualifiedNameCache::ThreadLocalCache().GetOrAdd(namespaceURI, prefix, localName, tagName,
                                                                 tagNamespace))
  {
  }

  KRYS_NODISCARD const DOMStringAtom &QualifiedName::LocalNameUpper() const noexcept
  {
    if (_storage->_localNameUpper == DOMStringAtom::Null())
    {
      _storage->_localNameUpper = DOMStringAtom(Krys::Text::ToASCIIUppercase(_storage->_localName.View()));
    }

    return _storage->_localNameUpper;
  }
}