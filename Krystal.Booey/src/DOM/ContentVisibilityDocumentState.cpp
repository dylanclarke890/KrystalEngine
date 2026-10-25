#include "Krystal.Booey/DOM/ContentVisibilityDocumentState.hpp"
#include "Krystal.Booey/Animation/AnimationTimeline.hpp"
#include "Krystal.Booey/Animation/StyleOriginatedAnimation.hpp"
#include "Krystal.Booey/Constants/EventNames.hpp"
#include "Krystal.Booey/DOM/Algorithms/EventDispatcher.hpp"
#include "Krystal.Booey/DOM/ContentVisibilityAutoStateChangeEvent.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/DOM/SimpleRange.hpp"
#include "Krystal.Booey/Editing/FrameSelection.hpp"
#include "Krystal.Booey/HTML/EventLoop.hpp"
#include "Krystal.Booey/Page/IntersectionObserverEntry.hpp"
#include "Krystal.Booey/Rendering/RenderElement.hpp"

namespace krys::boo::dom
{
  namespace
  {
    void UpdateViewportProximities(const List<Ref<IntersectionObserverEntry>> &entries,
                                   IntersectionObserver &) noexcept
    {
      krys_debug_assert(!entries.empty());

      for (auto &entry : entries)
      {
        if (RefPtr element = ShareRefPtr(entry->Target()))
        {
          element->NodeDocument().GetContentVisibilityDocumentState().UpdateViewportProximity(
            *element, entry->IsIntersecting() ? ViewportProximity::Near : ViewportProximity::Far);
        }
      }
    }
  }

  void ContentVisibilityDocumentState::Observe(Element &element) noexcept
  {
    Ref document = ShareRef(element.NodeDocument());
    auto &state = document->GetContentVisibilityDocumentState();
    if (RefPtr intersectionObserver = ShareRefPtr(state.GetIntersectionObserver(*document)))
    {
      intersectionObserver->Observe(element);
    }
  }

  void ContentVisibilityDocumentState::Unobserve(Element &element) noexcept
  {
    Ref document = ShareRef(element.NodeDocument());
    auto &state = document->GetContentVisibilityDocumentState();
    if (RefPtr intersectionObserver = state._observer)
    {
      intersectionObserver->Unobserve(element);
      state.RemoveViewportProximity(element);
    }
    element.SetContentRelevancy({});
  }

  IntersectionObserver *ContentVisibilityDocumentState::GetIntersectionObserver(Document &document) noexcept
  {
    if (!_observer)
    {
      auto observer = IntersectionObserver::Create(
        document, &UpdateViewportProximities,
        {.Root = RefPtr<Document>(ShareRefPtr(&document)), .RootMargin = u8"", .ScrollMargin = u8""});
      if (observer.HasException())
      {
        return nullptr;
      }
      _observer = observer.ReleaseValue();
    }
    return _observer.get();
  }

