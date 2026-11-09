/*
 * Copyright (C) 2005, 2006, 2009 Apple Inc. All rights reserved.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "Krystal.Booey/DOM/Types/QualifiedName.hpp"
#include "Krystal.Booey/DOM/Internals/QualifiedNameCache.hpp"

namespace krys::boo::dom
{
  QualifiedNameStorage::QualifiedNameStorage(DOMStringAtom namespaceURI, DOMStringAtom prefix,
                                             DOMStringAtom localName, html::TagName tagName,
                                             html::Namespace tagNamespace) noexcept
      : _namespaceURI(namespaceURI), _namespacePrefix(prefix), _localName(localName), _tagName(tagName),
        _namespace(tagNamespace)
  {
    krys_debug_assert(_localName != DOMStringAtom::Null() || _namespacePrefix != DOMStringAtom::Empty());

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