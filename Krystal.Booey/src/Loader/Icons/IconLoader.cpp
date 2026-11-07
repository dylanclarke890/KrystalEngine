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
    // FIXME: Ignore error pages (a status outside 200 to 299), once responses have a status.
    RefPtr<SharedBuffer> data =
      resource.LoadFailedOrCanceled() ? nullptr : ShareRefPtr(resource.ResourceBuffer());

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
