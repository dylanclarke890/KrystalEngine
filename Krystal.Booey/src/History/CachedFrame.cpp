#include "Krystal.Booey/History/CachedFrame.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/Loader/DocumentLoader.hpp"
#include "Krystal.Booey/Loader/FrameLoader.hpp"
#include "Krystal.Booey/Page/LocalFrame.hpp"
#include "Krystal.Booey/Page/LocalFrameView.hpp"
#include "Krystal.Booey/SVG/SVGDocumentExtensions.hpp"

namespace krys::boo
{
  CachedFrame::CachedFrame(LocalFrame &frame) noexcept
      : _document(ShareRefPtr(frame.Document())),
        _documentLoader(ShareRefPtr(frame.Loader().GetDocumentLoader())), _view(ShareRefPtr(frame.View())),
        _url(frame.Document()->GetURL())
  {
    RefPtr<dom::Document> document = _document;
    krys_debug_assert(document && _documentLoader && _view);
    krys_debug_assert(document->GetBackForwardCacheState() == dom::BackForwardCacheState::InBackForwardCache);

    // Active DOM objects must be suspended before we cache the frame script data.
    document->Suspend(dom::ReasonForSuspension::BackForwardCache);

    // FIXME: The window's suspension for its observers, the view's layout milestones and the client's
    // platform data, once there are those.

    document->DetachFromCachedFrame();
  }

  CachedFrame::~CachedFrame() noexcept
  {
    // CachedFrames should always have had destroy() called by their parent CachedPage
    krys_debug_assert(!_document);
  }

  dom::Document *CachedFrame::Document() const noexcept
  {
    return _document.get();
  }

  DocumentLoader *CachedFrame::GetDocumentLoader() const noexcept
  {
    return _documentLoader.get();
  }

  LocalFrameView *CachedFrame::View() const noexcept
  {
    return _view.get();
  }

  void CachedFrame::Open() noexcept
  {
    krys_debug_assert(_view && _document);
    _view->Frame().Loader().Open(*this);
  }

  void CachedFrame::Restore() noexcept
  {
    RefPtr<dom::Document> document = _document;
    krys_debug_assert(document->View() == _view.get());

    if (auto *svgExtensions = document->SVGExtensionsIfExists())
    {
      svgExtensions->UnpauseAnimations();
    }

    document->Resume(dom::ReasonForSuspension::BackForwardCache);

    if (auto *frame = document->Frame())
    {
      frame->Loader().Client().DidRestoreFromBackForwardCache();
    }
  }

  void CachedFrame::Clear() noexcept
  {
    if (!_document)
    {
      return;
    }

    // clear() should only be called for Frames representing documents that are no longer in the back/forward
    // cache. This means the CachedFrame has been:
    // 1 - Successfully restore()'d by going back/forward.
    // 2 - destroy()'ed because the BackForwardCache is pruning or the WebView was closed.
    krys_debug_assert(_document->GetBackForwardCacheState()
                      == dom::BackForwardCacheState::NotInBackForwardCache);

    _document = nullptr;
    _documentLoader = nullptr;
    _view = nullptr;
    _url = {};
  }

  void CachedFrame::Destroy() noexcept
  {
    RefPtr<dom::Document> document = _document;
    if (!document)
    {
      return;
    }

    // Only CachedFrames that are still in the BackForwardCache should be destroyed in this manner
    krys_debug_assert(document->GetBackForwardCacheState() == dom::BackForwardCacheState::InBackForwardCache);
    krys_debug_assert(!document->Frame());

    document->SetBackForwardCacheState(dom::BackForwardCacheState::NotInBackForwardCache);
    document->WillBeRemovedFromFrame();

    Clear();
  }
}
