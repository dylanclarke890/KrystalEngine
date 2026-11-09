/*
 * Copyright (C) 2008 Torch Mobile Inc. All rights reserved. (http://www.torchmobile.com/)
 * Copyright (C) 2009 Apple Inc. All rights reserved.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 *
 */

#pragma once

#include "Krystal.Booey/Rendering/FormControls/RenderTextControl.hpp"

namespace krys::boo::html
{
  class HTMLTextAreaElement;
}

namespace krys::boo
{
  /// @brief The renderer of a `<textarea>`: lines of text that wrap and scroll, sized by its `cols` and
  /// `rows`.
  class RenderTextControlMultiLine final : public RenderTextControl
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(RenderTextControlMultiLine);

  public:
    RenderTextControlMultiLine(html::HTMLTextAreaElement &element, RenderStyle &&style) noexcept;
    ~RenderTextControlMultiLine() noexcept override;

    KRYS_NODISCARD html::HTMLTextAreaElement &TextAreaElement() const noexcept;

  private:
    bool NodeAtPoint(const HitTestRequest &request, HitTestResult &result,
                     const HitTestLocation &locationInContainer, const LayoutPoint &accumulatedOffset,
                     HitTestAction hitTestAction) noexcept override;

    KRYS_NODISCARD float GetAverageCharWidth() noexcept override;
    KRYS_NODISCARD LayoutUnit PreferredContentLogicalWidth(float charWidth) const noexcept override;
    KRYS_NODISCARD LayoutUnit
      ComputeControlLogicalHeight(LayoutUnit lineHeight, LayoutUnit nonContentHeight) const noexcept override;

    void LayoutExcludedChildren(RelayoutChildren relayoutChildren) noexcept override;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::RenderTextControlMultiLine)
  KRYS_NODISCARD static bool IsType(const krys::boo::RenderObject &renderer) noexcept
  {
    return renderer.IsRenderTextControlMultiLine();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
