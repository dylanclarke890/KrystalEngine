#include "Krystal.Booey/Page/IntersectionObserver.hpp"
#include "Krystal.Booey/Core/BoxSides.hpp"
#include "Krystal.Booey/Core/MathExtras.hpp"
#include "Krystal.Booey/CSS/Parser/InputStream.hpp"
#include "Krystal.Booey/CSS/Parser/ParserContext.hpp"
#include "Krystal.Booey/CSS/Parser/Tokenizer.hpp"
#include "Krystal.Booey/CSS/Parser/TokenRange.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/Background.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/LengthPercentageDefinitions.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/PrimitiveValue.hpp"
#include "Krystal.Booey/CSS/Properties/PropertyParserState.hpp"
#include "Krystal.Booey/CSS/Values/PrimitiveValue.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/DOM/Element.hpp"
#include "Krystal.Booey/Graphics/FloatQuad.hpp"
#include "Krystal.Booey/Page/IntersectionObserverEntry.hpp"
#include "Krystal.Booey/Page/LocalDOMWindow.hpp"
#include "Krystal.Booey/Page/LocalFrame.hpp"
#include "Krystal.Booey/Page/LocalFrameView.hpp"
#include "Krystal.Booey/Page/Page.hpp"
#include "Krystal.Booey/Rendering/RenderBlock.hpp"
#include "Krystal.Booey/Rendering/RenderInline.hpp"
#include "Krystal.Booey/Rendering/RenderLineBreak.hpp"
#include "Krystal.Booey/Rendering/RenderView.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include "Krystal.Booey/Rendering/VisibleRectContext.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesEvaluation.hpp"
#include "Krystal.Core/Text/StringBuilder.hpp"
#include "Krystal.Core/Visitor.hpp"
#include <algorithm>

namespace krys::boo
{
  namespace
  {
    template <typename T>
    bool RemoveFirstWeak(List<WeakPtr<T>> &list, const T *value) noexcept
    {
      return RemoveFirstMatching(list, [&](auto &weak) { return weak.get() == value; });
    }

    dom::ExceptionOr<IntersectionObserverMarginBox> ParseMargin(const dom::DOMString &margin,
                                                                const utf8_string &marginName) noexcept
    {
      using namespace css::PropertyParserHelpers;

      auto parserContext = css::ParserContext {css::ParserMode::HTMLStandard};
      auto parserState = css::PropertyParserState {
        .Context = parserContext,
      };

      css::InputStream input {dom::DOMString(margin)};
      css::Tokenizer tokenizer {input};
      std::ignore = tokenizer.PumpTokenizer();
      auto tokenRange = tokenizer.Tokens();
      tokenRange.DiscardWhitespace();

      if (tokenRange.IsAtEnd())
      {
        return IntersectionObserverMarginBox {IntersectionObserverMarginEdge::Fixed {0}};
      }

      auto consumeEdge = [&] -> dom::ExceptionOr<IntersectionObserverMarginEdge>
      {
        auto parsedValue =
          css::PrimitiveValueResolver<css::LengthPercentage<>>::ConsumeAndResolve(tokenRange, parserState);

        if (!parsedValue || parsedValue->IsCalculated())
        {
          return dom::Exception(dom::ExceptionCode::SyntaxError,
                                u8"Failed to construct 'IntersectionObserver': " + marginName
                                  + u8" must be specified in pixels or percent.");
        }

        if (parsedValue->IsPercentage())
        {
          return IntersectionObserverMarginEdge {IntersectionObserverMarginEdge::Percentage {
            parsedValue->ResolveAsPercentageNoConversionDataRequired<float>()}};
        }

        // FIXME: This should support all absolute length units, not just px.
        // Spec states: "Similar to the CSS margin property, this is a string of 1-4 components, each either
        // an *absolute length* or a percentage."
        // https://w3c.github.io/IntersectionObserver/#dom-intersectionobserverinit-rootmargin
        if (parsedValue->IsPx())
        {
          return IntersectionObserverMarginEdge {IntersectionObserverMarginEdge::Fixed {
            parsedValue->ResolveAsLengthNoConversionDataRequired<float>()}};
        }

        return dom::Exception(dom::ExceptionCode::SyntaxError,
                              u8"Failed to construct 'IntersectionObserver': " + marginName
                                + u8" must be specified in pixels or percent.");
      };

      auto edge1 = consumeEdge();
      if (edge1.HasException())
      {
        return edge1.ReleaseException();
      }

      if (tokenRange.IsAtEnd())
      {
        return CompleteQuad<IntersectionObserverMarginBox>(edge1.ReleaseValue());
      }

      auto edge2 = consumeEdge();
      if (edge2.HasException())
      {
        return edge2.ReleaseException();
      }

      if (tokenRange.IsAtEnd())
      {
        return CompleteQuad<IntersectionObserverMarginBox>(edge1.ReleaseValue(), edge2.ReleaseValue());
      }

      auto edge3 = consumeEdge();
      if (edge3.HasException())
      {
        return edge3.ReleaseException();
      }

      if (tokenRange.IsAtEnd())
      {
        return CompleteQuad<IntersectionObserverMarginBox>(edge1.ReleaseValue(), edge2.ReleaseValue(),
                                                           edge3.ReleaseValue());
      }

      auto edge4 = consumeEdge();
      if (edge4.HasException())
      {
        return edge4.ReleaseException();
      }

      if (!tokenRange.IsAtEnd())
      {
        return dom::Exception(dom::ExceptionCode::SyntaxError,
                              u8"Failed to construct 'IntersectionObserver': Extra text found at the end of "
                                + marginName + u8".");
      }

      return IntersectionObserverMarginBox {edge1.ReleaseValue(), edge2.ReleaseValue(), edge3.ReleaseValue(),
                                            edge4.ReleaseValue()};
    }

