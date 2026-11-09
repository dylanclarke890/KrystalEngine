/*
 * Copyright (C) 2024 Keith Cirkel <webkit@keithcirkel.co.uk>. All rights reserved.
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

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/RefCounted.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include "Krystal.Core/Types/WeakPtr.hpp"

namespace krys::boo::dom
{
  class Element;

  enum class ToggleState : bool
  {
    Closed,
    Open,
  };

  struct ToggleEventData
  {
    ToggleState OldState;
    ToggleState NewState;
    RefPtr<Element> Source;
  };

  /// @brief An element's `toggle` event task: toggling it again before the task runs updates the queued
  /// event's new state (and source), keeping its old state, so one event reports both changes.
  /// @see https://html.spec.whatwg.org/#queue-a-popover-toggle-event-task
  /// @see https://html.spec.whatwg.org/#queue-a-dialog-toggle-event-task
  class ToggleEventTask final : public RefCounted<ToggleEventTask>
  {
    KRYS_NON_COPY_MOVABLE(ToggleEventTask);

  private:
    WeakPtr<Element> _element;
    Maybe<ToggleEventData> _data;

  public:
    explicit ToggleEventTask(Element &element) noexcept;

    KRYS_NODISCARD static Ref<ToggleEventTask> Create(Element &element) noexcept
    {
      return CreateRef<ToggleEventTask>(element);
    }

    /// @brief Queues the `toggle` event from @p oldState to @p newState, invoked by @p source, or updates the
    /// queued one.
    void Queue(ToggleState oldState, ToggleState newState, Element *source = nullptr) noexcept;
  };
}
