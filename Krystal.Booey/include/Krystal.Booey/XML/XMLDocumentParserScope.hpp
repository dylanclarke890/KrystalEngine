/*
 * Copyright (C) 2009-2024 Apple Inc. All rights reserved.
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
 */

#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Types/WeakPtr.hpp"

struct _xmlParserInput;
struct _xmlParserCtxt;

namespace krys::boo
{
  class CachedResourceLoader;
}

namespace krys::boo::xml
{
  /// @brief While alive, the loads libxml2 asks for (external entities, DTDs) are made for the given cached
  /// resource loader's document, under its rules about what may be loaded.
  class XMLDocumentParserScope
  {
    KRYS_NON_COPY_MOVABLE(XMLDocumentParserScope);
    using EntityLoaderFunction = _xmlParserInput *(*)(const char *, const char *, _xmlParserCtxt *);

  private:
    WeakPtr<CachedResourceLoader> _oldCachedResourceLoader;
    EntityLoaderFunction _oldEntityLoader {nullptr};

  public:
    XMLDocumentParserScope() = delete;
    explicit XMLDocumentParserScope(CachedResourceLoader *) noexcept;
    ~XMLDocumentParserScope() noexcept;

    KRYS_NODISCARD static WeakPtr<CachedResourceLoader> &CurrentCachedResourceLoader() noexcept;
  };
}