    dom::DOMString MarginBoxToString(const IntersectionObserverMarginBox &marginBox) noexcept
    {
      StringBuilder stringBuilder;
      for (auto side : AllBoxSides)
      {
        auto &edge = marginBox.At(side);
        auto separator = side != BoxSide::Left ? u8" " : u8"";
        if (auto percentage = edge.TryPercentage())
        {
          stringBuilder.Append(static_cast<int>(percentage->Value), u8"%", separator);
        }
        else
        {
          stringBuilder.Append(static_cast<int>(edge.TryFixed()->ResolveZoom(style::ZoomNeeded {})), u8"px",
                               separator);
        }
      }
      return krys::move(stringBuilder).ToString();
    }

    void ExpandRootBoundsWithRootMargin(FloatRect &rootBounds,
                                        const IntersectionObserverMarginBox &rootMargin,
                                        float zoomFactor) noexcept
    {
      auto zoomAdjustedLength =
        [](const IntersectionObserverMarginEdge &edge, float maximumValue, float zoomFactor)
      {
        if (auto percentage = edge.TryPercentage())
        {
          return style::Evaluate<float>(*percentage, maximumValue);
        }
        return style::Evaluate<float>(*edge.TryFixed(), style::ZoomNeeded {}) * zoomFactor;
      };

      auto rootMarginEdges = FloatBoxExtent {
        zoomAdjustedLength(rootMargin.Top(), rootBounds.Height(), zoomFactor),
        zoomAdjustedLength(rootMargin.Right(), rootBounds.Width(), zoomFactor),
        zoomAdjustedLength(rootMargin.Bottom(), rootBounds.Height(), zoomFactor),
        zoomAdjustedLength(rootMargin.Left(), rootBounds.Width(), zoomFactor),
      };

      rootBounds.Expand(rootMarginEdges);
    }

