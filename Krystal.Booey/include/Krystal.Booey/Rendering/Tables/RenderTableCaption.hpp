/*
 * Copyright (C) 2011 Robert Hogan <robert@roberthogan.net>
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

#include "Krystal.Booey/Rendering/RenderBlockFlow.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/TypeCast.hpp"

namespace krys::boo
{
  class RenderTable;

  /// @brief A table caption.
  class RenderTableCaption final : public RenderBlockFlow
  {
    KRYS_NON_COPY_MOVABLE(RenderTableCaption);
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(RenderTableCaption);

  public:
    RenderTableCaption(dom::Element &element, RenderStyle &&style) noexcept;
    ~RenderTableCaption() noexcept override;

  private:
    void InsertedIntoTree() noexcept override;
    void WillBeRemovedFromTree() noexcept override;

    KRYS_NODISCARD LayoutUnit ContainingBlockLogicalWidthForContent() const noexcept final;

    KRYS_NODISCARD RenderTable *Table() const noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::RenderTableCaption)
  KRYS_NODISCARD static bool IsType(const krys::boo::RenderObject &renderer) noexcept
  {
    return renderer.IsRenderTableCaption();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
