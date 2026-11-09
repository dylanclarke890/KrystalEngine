/*
 * Copyright 2008, The Android Open Source Project
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *  * Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include "Krystal.Booey/DOM/Events/Touch.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/RefCounted.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::dom
{
  /// @brief A list of touches, as a touch event's `touches`, `targetTouches` and `changedTouches`.
  /// @see https://w3c.github.io/touch-events/#touchlist-interface
  class TouchList : public RefCounted<TouchList>
  {
  private:
    List<Ref<Touch>> _values;

  public:
    KRYS_NODISCARD static Ref<TouchList> Create() noexcept
    {
      return AdoptRef(*new TouchList);
    }

    KRYS_NODISCARD static Ref<TouchList> Create(List<Ref<Touch>> touches) noexcept
    {
      return AdoptRef(*new TouchList(krys::move(touches)));
    }

    /// @see https://w3c.github.io/touch-events/#dom-touchlist-length
    KRYS_NODISCARD uint32 Length() const noexcept
    {
      return static_cast<uint32>(_values.size());
    }

    KRYS_NODISCARD bool IsSupportedPropertyIndex(uint32 index) const noexcept
    {
      return index < Length();
    }

    /// @see https://w3c.github.io/touch-events/#dom-touchlist-item
    KRYS_NODISCARD Touch *Item(uint32 index) const noexcept;

    void Append(Ref<Touch> &&touch) noexcept
    {
      _values.push_back(krys::move(touch));
    }

  private:
    TouchList() noexcept = default;

    explicit TouchList(List<Ref<Touch>> touches) noexcept : _values(krys::move(touches))
    {
    }
  };
}