    // There's no frame tree, so the renderer is always in the main frame, which is where the rect stops: its
    // visible rect in the render view is the rect in the root's contents.
    Maybe<LayoutRect>
      ComputeClippedRectInRootContentsSpace(const LayoutRect &rect, const RenderElement &renderer,
                                            Maybe<IntersectionObserverMarginBox> scrollMargin) noexcept
    {
      auto visibleRects = renderer.ComputeVisibleRectsInContainer(
        RenderObject::RepaintRects(rect), &renderer.View(),
        {
          .HasPositionFixedDescendant = false,
          .DirtyRectIsFlipped = false,
          .DescendantNeedsEnclosingIntRect = false,
          .Options = {VisibleRectContext::Option::UseEdgeInclusiveIntersection,
                      VisibleRectContext::Option::ApplyCompositedClips,
                      VisibleRectContext::Option::ApplyCompositedContainerScrolls},
          .ScrollMargin = krys::move(scrollMargin),
        });
      if (!visibleRects)
      {
        return null;
      }
      return visibleRects->ClippedOverflowRect;
    }
  }

  dom::ExceptionOr<Ref<IntersectionObserver>>
    IntersectionObserver::Create(dom::Document &document, IntersectionObserverCallback &&callback,
                                 Init &&init, IncludeObscuredInsets includeObscuredInsets) noexcept
  {
    RefPtr<dom::ContainerNode> root;
    if (init.Root)
    {
      root = SwitchOn(*init.Root, [](auto &elementOrDocument) -> RefPtr<dom::ContainerNode>
                      { return elementOrDocument; });
    }

    auto rootMarginOrException = ParseMargin(init.RootMargin, u8"rootMargin");
    if (rootMarginOrException.HasException())
    {
      return rootMarginOrException.ReleaseException();
    }

    auto scrollMarginOrException = ParseMargin(init.ScrollMargin, u8"scrollMargin");
    if (scrollMarginOrException.HasException())
    {
      return scrollMarginOrException.ReleaseException();
    }

    List<double> thresholds;
    SwitchOn(
      init.Threshold, [&thresholds](double initThreshold) { thresholds.push_back(initThreshold); },
      [&thresholds](List<double> &initThresholds) { thresholds = krys::move(initThresholds); });

    if (thresholds.empty())
    {
      thresholds.push_back(0.f);
    }

    for (auto threshold : thresholds)
    {
      if (!(threshold >= 0 && threshold <= 1))
      {
        return dom::Exception(
          dom::ExceptionCode::RangeError,
          u8"Failed to construct 'IntersectionObserver': all thresholds must lie in the range [0.0, 1.0].");
      }
    }

    return AdoptRef(*new IntersectionObserver(
      document, krys::move(callback), root.get(), rootMarginOrException.ReleaseValue(),
      scrollMarginOrException.ReleaseValue(), krys::move(thresholds), includeObscuredInsets));
  }

  IntersectionObserver::IntersectionObserver(dom::Document &document, IntersectionObserverCallback &&callback,
                                             dom::ContainerNode *root,
                                             IntersectionObserverMarginBox &&parsedRootMargin,
                                             IntersectionObserverMarginBox &&parsedScrollMargin,
                                             List<double> &&thresholds,
                                             IncludeObscuredInsets includeObscuredInsets) noexcept
      : _document(CreateWeakPtr(&document)), _root(CreateWeakPtr(root)),
        _rootMargin(krys::move(parsedRootMargin)), _scrollMargin(krys::move(parsedScrollMargin)),
        _thresholds(krys::move(thresholds)), _callback(krys::move(callback)),
        _includeObscuredInsets(includeObscuredInsets)
  {
    if (auto *rootDocument = DynamicDowncast<dom::Document>(root))
    {
      auto &observerData = rootDocument->EnsureIntersectionObserverData();
      observerData.Observers.push_back(CreateWeakPtr(this));
    }
    else if (root)
    {
      auto &observerData = Downcast<dom::Element>(*root).EnsureIntersectionObserverData();
      observerData.Observers.push_back(CreateWeakPtr(this));
    }
    else if (auto *frame = document.Frame())
    {
      if (auto *page = frame->GetPage())
      {
        _implicitRootDocument = CreateWeakPtr(page->MainFrame().Document());
      }
    }

    std::ranges::sort(_thresholds);
  }

