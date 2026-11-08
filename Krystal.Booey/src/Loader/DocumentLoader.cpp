#include "Krystal.Booey/Loader/DocumentLoader.hpp"
#include "Krystal.Booey/DOM/Algorithms/DocumentAlgorithms.hpp"
#include "Krystal.Booey/DOM/Nodes/Document.hpp"
#include "Krystal.Booey/HTML/EventLoop/EventLoop.hpp"
#include "Krystal.Booey/Loader/FrameLoader.hpp"
#include "Krystal.Booey/Loader/Icons/IconLoader.hpp"
#include "Krystal.Booey/Loader/Icons/LinkIconCollector.hpp"
#include "Krystal.Booey/Loader/LoaderStrategy.hpp"
#include "Krystal.Booey/Loader/LocalFrameLoaderClient.hpp"
#include "Krystal.Booey/Page/LocalFrame.hpp"
#include "Krystal.Booey/Page/Page.hpp"
#include "Krystal.Booey/Page/Settings.hpp"
#include "Krystal.Booey/Platform/Network/DataURLDecoder.hpp"
#include "Krystal.Booey/Platform/PlatformStrategies.hpp"
#include "Krystal.Booey/Platform/SharedBuffer.hpp"
#include "Krystal.Core/Text/ASCII.hpp"

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
    StopLoadingIcons();
  }

  void DocumentLoader::StartIconLoading() noexcept
  {
    RefPtr<LocalFrame> frame = ShareRefPtr(_frame.get());
    auto *document = frame ? frame->Document() : nullptr;
    if (!document || !document->GetSettings().IconLoadingEnabled)
    {
      return;
    }

    auto *page = frame->GetPage();
    if (!page || &page->MainFrame() != frame.get())
    {
      return;
    }

    if (document->GetURL().IsEmpty() || document->GetURL().ProtocolIsAbout())
    {
      return;
    }

    auto icons = CollectLinkIcons(*document);
    if (icons.empty() && document->GetURL().ProtocolIsInHTTPFamily())
    {
      icons.push_back({document->CompleteURL(u8"/favicon.ico"), {}, null});
    }
    if (icons == _linkIcons)
    {
      return;
    }
    _linkIcons = icons;

    // The client may decide at once, or later.
    List<LinkIconLoadDecision> decisions;
    decisions.reserve(icons.size());
    for (const auto &icon : icons)
    {
      auto identifier = _nextIconLoadIdentifier++;
      _iconsPendingLoadDecision.emplace(identifier, icon);
      decisions.push_back({icon, identifier});
    }
    Ref<DocumentLoader> protectedThis = ShareRef(*this);
    frame->Loader().Client().GetLoadDecisionForIcons(decisions);
  }

  void DocumentLoader::DidGetLoadDecisionForIcon(bool decision, uint64 loadIdentifier,
                                                 Func<void(const SharedBuffer *)> completionHandler) noexcept
  {
    auto pending = _iconsPendingLoadDecision.find(loadIdentifier);
    // Not to load, or the loads stopped while the decision was pending: there's no load to perform.
    if (!decision || !_frame || pending == _iconsPendingLoadDecision.end())
    {
      if (pending != _iconsPendingLoadDecision.end())
      {
        _iconsPendingLoadDecision.erase(pending);
      }
      completionHandler(nullptr);
      return;
    }
    auto icon = krys::move(pending->second);
    _iconsPendingLoadDecision.erase(pending);

    auto iconLoader = IconLoader::Create(*this, krys::move(icon.URL));
    _iconLoaders.emplace_back(iconLoader, krys::move(completionHandler));
    iconLoader->StartLoading();
  }

  void DocumentLoader::FinishedLoadingIcon(IconLoader &loader, const SharedBuffer *data) noexcept
  {
    auto found =
      std::ranges::find_if(_iconLoaders, [&](const auto &entry) { return entry.first.get() == &loader; });
    if (found == _iconLoaders.end())
    {
      return;
    }
    auto completionHandler = krys::move(found->second);
    _iconLoaders.erase(found);
    completionHandler(data);
  }

  void DocumentLoader::StopLoadingIcons() noexcept
  {
    _iconsPendingLoadDecision.clear();
    auto iconLoaders = std::exchange(_iconLoaders, {});
    for (auto &[iconLoader, completionHandler] : iconLoaders)
    {
      iconLoader->StopLoading();
      completionHandler(nullptr);
    }
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
      finish(LoadedResource {.Response = ResourceResponse(DocumentURL(), _substituteData->MimeType(),
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

    _writer.SetMIMEType(_response.MimeType());
    if (!_writer.Begin(DocumentURL()))
    {
      _isLoadingMainResource = false;
      return;
    }
    if (RefPtr<dom::Document> document = ShareRefPtr(frame->Document()))
    {
      document->SetLastModified(_response.LastModified());
      // The Content-Language header's first language is the document's language until a content-language
      // pragma sets one, as Chrome takes it.
      auto contentLanguage = _response.HTTPHeaderField(u8"Content-Language");
      contentLanguage = contentLanguage.substr(0, contentLanguage.find(u8','));
      while (!contentLanguage.empty() && text::IsASCIIWhitespace(contentLanguage.front()))
      {
        contentLanguage.remove_prefix(1);
      }
      while (!contentLanguage.empty() && text::IsASCIIWhitespace(contentLanguage.back()))
      {
        contentLanguage.remove_suffix(1);
      }
      if (!contentLanguage.empty())
      {
        document->SetContentLanguage(dom::DOMString(contentLanguage));
      }
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

    // https://html.spec.whatwg.org/#creating-a-new-browsing-context
    // 26. Completely finish loading document.
    // (The document is never parsed, so its readiness is "complete", and it gets no load event.)
    if (RefPtr<dom::Document> document = ShareRefPtr(_frame ? _frame->Document() : nullptr))
    {
      dom::DocumentAlgorithms::UpdateCurrentDocumentReadiness(*document, html::DocumentReadyState::Complete);
    }
  }
}
