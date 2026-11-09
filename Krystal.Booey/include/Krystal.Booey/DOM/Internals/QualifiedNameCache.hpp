/*
 * Copyright (C) 2017 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#pragma once

#include "Krystal.Booey/DOM/Types/DOMString.hpp"
#include "Krystal.Booey/DOM/Types/QualifiedName.hpp"
#include "Krystal.Booey/HTML/Types/Namespace.hpp"
#include "Krystal.Booey/HTML/Types/TagName.hpp"
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