  IntersectionObserver::~IntersectionObserver() noexcept
  {
    auto *root = _root.get();
    if (auto *document = DynamicDowncast<dom::Document>(root))
    {
      RemoveFirstWeak(document->IntersectionObserverDataIfExists()->Observers, this);
    }
    else if (root)
    {
      RemoveFirstWeak(Downcast<dom::Element>(*root).IntersectionObserverDataIfExists()->Observers, this);
    }
    Disconnect();
  }

  dom::Document *IntersectionObserver::TrackingDocument() const noexcept
  {
    return _root ? &_root->NodeDocument() : _implicitRootDocument.get();
  }

  dom::DOMString IntersectionObserver::RootMargin() const noexcept
  {
    return MarginBoxToString(_rootMargin);
  }

  dom::DOMString IntersectionObserver::ScrollMargin() const noexcept
  {
    return MarginBoxToString(_scrollMargin);
  }

  bool IntersectionObserver::IsObserving(const dom::Element &element) const noexcept
  {
    return std::ranges::any_of(_observationTargets, [&](auto &target) { return target.get() == &element; });
  }

  void IntersectionObserver::Observe(dom::Element &target) noexcept
  {
    if (!TrackingDocument() || !_callback || IsObserving(target))
    {
      return;
    }

    target.EnsureIntersectionObserverData().Registrations.push_back({CreateWeakPtr(this), null});
    bool hadObservationTargets = HasObservationTargets();
    _observationTargets.push_back(CreateWeakPtr(&target));

    // Per the specification, we should dispatch at least one observation for the target. For this reason, we
    // make sure to keep the target alive until this first observation.
    _targetsWaitingForFirstObservation.push_back(ShareRef(target));

    RefPtr<dom::Document> document = ShareRefPtr(TrackingDocument());
    if (!hadObservationTargets)
    {
      document->AddIntersectionObserver(*this);
    }
    document->ScheduleInitialIntersectionObservationUpdate();
  }

  void IntersectionObserver::Unobserve(dom::Element &target) noexcept
  {
    if (!RemoveTargetRegistration(target))
    {
      return;
    }

    bool removed = RemoveFirstWeak(_observationTargets, &target);
    krys_debug_assert(removed);
    (void)removed;
    RemoveFirstMatching(_targetsWaitingForFirstObservation,
                        [&](auto &pendingTarget) { return pendingTarget.get() == &target; });

    if (!HasObservationTargets())
    {
      if (auto *document = TrackingDocument())
      {
        document->RemoveIntersectionObserver(*this);
      }
    }
  }

  void IntersectionObserver::Disconnect() noexcept
  {
    if (!HasObservationTargets())
    {
      krys_debug_assert(_targetsWaitingForFirstObservation.empty());
      return;
    }

    RemoveAllTargets();
    if (auto *document = TrackingDocument())
    {
      document->RemoveIntersectionObserver(*this);
    }
  }

  List<Ref<IntersectionObserverEntry>> IntersectionObserver::TakeRecords() noexcept
  {
    return std::exchange(_queuedEntries, {});
  }

  void IntersectionObserver::TargetDestroyed(dom::Element &target) noexcept
  {
    RemoveFirstWeak(_observationTargets, &target);
    RemoveFirstMatching(_targetsWaitingForFirstObservation,
                        [&](auto &pendingTarget) { return pendingTarget.get() == &target; });
    if (!HasObservationTargets())
    {
      if (auto *document = TrackingDocument())
      {
        document->RemoveIntersectionObserver(*this);
      }
    }
  }

  bool IntersectionObserver::RemoveTargetRegistration(dom::Element &target) noexcept
  {
    auto *observerData = target.IntersectionObserverDataIfExists();
    if (!observerData)
    {
      return false;
    }

    return RemoveFirstMatching(observerData->Registrations,
                               [this](auto &registration) { return registration.Observer.get() == this; });
  }

