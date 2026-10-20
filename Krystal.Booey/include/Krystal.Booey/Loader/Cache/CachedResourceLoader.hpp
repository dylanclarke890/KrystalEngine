#pragma once

#include "Krystal.Booey/Loader/Cache/CachedCSSStyleSheet.hpp"
#include "Krystal.Booey/Loader/Cache/CachedFont.hpp"
#include "Krystal.Booey/Loader/Cache/CachedImage.hpp"
#include "Krystal.Booey/Loader/Cache/CachedResource.hpp"
#include "Krystal.Booey/Loader/Cache/CachedResourceHandle.hpp"
#include "Krystal.Booey/Loader/Cache/CachedResourceRequest.hpp"
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
  // FIXME: Scripts and the other resource types; preloads, request counting for the
  // load event, resource timing, security checks and revalidation, once there are those.
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
      Revalidate,
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

    KRYS_NODISCARD int32 RequestCount() const noexcept
    {
      return _requestCount;
    }

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
                                                                  CachedResourceRequest &,
                                                                  boo::CachedResource *existingResource,
                                                                  ImageLoading) const noexcept;

    KRYS_NODISCARD ImageLoading ClientDefersImage(const url::URL &) const noexcept;
    void ReloadImagesIfNotDeferred() noexcept;
  };
}
