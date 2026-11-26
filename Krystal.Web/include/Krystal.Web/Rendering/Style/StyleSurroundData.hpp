/*
 * Copyright (C) 2000 Lars Knoll (knoll@kde.org)
 *           (C) 2000 Antti Koivisto (koivisto@kde.org)
 *           (C) 2000 Dirk Mueller (mueller@kde.org)
 * Copyright (C) 2003, 2005, 2006, 2007, 2008 Apple Inc. All rights reserved.
 * Copyright (C) 2006 Graham Dennis (graham.dennis@gmail.com)
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

#include "Krystal.Core/Memory/RefCounted.hpp"
#include "Krystal.Core/Memory/RefPtr.hpp"
#include "Krystal.Web/Rendering/Style/BorderData.hpp"
#include "Krystal.Web/Style/Values/Box/Margin.hpp"
#include "Krystal.Web/Style/Values/Box/Padding.hpp"
#include "Krystal.Web/Style/Values/Position/Inset.hpp"

namespace krys::web
{
  class StyleSurroundData : public RefCounted<StyleSurroundData>
  {
  public:
    KRYS_NODISCARD static Ref<StyleSurroundData> Create() noexcept
    {
      return AdoptRef(*new StyleSurroundData);
    }

    KRYS_NODISCARD Ref<StyleSurroundData> Copy() const noexcept;

    bool operator==(const StyleSurroundData &) const noexcept;

    // Here instead of in BorderData to pack up against the refcount.
    bool HasExplicitlySetBorderBottomLeftRadius : 1 {false};
    bool HasExplicitlySetBorderBottomRightRadius : 1 {false};
    bool HasExplicitlySetBorderTopLeftRadius : 1 {false};
    bool HasExplicitlySetBorderTopRightRadius : 1 {false};

    bool HasExplicitlySetPaddingBottom : 1 {false};
    bool HasExplicitlySetPaddingLeft : 1 {false};
    bool HasExplicitlySetPaddingRight : 1 {false};
    bool HasExplicitlySetPaddingTop : 1 {false};

    style::InsetBox Inset;
    style::MarginBox Margin;
    style::PaddingBox Padding;
    BorderData Border;

  private:
    StyleSurroundData() noexcept;

    StyleSurroundData(const StyleSurroundData &) noexcept;
  };
}
