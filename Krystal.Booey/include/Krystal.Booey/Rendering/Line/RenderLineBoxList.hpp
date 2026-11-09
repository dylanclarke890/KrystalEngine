/*
 * Copyright (C) 2009 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1.  Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 * 2.  Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 * 3.  Neither the name of Apple Inc. ("Apple") nor the names of
 *     its contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE AND ITS CONTRIBUTORS "AS IS" AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL APPLE OR ITS CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include "Krystal.Booey/Graphics/Geometry/LayoutUnit.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Types/UniquePtr.hpp"

namespace krys::boo
{
  class LegacyInlineFlowBox;
  class RenderBoxModelObject;

  /// @brief The legacy inline boxes an inline renderer has, one for each line it's on.
  class RenderLineBoxList
  {
    KRYS_NON_COPY_MOVABLE(RenderLineBoxList);

  private:
    // For block flows, each box represents the root inline box for a line in the paragraph.
    // For inline flows, each box represents a portion of that inline.
    LegacyInlineFlowBox *_firstLineBox {nullptr};
    LegacyInlineFlowBox *_lastLineBox {nullptr};

  public:
    RenderLineBoxList() noexcept = default;
    ~RenderLineBoxList() noexcept;

    KRYS_NODISCARD LegacyInlineFlowBox *FirstLegacyLineBox() const noexcept
    {
      return _firstLineBox;
    }
    KRYS_NODISCARD LegacyInlineFlowBox *LastLegacyLineBox() const noexcept
    {
      return _lastLineBox;
    }

    void AppendLineBox(UniquePtr<LegacyInlineFlowBox>) noexcept;

    void DeleteLineBoxTree() noexcept;
    void DeleteLineBoxes() noexcept;

    void RemoveLineBox(LegacyInlineFlowBox *) noexcept;

    void DirtyLineBoxes() noexcept;
    void DirtyLineFromChangedChild(RenderBoxModelObject &parent) noexcept;
    void ShiftLinesBy(LayoutUnit shiftX, LayoutUnit shiftY) noexcept;
  };
}
