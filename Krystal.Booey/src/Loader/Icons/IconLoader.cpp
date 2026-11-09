/*
 * Copyright (C) 2006-2025 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/Loader/Icons/IconLoader.hpp"
#include "Krystal.Booey/DOM/Nodes/Document.hpp"
#include "Krystal.Booey/Loader/Cache/CachedResourceLoader.hpp"
#include "Krystal.Booey/Loader/Cache/CachedResourceRequest.hpp"
#include "Krystal.Booey/Loader/DocumentLoader.hpp"
#include "Krystal.Booey/Page/LocalFrame.hpp"
#include "Krystal.Booey/Platform/SharedBuffer.hpp"
#include <algorithm>

namespace krys::boo
{
  Ref<IconLoader> IconLoader::Create(DocumentLoader &documentLoader, url::URL url) noexcept
  {
    return AdoptRef(*new IconLoader(documentLoader, krys::move(url)));
  }

  IconLoader::IconLoader(DocumentLoader &documentLoader, url::URL url) noexcept
      : _documentLoader(CreateWeakPtr(&documentLoader)), _url(krys::move(url))
  {
  }

  IconLoader::~IconLoader() noexcept
  {
    StopLoading();
  }

  void IconLoader::StartLoading() noexcept
  {
    if (_resource)
    {
      return;
    }

    auto *documentLoader = _documentLoader.get();
    auto *frame = documentLoader ? documentLoader->Frame() : nullptr;
    auto *document = frame ? frame->Document() : nullptr;
    if (!document)
    {
      return;
    }

    CachedResourceRequest request(ResourceRequest(url::URL(_url)),
                                  CachedResourceLoader::DefaultCachedResourceOptions());
    request.SetInitiatorType(CachedResourceRequestInitiatorTypes::Icon);
    // An icon doesn't hold up the document's load.
    request.SetIgnoreForRequestCount(true);
    auto resource = document->GetCachedResourceLoader().RequestIcon(krys::move(request));
    if (!resource)
    {
      return;
    }
    _resource = *resource;
    // A resource that's already loaded tells its new client at once, which can end this loader.
    Ref<IconLoader> protectedThis = ShareRef(*this);
    _resource->AddClient(*this);
  }

  void IconLoader::StopLoading() noexcept
  {
    if (auto resource = std::exchange(_resource, nullptr))
    {
      resource->RemoveClient(*this);
    }
  }

  void IconLoader::NotifyFinished(CachedResource &resource) noexcept
  {
    krys_debug_assert(&resource == _resource.get());
    RefPtr<SharedBuffer> data =
      resource.LoadFailedOrCanceled() ? nullptr : ShareRefPtr(resource.ResourceBuffer());
    // An error page isn't an icon to decode.
    if (resource.Response().HTTPStatusCode() && !resource.Response().IsSuccessful())
    {
      data = nullptr;
    }

    // Not a PDF, which can't be an icon.
    constexpr Array<uint8, 4> pdfMagicNumber {'%', 'P', 'D', 'F'};
    if (data && std::ranges::starts_with(data->GetSpan(), pdfMagicNumber))
    {
      data = nullptr;
    }

    // Finishing ends this loader.
    Ref<IconLoader> protectedThis = ShareRef(*this);
    if (auto *documentLoader = _documentLoader.get())
    {
      documentLoader->FinishedLoadingIcon(*this, data.get());
    }
  }
}
