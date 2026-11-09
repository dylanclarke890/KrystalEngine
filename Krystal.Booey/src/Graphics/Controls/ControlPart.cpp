/*
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
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. ``AS IS'' AND ANY
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

#include "Krystal.Booey/Graphics/Controls/ControlPart.hpp"
#include "Krystal.Booey/Graphics/Geometry/FloatRoundedRect.hpp"
#include "Krystal.Booey/Graphics/GraphicsContext.hpp"

namespace krys::boo
{
  ControlPart::ControlPart(StyleAppearance type) noexcept : _type(type)
  {
  }

  ControlFactory &ControlPart::GetControlFactory() const noexcept
  {
    return _overrideControlFactory ? *_overrideControlFactory : ControlFactory::Singleton();
  }

  PlatformControl *ControlPart::GetPlatformControl() const noexcept
  {
    if (!_platformControl)
    {
      _platformControl = const_cast<ControlPart &>(*this).CreatePlatformControl();
    }
    return _platformControl.get();
  }

  FloatSize ControlPart::SizeForBounds(const FloatRect &bounds, const ControlStyle &style) noexcept
  {
    auto *platformControl = GetPlatformControl();
    if (!platformControl)
    {
      return bounds.Size();
    }

    platformControl->UpdateCellStates(bounds, style);
    return platformControl->SizeForBounds(bounds, style);
  }

  FloatRect ControlPart::RectForBounds(const FloatRect &bounds, const ControlStyle &style) noexcept
  {
    auto *platformControl = GetPlatformControl();
    if (!platformControl)
    {
      return bounds;
    }

    platformControl->UpdateCellStates(bounds, style);
    return platformControl->RectForBounds(bounds, style);
  }

  void ControlPart::Draw(GraphicsContext &context, const FloatRoundedRect &borderRect,
                         float deviceScaleFactor, const ControlStyle &style) const noexcept
  {
    auto *platformControl = GetPlatformControl();
    if (!platformControl)
    {
      return;
    }

    // It's important to get the clip from the context, because it may be significantly
    // smaller than the layer bounds (e.g. tiled layers)
    platformControl->SetFocusRingClipRect(FloatRect(context.ClipBounds()));

    platformControl->UpdateCellStates(borderRect.Rect(), style);
    platformControl->Draw(context, borderRect, deviceScaleFactor, style);

    platformControl->SetFocusRingClipRect({});
  }
}
