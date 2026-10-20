#include "Krystal.Booey/Page/Page.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/HTML/Algorithms/FocusAlgorithms.hpp"
#include "Krystal.Booey/HTML/EventLoop.hpp"
#include "Krystal.Booey/Page/EventHandler.hpp"
#include "Krystal.Booey/Page/LocalFrame.hpp"
#include "Krystal.Core/Types/List.hpp"

namespace krys::boo
{
  namespace
  {
    constexpr EnumSet<RenderingUpdateStep> AllRenderingUpdateSteps {
      RenderingUpdateStep::FlushAutofocusCandidates,
      RenderingUpdateStep::Resize,
      RenderingUpdateStep::Scroll,
      RenderingUpdateStep::MediaQueryEvaluation,
      RenderingUpdateStep::Animations,
      RenderingUpdateStep::AnimationFrameCallbacks,
      RenderingUpdateStep::FocusFixup,
      RenderingUpdateStep::CursorUpdate};
  }

  Page::Page() noexcept = default;

  Page::~Page() noexcept = default;

  Ref<Page> Page::Create() noexcept
  {
    Ref<Page> page = AdoptRef(*new Page);
    page->_mainFrame = LocalFrame::Create(page.get());
    return page;
  }

  void Page::ScheduleRenderingUpdate(EnumSet<RenderingUpdateStep> requestedSteps) noexcept
  {
    if (!_renderingUpdateDepth)
    {
      // FIXME: The rendering update scheduler (display refresh), once the host reports refreshes: the host
      // runs a scheduled update at its next frame.
      _renderingUpdateIsScheduled = true;
      return;
    }

    // Steps that already ran in this update run in the next one.
    for (auto step : requestedSteps)
    {
      if (!_renderingUpdateRemainingSteps.Contains(step))
      {
        _unfulfilledRequestedSteps.Add(step);
      }
    }
  }

  // https://html.spec.whatwg.org/#update-the-rendering
  void Page::UpdateRendering() noexcept
  {
    Ref protectedThis = ShareRef(*this);

    // This function is not reentrant, e.g. a requestAnimationFrame callback may force a rendering update.
    if (_renderingUpdateDepth)
    {
      LayoutIfNeeded();
      return;
    }

    ++_renderingUpdateDepth;
    _unfulfilledRequestedSteps = {};
    _renderingUpdateRemainingSteps = AllRenderingUpdateSteps;
    _lastRenderingUpdateTimestamp = html::EventLoop::Current().Now();
    _renderingUpdateIsScheduled = false;

    LayoutIfNeeded();

    // FIXME: Restoring the scroll position and view state, revealing documents and freezing the documents'
    // timestamps, once there are navigation, visibility and windows.

    RunProcessingStep(RenderingUpdateStep::FlushAutofocusCandidates, [](dom::Document &document)
                      { html::FocusAlgorithms::FlushAutofocusCandidates(document); });

    // FIXME: The resize steps fire `resize` at the window, once there's a window.
    RunProcessingStep(RenderingUpdateStep::Resize, [](dom::Document &) {});

    RunProcessingStep(RenderingUpdateStep::Scroll,
                      [](dom::Document &document) { document.RunScrollSteps(); });

    RunProcessingStep(RenderingUpdateStep::MediaQueryEvaluation,
                      [](dom::Document &document) { document.EvaluateMediaQueriesAndReportChanges(); });

    // FIXME: Fullscreen events, once there's fullscreen.
    RunProcessingStep(RenderingUpdateStep::Animations,
                      [](dom::Document &document) { document.UpdateAnimationsAndSendEvents(); });

    // FIXME: Running the animation frame callbacks (requestAnimationFrame), once there's scripting.
    RunProcessingStep(RenderingUpdateStep::AnimationFrameCallbacks, [](dom::Document &) {});

    LayoutIfNeeded();

    // FIXME: Resize observations, once there are resize observers.

    // https://html.spec.whatwg.org/#focus-fixup-rule
    RunProcessingStep(RenderingUpdateStep::FocusFixup,
                      [](dom::Document &document)
                      {
                        // If the focused area of the document is not a focusable area, run the focusing steps
                        // for the document's viewport.
                        if (RefPtr<dom::Element> focusedElement = ShareRefPtr(document.FocusedElement()))
                        {
                          if (!html::FocusAlgorithms::IsFocusableArea(*focusedElement))
                          {
                            html::FocusAlgorithms::FocusingSteps(document);
                          }
                        }
                      });

    // FIXME: Content relevancy, view transitions and intersection observations, once there are any.

    LayoutIfNeeded();
    DoAfterUpdateRendering();

    --_renderingUpdateDepth;
    RenderingUpdateCompleted();
  }

  void Page::DoAfterUpdateRendering() noexcept
  {
    // Code here should do once-per-frame work that needs to be done before painting, and requires
    // layout to be up-to-date. It should not run script, trigger layout, or dirty layout.
    RunProcessingStep(RenderingUpdateStep::CursorUpdate,
                      [](dom::Document &document)
                      {
                        if (auto *frame = document.Frame())
                        {
                          frame->GetEventHandler().UpdateCursorIfNeeded();
                        }
                      });

    // FIXME: Paint and event timing, selection appearance and highlights, once there are any.
  }

  void Page::RenderingUpdateCompleted() noexcept
  {
    _renderingUpdateRemainingSteps = {};
    if (!_unfulfilledRequestedSteps.IsEmpty())
    {
      _renderingUpdateIsScheduled = true;
      _unfulfilledRequestedSteps = {};
    }
  }

  void Page::LayoutIfNeeded() noexcept
  {
    // FIXME: Child frames, once there are frames.
    if (RefPtr<dom::Document> document = ShareRefPtr(_mainFrame->Document()); document && _mainFrame->View())
    {
      (void)document->UpdateLayout();
    }
  }

  void Page::ForEachDocument(const Func<void(dom::Document &)> &functor) const noexcept
  {
    // FIXME: Child frames, once there are frames.
    if (RefPtr<dom::Document> document = ShareRefPtr(_mainFrame->Document()))
    {
      functor(*document);
    }
  }

  void Page::ForEachRenderableDocument(const Func<void(dom::Document &)> &functor) const noexcept
  {
    List<Ref<dom::Document>> documents;
    ForEachDocument([&](dom::Document &document) { documents.push_back(ShareRef(document)); });
    for (auto &document : documents)
    {
      functor(*document);
    }
  }

  void Page::RunProcessingStep(RenderingUpdateStep step,
                               const Func<void(dom::Document &)> &perDocumentFunction) noexcept
  {
    _renderingUpdateRemainingSteps.Remove(step);
    ForEachRenderableDocument(perDocumentFunction);
  }
}
