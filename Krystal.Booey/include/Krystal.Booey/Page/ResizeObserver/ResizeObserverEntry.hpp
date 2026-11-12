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

#include "Krystal.Booey/DOM/Geometry/DOMRectReadOnly.hpp"
#include "Krystal.Booey/DOM/Nodes/Element.hpp"
#include "Krystal.Booey/Graphics/Geometry/FloatRect.hpp"
#include "Krystal.Booey/Graphics/Geometry/FloatSize.hpp"
#include "Krystal.Booey/Page/ResizeObserver/ResizeObserverSize.hpp"
#include "Krystal.Core/RefCounted.hpp"
#include "Krystal.Core/Types/List.hpp"

namespace krys::boo
{
  /// @brief An observed element's new sizes, delivered to its resize observer's callback.
  /// @see https://drafts.csswg.org/resize-observer/#resize-observer-entry-interface
  class ResizeObserverEntry : public RefCounted<ResizeObserverEntry>
  {
  private:
    const Ref<dom::Element> _target;
    const Ref<dom::DOMRectReadOnly> _contentRect;
    // The specification allows several boxes for fragmented elements (multicol), but these lists only ever
    // hold one.
    List<Ref<ResizeObserverSize>> _borderBoxSizes;
    List<Ref<ResizeObserverSize>> _contentBoxSizes;
    List<Ref<ResizeObserverSize>> _devicePixelContentBoxSizes;

    ResizeObserverEntry(Ref<dom::Element> &&target, const FloatRect &contentRect, FloatSize borderBoxSize,
                        FloatSize contentBoxSize, FloatSize devicePixelContentBoxSize) noexcept
        : _target(krys::move(target)),
          _contentRect(dom::DOMRectReadOnly::Create(contentRect.X(), contentRect.Y(), contentRect.Width(),
                                                    contentRect.Height())),
          _borderBoxSizes({ResizeObserverSize::Create(borderBoxSize.Width(), borderBoxSize.Height())}),
          _contentBoxSizes({ResizeObserverSize::Create(contentBoxSize.Width(), contentBoxSize.Height())}),
          _devicePixelContentBoxSizes({ResizeObserverSize::Create(devicePixelContentBoxSize.Width(),
                                                                  devicePixelContentBoxSize.Height())})
    {
    }

  public:
    KRYS_NODISCARD static Ref<ResizeObserverEntry> Create(Ref<dom::Element> &&target,
                                                          const FloatRect &contentRect,
                                                          FloatSize borderBoxSize, FloatSize contentBoxSize,
                                                          FloatSize devicePixelContentBoxSize) noexcept
    {
      return AdoptRef(*new ResizeObserverEntry(krys::move(target), contentRect, borderBoxSize, contentBoxSize,
                                               devicePixelContentBoxSize));
    }

    /// @see https://drafts.csswg.org/resize-observer/#dom-resizeobserverentry-target
    KRYS_NODISCARD dom::Element &Target() const noexcept
    {
      return *_target;
    }

    /// @see https://drafts.csswg.org/resize-observer/#dom-resizeobserverentry-contentrect
    KRYS_NODISCARD dom::DOMRectReadOnly &ContentRect() const noexcept
    {
      return *_contentRect;
    }

    /// @see https://drafts.csswg.org/resize-observer/#dom-resizeobserverentry-borderboxsize
    KRYS_NODISCARD const List<Ref<ResizeObserverSize>> &BorderBoxSize() const noexcept
    {
      return _borderBoxSizes;
    }

    /// @see https://drafts.csswg.org/resize-observer/#dom-resizeobserverentry-contentboxsize
    KRYS_NODISCARD const List<Ref<ResizeObserverSize>> &ContentBoxSize() const noexcept
    {
      return _contentBoxSizes;
    }

    /// @see https://drafts.csswg.org/resize-observer/#dom-resizeobserverentry-devicepixelcontentboxsize
    KRYS_NODISCARD const List<Ref<ResizeObserverSize>> &DevicePixelContentBoxSize() const noexcept
    {
      return _devicePixelContentBoxSizes;
    }
  };
}
