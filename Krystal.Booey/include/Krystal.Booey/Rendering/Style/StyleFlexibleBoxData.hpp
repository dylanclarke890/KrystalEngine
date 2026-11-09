/*
 * Copyright (C) 2011 Google Inc. All rights reserved.
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
#include "Krystal.Booey/Style/Values/Flexbox/FlexBasis.hpp"
#include "Krystal.Booey/Style/Values/Flexbox/FlexGrow.hpp"
#include "Krystal.Booey/Style/Values/Flexbox/FlexShrink.hpp"
#include "Krystal.Core/RefCounted.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo
{
  class StyleFlexibleBoxData : public RefCounted<StyleFlexibleBoxData>
  {
  public:
    KRYS_NODISCARD static Ref<StyleFlexibleBoxData> Create() noexcept
    {
      return AdoptRef(*new StyleFlexibleBoxData);
    }
    KRYS_NODISCARD Ref<StyleFlexibleBoxData> Copy() const noexcept;

    bool operator==(const StyleFlexibleBoxData &) const noexcept;

    style::FlexGrow FlexGrow;
    style::FlexShrink FlexShrink;
    style::FlexBasis FlexBasis;

    unsigned FlexDirection : 2;
    unsigned FlexWrap : 2;

  private:
    StyleFlexibleBoxData() noexcept;
    StyleFlexibleBoxData(const StyleFlexibleBoxData &) noexcept;
  };
}
