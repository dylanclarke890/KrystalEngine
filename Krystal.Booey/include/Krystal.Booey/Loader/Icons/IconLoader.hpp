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
