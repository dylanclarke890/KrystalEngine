#include "Krystal.Booey/Page/LocalFrame.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/Page/LocalFrameView.hpp"
#include "Krystal.Booey/Style/Scope.hpp"
#include "Krystal.Core/Assert.hpp"
#include "Krystal.Core/Utils/SetForScope.hpp"

namespace krys::boo
{
  LocalFrame::LocalFrame() noexcept : _eventHandler(CreateUnique<EventHandler>(*this))
  {
  }

  LocalFrame::~LocalFrame() noexcept
  {
    if (_document)
    {
      _document->WillBeRemovedFromFrame();
    }
  }

  Ref<LocalFrame> LocalFrame::Create() noexcept
  {
    return AdoptRef(*new LocalFrame);
  }

  void LocalFrame::SetView(RefPtr<LocalFrameView> &&view) noexcept
  {
    if (_view)
    {
      _view->LayoutContext().UnscheduleLayout();
    }

    // The document's render tree belongs to the view it's laid out in.
    if (_document && _document->HasLivingRenderTree())
    {
      _document->DestroyRenderTree();
    }

    _view = krys::move(view);

    if (_document && _view)
    {
      _document->CreateRenderTree();
    }
  }

  void LocalFrame::CreateView(const IntSize &viewportSize, const IntSize &fixedLayoutSize,
                              bool useFixedLayout) noexcept
  {
    SetView(nullptr);

    Ref frameView = LocalFrameView::Create(*this, viewportSize);
    frameView->SetFixedLayoutSize(fixedLayoutSize);
    frameView->SetUseFixedLayout(useFixedLayout);

    SetView(krys::move(frameView));
  }

  void LocalFrame::SetDocument(RefPtr<dom::Document> &&newDocument) noexcept
  {
    krys_assert_msg(!newDocument || !newDocument->Frame(), "a document is in one frame at most");

    if (_documentIsBeingReplaced)
    {
      return;
    }

    SetForScope documentIsBeingReplaced(_documentIsBeingReplaced, true);

    // The mouse's state refers to the old document's nodes.
    _eventHandler->Clear();

    if (RefPtr<dom::Document> previousDocument = _document)
    {
      previousDocument->WillBeRemovedFromFrame();
    }

    _document = newDocument;

    // Don't use _document because it can be overwritten and we want to guarantee
    // that the document is not destroyed during this function call.
    if (newDocument)
    {
      newDocument->DidBecomeCurrentDocumentInFrame(*this);
    }
  }

  void LocalFrame::SetPageZoomFactor(float factor) noexcept
  {
    SetPageAndTextZoomFactors(factor, _textZoomFactor);
  }

  void LocalFrame::SetTextZoomFactor(float factor) noexcept
  {
    SetPageAndTextZoomFactors(_pageZoomFactor, factor);
  }

  void LocalFrame::SetPageAndTextZoomFactors(float pageZoomFactor, float textZoomFactor) noexcept
  {
    if (_pageZoomFactor == pageZoomFactor && _textZoomFactor == textZoomFactor)
    {
      return;
    }

    _pageZoomFactor = pageZoomFactor;
    _textZoomFactor = textZoomFactor;

    RefPtr<dom::Document> document = _document;
    if (!document)
    {
      return;
    }

    // FIXME: Respect zoomAndPan="disabled" in standalone SVG documents, once SVG elements have it.

    document->ResolveStyle(dom::Document::ResolveStyleType::Rebuild);

    if (RefPtr<LocalFrameView> view = _view)
    {
      auto &layoutContext = view->LayoutContext();
      if (document->GetRenderView() && layoutContext.NeedsLayout() && layoutContext.DidFirstLayout())
      {
        layoutContext.Layout();
      }
    }
  }

  void LocalFrame::SetMediaEnvironment(const css::mq::MediaEnvironment &environment) noexcept
  {
    if (_mediaEnvironment == environment)
    {
      return;
    }

    bool deviceScaleFactorChanged = _mediaEnvironment.DeviceScaleFactor != environment.DeviceScaleFactor;
    _mediaEnvironment = environment;

    RefPtr<dom::Document> document = _document;
    if (!document)
    {
      return;
    }

    // The root style holds the device scale factor.
    if (deviceScaleFactorChanged)
    {
      document->ScheduleFullStyleRebuild();
    }

    document->StyleScope().DidChangeStyleSheetEnvironment();
  }
}
