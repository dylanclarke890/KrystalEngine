/*
 * Copyright (C) 2019 Igalia S.L.
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

#include "Krystal.Booey/DOM/ResizeObservers/ResizeObserverBoxOptions.hpp"
#include "Krystal.Booey/Graphics/Geometry/LayoutPoint.hpp"
#include "Krystal.Booey/Graphics/Geometry/LayoutSize.hpp"
#include "Krystal.Core/Geometry/Rect.hpp"
#include "Krystal.Core/Geometry/Size.hpp"
#include "Krystal.Core/RefCounted.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include "Krystal.Core/Types/WeakPtr.hpp"

namespace krys::boo
{
  class Element;

  /// @brief A resize observer's observation of one element: the box it observes and the sizes it last
  /// reported.
  /// @see https://drafts.csswg.org/resize-observer/#resizeobservation
  class ResizeObservation : public RefCounted<ResizeObservation>
  {
  public:
    struct BoxSizes
    {
      LayoutSize ContentBoxSize;
      LayoutSize ContentBoxLogicalSize;
      LayoutSize BorderBoxLogicalSize;
      /// @brief The content box's logical size in whole device pixels, snapped where it sits on the page.
      SizeF DevicePixelContentBoxLogicalSize;
    };

  private:
    WeakPtr<Element> _target;
    BoxSizes _lastObservationSizes;
    ResizeObserverBoxOptions _observedBox;

    ResizeObservation(Element &target, ResizeObserverBoxOptions observedBox) noexcept;

  public:
    KRYS_NODISCARD static Ref<ResizeObservation> Create(Element &target,
                                                        ResizeObserverBoxOptions observedBox) noexcept;

    ~ResizeObservation() noexcept;

    /// @brief The target's current sizes, if the observed box's size differs from the last reported one.
    /// @see https://drafts.csswg.org/resize-observer/#dom-resizeobservation-isactive
    KRYS_NODISCARD Maybe<BoxSizes> ElementSizeChanged() const noexcept;

    void UpdateObservationSize(const BoxSizes &boxSizes) noexcept;

    /// @brief Forgets the last reported sizes, so the next gathering reports the target.
    void ResetObservationSize() noexcept;

    KRYS_NODISCARD RectF ComputeContentRect() const noexcept;

    KRYS_NODISCARD SizeF BorderBoxSize() const noexcept;

    KRYS_NODISCARD SizeF ContentBoxSize() const noexcept;

    KRYS_NODISCARD SizeF DevicePixelContentBoxSize() const noexcept;

    KRYS_NODISCARD Element *Target() const noexcept
    {
      return _target.get();
    }

    KRYS_NODISCARD ResizeObserverBoxOptions ObservedBox() const noexcept
    {
      return _observedBox;
    }

    /// @see https://drafts.csswg.org/resize-observer/#calculate-depth-for-node
    KRYS_NODISCARD size_t TargetElementDepth() const noexcept;

  private:
    KRYS_NODISCARD Maybe<BoxSizes> ComputeObservedSizes() const noexcept;

    KRYS_NODISCARD LayoutPoint ComputeTargetLocation() const noexcept;
  };
}
