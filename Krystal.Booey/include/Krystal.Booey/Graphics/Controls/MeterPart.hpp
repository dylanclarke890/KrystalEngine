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

#include "Krystal.Booey/Graphics/Controls/ControlPart.hpp"

namespace krys::boo
{
  /// @brief A meter: its value in its range, and which region of the gauge it's in.
  class MeterPart : public ControlPart
  {
  public:
    enum class GaugeRegion : uint8
    {
      Optimum,
      Suboptimal,
      EvenLessGood
    };

  private:
    GaugeRegion _gaugeRegion;
    double _value;
    double _minimum;
    double _maximum;

  public:
    KRYS_NODISCARD static Ref<MeterPart> Create() noexcept;
    KRYS_NODISCARD static Ref<MeterPart> Create(GaugeRegion, double value, double minimum,
                                                double maximum) noexcept;

    KRYS_NODISCARD GaugeRegion GetGaugeRegion() const noexcept
    {
      return _gaugeRegion;
    }

    void SetGaugeRegion(GaugeRegion gaugeRegion) noexcept
    {
      _gaugeRegion = gaugeRegion;
    }

    KRYS_NODISCARD double Value() const noexcept
    {
      return _value;
    }

    void SetValue(double value) noexcept
    {
      _value = value;
    }

    KRYS_NODISCARD double Minimum() const noexcept
    {
      return _minimum;
    }

    void SetMinimum(double minimum) noexcept
    {
      _minimum = minimum;
    }

    KRYS_NODISCARD double Maximum() const noexcept
    {
      return _maximum;
    }

    void SetMaximum(double maximum) noexcept
    {
      _maximum = maximum;
    }

  private:
    MeterPart(GaugeRegion, double value, double minimum, double maximum) noexcept;

    KRYS_NODISCARD UniquePtr<PlatformControl> CreatePlatformControl() noexcept override;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_CONTROL_PART(Meter)
