/*
 * Copyright (C) 2011 Google Inc. All rights reserved.
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
 *  THIS SOFTWARE IS PROVIDED BY APPLE INC. AND ITS CONTRIBUTORS ``AS IS'' AND ANY
 *  EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 *  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 *  DISCLAIMED. IN NO EVENT SHALL APPLE INC. OR ITS CONTRIBUTORS BE LIABLE FOR ANY
 *  DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 *  (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 *  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 *  ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 *  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#pragma once

#include "Krystal.Booey/Rendering/Style/RenderStyleConstants.hpp"
#include "Krystal.Booey/Style/Values/Grid/GridAutoFlow.hpp"
#include "Krystal.Booey/Style/Values/Grid/GridTemplateAreas.hpp"
#include "Krystal.Booey/Style/Values/Grid/GridTemplateList.hpp"
#include "Krystal.Booey/Style/Values/Grid/GridTrackSizes.hpp"
#include "Krystal.Core/RefCounted.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo
{
  class RenderStyle;
  class StyleGridData : public RefCounted<StyleGridData>
  {
  public:
    KRYS_NODISCARD static Ref<StyleGridData> Create() noexcept
    {
      return AdoptRef(*new StyleGridData);
    }
    KRYS_NODISCARD Ref<StyleGridData> Copy() const noexcept;

    bool operator==(const StyleGridData &) const noexcept;

    style::GridAutoFlow GridAutoFlow;
    style::GridTrackSizes GridAutoColumns;
    style::GridTrackSizes GridAutoRows;
    style::GridTemplateAreas GridTemplateAreas;
    style::GridTemplateList GridTemplateColumns;
    style::GridTemplateList GridTemplateRows;

  private:
    StyleGridData() noexcept;
    StyleGridData(const StyleGridData &) noexcept;
  };
}