  void IntersectionObserver::RemoveAllTargets() noexcept
  {
    for (auto &target : _observationTargets)
    {
      bool removed = RemoveTargetRegistration(*target);
      krys_debug_assert(removed);
      (void)removed;
    }
    _observationTargets.clear();
    _targetsWaitingForFirstObservation.clear();
  }

  void IntersectionObserver::RootDestroyed() noexcept
  {
    krys_debug_assert(_root);
    Disconnect();
    _root = nullptr;
  }

  auto IntersectionObserver::ComputeIntersectionState(const IntersectionObserverRegistration &registration,
                                                      LocalFrameView &hostFrameView, dom::Element &target,
                                                      ApplyRootMargin applyRootMargin) const noexcept
    -> IntersectionObservationState
  {
    bool isFirstObservation = !registration.PreviousThresholdIndex;

    float rootUsedZoom = 1.0;
    // Only available in explicit root situation.
    RenderBlock *rootRenderer = nullptr;
    RenderElement *targetRenderer = nullptr;
    IntersectionObservationState intersectionState;

    auto layoutViewportRectForIntersection = [&]
    {
      // IncludeObscuredInsets::Yes is only used by ContentVisibilityDocumentState, which tracks the
      // visibility of an element wrt. its document. There are no obscured insets, so it's the layout viewport
      // either way.
      (void)_includeObscuredInsets;
      return hostFrameView.LayoutViewportRect();
    };

    auto computeRootBounds = [&]()
    {
      targetRenderer = target.Renderer();
      if (!targetRenderer)
      {
        return;
      }

      if (Root())
      {
        if (TrackingDocument() != &target.NodeDocument())
        {
          return;
        }

        if (!Root()->Renderer())
        {
          return;
        }

        rootRenderer = DynamicDowncast<RenderBlock>(Root()->Renderer());
        if (!rootRenderer || !rootRenderer->IsContainingBlockAncestorFor(*targetRenderer))
        {
          return;
        }

        intersectionState.CanComputeIntersection = true;
        if (Root() == &target.NodeDocument())
        {
          intersectionState.RootBounds = FloatRect(layoutViewportRectForIntersection());
        }
        else if (rootRenderer->HasNonVisibleOverflow())
        {
          intersectionState.RootBounds = FloatRect(rootRenderer->PaddingBoxRect());
        }
        else
        {
          intersectionState.RootBounds = FloatRect(FloatPoint(), FloatSize(rootRenderer->Size()));
        }

        rootUsedZoom = rootRenderer->Style().UsedZoom();

        return;
      }

      // FIXME: Handle the case of an implicit-root observer that has a target in a different frame tree.
      if (targetRenderer->GetDocument().Frame() != &hostFrameView.Frame())
      {
        return;
      }

      intersectionState.CanComputeIntersection = true;
      rootUsedZoom = hostFrameView.GetRenderView()->Style().UsedZoom();
      intersectionState.RootBounds = FloatRect(layoutViewportRectForIntersection());
    };

    computeRootBounds();
    if (!intersectionState.CanComputeIntersection)
    {
      intersectionState.ObservationChanged = isFirstObservation || *registration.PreviousThresholdIndex != 0;
      return intersectionState;
    }

    if (applyRootMargin == ApplyRootMargin::Yes)
    {
      ExpandRootBoundsWithRootMargin(intersectionState.RootBounds, ScrollMarginBox(), rootUsedZoom);
      ExpandRootBoundsWithRootMargin(intersectionState.RootBounds, RootMarginBox(), rootUsedZoom);
    }

    auto localTargetBounds = [&]() -> LayoutRect
    {
      if (auto *renderBox = DynamicDowncast<RenderBox>(*targetRenderer))
      {
        return renderBox->BorderBoundingBox();
      }

      if (Is<RenderInline>(targetRenderer))
      {
        List<LayoutRect> rects;
        targetRenderer->BoundingRects(rects, {});
        return UnionRect(rects);
      }

      if (auto *renderLineBreak = DynamicDowncast<RenderLineBreak>(targetRenderer))
      {
        return LayoutRect(renderLineBreak->LinesBoundingBox());
      }

      // FIXME: Implement for SVG etc.
      return {};
    }();

    auto rootRelativeTargetRect = [&]() -> Maybe<LayoutRect>
    {
      if (targetRenderer->IsSkippedContent())
      {
        return null;
      }

      if (Root())
      {
        auto result = targetRenderer->ComputeVisibleRectsInContainer(
          RenderObject::RepaintRects(localTargetBounds), rootRenderer,
          {
            .HasPositionFixedDescendant = false,
            .DirtyRectIsFlipped = false,
            .DescendantNeedsEnclosingIntRect = false,
            .Options = {VisibleRectContext::Option::UseEdgeInclusiveIntersection,
                        VisibleRectContext::Option::ApplyCompositedClips,
                        VisibleRectContext::Option::ApplyCompositedContainerScrolls},
            .ScrollMargin = {},
          });
        if (!result)
        {
          return null;
        }
        return result->ClippedOverflowRect;
      }

      return ComputeClippedRectInRootContentsSpace(localTargetBounds, *targetRenderer, ScrollMarginBox());
    }();

    auto rootLocalIntersectionRect = intersectionState.RootBounds;
    intersectionState.IsIntersecting =
      rootRelativeTargetRect
      && rootLocalIntersectionRect.EdgeInclusiveIntersect(FloatRect(*rootRelativeTargetRect));

    if (isFirstObservation || intersectionState.IsIntersecting)
    {
      intersectionState.AbsoluteTargetRect =
        targetRenderer->LocalToAbsoluteQuad(FloatQuad(FloatRect(localTargetBounds))).BoundingBox();
    }

    if (intersectionState.IsIntersecting)
    {
      // If implicit root, rootLocalIntersectionRect is already in absolute coordinates. There's no frame
      // tree, so the target is in the root's frame, and the rect needs no mapping between frames.
      intersectionState.AbsoluteIntersectionRect =
        Root() ? rootRenderer->LocalToAbsoluteQuad(FloatQuad(rootLocalIntersectionRect)).BoundingBox()
               : rootLocalIntersectionRect;

      intersectionState.IsIntersecting = intersectionState.AbsoluteIntersectionRect->EdgeInclusiveIntersect(
        *intersectionState.AbsoluteTargetRect);
    }

    if (intersectionState.IsIntersecting)
    {
      float absTargetArea = intersectionState.AbsoluteTargetRect->Area();
      if (absTargetArea)
      {
        intersectionState.IntersectionRatio =
          intersectionState.AbsoluteIntersectionRect->Area() / absTargetArea;
      }
      else
      {
        intersectionState.IntersectionRatio = 1;
      }

      size_t thresholdIndex = 0;
      for (auto threshold : Thresholds())
      {
        if (!(threshold <= intersectionState.IntersectionRatio
              || AreEssentiallyEqual<float>(static_cast<float>(threshold),
                                            intersectionState.IntersectionRatio)))
        {
          break;
        }
        ++thresholdIndex;
      }

      intersectionState.ThresholdIndex = thresholdIndex;
    }

    intersectionState.ObservationChanged =
      isFirstObservation || intersectionState.ThresholdIndex != registration.PreviousThresholdIndex;
    if (intersectionState.ObservationChanged)
    {
      if (Root())
      {
        intersectionState.AbsoluteRootBounds =
          rootRenderer->LocalToAbsoluteQuad(FloatQuad(intersectionState.RootBounds)).BoundingBox();
      }
      else
      {
        // Implicit root (the main frame's document): intersectionState.RootBounds is already in absolute
        // coordinates.
        intersectionState.AbsoluteRootBounds = intersectionState.RootBounds;
      }

      if (!intersectionState.AbsoluteTargetRect)
      {
        intersectionState.AbsoluteTargetRect =
          targetRenderer->LocalToAbsoluteQuad(FloatQuad(FloatRect(localTargetBounds))).BoundingBox();
      }
    }

    return intersectionState;
  }

