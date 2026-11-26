/*
 * Copyright (C) 2000 Lars Knoll (knoll@kde.org)
 *           (C) 2000 Antti Koivisto (koivisto@kde.org)
 *           (C) 2000 Dirk Mueller (mueller@kde.org)
 * Copyright (C) 2003-2017 Apple Inc. All rights reserved.
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

#include "Krystal.Core/Memory/DataRef.hpp"
#include "Krystal.Web/Graphics/Fonts/FontCascade.hpp"
#include "Krystal.Web/Rendering/Style/StyleFontData.hpp"
#include "Krystal.Web/Style/Values/Color/Color.hpp"
#include "Krystal.Web/Style/Values/CSS2/BorderSpacing.hpp"
#include "Krystal.Web/Style/Values/Inline/LineHeight.hpp"

namespace krys::web
{
  class StyleInheritedData : public RefCounted<StyleInheritedData>
  {
  public:
    // Copied only through Copy(), and never assigned.
    StyleInheritedData &operator=(const StyleInheritedData &) noexcept = delete;

    KRYS_NODISCARD static Ref<StyleInheritedData> Create() noexcept
    {
      return AdoptRef(*new StyleInheritedData);
    }

    KRYS_NODISCARD Ref<StyleInheritedData> Copy() const noexcept;

    bool operator==(const StyleInheritedData &) const noexcept;

    KRYS_NODISCARD bool FastPathInheritedEqual(const StyleInheritedData &) const noexcept;

    KRYS_NODISCARD bool NonFastPathInheritedEqual(const StyleInheritedData &) const noexcept;

    void FastPathInheritFrom(const StyleInheritedData &) noexcept;

    style::BorderSpacing BorderSpacing;

    style::LineHeight LineHeight;

    DataRef<StyleFontData> FontData;
    web::Color Color;

  private:
    StyleInheritedData() noexcept;

    StyleInheritedData(const StyleInheritedData &) noexcept;
  };
}
