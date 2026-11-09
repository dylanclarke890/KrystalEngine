/**
 * Copyright (C) 2023 Apple Inc. All rights reserved.
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
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDER "AS IS" AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY,
 * OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
 * TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF
 * THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include "Krystal.Booey/Rendering/Style/StyleNonInheritedData.hpp"

#include "Krystal.Booey/Rendering/Style/StyleBackgroundData.hpp"
#include "Krystal.Booey/Rendering/Style/StyleBoxData.hpp"
#include "Krystal.Booey/Rendering/Style/StyleMiscNonInheritedData.hpp"
#include "Krystal.Booey/Rendering/Style/StyleRareNonInheritedData.hpp"
#include "Krystal.Booey/Rendering/Style/StyleSurroundData.hpp"

namespace krys::boo
{

  StyleNonInheritedData::StyleNonInheritedData() noexcept
      : BoxData(StyleBoxData::Create()), BackgroundData(StyleBackgroundData::Create()),
        SurroundData(StyleSurroundData::Create()), MiscData(StyleMiscNonInheritedData::Create()),
        RareData(StyleRareNonInheritedData::Create())
  {
  }

  StyleNonInheritedData::StyleNonInheritedData(const StyleNonInheritedData &other) noexcept
      : RefCounted<StyleNonInheritedData>(), BoxData(other.BoxData), BackgroundData(other.BackgroundData),
        SurroundData(other.SurroundData), MiscData(other.MiscData), RareData(other.RareData)
  {
    krys_debug_assert_msg(other == *this, "StyleNonInheritedData should be properly copied.");
  }

  Ref<StyleNonInheritedData> StyleNonInheritedData::Create() noexcept
  {
    return AdoptRef(*new StyleNonInheritedData);
  }

  Ref<StyleNonInheritedData> StyleNonInheritedData::Copy() const noexcept
  {
    return AdoptRef(*new StyleNonInheritedData(*this));
  }

  bool StyleNonInheritedData::operator==(const StyleNonInheritedData &other) const noexcept
  {
    return BoxData == other.BoxData && BackgroundData == other.BackgroundData
           && SurroundData == other.SurroundData && MiscData == other.MiscData && RareData == other.RareData;
  }
}
