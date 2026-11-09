/*
 * Copyright (C) 2001 Peter Kelly (pmk@post.com)
 * Copyright (C) 2001 Tobias Anton (anton@stud.fbi.fh-darmstadt.de)
 * Copyright (C) 2006 Samuel Weinig (sam.weinig@gmail.com)
 * Copyright (C) 2003, 2004, 2005, 2006, 2008, 2010, 2013 Apple Inc. All rights reserved.
 * Copyright (C) 2013 Samsung Electronics. All rights reserved.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 *
 */

#pragma once

#include "Krystal.Booey/DOM/Events/MouseEvent.hpp"
#include "Krystal.Booey/Platform/Events/PlatformWheelEvent.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Numeric.hpp"
#include "Krystal.Core/Types/Maybe.hpp"

namespace krys::boo::dom
{
  /// @see https://w3c.github.io/uievents/#dictdef-wheeleventinit
  struct WheelEventInit : MouseEventInit
  {
    double DeltaX {0};
    double DeltaY {0};
    double DeltaZ {0};
    uint32 DeltaMode {0};
  };

  /// @brief A turn of the mouse wheel or a similar device: `wheel`.
  /// @see https://w3c.github.io/uievents/#interface-wheelevent
  class WheelEvent final : public MouseEvent
  {
  public:
    /// @brief The dictionary its constructor takes.
    using dictionary_type = WheelEventInit;

    enum class IsCancelable : bool
    {
      No,
      Yes
    };

    /// @see https://w3c.github.io/uievents/#dom-wheelevent-dom_delta_pixel
    static constexpr uint32 DOM_DELTA_PIXEL = 0x00;
    /// @see https://w3c.github.io/uievents/#dom-wheelevent-dom_delta_line
    static constexpr uint32 DOM_DELTA_LINE = 0x01;
    /// @see https://w3c.github.io/uievents/#dom-wheelevent-dom_delta_page
    static constexpr uint32 DOM_DELTA_PAGE = 0x02;

  private:
    double _deltaX {0};
    double _deltaY {0};
    double _deltaZ {0};
    uint32 _deltaMode {DOM_DELTA_PIXEL};
    Maybe<PlatformWheelEvent> _underlyingPlatformEvent;

  public:
    /// @see https://w3c.github.io/uievents/#dom-wheelevent-wheelevent
    WheelEvent(DOMStringAtom type, const WheelEventInit &initializer = {}) noexcept;

    /// @brief A trusted `wheel` event from @p event, which happened in @p view.
    KRYS_NODISCARD static Ref<WheelEvent> Create(const PlatformWheelEvent &event, LocalFrameView *view,
                                                 IsCancelable isCancelable = IsCancelable::Yes) noexcept;

    KRYS_NODISCARD const Maybe<PlatformWheelEvent> &UnderlyingPlatformEvent() const noexcept
    {
      return _underlyingPlatformEvent;
    }

    /// @brief Positive when scrolling right.
    /// @see https://w3c.github.io/uievents/#dom-wheelevent-deltax
    KRYS_NODISCARD double DeltaX() const noexcept
    {
      return _deltaX;
    }
    /// @brief Positive when scrolling down.
    /// @see https://w3c.github.io/uievents/#dom-wheelevent-deltay
    KRYS_NODISCARD double DeltaY() const noexcept
    {
      return _deltaY;
    }
    /// @see https://w3c.github.io/uievents/#dom-wheelevent-deltaz
    KRYS_NODISCARD double DeltaZ() const noexcept
    {
      return _deltaZ;
    }
    /// @see https://w3c.github.io/uievents/#dom-wheelevent-deltamode
    KRYS_NODISCARD uint32 DeltaMode() const noexcept
    {
      return _deltaMode;
    }

    KRYS_NODISCARD bool IsWheelEvent() const noexcept override
    {
      return true;
    }

  private:
    WheelEvent(const PlatformWheelEvent &event, LocalFrameView *view, IsCancelable isCancelable) noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::dom::WheelEvent)
  KRYS_NODISCARD static bool IsType(const krys::boo::dom::Event &event) noexcept
  {
    return event.IsWheelEvent();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
