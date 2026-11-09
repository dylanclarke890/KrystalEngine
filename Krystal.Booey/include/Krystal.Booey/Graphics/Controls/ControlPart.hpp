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

#pragma once

#include "Krystal.Booey/Graphics/Controls/ControlFactory.hpp"
#include "Krystal.Booey/Graphics/Controls/PlatformControl.hpp"
#include "Krystal.Booey/Style/StyleAppearance.hpp"
#include "Krystal.Core/Assert.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/RefCounted.hpp"
#include "Krystal.Core/TypeCast.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include "Krystal.Core/Types/UniquePtr.hpp"

namespace krys::boo
{
  class FloatRect;
  class FloatRoundedRect;
  class GraphicsContext;

  /// @brief A form control a theme draws, of one appearance, with what it needs to draw it (a slider's thumb
  /// position, a progress bar's value).
  class ControlPart : public RefCounted<ControlPart>
  {
  protected:
    const StyleAppearance _type;

    mutable UniquePtr<PlatformControl> _platformControl;
    RefPtr<ControlFactory> _controlFactory;
    RefPtr<ControlFactory> _overrideControlFactory;

  public:
    virtual ~ControlPart() noexcept = default;

    KRYS_NODISCARD StyleAppearance GetType() const noexcept
    {
      return _type;
    }

    KRYS_NODISCARD ControlFactory &GetControlFactory() const noexcept;
    void SetOverrideControlFactory(RefPtr<ControlFactory> &&controlFactory) noexcept
    {
      _overrideControlFactory = krys::move(controlFactory);
    }

    KRYS_NODISCARD FloatSize SizeForBounds(const FloatRect &bounds, const ControlStyle &) noexcept;
    KRYS_NODISCARD FloatRect RectForBounds(const FloatRect &bounds, const ControlStyle &) noexcept;
    void Draw(GraphicsContext &, const FloatRoundedRect &borderRect, float deviceScaleFactor,
              const ControlStyle &) const noexcept;

  protected:
    explicit ControlPart(StyleAppearance) noexcept;

    KRYS_NODISCARD PlatformControl *GetPlatformControl() const noexcept;
    KRYS_NODISCARD virtual UniquePtr<PlatformControl> CreatePlatformControl() noexcept = 0;
  };
}

#define KRYS_SPECIALIZE_TYPE_CAST_TRAITS_CONTROL_PART(PartName)                                              \
  KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::PartName##Part)                                          \
    KRYS_NODISCARD static bool IsType(const krys::boo::ControlPart &part) noexcept                           \
    {                                                                                                        \
      return part.GetType() == krys::boo::StyleAppearance::PartName;                                         \
    }                                                                                                        \
  KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
