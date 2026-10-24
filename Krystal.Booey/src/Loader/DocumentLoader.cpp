#include "Krystal.Booey/Loader/DocumentLoader.hpp"
#include "Krystal.Booey/HTML/EventLoop.hpp"
#include "Krystal.Booey/Loader/FrameLoader.hpp"
#include "Krystal.Booey/Loader/LoaderStrategy.hpp"
#include "Krystal.Booey/Page/LocalFrame.hpp"
#include "Krystal.Booey/Platform/Network/DataURLDecoder.hpp"
#include "Krystal.Booey/Platform/PlatformStrategies.hpp"
#include "Krystal.Booey/Platform/SharedBuffer.hpp"

namespace krys::boo
{
  DocumentLoader::DocumentLoader(ResourceRequest &&request, Maybe<SubstituteData> &&substituteData) noexcept
      : _request(krys::move(request)), _substituteData(krys::move(substituteData))
  {
  }

  DocumentLoader::~DocumentLoader() noexcept = default;

  Ref<DocumentLoader> DocumentLoader::Create(ResourceRequest &&request,
                                             Maybe<SubstituteData> &&substituteData) noexcept
  {
    return AdoptRef(*new DocumentLoader(krys::move(request), krys::move(substituteData)));
  }

  void DocumentLoader::AttachToFrame(LocalFrame &frame) noexcept
  {
    _frame = CreateWeakPtr(&frame);
    _writer.SetFrame(frame);
  }

  void DocumentLoader::DetachFromFrame() noexcept
  {
    // Detached, the loader tells no one it stopped.
    _frame = nullptr;
    StopLoading();
  }

  const url::URL &DocumentLoader::DocumentURL() const noexcept
  {
    if (_substituteData && !_substituteData->ResponseURL().IsNull())
    {
      return _substituteData->ResponseURL();
    }
    if (!_response.IsNull())
    {
      return _response.URL();
    }
    return _request.URL();
  }

  void DocumentLoader::StartLoadingMainResource() noexcept
  {
    RefPtr<LocalFrame> frame = ShareRefPtr(_frame.get());
    if (!frame)
    {
      return;
    }

    _isLoadingMainResource = true;
    _isStopping = false;

    // The main resource arrives as a networking task, whoever loads it.
    auto finish = [weakThis = CreateWeakPtr(this)](LoadResult &&result)
    {
      html::EventLoop::Current().QueueTask(html::TaskSource::Networking,
                                           [weakThis, result = krys::move(result)]() mutable
                                           {
                                             RefPtr<DocumentLoader> loader = ShareRefPtr(weakThis.get());
                                             if (!loader || !loader->_isLoadingMainResource)
                                             {
                                               return;
                                             }
                                             if (!result)
                                             {
                                               loader->MainReceivedError(ResourceError(result.error()));
                                               return;
                                             }
                                             loader->ContinueAfterMainResourceLoad(
                                               krys::move(result->Response), result->Data->GetSpan());
                                           });
    };

    // Substitute data: the host's content, in place of the URL's.
    if (_substituteData)
    {
      auto &content = _substituteData->Content();
      auto data = SharedBuffer::Create(
        Span<const uint8>(reinterpret_cast<const uint8 *>(content.data()), content.size()));
      finish(
        LoadedResource {.Response = ResourceResponse(DocumentURL(), utf8_string(_substituteData->MimeType()),
                                                     static_cast<int64>(content.size()), u8"UTF-8"),
                        .Data = krys::move(data)});
      return;
    }

    auto &url = _request.URL();

    // about:blank, and the other about: URLs, are empty documents.
    if (url.ProtocolIsAbout())
    {
      finish(LoadedResource {.Response = ResourceResponse(url, u8"text/html", 0, u8"UTF-8"),
                             .Data = SharedBuffer::Create(Span<const uint8>())});
      return;
    }

    // Data URLs are decoded here, rather than by the host.
    if (url.ProtocolIsData())
    {
      auto decoded = data_url_decoder::Decode(url);
      if (!decoded)
      {
        finish(
          Unexpected(ResourceError(utf8_string(ErrorDomainBooeyInternal), 0, url, u8"Invalid data URL")));
        return;
      }
      auto size = static_cast<int64>(decoded->Data.size());
      finish(LoadedResource {
        .Response = ResourceResponse(url, krys::move(decoded->MimeType), size, krys::move(decoded->Charset)),
        .Data = SharedBuffer::Create(krys::move(decoded->Data))});
      return;
    }

    auto *loaderStrategy = PlatformStrategies::Singleton().GetLoaderStrategy();
    if (!loaderStrategy)
    {
      finish(
        Unexpected(ResourceError(utf8_string(ErrorDomainBooeyInternal), 0, url, u8"No loader strategy")));
      return;
    }

    ResourceLoaderOptions options;
    loaderStrategy->LoadResource(*frame, _request, options, krys::move(finish));
  }

  void DocumentLoader::StopLoading() noexcept
  {
    if (!_isLoadingMainResource || _isStopping)
    {
      return;
    }

    // The data, if it arrives, is dropped.
    _isStopping = true;
    _isLoadingMainResource = false;
    if (RefPtr<LocalFrame> frame = ShareRefPtr(_frame.get()))
    {
      ResourceError cancelled(utf8_string(ErrorDomainBooeyInternal), 0, _request.URL(), u8"Cancelled",
                              ResourceError::Type::Cancellation);
      _mainDocumentError = cancelled;
      Ref<DocumentLoader> protectedThis = ShareRef(*this);
      frame->Loader().ReceivedMainResourceError(*this, cancelled);
    }
  }

  void DocumentLoader::MainReceivedError(ResourceError &&error) noexcept
  {
    _isLoadingMainResource = false;
    _mainDocumentError = krys::move(error);
    if (RefPtr<LocalFrame> frame = ShareRefPtr(_frame.get()))
    {
      Ref<DocumentLoader> protectedThis = ShareRef(*this);
      frame->Loader().ReceivedMainResourceError(*this, _mainDocumentError);
    }
  }

  void DocumentLoader::ContinueAfterMainResourceLoad(ResourceResponse &&response,
                                                     Span<const uint8> data) noexcept
  {
    _response = krys::move(response);

    // FIXME: The content policy (responses shown as downloads, or not shown), once there are downloads.
    CommitLoad(data);
  }

  void DocumentLoader::CommitLoad(Span<const uint8> data) noexcept
  {
    RefPtr<LocalFrame> frame = ShareRefPtr(_frame.get());
    if (!frame)
    {
      return;
    }

    Ref<DocumentLoader> protectedThis = ShareRef(*this);
    frame->Loader().CommitProvisionalLoad();
    // The old document's unload handlers may have started another load, which stopped this one.
    if (frame->Loader().GetDocumentLoader() != this)
    {
      return;
    }
    _committed = true;

    _writer.SetMIMEType(utf8_string(_response.MimeType()));
    if (!_writer.Begin(DocumentURL()))
    {
      _isLoadingMainResource = false;
      return;
    }
    _writer.AddData(data);

    // The data is all there.
    _isLoadingMainResource = false;
    _writer.End();
    frame->Loader().FinishedLoading(*this);
  }

  void DocumentLoader::CommitInitialEmptyDocument() noexcept
  {
    _committed = true;
    _response = ResourceResponse(url::URL(u8"about:blank"), u8"text/html", 0, u8"UTF-8");
    _writer.SetMIMEType(u8"text/html");
    if (!_writer.Begin(url::URL(u8"about:blank")))
    {
      return;
    }
    _writer.End();
  }
}