  bool ContentVisibilityDocumentState::CheckRelevancyOfContentVisibilityElement(
    Element &target, EnumSet<ContentRelevancy> relevancyToCheck) const noexcept
  {
    auto oldRelevancy = target.GetContentRelevancy();
    EnumSet<ContentRelevancy> newRelevancy;
    if (oldRelevancy)
    {
      newRelevancy = *oldRelevancy;
    }

    auto setRelevancyValue = [&](ContentRelevancy reason, bool value)
    {
      if (value)
      {
        newRelevancy.Add(reason);
      }
      else
      {
        newRelevancy.Remove(reason);
      }
    };

    if (relevancyToCheck.Contains(ContentRelevancy::OnScreen))
    {
      auto viewportProximityIterator = _elementViewportProximities.find(&target);
      auto viewportProximity = ViewportProximity::Far;
      if (viewportProximityIterator != _elementViewportProximities.end())
      {
        viewportProximity = viewportProximityIterator->second;
      }
      setRelevancyValue(ContentRelevancy::OnScreen, viewportProximity == ViewportProximity::Near);
    }

    if (relevancyToCheck.Contains(ContentRelevancy::Focused))
    {
      setRelevancyValue(ContentRelevancy::Focused, target.HasFocusWithin());
    }

    auto targetContainsSelection = [](Element &target)
    {
      auto selectionRange = target.NodeDocument().Selection().Selection().Range();
      return selectionRange && Intersects<TreeType::ComposedTree>(*selectionRange, target);
    };

    if (relevancyToCheck.Contains(ContentRelevancy::Selected))
    {
      setRelevancyValue(ContentRelevancy::Selected, targetContainsSelection(target));
    }

    auto hasTopLayerInSubtree = [](const Element &target)
    {
      for (auto &element : target.NodeDocument().TopLayerElements())
      {
        if (element->IsDescendantOf(target))
        {
          return true;
        }
      }
      return false;
    };
    if (relevancyToCheck.Contains(ContentRelevancy::IsInTopLayer))
    {
      setRelevancyValue(ContentRelevancy::IsInTopLayer, hasTopLayerInSubtree(target));
    }

    if (oldRelevancy && *oldRelevancy == newRelevancy)
    {
      return false;
    }

    auto wasSkippedContent = target.IsRelevantToUser() ? IsSkippedContent::No : IsSkippedContent::Yes;
    target.SetContentRelevancy(newRelevancy);
    auto isSkippedContent = target.IsRelevantToUser() ? IsSkippedContent::No : IsSkippedContent::Yes;
    target.InvalidateStyle();
    UpdateAnimations(target, wasSkippedContent, isSkippedContent);
    html::EventLoop::Current().QueueTask(
      html::TaskSource::DOMManipulation, target.NodeDocument(),
      [protectedTarget = ShareRef(target), isSkippedContent]
      {
        if (protectedTarget->IsConnected())
        {
          ContentVisibilityAutoStateChangeEventInit init;
          init.Skipped = isSkippedContent == IsSkippedContent::Yes;
          auto event = ContentVisibilityAutoStateChangeEvent::Create(
            DOMStringAtom(EventNames::ContentVisibilityAutoStateChange), init);
          (void)EventDispatcher::DispatchToTarget(*event, protectedTarget.get());
        }
      });
    return true;
  }

  DidUpdateAnyContentRelevancy ContentVisibilityDocumentState::UpdateRelevancyOfContentVisibilityElements(
    EnumSet<ContentRelevancy> relevancyToCheck) const noexcept
  {
    auto didUpdateAnyContentRelevancy = DidUpdateAnyContentRelevancy::No;
    // Updating relevancy queues tasks and invalidates style, but doesn't change the targets.
    for (auto &weakTarget : _observer->ObservationTargets())
    {
      if (RefPtr target = ShareRefPtr(weakTarget.get()))
      {
        if (CheckRelevancyOfContentVisibilityElement(*target, relevancyToCheck))
        {
          didUpdateAnyContentRelevancy = DidUpdateAnyContentRelevancy::Yes;
        }
      }
    }
    return didUpdateAnyContentRelevancy;
  }

  HadInitialVisibleContentVisibilityDetermination
    ContentVisibilityDocumentState::DetermineInitialVisibleContentVisibility() const noexcept
  {
    if (!_observer)
    {
      return HadInitialVisibleContentVisibilityDetermination::No;
    }
    List<Ref<Element>> elementsToCheck;
    for (auto &weakTarget : _observer->ObservationTargets())
    {
      if (RefPtr target = ShareRefPtr(weakTarget.get()))
      {
        bool checkForInitialDetermination =
          !_elementViewportProximities.contains(target.get()) && !target->IsRelevantToUser();
        if (checkForInitialDetermination)
        {
          elementsToCheck.push_back(ShareRef(*target));
        }
      }
    }
    auto hadInitialVisibleContentVisibilityDetermination =
      HadInitialVisibleContentVisibilityDetermination::No;
    if (!elementsToCheck.empty())
    {
      Ref document = ShareRef(elementsToCheck.front()->NodeDocument());
      document->UpdateIntersectionObservations({CreateWeakPtr(_observer.get())});
      for (auto &element : elementsToCheck)
      {
        (void)CheckRelevancyOfContentVisibilityElement(*element, {ContentRelevancy::OnScreen});
        if (element->IsRelevantToUser())
        {
          hadInitialVisibleContentVisibilityDetermination =
            HadInitialVisibleContentVisibilityDetermination::Yes;
        }
      }
    }
    return hadInitialVisibleContentVisibilityDetermination;
  }

