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

#include "Krystal.Booey/Rendering/Tables/RenderTableCaption.hpp"
#include "Krystal.Booey/Rendering/Tables/RenderTable.hpp"

namespace krys::boo
{
  RenderTableCaption::RenderTableCaption(dom::Element &element, RenderStyle &&style) noexcept
      : RenderBlockFlow(Type::TableCaption, element, krys::move(style))
  {
    krys_debug_assert(IsRenderTableCaption());
  }

  // Do not add any code in below destructor. Add it to willBeDestroyed() instead.
  RenderTableCaption::~RenderTableCaption() noexcept = default;

  void RenderTableCaption::InsertedIntoTree() noexcept
  {
    RenderBlockFlow::InsertedIntoTree();
    Table()->AddCaption(*this);
  }

  void RenderTableCaption::WillBeRemovedFromTree() noexcept
  {
    RenderBlockFlow::WillBeRemovedFromTree();
    Table()->RemoveCaption(*this);
  }

  RenderTable *RenderTableCaption::Table() const noexcept
  {
    return Downcast<RenderTable>(Parent());
  }

  LayoutUnit RenderTableCaption::ContainingBlockLogicalWidthForContent() const noexcept
  {
    if (auto *containingBlock = ContainingBlock())
    {
      return containingBlock->LogicalWidth();
    }
    return {};
  }
}
