/*
 * Copyright (C) 2000 Lars Knoll (knoll@kde.org)
 *           (C) 2000 Antti Koivisto (koivisto@kde.org)
 *           (C) 2000 Dirk Mueller (mueller@kde.org)
 * Copyright (C) 2003, 2005, 2006, 2007, 2008 Apple Inc. All rights reserved.
 * Copyright (C) 2006 Graham Dennis (graham.dennis@gmail.com)
 * Copyright (C) 2025 Samuel Weinig <sam@webkit.org>
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

#include "Krystal.Booey/Rendering/Style/RenderStyleConstants.hpp"
#include "Krystal.Booey/Style/Values/CSS2/ZIndex.hpp"
#include "Krystal.Booey/Style/Values/Inline/VerticalAlign.hpp"
#include "Krystal.Booey/Style/Values/Sizing/MaximumSize.hpp"
#include "Krystal.Booey/Style/Values/Sizing/MinimumSize.hpp"
#include "Krystal.Booey/Style/Values/Sizing/PreferredSize.hpp"
#include "Krystal.Core/RefCounted.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo
{
  class StyleBoxData : public RefCounted<StyleBoxData>
  {
  public:
    KRYS_NODISCARD static Ref<StyleBoxData> Create() noexcept
    {
      return AdoptRef(*new StyleBoxData);
    }
    KRYS_NODISCARD Ref<StyleBoxData> Copy() const noexcept;

    bool operator==(const StyleBoxData &) const noexcept;

    style::PreferredSize Width;
    style::PreferredSize Height;

    style::MinimumSize MinWidth;
    style::MinimumSize MinHeight;

    style::MaximumSize MaxWidth;
    style::MaximumSize MaxHeight;

    style::VerticalAlign VerticalAlign;

    uint8 HasAutoSpecifiedZIndex : 1;
    uint8 HasAutoUsedZIndex : 1;
    uint8 BoxSizing : 1;
    uint8 BoxDecorationBreak : 1;

    style::ZIndex::Value SpecifiedZIndexValue;
    style::ZIndex::Value UsedZIndexValue;

    KRYS_NODISCARD style::ZIndex SpecifiedZIndex() const noexcept
    {
      return {static_cast<bool>(HasAutoSpecifiedZIndex), SpecifiedZIndexValue};
    }
    KRYS_NODISCARD style::ZIndex UsedZIndex() const noexcept
    {
      return {static_cast<bool>(HasAutoUsedZIndex), UsedZIndexValue};
    }

  private:
    StyleBoxData() noexcept;
    StyleBoxData(const StyleBoxData &) noexcept;
  };
}
