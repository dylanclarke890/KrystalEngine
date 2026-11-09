/*
 * Copyright (C) 2006, 2008 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/Loader/Cache/CachedRawResource.hpp"
#include "Krystal.Booey/Loader/Cache/CachedResourceClient.hpp"
#include "Krystal.Booey/Loader/Cache/CachedResourceHandle.hpp"
#include "Krystal.Booey/URL/URL.hpp"
#include "Krystal.Core/RefCounted.hpp"
#include "Krystal.Core/Types/WeakPtr.hpp"

namespace krys::boo
{
  class DocumentLoader;

  /// @brief Loads one of a document's icons for its document loader, which hands its bytes to the host.
  class IconLoader final : public RefCounted<IconLoader>, public CachedResourceClient
  {
    KRYS_NON_COPY_MOVABLE(IconLoader);

  private:
    WeakPtr<DocumentLoader> _documentLoader;
    url::URL _url;
    CachedResourceHandle<CachedRawResource> _resource;

  public:
    KRYS_NODISCARD static Ref<IconLoader> Create(DocumentLoader &documentLoader, url::URL url) noexcept;
    ~IconLoader() noexcept override;

    void StartLoading() noexcept;
    void StopLoading() noexcept;

  private:
    IconLoader(DocumentLoader &documentLoader, url::URL url) noexcept;

    void NotifyFinished(CachedResource &resource) noexcept override;
  };
}
