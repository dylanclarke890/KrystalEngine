/*
 * Copyright (C) 2019 Igalia S.L.
 * Copyright (C) 2022 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. AND ITS CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL APPLE INC. OR ITS CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "Krystal.Booey/Page/ResizeObserver/ResizeObserver.hpp"
#include "Krystal.Booey/DOM/Nodes/Document.hpp"
#include "Krystal.Booey/DOM/Nodes/Element.hpp"
#include "Krystal.Booey/Page/ResizeObserver/ResizeObserverEntry.hpp"
#include <algorithm>

namespace krys::boo
{
  Ref<ResizeObserver> ResizeObserver::Create(dom::Document &document,
                                             ResizeObserverCallback &&callback) noexcept
  {
    return AdoptRef(*new ResizeObserver(document, krys::move(callback), false));
  }

  Ref<ResizeObserver> ResizeObserver::CreateNativeObserver(dom::Document &document,
                                                           ResizeObserverCallback &&callback) noexcept
  {
    return AdoptRef(*new ResizeObserver(document, krys::move(callback), true));
  }

  ResizeObserver::ResizeObserver(dom::Document &document, ResizeObserverCallback &&callback,
                                 bool isNative) noexcept
      : _document(CreateWeakPtr(&document)), _callback(krys::move(callback)), _isNative(isNative)
  {
  }

  ResizeObserver::~ResizeObserver() noexcept
  {
    Disconnect();
    if (_document)
    {
      _document->RemoveResizeObserver(*this);
    }
  }

  void ResizeObserver::ObserveInternal(dom::Element &target, ResizeObserverBoxOptions boxOptions) noexcept
  {
    auto position = std::ranges::find_if(_observations,
                                         [&](auto &observation) { return observation->Target() == &target; });
    if (position != _observations.end())
    {
      // The spec suggests unconditionally unobserving here, but that causes a test failure:
      // https://github.com/web-platform-tests/wpt/issues/30708
      if ((*position)->ObservedBox() == boxOptions)
      {
        return;
      }
      Unobserve(target);
    }

    auto &observerData = target.EnsureResizeObserverData();
    observerData.Observers.push_back(CreateWeakPtr(this));
    _observations.push_back(ResizeObservation::Create(target, boxOptions));

    // Per the specification, we should dispatch at least one observation for the target. For this reason, we
    // make sure to keep the target alive until this first observation.
    _targetsWaitingForFirstObservation.push_back(ShareRef(target));

    if (_document && !_isNative)
    {
      _document->AddResizeObserver(*this);
      _document->ScheduleRenderingUpdate({RenderingUpdateStep::ResizeObservations});
    }
  }

  void ResizeObserver::Observe(dom::Element &target, const ResizeObserverOptions &options) noexcept
  {
    ObserveInternal(target, options.Box);
  }

  void ResizeObserver::Unobserve(dom::Element &target) noexcept
  {
    if (!RemoveTarget(target))
    {
      return;
    }
    RemoveObservation(target);
  }

  void ResizeObserver::Disconnect() noexcept
  {
    RemoveAllTargets();
  }

  void ResizeObserver::TargetDestroyed(dom::Element &target) noexcept
  {
    RemoveObservation(target);
  }

  size_t ResizeObserver::GatherObservations(size_t deeperThan) noexcept
  {
    _hasSkippedObservations = false;
    size_t minObservedDepth = MaxElementDepth();
    for (const auto &observation : _observations)
    {
      if (auto currentSizes = observation->ElementSizeChanged())
      {
        size_t depth = observation->TargetElementDepth();
        if (depth > deeperThan)
        {
          observation->UpdateObservationSize(*currentSizes);
          _activeObservations.push_back(observation);
          _activeObservationTargets.push_back(ShareRef(*observation->Target()));
          minObservedDepth = std::min(depth, minObservedDepth);
        }
        else
        {
          _hasSkippedObservations = true;
        }
      }
    }
    return minObservedDepth;
  }

  void ResizeObserver::DeliverObservations() noexcept
  {
    List<Ref<ResizeObserverEntry>> entries;
    entries.reserve(_activeObservations.size());
    for (auto &observation : _activeObservations)
    {
      // The target is kept alive by `_activeObservationTargets`.
      auto *target = observation->Target();
      krys_debug_assert(target);
      if (!target)
      {
        continue;
      }
      entries.push_back(ResizeObserverEntry::Create(ShareRef(*target), observation->ComputeContentRect(),
                                                    observation->BorderBoxSize(),
                                                    observation->ContentBoxSize()));
    }
    _activeObservations.clear();

    // The targets are kept alive while the callback runs, since their lists are cleared first.
    auto activeObservationTargets = std::exchange(_activeObservationTargets, {});
    auto targetsWaitingForFirstObservation = std::exchange(_targetsWaitingForFirstObservation, {});

    Ref<ResizeObserver> protectedThis = ShareRef(*this);
    _callback(entries, *this);
  }

  bool ResizeObserver::RemoveTarget(dom::Element &target) noexcept
  {
    auto *observerData = target.ResizeObserverDataIfExists();
    if (!observerData)
    {
      return false;
    }
    return RemoveFirstMatching(observerData->Observers,
                               [this](auto &observer) { return observer.get() == this; });
  }

  void ResizeObserver::RemoveAllTargets() noexcept
  {
    for (auto &observation : _observations)
    {
      bool removed = RemoveTarget(*observation->Target());
      krys_debug_assert(removed);
      (void)removed;
    }
    _activeObservationTargets.clear();
    _targetsWaitingForFirstObservation.clear();
    _activeObservations.clear();
    _observations.clear();
  }

  bool ResizeObserver::RemoveObservation(const dom::Element &target) noexcept
  {
    RemoveFirstMatching(_targetsWaitingForFirstObservation,
                        [&target](auto &pendingTarget) { return pendingTarget.get() == &target; });
    return RemoveFirstMatching(_observations,
                               [&target](auto &observation) { return observation->Target() == &target; });
  }

  void ResizeObserver::ResetObservationSize(dom::Element &target) noexcept
  {
    auto position = std::ranges::find_if(_observations,
                                         [&](auto &observation) { return observation->Target() == &target; });
    if (position != _observations.end())
    {
      (*position)->ResetObservationSize();
    }
  }
}
