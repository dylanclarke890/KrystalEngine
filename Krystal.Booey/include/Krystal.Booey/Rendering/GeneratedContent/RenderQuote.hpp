/*
 * Copyright (C) 2011 Nokia Inc. All rights reserved.
 * Copyright (C) 2012 Google Inc. All rights reserved.
 * Copyright (C) 2013 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/Rendering/RenderInline.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/TypeCast.hpp"

namespace krys::boo
{
  /// @brief An `open-quote`, `close-quote`, `no-open-quote` or `no-close-quote` in generated content. Its
  /// text depends on its nesting depth, so the quotes are updated in tree order after the render tree
  /// changes.
  class RenderQuote final : public RenderInline
  {
    KRYS_NON_COPY_MOVABLE(RenderQuote);
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(RenderQuote);

  private:
    const QuoteType _type;
    int32 _depth {-1};
    utf8_string _text;

    bool _needsTextUpdate {false};

  public:
    RenderQuote(dom::Document &document, RenderStyle &&style, QuoteType quote) noexcept;
    ~RenderQuote() noexcept override;

    void UpdateRenderer(RenderTreeBuilder &builder, RenderQuote *previousQuote) noexcept;

    KRYS_NODISCARD utf8_stringview RenderName() const noexcept override
    {
      return u8"RenderQuote";
    }

  private:
    KRYS_NODISCARD bool IsOpen() const noexcept;
    void StyleDidChange(style::Difference diff, const RenderStyle *oldStyle) noexcept override;
    void InsertedIntoTree() noexcept override;
    void WillBeRemovedFromTree() noexcept override;

    KRYS_NODISCARD utf8_string ComputeText() const noexcept;
    void UpdateTextRenderer(RenderTreeBuilder &builder) noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::RenderQuote)
  KRYS_NODISCARD static bool IsType(const krys::boo::RenderObject &renderer) noexcept
  {
    return renderer.IsRenderQuote();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
