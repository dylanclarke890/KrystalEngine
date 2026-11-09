/*
 * Copyright (C) 2003-2025 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/DOM/Types/DOMString.hpp"
#include "Krystal.Booey/Loader/FetchOptions.hpp"
#include "Krystal.Booey/Loader/FrameLoaderTypes.hpp"
#include "Krystal.Booey/Loader/SubstituteData.hpp"
#include "Krystal.Booey/Platform/Network/ResourceRequest.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::dom
{
  class Document;
  class Element;
}

namespace krys::boo
{
  /// @brief What a request to load a frame and the navigation action it becomes have in common.
  class FrameLoadRequestBase
  {
  private:
    dom::DOMStringAtom _downloadAttribute {dom::DOMStringAtom::Null()};
    RefPtr<dom::Element> _sourceElement;

  public:
    /// @brief The `download` attribute of the link followed; null if it has none.
    KRYS_NODISCARD dom::DOMStringAtom DownloadAttribute() const noexcept
    {
      return _downloadAttribute;
    }

    /// @brief The element that started the navigation (a link or a form), if any.
    KRYS_NODISCARD dom::Element *SourceElement() const noexcept
    {
      return _sourceElement.get();
    }
    void SetSourceElement(dom::Element *sourceElement) noexcept;

  protected:
    FrameLoadRequestBase() noexcept;
    FrameLoadRequestBase(const FrameLoadRequestBase &) noexcept;
    FrameLoadRequestBase &operator=(const FrameLoadRequestBase &) noexcept;
    FrameLoadRequestBase(FrameLoadRequestBase &&) noexcept;
    FrameLoadRequestBase &operator=(FrameLoadRequestBase &&) noexcept;
    ~FrameLoadRequestBase() noexcept;

    void SetDownloadAttribute(dom::DOMStringAtom downloadAttribute) noexcept
    {
      _downloadAttribute = downloadAttribute;
    }
  };

  /// @brief A request to load a URL into a frame: what's loaded, from which document, into which browsing
  /// context.
  class FrameLoadRequest : public FrameLoadRequestBase
  {
  private:
    Ref<dom::Document> _requester;
    ResourceRequest _resourceRequest;
    dom::DOMStringAtom _frameName;
    ReferrerPolicy _referrerPolicy {ReferrerPolicy::EmptyString};
    Maybe<SubstituteData> _substituteData;

  public:
    /// @param requester The document asking for the load.
    /// @param frameName The browsing context to load into (`_self`, `_blank` and the others, or a name).
    /// @param downloadAttribute The `download` attribute of the link followed; null if it has none.
    FrameLoadRequest(Ref<dom::Document> &&requester, ResourceRequest &&resourceRequest,
                     dom::DOMStringAtom frameName,
                     dom::DOMStringAtom downloadAttribute = dom::DOMStringAtom::Null()) noexcept;
    ~FrameLoadRequest() noexcept;
    FrameLoadRequest(const FrameLoadRequest &) noexcept;
    FrameLoadRequest &operator=(const FrameLoadRequest &) noexcept;
    FrameLoadRequest(FrameLoadRequest &&) noexcept;
    FrameLoadRequest &operator=(FrameLoadRequest &&) noexcept;

    KRYS_NODISCARD bool IsEmpty() const noexcept
    {
      return _resourceRequest.IsNull();
    }

    KRYS_NODISCARD dom::Document &Requester() const noexcept
    {
      return *_requester;
    }

    KRYS_NODISCARD ResourceRequest &GetResourceRequest() noexcept
    {
      return _resourceRequest;
    }
    KRYS_NODISCARD const ResourceRequest &GetResourceRequest() const noexcept
    {
      return _resourceRequest;
    }

    KRYS_NODISCARD dom::DOMStringAtom FrameName() const noexcept
    {
      return _frameName;
    }

    /// @brief The document's content, which the host gives in place of loading the request's URL.
    KRYS_NODISCARD const Maybe<SubstituteData> &GetSubstituteData() const noexcept
    {
      return _substituteData;
    }
    void SetSubstituteData(SubstituteData &&substituteData) noexcept
    {
      _substituteData = krys::move(substituteData);
    }
    void SetFrameName(dom::DOMStringAtom frameName) noexcept
    {
      _frameName = frameName;
    }

    KRYS_NODISCARD ReferrerPolicy GetReferrerPolicy() const noexcept
    {
      return _referrerPolicy;
    }
    void SetReferrerPolicy(ReferrerPolicy referrerPolicy) noexcept
    {
      _referrerPolicy = referrerPolicy;
    }
  };
}
