/*
 * Copyright (C) 2006 Oliver Hunt <ojh16@student.canterbury.ac.nz>
 * Copyright (C) 2006 Apple Inc. All rights reserved.
 * Copyright (C) 2007 Nikolas Zimmermann <zimmermann@kde.org>
 * Copyright (C) Research In Motion Limited 2010. All rights reserved.
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
 */

#pragma once

#include "Krystal.Booey/Rendering/Line/LegacyRootInlineBox.hpp"

namespace krys::boo
{
  class RenderSVGText;
  class SVGInlineTextBox;

  /// @brief The legacy root inline box of an SVG text renderer's single line.
  class SVGRootInlineBox final : public LegacyRootInlineBox
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(SVGRootInlineBox);

  private:
    float _logicalHeight {0};

  public:
    explicit SVGRootInlineBox(RenderSVGText &) noexcept;

    KRYS_NODISCARD float VirtualLogicalHeight() const noexcept override
    {
      return _logicalHeight;
    }
    void SetLogicalHeight(float height) noexcept
    {
      _logicalHeight = height;
    }

  private:
    KRYS_NODISCARD RenderSVGText &GetRenderSVGText() const noexcept;

    KRYS_NODISCARD bool IsSVGRootInlineBox() const noexcept override
    {
      return true;
    }
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::SVGRootInlineBox)
  KRYS_NODISCARD static bool IsType(const krys::boo::LegacyInlineBox &box) noexcept
  {
    return box.IsSVGRootInlineBox();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
