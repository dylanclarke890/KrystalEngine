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
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include "Krystal.Booey/CSS/Queries/MediaQuery.hpp"

namespace krys::boo::css::mq::Features
{
  KRYS_NODISCARD const FeatureSchema &AnyHover() noexcept;
  KRYS_NODISCARD const FeatureSchema &AnyPointer() noexcept;
  KRYS_NODISCARD const FeatureSchema &AspectRatio() noexcept;
  KRYS_NODISCARD const FeatureSchema &Color() noexcept;
  KRYS_NODISCARD const FeatureSchema &ColorGamut() noexcept;
  KRYS_NODISCARD const FeatureSchema &ColorIndex() noexcept;
  KRYS_NODISCARD const FeatureSchema &DeviceAspectRatio() noexcept;
  KRYS_NODISCARD const FeatureSchema &DeviceHeight() noexcept;
  KRYS_NODISCARD const FeatureSchema &DeviceWidth() noexcept;
  KRYS_NODISCARD const FeatureSchema &DynamicRange() noexcept;
  KRYS_NODISCARD const FeatureSchema &ForcedColors() noexcept;
  KRYS_NODISCARD const FeatureSchema &Grid() noexcept;
  KRYS_NODISCARD const FeatureSchema &Height() noexcept;
  KRYS_NODISCARD const FeatureSchema &Hover() noexcept;
  KRYS_NODISCARD const FeatureSchema &InvertedColors() noexcept;
  KRYS_NODISCARD const FeatureSchema &Monochrome() noexcept;
  KRYS_NODISCARD const FeatureSchema &Orientation() noexcept;
  KRYS_NODISCARD const FeatureSchema &OverflowBlock() noexcept;
  KRYS_NODISCARD const FeatureSchema &OverflowInline() noexcept;
  KRYS_NODISCARD const FeatureSchema &Pointer() noexcept;
  KRYS_NODISCARD const FeatureSchema &PrefersContrast() noexcept;
  KRYS_NODISCARD const FeatureSchema &PrefersReducedMotion() noexcept;
  KRYS_NODISCARD const FeatureSchema &Resolution() noexcept;
  KRYS_NODISCARD const FeatureSchema &Scan() noexcept;
  KRYS_NODISCARD const FeatureSchema &Scripting() noexcept;
  KRYS_NODISCARD const FeatureSchema &Update() noexcept;
  KRYS_NODISCARD const FeatureSchema &Width() noexcept;
  KRYS_NODISCARD const FeatureSchema &DisplayMode() noexcept;
  KRYS_NODISCARD const FeatureSchema &PrefersColorScheme() noexcept;

  KRYS_NODISCARD List<const FeatureSchema *> AllSchemas() noexcept;
}
