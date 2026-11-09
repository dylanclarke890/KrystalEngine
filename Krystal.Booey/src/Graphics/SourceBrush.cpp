/*
 * Copyright (C) 2022-2023 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/Graphics/SourceBrush.hpp"
#include "Krystal.Core/Text/TextStream.hpp"

namespace krys::boo
{
  const AffineTransform &SourceBrush::GradientSpaceTransform() const noexcept
  {
    if (auto *logicalGradient = std::get_if<SourceBrushLogicalGradient>(&_patternGradient))
    {
      return logicalGradient->SpaceTransform;
    }
    return AffineTransform::Identity;
  }

  boo::Gradient *SourceBrush::GetGradient() const noexcept
  {
    if (auto *logicalGradient = std::get_if<SourceBrushLogicalGradient>(&_patternGradient))
    {
      return logicalGradient->Gradient.get();
    }
    return nullptr;
  }

  boo::Pattern *SourceBrush::GetPattern() const noexcept
  {
    if (auto *pattern = std::get_if<Ref<boo::Pattern>>(&_patternGradient))
    {
      return pattern->get();
    }
    return nullptr;
  }

  void SourceBrush::SetGradient(Ref<boo::Gradient> &&gradient, const AffineTransform &spaceTransform) noexcept
  {
    _patternGradient = SourceBrushLogicalGradient {krys::move(gradient), spaceTransform};
  }

  void SourceBrush::SetPattern(Ref<boo::Pattern> &&pattern) noexcept
  {
    _patternGradient.emplace<Ref<boo::Pattern>>(krys::move(pattern));
  }

  TextStream &operator<<(TextStream &ts, const SourceBrush &brush) noexcept
  {
    ts.DumpProperty(u8"color", brush.GetColor());
    if (auto *gradient = brush.GetGradient())
    {
      ts.DumpProperty(u8"gradient", *gradient);
      ts.DumpProperty(u8"gradient-space-transform", brush.GradientSpaceTransform());
    }
    if (auto *pattern = brush.GetPattern())
    {
      ts.DumpProperty(u8"pattern", *pattern);
    }
    return ts;
  }
}
