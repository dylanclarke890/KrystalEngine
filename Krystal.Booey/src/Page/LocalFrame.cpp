#include "Krystal.Booey/Page/LocalFrame.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/Editing/Editing.hpp"
#include "Krystal.Booey/Editing/VisiblePosition.hpp"
#include "Krystal.Booey/Page/LocalFrameView.hpp"
#include "Krystal.Booey/Page/Page.hpp"
#include "Krystal.Booey/Rendering/HitTestSource.hpp"
#include "Krystal.Booey/Rendering/RenderLayerCompositor.hpp"
#include "Krystal.Booey/Rendering/RenderView.hpp"
#include "Krystal.Booey/Style/Scope.hpp"
#include "Krystal.Core/Assert.hpp"
#include "Krystal.Core/Utils/SetForScope.hpp"

namespace krys::boo
{
  LocalFrame::LocalFrame(Page *page, UniquePtr<LocalFrameLoaderClient> &&loaderClient) noexcept
      : _page(page ? WeakPtr<Page>::WithRef(*page) : WeakPtr<Page>()),
        _eventHandler(CreateUnique<EventHandler>(*this)),
        _loader(CreateUnique<FrameLoader>(*this, krys::move(loaderClient))),
        _navigationScheduler(CreateUnique<NavigationScheduler>(*this))
  {
  }

  LocalFrame::~LocalFrame() noexcept
  {
    if (_document)
    {
      _document->WillBeRemovedFromFrame();
    }
  }

  Ref<LocalFrame> LocalFrame::Create(Page *page, UniquePtr<LocalFrameLoaderClient> loaderClient) noexcept
  {
    return AdoptRef(*new LocalFrame(page, krys::move(loaderClient)));
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

    // A document in the back/forward cache has no render tree until it's shown again.
    if (_document && _view
        && _document->GetBackForwardCacheState() == dom::BackForwardCacheState::NotInBackForwardCache)
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

  VisiblePosition LocalFrame::VisiblePositionForPoint(const IntPoint &framePoint) const noexcept
  {
    HitTestResult result = GetEventHandler().HitTestResultAtPoint(
      LayoutPoint(framePoint), {HitTestRequest::Type::ReadOnly, HitTestRequest::Type::Active,
                                HitTestRequest::Type::AllowVisibleChildFrameContentOnly});
    RefPtr<dom::Node> node = ShareRefPtr(result.InnerNonSharedNode());
    if (!node)
    {
      return {};
    }
    auto *renderer = node->Renderer();
    if (!renderer)
    {
      return {};
    }
    auto visiblePos = renderer->VisiblePositionForPoint(result.LocalPoint(), HitTestSource::User);
    if (visiblePos.IsNull())
    {
      visiblePos = VisiblePosition(FirstPositionInOrBeforeNode(node.get()));
    }
    return visiblePos;
  }

  dom::Document *LocalFrame::DocumentAtPoint(const IntPoint &point) noexcept
  {
    if (!View())
    {
      return nullptr;
    }

    IntPoint pt = View()->WindowToContents(point);
    HitTestResult result = HitTestResult(LayoutPoint(pt));

    if (Document() && Document()->GetRenderView())
    {
      result = GetEventHandler().HitTestResultAtPoint(
        LayoutPoint(pt),
        {HitTestRequest::Type::ReadOnly, HitTestRequest::Type::Active,
         HitTestRequest::Type::DisallowUserAgentShadowContent, HitTestRequest::Type::AllowChildFrameContent});
    }
    return result.InnerNode() ? &result.InnerNode()->NodeDocument() : nullptr;
  }

  void LocalFrame::SetDocument(RefPtr<dom::Document> &&newDocument) noexcept
  {
    krys_assert_msg(!newDocument || !newDocument->Frame(), "a document is in one frame at most");

    if (_documentIsBeingReplaced)
    {
      return;
    }

    SetForScope documentIsBeingReplaced(_documentIsBeingReplaced, true);

    if (auto *page = GetPage(); page && &page->MainFrame() == this)
    {
      page->DidChangeMainDocument(newDocument.get());
    }

    // The mouse's state refers to the old document's nodes.
    _eventHandler->Clear();

    // A document entering the back/forward cache keeps its state, and is destroyed only if the cache drops
    // it.
    if (RefPtr<dom::Document> previousDocument = _document;
        previousDocument
        && previousDocument->GetBackForwardCacheState() != dom::BackForwardCacheState::InBackForwardCache)
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

  float LocalFrame::FrameScaleFactor() const noexcept
  {
    auto *page = GetPage();
    if (!page)
    {
      return 1;
    }

    // Main frame is scaled with respect to the container.
    return page->PageScaleFactor();
  }

  void LocalFrame::DeviceOrPageScaleFactorChanged() noexcept
  {
    if (auto *root = _document ? _document->GetRenderView() : nullptr)
    {
      root->Compositor().DeviceOrPageScaleFactorChanged();
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
