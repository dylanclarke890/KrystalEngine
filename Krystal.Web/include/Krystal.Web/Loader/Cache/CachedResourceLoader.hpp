#pragma once

#include "Krystal.Core/Containers/HashMap.hpp"
#include "Krystal.Core/Containers/HashSet.hpp"
#include "Krystal.Core/Memory/CanMakeWeakPtr.hpp"
#include "Krystal.Core/Memory/RefCounted.hpp"
#include "Krystal.Core/Memory/WeakPtr.hpp"
#include "Krystal.Core/Time/Timer.hpp"
#include "Krystal.Core/Types/Expected.hpp"
#include "Krystal.Net/ResourceError.hpp"
#include "Krystal.Web/Loader/Cache/CachedCSSStyleSheet.hpp"
#include "Krystal.Web/Loader/Cache/CachedFont.hpp"
#include "Krystal.Web/Loader/Cache/CachedImage.hpp"
#include "Krystal.Web/Loader/Cache/CachedRawResource.hpp"
#include "Krystal.Web/Loader/Cache/CachedResource.hpp"
#include "Krystal.Web/Loader/Cache/CachedResourceHandle.hpp"
#include "Krystal.Web/Loader/Cache/CachedResourceRequest.hpp"
#include "Krystal.Web/Loader/Cache/CachedSVGDocument.hpp"
#include "Krystal.Web/Loader/Cache/CachePolicy.hpp"

namespace krys::web
{
  class Document;
  class Frame;

  template <typename T>
  using ResourceErrorOr = Expected<T, net::ResourceError>;

  enum class ImageLoading : uint8
  {
    Immediate,
    DeferredUntilVisible,
  };

  /// @brief A document's way to its resources: it finds them in the memory cache or loads them, keeps the
  /// ones the document uses, and counts the loads that delay its load event.
  /// @note One per document, which creates it; it loads through the document's frame.
  // FIXME: The other resource types (not scripts: scripting isn't planned); preloads, resource timing,
  // security checks and revalidation, once there are those.
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
      Load,
    };

    mutable DocumentResourceMap _documentResources;
    // The URLs the document has loaded or validated, which it doesn't load again whatever the cache policy.
    HashSet<utf8_string> _validatedUrls;
    WeakPtr<web::Document> _document;
    int32 _requestCount {0};
    Timer _garbageCollectDocumentResourcesTimer;

  public:
    /// @brief A loader for @p document.
    KRYS_NODISCARD static Ref<CachedResourceLoader> Create(web::Document *document) noexcept
    {
      return AdoptRef(*new CachedResourceLoader(document));
    }

    ~CachedResourceLoader() noexcept;

    ResourceErrorOr<CachedResourceHandle<CachedImage>>
      /// @brief The image for the request, from the cache or loaded; a deferred one isn't loaded until it's
      /// asked again.
      RequestImage(CachedResourceRequest &&, ImageLoading = ImageLoading::Immediate) noexcept;

    ResourceErrorOr<CachedResourceHandle<CachedCSSStyleSheet>>
      /// @brief The style sheet, font or SVG document for the request, from the cache or loaded.
      RequestCSSStyleSheet(CachedResourceRequest &&) noexcept;

    ResourceErrorOr<CachedResourceHandle<CachedFont>> RequestFont(CachedResourceRequest &&) noexcept;

    ResourceErrorOr<CachedResourceHandle<CachedSVGDocument>>
      RequestSVGDocument(CachedResourceRequest &&) noexcept;

    /// @brief An icon (`<link rel=icon>`), kept as its bytes.
    ResourceErrorOr<CachedResourceHandle<CachedRawResource>> RequestIcon(CachedResourceRequest &&) noexcept;

    /// @brief An image whose data its requester gives it (an image document's), loading until it's finished;
    /// it's one of the document's resources but not in the memory cache.
    KRYS_NODISCARD CachedResourceHandle<CachedImage>
      CreateManuallyLoadedImage(CachedResourceRequest &&) noexcept;

    /// @brief The document's resource for @p url (completed against the document's base URL), if it has one.
    KRYS_NODISCARD web::CachedResource *GetCachedResource(utf8_stringview url) const noexcept;

    KRYS_NODISCARD web::CachedResource *GetCachedResource(const net::Url &url) const noexcept;

    /// @brief The resources the document uses, by URL.
    KRYS_NODISCARD const DocumentResourceMap &AllCachedResources() const noexcept
    {
      return _documentResources;
    }

    /// @brief The document's frame and the document, if there are.
    KRYS_NODISCARD web::Frame *Frame() const noexcept;

    KRYS_NODISCARD web::Document *GetDocument() const noexcept;

    /// @brief Sets or clears the document.
    void SetDocument(web::Document *) noexcept;

    /// @brief Forgets the document's resources that nothing else holds and that aren't loading.
    void GarbageCollectDocumentResources() noexcept;

    /// @brief How many of the document's loads are in flight, which delay its load event.
    KRYS_NODISCARD int32 RequestCount() const noexcept
    {
      return _requestCount;
    }

    /// @brief Counts a load in or out of the ones that delay the load event (unless it's left out).
    void IncrementRequestCount(const CachedResource &resource) noexcept;

    void DecrementRequestCount(const CachedResource &resource) noexcept;

    /// @brief A load is done: the frame checks whether its document has loaded.
    void LoadDone() noexcept;

    /// @brief The document's load event finished: URLs are validated again.
    void DocumentDidFinishLoadEvent() noexcept;

    /// @brief The options of a subresource load: buffered, cached and policy-checked.
    KRYS_NODISCARD static const ResourceLoaderOptions &DefaultCachedResourceOptions() noexcept;

  private:
    /// @brief A loader for the document.
    explicit CachedResourceLoader(web::Document *) noexcept;

    ResourceErrorOr<CachedResourceHandle<web::CachedResource>>
      /// @brief Finds or loads a resource of @p type, as the revalidation policy says, and keeps it as the
      /// document's.
      RequestResource(web::CachedResource::Type, CachedResourceRequest &&,
                      ImageLoading = ImageLoading::Immediate) noexcept;

    /// @brief Makes a resource of @p type for the request, and adds it to the memory cache.
    CachedResourceHandle<web::CachedResource> LoadResource(web::CachedResource::Type,
                                                           CachedResourceRequest &&) noexcept;

    /// @brief The cache policy for a load: a reload's, else the host's.
    KRYS_NODISCARD CachePolicy GetCachePolicy(web::CachedResource::Type type) const noexcept;

    /// @brief Whether to use @p existingResource, replace it, or load a new resource.
    KRYS_NODISCARD RevalidationPolicy DetermineRevalidationPolicy(web::CachedResource::Type,
                                                                  web::CachedResource *existingResource,
                                                                  ImageLoading) const noexcept;
  };
}
