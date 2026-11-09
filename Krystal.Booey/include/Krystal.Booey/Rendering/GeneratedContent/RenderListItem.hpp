/*
 * Copyright (C) 1999 Lars Knoll (knoll@kde.org)
 *           (C) 1999 Antti Koivisto (koivisto@kde.org)
 * Copyright (C) 2003-2023 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/Rendering/RenderBlockFlow.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/TypeCast.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/WeakPtr.hpp"

namespace krys::boo::html
{
  class HTMLOListElement;
}

namespace krys::boo
{
  class RenderListMarker;

  /// @brief A list item: a block flow with a marker and an ordinal value.
  class RenderListItem final : public RenderBlockFlow
  {
    KRYS_NON_COPY_MOVABLE(RenderListItem);
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(RenderListItem);

  private:
    WeakPtr<RenderListMarker> _marker;
    mutable Maybe<int32> _value;

  public:
    RenderListItem(dom::Element &element, RenderStyle &&style) noexcept;
    ~RenderListItem() noexcept override;

    void Paint(PaintInfo &, const LayoutPoint &) noexcept final;

    KRYS_NODISCARD utf8_stringview RenderName() const noexcept final
    {
      return u8"RenderListItem";
    }

    /// @brief The item's ordinal value.
    KRYS_NODISCARD int32 Value() const noexcept
    {
      if (!_value)
      {
        UpdateValueNow();
      }
      return *_value;
    }
    void UpdateValue() noexcept;

    KRYS_NODISCARD utf8_string MarkerTextWithoutSuffix() const noexcept;
    KRYS_NODISCARD utf8_string MarkerTextWithSuffix() const noexcept;

    void UpdateListMarkerNumbers() noexcept;

    static void UpdateItemValuesForOrderedList(const html::HTMLOListElement &list) noexcept;
    KRYS_NODISCARD static uint32 ItemCountForOrderedList(const html::HTMLOListElement &list) noexcept;

    KRYS_NODISCARD RenderStyle ComputeMarkerStyle() const noexcept;

    KRYS_NODISCARD RenderListMarker *MarkerRenderer() const noexcept;
    void SetMarkerRenderer(RenderListMarker &marker) noexcept;

    KRYS_NODISCARD bool IsInReversedOrderedList() const noexcept;

  private:
    void StyleDidChange(style::Difference diff, const RenderStyle *oldStyle) noexcept final;

    void ComputePreferredLogicalWidths() noexcept final;

    void UpdateValueNow() const noexcept;
    void CounterDirectivesChanged() noexcept;
  };

  /// @brief Whether @p node is a `<ul>` or `<ol>`.
  KRYS_NODISCARD bool IsHTMLListElement(const dom::Node &node) noexcept;
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::RenderListItem)
  KRYS_NODISCARD static bool IsType(const krys::boo::RenderObject &renderer) noexcept
  {
    return renderer.IsRenderListItem();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
