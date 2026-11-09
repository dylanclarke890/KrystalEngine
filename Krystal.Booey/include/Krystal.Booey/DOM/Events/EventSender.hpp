/*
 * Copyright (C) 2012-2024 Apple Inc. All rights reserved.
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

#pragma once

#include "Krystal.Booey/Platform/Timer.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/StringAtom.hpp"
#include <algorithm>
#include <utility>

namespace krys::boo::dom
{
  /// @brief Dispatches events of senders soon, on a zero-delay timer, in the order they were asked for.
  /// @note Senders cancel their events before they're destroyed.
  // FIXME: Dispatch only one page's events (dispatchPendingEvents(Page*)), once loading images delays the
  // window's load event.
  template <typename T>
  class EventSender
  {
    KRYS_NON_COPY_MOVABLE(EventSender);

  private:
    struct DispatchTask
    {
      T *Sender {nullptr};
      StringAtom EventType;
    };

    Timer _timer;
    List<DispatchTask> _dispatchSoonList;
    List<DispatchTask> _dispatchingList;

  public:
    EventSender() noexcept : _timer(*this, &EventSender::TimerFired)
    {
    }

    void DispatchEventSoon(T &sender, const StringAtom &eventType) noexcept
    {
      _dispatchSoonList.push_back(DispatchTask {&sender, eventType});
      if (!_timer.IsActive())
      {
        _timer.StartOneShot({});
      }
    }

    void CancelEvent(T &senderToCancel) noexcept
    {
      // Remove instances of this sender from both lists.
      // Use loops because we allow multiple instances to get into the lists.
      for (auto &[sender, eventType] : _dispatchSoonList)
      {
        if (sender == &senderToCancel)
        {
          sender = nullptr;
        }
      }
      for (auto &[sender, eventType] : _dispatchingList)
      {
        if (sender == &senderToCancel)
        {
          sender = nullptr;
        }
      }
    }

    void CancelEvent(T &senderToCancel, const StringAtom &eventTypeToCancel) noexcept
    {
      // Remove instances of this sender from both lists.
      // Use loops because we allow multiple instances to get into the lists.
      for (auto &[sender, eventType] : _dispatchSoonList)
      {
        if (sender == &senderToCancel && eventType == eventTypeToCancel)
        {
          sender = nullptr;
        }
      }
      for (auto &[sender, eventType] : _dispatchingList)
      {
        if (sender == &senderToCancel && eventType == eventTypeToCancel)
        {
          sender = nullptr;
        }
      }
    }

    void DispatchPendingEvents() noexcept
    {
      // Need to avoid re-entering this function; if new dispatches are
      // scheduled before the parent finishes processing the list, they
      // will set a timer and eventually be processed.
      if (!_dispatchingList.empty())
      {
        return;
      }

      _timer.Stop();

      _dispatchingList = std::exchange(_dispatchSoonList, {});
      // Index rather than iterate: cancelling an event while dispatching another nulls its sender in place.
      for (size_t i = 0; i < _dispatchingList.size(); ++i)
      {
        auto *sender = std::exchange(_dispatchingList[i].Sender, nullptr);
        if (!sender)
        {
          continue;
        }
        sender->DispatchPendingEvent(this, _dispatchingList[i].EventType);
      }
      _dispatchingList.clear();
    }

    KRYS_NODISCARD bool HasPendingEvents(T &sender) const noexcept
    {
      auto isSender = [&](const DispatchTask &task)
      {
        return task.Sender == &sender;
      };
      return std::ranges::any_of(_dispatchSoonList, isSender)
             || std::ranges::any_of(_dispatchingList, isSender);
    }

  private:
    void TimerFired() noexcept
    {
      DispatchPendingEvents();
    }
  };
}