  // Make sure any skipped content we want to scroll to is in the viewport, so it can be actually scrolled to
  // (i.e. the skipped content early exit in LocalFrameView::ScrollRectToVisible does not apply anymore).
  void ContentVisibilityDocumentState::UpdateContentRelevancyForScrollIfNeeded(
    const Element &scrollAnchor) noexcept
  {
    if (!_observer)
    {
      return;
    }
    auto findSkippedContentRoot = [](const Element &element) -> RefPtr<const Element>
    {
      RefPtr<const Element> found;
      if (element.Renderer() && element.Renderer()->IsSkippedContent())
      {
        for (RefPtr candidate = ShareRefPtr(&element); candidate;
             candidate = ShareRefPtr<const Element>(candidate->ParentElementInComposedTree()))
        {
          if (candidate->Renderer()
              && candidate->Renderer()->Style().GetContentVisibility() == ContentVisibility::Auto)
          {
            found = candidate;
          }
        }
      }
      return found;
    };

    if (RefPtr scrollAnchorRoot = findSkippedContentRoot(scrollAnchor))
    {
      UpdateViewportProximity(*scrollAnchorRoot, ViewportProximity::Near);
      // Since we may not have determined initial visibility yet, force scheduling the content relevancy
      // update.
      Ref document = ShareRef(scrollAnchorRoot->NodeDocument());
      document->ScheduleContentRelevancyUpdate(ContentRelevancy::OnScreen);
      (void)document->UpdateRelevancyOfContentVisibilityElements();
    }
  }

  void ContentVisibilityDocumentState::UpdateViewportProximity(const Element &element,
                                                               ViewportProximity viewportProximity) noexcept
  {
    // No need to schedule content relevancy update for first time call, since that will be handled by
    // DetermineInitialVisibleContentVisibility.
    auto [iterator, inserted] = _elementViewportProximities.try_emplace(&element, ViewportProximity::Far);
    if (!inserted)
    {
      element.NodeDocument().ScheduleContentRelevancyUpdate(ContentRelevancy::OnScreen);
    }
    iterator->second = viewportProximity;
  }

  void ContentVisibilityDocumentState::RemoveViewportProximity(const Element &element) noexcept
  {
    _elementViewportProximities.erase(&element);
  }

  void ContentVisibilityDocumentState::UpdateAnimations(const Element &element, IsSkippedContent wasSkipped,
                                                        IsSkippedContent becomesSkipped) noexcept
  {
    if (wasSkipped == IsSkippedContent::No || becomesSkipped == IsSkippedContent::Yes)
    {
      return;
    }
    for (auto *animation : WebAnimation::Instances())
    {
      RefPtr styleOriginatedAnimation = ShareRefPtr(DynamicDowncast<StyleOriginatedAnimation>(animation));
      if (!styleOriginatedAnimation)
      {
        continue;
      }
      auto owningElement = styleOriginatedAnimation->OwningElement();
      if (!owningElement || !owningElement->Element.IsShadowIncludingDescendantOf(&element))
      {
        continue;
      }

      if (RefPtr timeline = ShareRefPtr(styleOriginatedAnimation->Timeline()))
      {
        timeline->AnimationTimingDidChange(*styleOriginatedAnimation);
      }
    }
  }
}
