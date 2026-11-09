/*
 * Copyright (C) 2025 Samuel Weinig <sam@webkit.org>
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

#include "Krystal.Booey/Rendering/Style/StyleMaskBorderData.hpp"

#include "Krystal.Booey/Style/Computed/ComputedStyle+InitialInlines.hpp"

namespace krys::boo
{

  Ref<StyleMaskBorderData> StyleMaskBorderData::Copy() const noexcept
  {
    return AdoptRef(*new StyleMaskBorderData(*this));
  }

  StyleMaskBorderData::StyleMaskBorderData() noexcept
      : MaskBorder {
          style::ComputedStyle::InitialMaskBorderSource(), style::ComputedStyle::InitialMaskBorderSlice(),
          style::ComputedStyle::InitialMaskBorderWidth(), style::ComputedStyle::InitialMaskBorderOutset(),
          style::ComputedStyle::InitialMaskBorderRepeat()}
  {
  }

  inline StyleMaskBorderData::StyleMaskBorderData(const StyleMaskBorderData &other) noexcept
      : RefCounted<StyleMaskBorderData>(), MaskBorder {other.MaskBorder}
  {
  }

  bool StyleMaskBorderData::operator==(const StyleMaskBorderData &other) const noexcept
  {
    return MaskBorder == other.MaskBorder;
  }
}