  auto IntersectionObserver::UpdateObservations(const LocalFrame &hostFrame) noexcept -> NeedNotify
  {
    RefPtr<LocalFrameView> hostFrameView = ShareRefPtr(hostFrame.View());
    if (!hostFrameView)
    {
      return NeedNotify::No;
    }

    auto timestamp = NowTimestamp();
    if (!timestamp)
    {
      return NeedNotify::No;
    }

    auto needNotify = NeedNotify::No;

    for (auto &target : ObservationTargets())
    {
      auto &targetRegistrations = target->IntersectionObserverDataIfExists()->Registrations;
      auto registration = std::ranges::find_if(targetRegistrations, [&](auto &registration)
                                               { return registration.Observer.get() == this; });
      krys_debug_assert(registration != targetRegistrations.end());

      // There's no frame tree, so every observation is in the host frame's origin, and the root margin
      // applies.
      auto intersectionState =
        ComputeIntersectionState(*registration, *hostFrameView, *target, ApplyRootMargin::Yes);

      if (intersectionState.ObservationChanged)
      {
        FloatRect targetBoundingClientRect;
        FloatRect clientIntersectionRect;
        FloatRect clientRootBounds;
        if (intersectionState.CanComputeIntersection)
        {
          krys_debug_assert(intersectionState.AbsoluteTargetRect);
          krys_debug_assert(intersectionState.AbsoluteRootBounds);

          auto *targetFrameView = target->NodeDocument().View();
          targetBoundingClientRect = targetFrameView->AbsoluteToClientRect(
            *intersectionState.AbsoluteTargetRect, target->Renderer()->Style().UsedZoom());
          clientRootBounds =
            hostFrameView->AbsoluteToLayoutViewportRect(*intersectionState.AbsoluteRootBounds);

          if (intersectionState.IsIntersecting)
          {
            krys_debug_assert(intersectionState.AbsoluteIntersectionRect);
            clientIntersectionRect = targetFrameView->AbsoluteToClientRect(
              *intersectionState.AbsoluteIntersectionRect, target->Renderer()->Style().UsedZoom());
          }
        }

        AppendQueuedEntry(IntersectionObserverEntry::Create({
          .Time = *timestamp,
          .RootBounds = dom::DOMRectInit {clientRootBounds.X(), clientRootBounds.Y(),
                                          clientRootBounds.Width(), clientRootBounds.Height()},
          .BoundingClientRect = {targetBoundingClientRect.X(), targetBoundingClientRect.Y(),
                                 targetBoundingClientRect.Width(), targetBoundingClientRect.Height()},
          .IntersectionRect = {clientIntersectionRect.X(), clientIntersectionRect.Y(),
                               clientIntersectionRect.Width(), clientIntersectionRect.Height()},
          .IntersectionRatio = intersectionState.IntersectionRatio,
          .Target = ShareRefPtr(target.get()),
          .IsIntersecting = intersectionState.ThresholdIndex > 0,
        }));

        needNotify = NeedNotify::Yes;
        registration->PreviousThresholdIndex = intersectionState.ThresholdIndex;
      }
    }

    return needNotify;
  }

  Maybe<double> IntersectionObserver::NowTimestamp() const noexcept
  {
    if (!_callback || !_document)
    {
      return null;
    }

    auto *window = _document->Window();
    if (!window)
    {
      return null;
    }
    return window->FrozenNowTimestamp();
  }

  void IntersectionObserver::AppendQueuedEntry(Ref<IntersectionObserverEntry> &&entry) noexcept
  {
    krys_debug_assert(entry->Target());
    _queuedEntries.push_back(krys::move(entry));
  }

  void IntersectionObserver::Notify() noexcept
  {
    if (_queuedEntries.empty())
    {
      return;
    }

    auto records = TakeRecords();
    auto targetsWaitingForFirstObservation = std::exchange(_targetsWaitingForFirstObservation, {});

    if (!_callback || !_document)
    {
      return;
    }

    Ref<IntersectionObserver> protectedThis = ShareRef(*this);
    _callback(records, *this);
  }
}
