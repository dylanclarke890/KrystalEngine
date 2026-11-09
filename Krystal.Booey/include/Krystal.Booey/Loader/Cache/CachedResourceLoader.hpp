/*
    Copyright (C) 1998 Lars Knoll (knoll@mpi-hd.mpg.de)
    Copyright (C) 2001 Dirk Mueller <mueller@kde.org>
    Copyright (C) 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011 Apple Inc. All rights reserved.
    Copyright (C) 2009 Torch Mobile Inc. http://www.torchmobile.com/

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.

    This class provides all functionality needed for loading images, style sheets and html
    pages from the web. It has a memory cache for these objects.
*/

#pragma once

#include "Krystal.Booey/Loader/Cache/CachedCSSStyleSheet.hpp"
#include "Krystal.Booey/Loader/Cache/CachedFont.hpp"
#include "Krystal.Booey/Loader/Cache/CachedImage.hpp"
#include "Krystal.Booey/Loader/Cache/CachedRawResource.hpp"
#include "Krystal.Booey/Loader/Cache/CachedResource.hpp"
#include "Krystal.Booey/Loader/Cache/CachedResourceHandle.hpp"
#include "Krystal.Booey/Loader/Cache/CachedResourceRequest.hpp"
#include "Krystal.Booey/Loader/Cache/CachedSVGDocument.hpp"
#include "Krystal.Booey/Loader/Cache/CachePolicy.hpp"
#include "Krystal.Booey/Platform/Network/ResourceError.hpp"
#include "Krystal.Booey/Platform/Timer.hpp"
#include "Krystal.Core/CanMakeWeakPtr.hpp"
#include "Krystal.Core/RefCounted.hpp"
#include "Krystal.Core/Types/Expected.hpp"
#include "Krystal.Core/Types/HashMap.hpp"
#include "Krystal.Core/Types/HashSet.hpp"
#include "Krystal.Core/Types/WeakPtr.hpp"

namespace krys::boo
{
  class LocalFrame;

  namespace dom
  {
    class Document;
  }

  template <typename T>
  using ResourceErrorOr = Expected<T, ResourceError>;

  enum class ImageLoading : uint8
  {
    Immediate,
    DeferredUntilVisible
  };

  // The CachedResourceLoader provides a per-context interface to the MemoryCache
  // and enforces a bunch of security checks and rules for resource revalidation.
  /// @note One per document, which creates it; it loads through the document's frame.
  // FIXME: The other resource types (not scripts: scripting isn't planned); preloads, request counting
  // for the load event, resource timing, security checks and revalidation, once there are those.
  class CachedResourceLoader : public RefCounted<CachedResourceLoader>,
                               public CanMakeWeakPtr<CachedResourceLoader>
  {
    KRYS_NON_COPY_MOVABLE(CachedResourceLoader);

  public:
    using DocumentResourceMap = HashMap<utf8_string, CachedResourceHandle<CachedResource>>;

  private:
    enum RevalidationPolicy : uint8
    {
      Use,
      Reload,
      Load
    };

    mutable DocumentResourceMap _documentResources;
    // The URLs the document has loaded or validated, which it doesn't load again whatever the cache policy.
    HashSet<utf8_string> _validatedURLs;
    WeakPtr<dom::Document> _document;
    int32 _requestCount {0};
    Timer _garbageCollectDocumentResourcesTimer;
    bool _autoLoadImages {true};
    bool _imagesEnabled {true};

  public:
    KRYS_NODISCARD static Ref<CachedResourceLoader> Create(dom::Document *document) noexcept
    {
      return AdoptRef(*new CachedResourceLoader(document));
    }
    ~CachedResourceLoader() noexcept;

    ResourceErrorOr<CachedResourceHandle<CachedImage>>
      RequestImage(CachedResourceRequest &&, ImageLoading = ImageLoading::Immediate) noexcept;
    ResourceErrorOr<CachedResourceHandle<CachedCSSStyleSheet>>
      RequestCSSStyleSheet(CachedResourceRequest &&) noexcept;
    ResourceErrorOr<CachedResourceHandle<CachedFont>> RequestFont(CachedResourceRequest &&,
                                                                  bool isSVG) noexcept;
    ResourceErrorOr<CachedResourceHandle<CachedSVGDocument>>
      RequestSVGDocument(CachedResourceRequest &&) noexcept;
    /// @brief An icon (`<link rel=icon>`), kept as its bytes.
    ResourceErrorOr<CachedResourceHandle<CachedRawResource>> RequestIcon(CachedResourceRequest &&) noexcept;

    KRYS_NODISCARD boo::CachedResource *GetCachedResource(utf8_stringview url) const noexcept;
    KRYS_NODISCARD boo::CachedResource *GetCachedResource(const url::URL &url) const noexcept;

    KRYS_NODISCARD const DocumentResourceMap &AllCachedResources() const noexcept
    {
      return _documentResources;
    }

    KRYS_NODISCARD bool AutoLoadImages() const noexcept
    {
      return _autoLoadImages;
    }
    void SetAutoLoadImages(bool) noexcept;

    KRYS_NODISCARD bool ImagesEnabled() const noexcept
    {
      return _imagesEnabled;
    }
    void SetImagesEnabled(bool) noexcept;

    KRYS_NODISCARD bool ShouldDeferImageLoad(const url::URL &) const noexcept;
    KRYS_NODISCARD bool ShouldPerformImageLoad(const url::URL &) const noexcept;

    KRYS_NODISCARD LocalFrame *Frame() const noexcept;          // Can be null
    KRYS_NODISCARD dom::Document *GetDocument() const noexcept; // Can be null
    void SetDocument(dom::Document *) noexcept;

    void GarbageCollectDocumentResources() noexcept;

    /// @brief How many of the document's loads are in flight, which delay its load event.
    KRYS_NODISCARD int32 RequestCount() const noexcept
    {
      return _requestCount;
    }

    void IncrementRequestCount(const CachedResource &resource) noexcept;
    void DecrementRequestCount(const CachedResource &resource) noexcept;

    /// @brief A load is done: the frame checks whether its document has loaded.
    void LoadDone() noexcept;

    /// @brief The document's load event finished: URLs are validated again.
    void DocumentDidFinishLoadEvent() noexcept;

    KRYS_NODISCARD static const ResourceLoaderOptions &DefaultCachedResourceOptions() noexcept;

  private:
    explicit CachedResourceLoader(dom::Document *) noexcept;

    ResourceErrorOr<CachedResourceHandle<boo::CachedResource>>
      RequestResource(boo::CachedResource::Type, CachedResourceRequest &&,
                      ImageLoading = ImageLoading::Immediate) noexcept;

    CachedResourceHandle<boo::CachedResource> LoadResource(boo::CachedResource::Type,
                                                           CachedResourceRequest &&) noexcept;

    KRYS_NODISCARD CachePolicy GetCachePolicy(boo::CachedResource::Type type) const noexcept;
    KRYS_NODISCARD RevalidationPolicy DetermineRevalidationPolicy(boo::CachedResource::Type,
                                                                  boo::CachedResource *existingResource,
                                                                  ImageLoading) const noexcept;

    KRYS_NODISCARD ImageLoading ClientDefersImage(const url::URL &) const noexcept;
    void ReloadImagesIfNotDeferred() noexcept;
  };
}
